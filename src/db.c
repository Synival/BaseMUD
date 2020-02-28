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
 *  ROM 2.4 is copyright 1993-1998 Russ Taylor                             *
 *  ROM has been brought to you by the ROM consortium                      *
 *      Russ Taylor (rtaylor@hypercube.org)                                *
 *      Gabrielle Taylor (gtaylor@hypercube.org)                           *
 *      Brian Moore (zump@rom.org)                                         *
 *  By using this code, you have agreed to follow the terms of the         *
 *  ROM license, in the file Rom24/doc/rom.license                         *
 ***************************************************************************/

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stddef.h>
#include <dirent.h>

#include "utils.h"
#include "recycle.h"
#include "affects.h"
#include "lookup.h"
#include "json_import.h"
#include "music.h"
#include "ban.h"
#include "board.h"
#include "portals.h"
#include "rooms.h"
#include "objs.h"
#include "db_old.h"
#include "globals.h"
#include "memory.h"
#include "items.h"
#include "mobiles.h"
#include "skills.h"
#include "fread.h"
#include "areas.h"
#include "mob_prog.h"
#include "resets.h"
#include "extra_descrs.h"
#include "help.h"

#include "db.h"

#if !defined(OLD_RAND)
    #if !defined(linux)
        #if !defined(QMFIXES)
            long random ();
        #endif
    #endif

    #if !defined(QMFIXES)
        void srandom (unsigned int);
    #endif

    int getpid ();
    time_t time (time_t * tloc);
#endif

/* Globals used only during loading. */
static AREA_T *current_area;

int db_hash_from_vnum (int vnum)
    { return vnum % MAX_KEY_HASH; }

void db_register_new_room (ROOM_INDEX_T *room) {
    int vnum = room->vnum;
    room_index_to_hash (room);
    top_vnum_room = top_vnum_room < vnum ? vnum : top_vnum_room; /* OLC */
    assign_area_vnum (vnum, room->area); /* OLC */
}

void db_register_new_mob (MOB_INDEX_T *mob) {
    int vnum = mob->vnum;
    mob_index_to_hash (mob);
    top_vnum_mob = top_vnum_mob < vnum ? vnum : top_vnum_mob; /* OLC */
    assign_area_vnum (vnum, mob->area); /* OLC */
    kill_table[URANGE (0, mob->level, MAX_LEVEL - 1)].number++;
}

void db_register_new_obj (OBJ_INDEX_T *obj) {
    int vnum = obj->vnum;
    obj_index_to_hash (obj);
    top_vnum_obj = top_vnum_obj < vnum ? vnum : top_vnum_obj; /* OLC */
    assign_area_vnum (vnum, obj->area); /* OLC */
}

/* Big mama top level function. */
void boot_db (void) {
    HELP_T *help;

    /* Declare that we're booting the database. */
    in_boot_db = TRUE;

    string_space_init ();
    number_mm_init ();
    init_time_weather ();
    skill_init_mapping ();

    json_import_all ();
    init_areas ();
    music_load_songs ();

    room_link_exits_by_vnum_all ();

    portal_create_missing ();
    portal_link_exits ();

    EXIT_IF_BUGF ((help = help_get_by_name_exact ("greeting")) == NULL,
        "boot_db(): Cannot find help entry 'greeting'.");
    help_greeting = help->text;

    reset_commit_all ();
    if (room_check_resets_all () > 0)
        exit (1);
    room_fix_two_way_exits_all ();

    fix_mobprogs ();

    /* Boot process is over(?) */
    in_boot_db = FALSE;
    log_f ("Boot process complete. %d strings allocated.",
        str_get_strings_allocated());

    convert_objects (); /* ROM OLC */

    area_update_all ();
    board_load_all ();
    board_save_all ();
    ban_load_all ();
}

void init_time_weather (void) {
    const SUN_T *sun;
    const SKY_T *sky;
    long lhour, lday, lmonth;

    /* Set the clock. */
    lhour = (current_time - 650336715) / (PULSE_TICK / PULSE_PER_SECOND);
    time_info.hour  = lhour  % HOURS_PER_DAY;
    lday            = lhour  / HOURS_PER_DAY;
    time_info.day   = lday   % DAYS_PER_MONTH;
    lmonth          = lday   / DAYS_PER_MONTH;
    time_info.month = lmonth % MONTH_MAX;
    time_info.year  = lmonth / MONTH_MAX;

    /* Set the sun type based on hour. */
    sun = sun_get_by_hour (time_info.hour);
    weather_info.sunlight = sun->type;

    /* Randomize barometric pressure. Inner months are stormier. */
    weather_info.change = 0;
    weather_info.mmhg = 960;
    if (time_info.month >= 7 && time_info.month <= 12)
        weather_info.mmhg += number_range (1, 50);
    else
        weather_info.mmhg += number_range (1, 80);

    /* Set the sky type based on barometric pressure. */
    sky = sky_get_by_mmhg (weather_info.mmhg);
    weather_info.sky = sky->type;
}

void init_areas (void) {
    FILE *file_list;
    char fname[MAX_STRING_LENGTH];

    if ((file_list = fopen (AREA_LIST, "r")) == NULL) {
        perror (AREA_LIST);
        exit (1);
    }

    while (1) {
        fread_word (file_list, current_area_filename,
            sizeof (current_area_filename));
        if (current_area_filename[0] == '$')
            break;
        if (current_area_filename[0] == '#')
            continue;

        if (current_area_filename[0] == '-')
            current_area_file = stdin;
        else {
            snprintf (fname, sizeof (fname), "%s%s", AREA_DIR,
                current_area_filename);
            if (area_get_by_filename (current_area_filename) != NULL) {
#ifdef BASEMUD_LOG_FILES_LOADED
                log_f ("Ignoring loaded area '%s'", fname);
#endif
                continue;
            }
            if (help_area_get_by_filename (current_area_filename) != NULL) {
#ifdef BASEMUD_LOG_FILES_LOADED
                log_f ("Ignoring loaded help area '%s'", fname);
#endif
                continue;
            }

            if ((current_area_file = fopen (fname, "r")) == NULL) {
                perror (fname);
                exit (1);
            }
        }

        current_area = NULL;
        while (1) {
            char *word;

            EXIT_IF_BUG (fread_letter (current_area_file) != '#',
                "boot_db: # not found.", 0);

            word = fread_word_static (current_area_file);
            if (word[0] == '$')
                break;

            #define CAF current_area_file
                 if (!str_cmp (word, "AREA"))     load_area     (CAF);
            else if (!str_cmp (word, "AREADATA")) load_area_olc (CAF); /* OLC */
            else if (!str_cmp (word, "HELPS"))    load_helps    (CAF, current_area_filename);
            else if (!str_cmp (word, "MOBOLD"))   load_old_mob  (CAF);
            else if (!str_cmp (word, "MOBILES"))  load_mobiles  (CAF);
            else if (!str_cmp (word, "MOBPROGS")) load_mobprogs (CAF);
            else if (!str_cmp (word, "OBJOLD"))   load_old_obj  (CAF);
            else if (!str_cmp (word, "OBJECTS"))  load_objects  (CAF);
            else if (!str_cmp (word, "RESETS"))   load_resets   (CAF);
            else if (!str_cmp (word, "ROOMS"))    load_rooms    (CAF);
            else if (!str_cmp (word, "SHOPS"))    load_shops    (CAF);
            else if (!str_cmp (word, "SOCIALS"))  load_socials  (CAF);
            else if (!str_cmp (word, "SPECIALS")) load_specials (CAF);
            #undef CAF
            else {
                EXIT_IF_BUG (TRUE, "boot_db: bad section name.", 0);
            }
        }

        if (current_area_file != stdin)
            fclose (current_area_file);
        current_area_file = NULL;
    }
    if (area_last)
        REMOVE_BIT (area_last->area_flags, AREA_LOADING); /* OLC */
    fclose (file_list);
}

