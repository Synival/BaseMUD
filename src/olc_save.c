/***************************************************************************
 *  File: olc_save.c                                                       *
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
 *                                                                         *
 *  This code was freely distributed with the The Isles 1.1 source code,   *
 *  and has been used here for OLC - OLC would not be what it is without   *
 *  all the previous coders who released their source code.                *
 *                                                                         *
 ***************************************************************************/

/* OLC_SAVE.C
 * This takes care of saving all the .are information.
 * Notes:
 * -If a good syntax checker is used for setting vnum ranges of areas
 *  then it would become possible to just cycle through vnums instead
 *  of using the iHash stuff and checking that the room or reset or
 *  mob etc is part of that area.
 */

#include <string.h>
#include <stdlib.h>

#include "merc.h"
#include "lookup.h"
#include "utils.h"
#include "db.h"
#include "mob_cmds.h"
#include "comm.h"
#include "interp.h"
#include "globals.h"
#include "olc.h"

#include "olc_save.h"

/* Verbose writes reset data in plain english into the comments
 * section of the resets.  It makes areas considerably larger but
 * may aid in debugging. */

/* #define VERBOSE */

/*****************************************************************************
 Name:        fix_string
 Purpose:    Returns a string without \r and ~.
 ****************************************************************************/
char *fix_string (const char *str) {
    static char strfix[MAX_STRING_LENGTH * 2];
    int i;
    int o;

    if (str == NULL)
        return '\0';

    for (o = i = 0; str[i + o] != '\0'; i++) {
        if (str[i + o] == '\r' || str[i + o] == '~')
            o++;
        strfix[i] = str[i + o];
    }
    strfix[i] = '\0';
    return strfix;
}

/*****************************************************************************
 Name:        save_area_list
 Purpose:    Saves the listing of files to be loaded at startup.
 Called by:    do_asave(olc_save.c).
 ****************************************************************************/
void save_area_list () {
    FILE *fp;
    AREA_T *pArea;
    HELP_AREA_T *ha;

    if ((fp = fopen ("area.lst", "w")) == NULL) {
        bug ("save_area_list: fopen", 0);
        perror ("area.lst");
    }
    else {
        /* Add any help files that need to be loaded at
         * startup to this section. */
        fprintf (fp, "social.are\n");    /* ROM OLC */

        for (ha = had_first; ha; ha = ha->next)
            if (ha->area == NULL)
                fprintf (fp, "%s\n", ha->filename);
        for (pArea = area_first; pArea; pArea = pArea->next)
            fprintf (fp, "%s\n", pArea->filename);

        fprintf (fp, "$\n");
        fclose (fp);
    }
}

/*
 * ROM OLC
 * Used in save_mobile and save_object below.  Writes
 * flags on the form fread_flag reads.
 *
 * buf[] must hold at least 32+1 characters.
 *
 * -- Hugin
 */
char *fwrite_flag (long flags, char buf[]) {
    char offset;
    char *cp;

    buf[0] = '\0';
    if (flags == 0) {
        strcpy (buf, "0");
        return buf;
    }

    /* 32 -- number of bits in a long */
    for (offset = 0, cp = buf; offset < 32; offset++) {
        if (flags & ((long) 1 << offset)) {
            if (offset <= 'Z' - 'A')
                *(cp++) = 'A' + offset;
            else
                *(cp++) = 'a' + offset - ('Z' - 'A' + 1);
        }
    }

    *cp = '\0';
    return buf;
}

void save_mobprogs (FILE *fp, AREA_T *pArea) {
    MPROG_CODE_T *pMprog;
    int i;

    fprintf (fp, "#MOBPROGS\n");
    for (i = pArea->min_vnum; i <= pArea->max_vnum; i++) {
        if ((pMprog = get_mprog_index (i)) != NULL) {
            fprintf (fp, "#%d\n", i);
            fprintf (fp, "%s~\n", fix_string (pMprog->code));
        }
    }
    fprintf (fp, "#0\n\n");
}

/*****************************************************************************
 Name:        save_mobile
 Purpose:    Save one mobile to file, new format -- Hugin
 Called by:    save_mobiles (below).
 ****************************************************************************/
