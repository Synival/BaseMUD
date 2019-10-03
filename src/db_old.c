/***************************************************************************
 *  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,        *
 *  Michael Seifert, Hans Henrik Strfeldt, Tom Madsen, and Katja Nyboe.    *
 *                                                                         *
 *  Merc Diku Mud improvments copyright (C) 1992, 1993 by Michael          *
 *  Chastain, Michael Quan, and Mitchell Tse.                              *
 *                                                                         *
 *  In order to use any part of this Merc Diku Mud, you must comply with   *
 *  both the original Diku license in 'license.doc' as well the Merc       *
 *  license in 'license.txt'.  In particular, you may not remove either of *
 *  these copyright notices.                                               *
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
 ***************************************************************************/

/***************************************************************************
 *   ROM 2.4 is copyright 1993-1998 Russ Taylor                            *
 *   ROM has been brought to you by the ROM consortium                     *
 *       Russ Taylor (rtaylor@hypercube.org)                               *
 *       Gabrielle Taylor (gtaylor@hypercube.org)                          *
 *       Brian Moore (zump@rom.org)                                        *
 *   By using this code, you have agreed to follow the terms of the        *
 *   ROM license, in the file Rom24/doc/rom.license                        *
 ***************************************************************************/

#include <string.h>
#include <stdlib.h>

#include "db.h"
#include "recycle.h"
#include "utils.h"
#include "interp.h"
#include "lookup.h"
#include "olc.h"
#include "affects.h"

#include "db_old.h"

/* Snarf a mob section.  old style */
void load_old_mob (FILE * fp) {
    MOB_INDEX_DATA *pMobIndex;
    /* for race updating */
    int race;
    char name[MAX_STRING_LENGTH];

    if (!area_last) { /* OLC */
        bug ("load_mobiles: no #AREA seen yet.", 0);
        exit (1);
    }

    while (1) {
        sh_int vnum;
        char letter;
        int iHash;

        letter = fread_letter (fp);
        if (letter != '#') {
            bug ("load_mobiles: # not found.", 0);
            exit (1);
        }

        vnum = fread_number (fp);
        if (vnum == 0)
            break;

        fBootDb = FALSE;
        if (get_mob_index (vnum) != NULL) {
            bug ("load_mobiles: vnum %d duplicated.", vnum);
            exit (1);
        }
        fBootDb = TRUE;

        pMobIndex = mob_index_new ();
        pMobIndex->area = area_last;    /* OLC */
        pMobIndex->vnum = vnum;
        pMobIndex->anum = vnum - area_last->min_vnum;
        pMobIndex->new_format = FALSE;
        str_replace_dup (&pMobIndex->name,        fread_string (fp));
        str_replace_dup (&pMobIndex->short_descr, fread_string (fp));
        str_replace_dup (&pMobIndex->long_descr,  fread_string (fp));
        str_replace_dup (&pMobIndex->description, fread_string (fp));

        pMobIndex->long_descr[0] = UPPER (pMobIndex->long_descr[0]);
        pMobIndex->description[0] = UPPER (pMobIndex->description[0]);

        pMobIndex->mob = fread_flag (fp) | MOB_IS_NPC;
        pMobIndex->affected_by = fread_flag (fp);
        pMobIndex->pShop = NULL;
        pMobIndex->alignment = fread_number (fp);
        letter = fread_letter (fp);
        pMobIndex->level = fread_number (fp);

        /*
         * The unused stuff is for imps who want to use the old-style
         * stats-in-files method.
         */
        fread_number (fp);        /* Unused */
        fread_number (fp);        /* Unused */
        fread_number (fp);        /* Unused */
        /* 'd'      */ fread_letter (fp);
        /* Unused */
        fread_number (fp);        /* Unused */
        /* '+'      */ fread_letter (fp);
        /* Unused */
        fread_number (fp);        /* Unused */
        fread_number (fp);        /* Unused */
        /* 'd'      */ fread_letter (fp);
        /* Unused */
        fread_number (fp);        /* Unused */
        /* '+'      */ fread_letter (fp);
        /* Unused */
        fread_number (fp);        /* Unused */
        pMobIndex->wealth = fread_number (fp) / 20;
        /* xp can't be used! */ fread_number (fp);
        /* Unused */
        pMobIndex->start_pos = fread_number (fp);    /* Unused */
        pMobIndex->default_pos = fread_number (fp);    /* Unused */

        if (pMobIndex->start_pos < POS_SLEEPING)
            pMobIndex->start_pos = POS_STANDING;
        if (pMobIndex->default_pos < POS_SLEEPING)
            pMobIndex->default_pos = POS_STANDING;

        /* Back to meaningful values. */
        pMobIndex->sex = fread_number (fp);

        /* compute the race BS */
        one_argument (pMobIndex->name, name);
        if (name[0] == '\0' || (race = race_lookup_exact (name)) < 0) {
            if (name[0] != '\0')
                bugf("Unknown race '%s'", name);

            /* fill in with blanks */
            pMobIndex->race = race_lookup_exact ("human");
            pMobIndex->off_flags =
                OFF_DODGE | OFF_DISARM | OFF_TRIP | ASSIST_VNUM;
            pMobIndex->imm_flags = 0;
            pMobIndex->res_flags = 0;
            pMobIndex->vuln_flags = 0;
            pMobIndex->form =
                FORM_EDIBLE | FORM_SENTIENT | FORM_BIPED | FORM_MAMMAL;
            pMobIndex->parts =
                PART_HEAD | PART_ARMS | PART_LEGS | PART_HEART | PART_BRAINS |
                PART_GUTS;
        }
        else {
            pMobIndex->race = race;
            pMobIndex->off_flags =
                OFF_DODGE | OFF_DISARM | OFF_TRIP | ASSIST_RACE |
                race_table[race].off;
            pMobIndex->imm_flags = race_table[race].imm;
            pMobIndex->res_flags = race_table[race].res;
            pMobIndex->vuln_flags = race_table[race].vuln;
            pMobIndex->form = race_table[race].form;
            pMobIndex->parts = race_table[race].parts;
        }

        if (letter != 'S') {
            bug ("load_mobiles: vnum %d non-S.", vnum);
            exit (1);
        }

        convert_mobile (pMobIndex);    /* ROM OLC */

        iHash = vnum % MAX_KEY_HASH;
        LIST_FRONT (pMobIndex, next, mob_index_hash[iHash]);
        top_vnum_mob = top_vnum_mob < vnum ? vnum : top_vnum_mob;    /* OLC */
        assign_area_vnum (vnum);    /* OLC */
        kill_table[URANGE (0, pMobIndex->level, MAX_LEVEL - 1)].number++;
    }
}

