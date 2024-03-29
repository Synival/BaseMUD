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
 *  of using the hash stuff and checking that the room or reset or
 *  mob etc is part of that area.
 */

#include "olc_save.h"

#include "comm.h"
#include "fwrite.h"
#include "globals.h"
#include "items.h"
#include "json_export.h"
#include "lookup.h"
#include "mob_cmds.h"
#include "mob_prog.h"
#include "mobiles.h"
#include "objs.h"
#include "rooms.h"
#include "tables.h"
#include "utils.h"

/* Verbose writes reset data in plain english into the comments
 * section of the resets.  It makes areas considerably larger but
 * may aid in debugging. */

#define VERBOSE

char *olc_save_filename_static (const char *filename) {
    static char buf[MAX_STRING_LENGTH];
    snprintf (buf, sizeof (buf), "%s%s", AREA_DIR, filename);
    return buf;
}

/*****************************************************************************
 Name:     fix_string
 Purpose:  Returns a string without \r and ~.
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
 Name:       save_area_list
 Purpose:    Saves the listing of files to be loaded at startup.
 Called by:  do_asave(olc_save.c).
 ****************************************************************************/
void save_area_list () {
    FILE *fp;
    AREA_T *area;
    HELP_AREA_T *ha;

    if ((fp = fopen (olc_save_filename_static ("area.lst"), "w")) == NULL) {
        bug ("save_area_list: fopen", 0);
        perror ("area.lst");
    }
    else {
        /* Add any help files that need to be loaded at
         * startup to this section. */
        fprintf (fp, "social.are\n");    /* ROM OLC */

        for (ha = had_first; ha; ha = ha->global_next)
            if (ha->area == NULL)
                fprintf (fp, "%s\n", ha->filename);
        for (area = area_first; area; area = area->global_next)
            fprintf (fp, "%s\n", area->filename);

        fprintf (fp, "$\n");
        fclose (fp);
    }
}

void save_mobprogs (FILE *fp, AREA_T *area) {
    MPROG_CODE_T *mprog;
    int i;

    fprintf (fp, "#MOBPROGS\n");
    for (i = area->min_vnum; i <= area->max_vnum; i++) {
        if ((mprog = mpcode_get_index (i)) != NULL) {
            fprintf (fp, "#%d\n", i);
            fprintf (fp, "%s~\n", fix_string (mprog->code));
        }
    }
    fprintf (fp, "#0\n\n");
}

/*****************************************************************************
 Name:       save_mobile
 Purpose:    Save one mobile to file, new format -- Hugin
 Called by:  save_mobiles (below).
 ****************************************************************************/