void save_mobile (FILE *fp, MOB_INDEX_T *mob_index)
{
    MPROG_LIST_T *pMprog;
    const RACE_T *race;
    char buf[MAX_STRING_LENGTH];

    race = &(race_table[mob_index->race]);
    fprintf (fp, "#%d\n", mob_index->vnum);
    fprintf (fp, "%s~\n", mob_index->name);
    fprintf (fp, "%s~\n", mob_index->short_descr);
    fprintf (fp, "%s~\n", fix_string (mob_index->long_descr));
    fprintf (fp, "%s~\n", fix_string (mob_index->description));
    fprintf (fp, "%s~\n", race->name);
    fprintf (fp, "%s ", fwrite_flag (mob_index->mob_plus, buf));
    fprintf (fp, "%s ", fwrite_flag (mob_index->affected_by_plus, buf));
    fprintf (fp, "%d %d\n", mob_index->alignment, mob_index->group);
    fprintf (fp, "%d ", mob_index->level);
    fprintf (fp, "%d ", mob_index->hitroll);
    fprintf (fp, "%dd%d+%d ", mob_index->hit.number,
             mob_index->hit.size, mob_index->hit.bonus);
    fprintf (fp, "%dd%d+%d ", mob_index->mana.number,
             mob_index->mana.size, mob_index->mana.bonus);
    fprintf (fp, "%dd%d+%d ", mob_index->damage.number,
             mob_index->damage.size, mob_index->damage.bonus);
    fprintf (fp, "%s\n", attack_table[mob_index->dam_type].name);
    fprintf (fp, "%d %d %d %d\n",
             mob_index->ac[AC_PIERCE] / 10,
             mob_index->ac[AC_BASH] / 10,
             mob_index->ac[AC_SLASH] / 10, mob_index->ac[AC_EXOTIC] / 10);
    fprintf (fp, "%s ", fwrite_flag (mob_index->off_flags_plus, buf));
    fprintf (fp, "%s ", fwrite_flag (mob_index->imm_flags_plus, buf));
    fprintf (fp, "%s ", fwrite_flag (mob_index->res_flags_plus, buf));
    fprintf (fp, "%s\n", fwrite_flag (mob_index->vuln_flags_plus, buf));
    fprintf (fp, "%s %s %s %ld\n",
             position_table[mob_index->start_pos].name,
             position_table[mob_index->default_pos].name,
             sex_table[mob_index->sex].name, mob_index->wealth);
    fprintf (fp, "%s ", fwrite_flag (mob_index->form_plus, buf));
    fprintf (fp, "%s ", fwrite_flag (mob_index->parts_plus, buf));

    fprintf (fp, "%s ", size_table[mob_index->size].name);
    fprintf (fp, "%s\n", if_null_str (
        (char *) material_get_name (mob_index->material), "unknown"));

    if (mob_index->mob_minus != 0)
        fprintf (fp, "F act %s\n", fwrite_flag (mob_index->mob_minus, buf));
    if (mob_index->affected_by_minus != 0)
        fprintf (fp, "F aff %s\n", fwrite_flag (mob_index->affected_by_minus, buf));
    if (mob_index->off_flags_minus != 0)
        fprintf (fp, "F off %s\n", fwrite_flag (mob_index->off_flags_minus, buf));
    if (mob_index->imm_flags_minus != 0)
        fprintf (fp, "F imm %s\n", fwrite_flag (mob_index->imm_flags_minus, buf));
    if (mob_index->res_flags_minus != 0)
        fprintf (fp, "F res %s\n", fwrite_flag (mob_index->res_flags_minus, buf));
    if (mob_index->vuln_flags_minus != 0)
        fprintf (fp, "F vul %s\n", fwrite_flag (mob_index->vuln_flags_minus, buf));
    if (mob_index->form_minus != 0)
        fprintf (fp, "F for %s\n", fwrite_flag (mob_index->form_minus, buf));
    if (mob_index->parts_minus != 0)
        fprintf (fp, "F par %s\n", fwrite_flag (mob_index->parts_minus, buf));

    for (pMprog = mob_index->mprogs; pMprog; pMprog = pMprog->next)
        fprintf (fp, "M %s %d %s~\n",
                 mprog_type_to_name (pMprog->trig_type), pMprog->vnum,
                 pMprog->trig_phrase);
}