/* Snarf an obj section.  old style */
void load_old_obj (FILE * fp) {
    OBJ_INDEX_DATA *pObjIndex;

    if (!area_last) { /* OLC */
        bug ("load_objects: no #AREA seen yet.", 0);
        exit (1);
    }

    while (1) {
        sh_int vnum;
        char letter;
        int iHash;

        letter = fread_letter (fp);
        if (letter != '#') {
            bug ("load_objects: # not found.", 0);
            exit (1);
        }

        vnum = fread_number (fp);
        if (vnum == 0)
            break;

        fBootDb = FALSE;
        if (get_obj_index (vnum) != NULL) {
            bug ("load_objects: vnum %d duplicated.", vnum);
            exit (1);
        }
        fBootDb = TRUE;

        pObjIndex = obj_index_new ();
        pObjIndex->area = area_last; /* OLC */
        pObjIndex->vnum = vnum;
        pObjIndex->anum = vnum - area_last->min_vnum;
        pObjIndex->new_format = FALSE;
        pObjIndex->reset_num = 0;
        str_replace_dup (&pObjIndex->name,        fread_string (fp));
        str_replace_dup (&pObjIndex->short_descr, fread_string (fp));
        str_replace_dup (&pObjIndex->description, fread_string (fp));
        /* Action description */ fread_string (fp);

        pObjIndex->short_descr[0] = LOWER (pObjIndex->short_descr[0]);
        pObjIndex->description[0] = UPPER (pObjIndex->description[0]);

        pObjIndex->item_type = fread_number (fp);
        pObjIndex->extra_flags = fread_flag (fp);
        pObjIndex->wear_flags = fread_flag (fp);
        pObjIndex->value[0] = fread_number (fp);
        pObjIndex->value[1] = fread_number (fp);
        pObjIndex->value[2] = fread_number (fp);
        pObjIndex->value[3] = fread_number (fp);
        pObjIndex->value[4] = 0;
        pObjIndex->level = 0;
        pObjIndex->condition = 100;
        pObjIndex->weight = fread_number (fp);
        pObjIndex->cost = fread_number (fp);
        fread_number (fp); /* Cost per day? Unused? */

        if (pObjIndex->item_type == ITEM_WEAPON) {
            if (is_name ("two", pObjIndex->name)
                || is_name ("two-handed", pObjIndex->name)
                || is_name ("claymore", pObjIndex->name))
            {
                SET_BIT (pObjIndex->value[4], WEAPON_TWO_HANDS);
            }
        }

        while (1) {
            char letter = fread_letter (fp);
            if (letter == 'A') {
                AFFECT_DATA *paf = affect_new ();
                sh_int duration, modifier;

                duration = fread_number (fp);
                modifier = fread_number (fp);
                affect_init (paf, TO_OBJECT, -1, 20, -1, duration, modifier, 0);
                LIST_BACK (paf, next, pObjIndex->affected, AFFECT_DATA);
            }
            else if (letter == 'E') {
                EXTRA_DESCR_DATA *ed = extra_descr_new ();
                ed = alloc_perm (sizeof (*ed));
                str_replace_dup (&ed->keyword, fread_string (fp));
                str_replace_dup (&ed->description, fread_string (fp));
                LIST_BACK (ed, next, pObjIndex->extra_descr, EXTRA_DESCR_DATA);
            }
            else {
                ungetc (letter, fp);
                break;
            }
        }

        /* fix armors */
        if (pObjIndex->item_type == ITEM_ARMOR) {
            pObjIndex->value[1] = pObjIndex->value[0];
            pObjIndex->value[2] = pObjIndex->value[1];
        }

        /* Translate spell "slot numbers" to internal "skill numbers." */
        switch (pObjIndex->item_type) {
            case ITEM_PILL:
            case ITEM_POTION:
            case ITEM_SCROLL:
                pObjIndex->value[1] = slot_lookup (pObjIndex->value[1]);
                pObjIndex->value[2] = slot_lookup (pObjIndex->value[2]);
                pObjIndex->value[3] = slot_lookup (pObjIndex->value[3]);
                pObjIndex->value[4] = slot_lookup (pObjIndex->value[4]);
                break;

            case ITEM_STAFF:
            case ITEM_WAND:
                pObjIndex->value[3] = slot_lookup (pObjIndex->value[3]);
                break;
        }

        /* Check for some bogus items. */
        fix_bogus_obj (pObjIndex);

        iHash = vnum % MAX_KEY_HASH;
        LIST_FRONT (pObjIndex, next, obj_index_hash[iHash]);
        top_vnum_obj = top_vnum_obj < vnum ? vnum : top_vnum_obj;    /* OLC */
        assign_area_vnum (vnum);    /* OLC */
    }
}