void save_mobile (FILE *fp, MOB_INDEX_T *mob_index) {
    MPROG_LIST_T *mprog;
    const RACE_T *race;
    char buf[MAX_STRING_LENGTH];

    race = race_get (mob_index->race);
    fprintf (fp, "#%d\n", mob_index->vnum);
    fprintf (fp, "%s~\n", mob_index->name);
    fprintf (fp, "%s~\n", mob_index->short_descr);
    fprintf (fp, "%s~\n", fix_string (mob_index->long_descr));
    fprintf (fp, "%s~\n", fix_string (mob_index->description));
    fprintf (fp, "%s~\n", race->name);
    fprintf (fp, "%s ", fwrite_ext_flags_buf (
        mob_flags, mob_index->ext_mob_plus, buf));
    fprintf (fp, "%s ", fwrite_flags_buf (
        affect_flags, mob_index->affected_by_plus, buf));
    fprintf (fp, "%d %d\n", mob_index->alignment, mob_index->group);
    fprintf (fp, "%d ", mob_index->level);
    fprintf (fp, "%d ", mob_index->hitroll);
    fprintf (fp, "%dd%d+%d ", mob_index->hit.number,
             mob_index->hit.size, mob_index->hit.bonus);
    fprintf (fp, "%dd%d+%d ", mob_index->mana.number,
             mob_index->mana.size, mob_index->mana.bonus);
    fprintf (fp, "%dd%d+%d ", mob_index->damage.number,
             mob_index->damage.size, mob_index->damage.bonus);
    fprintf (fp, "%s\n", attack_table[mob_index->attack_type].name);
    fprintf (fp, "%d %d %d %d\n",
             mob_index->ac[AC_PIERCE] / 10,
             mob_index->ac[AC_BASH] / 10,
             mob_index->ac[AC_SLASH] / 10, mob_index->ac[AC_EXOTIC] / 10);
    fprintf (fp, "%s ", fwrite_flags_buf (
        off_flags, mob_index->off_flags_plus, buf));
    fprintf (fp, "%s ", fwrite_flags_buf (
        res_flags, mob_index->imm_flags_plus, buf));
    fprintf (fp, "%s ", fwrite_flags_buf (
        res_flags, mob_index->res_flags_plus, buf));
    fprintf (fp, "%s\n", fwrite_flags_buf (
        res_flags, mob_index->vuln_flags_plus, buf));
    fprintf (fp, "%s %s %s %ld\n",
             position_table[mob_index->start_pos].name,
             position_table[mob_index->default_pos].name,
             sex_table[mob_index->sex].name, mob_index->wealth);
    fprintf (fp, "%s ", fwrite_flags_buf (
        form_flags, mob_index->form_plus, buf));
    fprintf (fp, "%s ", fwrite_flags_buf (
        part_flags, mob_index->parts_plus, buf));

    fprintf (fp, "%s ", size_table[mob_index->size].name);
    fprintf (fp, "%s\n", str_if_null (
        (char *) material_get_name (mob_index->material), "unknown"));

    if (EXT_IS_NONZERO (mob_index->ext_mob_minus))
        fprintf (fp, "F act %s\n", fwrite_ext_flags_buf (
            mob_flags, mob_index->ext_mob_minus, buf));
    if (mob_index->affected_by_minus != 0)
        fprintf (fp, "F aff %s\n", fwrite_flags_buf (
            affect_flags, mob_index->affected_by_minus, buf));
    if (mob_index->off_flags_minus != 0)
        fprintf (fp, "F off %s\n", fwrite_flags_buf (
            off_flags, mob_index->off_flags_minus, buf));
    if (mob_index->imm_flags_minus != 0)
        fprintf (fp, "F imm %s\n", fwrite_flags_buf (
            res_flags, mob_index->imm_flags_minus, buf));
    if (mob_index->res_flags_minus != 0)
        fprintf (fp, "F res %s\n", fwrite_flags_buf (
            res_flags, mob_index->res_flags_minus, buf));
    if (mob_index->vuln_flags_minus != 0)
        fprintf (fp, "F vul %s\n", fwrite_flags_buf (
            res_flags, mob_index->vuln_flags_minus, buf));
    if (mob_index->form_minus != 0)
        fprintf (fp, "F for %s\n", fwrite_flags_buf (
            form_flags, mob_index->form_minus, buf));
    if (mob_index->parts_minus != 0)
        fprintf (fp, "F par %s\n", fwrite_flags_buf (
            part_flags, mob_index->parts_minus, buf));

    for (mprog = mob_index->mprog_first; mprog; mprog = mprog->mob_next) {
        fprintf (fp, "M %s %d %s~\n",
                 mprog_type_to_name (mprog->trig_type), mprog->vnum,
                 mprog->trig_phrase);
    }
}


/*****************************************************************************
 Name:       save_mobiles
 Purpose:    Save #MOBILES secion of an area file.
 Called by:  save_area(olc_save.c).
 Notes:      Changed for ROM OLC.
 ****************************************************************************/
void save_mobiles (FILE *fp, AREA_T *area) {
    int i;
    MOB_INDEX_T *mob;

    fprintf (fp, "#MOBILES\n");
    for (i = area->min_vnum; i <= area->max_vnum; i++)
        if ((mob = mobile_get_index (i)))
            save_mobile (fp, mob);
    fprintf (fp, "#0\n\n");
}