/*****************************************************************************
 Name:        save_mobiles
 Purpose:    Save #MOBILES secion of an area file.
 Called by:    save_area(olc_save.c).
 Notes:         Changed for ROM OLC.
 ****************************************************************************/
void save_mobiles (FILE *fp, AREA_T *pArea) {
    int i;
    MOB_INDEX_T *pMob;

    fprintf (fp, "#MOBILES\n");
    for (i = pArea->min_vnum; i <= pArea->max_vnum; i++)
        if ((pMob = get_mob_index (i)))
            save_mobile (fp, pMob);
    fprintf (fp, "#0\n\n");
}

/*****************************************************************************
 Name:        save_object
 Purpose:    Save one object to file.
                new ROM format saving -- Hugin
 Called by:    save_objects (below).
 ****************************************************************************/
void save_object (FILE *fp, OBJ_INDEX_T *obj_index) {
    char letter;
    AFFECT_T *pAf;
    EXTRA_DESCR_T *pEd;
    char buf[MAX_STRING_LENGTH];

    fprintf (fp, "#%d\n", obj_index->vnum);
    fprintf (fp, "%s~\n", obj_index->name);
    fprintf (fp, "%s~\n", obj_index->short_descr);
    fprintf (fp, "%s~\n", fix_string (obj_index->description));
    fprintf (fp, "%s~\n", if_null_str (
        (char *) material_get_name (obj_index->material), "unknown"));
    fprintf (fp, "%s ", item_get_name (obj_index->item_type));
    fprintf (fp, "%s ", fwrite_flag (obj_index->extra_flags, buf));
    fprintf (fp, "%s\n", fwrite_flag (obj_index->wear_flags, buf));

/*
 *  Using fwrite_flag to write most values gives a strange
 *  looking area file, consider making a case for each
 *  item type later.
 */

    switch (obj_index->item_type) {
        default:
            fprintf (fp, "%s ",  fwrite_flag (obj_index->v.value[0], buf));
            fprintf (fp, "%s ",  fwrite_flag (obj_index->v.value[1], buf));
            fprintf (fp, "%s ",  fwrite_flag (obj_index->v.value[2], buf));
            fprintf (fp, "%s ",  fwrite_flag (obj_index->v.value[3], buf));
            fprintf (fp, "%s\n", fwrite_flag (obj_index->v.value[4], buf));
            break;

        case ITEM_DRINK_CON:
        case ITEM_FOUNTAIN:
            fprintf (fp, "%ld %ld '%s' %ld %ld\n",
                     obj_index->v.value[0],
                     obj_index->v.value[1],
                     liq_table[obj_index->v.value[2]].name,
                     obj_index->v.value[3],
                     obj_index->v.value[4]);
            break;

        case ITEM_CONTAINER:
            fprintf (fp, "%ld %s %ld %ld %ld\n",
                     obj_index->v.value[0],
                     fwrite_flag (obj_index->v.value[1], buf),
                     obj_index->v.value[2],
                     obj_index->v.value[3],
                     obj_index->v.value[4]);
            break;

        case ITEM_WEAPON:
            fprintf (fp, "%s %ld %ld %s %s\n",
                     weapon_get_name (obj_index->v.value[0]),
                     obj_index->v.value[1],
                     obj_index->v.value[2],
                     attack_table[obj_index->v.value[3]].name,
                     fwrite_flag (obj_index->v.value[4], buf));
            break;

        case ITEM_PILL:
        case ITEM_POTION:
        case ITEM_SCROLL:
            /* no negative numbers */
            fprintf (fp, "%ld '%s' '%s' '%s' '%s'\n",
                     obj_index->v.value[0]  >  0 ? obj_index->v.value[0] : 0,
                     obj_index->v.value[1] != -1 ? skill_table[obj_index->v.value[1]].name : "",
                     obj_index->v.value[2] != -1 ? skill_table[obj_index->v.value[2]].name : "",
                     obj_index->v.value[3] != -1 ? skill_table[obj_index->v.value[3]].name : "",
                     obj_index->v.value[4] != -1 ? skill_table[obj_index->v.value[4]].name : "");
            break;

        case ITEM_STAFF:
        case ITEM_WAND:
            fprintf (fp, "%ld %ld %ld '%s' %ld\n",
                     obj_index->v.value[0],
                     obj_index->v.value[1],
                     obj_index->v.value[2],
                     obj_index->v.value[3] != -1
                        ? skill_table[obj_index->v.value[3]].name : "",
                     obj_index->v.value[4]);
            break;
    }

    fprintf (fp, "%d ", obj_index->level);
    fprintf (fp, "%d ", obj_index->weight);
    fprintf (fp, "%d ", obj_index->cost);

         if (obj_index->condition > 90) letter = 'P';
    else if (obj_index->condition > 75) letter = 'G';
    else if (obj_index->condition > 50) letter = 'A';
    else if (obj_index->condition > 25) letter = 'W';
    else if (obj_index->condition > 10) letter = 'D';
    else if (obj_index->condition >  0) letter = 'B';
    else                                letter = 'R';

    fprintf (fp, "%c\n", letter);

    for (pAf = obj_index->affected; pAf; pAf = pAf->next) {
        if (pAf->bit_type == AFF_TO_OBJECT || pAf->bits == 0)
            fprintf (fp, "A\n%d %d\n", pAf->apply, pAf->modifier);
        else {
            fprintf (fp, "F\n");

            switch (pAf->bit_type) {
                case AFF_TO_AFFECTS: fprintf (fp, "A "); break;
                case AFF_TO_IMMUNE:  fprintf (fp, "I "); break;
                case AFF_TO_RESIST:  fprintf (fp, "R "); break;
                case AFF_TO_VULN:    fprintf (fp, "V "); break;
                default:
                    bug ("olc_save: Invalid Affect->where", 0);
                    break;
            }

            fprintf (fp, "%d %d %s\n", pAf->apply, pAf->modifier,
                     fwrite_flag (pAf->bits, buf));
        }
    }

    for (pEd = obj_index->extra_descr; pEd; pEd = pEd->next)
        fprintf (fp, "E\n%s~\n%s~\n", pEd->keyword,
                 fix_string (pEd->description));
}