/*****************************************************************************
 Name:       convert_objects
 Purpose:    Converts all old format objects to new format
 Called by:  boot_db (db.c).
 Note:       Loops over all resets to find the level of the mob
             loaded before the object to determine the level of
             the object.
             It might be better to update the levels in load_resets().
             This function is not pretty.. Sorry about that :)
 Author:        Hugin
 ****************************************************************************/
static MOB_INDEX_DATA *convert_object_reset_mob = NULL;
static OBJ_INDEX_DATA *convert_object_reset_obj = NULL;

int convert_object_reset (RESET_DATA *pReset) {
    #define pMob (convert_object_reset_mob)
    #define pObj (convert_object_reset_obj)

    switch (pReset->command) {
        case 'M':
            if (!(pMob = get_mob_index (pReset->value[1]))) {
                bug ("convert_objects: 'M': bad vnum %d.", pReset->value[1]);
                return 0;
            }
            break;

        case 'O':
            if (!(pObj = get_obj_index (pReset->value[1]))) {
                bug ("convert_objects: 'O': bad vnum %d.", pReset->value[1]);
                return 0;
            }
            if (pObj->new_format)
                return 1;
            if (!pMob) {
                bug ("convert_objects: 'O': No mob reset yet.", 0);
                return 0;
            }
            pObj->level = pObj->level < 1 ? pMob->level - 2
                : UMIN (pObj->level, pMob->level - 2);
            break;

        case 'P': {
            OBJ_INDEX_DATA *pObj, *pObjTo;
            if (!(pObj = get_obj_index (pReset->value[1]))) {
                bug ("convert_objects: 'P': bad vnum %d.", pReset->value[1]);
                return 0;
            }
            if (pObj->new_format)
                return 1;
            if (!(pObjTo = get_obj_index (pReset->value[3]))) {
                bug ("convert_objects: 'P': bad vnum %d.", pReset->value[3]);
                return 0;
            }
            pObj->level = pObj->level < 1 ? pObjTo->level
                : UMIN (pObj->level, pObjTo->level);
            break;
        }

        case 'G':
        case 'E':
            if (!(pObj = get_obj_index (pReset->value[1]))) {
                bug ("convert_objects: 'E' or 'G': bad vnum %d.",
                    pReset->value[1]);
                return 0;
            }
            if (!pMob) {
                bug ("convert_objects: 'E' or 'G': null mob for vnum %d.",
                    pReset->value[1]);
                return 0;
            }
            if (pObj->new_format)
                return 1;
            if (pMob->pShop) {
                switch (pObj->item_type) {
                    default:
                        pObj->level = UMAX (0, pObj->level);
                        break;
                    case ITEM_PILL:
                    case ITEM_POTION:
                        pObj->level = UMAX (5, pObj->level);
                        break;
                    case ITEM_SCROLL:
                    case ITEM_ARMOR:
                    case ITEM_WEAPON:
                        pObj->level = UMAX (10, pObj->level);
                        break;
                    case ITEM_WAND:
                    case ITEM_TREASURE:
                        pObj->level = UMAX (15, pObj->level);
                        break;
                    case ITEM_STAFF:
                        pObj->level = UMAX (20, pObj->level);
                        break;
                }
            }
            else
                pObj->level = pObj->level < 1 ? pMob->level
                    : UMIN (pObj->level, pMob->level);
            break;

        default:
            bug ("convert_objects: '%c': unknown command.", pReset->command);
            return 0;
    }

    #undef pMob
    #undef pObj
    return 1;
}