/*****************************************************************************
 Name:       save_object
 Purpose:    Save one object to file.
             new ROM format saving -- Hugin
 Called by:  save_objects (below).
 ****************************************************************************/
void save_object (FILE *fp, OBJ_INDEX_T *obj_index) {
    char letter;
    AFFECT_T *aff;
    EXTRA_DESCR_T *ed;
    char buf[MAX_STRING_LENGTH];

    fprintf (fp, "#%d\n", obj_index->vnum);
    fprintf (fp, "%s~\n", obj_index->name);
    fprintf (fp, "%s~\n", obj_index->short_descr);
    fprintf (fp, "%s~\n", fix_string (obj_index->description));
    fprintf (fp, "%s~\n", str_if_null (
        (char *) material_get_name (obj_index->material), "unknown"));
    fprintf (fp, "%s ", item_get_name (obj_index->item_type));
    fprintf (fp, "%s ", fwrite_flags_buf (
        extra_flags, obj_index->extra_flags, buf));
    fprintf (fp, "%s\n", fwrite_flags_buf (
        wear_flags, obj_index->wear_flags, buf));

    item_index_fwrite_values (obj_index, fp);

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

    for (aff = obj_index->affect_first; aff; aff = aff->on_next) {
        if (aff->bit_type == AFF_TO_OBJECT || aff->bits == 0)
            fprintf (fp, "A\n%d %d\n", aff->apply, aff->modifier);
        else {
            fprintf (fp, "F\n");

            switch (aff->bit_type) {
                case AFF_TO_AFFECTS: fprintf (fp, "A "); break;
                case AFF_TO_IMMUNE:  fprintf (fp, "I "); break;
                case AFF_TO_RESIST:  fprintf (fp, "R "); break;
                case AFF_TO_VULN:    fprintf (fp, "V "); break;
                default:
                    bug ("olc_save: Invalid Affect->where", 0);
                    break;
            }

            fprintf (fp, "%d %d %s\n", aff->apply, aff->modifier,
                     fwrite_flags_buf (affect_flags, aff->bits, buf));
        }
    }

    for (ed = obj_index->extra_descr_first; ed; ed = ed->on_next) {
        fprintf (fp, "E\n%s~\n%s~\n", ed->keyword,
                 fix_string (ed->description));
    }
}

/*****************************************************************************
 Name:       save_objects
 Purpose:    Save #OBJECTS section of an area file.
 Called by:  save_area(olc_save.c).
 Notes:      Changed for ROM OLC.
 ****************************************************************************/
void save_objects (FILE *fp, AREA_T *area) {
    int i;
    OBJ_INDEX_T *obj;

    fprintf (fp, "#OBJECTS\n");
    for (i = area->min_vnum; i <= area->max_vnum; i++)
        if ((obj = obj_get_index (i)))
            save_object (fp, obj);
    fprintf (fp, "#0\n\n");
}

void save_room (FILE *fp, ROOM_INDEX_T *room_index) {
    EXTRA_DESCR_T *ed;
    EXIT_T *ex;
    char buf[MAX_STRING_LENGTH];
    int door;

    fprintf (fp, "#%d\n", room_index->vnum);
    fprintf (fp, "%s~\n", room_index->name);
    fprintf (fp, "%s~\n", fix_string (room_index->description));
    fprintf (fp, "0 ");
    fprintf (fp, "%s ", fwrite_flags_buf (
        room_flags, room_index->room_flags, buf));
    fprintf (fp, "%d\n", room_index->sector_type);

#ifdef BASEMUD_WRITE_PORTALS
    if (room_index->portal)
        fprintf (fp, "P %s~\n", room_index->portal->name);
#endif

    for (door = 0; door < DIR_MAX; door++)
        if ((ex = room_get_orig_exit (room_index, door)))
            save_exit (fp, ex);

    for (ed = room_index->extra_descr_first; ed; ed = ed->on_next)
        fprintf (fp, "E\n%s~\n%s~\n", ed->keyword,
                 fix_string (ed->description));

    if (!IS_NULLSTR (room_index->owner))
        fprintf (fp, "O %s~\n", room_index->owner);

    if (room_index->mana_rate != 100 || room_index->heal_rate != 100)
        fprintf (fp, "M %d H %d\n", room_index->mana_rate, room_index->heal_rate);
    if (room_index->clan > 0)
        fprintf (fp, "C %s~\n", clan_table[room_index->clan].name);

    fprintf (fp, "S\n");
}