/*****************************************************************************
 Name:       save_objects
 Purpose:    Save #OBJECTS section of an area file.
 Called by:  save_area(olc_save.c).
 Notes:      Changed for ROM OLC.
 ****************************************************************************/
void save_objects (FILE *fp, AREA_T *pArea) {
    int i;
    OBJ_INDEX_T *pObj;

    fprintf (fp, "#OBJECTS\n");
    for (i = pArea->min_vnum; i <= pArea->max_vnum; i++)
        if ((pObj = get_obj_index (i)))
            save_object (fp, pObj);
    fprintf (fp, "#0\n\n");
}

void save_room (FILE *fp, ROOM_INDEX_T *room_index) {
    EXTRA_DESCR_T *pEd;
    EXIT_T *pExit;
    char buf[MAX_STRING_LENGTH];
    int door;

    fprintf (fp, "#%d\n", room_index->vnum);
    fprintf (fp, "%s~\n", room_index->name);
    fprintf (fp, "%s~\n", fix_string (room_index->description));
    fprintf (fp, "0 ");
    fprintf (fp, "%s ", fwrite_flag (room_index->room_flags, buf));
    fprintf (fp, "%d\n", room_index->sector_type);

    for (door = 0; door < DIR_MAX; door++) {
        int locks = 0;
        if (!(pExit = room_index->exit[door]))
            continue;

#if 0
        /* HACK : TO PREVENT EX_LOCKED etc without EX_ISDOOR
           to stop booting the mud */
        if (IS_SET (pExit->rs_flags, EX_CLOSED)
            || IS_SET (pExit->rs_flags, EX_LOCKED)
            || IS_SET (pExit->rs_flags, EX_PICKPROOF)
            || IS_SET (pExit->rs_flags, EX_NOPASS)
            || IS_SET (pExit->rs_flags, EX_EASY)
            || IS_SET (pExit->rs_flags, EX_HARD)
            || IS_SET (pExit->rs_flags, EX_INFURIATING)
            || IS_SET (pExit->rs_flags, EX_NOCLOSE)
            || IS_SET (pExit->rs_flags, EX_NOLOCK))
            SET_BIT (pExit->rs_flags, EX_ISDOOR);
        else
            REMOVE_BIT (pExit->rs_flags, EX_ISDOOR);
#endif

        /* THIS SUCKS but it's backwards compatible */
        /* NOTE THAT EX_NOCLOSE NOLOCK etc aren't being saved */
        if (IS_SET (pExit->rs_flags, EX_ISDOOR)
            && (!IS_SET (pExit->rs_flags, EX_PICKPROOF))
            && (!IS_SET (pExit->rs_flags, EX_NOPASS)))
            locks = 1;
        if (IS_SET (pExit->rs_flags, EX_ISDOOR)
            && (IS_SET (pExit->rs_flags, EX_PICKPROOF))
            && (!IS_SET (pExit->rs_flags, EX_NOPASS)))
            locks = 2;
        if (IS_SET (pExit->rs_flags, EX_ISDOOR)
            && (!IS_SET (pExit->rs_flags, EX_PICKPROOF))
            && (IS_SET (pExit->rs_flags, EX_NOPASS)))
            locks = 3;
        if (IS_SET (pExit->rs_flags, EX_ISDOOR)
            && (IS_SET (pExit->rs_flags, EX_PICKPROOF))
            && (IS_SET (pExit->rs_flags, EX_NOPASS)))
            locks = 4;

        fprintf (fp, "D%d\n", pExit->orig_door);
        fprintf (fp, "%s~\n", fix_string (pExit->description));
        fprintf (fp, "%s~\n", pExit->keyword);
        fprintf (fp, "%d %d %d\n", locks, pExit->key,
            (pExit->to_room ? pExit->to_room->vnum : -1));
    }

    for (pEd = room_index->extra_descr; pEd; pEd = pEd->next)
        fprintf (fp, "E\n%s~\n%s~\n", pEd->keyword,
                 fix_string (pEd->description));

    if (!IS_NULLSTR (room_index->owner))
        fprintf (fp, "O %s~\n", room_index->owner);

    if (room_index->mana_rate != 100 || room_index->heal_rate != 100)
        fprintf (fp, "M %d H %d\n", room_index->mana_rate, room_index->heal_rate);
    if (room_index->clan > 0)
        fprintf (fp, "C %s~\n", clan_table[room_index->clan].name);

    fprintf (fp, "S\n");
}