/* Snarf an 'area' header line. */
void load_area (FILE *fp) {
    AREA_T *area, *old_area_last;

    area = area_new ();
    fread_string_replace (fp, &area->filename);
    str_replace_dup (&area->name, str_without_extension (area->filename));

    /* Pretty up the log a little */
    log_f ("Loading area '%s'", area->filename);

    area->area_flags = AREA_LOADING;           /* OLC */
    area->security = 9;                        /* OLC 9 -- Hugin */
    str_replace_dup (&area->builders, "None"); /* OLC */
    area->vnum = TOP (RECYCLE_AREA_T);         /* OLC */

    fread_string_replace (fp, &area->title);
    fread_string_replace (fp, &area->credits);
    area->min_vnum = fread_number (fp);
    area->max_vnum = fread_number (fp);
    area->age = 15;
    area->nplayer = 0;
    area->empty = FALSE;

    old_area_last = area_last;
    LIST2_BACK (area, global_prev, global_next, area_first, area_last);
    if (old_area_last)
        REMOVE_BIT (old_area_last->area_flags, AREA_LOADING); /* OLC */
    current_area = area;
}

/* OLC
 * Use these macros to load any new area formats that you choose to
 * support on your MUD.  See the load_area_olc() format below for
 * a short example. */
#if defined(KEY)
    #undef KEY
#endif

#define KEY(literal, field, value)  \
    if (!str_cmp (word, literal)) { \
        field = value;              \
        break;                      \
    }

#define SKEY(string, field)          \
    if (!str_cmp (word, (string))) { \
        fread_string_replace (fp, &(field)); \
        break;                       \
    }

/* OLC
 * Snarf an 'area' header line.   Check this format.  MUCH better.  Add fields
 * too.
 *
 * #AREAFILE
 * Name   { All } Locke    Newbie School~
 * Repop  A teacher pops in the room and says, 'Repop coming!'~
 * Recall 3001
 * End */
void load_area_olc (FILE *fp) {
    AREA_T *area;
    char *word;

    area = area_new ();
    area->age = 15;
    area->nplayer = 0;
    str_replace_dup (&area->filename, current_area_filename);
    area->vnum = TOP (RECYCLE_AREA_T);
    str_replace_dup (&area->title, "New Area");
    str_replace_dup (&area->builders, "");
    area->security = 9;        /* 9 -- Hugin */
    area->min_vnum = 0;
    area->max_vnum = 0;
    area->area_flags = 0;
/*  area->recall       = ROOM_VNUM_TEMPLE;        ROM OLC */

    str_replace_dup (&area->name, str_without_extension (area->filename));
    log_f ("Loading area '%s'", area->filename);

    while (1) {
        word = feof (fp) ? "End" : fread_word_static (fp);
        switch (UPPER (word[0])) {
            case 'N':
                SKEY ("Name", area->title);
                break;

            case 'S':
                KEY ("Security", area->security, fread_number (fp));
                break;

            case 'V':
                if (!str_cmp (word, "VNUMs")) {
                    area->min_vnum = fread_number (fp);
                    area->max_vnum = fread_number (fp);
                }
                break;

            case 'E':
                if (!str_cmp (word, "End")) {
                    LIST2_BACK (area, global_prev, global_next, area_first, area_last);
                    current_area = area;
                    return;
                }
                break;

            case 'B':
                SKEY ("Builders", area->builders);
                break;

            case 'C':
                SKEY ("Credits", area->credits);
                break;
        }
    }
}

/* Sets vnum range for area using OLC protection features. */
void assign_area_vnum (int vnum, AREA_T *area) {
    if (area->min_vnum == 0 && area->max_vnum == 0)
        area->min_vnum = area->max_vnum = vnum;
    if (vnum != URANGE (area->min_vnum, vnum, area->max_vnum)) {
        if (vnum < area->min_vnum)
            area->min_vnum = vnum;
        else
            area->max_vnum = vnum;
    }
}

/* Snarf a help section. */
void load_helps (FILE *fp, char *fname) {
    HELP_T *help;
    int level;
    char *keyword;

    while (1) {
        HELP_AREA_T *had;

        level = fread_number (fp);
        keyword = fread_string_static (fp);

        if (keyword[0] == '$')
            break;

        if (!had_last || str_cmp (fname, had_last->filename)) {
            had = had_new ();
            str_replace_dup (&had->filename, fname);
            str_replace_dup (&had->name, str_without_extension (had->filename));

            help_area_to_area (had, current_area);
            LIST2_BACK (had, global_prev, global_next, had_first, had_last);
        }
        else
            had = had_last;

        help = help_new ();
        help->level = level;
        str_replace_dup (&help->keyword, keyword);
        fread_string_replace (fp, &help->text);

        help_to_help_area (help, had);
        LIST2_BACK (help, global_prev, global_next, help_first, help_last);
    }
}

/* Snarf a reset section. */
void load_resets (FILE *fp) {
    RESET_T *reset;
    RESET_VALUE_T *v;
    int vnum = -1;

    EXIT_IF_BUG (!area_last,
        "load_resets: no #AREA seen yet.", 0);

    while (1) {
        char letter;
        if ((letter = fread_letter (fp)) == 'S')
            break;

        if (letter == '*') {
            fread_to_eol (fp);
            continue;
        }

        reset = reset_data_new ();
        reset_to_area (reset, area_last);
        reset->command = letter;
        v = &(reset->v);

        switch (reset->command) {
            case 'M':
                v->mob._value1      = fread_number (fp);
                v->mob.mob_vnum     = fread_number (fp);
                v->mob.global_limit = fread_number (fp);
                v->mob.room_vnum    = fread_number (fp);
                v->mob.room_limit   = fread_number (fp);
                fread_to_eol (fp);
                vnum = v->mob.room_vnum;
                break;

            case 'O':
                v->obj.room_limit   = fread_number (fp);
                v->obj.obj_vnum     = fread_number (fp);
                v->obj.global_limit = fread_number (fp);
                v->obj.room_vnum    = fread_number (fp);
                v->obj._value5      = 0;
                fread_to_eol (fp);
                vnum = v->obj.room_vnum;
                break;

            case 'P':
                v->put._value1      = fread_number (fp);
                v->put.obj_vnum     = fread_number (fp);
                v->put.global_limit = fread_number (fp);
                v->put.into_vnum    = fread_number (fp);
                v->put.put_count    = fread_number (fp);
                fread_to_eol (fp);
                break;

            case 'G':
                v->give._value1      = fread_number (fp);
                v->give.obj_vnum     = fread_number (fp);
                v->give.global_limit = fread_number (fp);
                v->give._value4      = 0;
                v->give._value5      = 0;
                fread_to_eol (fp);
                break;

            case 'E':
                v->equip._value1      = fread_number (fp);
                v->equip.obj_vnum     = fread_number (fp);
                v->equip.global_limit = fread_number (fp);
                v->equip.wear_loc     = fread_number (fp);
                v->equip._value5      = 0;
                fread_to_eol (fp);
                break;

            case 'D':
                v->door._value1   = fread_number (fp);
                v->door.room_vnum = fread_number (fp);
                v->door.dir       = fread_number (fp);
                v->door.locks     = fread_number (fp);
                v->door._value5   = 0;
                fread_to_eol (fp);
                vnum = v->door.room_vnum;
                break;

            case 'R':
                v->randomize._value1   = fread_number (fp);
                v->randomize.room_vnum = fread_number (fp);
                v->randomize.dir_count = fread_number (fp);
                v->randomize._value4   = 0;
                v->randomize._value5   = 0;
                fread_to_eol (fp);
                vnum = v->randomize.room_vnum;
                break;
        }

        EXIT_IF_BUG (vnum == -1,
            "load_resets: vnum == -1", 0);
        reset->room_vnum = vnum;
    }
}