void save_exit (FILE *fp, EXIT_T *ex) {
    int locks = 0;

#if 0
    /* HACK : TO PREVENT EX_LOCKED etc without EX_ISDOOR
       to stop booting the mud */
    if (IS_SET (ex->rs_flags, EX_CLOSED)
        || IS_SET (ex->rs_flags, EX_LOCKED)
        || IS_SET (ex->rs_flags, EX_PICKPROOF)
        || IS_SET (ex->rs_flags, EX_NOPASS)
        || IS_SET (ex->rs_flags, EX_EASY)
        || IS_SET (ex->rs_flags, EX_HARD)
        || IS_SET (ex->rs_flags, EX_INFURIATING)
        || IS_SET (ex->rs_flags, EX_NOCLOSE)
        || IS_SET (ex->rs_flags, EX_NOLOCK))
        SET_BIT (ex->rs_flags, EX_ISDOOR);
    else
        REMOVE_BIT (ex->rs_flags, EX_ISDOOR);
#endif

    /* THIS SUCKS but it's backwards compatible */
    /* NOTE THAT EX_NOCLOSE NOLOCK etc aren't being saved */
    if (IS_SET (ex->rs_flags, EX_ISDOOR)
        && (!IS_SET (ex->rs_flags, EX_PICKPROOF))
        && (!IS_SET (ex->rs_flags, EX_NOPASS)))
        locks = 1;
    if (IS_SET (ex->rs_flags, EX_ISDOOR)
        && (IS_SET (ex->rs_flags, EX_PICKPROOF))
        && (!IS_SET (ex->rs_flags, EX_NOPASS)))
        locks = 2;
    if (IS_SET (ex->rs_flags, EX_ISDOOR)
        && (!IS_SET (ex->rs_flags, EX_PICKPROOF))
        && (IS_SET (ex->rs_flags, EX_NOPASS)))
        locks = 3;
    if (IS_SET (ex->rs_flags, EX_ISDOOR)
        && (IS_SET (ex->rs_flags, EX_PICKPROOF))
        && (IS_SET (ex->rs_flags, EX_NOPASS)))
        locks = 4;

    fprintf (fp, "D%d\n", ex->orig_door);
    fprintf (fp, "%s~\n", fix_string (ex->description));
    fprintf (fp, "%s~\n", ex->keyword);
    fprintf (fp, "%d %d %d\n", locks, ex->key,
        (ex->to_room ? ex->to_room->vnum : -1));

#ifdef BASEMUD_WRITE_PORTALS
    if (ex->portal)
        fprintf (fp, "P %s~\n", ex->portal->name);
#endif
}

/*****************************************************************************
 Name:       save_rooms
 Purpose:    Save #ROOMS section of an area file.
 Called by:  save_area(olc_save.c).
 ****************************************************************************/
void save_rooms (FILE *fp, AREA_T *area) {
    ROOM_INDEX_T *room_index;
    int i;

    fprintf (fp, "#ROOMS\n");
    for (i = area->min_vnum; i <= area->max_vnum; i++)
        if ((room_index = room_get_index (i)))
            save_room (fp, room_index);
    fprintf (fp, "#0\n\n");
}

/*****************************************************************************
 Name:       save_specials
 Purpose:    Save #SPECIALS section of area file.
 Called by:  save_area(olc_save.c).
 ****************************************************************************/