/*****************************************************************************
 Name:       save_rooms
 Purpose:    Save #ROOMS section of an area file.
 Called by:  save_area(olc_save.c).
 ****************************************************************************/
void save_rooms (FILE *fp, AREA_T *pArea) {
    ROOM_INDEX_T *room_index;
    int i;

    fprintf (fp, "#ROOMS\n");
    for (i = pArea->min_vnum; i <= pArea->max_vnum; i++)
        if ((room_index = get_room_index (i)))
            save_room (fp, room_index);
    fprintf (fp, "#0\n\n");
}

/*****************************************************************************
 Name:       save_specials
 Purpose:    Save #SPECIALS section of area file.
 Called by:  save_area(olc_save.c).
 ****************************************************************************/
void save_specials (FILE *fp, AREA_T *pArea) {
    int iHash;
    MOB_INDEX_T *mob_index;

    fprintf (fp, "#SPECIALS\n");
    for (iHash = 0; iHash < MAX_KEY_HASH; iHash++) {
        for (mob_index = mob_index_hash[iHash]; mob_index;
             mob_index = mob_index->next)
        {
            if (mob_index == NULL)
                continue;
            if (mob_index->area != pArea)
                continue;
            if (!mob_index->spec_fun)
                continue;

#if defined(VERBOSE)
            fprintf (fp, "M %5d %-20s * load to: %s\n", mob_index->vnum,
                spec_function_name (mob_index->spec_fun),
                mob_index->short_descr);
#else
            fprintf (fp, "M %5d %-20s\n", mob_index->vnum,
                spec_function_name (mob_index->spec_fun));
#endif
        }
    }
    fprintf (fp, "S\n\n");
}

/* This function is obsolete.  It it not needed but has been left here
 * for historical reasons.  It is used currently for the same reason.
 *
 * I don't think it's obsolete in ROM -- Hugin. */