/* Snarf a room section. */
void load_rooms (FILE *fp) {
    ROOM_INDEX_T *room_index;

    EXIT_IF_BUG (area_last == NULL,
        "load_resets: no #AREA seen yet.", 0);

    while (1) {
        sh_int vnum;
        char letter;
        int door;

        letter = fread_letter (fp);
        EXIT_IF_BUG (letter != '#',
            "load_rooms: # not found.", 0);

        vnum = fread_number (fp);
        if (vnum == 0)
            break;

        in_boot_db = FALSE;
        EXIT_IF_BUG (room_get_index (vnum) != NULL,
            "load_rooms: vnum %d duplicated.", vnum);
        in_boot_db = TRUE;

        room_index = room_index_new ();
        str_replace_dup (&room_index->owner, "");
        room_index->people_first      = NULL;
        room_index->people_last       = NULL;
        room_index->content_first     = NULL;
        room_index->content_last      = NULL;
        room_index->extra_descr_first = NULL;
        room_index->extra_descr_last  = NULL;
        room_to_area (room_index, area_last);
        room_index->vnum        = vnum;
        room_index->anum        = vnum - area_last->min_vnum;

        fread_string_replace (fp, &room_index->name);
        fread_string_replace (fp, &room_index->description);
        /* Area number */ fread_number (fp);
        room_index->room_flags = fread_flag (fp);
        /* horrible hack */
        if (3000 <= vnum && vnum < 3400)
            SET_BIT (room_index->room_flags, ROOM_LAW);
        room_index->sector_type = fread_number (fp);
        room_index->light = 0;
        for (door = 0; door < DIR_MAX; door++)
            room_index->exit[door] = NULL;

        /* defaults */
        room_index->heal_rate = 100;
        room_index->mana_rate = 100;

        while (1) {
            letter = fread_letter (fp);
            if (letter == 'S')
                break;
            if (letter == 'H')    /* healing room */
                room_index->heal_rate = fread_number (fp);
            else if (letter == 'M') /* mana room */
                room_index->mana_rate = fread_number (fp);
            else if (letter == 'C') { /* clan */
                char *clan_str = fread_string_static (fp);
                EXIT_IF_BUG (room_index->clan != 0,
                    "load_rooms: duplicate clan fields.", 0);
                room_index->clan = lookup_func_backup (clan_lookup_exact, clan_str,
                    "Unknown clan '%s'", 0);
            }
            else if (letter == 'D') {
                EXIT_T *pexit;
                int locks;

                door = fread_number (fp);
                EXIT_IF_BUG (door < 0 || door > 5,
                    "load_rooms: vnum %d has bad door number.", vnum);

                pexit = exit_new ();
                fread_string_replace (fp, &pexit->description);
                fread_string_replace (fp, &pexit->keyword);
                locks = fread_number (fp);
                pexit->key = fread_number (fp);
                pexit->vnum = fread_number (fp);
                pexit->area_vnum = -1;
                pexit->room_anum = -1;
                pexit->orig_door = door; /* OLC */

                switch (locks) {
                    case 0:
                        /* Some exits without doors are stored with key 0
                         * (no key) when it should be -1 (no keyhole). Fix
                         * that. */
#ifdef BASEMUD_LOG_KEY_WARNINGS
                        if (pexit->key >= KEY_VALID)
                            bugf ("Warning: Room %d with non-door exit %d has "
                                  "key %d", room_index->vnum, door, pexit->key);
#endif
                        pexit->key = KEY_NOKEYHOLE;
                        break;
                    case 1:
                        pexit->exit_flags = EX_ISDOOR;
                        break;
                    case 2:
                        pexit->exit_flags = EX_ISDOOR | EX_PICKPROOF;
                        break;
                    case 3:
                        pexit->exit_flags = EX_ISDOOR | EX_NOPASS;
                        break;
                    case 4:
                        pexit->exit_flags = EX_ISDOOR | EX_NOPASS | EX_PICKPROOF;
                        break;
                    default:
                        bug ("fread_rooms: bad lock type '%d'.", locks);
                }
                pexit->rs_flags = pexit->exit_flags;
                room_index->exit[door] = pexit;
            }
            else if (letter == 'E') {
                EXTRA_DESCR_T *ed = extra_descr_new ();
                fread_string_replace (fp, &ed->keyword);
                fread_string_replace (fp, &ed->description);
                extra_descr_to_room_index_back (ed, room_index);
            }
            else if (letter == 'O') {
                EXIT_IF_BUG (room_index->owner[0] != '\0',
                    "load_rooms: duplicate owner.", 0);
                fread_string_replace (fp, &room_index->owner);
            }
            else {
                EXIT_IF_BUG (TRUE,
                    "load_rooms: vnum %d has flag not 'DES'.", vnum);
            }
        }

        db_register_new_room (room_index);
    }
}

/* Snarf a shop section. */
void load_shops (FILE *fp) {
    SHOP_T *shop;
    int keeper = 0;

    while (1) {
        MOB_INDEX_T *mob_index;
        int trade;

        /* ROM mem leak fix, check the keeper before allocating the memory
         * to the SHOP_T variable.  -Rhien */
        keeper = fread_number(fp);
        if (keeper == 0)
            break;

        /* Now that we have a non zero keeper number we can allocate */
        shop = shop_new ();
        shop->keeper = keeper;

        for (trade = 0; trade < MAX_TRADE; trade++)
            shop->buy_type[trade] = fread_number(fp);

        shop->profit_buy  = fread_number(fp);
        shop->profit_sell = fread_number(fp);
        shop->open_hour   = fread_number(fp);
        shop->close_hour  = fread_number(fp);
        fread_to_eol(fp);
        mob_index = mobile_get_index(shop->keeper);
        mob_index->shop = shop;

        LIST2_BACK (shop, global_prev, global_next, shop_first, shop_last);
    }
}