void convert_objects (void) {
    int vnum;
    AREA_DATA *pArea;
    ROOM_INDEX_DATA *pRoom;
    RESET_DATA *pReset;
    OBJ_INDEX_DATA *pObj;

    if (newobjs == TOP (RECYCLE_OBJ_INDEX_DATA))
        return; /* all objects in new format */

    convert_object_reset_mob = NULL;
    convert_object_reset_obj = NULL;

    for (pArea = area_first; pArea; pArea = pArea->next) {
        for (vnum = pArea->min_vnum; vnum <= pArea->max_vnum; vnum++) {
            if (!(pRoom = get_room_index (vnum)))
                continue;
            for (pReset = pRoom->reset_first; pReset; pReset = pReset->next)
                convert_object_reset (pReset);
        }
    }

    /* do the conversion: */
    for (pObj = obj_index_get_first(); pObj; pObj = obj_index_get_next (pObj))
        if (!pObj->new_format)
            convert_object (pObj);
}

/*****************************************************************************
 Name:       convert_object
 Purpose:    Converts an old_format obj to new_format
 Called by:  convert_objects (db_old.c).
 Note:       Dug out of create_obj (db.c)
 Author:     Hugin
 ****************************************************************************/
void convert_object (OBJ_INDEX_DATA * pObjIndex) {
    int level;
    int number, type;            /* for dice-conversion */

    if (!pObjIndex || pObjIndex->new_format)
        return;

    level = pObjIndex->level;

    pObjIndex->level = UMAX (0, pObjIndex->level);    /* just to be sure */
    pObjIndex->cost = 10 * level;

    switch (pObjIndex->item_type) {
        default:
            bug ("obj_convert: vnum %d bad type.", pObjIndex->item_type);
            break;

        case ITEM_LIGHT:
        case ITEM_TREASURE:
        case ITEM_FURNITURE:
        case ITEM_TRASH:
        case ITEM_CONTAINER:
        case ITEM_DRINK_CON:
        case ITEM_KEY:
        case ITEM_FOOD:
        case ITEM_BOAT:
        case ITEM_CORPSE_NPC:
        case ITEM_CORPSE_PC:
        case ITEM_FOUNTAIN:
        case ITEM_MAP:
        case ITEM_CLOTHING:
        case ITEM_SCROLL:
            break;

        case ITEM_WAND:
        case ITEM_STAFF:
            pObjIndex->value[2] = pObjIndex->value[1];
            break;

        case ITEM_WEAPON:
            /*
             * The conversion below is based on the values generated
             * in one_hit() (fight.c).  Since I don't want a lvl 50
             * weapon to do 15d3 damage, the min value will be below
             * the one in one_hit, and to make up for it, I've made
             * the max value higher.
             * (I don't want 15d2 because this will hardly ever roll
             * 15 or 30, it will only roll damage close to 23.
             * I can't do 4d8+11, because one_hit there is no dice-
             * bounus value to set...)
             *
             * The conversion below gives:

             level:   dice      min      max      mean
              1:     1d8      1( 2)    8( 7)     5( 5)
              2:     2d5      2( 3)   10( 8)     6( 6)
              3:     2d5      2( 3)   10( 8)     6( 6)
              5:     2d6      2( 3)   12(10)     7( 7)
             10:     4d5      4( 5)   20(14)    12(10)
             20:     5d5      5( 7)   25(21)    15(14)
             30:     5d7      5(10)   35(29)    20(20)
             50:     5d11     5(15)   55(44)    30(30)

             */

            number = UMIN (level / 4 + 1, 5);
            type = (level + 7) / number;

            pObjIndex->value[1] = number;
            pObjIndex->value[2] = type;
            break;

        case ITEM_ARMOR:
            pObjIndex->value[0] = level / 5 + 3;
            pObjIndex->value[1] = pObjIndex->value[0];
            pObjIndex->value[2] = pObjIndex->value[0];
            break;

        case ITEM_POTION:
        case ITEM_PILL:
            break;

        case ITEM_MONEY:
            pObjIndex->value[0] = pObjIndex->cost;
            break;
    }

    pObjIndex->new_format = TRUE;
    ++newobjs;
}