void save_door_resets (FILE *fp, AREA_T *pArea) {
    int iHash;
    ROOM_INDEX_T *room;
    EXIT_T *pExit;
    int door;

    for (iHash = 0; iHash < MAX_KEY_HASH; iHash++) {
        for (room = room_index_hash[iHash]; room; room = room->next) {
            if (room->area != pArea)
                continue;
            for (door = 0; door < DIR_MAX; door++) {
                if ((pExit = room->exit[door]) == NULL)
                    continue;
                if (pExit->to_room == NULL)
                    continue;
                if (!(IS_SET (pExit->rs_flags, EX_CLOSED) ||
                      IS_SET (pExit->rs_flags, EX_LOCKED)))
                    continue;

#if defined(VERBOSE)
                fprintf (fp, "D 0 %5d %3d %5d    * The %s door of %s is %s\n",
                    room->vnum, pExit->orig_door,
                    IS_SET (pExit->rs_flags, EX_LOCKED) ? 2 : 1,
                    door_get_name(pExit->orig_door), room->name,
                    IS_SET (pExit->rs_flags, EX_LOCKED)
                        ? "closed and locked"
                        : "closed");
#else
                fprintf (fp, "D 0 %5d %3d %5d\n",
                    room->vnum, pExit->orig_door,
                    IS_SET (pExit->rs_flags, EX_LOCKED) ? 2 : 1);
#endif
            }
        }
    }
}


/*****************************************************************************
 Name:        save_resets
 Purpose:    Saves the #RESETS section of an area file.
 Called by:    save_area(olc_save.c)
 ****************************************************************************/
void save_resets (FILE *fp, AREA_T *pArea) {
    RESET_T *r;
    MOB_INDEX_T *pLastMob = NULL;
    OBJ_INDEX_T *pLastObj;
    ROOM_INDEX_T *pRoom;
    int iHash;

    fprintf (fp, "#RESETS\n");
    save_door_resets (fp, pArea);
    for (iHash = 0; iHash < MAX_KEY_HASH; iHash++) {
        for (pRoom = room_index_hash[iHash]; pRoom; pRoom = pRoom->next) {
            if (pRoom->area != pArea)
                continue;

            for (r = pRoom->reset_first; r; r = r->next) {
                switch (r->command) {
                    case 'M':
                        pLastMob = get_mob_index (r->v.value[1]);
                        fprintf (fp, "M %d %5d %3d %5d %2d * load %s\n",
                            r->v.value[0], r->v.value[1], r->v.value[2],
                            r->v.value[3], r->v.value[4], pLastMob->short_descr);
                        break;

                    case 'O':
                        pLastObj = get_obj_index (r->v.value[1]);
                        pRoom = get_room_index (r->v.value[3]);
                        fprintf (fp,
                            "O %d %5d %3d %5d    * %s loaded to %s\n",
                            r->v.value[0], r->v.value[1], r->v.value[2],
                            r->v.value[3], capitalize (pLastObj->short_descr),
                            pRoom->name);
                        break;

                    case 'P':
                        pLastObj = get_obj_index (r->v.value[1]);
                        fprintf (fp,
                            "P %d %5d %3d %5d %2d %s * put inside %s\n",
                            r->v.value[0], r->v.value[1], r->v.value[2],
                            r->v.value[3], r->v.value[4],
                            capitalize (get_obj_index (r->v.value[1])->short_descr),
                            pLastObj->short_descr);
                        break;

                    case 'G':
                        fprintf (fp,
                            "G %d %5d %3d          * %s is given to %s\n",
                            r->v.value[0], r->v.value[1], r->v.value[2],
                            capitalize (get_obj_index (r->v.value[1])->short_descr),
                            pLastMob ? pLastMob->short_descr : "!NO_MOB!");

                        if (!pLastMob)
                            bugf ("Save_resets: !NO_MOB! in [%s]", pArea->filename);
                        break;

                    case 'E':
                        fprintf (fp,
                            "E %d %5d %3d %5d    * %s is loaded %s of %s\n",
                            r->v.value[0], r->v.value[1], r->v.value[2],
                            r->v.value[3],
                            capitalize (get_obj_index (r->v.value[1])->short_descr),
                            flag_string (wear_loc_phrases, r->v.value[3]),
                            pLastMob ? pLastMob->short_descr : "!NO_MOB!");

                        if (!pLastMob)
                            bugf ("Save_resets: !NO_MOB! in [%s]", pArea->filename);
                        break;

                    case 'D':
                        break;

                    case 'R':
                        pRoom = get_room_index (r->v.value[1]);
                        fprintf (fp, "R %d %5d %3d          * randomize %s\n",
                            r->v.value[0], r->v.value[1], r->v.value[2],
                            pRoom->name);
                        break;

                    default:
                        bug ("save_resets: bad command %c.", r->command);
                        break;
                }
            }
        }
    }

    fprintf (fp, "S\n\n");
}

