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
#include "lookup.h"
#include "json.h"
#include "utils.h"
#include "olc.h"
#include "recycle.h"
#include "utils.h"
#include "affects.h"

#include "db2.h"

/* TODO: load_socials() is filthy :( */
/* TODO: this can be integrated into db.c, everything old should be
 *       moved to db_old.c */

bool load_socials_str (FILE *fp, char **str) {
    char *temp = fread_string_eol (fp);
    if (!strcmp (temp, "$"))
        *str = NULL;
    else if (!strcmp (temp, "#"))
        return FALSE;
    else
        *str = temp;
    return TRUE;
}

/* snarf a socials file */
void load_socials (FILE * fp) {
    while (1) {
        SOCIAL_TYPE *new;
        char *temp;

        temp = fread_word (fp);
        if (!strcmp (temp, "#0"))
            return; /* done */
#if defined(social_debug)
        else
            fprintf (stderr, "%s\n\r", temp);
#endif

        /* initialize our new social. */
        new = social_new ();
        strncpy (new->name, temp, sizeof (new->name) - 1);
        new->name[sizeof (new->name) - 1] = '\0';
        fread_to_eol (fp);

        if (!load_socials_str (fp, &(new->char_no_arg)))
            continue;
        if (!load_socials_str (fp, &(new->others_no_arg)))
            continue;
        if (!load_socials_str (fp, &(new->char_found)))
            continue;
        if (!load_socials_str (fp, &(new->others_found)))
            continue;
        if (!load_socials_str (fp, &(new->vict_found)))
            continue;
        if (!load_socials_str (fp, &(new->char_not_found)))
            continue;
        if (!load_socials_str (fp, &(new->char_auto)))
            continue;
        if (!load_socials_str (fp, &(new->others_auto)))
            continue;
    }
}