/* Snarf spec proc declarations. */
void load_specials (FILE *fp) {
    char *func_name;
    while (1) {
        MOB_INDEX_T *mob_index;
        char letter;

        switch (letter = fread_letter (fp)) {
            case 'S':
                return;

            case '*':
                break;

            case 'M':
                mob_index = mobile_get_index (fread_number (fp));
                func_name = fread_word_static (fp);
                mob_index->spec_fun = spec_lookup_function (func_name);
                EXIT_IF_BUGF (mob_index->spec_fun == NULL,
                    "Unknown special function '%s'", func_name);
                break;

            default:
                EXIT_IF_BUG (TRUE,
                    "load_specials: letter '%c' not *MS.", letter);
        }

        fread_to_eol (fp);
    }
}

/* Load mobprogs section */
void load_mobprogs (FILE *fp) {
    MPROG_CODE_T *mpcode;

    EXIT_IF_BUG (area_last == NULL,
        "load_mobprogs: no #AREA seen yet.", 0);

    while (1) {
        sh_int vnum;
        char letter;

        letter = fread_letter (fp);
        EXIT_IF_BUG (letter != '#',
            "load_mobprogs: # not found.", 0);

        vnum = fread_number (fp);
        if (vnum == 0)
            break;

        in_boot_db = FALSE;
        EXIT_IF_BUG (mpcode_get_index (vnum) != NULL,
            "load_mobprogs: vnum %d duplicated.", vnum);
        in_boot_db = TRUE;

        mpcode = mpcode_new ();
        mpcode_to_area (mpcode, area_last);
        mpcode->vnum = vnum;
        mpcode->anum = vnum - area_last->min_vnum;
        fread_string_replace (fp, &mpcode->code);

        LIST2_FRONT (mpcode, global_prev, global_next, mpcode_first, mpcode_last);
    }
}

/* Translate mobprog vnums pointers to real code */
void fix_mobprogs (void) {
    MOB_INDEX_T *mob_index;
    MPROG_LIST_T *list;
    MPROG_CODE_T *prog;
    int hash;

    for (hash = 0; hash < MAX_KEY_HASH; hash++) {
        for (mob_index = mob_index_hash[hash];
             mob_index != NULL; mob_index = mob_index->hash_next)
        {
            for (list = mob_index->mprog_first; list != NULL; list = list->mob_next) {
                EXIT_IF_BUG ((prog = mpcode_get_index (list->vnum)) == NULL,
                    "fix_mobprogs: code vnum %d not found.", list->vnum);
                list->code = prog->code;
            }
        }
    }
}

/* Added this as part of a bugfix suggested by Chris Litchfield (of
 * "The Mage's Lair" fame). Pets were being loaded with any saved
 * affects, then having those affects loaded again. -- JR 2002/01/31 */
bool check_pet_affected (int vnum, AFFECT_T *paf) {
    MOB_INDEX_T *petIndex;

    petIndex = mobile_get_index (vnum);
    if (petIndex == NULL)
        return FALSE;
    if (paf->bit_type == AFF_TO_AFFECTS &&
        IS_SET (petIndex->affected_by_final, paf->bits))
    {
        return TRUE;
    }
    return FALSE;
}

/* snarf a socials file */
void load_socials (FILE *fp) {
    while (1) {
        SOCIAL_T *new;
        char *temp;

        temp = fread_word_static (fp);
        if (!strcmp (temp, "#0"))
            return; /* done */
#if defined(social_debug)
        else
            fprintf (stderr, "%s\n\r", temp);
#endif

        /* initialize our new social. */
        new = social_new ();
        str_replace_dup (&(new->name), temp);
        fread_to_eol (fp);

        do {
            if (!load_socials_string (fp, &(new->char_no_arg)))    break;
            if (!load_socials_string (fp, &(new->others_no_arg)))  break;
            if (!load_socials_string (fp, &(new->char_found)))     break;
            if (!load_socials_string (fp, &(new->others_found)))   break;
            if (!load_socials_string (fp, &(new->vict_found)))     break;
            if (!load_socials_string (fp, &(new->char_not_found))) break;
            if (!load_socials_string (fp, &(new->char_auto)))      break;
            if (!load_socials_string (fp, &(new->others_auto)))    break;
        } while (0);

        /* I just know this is the path to a 12" 'if' statement.  :(
         * But two players asked for it already!  -- Furey */
        if (!str_cmp (new->name, "snore"))
            new->min_pos = POS_SLEEPING;

        /* if this social already exists, don't load it. */
        if (social_lookup_exact (new->name) != new)
            social_free (new);
    }
}

bool load_socials_string (FILE *fp, char **str) {
    char *temp = fread_string_eol_static (fp);
    if (!strcmp (temp, "$"))
        *str = NULL;
    else if (!strcmp (temp, "#"))
        return FALSE;
    else
        *str = str_dup (temp);
    return TRUE;
}