/*****************************************************************************
 Name:       save_shops
 Purpose:    Saves the #SHOPS section of an area file.
 Called by:  save_area(olc_save.c)
 ****************************************************************************/
void save_shops (FILE *fp, AREA_T *pArea) {
    SHOP_T *shop_index;
    MOB_INDEX_T *mob_index;
    int iTrade;
    int iHash;

    fprintf (fp, "#SHOPS\n");
    for (iHash = 0; iHash < MAX_KEY_HASH; iHash++) {
        for (mob_index = mob_index_hash[iHash]; mob_index;
             mob_index = mob_index->next)
        {
            if (mob_index && mob_index->area == pArea && mob_index->shop) {
                shop_index = mob_index->shop;

                fprintf (fp, "%5d ", shop_index->keeper);
                for (iTrade = 0; iTrade < MAX_TRADE; iTrade++) {
                    if (shop_index->buy_type[iTrade] != 0)
                        fprintf (fp, "%2d ", shop_index->buy_type[iTrade]);
                    else
                        fprintf (fp, " 0 ");
                }
                fprintf (fp, "%3d %3d ", shop_index->profit_buy,
                         shop_index->profit_sell);
                fprintf (fp, "%2d %2d\n", shop_index->open_hour,
                         shop_index->close_hour);
            }
        }
    }
    fprintf (fp, "0\n\n");
}

void save_helps (FILE *fp, HELP_AREA_T *ha) {
    HELP_T *help = ha->first;

    fprintf (fp, "#HELPS\n");
    for (; help; help = help->next_area) {
        fprintf (fp, "%d %s~\n", help->level, help->keyword);
        fprintf (fp, "%s~\n\n", fix_string (help->text));
    }
    fprintf (fp, "-1 $~\n\n");
    ha->changed = FALSE;
}

int save_other_helps (CHAR_T *ch) {
    HELP_AREA_T *ha;
    FILE *fp;
    int saved = 0;

    for (ha = had_first; ha; ha = ha->next) {
        if (ha->changed != TRUE)
            break;
        if (!(fp = fopen (ha->filename, "w"))) {
            perror (ha->filename);
            return saved;
        }

        save_helps (fp, ha);
        fprintf (fp, "#$\n");
        fclose (fp);

        saved++;
        if (ch)
            printf_to_char (ch, "%s\n\r", ha->filename);
    }

    return saved;
}

/*****************************************************************************
 Name:        save_area
 Purpose:    Save an area, note that this format is new.
 Called by:    do_asave(olc_save.c).
 ****************************************************************************/
void save_area (AREA_T *pArea) {
    FILE *fp;

    fclose (reserve_file);
    if (!(fp = fopen (pArea->filename, "w"))) {
        bug ("save_area: fopen", 0);
        perror (pArea->filename);
    }

    fprintf (fp, "#AREADATA\n");
    fprintf (fp, "Name %s~\n", pArea->title);
    fprintf (fp, "Builders %s~\n", fix_string (pArea->builders));
    fprintf (fp, "VNUMs %d %d\n", pArea->min_vnum, pArea->max_vnum);
    fprintf (fp, "Credits %s~\n", pArea->credits);
    fprintf (fp, "Security %d\n", pArea->security);
    fprintf (fp, "End\n\n");

    save_mobiles (fp, pArea);
    save_objects (fp, pArea);
    save_rooms (fp, pArea);
    save_resets (fp, pArea);
    save_shops (fp, pArea);
    save_specials (fp, pArea);
    save_mobprogs (fp, pArea);

    if (pArea->helps && pArea->helps->first)
        save_helps (fp, pArea->helps);
    fprintf (fp, "#$\n");

    fclose (fp);
    reserve_file = fopen (NULL_FILE, "r");
}