/*****************************************************************************
 Name:       convert_mobile
 Purpose:    Converts an old_format mob into new_format
 Called by:  load_old_mob (db.c).
 Note:       Dug out of create_mobile (db.c)
 Author:     Hugin
 ****************************************************************************/
void convert_mobile (MOB_INDEX_DATA * pMobIndex) {
    int i;
    int type, number, bonus;
    int level;

    if (!pMobIndex || pMobIndex->new_format)
        return;

    level = pMobIndex->level;
    pMobIndex->mob |= MOB_WARRIOR;

    /*
     * Calculate hit dice.  Gives close to the hitpoints
     * of old format mobs created with create_mobile()  (db.c)
     * A high number of dice makes for less variance in mobiles
     * hitpoints.
     * (might be a good idea to reduce the max number of dice)
     *
     * The conversion below gives:

     level:     dice         min         max        diff       mean
      1:       1d2+6       7(  7)     8(   8)     1(   1)     8(   8)
      2:       1d3+15     16( 15)    18(  18)     2(   3)    17(  17)
      3:       1d6+24     25( 24)    30(  30)     5(   6)    27(  27)
      5:      1d17+42     43( 42)    59(  59)    16(  17)    51(  51)
     10:      3d22+96     99( 95)   162( 162)    63(  67)   131(    )
     15:     5d30+161    166(159)   311( 311)   145( 150)   239(    )
     30:    10d61+416    426(419)  1026(1026)   600( 607)   726(    )
     50:    10d169+920   930(923)  2610(2610)  1680(1688)  1770(    )

     The values in parenthesis give the values generated in create_mobile.
     Diff = max - min.  Mean is the arithmetic mean.
     (hmm.. must be some roundoff error in my calculations.. smurfette got
     1d6+23 hp at level 3 ? -- anyway.. the values above should be
     approximately right..)
    */

    type   = level * level * 27 / 40;
    number = UMIN (type / 40 + 1, 10);    /* how do they get 11 ??? */
    type   = UMAX (2, type / number);
    bonus  = UMAX (0, level * (8 + level) * .9 - number * type);

    pMobIndex->hit[DICE_NUMBER] = number;
    pMobIndex->hit[DICE_TYPE]   = type;
    pMobIndex->hit[DICE_BONUS]  = bonus;

    pMobIndex->mana[DICE_NUMBER] = level;
    pMobIndex->mana[DICE_TYPE]   = 10;
    pMobIndex->mana[DICE_BONUS]  = 100;

    /* Calculate dam dice.  Gives close to the damage
     * of old format mobs in damage()  (fight.c) */
    type   = level * 7 / 4;
    number = UMIN (type / 8 + 1, 5);
    type   = UMAX (2, type / number);
    bonus  = UMAX (0, level * 9 / 4 - number * type);

    pMobIndex->damage[DICE_NUMBER] = number;
    pMobIndex->damage[DICE_TYPE]   = type;
    pMobIndex->damage[DICE_BONUS]  = bonus;

    switch (number_range (1, 3)) {
        case (1): pMobIndex->dam_type = 3;  break; /* slash  */
        case (2): pMobIndex->dam_type = 7;  break; /* pound  */
        case (3): pMobIndex->dam_type = 11; break; /* pierce */
    }

    for (i = 0; i < 3; i++)
        pMobIndex->ac[i] = interpolate (level, 100, -100);
    pMobIndex->ac[3] = interpolate (level, 100, 0);    /* exotic */

    pMobIndex->wealth /= 100;
    pMobIndex->size = SIZE_MEDIUM;
    str_replace_dup (&pMobIndex->material_str, "none");
    pMobIndex->material = material_lookup_exact (pMobIndex->material_str);
    if (pMobIndex->material < 0)
        pMobIndex->material = 0;

    pMobIndex->new_format = TRUE;
    ++newmobs;
}