/* Snarf a mob section.  new style */
void load_mobiles (FILE *fp) {
    MOB_INDEX_T *mob_index;
    char *name, *str;

    EXIT_IF_BUG (!area_last, /* OLC */
        "load_mobiles: no #AREA seen yet.", 0);

    while (1) {
        sh_int vnum;
        char letter;

        letter = fread_letter (fp);
        EXIT_IF_BUG (letter != '#',
            "load_mobiles: # not found.", 0);

        vnum = fread_number (fp);
        if (vnum == 0)
            break;

        in_boot_db = FALSE;
        EXIT_IF_BUG (mobile_get_index (vnum) != NULL,
            "load_mobiles: vnum %d duplicated.", vnum);
        in_boot_db = TRUE;

        mob_index = mob_index_new ();
        mob_index_to_area (mob_index, area_last);
        mob_index->vnum = vnum;
        mob_index->anum = vnum - area_last->min_vnum;
        mob_index->new_format = TRUE;
        newmob_count++;

        /* For some reason, 'oldstyle' is in front of a lot of the names.
         * If this is the case, we're going to ignore it completely. */
        name = fread_string_static (fp);
        if (!str_prefix ("oldstyle", name)) {
            name += 8;
            while (*name == ' ')
                name++;
        }
        str_replace_dup (&mob_index->name, name);

        fread_string_replace (fp, &mob_index->short_descr);
        fread_string_replace (fp, &mob_index->long_descr);
        fread_string_replace (fp, &mob_index->description);

        str = fread_string_static (fp);
        mob_index->race = lookup_func_backup (race_lookup_exact,
            str, "Unknown race '%s'", 0);

        mob_index->long_descr[0] = UPPER (mob_index->long_descr[0]);
        mob_index->description[0] = UPPER (mob_index->description[0]);

        mob_index->ext_mob_plus = fread_ext_flag (fp, mob_flags);
        EXT_UNSET (mob_index->ext_mob_plus, MOB_IS_NPC);

        mob_index->affected_by_plus = fread_flag (fp);

        mob_index->shop = NULL;
        mob_index->alignment = fread_number (fp);
        mob_index->group = fread_number (fp);

        mob_index->level = fread_number (fp);
        mob_index->hitroll = fread_number (fp);

        /* read hit, mana, dam dice */
        fread_dice (fp, &(mob_index->hit));
        fread_dice (fp, &(mob_index->mana));
        fread_dice (fp, &(mob_index->damage));

        str = fread_word_static (fp);
        mob_index->attack_type = lookup_func_backup (attack_lookup_exact,
            str, "Unknown damage type '%s'", 0);

        /* read armor class */
        mob_index->ac[AC_PIERCE] = fread_number (fp) * 10;
        mob_index->ac[AC_BASH]   = fread_number (fp) * 10;
        mob_index->ac[AC_SLASH]  = fread_number (fp) * 10;
        mob_index->ac[AC_EXOTIC] = fread_number (fp) * 10;

        /* read flags and add in data from the race table */
        mob_index->off_flags_plus  = fread_flag (fp);
        mob_index->imm_flags_plus  = fread_flag (fp);
        mob_index->res_flags_plus  = fread_flag (fp);
        mob_index->vuln_flags_plus = fread_flag (fp);

        /* vital statistics */
        str = fread_word_static (fp);
        mob_index->start_pos = lookup_func_backup (position_lookup_exact,
            str, "Unknown start position '%s'", POS_STANDING);

        str = fread_word_static (fp);
        mob_index->default_pos = lookup_func_backup (position_lookup_exact,
            str, "Unknown default position '%s'", POS_STANDING);

        /* read sex. 'none' has been replaced with 'neutral' to avoid confusion
         * between 'sexless' and 'sex is missing'. */
        str = fread_word_static (fp);
        if (!str_cmp (str, "none"))
            str = "neutral";
        mob_index->sex = lookup_func_backup (sex_lookup_exact,
            str, "Unknown sex '%s'", SEX_EITHER);

        mob_index->wealth     = fread_number (fp);
        mob_index->form_plus  = fread_flag (fp);
        mob_index->parts_plus = fread_flag (fp);

        /* Size. */
        str = fread_word_static (fp);
        mob_index->size = lookup_func_backup (size_lookup_exact, str,
            "Unknown size '%s'", SIZE_MEDIUM);

        /* Material. Sometimes this is '0', in which case, just replace it
         * with the default material's keyword. */
        str = fread_word_static (fp);
        if (!str_cmp (str, "0") || str[0] == '\0')
            str = (char *) material_get_name (MATERIAL_GENERIC);
        mob_index->material = lookup_func_backup (material_lookup_exact,
            str, "Unknown material '%s'", MATERIAL_GENERIC);

        while (1) {
            letter = fread_letter (fp);
            if (letter == 'F') {
                char *word;
                long vector;

                word = fread_word_static (fp);
                vector = fread_flag (fp);

                if (!str_prefix (word, "act") || !str_prefix (word, "mob"))
                    mob_index->ext_mob_minus = EXT_FROM_FLAG_T (vector);
                else if (!str_prefix (word, "aff"))
                    mob_index->affected_by_minus = vector;
                else if (!str_prefix (word, "off"))
                    mob_index->off_flags_minus = vector;
                else if (!str_prefix (word, "imm"))
                    mob_index->imm_flags_minus = vector;
                else if (!str_prefix (word, "res"))
                    mob_index->res_flags_minus = vector;
                else if (!str_prefix (word, "vul"))
                    mob_index->vuln_flags_minus = vector;
                else if (!str_prefix (word, "for"))
                    mob_index->form_minus = vector;
                else if (!str_prefix (word, "par"))
                    mob_index->parts_minus = vector;
                else {
                    EXIT_IF_BUG (TRUE,
                        "flag remove: flag not found.", 0);
                }
            }
            else if (letter == 'M') {
                MPROG_LIST_T *mprog;
                char *word;
                int trigger = 0;

                mprog = mprog_new ();
                word = fread_word_static (fp);
                EXIT_IF_BUG (
                    (trigger = flag_lookup_exact (mprog_flags, word)) == FLAG_NONE,
                    "load_mobiles: invalid mob prog trigger.", 0);
                SET_BIT (mob_index->mprog_flags, trigger);
                mprog->trig_type = trigger;
                mprog_to_area (mprog, area_last);
                mprog->vnum = fread_number (fp);
                mprog->anum = mprog->vnum - area_last->min_vnum;
                fread_string_replace (fp, &mprog->trig_phrase);

                LIST2_BACK (mprog, mob_prev, mob_next,
                    mob_index->mprog_first, mob_index->mprog_last);
            }
            else {
                ungetc (letter, fp);
                break;
            }
        }

        /* Perform post-processing on loaded mob. */
        db_finalize_mob (mob_index);

        /* Loading finished - register our mob. */
        db_register_new_mob (mob_index);
    }
}

void db_finalize_mob (MOB_INDEX_T *mob) {
    const RACE_T *race;
    if ((mob->race >= 0 || mob->race < RACE_MAX) &&
            (race = race_get (mob->race)) != NULL)
    {
        mob->ext_mob_final     = mob->ext_mob_plus;
        EXT_SET      (mob->ext_mob_final, MOB_IS_NPC);
        EXT_SET_MANY (mob->ext_mob_final, race->ext_mob);

        mob->affected_by_final = mob->affected_by_plus | race->aff;
        mob->off_flags_final   = mob->off_flags_plus   | race->off;
        mob->imm_flags_final   = mob->imm_flags_plus   | race->imm;
        mob->res_flags_final   = mob->res_flags_plus   | race->res;
        mob->vuln_flags_final  = mob->vuln_flags_plus  | race->vuln;
        mob->form_final        = mob->form_plus        | race->form;
        mob->parts_final       = mob->parts_plus       | race->parts;
    }
    else {
        mob->ext_mob_final     = mob->ext_mob_plus;
        EXT_SET  (mob->ext_mob_final, MOB_IS_NPC);

        mob->affected_by_final = mob->affected_by_plus;
        mob->off_flags_final   = mob->off_flags_plus;
        mob->imm_flags_final   = mob->imm_flags_plus;
        mob->res_flags_final   = mob->res_flags_plus;
        mob->vuln_flags_final  = mob->vuln_flags_plus;
        mob->form_final        = mob->form_plus;
        mob->parts_final       = mob->parts_plus;
    }

    EXT_UNSET_MANY (mob->ext_mob_final, mob->ext_mob_minus);
    REMOVE_BIT (mob->affected_by_final, mob->affected_by_minus);
    REMOVE_BIT (mob->off_flags_final,   mob->off_flags_minus);
    REMOVE_BIT (mob->imm_flags_final,   mob->imm_flags_minus);
    REMOVE_BIT (mob->res_flags_final,   mob->res_flags_minus);
    REMOVE_BIT (mob->vuln_flags_final,  mob->vuln_flags_minus);
    REMOVE_BIT (mob->form_final,        mob->form_minus);
    REMOVE_BIT (mob->parts_final,       mob->parts_minus);
}

void db_finalize_obj (OBJ_INDEX_T *obj_index) {
    item_index_finalize (obj_index);
}