/* Snarf a mob section.  new style */
void load_mobiles (FILE * fp) {
    MOB_INDEX_DATA *pMobIndex;
    char *name;

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
        pMobIndex->area = area_last; /* OLC */
        pMobIndex->vnum = vnum;
        pMobIndex->anum = vnum - area_last->min_vnum;
        pMobIndex->new_format = TRUE;
        newmobs++;

        /* For some reason, 'oldstyle' is in front of a lot of the names.
         * If this is the case, we're going to ignore it completely. */
        name = fread_string (fp);
        if (!str_prefix ("oldstyle", name)) {
            name += 8;
            while (*name == ' ')
                name++;
        }
        str_replace_dup (&pMobIndex->name, name);

        str_replace_dup (&pMobIndex->short_descr, fread_string (fp));
        str_replace_dup (&pMobIndex->long_descr,  fread_string (fp));
        str_replace_dup (&pMobIndex->description, fread_string (fp));
        str_replace_dup (&pMobIndex->race_str,    fread_string (fp));
        pMobIndex->race = lookup_backup (race_lookup_exact, pMobIndex->race_str,
            "Unknown race '%s'", 0);

        pMobIndex->long_descr[0] = UPPER (pMobIndex->long_descr[0]);
        pMobIndex->description[0] = UPPER (pMobIndex->description[0]);

        pMobIndex->mob_orig = fread_flag (fp);
        pMobIndex->affected_by_orig = fread_flag (fp);

        pMobIndex->mob = pMobIndex->mob_orig | MOB_IS_NPC
            | race_table[pMobIndex->race].mob;
        pMobIndex->affected_by = pMobIndex->affected_by_orig
            | race_table[pMobIndex->race].aff;
        pMobIndex->pShop = NULL;
        pMobIndex->alignment = fread_number (fp);
        pMobIndex->group = fread_number (fp);

        pMobIndex->level = fread_number (fp);
        pMobIndex->hitroll = fread_number (fp);

        /* read hit, mana, dam dice */
        fread_dice (fp, pMobIndex->hit);
        fread_dice (fp, pMobIndex->mana);
        fread_dice (fp, pMobIndex->damage);

        str_replace_dup (&pMobIndex->dam_type_str, fread_word (fp));
        pMobIndex->dam_type = lookup_backup (attack_lookup_exact,
            pMobIndex->dam_type_str, "Unknown damage type '%s'", 0);

        /* read armor class */
        pMobIndex->ac[AC_PIERCE] = fread_number (fp) * 10;
        pMobIndex->ac[AC_BASH]   = fread_number (fp) * 10;
        pMobIndex->ac[AC_SLASH]  = fread_number (fp) * 10;
        pMobIndex->ac[AC_EXOTIC] = fread_number (fp) * 10;

        /* read flags and add in data from the race table */
        pMobIndex->off_flags_orig  = fread_flag (fp);
        pMobIndex->imm_flags_orig  = fread_flag (fp);
        pMobIndex->res_flags_orig  = fread_flag (fp);
        pMobIndex->vuln_flags_orig = fread_flag (fp);

        pMobIndex->off_flags  = pMobIndex->off_flags_orig  | race_table[pMobIndex->race].off;
        pMobIndex->imm_flags  = pMobIndex->imm_flags_orig  | race_table[pMobIndex->race].imm;
        pMobIndex->res_flags  = pMobIndex->res_flags_orig  | race_table[pMobIndex->race].res;
        pMobIndex->vuln_flags = pMobIndex->vuln_flags_orig | race_table[pMobIndex->race].vuln;

        /* vital statistics */
        str_replace_dup (&pMobIndex->start_pos_str,   fread_word (fp));
        str_replace_dup (&pMobIndex->default_pos_str, fread_word (fp));
        str_replace_dup (&pMobIndex->sex_str,         fread_word (fp));

        pMobIndex->start_pos = lookup_backup (position_lookup_exact,
            pMobIndex->start_pos_str, "Unknown start position '%s'",
            POS_STANDING);
        pMobIndex->default_pos = lookup_backup (position_lookup_exact,
            pMobIndex->default_pos_str, "Unknown default position '%s'",
            POS_STANDING);
        pMobIndex->sex = lookup_backup (sex_lookup_exact,
            pMobIndex->sex_str, "Unknown sex '%s'", SEX_EITHER);

        pMobIndex->wealth     = fread_number (fp);
        pMobIndex->form_orig  = fread_flag (fp);
        pMobIndex->parts_orig = fread_flag (fp);
        pMobIndex->form  = pMobIndex->form_orig  | race_table[pMobIndex->race].form;
        pMobIndex->parts = pMobIndex->parts_orig | race_table[pMobIndex->race].parts;

        /* Size. */
        str_replace_dup (&pMobIndex->size_str, fread_word (fp));
        pMobIndex->size = lookup_backup (size_lookup_exact, pMobIndex->size_str,
            "Unknown size '%s'", SIZE_MEDIUM);

        /* Material. Sometimes this is '0', in which case, just replace it
         * with the default material's keyword. */
        str_replace_dup (&pMobIndex->material_str, fread_word (fp));
        if (!str_cmp (pMobIndex->material_str, "0") || pMobIndex->material_str[0] == '\0')
            str_replace_dup (&pMobIndex->material_str, material_get_name(0));
        pMobIndex->material = lookup_backup (material_lookup_exact,
            pMobIndex->material_str, "Unknown material '%s'", 0);

        while (1) {
            letter = fread_letter (fp);
            if (letter == 'F') {
                char *word;
                long vector;

                word = fread_word (fp);
                vector = fread_flag (fp);

                if (!str_prefix (word, "act") || !str_prefix (word, "mob"))
                    REMOVE_BIT (pMobIndex->mob, vector);
                else if (!str_prefix (word, "aff"))
                    REMOVE_BIT (pMobIndex->affected_by, vector);
                else if (!str_prefix (word, "off"))
                    REMOVE_BIT (pMobIndex->off_flags, vector);
                else if (!str_prefix (word, "imm"))
                    REMOVE_BIT (pMobIndex->imm_flags, vector);
                else if (!str_prefix (word, "res"))
                    REMOVE_BIT (pMobIndex->res_flags, vector);
                else if (!str_prefix (word, "vul"))
                    REMOVE_BIT (pMobIndex->vuln_flags, vector);
                else if (!str_prefix (word, "for"))
                    REMOVE_BIT (pMobIndex->form, vector);
                else if (!str_prefix (word, "par"))
                    REMOVE_BIT (pMobIndex->parts, vector);
                else {
                    bug ("flag remove: flag not found.", 0);
                    exit (1);
                }
            }
            else if (letter == 'M') {
                MPROG_LIST *pMprog;
                char *word;
                int trigger = 0;

                pMprog = mprog_new ();
                word = fread_word (fp);
                if ((trigger = flag_lookup_exact (word, mprog_flags)) <= 0) {
                    bug ("mOBprogs: invalid trigger.", 0);
                    exit (1);
                }
                SET_BIT (pMobIndex->mprog_flags, trigger);
                pMprog->trig_type = trigger;
                pMprog->area = area_last;
                pMprog->vnum = fread_number (fp);
                pMprog->anum = pMprog->vnum - area_last->min_vnum;
                str_replace_dup (&pMprog->trig_phrase, fread_string (fp));

                LIST_BACK (pMprog, next, pMobIndex->mprogs, MPROG_LIST);
            }
            else {
                ungetc (letter, fp);
                break;
            }
        }

        iHash = vnum % MAX_KEY_HASH;
        LIST_FRONT (pMobIndex, next, mob_index_hash[iHash]);

        top_vnum_mob = top_vnum_mob < vnum ? vnum : top_vnum_mob;    /* OLC */
        assign_area_vnum (vnum);    /* OLC */
        kill_table[URANGE (0, pMobIndex->level, MAX_LEVEL - 1)].number++;
    }
}