void save_specials (FILE *fp, AREA_T *area) {
    int hash;
    MOB_INDEX_T *mob_index;

    fprintf (fp, "#SPECIALS\n");
    for (hash = 0; hash < MAX_KEY_HASH; hash++) {
        for (mob_index = mob_index_hash[hash]; mob_index;
             mob_index = mob_index->hash_next)
        {
            if (mob_index == NULL)
                continue;
            if (mob_index->area != area)
                continue;
            if (!mob_index->spec_fun)
                continue;

#if defined(VERBOSE)
            fprintf (fp, "M %5d %-20s * load to: %s\n", mob_index->vnum,
                spec_function_name (mob_index->spec_fun),
                mob_index->short_descr);
#else
            fprintf (fp, "M %5d %s\n", mob_index->vnum,
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
void save_door_resets (FILE *fp, AREA_T *area) {
    int hash;
    ROOM_INDEX_T *room;
    EXIT_T *ex;
    int door;

    for (hash = 0; hash < MAX_KEY_HASH; hash++) {
        for (room = room_index_hash[hash]; room; room = room->hash_next) {
            if (room->area != area)
                continue;
            for (door = 0; door < DIR_MAX; door++) {
                if ((ex = room->exit[door]) == NULL)
                    continue;
                if (ex->to_room == NULL)
                    continue;
                if (!(IS_SET (ex->rs_flags, EX_CLOSED) ||
                      IS_SET (ex->rs_flags, EX_LOCKED)))
                    continue;

#if defined(VERBOSE)
                fprintf (fp, "D 0 %5d %3d %5d    * The %s door of %s is %s\n",
                    room->vnum, ex->orig_door,
                    IS_SET (ex->rs_flags, EX_LOCKED) ? 2 : 1,
                    door_get_name(ex->orig_door), room->name,
                    IS_SET (ex->rs_flags, EX_LOCKED)
                        ? "closed and locked"
                        : "closed");
#else
                fprintf (fp, "D 0 %5d %3d %5d\n",
                    room->vnum, ex->orig_door,
                    IS_SET (ex->rs_flags, EX_LOCKED) ? 2 : 1);
#endif
            }
        }
    }
}


/*****************************************************************************
 Name:       save_resets
 Purpose:    Saves the #RESETS section of an area file.
 Called by:  save_area(olc_save.c)
 ****************************************************************************/
void save_resets (FILE *fp, AREA_T *area) {
    RESET_T *r;
    MOB_INDEX_T *last_mob = NULL;
    OBJ_INDEX_T *last_obj;
    ROOM_INDEX_T *room;
    int hash;

    fprintf (fp, "#RESETS\n");
    save_door_resets (fp, area);
    for (hash = 0; hash < MAX_KEY_HASH; hash++) {
        for (room = room_index_hash[hash]; room; room = room->hash_next) {
            if (room->area != area)
                continue;

            for (r = room->reset_first; r; r = r->room_next) {
                switch (r->command) {
                    case 'M':
                        last_mob = mobile_get_index (r->v.value[1]);
                        fprintf (fp, "M %d %5d %3d %5d %2d * load %s\n",
                            r->v.value[0], r->v.value[1], r->v.value[2],
                            r->v.value[3], r->v.value[4], last_mob->short_descr);
                        break;

                    case 'O':
                        last_obj = obj_get_index (r->v.value[1]);
                        room = room_get_index (r->v.value[3]);
                        fprintf (fp,
                            "O %d %5d %3d %5d    * %s loaded to %s\n",
                            r->v.value[0], r->v.value[1], r->v.value[2],
                            r->v.value[3], str_capitalized (last_obj->short_descr),
                            room->name);
                        break;

                    case 'P':
                        last_obj = obj_get_index (r->v.value[1]);
                        fprintf (fp,
                            "P %d %5d %3d %5d %2d %s * put inside %s\n",
                            r->v.value[0], r->v.value[1], r->v.value[2],
                            r->v.value[3], r->v.value[4],
                            str_capitalized (obj_get_index (r->v.value[1])->short_descr),
                            last_obj->short_descr);
                        break;

                    case 'G':
                        fprintf (fp,
                            "G %d %5d %3d          * %s is given to %s\n",
                            r->v.value[0], r->v.value[1], r->v.value[2],
                            str_capitalized (obj_get_index (r->v.value[1])->short_descr),
                            last_mob ? last_mob->short_descr : "!NO_MOB!");

                        if (!last_mob)
                            bugf ("Save_resets: !NO_MOB! in [%s]", area->filename);
                        break;

                    case 'E': {
                        const WEAR_LOC_T *wear_loc = wear_loc_get (r->v.value[3]);
                        fprintf (fp,
                            "E %d %5d %3d %5d    * %s is loaded %s of %s\n",
                            r->v.value[0], r->v.value[1], r->v.value[2],
                            r->v.value[3],
                            str_capitalized (obj_get_index (r->v.value[1])->short_descr),
                            wear_loc ? wear_loc->phrase : "none",
                            last_mob ? last_mob->short_descr : "!NO_MOB!");

                        if (!last_mob)
                            bugf ("Save_resets: !NO_MOB! in [%s]", area->filename);
                        break;
                    }

                    case 'D':
                        break;

                    case 'R':
                        room = room_get_index (r->v.value[1]);
                        fprintf (fp, "R %d %5d %3d          * randomize %s\n",
                            r->v.value[0], r->v.value[1], r->v.value[2],
                            room->name);
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
void save_shops (FILE *fp, AREA_T *area) {
    SHOP_T *shop_index;
    MOB_INDEX_T *mob_index;
    int trade, hash;

    fprintf (fp, "#SHOPS\n");
    for (hash = 0; hash < MAX_KEY_HASH; hash++) {
        for (mob_index = mob_index_hash[hash]; mob_index;
             mob_index = mob_index->hash_next)
        {
            if (mob_index && mob_index->area == area && mob_index->shop) {
                shop_index = mob_index->shop;

                fprintf (fp, "%5d ", shop_index->keeper);
                for (trade = 0; trade < MAX_TRADE; trade++) {
                    if (shop_index->buy_type[trade] != 0)
                        fprintf (fp, "%2d ", shop_index->buy_type[trade]);
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
    HELP_T *help = ha->help_first;

    fprintf (fp, "#HELPS\n");
    for (; help; help = help->had_next) {
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

    for (ha = had_first; ha; ha = ha->global_next) {
        if (ha->changed != TRUE)
            break;
        if (!(fp = fopen (olc_save_filename_static (ha->filename), "w"))) {
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
 Name:       save_area
 Purpose:    Save an area, note that this format is new.
 Called by:  do_asave(olc_save.c).
 ****************************************************************************/
void save_area (AREA_T *area) {
    FILE *fp;

    fclose (reserve_file);
    if (!(fp = fopen (olc_save_filename_static (area->filename), "w"))) {
        bug ("save_area: fopen", 0);
        perror (area->filename);
    }

    fprintf (fp, "#AREADATA\n");
    fprintf (fp, "Name %s~\n", area->title);
    fprintf (fp, "Builders %s~\n", fix_string (area->builders));
    fprintf (fp, "VNUMs %d %d\n", area->min_vnum, area->max_vnum);
    fprintf (fp, "Credits %s~\n", area->credits);
    fprintf (fp, "Security %d\n", area->security);
    fprintf (fp, "End\n\n");

    save_mobiles (fp, area);
    save_objects (fp, area);
    save_rooms (fp, area);
    save_resets (fp, area);
    save_shops (fp, area);
    save_specials (fp, area);
    save_mobprogs (fp, area);

    if (area->had_first && area->had_first->help_first)
        save_helps (fp, area->had_first);
    fprintf (fp, "#$\n");

    fclose (fp);
    reserve_file = fopen (NULL_FILE, "r");

    json_export_area (area, JSON_EXPORT_MODE_SAVE);
}