/* Snarf an obj section. new style */
void load_objects (FILE *fp) {
    OBJ_INDEX_T *obj_index;
    char *str;
    sh_int location, modifier;
    flag_t bits;

    EXIT_IF_BUG (!area_last, /* OLC */
        "load_objects: no #AREA seen yet.", 0);

    while (1) {
        sh_int vnum;
        char letter;

        letter = fread_letter (fp);
        EXIT_IF_BUG (letter != '#',
            "load_objects: # not found.", 0);

        vnum = fread_number (fp);
        if (vnum == 0)
            break;

        in_boot_db = FALSE;
        EXIT_IF_BUG (obj_get_index (vnum) != NULL,
            "load_objects: vnum %d duplicated.", vnum);
        in_boot_db = TRUE;

        obj_index = obj_index_new ();
        obj_index_to_area (obj_index, area_last);
        obj_index->vnum = vnum;
        obj_index->anum = vnum - area_last->min_vnum;
        obj_index->new_format = TRUE;
        obj_index->reset_num = 0;
        newobj_count++;
        fread_string_replace (fp, &obj_index->name);
        fread_string_replace (fp, &obj_index->short_descr);
        fread_string_replace (fp, &obj_index->description);

        /* Read material, and replace 'oldstyle' (???) or 'blank' with 'generic'. */
        str = fread_string_static (fp);
        if (!str_cmp (str, "oldstyle") || str[0] == '\0')
            str = (char *) material_get_name (MATERIAL_GENERIC);
        obj_index->material = lookup_func_backup (material_lookup_exact,
            str, "Unknown material '%s'", MATERIAL_GENERIC);

        str = fread_word_static (fp);
        obj_index->item_type = lookup_func_backup (item_lookup_exact,
            str, "Unknown item type '%s'", 0);

        obj_index->extra_flags = fread_flag (fp);
        obj_index->wear_flags = fread_flag (fp);

        item_index_read_values_from_file (obj_index, fp);

        obj_index->level  = fread_number (fp);
        obj_index->weight = fread_number (fp);
        obj_index->cost   = fread_number (fp);

        /* condition */
        letter = fread_letter (fp);
        switch (letter) {
            case ('P'): obj_index->condition = 100; break;
            case ('G'): obj_index->condition = 90;  break;
            case ('A'): obj_index->condition = 75;  break;
            case ('W'): obj_index->condition = 50;  break;
            case ('D'): obj_index->condition = 25;  break;
            case ('B'): obj_index->condition = 10;  break;
            case ('R'): obj_index->condition = 0;   break;
            default:    obj_index->condition = 100; break;
        }

        while (1) {
            char letter = fread_letter (fp);
            if (letter == 'A') {
                AFFECT_T *paf = affect_new ();

                location = fread_number (fp);
                modifier = fread_number (fp);
                affect_init (paf, AFF_TO_OBJECT, -1, obj_index->level, -1, location, modifier, 0);
                affect_to_obj_index_back (paf, obj_index);
            }
            else if (letter == 'F') {
                AFFECT_T *paf = affect_new ();

                letter = fread_letter (fp);
                switch (letter) {
                    case 'A': paf->bit_type = AFF_TO_AFFECTS; break;
                    case 'I': paf->bit_type = AFF_TO_IMMUNE;  break;
                    case 'R': paf->bit_type = AFF_TO_RESIST;  break;
                    case 'V': paf->bit_type = AFF_TO_VULN;    break;
                    default:
                        EXIT_IF_BUG (TRUE,
                            "load_objects: Bad 'bit_type' on flag set.", 0);
                }

                location  = fread_number (fp);
                modifier  = fread_number (fp);
                bits = fread_flag (fp);
                affect_init (paf, paf->bit_type, -1, obj_index->level, -1,
                    location, modifier, bits);
                affect_to_obj_index_back (paf, obj_index);
            }
            else if (letter == 'E') {
                EXTRA_DESCR_T *ed = extra_descr_new ();
                fread_string_replace (fp, &ed->keyword);
                fread_string_replace (fp, &ed->description);
                extra_descr_to_obj_index_back (ed, obj_index);
            }
            else {
                ungetc (letter, fp);
                break;
            }
        }

        /* Perform post-processing on loaded obj. */
        db_finalize_obj (obj_index);

        /* Loading finished - register our object. */
        db_register_new_obj (obj_index);
    }
}