/* Snarf an obj section. new style */
void load_objects (FILE * fp) {
    OBJ_INDEX_DATA *pObjIndex;
    sh_int location, modifier;
    flag_t bits;

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
        pObjIndex->new_format = TRUE;
        pObjIndex->reset_num = 0;
        newobjs++;
        str_replace_dup (&pObjIndex->name,          fread_string (fp));
        str_replace_dup (&pObjIndex->short_descr,   fread_string (fp));
        str_replace_dup (&pObjIndex->description,   fread_string (fp));
        str_replace_dup (&pObjIndex->material_str,  fread_string (fp));
        str_replace_dup (&pObjIndex->item_type_str, fread_word (fp));

        if (!str_cmp (pObjIndex->material_str, "oldstyle") || /* strange! */
             pObjIndex->material_str[0] == '\0')
            str_replace_dup (&pObjIndex->material_str, material_get_name(0));
        pObjIndex->material = lookup_backup (material_lookup_exact,
            pObjIndex->material_str, "Unknown material '%s'", 0);
        pObjIndex->item_type = lookup_backup (item_lookup_exact,
            pObjIndex->item_type_str, "Unknown item type '%s'", 0);

        pObjIndex->extra_flags = fread_flag (fp);
        pObjIndex->wear_flags = fread_flag (fp);

        switch (pObjIndex->item_type) {
            case ITEM_WEAPON:
                pObjIndex->value[0] = weapon_lookup_exact (fread_word (fp));
                pObjIndex->value[1] = fread_number (fp);
                pObjIndex->value[2] = fread_number (fp);
                pObjIndex->value[3] = attack_lookup_exact (fread_word (fp));
                pObjIndex->value[4] = fread_flag (fp);
                break;
            case ITEM_CONTAINER:
                pObjIndex->value[0] = fread_number (fp);
                pObjIndex->value[1] = fread_flag (fp);
                pObjIndex->value[2] = fread_number (fp);
                pObjIndex->value[3] = fread_number (fp);
                pObjIndex->value[4] = fread_number (fp);
                break;
            case ITEM_DRINK_CON:
            case ITEM_FOUNTAIN:
                pObjIndex->value[0] = fread_number (fp);
                pObjIndex->value[1] = fread_number (fp);
                pObjIndex->value[2] = lookup_backup (liq_lookup_exact,
                    fread_word (fp), "Unknown liquid type '%s'", 0);
                pObjIndex->value[3] = fread_number (fp);
                pObjIndex->value[4] = fread_number (fp);
                break;
            case ITEM_WAND:
            case ITEM_STAFF:
                pObjIndex->value[0] = fread_number (fp);
                pObjIndex->value[1] = fread_number (fp);
                pObjIndex->value[2] = fread_number (fp);
                pObjIndex->value[3] = skill_lookup_exact (fread_word (fp));
                pObjIndex->value[4] = fread_number (fp);
                break;
            case ITEM_POTION:
            case ITEM_PILL:
            case ITEM_SCROLL:
                pObjIndex->value[0] = fread_number (fp);
                pObjIndex->value[1] = skill_lookup_exact (fread_word (fp));
                pObjIndex->value[2] = skill_lookup_exact (fread_word (fp));
                pObjIndex->value[3] = skill_lookup_exact (fread_word (fp));
                pObjIndex->value[4] = skill_lookup_exact (fread_word (fp));
                break;
            default:
                pObjIndex->value[0] = fread_flag (fp);
                pObjIndex->value[1] = fread_flag (fp);
                pObjIndex->value[2] = fread_flag (fp);
                pObjIndex->value[3] = fread_flag (fp);
                pObjIndex->value[4] = fread_flag (fp);
                break;
        }
        pObjIndex->level  = fread_number (fp);
        pObjIndex->weight = fread_number (fp);
        pObjIndex->cost   = fread_number (fp);

        /* condition */
        letter = fread_letter (fp);
        switch (letter) {
            case ('P'): pObjIndex->condition = 100; break;
            case ('G'): pObjIndex->condition = 90;  break;
            case ('A'): pObjIndex->condition = 75;  break;
            case ('W'): pObjIndex->condition = 50;  break;
            case ('D'): pObjIndex->condition = 25;  break;
            case ('B'): pObjIndex->condition = 10;  break;
            case ('R'): pObjIndex->condition = 0;   break;
            default:    pObjIndex->condition = 100; break;
        }

        while (1) {
            char letter = fread_letter (fp);
            if (letter == 'A') {
                AFFECT_DATA *paf = affect_new ();

                location = fread_number (fp);
                modifier = fread_number (fp);
                affect_init (paf, TO_OBJECT, -1, pObjIndex->level, -1, location, modifier, 0);

                LIST_BACK (paf, next, pObjIndex->affected, AFFECT_DATA);
            }
            else if (letter == 'F') {
                AFFECT_DATA *paf = affect_new ();

                letter = fread_letter (fp);
                switch (letter) {
                    case 'A': paf->bit_type = TO_AFFECTS; break;
                    case 'I': paf->bit_type = TO_IMMUNE;  break;
                    case 'R': paf->bit_type = TO_RESIST;  break;
                    case 'V': paf->bit_type = TO_VULN;    break;
                    default:
                        bug ("load_objects: Bad 'bit_type' on flag set.", 0);
                        exit (1);
                }

                location  = fread_number (fp);
                modifier  = fread_number (fp);
                bits = fread_flag (fp);
                affect_init (paf, paf->bit_type, -1, pObjIndex->level, -1,
                    location, modifier, bits);

                LIST_BACK (paf, next, pObjIndex->affected, AFFECT_DATA);
            }
            else if (letter == 'E') {
                EXTRA_DESCR_DATA *ed = extra_descr_new ();
                str_replace_dup (&ed->keyword,     fread_string (fp));
                str_replace_dup (&ed->description, fread_string (fp));
                LIST_BACK (ed, next, pObjIndex->extra_descr, EXTRA_DESCR_DATA);
            }
            else {
                ungetc (letter, fp);
                break;
            }
        }

        /* Check for some bogus items. */
        fix_bogus_obj (pObjIndex);

        iHash = vnum % MAX_KEY_HASH;
        LIST_FRONT (pObjIndex, next, obj_index_hash[iHash]);

        top_vnum_obj = top_vnum_obj < vnum ? vnum : top_vnum_obj; /* OLC */
        assign_area_vnum (vnum); /* OLC */
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
 Called by:  convert_objects (db2.c).
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
 Name:        convert_mobile
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