void db_dump_world (const char *filename) {
    FILE *file = fopen (filename, "w");
    const AREA_T *area;
    const ROOM_INDEX_T *room;
    const EXTRA_DESCR_T *ed;
    const RESET_T *reset;
    const MOB_INDEX_T *mob;
    const OBJ_INDEX_T *obj;
    const AFFECT_T *aff;
    const HELP_AREA_T *had;
    const HELP_T *help;
    const SOCIAL_T *soc;
    const PORTAL_T *portal;
    int i, count = 0;

    log_f ("Dumping entire world to '%s'", filename);

    for (area = area_get_first(); area; area = area_get_next (area)) {
        fprintf (file, "global_next:%s\n",  area->global_next ? "yes" : "no");
        fprintf (file, "had_first:  %s\n",  area->had_first   ? "yes" : "no");
        fprintf (file, "name:       %s\n",  area->name);
        fprintf (file, "filename:   %s\n",  area->filename);
        fprintf (file, "title:      %s\n",  area->title);
        fprintf (file, "credits:    %s\n",  area->credits);
        fprintf (file, "age:        %d\n",  area->age);
        fprintf (file, "nplayer:    %d\n",  area->nplayer);
        fprintf (file, "low_range:  %d\n",  area->low_range);
        fprintf (file, "high_range: %d\n",  area->high_range);
        fprintf (file, "min_vnum:   %d\n",  area->min_vnum);
        fprintf (file, "max_vnum:   %d\n",  area->max_vnum);
        fprintf (file, "empty:      %d\n",  area->empty);
        fprintf (file, "builders:   %s\n",  area->builders);
        fprintf (file, "vnum:       %d\n",  area->vnum);
        fprintf (file, "area_flags: %ld\n", area->area_flags);
        fprintf (file, "security:   %d\n",  area->security);
        fprintf (file, "-\n");
        count++;
    }

    for (room = room_index_get_first(); room;
         room = room_index_get_next (room))
    {
        fprintf (file, "hash_next:   %s\n", room->hash_next     ? "yes" : "no");
        fprintf (file, "people:      %s\n", room->people_first  ? "yes" : "no");
        fprintf (file, "contents:    %s\n", room->content_first ? "yes" : "no");
        fprintf (file, "extra_descr: %s\n", room->extra_descr_first ? "yes" : "no");
        for (ed = room->extra_descr_first; ed != NULL; ed = ed->on_next) {
            fprintf (file, "   keyword:     %s\n", ed->keyword);
            fprintf (file, "   description: %s\n", ed->description);
        }

        fprintf (file, "area_str:    %s\n", room->area_str);
        fprintf (file, "area:        %s\n", room->area        ? "yes" : "no");

        for (i = 0; i < DIR_MAX; i++) {
            fprintf (file, "exit[%d]:    %s\n", i, room->exit[i] ? "yes" : "no");
            if (room->exit[i]) {
                EXIT_T *exit = room->exit[i];
                fprintf (file, "   to_room:     %s\n",  exit->to_room ? "yes" : "no");
                fprintf (file, "   vnum:        %d\n",  exit->vnum);
                fprintf (file, "   area_vnum:   %d\n",  exit->area_vnum);
                fprintf (file, "   room_anum:   %d\n",  exit->room_anum);
                fprintf (file, "   exit_flags:  %ld\n", exit->exit_flags);
                fprintf (file, "   key:         %d\n",  exit->key);
                fprintf (file, "   keyword:     %s\n",  exit->keyword);
                fprintf (file, "   description: %s\n",  exit->description);
                fprintf (file, "   rs_flags:    %ld\n", exit->rs_flags);
                fprintf (file, "   orig_door:   %d\n",  exit->orig_door);
                fprintf (file, "   portal:      %s\n",  exit->portal ? "yes" : "no");
            }
        }

        fprintf (file, "reset_first: %s\n",  room->reset_first   ? "yes" : "no");
        fprintf (file, "reset_last:  %s\n",  room->reset_last    ? "yes" : "no");
        for (reset = room->reset_first; reset != NULL; reset = reset->room_next) {
            fprintf (file, "   room_next: %s\n",   reset->room_next ? "yes" : "no");
            fprintf (file, "   area:      %s\n",   reset->area ? "yes" : "no");
            fprintf (file, "   command:   '%c'\n", reset->command);
            fprintf (file, "   room_vnum: %d\n",   reset->room_vnum);
            for (i = 0; i < RESET_VALUE_MAX; i++)
                fprintf (file, "   v[%d]:      %d\n", i, reset->v.value[i]);
        }

        fprintf (file, "name:        %s\n",  room->name);
        fprintf (file, "description: %s\n",  room->description);
        fprintf (file, "owner:       %s\n",  room->owner);
        fprintf (file, "vnum:        %d\n",  room->vnum);
        fprintf (file, "anum:        %d\n",  room->anum);
        fprintf (file, "room_flags:  %ld\n", room->room_flags);
        fprintf (file, "light:       %d\n",  room->light);
        fprintf (file, "sector_type: %d\n",  room->sector_type);
        fprintf (file, "heal_rate:   %d\n",  room->heal_rate);
        fprintf (file, "mana_rate:   %d\n",  room->mana_rate);
        fprintf (file, "clan:        %d\n",  room->clan);
        fprintf (file, "portal:      %s\n",  room->portal ? "yes" : "no");
        fprintf (file, "-\n");
        count++;
    }

    for (mob = mob_index_get_first(); mob; mob = mob_index_get_next (mob)) {
        fprintf (file, "hash_next:          %s\n",  mob->hash_next ? "yes" : "no");
        fprintf (file, "spec_fun:           %s\n",  spec_function_name (mob->spec_fun));
        fprintf (file, "shop:               %s\n",  mob->shop ? "yes" : "no");
        if (mob->shop) {
            SHOP_T *shop = mob->shop;
            fprintf (file, "   next:         %s\n", shop->global_next ? "yes" : "no");
            fprintf (file, "   keeper:       %d\n", shop->keeper);
            for (i = 0; i < MAX_TRADE; i++)
                if (shop->buy_type[i] != 0)
                    fprintf (file, "   buy_type[%d]: %d\n", i, shop->buy_type[i]);
            fprintf (file, "   profit_buy:   %d\n", shop->profit_buy);
            fprintf (file, "   profit_sell:  %d\n", shop->profit_sell);
            fprintf (file, "   open_hour:    %d\n", shop->open_hour);
            fprintf (file, "   close_hour:   %d\n", shop->close_hour);
        }
        fprintf (file, "mprog_first:        %s\n",  mob->mprog_first ? "yes" : "no");
        fprintf (file, "area_str:           %s\n",  mob->area_str);
        fprintf (file, "area:               %s\n",  mob->area ? "yes" : "no");
        fprintf (file, "vnum:               %d\n",  mob->vnum);
        fprintf (file, "anum:               %d\n",  mob->anum);
        fprintf (file, "group:              %d\n",  mob->group);
        fprintf (file, "new_format:         %d\n",  mob->new_format);
        fprintf (file, "count:              %d\n",  mob->mob_count);
        fprintf (file, "killed:             %d\n",  mob->killed);
        fprintf (file, "name:               %s\n",  mob->name);
        fprintf (file, "short_descr:        %s\n",  mob->short_descr);
        fprintf (file, "long_descr:         %s\n",  mob->long_descr);
        fprintf (file, "description:        %s\n",  mob->description);
        fprintf (file, "alignment:          %d\n",  mob->alignment);
        fprintf (file, "level:              %d\n",  mob->level);
        fprintf (file, "hitroll:            %d\n",  mob->hitroll);
        fprintf (file, "hit.number:         %d\n",  mob->hit.number);
        fprintf (file, "hit.size:           %d\n",  mob->hit.size);
        fprintf (file, "hit.bonus:          %d\n",  mob->hit.bonus);
        fprintf (file, "mana.number:        %d\n",  mob->mana.number);
        fprintf (file, "mana.size:          %d\n",  mob->mana.size);
        fprintf (file, "mana.bonus:         %d\n",  mob->mana.bonus);
        fprintf (file, "damage.number:      %d\n",  mob->damage.number);
        fprintf (file, "damage.size:        %d\n",  mob->damage.size);
        fprintf (file, "damage.bonus:       %d\n",  mob->damage.bonus);
        fprintf (file, "ac[0]:              %d\n",  mob->ac[0]);
        fprintf (file, "ac[1]:              %d\n",  mob->ac[1]);
        fprintf (file, "ac[2]:              %d\n",  mob->ac[2]);
        fprintf (file, "ac[3]:              %d\n",  mob->ac[3]);
        fprintf (file, "attack_type:        %d\n",  mob->attack_type);
        fprintf (file, "start_pos:          %d\n",  mob->start_pos);
        fprintf (file, "default_pos:        %d\n",  mob->default_pos);
        fprintf (file, "sex:                %d\n",  mob->sex);
        fprintf (file, "race:               %d\n",  mob->race);
        fprintf (file, "wealth:             %ld\n", mob->wealth);
        fprintf (file, "size:               %d\n",  mob->size);
        fprintf (file, "material:           %d\n",  mob->material);
        fprintf (file, "mprog_flags:        %ld\n", mob->mprog_flags);
        fprintf (file, "mob_plus:           %ld\n", EXT_TO_FLAG_T (mob->ext_mob_plus));
        fprintf (file, "mob_final:          %ld\n", EXT_TO_FLAG_T (mob->ext_mob_final));
        fprintf (file, "mob_minus:          %ld\n", EXT_TO_FLAG_T (mob->ext_mob_minus));
        fprintf (file, "affected_by_plus:   %ld\n", mob->affected_by_plus);
        fprintf (file, "affected_by_final:  %ld\n", mob->affected_by_final);
        fprintf (file, "affected_by_minus:  %ld\n", mob->affected_by_minus);
        fprintf (file, "off_flags_plus:     %ld\n", mob->off_flags_plus);
        fprintf (file, "off_flags_final:    %ld\n", mob->off_flags_final);
        fprintf (file, "off_flags_minus:    %ld\n", mob->off_flags_minus);
        fprintf (file, "imm_flags_plus:     %ld\n", mob->imm_flags_plus);
        fprintf (file, "imm_flags_final:    %ld\n", mob->imm_flags_final);
        fprintf (file, "imm_flags_minus:    %ld\n", mob->imm_flags_minus);
        fprintf (file, "res_flags_plus:     %ld\n", mob->res_flags_plus);
        fprintf (file, "res_flags_final:    %ld\n", mob->res_flags_final);
        fprintf (file, "res_flags_minus:    %ld\n", mob->res_flags_minus);
        fprintf (file, "vuln_flags_plus:    %ld\n", mob->vuln_flags_plus);
        fprintf (file, "vuln_flags_final:   %ld\n", mob->vuln_flags_final);
        fprintf (file, "vuln_flags_minus:   %ld\n", mob->vuln_flags_minus);
        fprintf (file, "form_plus:          %ld\n", mob->form_plus);
        fprintf (file, "form_final:         %ld\n", mob->form_final);
        fprintf (file, "form_minus:         %ld\n", mob->form_minus);
        fprintf (file, "parts_plus:         %ld\n", mob->parts_plus);
        fprintf (file, "parts_final:        %ld\n", mob->parts_final);
        fprintf (file, "parts_minus:        %ld\n", mob->parts_minus);
        fprintf (file, "-\n");
        count++;
    }

    for (obj = obj_index_get_first(); obj; obj = obj_index_get_next (obj)) {
        fprintf (file, "hash_next:     %s\n",  obj->hash_next ? "yes" : "no");
        fprintf (file, "extra_descr:   %s\n",  obj->extra_descr_first ? "yes" : "no");
        for (ed = obj->extra_descr_first; ed != NULL; ed = ed->on_next) {
            fprintf (file, "   keyword:     %s\n", ed->keyword);
            fprintf (file, "   description: %s\n", ed->description);
        }

        fprintf (file, "affected:      %s\n",  obj->affect_first ? "yes" : "no");
        for (aff = obj->affect_first; ed != NULL; ed = ed->on_next) {
            fprintf (file, "   next:     %s\n",  aff->on_next ? "yes" : "no");
            fprintf (file, "   bit_type: %d\n",  aff->bit_type);
            fprintf (file, "   type:     %d\n",  aff->type);
            fprintf (file, "   level:    %d\n",  aff->level);
            fprintf (file, "   duration: %d\n",  aff->duration);
            fprintf (file, "   apply:    %d\n",  aff->apply);
            fprintf (file, "   modifier: %d\n",  aff->modifier);
            fprintf (file, "   bits:     %ld\n", aff->bits);
        }

        fprintf (file, "area_str:      %s\n",  obj->area_str);
        fprintf (file, "area:          %s\n",  obj->area ? "yes" : "no");
        fprintf (file, "new_format:    %d\n",  obj->new_format);
        fprintf (file, "name:          %s\n",  obj->name);
        fprintf (file, "short_descr:   %s\n",  obj->short_descr);
        fprintf (file, "description:   %s\n",  obj->description);
        fprintf (file, "vnum:          %d\n",  obj->vnum);
        fprintf (file, "anum:          %d\n",  obj->anum);
        fprintf (file, "reset_num:     %d\n",  obj->reset_num);
        fprintf (file, "material:      %d\n",  obj->material);
        fprintf (file, "item_type:     %d\n",  obj->item_type);
        fprintf (file, "extra_flags:   %ld\n", obj->extra_flags);
        fprintf (file, "wear_flags:    %ld\n", obj->wear_flags);
        fprintf (file, "level:         %d\n",  obj->level);
        fprintf (file, "condition:     %d\n",  obj->condition);
        fprintf (file, "count:         %d\n",  obj->obj_count);
        fprintf (file, "weight:        %d\n",  obj->weight);
        fprintf (file, "cost:          %d\n",  obj->cost);
        for (i = 0; i < OBJ_VALUE_MAX; i++)
            fprintf (file, "v.value[%d]:    %ld\n", i, obj->v.value[i]);
        fprintf (file, "-\n");
        count++;
    }

    for (had = had_get_first(); had; had = had_get_next (had)) {
        fprintf (file, "global_next: %s\n", had->global_next  ? "yes" : "no");
        fprintf (file, "area_next:   %s\n", had->area_next ? "yes" : "no");
        fprintf (file, "help_first:  %s\n", had->help_first ? "yes" : "no");
        fprintf (file, "area_str:    %s\n", had->area_str);
        fprintf (file, "area:        %s\n", had->area  ? "yes" : "no");
        fprintf (file, "filename:    %s\n", had->filename);
        fprintf (file, "name:        %s\n", had->name);
        fprintf (file, "changed:     %d\n", had->changed);
        fprintf (file, "-\n");
        count++;
    }

    for (help = help_get_first(); help; help = help_get_next (help)) {
        fprintf (file, "global_next: %s\n", help->global_next ? "yes" : "no");
        fprintf (file, "had_next:    %s\n", help->had_next ? "yes" : "no");
        fprintf (file, "level:       %d\n", help->level);
        fprintf (file, "keyword:     %s\n", help->keyword);
        fprintf (file, "text:        %s\n", help->text);
        fprintf (file, "-\n");
        count++;
    }

    for (soc = social_get_first(); soc; soc = social_get_next (soc)) {
        fprintf (file, "name:           %s\n", soc->name);
        fprintf (file, "char_no_arg:    %s\n", soc->char_no_arg);
        fprintf (file, "others_no_arg:  %s\n", soc->others_no_arg);
        fprintf (file, "char_found:     %s\n", soc->char_found);
        fprintf (file, "others_found:   %s\n", soc->others_found);
        fprintf (file, "vict_found:     %s\n", soc->vict_found);
        fprintf (file, "char_not_found: %s\n", soc->char_not_found);
        fprintf (file, "char_auto:      %s\n", soc->char_auto);
        fprintf (file, "others_auto:    %s\n", soc->others_auto);
        fprintf (file, "-\n");
        count++;
    }

    for (portal = portal_get_first(); portal; portal = portal_get_next (portal)) {
        fprintf (file, "name_from: %s\n", portal->name_from);
        fprintf (file, "name_to:   %s\n", portal->name_to);
        fprintf (file, "two_way:   %d\n", portal->two_way);
        fprintf (file, "from:      %s\n", portal->from ? "yes" : "no");
        fprintf (file, "to:        %s\n", portal->to   ? "yes" : "no");
        count++;
    }

    log_f ("%d entities dumped.", count);
    fclose (file);
}

ANUM_T *anum_new (void) {
    ANUM_T *anum;
    anum = calloc (1, sizeof (ANUM_T));
    LIST2_BACK (anum, global_prev, global_next, anum_first, anum_last);
    return anum;
}

void anum_free (ANUM_T *anum) {
    str_free (&(anum->area_str));
    LIST2_REMOVE (anum, global_prev, global_next, anum_first, anum_last);
    free (anum);
}
