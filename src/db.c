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

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stddef.h>
#include <dirent.h>

#include "utils.h"
#include "recycle.h"
#include "affects.h"
#include "lookup.h"
#include "json.h"
#include "json_obj.h"
#include "json_write.h"
#include "json_read.h"
#include "json_import.h"
#include "music.h"
#include "ban.h"
#include "board.h"
#include "olc.h"
#include "portals.h"
#include "chars.h"
#include "rooms.h"
#include "objs.h"
#include "db_old.h"
#include "update.h"
#include "globals.h"
#include "memory.h"

#include "db.h"

#if !defined(macintosh)
    extern int _filbuf args ((FILE *));
#endif

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

AREA_T *db_link_areas_get_area (char **name) {
    AREA_T *area;

    if (*name == NULL)
        return NULL;

    area = area_get_by_name (*name);
    if (area == NULL)
        bugf ("db_link_areas_get_area(): area '%s' not found.", *name);
    str_free (&(*name));

    return area;
}

void db_link_areas (void) {
    HELP_AREA_T *had;
    AREA_T *area;
    ROOM_INDEX_T *room, *to_room;
    MOB_INDEX_T *mob;
    OBJ_INDEX_T *obj;
    EXIT_T *exit;
    ANUM_T *anum, *anum_next;
    RESET_T *reset;
    int i;

    /* For each 'anum' value, determine the 'vnum'. */
    for (anum = anum_first; anum != NULL; anum = anum_next) {
        anum_next = anum->next;
        do {
            if (anum->vnum_ref == NULL)
                continue;
            if ((area = db_link_areas_get_area (&(anum->area_str))) == NULL)
                continue;
            *(anum->vnum_ref) = area->min_vnum + anum->anum;
        } while (0);
        anum_free (anum);
    }

    /* Link rooms and their resets to areas and register them. */
    for (room = room_index_get_first(); room != NULL;
         room = room_index_get_next(room))
    {
        if ((area = db_link_areas_get_area (&(room->area_str))) == NULL)
            continue;
        room->area = area;
        room->vnum = room->anum + area->min_vnum;
        for (reset = room->reset_first; reset != NULL; reset = reset->next)
            reset->room_vnum = room->vnum;
        db_register_new_room (room);
    }

    /* Now that rooms have vnums, link exits. */
    for (room = room_index_get_first(); room != NULL;
         room = room_index_get_next(room))
    {
        for (i = 0; i < DIR_MAX; i++) {
            if ((exit = room->exit[i]) == NULL)
                continue;
            exit->area_vnum = -1;
            if (exit->room_anum < 0)
                continue;

            exit->vnum = exit->room_anum + room->area->min_vnum;
            if ((to_room = get_room_index (exit->vnum)) == NULL) {
                bugf ("db_link_areas(): Room '%s' (%d) exit '%d' cannot find "
                    "room with vnum %d.\n", room->name, room->vnum, i,
                    exit->vnum);
                continue;
            }
            exit->to_room = to_room;
            exit->area_vnum = to_room->area->vnum;
        }
    }

    /* Link mobs to areas and register them. */
    for (mob = mob_index_get_first(); mob != NULL;
         mob = mob_index_get_next(mob))
    {
        if ((area = db_link_areas_get_area (&(mob->area_str))) == NULL)
            continue;
        mob->area = area;
        mob->vnum = mob->anum + area->min_vnum;
        if (mob->pShop)
            mob->pShop->keeper = mob->vnum;

        db_register_new_mob (mob);
    }

    /* Link objs to areas and register them. */
    for (obj = obj_index_get_first(); obj != NULL;
         obj = obj_index_get_next(obj))
    {
        if ((area = db_link_areas_get_area (&(obj->area_str))) == NULL)
            continue;
        obj->area = area;
        obj->vnum = obj->anum + area->min_vnum;
        db_register_new_obj (obj);
    }

    /* Link help groups to areas. */
    for (had = had_get_first(); had != NULL; had = had_get_next(had)) {
        if ((area = db_link_areas_get_area (&(had->area_str))) == NULL)
            continue;
        area->helps = had;
        had->area = area;
    }
}

void db_register_new_room (ROOM_INDEX_T *room) {
    int vnum = room->vnum;
    int hash = vnum % MAX_KEY_HASH;

    LIST_FRONT (room, next, room_index_hash[hash]);
    top_vnum_room = top_vnum_room < vnum ? vnum : top_vnum_room; /* OLC */
    assign_area_vnum (vnum, room->area); /* OLC */
}

void db_register_new_mob (MOB_INDEX_T *mob) {
    int vnum = mob->vnum;
    int hash = vnum % MAX_KEY_HASH;

    LIST_FRONT (mob, next, mob_index_hash[hash]);
    top_vnum_mob = top_vnum_mob < vnum ? vnum : top_vnum_mob; /* OLC */
    assign_area_vnum (vnum, mob->area); /* OLC */
    kill_table[URANGE (0, mob->level, MAX_LEVEL - 1)].number++;
}

void db_register_new_obj (OBJ_INDEX_T *obj) {
    int vnum = obj->vnum;
    int hash = vnum % MAX_KEY_HASH;

    LIST_FRONT (obj, next, obj_index_hash[hash]);
    top_vnum_obj = top_vnum_obj < vnum ? vnum : top_vnum_obj; /* OLC */
    assign_area_vnum (vnum, obj->area); /* OLC */
}

void db_import_json (void) {
    JSON_T *json;
    int imported;

    /* read all files instead simple JSON objects. */
    imported = 0;
    log_f ("Loading JSON objects from '%s'...", JSON_DIR);
    json = json_read_directory_recursive (JSON_DIR,
        json_import_objects, &imported);

    /* reconstruct the world based on the JSON read. */
    log_f ("%d object(s) loaded successfully.", imported);

    /* free any leftover generated json. */
    if (json != NULL)
        json_free (json);

    /* link any objects with 'area_str' to their respective objects. */
    log_f ("Linking everything together...");
    db_link_areas ();
    portal_link_exits ();
}

int help_area_count_pages (HELP_AREA_T *had) {
    HELP_T *h;
    int count = 0;

    for (h = had->first; h != NULL; h = h->next_area)
        count++;
    return count;
}

void db_export_json (bool write_indiv, const char *everything) {
    JSON_T *jarea, *jgrp, *json;
    AREA_T *area;
    char buf[256], fbuf[256];

    /* Write all areas. */
    for (area = area_get_first(); area != NULL; area = area_get_next(area)) {
        snprintf (fbuf, sizeof(fbuf), "%s/", area->name);

        snprintf (buf, sizeof(buf), "areas/%s", area->name);
        jarea = json_root_area (buf);
        jgrp = json_prop_array (jarea, NULL);

        if (write_indiv)
            log_f("Exporting JSON: %sareas/%s*", JSON_DIR, fbuf);

        json = json_wrap_obj(json_new_obj_area (NULL, area), "area");
        json_attach_under (json, jgrp);

        if (write_indiv) {
            snprintf (buf, sizeof(buf), "%sareas/%sarea.json", JSON_DIR, fbuf);
            json_mkdir_to (buf);
            json_write_to_file (jgrp, buf);
        }

        /* NOTE: This is extremely nasty, but refactoring it into a function
         * or having copy-pasted code is even nastier. This is the least-worst
         * solution, IMO. -- Synival */
        #define ADD_AREA_JSON(oname, fname, btype, vtype, func) \
            do { \
                const vtype *obj; \
                jgrp = json_prop_array (jarea, NULL); \
                \
                for (obj = btype ## _get_first(); obj != NULL; \
                     obj = btype ## _get_next(obj)) \
                { \
                    if (obj->area != area) \
                        continue; \
                    if (obj->vnum < area->min_vnum || obj->vnum > area->max_vnum) \
                        bugf ("Warning: " #btype " #%d should be >= %d and <= %d", \
                            obj->vnum, area->min_vnum, area->max_vnum);\
                    json = json_wrap_obj (func (NULL, obj), oname); \
                    json_attach_under (json, jgrp); \
                } \
                if (jgrp->first_child && write_indiv) { \
                    snprintf (buf, sizeof (buf), "%sareas/%s" fname, JSON_DIR, fbuf); \
                    json_mkdir_to (buf); \
                    json_write_to_file (jgrp, buf); \
                } \
            } while (0)

        ADD_AREA_JSON ("room",   "rooms.json",   room_index, ROOM_INDEX_T, json_new_obj_room);
        ADD_AREA_JSON ("object", "objects.json", obj_index,  OBJ_INDEX_T,  json_new_obj_object);
        ADD_AREA_JSON ("mobile", "mobiles.json", mob_index,  MOB_INDEX_T,  json_new_obj_mobile);
    }

    /* Write json that doesn't need subdirectories. */
    #define ADD_META_JSON(oname, fname, btype, vtype, func, check) \
        jarea = json_root_area (fname); \
        if (write_indiv) \
            log_f("Exporting JSON: %s%s.json", JSON_DIR, fname); \
        do { \
            const vtype *obj; \
            \
            for (obj = btype ## _get_first(); obj != NULL; \
                 obj = btype ## _get_next(obj)) \
            { \
                if (!(check)) \
                    continue; \
                json = json_wrap_obj (func (NULL, obj), oname); \
                json_attach_under (json, jarea); \
            } \
            \
            if (jarea->first_child && write_indiv) { \
                snprintf (buf, sizeof (buf), "%s" fname ".json", JSON_DIR); \
                json_mkdir_to (buf); \
                json_write_to_file (jarea, buf); \
            } \
        } while (0)

    ADD_META_JSON ("social", "config/socials", social, SOCIAL_T,
        json_new_obj_social, 1);
    ADD_META_JSON ("portal", "config/portals", portal, PORTAL_T,
        json_new_obj_portal, (obj->generated == FALSE));

    ADD_META_JSON ("table", "meta/flags", master, TABLE_T,
        json_new_obj_table, ARE_SET (obj->flags, TABLE_FLAG_TYPE | TABLE_BITS));
    ADD_META_JSON ("table", "meta/types", master, TABLE_T,
        json_new_obj_table, IS_SET (obj->flags, TABLE_FLAG_TYPE) &&
                           !IS_SET (obj->flags, TABLE_BITS));
    ADD_META_JSON ("table", "meta/tables", master, TABLE_T,
        json_new_obj_table, !IS_SET (obj->flags, TABLE_FLAG_TYPE) &&
                             obj->json_write_func);

    /* Add help areas. */
    do {
        HELP_AREA_T *had;
        for (had = had_get_first(); had; had = had_get_next (had)) {
            snprintf (buf, sizeof(buf), "help/%s", had->name);
            jarea = json_root_area (buf);
            snprintf (fbuf, sizeof (fbuf), "%shelp/%s.json", JSON_DIR, had->name);

            if (write_indiv)
                log_f("Exporting JSON: %s", fbuf);

            json = json_wrap_obj (json_new_obj_help_area (NULL, had), "help");
            json_attach_under (json, jarea);

            if (write_indiv) {
                json_mkdir_to (fbuf);
                json_write_to_file (jarea, fbuf);
            }
        }
    } while (0);

    /* Write one giant file with everything. */
    if (everything) {
        log_f("Exporting JSON: %s", everything);
        json_write_to_file (json_root(), everything);
    }

    /* Free all allocated JSON. */
    json_free (json_root());
}

/* Big mama top level function. */
void boot_db (void) {
    HELP_T *help;

    /* Declare that we're booting the database. */
    fBootDb = TRUE;

    string_space_init ();
    init_mm ();
    init_time_weather ();
    init_gsns ();

#ifdef BASEMUD_IMPORT_JSON
    db_import_json ();
#else
    log_string ("Ignoring JSON files. "
        "Define BASEMUD_IMPORT_JSON in 'basemud.h' to enable.");
#endif
    init_areas ();

    EXIT_IF_BUGF ((help = help_get_by_name ("greeting")) == NULL,
        "boot_db(): Cannot find help entry 'greeting'.");
    help_greeting = help->text;

    fix_resets ();
    fix_exits ();
    portal_create_missing ();
    fix_mobprogs ();

    /* Boot process is over(?) */
    fBootDb = FALSE;
    convert_objects (); /* ROM OLC */

    area_update ();
    load_boards ();
    save_notes ();
    load_bans ();
    load_songs ();
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

void init_gsns (void) {
    int sn;
    for (sn = 0; sn < SKILL_MAX; sn++)
        if (skill_table[sn].pgsn != NULL)
            *skill_table[sn].pgsn = sn;
}

void init_areas (void) {
    FILE *fpList;
    char fname[MAX_STRING_LENGTH];

    if ((fpList = fopen (AREA_LIST, "r")) == NULL) {
        perror (AREA_LIST);
        exit (1);
    }

    while (1) {
        strcpy (strArea, fread_word (fpList));
        if (strArea[0] == '$')
            break;
        if (strArea[0] == '#')
            continue;

        if (strArea[0] == '-')
            fpArea = stdin;
        else {
            snprintf (fname, sizeof (fname), "%s%s", AREA_DIR, strArea);
            if (area_get_by_filename (strArea) != NULL) {
                log_f ("Ignoring loaded area '%s'", fname);
                continue;
            }
            if (help_area_get_by_filename (strArea) != NULL) {
                log_f ("Ignoring loaded help area '%s'", fname);
                continue;
            }

            if ((fpArea = fopen (fname, "r")) == NULL) {
                perror (fname);
                exit (1);
            }
        }

        current_area = NULL;
        while (1) {
            char *word;

            EXIT_IF_BUG (fread_letter (fpArea) != '#',
                "boot_db: # not found.", 0);

            word = fread_word (fpArea);
            if (word[0] == '$')
                break;

                 if (!str_cmp (word, "AREA"))     load_area (fpArea);
            else if (!str_cmp (word, "AREADATA")) load_area_olc (fpArea); /* OLC */
            else if (!str_cmp (word, "HELPS"))    load_helps (fpArea, strArea);
            else if (!str_cmp (word, "MOBOLD"))   load_old_mob (fpArea);
            else if (!str_cmp (word, "MOBILES"))  load_mobiles (fpArea);
            else if (!str_cmp (word, "MOBPROGS")) load_mobprogs (fpArea);
            else if (!str_cmp (word, "OBJOLD"))   load_old_obj (fpArea);
            else if (!str_cmp (word, "OBJECTS"))  load_objects (fpArea);
            else if (!str_cmp (word, "RESETS"))   load_resets (fpArea);
            else if (!str_cmp (word, "ROOMS"))    load_rooms (fpArea);
            else if (!str_cmp (word, "SHOPS"))    load_shops (fpArea);
            else if (!str_cmp (word, "SOCIALS"))  load_socials (fpArea);
            else if (!str_cmp (word, "SPECIALS")) load_specials (fpArea);
            else {
                EXIT_IF_BUG (TRUE,
                    "boot_db: bad section name.", 0);
            }
        }

        if (fpArea != stdin)
            fclose (fpArea);
        fpArea = NULL;
    }
    if (area_last)
        REMOVE_BIT (area_last->area_flags, AREA_LOADING); /* OLC */
    fclose (fpList);
}

/* Snarf an 'area' header line. */
void load_area (FILE *fp) {
    AREA_T *pArea, *pLast;

    pArea = area_new ();
    str_replace_dup (&pArea->filename, fread_string (fp));
    str_replace_dup (&pArea->name, trim_extension (pArea->filename));

    /* Pretty up the log a little */
    log_f("Loading area '%s'", pArea->filename);

    pArea->area_flags = AREA_LOADING;           /* OLC */
    pArea->security = 9;                        /* OLC 9 -- Hugin */
    str_replace_dup (&pArea->builders, "None"); /* OLC */
    pArea->vnum = TOP(RECYCLE_AREA_T);          /* OLC */

    str_replace_dup (&pArea->title,   fread_string (fp));
    str_replace_dup (&pArea->credits, fread_string (fp));
    pArea->min_vnum = fread_number (fp);
    pArea->max_vnum = fread_number (fp);
    pArea->age = 15;
    pArea->nplayer = 0;
    pArea->empty = FALSE;

    pLast = area_last;
    LISTB_BACK (pArea, next, area_first, area_last);
    if (pLast)
        REMOVE_BIT (pLast->area_flags, AREA_LOADING); /* OLC */
    current_area = pArea;
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
        str_free (&(field));         \
        (field) = fread_string (fp); \
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
    AREA_T *pArea;
    char *word;

    pArea = area_new ();
    pArea->age = 15;
    pArea->nplayer = 0;
    str_replace_dup (&pArea->filename, strArea);
    pArea->vnum = TOP(RECYCLE_AREA_T);
    str_replace_dup (&pArea->title, "New Area");
    str_replace_dup (&pArea->builders, "");
    pArea->security = 9;        /* 9 -- Hugin */
    pArea->min_vnum = 0;
    pArea->max_vnum = 0;
    pArea->area_flags = 0;
/*  pArea->recall       = ROOM_VNUM_TEMPLE;        ROM OLC */

    str_replace_dup (&pArea->name, trim_extension (pArea->filename));
    log_f("Loading area %s", pArea->filename);

    while (1) {
        word = feof (fp) ? "End" : fread_word (fp);
        switch (UPPER (word[0])) {
            case 'N':
                SKEY ("Name", pArea->title);
                break;

            case 'S':
                KEY ("Security", pArea->security, fread_number (fp));
                break;

            case 'V':
                if (!str_cmp (word, "VNUMs")) {
                    pArea->min_vnum = fread_number (fp);
                    pArea->max_vnum = fread_number (fp);
                }
                break;

            case 'E':
                if (!str_cmp (word, "End")) {
                    LISTB_BACK (pArea, next, area_first, area_last);
                    current_area = pArea;
                    return;
                }
                break;

            case 'B':
                SKEY ("Builders", pArea->builders);
                break;

            case 'C':
                SKEY ("Credits", pArea->credits);
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
    HELP_T *pHelp;
    int level;
    char *keyword;

    while (1) {
        HELP_AREA_T *had;

        level = fread_number (fp);
        keyword = fread_string (fp);

        if (keyword[0] == '$')
            break;

        if (!had_last || str_cmp (fname, had_last->filename)) {
            had = had_new ();
            str_replace_dup (&had->filename, fname);
            str_replace_dup (&had->name, trim_extension (had->filename));
            had->area = current_area;
            if (current_area)
                current_area->helps = had;
            LISTB_BACK (had, next, had_first, had_last);
        }
        else
            had = had_last;

        pHelp = help_new ();
        pHelp->level = level;
        str_replace_dup (&pHelp->keyword, keyword);
        str_replace_dup (&pHelp->text, fread_string (fp));

        LISTB_BACK (pHelp, next, help_first, help_last);
        LISTB_BACK (pHelp, next_area, had->first, had->last);
    }
}

void db_finalize_obj (OBJ_INDEX_T *obj) {
    switch (obj->item_type) {
        case ITEM_FURNITURE: {
            int min_occupants = 0;
            int min_hit       = 100;
            int min_mana      = 100;

            if (is_name ("tent", obj->name) ||
                is_name ("cabin", obj->name))
            {
                SET_BIT (obj->v.furniture.flags, REST_IN);
                SET_BIT (obj->v.furniture.flags, SIT_IN);
                SET_BIT (obj->v.furniture.flags, SLEEP_IN);
                SET_BIT (obj->v.furniture.flags, STAND_IN);
                min_occupants = 1;
                min_hit = 250;
            }
            if (is_name ("bed", obj->name)) {
                SET_BIT (obj->v.furniture.flags, REST_ON);
                SET_BIT (obj->v.furniture.flags, SIT_ON);
                SET_BIT (obj->v.furniture.flags, SLEEP_IN);
                min_occupants = 1;
                min_hit = 200;
            }
            if (is_name ("sofa", obj->name) ||
                is_name ("couch", obj->name))
            {
                SET_BIT (obj->v.furniture.flags, REST_ON);
                SET_BIT (obj->v.furniture.flags, SIT_ON);
                SET_BIT (obj->v.furniture.flags, SLEEP_ON);
                min_occupants = 1;
                min_hit = 150;
            }
            if (is_name ("bench", obj->name)) {
                SET_BIT (obj->v.furniture.flags, REST_ON);
                SET_BIT (obj->v.furniture.flags, SIT_ON);
                SET_BIT (obj->v.furniture.flags, SLEEP_ON);
                min_occupants = 1;
                min_hit = 125;
            }
            if (is_name ("chair", obj->name) ||
                is_name ("stool", obj->name))
            {
                SET_BIT (obj->v.furniture.flags, STAND_ON);
                SET_BIT (obj->v.furniture.flags, REST_ON);
                SET_BIT (obj->v.furniture.flags, SIT_ON);
                SET_BIT (obj->v.furniture.flags, SLEEP_ON);
                min_occupants = 1;
                min_hit = 125;
            }

            if (obj->v.furniture.max_people < min_occupants)
                obj->v.furniture.max_people = min_occupants;
            if (obj->v.furniture.heal_rate < min_hit)
                obj->v.furniture.heal_rate = min_hit;
            if (obj->v.furniture.mana_rate < min_mana)
                obj->v.furniture.mana_rate = min_mana;
            break;
        }
    }
}

/* Snarf a reset section. */
void load_resets (FILE *fp) {
    RESET_T *pReset;
    RESET_VALUE_T *v;
    int rVnum = -1;

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

        pReset = reset_data_new ();
        pReset->area = area_last;
        pReset->command = letter;
        v = &(pReset->v);

        switch (pReset->command) {
            case 'M':
                v->mob._value1      = fread_number (fp);
                v->mob.mob_vnum     = fread_number (fp);
                v->mob.global_limit = fread_number (fp);
                v->mob.room_vnum    = fread_number (fp);
                v->mob.room_limit   = fread_number (fp);
                fread_to_eol (fp);
                rVnum = v->mob.room_vnum;
                break;

            case 'O':
                v->obj.room_limit   = fread_number (fp);
                v->obj.obj_vnum     = fread_number (fp);
                v->obj.global_limit = fread_number (fp);
                v->obj.room_vnum    = fread_number (fp);
                v->obj._value5      = 0;
                fread_to_eol (fp);
                rVnum = v->obj.room_vnum;
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
                rVnum = v->door.room_vnum;
                break;

            case 'R':
                v->randomize._value1   = fread_number (fp);
                v->randomize.room_vnum = fread_number (fp);
                v->randomize.dir_count = fread_number (fp);
                v->randomize._value4   = 0;
                v->randomize._value5   = 0;
                fread_to_eol (fp);
                rVnum = v->randomize.room_vnum;
                break;
        }

        EXIT_IF_BUG (rVnum == -1,
            "load_resets: rVnum == -1", 0);
        pReset->room_vnum = rVnum;
    }
}

void fix_resets (void) {
    RESET_T *pReset, *next;
    RESET_VALUE_T *v;
    EXIT_T *pexit;
    ROOM_INDEX_T *pRoomIndex;
    bool fail;

    for (pReset = reset_data_get_first(); pReset != NULL; pReset = next) {
        next = reset_data_get_next (pReset);
        v = &(pReset->v);
        fail = FALSE;

        switch (pReset->command) {
            case 'D':
                pRoomIndex = get_room_index (v->door.room_vnum);
                if (v->door.dir < 0 ||
                    v->door.dir >= DIR_MAX ||
                    !pRoomIndex ||
                    !(pexit = pRoomIndex->exit[v->door.dir]) ||
                    !IS_SET (pexit->rs_flags, EX_ISDOOR))
                {
                    bugf ("fix_resets: 'D': exit %d, room %d not door.",
                          v->door.dir, v->door.room_vnum);
                    fail = TRUE;
                    break;
                }

                switch (v->door.locks) {
                    case RESET_DOOR_NONE:
                        break;
                    case RESET_DOOR_CLOSED:
                        SET_BIT (pexit->exit_flags, EX_CLOSED);
                        break;
                    case RESET_DOOR_LOCKED:
                        SET_BIT (pexit->exit_flags, EX_CLOSED | EX_LOCKED);
                        break;
                    default:
                        bug ("load_resets: 'D': bad 'locks': %d.",
                             v->door.locks);
                        break;
                }
                pexit->rs_flags = pexit->exit_flags;
                break;
        }

        /* Remove broken resets. */
        if (fail == TRUE) {
            reset_data_free (pReset);
            continue;
        }

        /* Door resets are removed - all others belong to their room. */
        if (pReset->command == 'D') {
            reset_data_free (pReset);
            continue;
        }

        /* Complain if resets don't belong to any particular room. */
        if ((pRoomIndex = get_room_index (pReset->room_vnum)) == NULL) {
            bugf ("fix_resets: '%c': room %d does not exist.",
                pReset->command, pReset->room_vnum);
            reset_data_free (pReset);
            continue;
        }

        /* Assign the reset to the room. */
        room_take_reset (pRoomIndex, pReset);
    }
}

/* Snarf a room section. */
void load_rooms (FILE *fp) {
    ROOM_INDEX_T *pRoomIndex;

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

        fBootDb = FALSE;
        EXIT_IF_BUG (get_room_index (vnum) != NULL,
            "load_rooms: vnum %d duplicated.", vnum);
        fBootDb = TRUE;

        pRoomIndex = room_index_new ();
        str_replace_dup (&pRoomIndex->owner, "");
        pRoomIndex->people      = NULL;
        pRoomIndex->contents    = NULL;
        pRoomIndex->extra_descr = NULL;
        pRoomIndex->area        = area_last;
        pRoomIndex->vnum        = vnum;
        pRoomIndex->anum        = vnum - area_last->min_vnum;

        str_replace_dup (&pRoomIndex->name, fread_string (fp));
        str_replace_dup (&pRoomIndex->description, fread_string (fp));
        /* Area number */ fread_number (fp);
        pRoomIndex->room_flags = fread_flag (fp);
        /* horrible hack */
        if (3000 <= vnum && vnum < 3400)
            SET_BIT (pRoomIndex->room_flags, ROOM_LAW);
        pRoomIndex->sector_type = fread_number (fp);
        pRoomIndex->light = 0;
        for (door = 0; door < DIR_MAX; door++)
            pRoomIndex->exit[door] = NULL;

        /* defaults */
        pRoomIndex->heal_rate = 100;
        pRoomIndex->mana_rate = 100;

        while (1) {
            letter = fread_letter (fp);
            if (letter == 'S')
                break;
            if (letter == 'H')    /* healing room */
                pRoomIndex->heal_rate = fread_number (fp);
            else if (letter == 'M') /* mana room */
                pRoomIndex->mana_rate = fread_number (fp);
            else if (letter == 'C') { /* clan */
                char *clan_str = fread_string (fp);
                EXIT_IF_BUG (pRoomIndex->clan != 0,
                    "load_rooms: duplicate clan fields.", 0);
                pRoomIndex->clan = lookup_backup (clan_lookup_exact, clan_str,
                    "Unknown clan '%s'", 0);
            }
            else if (letter == 'D') {
                EXIT_T *pexit;
                int locks;

                door = fread_number (fp);
                EXIT_IF_BUG (door < 0 || door > 5,
                    "load_rooms: vnum %d has bad door number.", vnum);

                pexit = exit_new ();
                str_replace_dup (&pexit->description, fread_string (fp));
                str_replace_dup (&pexit->keyword,     fread_string (fp));
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
                        if (pexit->key >= KEY_VALID)
                            bugf ("Warning: Room %d with non-door exit %d has "
                                  "key %d", pRoomIndex->vnum, door, pexit->key);
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
                pRoomIndex->exit[door] = pexit;
            }
            else if (letter == 'E') {
                EXTRA_DESCR_T *ed = extra_descr_new ();
                str_replace_dup (&ed->keyword,     fread_string (fp));
                str_replace_dup (&ed->description, fread_string (fp));
                LIST_BACK (ed, next, pRoomIndex->extra_descr, EXTRA_DESCR_T);
            }
            else if (letter == 'O') {
                EXIT_IF_BUG (pRoomIndex->owner[0] != '\0',
                    "load_rooms: duplicate owner.", 0);
                str_replace_dup (&pRoomIndex->owner, fread_string (fp));
            }
            else {
                EXIT_IF_BUG (TRUE,
                    "load_rooms: vnum %d has flag not 'DES'.", vnum);
            }
        }

        db_register_new_room (pRoomIndex);
    }
}

/* Snarf a shop section. */
void load_shops (FILE *fp) {
    SHOP_T *pShop;
    int keeper = 0;

    while (1) {
        MOB_INDEX_T *pMobIndex;
        int iTrade;

        /* ROM mem leak fix, check the keeper before allocating the memory
         * to the SHOP_T variable.  -Rhien */
        keeper = fread_number(fp);
        if (keeper == 0)
            break;

        /* Now that we have a non zero keeper number we can allocate */
        pShop = shop_new ();
        pShop->keeper = keeper;

        for (iTrade = 0; iTrade < MAX_TRADE; iTrade++)
            pShop->buy_type[iTrade] = fread_number(fp);

        pShop->profit_buy  = fread_number(fp);
        pShop->profit_sell = fread_number(fp);
        pShop->open_hour   = fread_number(fp);
        pShop->close_hour  = fread_number(fp);
        fread_to_eol(fp);
        pMobIndex = get_mob_index(pShop->keeper);
        pMobIndex->pShop = pShop;

        LISTB_BACK (pShop, next, shop_first, shop_last);
    }
}

/* Snarf spec proc declarations. */
void load_specials (FILE *fp) {
    char *func_name;
    while (1) {
        MOB_INDEX_T *pMobIndex;
        char letter;

        switch (letter = fread_letter (fp)) {
            case 'S':
                return;

            case '*':
                break;

            case 'M':
                pMobIndex = get_mob_index (fread_number (fp));
                func_name = fread_word (fp);
                pMobIndex->spec_fun = spec_lookup_function (func_name);
                EXIT_IF_BUGF (pMobIndex->spec_fun == NULL,
                    "Unknown special function '%s'", func_name);
                break;

            default:
                EXIT_IF_BUG (TRUE,
                    "load_specials: letter '%c' not *MS.", letter);
        }

        fread_to_eol (fp);
    }
}

void fix_exit_doors (ROOM_INDEX_T *room_from, int dir_from,
                     ROOM_INDEX_T *room_to,   int dir_to)
{
    EXIT_T *exit_from = room_from->exit[dir_from];
    EXIT_T *exit_to   = room_to  ->exit[dir_to];
    flag_t flags_from = exit_from->exit_flags;
    flag_t flags_to   = exit_to  ->exit_flags;

    if (dir_to != REV_DIR(dir_from))
        return;
    if (exit_from->to_room != room_to || exit_to->to_room != room_from)
        return;

    if (IS_SET(flags_from, EX_ISDOOR) && !IS_SET(flags_to, EX_ISDOOR)) {
        bugf ("Warning: Exit %d[%s, %s] is a door but %d[%s, %s] is not.",
            room_from->vnum, door_get_name (dir_from), exit_from->keyword,
            room_to  ->vnum, door_get_name (dir_to),   exit_to  ->keyword);
        return;
    }

    /* We only care about doors from now on. */
    if (!(IS_SET(flags_from, EX_ISDOOR) && IS_SET(flags_to, EX_ISDOOR)))
        return;

    /* Doors closed or locked on one side should definitely be
     * on the other. Make sure this is the case. */
    if (IS_SET (flags_from, EX_CLOSED) || IS_SET (flags_to, EX_CLOSED)) {
        SET_BIT (exit_from->exit_flags, EX_CLOSED);
        SET_BIT (exit_from->rs_flags,   EX_CLOSED);
        SET_BIT (exit_to->exit_flags,   EX_CLOSED);
        SET_BIT (exit_to->rs_flags,     EX_CLOSED);
    }
    if (IS_SET (flags_from, EX_LOCKED) || IS_SET (flags_to, EX_LOCKED)) {
        SET_BIT (exit_from->exit_flags, EX_LOCKED);
        SET_BIT (exit_from->rs_flags,   EX_LOCKED);
        SET_BIT (exit_to->exit_flags,   EX_LOCKED);
        SET_BIT (exit_to->rs_flags,     EX_LOCKED);
    }

    /* Different keys is bad! But we don't care if one side is pick-proof
     * and the other isn't. */
    if (exit_from->key != exit_to->key &&
        exit_from->key != KEY_NOKEYHOLE &&
        exit_to->key   != KEY_NOKEYHOLE)
    {
        bugf ("Warning: Exits %d[%s] and %d[%s] have "
            "different keys [%d, %d]",
            room_from->vnum, door_get_name (dir_from),
            room_to  ->vnum, door_get_name (dir_to),
            exit_from->key, exit_to->key);
    }
}

/* Translate all room exits from virtual to real.
 * Has to be done after all rooms are read in.
 * Check for bad reverse exits. */
void fix_exits (void) {
    ROOM_INDEX_T *pRoomIndex;
    ROOM_INDEX_T *to_room;
    EXIT_T *pexit;
    EXIT_T *pexit_rev;
    RESET_T *pReset;
    RESET_VALUE_T *v;
    ROOM_INDEX_T *iLastRoom, *iLastObj;
    OBJ_INDEX_T *key;
    int iHash;
    int door, oldBootDb;

    for (iHash = 0; iHash < MAX_KEY_HASH; iHash++) {
        for (pRoomIndex = room_index_hash[iHash];
             pRoomIndex != NULL; pRoomIndex = pRoomIndex->next)
        {
            bool fexit;

            iLastRoom = iLastObj = NULL;

            /* OLC : new check of resets */
            for (pReset = pRoomIndex->reset_first; pReset;
                 pReset = pReset->next)
            {
                v = &(pReset->v);
                switch (pReset->command) {
                    case 'M':
                        get_mob_index (v->mob.mob_vnum);
                        iLastRoom = get_room_index (v->mob.room_vnum);
                        break;

                    case 'O':
                        get_obj_index (v->obj.obj_vnum);
                        iLastObj = get_room_index (v->obj.room_vnum);
                        break;

                    case 'P':
                        get_obj_index (v->obj.obj_vnum);
                        EXIT_IF_BUG (iLastObj == NULL,
                            "fix_exits: reset 'P' in room %d with iLastObj NULL",
                            pRoomIndex->vnum);
                        break;

                    case 'G':
                        get_obj_index (v->give.obj_vnum);
                        EXIT_IF_BUG (iLastRoom == NULL,
                            "fix_exits: reset 'G' in room %d with iLastRoom NULL",
                            pRoomIndex->vnum);
                        iLastObj = iLastRoom;
                        break;

                    case 'E':
                        get_obj_index (v->equip.obj_vnum);
                        EXIT_IF_BUG (iLastRoom == NULL,
                            "fix_exits: reset 'E' in room %d with iLastRoom NULL",
                            pRoomIndex->vnum);
                        iLastObj = iLastRoom;
                        break;

                    case 'D':
                        bugf ("???");
                        break;

                    case 'R': {
                        get_room_index (v->randomize.room_vnum);
                        int dir_count = v->randomize.dir_count;
                        EXIT_IF_BUGF (dir_count < 0,
                            "fix_exits: reset 'R' in room %d with dir_count %d < 0",
                            pRoomIndex->vnum, dir_count);
                        EXIT_IF_BUGF (dir_count > DIR_MAX,
                            "fix_exits: reset 'R' in room %d with dir_count %d > DIR_MAX",
                            pRoomIndex->vnum, dir_count);
                        break;
                    }

                    default:
                        EXIT_IF_BUGF (TRUE,
                            "fix_exits: room %d with reset cmd %c",
                            pRoomIndex->vnum, pReset->command);
                        break;
                }
            }

            fexit = FALSE;
            for (door = 0; door < DIR_MAX; door++) {
                if ((pexit = pRoomIndex->exit[door]) == NULL)
                    continue;
                if (pexit->vnum <= 0 || get_room_index (pexit->vnum) == NULL)
                    pexit->to_room = NULL;
                else {
                    fexit = TRUE;
                    pexit->to_room = get_room_index (pexit->vnum);
                    pexit->area_vnum = pexit->to_room->area->vnum;
                    pexit->room_anum = pexit->to_room->anum;
                }

                oldBootDb = fBootDb;
                fBootDb = FALSE;
                if (pexit->key >= KEY_VALID &&
                    !(key = get_obj_index (pexit->key)))
                {
                    bugf ("Warning: Cannot find key %d for room %d exit %d",
                        pexit->key, pRoomIndex->vnum, door);
                }
                fBootDb = oldBootDb;
            }
            if (!fexit)
                SET_BIT (pRoomIndex->room_flags, ROOM_NO_MOB);
        }
    }

    for (iHash = 0; iHash < MAX_KEY_HASH; iHash++) {
        for (pRoomIndex = room_index_hash[iHash];
             pRoomIndex != NULL; pRoomIndex = pRoomIndex->next)
        {
            for (door = 0; door < DIR_MAX; door++) {
                int rev_dir;
                if ((pexit = pRoomIndex->exit[door]) == NULL)
                    continue;
                if ((to_room = pexit->to_room) == NULL)
                    continue;
                rev_dir = REV_DIR(door);
                if ((pexit_rev = to_room->exit[rev_dir]) == NULL)
                    continue;

                fix_exit_doors (pRoomIndex, door, to_room, rev_dir);

                /* If the reverse exit does not lead to the same exception,
                 * print a warning. Make an exception for the 'immort.are'
                 * zone. */
                if (pexit_rev->to_room != pRoomIndex &&
                    (pRoomIndex->vnum < 1200 || pRoomIndex->vnum > 1299))
                {
                    bugf ("Warning: Exit %d[%s] reverse exit %d[%s] "
                          "leads to wrong room (%d).",
                        pRoomIndex->vnum, door_get_name(door),
                        to_room->vnum,    door_get_name(rev_dir),
                        (pexit_rev->to_room == NULL)
                             ? 0 : pexit_rev->to_room->vnum
                    );
                }
            }
        }
    }
}

/* Load mobprogs section */
void load_mobprogs (FILE *fp) {
    MPROG_CODE_T *pMprog;

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

        fBootDb = FALSE;
        EXIT_IF_BUG (get_mprog_index (vnum) != NULL,
            "load_mobprogs: vnum %d duplicated.", vnum);
        fBootDb = TRUE;

        pMprog = mpcode_new ();
        pMprog->area = area_last;
        pMprog->vnum = vnum;
        pMprog->anum = vnum - area_last->min_vnum;
        str_replace_dup (&pMprog->code, fread_string (fp));
        LIST_FRONT (pMprog, next, mprog_list);
    }
}

/* Translate mobprog vnums pointers to real code */
void fix_mobprogs (void) {
    MOB_INDEX_T *pMobIndex;
    MPROG_LIST_T *list;
    MPROG_CODE_T *prog;
    int iHash;

    for (iHash = 0; iHash < MAX_KEY_HASH; iHash++) {
        for (pMobIndex = mob_index_hash[iHash];
             pMobIndex != NULL; pMobIndex = pMobIndex->next)
        {
            for (list = pMobIndex->mprogs; list != NULL; list = list->next) {
                EXIT_IF_BUG ((prog = get_mprog_index (list->vnum)) == NULL,
                    "fix_mobprogs: code vnum %d not found.", list->vnum);
                list->code = prog->code;
            }
        }
    }
}

/* OLC
 * Reset one room.  Called by reset_area and olc. */
static CHAR_T *reset_last_mob = NULL;
static OBJ_T  *reset_last_obj = NULL;
static bool    reset_last_created = FALSE;

int reset_room_reset_shop_obj_level (OBJ_INDEX_T *pObjIndex) {
    if (pObjIndex->new_format)
        return 0;

    switch (pObjIndex->item_type) {
        case ITEM_PILL:
        case ITEM_POTION:
        case ITEM_SCROLL: {
            int olevel = 53, skill_max, i, j;
            flag_t *skill = NULL;

            switch (pObjIndex->item_type) {
                case ITEM_PILL:
                    skill = pObjIndex->v.pill.skill;
                    skill_max = PILL_SKILL_MAX;
                    break;
                case ITEM_POTION:
                    skill = pObjIndex->v.potion.skill;
                    skill_max = POTION_SKILL_MAX;
                    break;
                case ITEM_SCROLL:
                    skill = pObjIndex->v.scroll.skill;
                    skill_max = SCROLL_SKILL_MAX;
                    break;
                default:
                    return 0;
            }

            for (i = 0; i < skill_max; i++) {
                int sk = skill[i];
                if (sk <= 0 || sk >= SKILL_MAX)
                    continue;
                for (j = 0; j < CLASS_MAX; j++)
                    olevel = UMIN (olevel, skill_table[sk].skill_level[j]);
            }
            return UMAX (0, (olevel * 3 / 4) - 2);
        }

        case ITEM_WAND:     return number_range (10, 20);
        case ITEM_STAFF:    return number_range (15, 25);
        case ITEM_ARMOR:    return number_range (5, 15);

        /* ROM patch weapon, treasure */
        case ITEM_WEAPON:   return number_range (5, 15);
        case ITEM_TREASURE: return number_range (10, 20);

#if 0 /* envy version */
        case ITEM_WEAPON:
            if (pReset->command == 'G')
                olevel = number_range (5, 15);
            else
                olevel = number_fuzzy (level);
#endif /* envy version */

        default:
            return 0;
    }
}

void reset_room_reset (ROOM_INDEX_T *pRoom, RESET_T *pReset) {
    MOB_INDEX_T *pMobIndex;
    OBJ_INDEX_T *pObjIndex;
    OBJ_INDEX_T *pObjToIndex;
    ROOM_INDEX_T *pRoomIndex;
    ROOM_INDEX_T *pRoomIndexPrev;
    CHAR_T *pMob;
    OBJ_T *pObj;
    int level = 0;
    int count, limit;

    switch (pReset->command) {
        case 'M':
            BAIL_IF_BUG (!(pMobIndex = get_mob_index (pReset->v.mob.mob_vnum)),
                "reset_room: 'M': bad mob_vnum %d.", pReset->v.mob.mob_vnum);
            BAIL_IF_BUG (!(pRoomIndex = get_room_index (pReset->v.mob.room_vnum)),
                "reset_room: 'M': bad room_vnum %d.", pReset->v.mob.room_vnum);

            if (pMobIndex->count >= pReset->v.mob.global_limit) {
                reset_last_created = FALSE;
                return;
            }

            count = 0;
            for (pMob = pRoomIndex->people; pMob != NULL;
                 pMob = pMob->next_in_room)
            {
                if (pMob->pIndexData != pMobIndex)
                    continue;
                if (++count >= pReset->v.mob.room_limit)
                    break;
            }
            if (count >= pReset->v.mob.room_limit) {
                reset_last_created = FALSE;
                return;
            }

            pMob = char_create_mobile (pMobIndex);

            /* Some more hard coding. */
            if (room_is_dark (pRoomIndex))
                SET_BIT (pMob->affected_by, AFF_INFRARED);

            /* Pet shop mobiles get ACT_PET set. */
            pRoomIndexPrev = get_room_index (pRoomIndex->vnum - 1);
            if (pRoomIndexPrev
                && IS_SET (pRoomIndexPrev->room_flags, ROOM_PET_SHOP))
                SET_BIT (pMob->mob, MOB_PET);

            char_to_room (pMob, pRoomIndex);
            reset_last_mob = pMob;
            level = URANGE (0, pMob->level - 2, LEVEL_HERO - 1); /* -1 ROM */
            reset_last_created = TRUE;
            break;

        case 'O': {
            int obj_count = UMAX(1, pReset->v.obj.room_limit);

            if (!(pObjIndex = get_obj_index (pReset->v.obj.obj_vnum))) {
                bug ("reset_room: 'O': bad obj_vnum %d.", pReset->v.obj.obj_vnum);
                bugf ("%d %d %d %d",
                    pReset->v.value[1], pReset->v.value[2],
                    pReset->v.value[3], pReset->v.value[4]);
                return;
            }
            if (!(pRoomIndex = get_room_index (pReset->v.obj.room_vnum))) {
                bug ("reset_room: 'O': bad room_vnum %d.", pReset->v.obj.room_vnum);
                bugf ("%d %d %d %d",
                    pReset->v.value[1], pReset->v.value[2],
                    pReset->v.value[3], pReset->v.value[4]);
                return;
            }

            if (pRoomIndex->area->nplayer > 0 ||
                obj_index_count_in_list (pObjIndex, pRoomIndex->contents)
                    >= obj_count)
            {
                reset_last_created = FALSE;
                return;
            }

            pObj = obj_create (pObjIndex,    /* UMIN - ROM OLC */
               UMIN (number_fuzzy (level), LEVEL_HERO - 1));
            if (pObj->item_type != ITEM_TREASURE)
                pObj->cost = 0;
            obj_to_room (pObj, pRoomIndex);
            reset_last_created = TRUE;
            break;
        }

        case 'P':
            BAIL_IF_BUG (!(pObjIndex = get_obj_index (pReset->v.put.obj_vnum)),
                "reset_room: 'P': bad obj_vnum %d.", pReset->v.put.obj_vnum);
            BAIL_IF_BUG (!(pObjToIndex = get_obj_index (pReset->v.put.into_vnum)),
                "reset_room: 'P': bad into_vnum %d.", pReset->v.put.into_vnum);
            BAIL_IF_BUG (!(pObjToIndex->item_type == ITEM_CONTAINER ||
                           pObjToIndex->item_type == ITEM_CORPSE_NPC ||
                           pObjToIndex->item_type == ITEM_CORPSE_PC),
                "reset_room: 'P': to object %d is not a container.",
                    pReset->v.put.into_vnum);

            if (pReset->v.put.global_limit > 50) /* old format */
                limit = 6;
            else if (pReset->v.put.global_limit == -1) /* no limit */
                limit = 999;
            else
                limit = pReset->v.put.global_limit;

            if (pRoom->area->nplayer > 0                           ||
                (reset_last_obj = obj_get_by_index (pObjToIndex)) == NULL ||
                (reset_last_obj->in_room == NULL && !reset_last_created)    ||
                (pObjIndex->count >= limit)                        ||
                (count = obj_index_count_in_list (pObjIndex,
                    reset_last_obj->contains)) > pReset->v.put.put_count)
            {
                reset_last_created = FALSE;
                return;
            }

            while (count < pReset->v.put.put_count) {
                pObj = obj_create (pObjIndex,
                    number_fuzzy (reset_last_obj->level));
                obj_to_obj (pObj, reset_last_obj);
                if (++pObjIndex->count >= limit)
                    break;
            }

            /* fix object lock state! */
            reset_last_obj->v.container.flags =
                reset_last_obj->pIndexData->v.container.flags;
            reset_last_created = TRUE;
            break;

        case 'G':
        case 'E': {
            struct reset_values_give  *gv = &(pReset->v.give);
            struct reset_values_equip *eq = &(pReset->v.equip);
            char cmd = pReset->command;
            int obj_vnum, global_limit;

            obj_vnum     = (cmd == 'G') ? gv->obj_vnum     : eq->obj_vnum;
            global_limit = (cmd == 'G') ? gv->global_limit : eq->global_limit;

            BAIL_IF_BUG (!(pObjIndex = get_obj_index (obj_vnum)),
                "reset_room: 'E' or 'G': bad obj_vnum %d.", obj_vnum);

            /* If we haven't instantiated the previous mob (at least we
             * ASSUME the command was 'M'), don't give or equip. */
            if (!reset_last_created)
                return;

            if (!reset_last_mob) {
                reset_last_created = FALSE;
                bug ("reset_room: 'E' or 'G': null mob for vnum %d.",
                    obj_vnum);
                return;
            }

            /* Show warnings for items in bad slots. */
            if (cmd == 'E') {
                flag_t wear_flag = wear_get_type_by_loc (eq->wear_loc);
                if (!obj_index_can_wear_flag (pObjIndex, wear_flag)) {
                    bugf ("Warning: 'E' for object %d (%s) into unequippable "
                          "location '%s'.",
                        obj_vnum, pObjIndex->short_descr,
                        wear_loc_name (eq->wear_loc));
                }
                if (!char_has_available_wear_loc (reset_last_mob, eq->wear_loc)) {
                    bugf ("Warning: 'E' for object %d (%s) on mob %d (%s), "
                          "wear loc '%s' is already taken.",
                        obj_vnum, pObjIndex->short_descr,
                        reset_last_mob->pIndexData->vnum, reset_last_mob->short_descr,
                        wear_loc_name (eq->wear_loc));
                }
            }

            /* Shop-keeper? */
            if (reset_last_mob->pIndexData->pShop) {
                int olevel = reset_room_reset_shop_obj_level (pObjIndex);
                pObj = obj_create (pObjIndex, olevel);
                SET_BIT (pObj->extra_flags, ITEM_INVENTORY); /* ROM OLC */

#if 0 /* envy version */
                if (pReset->command == 'G')
                    SET_BIT (pObj->extra_flags, ITEM_INVENTORY);
#endif /* envy version */
            }
            /* ROM OLC else version */
            else {
                int limit;
                const char *warn_type;

                if (global_limit > 50) /* old format */
                    limit = 6;
                else if (global_limit == -1 || global_limit == 0) /* no limit */
                    limit = 999;
                else
                    limit = global_limit;

                /* Allow going over the global item limit 25% of the time. */
                if (pObjIndex->count >= limit && number_range (0, 4) != 0)
                    break;

                pObj = obj_create (pObjIndex,
                    UMIN (number_fuzzy (level), LEVEL_HERO - 1));

                /* Warnings for various bad ideas. */
                if (pObj->level > reset_last_mob->level + 3)
                    warn_type = "too strong";
                else if (
                    pObj->item_type == ITEM_WEAPON   &&
                    pReset->command == 'E'           &&
                    pObj->level < reset_last_mob->level - 5 &&
                    pObj->level < 45)
                {
                    warn_type = "too weak";
                }
                else
                    warn_type = NULL;

                /* Was a warning found? */
                if (warn_type != NULL) {
                    log_f ("%4s - %s: ", (cmd == 'G') ? "Give" : "Equip",
                        warn_type);
                    log_f ("  Object: (VNUM %5d)(Level %2d) %s",
                        pObj->pIndexData->vnum,
                        pObj->level,
                        pObj->short_descr);
                    log_f ("     Mob: (VNUM %5d)(Level %2d) %s",
                        reset_last_mob->pIndexData->vnum,
                        reset_last_mob->level,
                        reset_last_mob->short_descr);
                }
            }

            obj_to_char (pObj, reset_last_mob);
            if (cmd == 'E')
                char_equip_obj (reset_last_mob, pObj, eq->wear_loc);
            reset_last_created = TRUE;
            break;
        }

        case 'D':
            break;

        case 'R': {
            EXIT_T *pExit;
            int d1, d2;

            BAIL_IF_BUG (!(pRoomIndex = get_room_index (pReset->v.randomize.room_vnum)),
                "reset_room: 'R': bad vnum %d.", pReset->v.randomize.room_vnum);

            for (d1 = 0; d1 < pReset->v.randomize.dir_count - 1; d1++) {
                d2 = number_range (d1, pReset->v.randomize.dir_count - 1);
                pExit = pRoomIndex->exit[d1];
                pRoomIndex->exit[d1] = pRoomIndex->exit[d2];
                pRoomIndex->exit[d2] = pExit;
            }
            break;
        }

        default:
            bug ("reset_room: bad command %c.", pReset->command);
            break;
    }
}

void reset_room (ROOM_INDEX_T *pRoom) {
    RESET_T *pReset;
    int iExit;

    if (!pRoom)
        return;

    reset_last_mob = NULL;
    reset_last_obj = NULL;
    reset_last_created = FALSE;

    /* Reset exits. */
    for (iExit = 0; iExit < DIR_MAX; iExit++) {
        EXIT_T *pExit, *pRev;
        if ((pExit = pRoom->exit[iExit]) == NULL)
            continue;
     /* if (IS_SET (pExit->exit_flags, EX_BASHED))
            continue; */
        pExit->exit_flags = pExit->rs_flags;

        if (pExit->to_room == NULL)
            continue;
        if ((pRev = pExit->to_room->exit[door_table[iExit].reverse]) == NULL)
            continue;
        pRev->exit_flags = pRev->rs_flags;
    }

    /* Perform all other room resets. */
    for (pReset = pRoom->reset_first; pReset != NULL; pReset = pReset->next)
        reset_room_reset (pRoom, pReset);
}

/* OLC
 * Reset one area. */
void reset_area (AREA_T *pArea) {
    ROOM_INDEX_T *pRoom;
    int vnum;
    for (vnum = pArea->min_vnum; vnum <= pArea->max_vnum; vnum++)
        if ((pRoom = get_room_index (vnum)))
            reset_room (pRoom);
}

/* Clear a new character. */
void clear_char (CHAR_T *ch) {
    static CHAR_T ch_zero;
    int i;

    *ch = ch_zero;
    ch->name        = &str_empty[0];
    ch->short_descr = &str_empty[0];
    ch->long_descr  = &str_empty[0];
    ch->description = &str_empty[0];
    ch->prompt      = &str_empty[0];
    ch->logon       = current_time;
    ch->lines       = PAGELEN;
    for (i = 0; i < 4; i++)
        ch->armor[i] = 100;
    ch->position    = POS_STANDING;
    ch->hit         = 20;
    ch->max_hit     = 20;
    ch->mana        = 100;
    ch->max_mana    = 100;
    ch->move        = 100;
    ch->max_move    = 100;
    ch->on          = NULL;
    for (i = 0; i < STAT_MAX; i++) {
        ch->perm_stat[i] = 13;
        ch->mod_stat[i] = 0;
    }
}

/* Get an extra description from a list. */
char *get_extra_descr (const char *name, EXTRA_DESCR_T *ed) {
    for (; ed != NULL; ed = ed->next)
        if (is_name ((char *) name, ed->keyword))
            return ed->description;
    return NULL;
}

/* Translates mob virtual number to its mob index struct.
 * Hash table lookup. */
MOB_INDEX_T *get_mob_index (int vnum) {
    MOB_INDEX_T *pMobIndex;

    for (pMobIndex = mob_index_hash[vnum % MAX_KEY_HASH];
         pMobIndex != NULL; pMobIndex = pMobIndex->next)
        if (pMobIndex->vnum == vnum)
            return pMobIndex;

    /* NOTE: This did cause the server not to boot, but that seems a bit
     *       extreme. So we just return NULL instead, and keep on booting.
     *       -- Synival */
    if (fBootDb) {
        bug ("get_mob_index: bad vnum %d.", vnum);
     // exit (1);
    }

    return NULL;
}

/* Translates mob virtual number to its obj index struct.
 * Hash table lookup. */
OBJ_INDEX_T *get_obj_index (int vnum) {
    OBJ_INDEX_T *pObjIndex;

    for (pObjIndex = obj_index_hash[vnum % MAX_KEY_HASH];
         pObjIndex != NULL; pObjIndex = pObjIndex->next)
        if (pObjIndex->vnum == vnum)
            return pObjIndex;

    /* NOTE: This did cause the server not to boot, but that seems a bit
     *       extreme. So we just return NULL instead, and keep on booting.
     *       -- Synival */
    if (fBootDb) {
        bug ("get_obj_index: bad vnum %d.", vnum);
     // exit (1);
    }

    return NULL;
}

/* Translates mob virtual number to its room index struct.
 * Hash table lookup. */
ROOM_INDEX_T *get_room_index (int vnum) {
    ROOM_INDEX_T *pRoomIndex;

    for (pRoomIndex = room_index_hash[vnum % MAX_KEY_HASH];
         pRoomIndex != NULL; pRoomIndex = pRoomIndex->next)
        if (pRoomIndex->vnum == vnum)
            return pRoomIndex;

    /* NOTE: This did cause the server not to boot, but that seems a bit
     *       extreme. So we just return NULL instead, and keep on booting.
     *       -- Synival */
    if (fBootDb) {
        bug ("get_room_index: bad vnum %d.", vnum);
     // exit (1);
    }

    return NULL;
}

MPROG_CODE_T *get_mprog_index (int vnum) {
    MPROG_CODE_T *prg;
    for (prg = mprog_list; prg; prg = prg->next)
        if (prg->vnum == vnum)
            return (prg);
    return NULL;
}

/* Read a letter from a file.  */
char fread_letter (FILE *fp) {
    char c;
    do {
        c = getc (fp);
    } while (isspace (c));
    return c;
}

/* Read a number from a file. */
int fread_number (FILE *fp) {
    int number = 0;
    bool sign = FALSE;
    char c = fread_letter(fp);

    if (c == '+')
        c = getc (fp);
    else if (c == '-') {
        sign = TRUE;
        c = getc (fp);
    }

    EXIT_IF_BUG (!isdigit (c),
        "fread_number: bad format.", 0);

    while (isdigit (c)) {
        number = number * 10 + c - '0';
        c = getc (fp);
    }

    if (sign)
        number = 0 - number;

    if (c == '|')
        number += fread_number (fp);
    else if (c != ' ')
        ungetc (c, fp);

    return number;
}

flag_t fread_flag (FILE *fp) {
    int number = 0;
    bool negative = FALSE;
    char c = fread_letter(fp);

    if (c == '-') {
        negative = TRUE;
        c = getc (fp);
    }

    if (!isdigit (c)) {
        while (('A' <= c && c <= 'Z') || ('a' <= c && c <= 'z')) {
            number += flag_convert (c);
            c = getc (fp);
        }
    }

    while (isdigit (c)) {
        number = number * 10 + c - '0';
        c = getc (fp);
    }

    if (c == '|')
        number += fread_flag (fp);
    else if (c != ' ')
        ungetc (c, fp);

    return negative ? -1 * number : number;
}

flag_t flag_convert (char letter) {
    flag_t bitsum = 0;
    char i;

    if ('A' <= letter && letter <= 'Z') {
        bitsum = 1;
        for (i = letter; i > 'A'; i--)
            bitsum *= 2;
    }
    else if ('a' <= letter && letter <= 'z') {
        bitsum = 67108864;        /* 2^26 */
        for (i = letter; i > 'a'; i--)
            bitsum *= 2;
    }

    return bitsum;
}

/* Read and allocate space for a string from a file.
 * These strings are read-only and shared.
 * Strings are hashed:
 *   each string prepended with hash pointer to prev string,
 *   hash code is simply the string length.
 *   this function takes 40% to 50% of boot-up time. */
char *fread_string (FILE *fp) {
    char *str, *plast;
    char c;

    str = string_space_next ();
    plast = str;

    c = fread_letter(fp);
    if ((*plast++ = c) == '~')
        return str_empty;

    while (1) {
        /* Back off the char type lookup,
         * it was too dirty for portability.
         *   -- Furey */

        switch (*plast = getc (fp)) {
            case EOF:
                /* temp fix */
                bug ("fread_string: EOF", 0);
                return NULL;

            case '\n':
                plast++;
                *plast++ = '\r';
                break;

            case '\r':
                break;

            case '~':
                *plast = '\0';
                return str_dup (str);

            default:
                plast++;
                break;
        }
    }
}

char *fread_string_eol (FILE *fp) {
    static bool char_special[256 - EOF];
    char *str, *plast;

    if (char_special[EOF - EOF] != TRUE) {
        char_special[EOF - EOF]  = TRUE;
        char_special['\n' - EOF] = TRUE;
        char_special['\r' - EOF] = TRUE;
    }

    str = string_space_next ();
    plast = str;

    *plast++ = fread_letter(fp);
    if (plast[-1] == '\n' || plast[-1] == '\r')
        return str_empty;

    while (1) {
        if (!char_special[(*plast++ = getc (fp)) - EOF])
            continue;

        switch (plast[-1]) {
            case EOF:
                bug ("fread_string_eol: EOF", 0);
                exit (1);
                break;

            case '\n':
            case '\r':
                plast--;
                *plast = '\0';
                return str_dup (str);

            default:
                break;
        }
    }
}

/* Read to end of line (for comments). */
void fread_to_eol (FILE *fp) {
    char c;

    do {
        c = getc (fp);
    } while (c != '\n' && c != '\r');

    do {
        c = getc (fp);
    } while (c == '\n' || c == '\r');

    ungetc (c, fp);
}

/* Read one word (into static buffer). */
char *fread_word (FILE *fp) {
    static char word[MAX_INPUT_LENGTH];
    char *pword;
    char cEnd;

    cEnd = fread_letter (fp);
    if (cEnd == '\'' || cEnd == '"')
        pword = word;
    else {
        word[0] = cEnd;
        pword = word + 1;
        cEnd = ' ';
    }

    for (; pword < word + MAX_INPUT_LENGTH; pword++) {
        *pword = getc (fp);
        if (cEnd == ' ' ? isspace (*pword) : *pword == cEnd) {
            if (cEnd == ' ')
                ungetc (*pword, fp);
            *pword = '\0';
            return word;
        }
    }

    EXIT_IF_BUG (TRUE,
        "fread_word: word too long.", 0);
    return NULL;
}

void fread_dice (FILE *fp, DICE_T *out) {
    out->number = fread_number (fp);
    /* 'd'     */ fread_letter (fp);
    out->size   = fread_number (fp);
    /* '+'     */ fread_letter (fp);
    out->bonus  = fread_number (fp);
}

/* Added this as part of a bugfix suggested by Chris Litchfield (of
 * "The Mage's Lair" fame). Pets were being loaded with any saved
 * affects, then having those affects loaded again. -- JR 2002/01/31 */
bool check_pet_affected(int vnum, AFFECT_T *paf) {
    MOB_INDEX_T *petIndex;

    petIndex = get_mob_index(vnum);
    if (petIndex == NULL)
        return FALSE;
    if (paf->bit_type == AFF_TO_AFFECTS &&
        IS_SET (petIndex->affected_by_final, paf->bits))
    {
        return TRUE;
    }
    return FALSE;
}

/* random room generation procedure */
ROOM_INDEX_T *get_random_room (CHAR_T *ch) {
    ROOM_INDEX_T *room;

    while (1) {
        room = get_room_index (number_range (0, 65535));
        if (room == NULL)
            continue;
        if (!char_can_see_room (ch, room))
            continue;
        if (room_is_private (room))
            continue;
        if (IS_SET (room->room_flags, ROOM_PRIVATE))
            continue;
        if (IS_SET (room->room_flags, ROOM_SOLITARY))
            continue;
        if (IS_SET (room->room_flags, ROOM_SAFE))
            continue;
        if (IS_NPC (ch) || !IS_SET (room->room_flags, ROOM_LAW))
            break;
    }

    return room;
}

bool fread_social_str (FILE *fp, char **str) {
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
void load_socials (FILE *fp) {
    while (1) {
        SOCIAL_T *new;
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

        do {
            if (!fread_social_str (fp, &(new->char_no_arg)))    break;
            if (!fread_social_str (fp, &(new->others_no_arg)))  break;
            if (!fread_social_str (fp, &(new->char_found)))     break;
            if (!fread_social_str (fp, &(new->others_found)))   break;
            if (!fread_social_str (fp, &(new->vict_found)))     break;
            if (!fread_social_str (fp, &(new->char_not_found))) break;
            if (!fread_social_str (fp, &(new->char_auto)))      break;
            if (!fread_social_str (fp, &(new->others_auto)))    break;
        } while (0);

        /* if this social already exists, don't load it. */
        if (social_lookup_exact (new->name) != new)
            social_free (new);
    }
}

/* Snarf a mob section.  new style */
void load_mobiles (FILE *fp) {
    MOB_INDEX_T *pMobIndex;
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

        fBootDb = FALSE;
        EXIT_IF_BUG (get_mob_index (vnum) != NULL,
            "load_mobiles: vnum %d duplicated.", vnum);
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

        str = fread_string (fp);
        pMobIndex->race = lookup_backup (race_lookup_exact,
            str, "Unknown race '%s'", 0);

        pMobIndex->long_descr[0] = UPPER (pMobIndex->long_descr[0]);
        pMobIndex->description[0] = UPPER (pMobIndex->description[0]);

        pMobIndex->mob_plus = fread_flag (fp) & ~MOB_IS_NPC;
        pMobIndex->affected_by_plus = fread_flag (fp);

        pMobIndex->pShop = NULL;
        pMobIndex->alignment = fread_number (fp);
        pMobIndex->group = fread_number (fp);

        pMobIndex->level = fread_number (fp);
        pMobIndex->hitroll = fread_number (fp);

        /* read hit, mana, dam dice */
        fread_dice (fp, &(pMobIndex->hit));
        fread_dice (fp, &(pMobIndex->mana));
        fread_dice (fp, &(pMobIndex->damage));

        str = fread_word (fp);
        pMobIndex->dam_type = lookup_backup (attack_lookup_exact,
            str, "Unknown damage type '%s'", 0);

        /* read armor class */
        pMobIndex->ac[AC_PIERCE] = fread_number (fp) * 10;
        pMobIndex->ac[AC_BASH]   = fread_number (fp) * 10;
        pMobIndex->ac[AC_SLASH]  = fread_number (fp) * 10;
        pMobIndex->ac[AC_EXOTIC] = fread_number (fp) * 10;

        /* read flags and add in data from the race table */
        pMobIndex->off_flags_plus  = fread_flag (fp);
        pMobIndex->imm_flags_plus  = fread_flag (fp);
        pMobIndex->res_flags_plus  = fread_flag (fp);
        pMobIndex->vuln_flags_plus = fread_flag (fp);

        /* vital statistics */
        str = fread_word (fp);
        pMobIndex->start_pos = lookup_backup (position_lookup_exact,
            str, "Unknown start position '%s'", POS_STANDING);

        str = fread_word (fp);
        pMobIndex->default_pos = lookup_backup (position_lookup_exact,
            str, "Unknown default position '%s'", POS_STANDING);

        /* read sex. 'none' has been replaced with 'neutral' to avoid confusion
         * between 'sexless' and 'sex is missing'. */
        str = fread_word (fp);
        if (!str_cmp (str, "none"))
            str = "neutral";
        pMobIndex->sex = lookup_backup (sex_lookup_exact,
            str, "Unknown sex '%s'", SEX_EITHER);

        pMobIndex->wealth     = fread_number (fp);
        pMobIndex->form_plus  = fread_flag (fp);
        pMobIndex->parts_plus = fread_flag (fp);

        /* Size. */
        str = fread_word (fp);
        pMobIndex->size = lookup_backup (size_lookup_exact, str,
            "Unknown size '%s'", SIZE_MEDIUM);

        /* Material. Sometimes this is '0', in which case, just replace it
         * with the default material's keyword. */
        str = fread_word (fp);
        if (!str_cmp (str, "0") || str[0] == '\0')
            str = (char *) material_get_name (MATERIAL_GENERIC);
        pMobIndex->material = lookup_backup (material_lookup_exact,
            str, "Unknown material '%s'", MATERIAL_GENERIC);

        while (1) {
            letter = fread_letter (fp);
            if (letter == 'F') {
                char *word;
                long vector;

                word = fread_word (fp);
                vector = fread_flag (fp);

                if (!str_prefix (word, "act") || !str_prefix (word, "mob"))
                    pMobIndex->mob_minus = vector;
                else if (!str_prefix (word, "aff"))
                    pMobIndex->affected_by_minus = vector;
                else if (!str_prefix (word, "off"))
                    pMobIndex->off_flags_minus = vector;
                else if (!str_prefix (word, "imm"))
                    pMobIndex->imm_flags_minus = vector;
                else if (!str_prefix (word, "res"))
                    pMobIndex->res_flags_minus = vector;
                else if (!str_prefix (word, "vul"))
                    pMobIndex->vuln_flags_minus = vector;
                else if (!str_prefix (word, "for"))
                    pMobIndex->form_minus = vector;
                else if (!str_prefix (word, "par"))
                    pMobIndex->parts_minus = vector;
                else {
                    EXIT_IF_BUG (TRUE,
                        "flag remove: flag not found.", 0);
                }
            }
            else if (letter == 'M') {
                MPROG_LIST_T *pMprog;
                char *word;
                int trigger = 0;

                pMprog = mprog_new ();
                word = fread_word (fp);
                EXIT_IF_BUG ((trigger = flag_lookup_exact (word, mprog_flags)) <= 0,
                    "load_mobiles: invalid mob prog trigger.", 0);
                SET_BIT (pMobIndex->mprog_flags, trigger);
                pMprog->trig_type = trigger;
                pMprog->area = area_last;
                pMprog->vnum = fread_number (fp);
                pMprog->anum = pMprog->vnum - area_last->min_vnum;
                str_replace_dup (&pMprog->trig_phrase, fread_string (fp));

                LIST_BACK (pMprog, next, pMobIndex->mprogs, MPROG_LIST_T);
            }
            else {
                ungetc (letter, fp);
                break;
            }
        }

        /* Perform post-processing on loaded mob. */
        db_finalize_mob (pMobIndex);

        /* Loading finished - register our mob. */
        db_register_new_mob (pMobIndex);
    }
}

void db_finalize_mob (MOB_INDEX_T *mob) {
    if (mob->race >= 0 || mob->race < RACE_MAX) {
        const RACE_T *race;
        race = &(race_table[mob->race]);

        mob->mob_final         = mob->mob_plus | MOB_IS_NPC | race->mob;
        mob->affected_by_final = mob->affected_by_plus | race->aff;
        mob->off_flags_final   = mob->off_flags_plus   | race->off;
        mob->imm_flags_final   = mob->imm_flags_plus   | race->imm;
        mob->res_flags_final   = mob->res_flags_plus   | race->res;
        mob->vuln_flags_final  = mob->vuln_flags_plus  | race->vuln;
        mob->form_final        = mob->form_plus        | race->form;
        mob->parts_final       = mob->parts_plus       | race->parts;
    }
    else {
        mob->mob_final         = mob->mob_plus | MOB_IS_NPC;
        mob->affected_by_final = mob->affected_by_plus;
        mob->off_flags_final   = mob->off_flags_plus;
        mob->imm_flags_final   = mob->imm_flags_plus;
        mob->res_flags_final   = mob->res_flags_plus;
        mob->vuln_flags_final  = mob->vuln_flags_plus;
        mob->form_final        = mob->form_plus;
        mob->parts_final       = mob->parts_plus;
    }

    REMOVE_BIT (mob->mob_final,         mob->mob_minus);
    REMOVE_BIT (mob->affected_by_final, mob->affected_by_minus);
    REMOVE_BIT (mob->off_flags_final,   mob->off_flags_minus);
    REMOVE_BIT (mob->imm_flags_final,   mob->imm_flags_minus);
    REMOVE_BIT (mob->res_flags_final,   mob->res_flags_minus);
    REMOVE_BIT (mob->vuln_flags_final,  mob->vuln_flags_minus);
    REMOVE_BIT (mob->form_final,        mob->form_minus);
    REMOVE_BIT (mob->parts_final,       mob->parts_minus);
}

/* Snarf an obj section. new style */
void load_objects (FILE *fp) {
    OBJ_INDEX_T *pObjIndex;
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

        fBootDb = FALSE;
        EXIT_IF_BUG (get_obj_index (vnum) != NULL,
            "load_objects: vnum %d duplicated.", vnum);
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

        /* Read material, and replace 'oldstyle' (???) or 'blank' with 'generic'. */
        str = fread_string (fp);
        if (!str_cmp (str, "oldstyle") || str[0] == '\0')
            str_replace_dup (&str, material_get_name (MATERIAL_GENERIC));
        pObjIndex->material = lookup_backup (material_lookup_exact,
            str, "Unknown material '%s'", MATERIAL_GENERIC);

        str = fread_word (fp);
        pObjIndex->item_type = lookup_backup (item_lookup_exact,
            str, "Unknown item type '%s'", 0);

        pObjIndex->extra_flags = fread_flag (fp);
        pObjIndex->wear_flags = fread_flag (fp);

        switch (pObjIndex->item_type) {
            case ITEM_WEAPON:
                pObjIndex->v.weapon.weapon_type = weapon_lookup_exact (fread_word (fp));
                pObjIndex->v.weapon.dice_num    = fread_number (fp);
                pObjIndex->v.weapon.dice_size   = fread_number (fp);
                pObjIndex->v.weapon.attack_type = attack_lookup_exact (fread_word (fp));
                pObjIndex->v.weapon.flags       = fread_flag (fp);
                break;

            case ITEM_CONTAINER:
                pObjIndex->v.container.capacity    = fread_number (fp);
                pObjIndex->v.container.flags       = fread_flag (fp);
                pObjIndex->v.container.key         = fread_number (fp);
                pObjIndex->v.container.max_weight  = fread_number (fp);
                pObjIndex->v.container.weight_mult = fread_number (fp);
                break;

            case ITEM_DRINK_CON:
                pObjIndex->v.drink_con.capacity = fread_number (fp);
                pObjIndex->v.drink_con.filled   = fread_number (fp);
                pObjIndex->v.drink_con.liquid   = lookup_backup (liq_lookup_exact,
                    fread_word (fp), "Unknown liquid type '%s'", 0);
                pObjIndex->v.drink_con.poisoned = fread_number (fp);
                pObjIndex->v.drink_con._value5  = fread_number (fp);
                break;

            case ITEM_FOUNTAIN:
                pObjIndex->v.fountain.capacity = fread_number (fp);
                pObjIndex->v.fountain.filled   = fread_number (fp);
                pObjIndex->v.fountain.liquid   = lookup_backup (liq_lookup_exact,
                    fread_word (fp), "Unknown liquid type '%s'", 0);
                pObjIndex->v.fountain.poisoned = fread_number (fp);
                pObjIndex->v.fountain._value5  = fread_number (fp);
                break;

            case ITEM_WAND:
                pObjIndex->v.wand.level    = fread_number (fp);
                pObjIndex->v.wand.recharge = fread_number (fp);
                pObjIndex->v.wand.charges  = fread_number (fp);
                pObjIndex->v.wand.skill    = skill_lookup_exact (fread_word (fp));
                pObjIndex->v.wand._value5  = fread_number (fp);
                break;

            case ITEM_STAFF:
                pObjIndex->v.staff.level    = fread_number (fp);
                pObjIndex->v.staff.recharge = fread_number (fp);
                pObjIndex->v.staff.charges  = fread_number (fp);
                pObjIndex->v.staff.skill    = skill_lookup_exact (fread_word (fp));
                pObjIndex->v.staff._value5  = fread_number (fp);
                break;

            case ITEM_POTION: {
                pObjIndex->v.potion.level  = fread_number (fp);
                pObjIndex->v.potion.skill[0] = skill_lookup_exact (fread_word (fp));
                pObjIndex->v.potion.skill[1] = skill_lookup_exact (fread_word (fp));
                pObjIndex->v.potion.skill[2] = skill_lookup_exact (fread_word (fp));
                pObjIndex->v.potion.skill[3] = skill_lookup_exact (fread_word (fp));
                break;
            }

            case ITEM_PILL:
                pObjIndex->v.pill.level  = fread_number (fp);
                pObjIndex->v.pill.skill[0] = skill_lookup_exact (fread_word (fp));
                pObjIndex->v.pill.skill[1] = skill_lookup_exact (fread_word (fp));
                pObjIndex->v.pill.skill[2] = skill_lookup_exact (fread_word (fp));
                pObjIndex->v.pill.skill[3] = skill_lookup_exact (fread_word (fp));
                break;

            case ITEM_SCROLL:
                pObjIndex->v.scroll.level  = fread_number (fp);
                pObjIndex->v.scroll.skill[0] = skill_lookup_exact (fread_word (fp));
                pObjIndex->v.scroll.skill[1] = skill_lookup_exact (fread_word (fp));
                pObjIndex->v.scroll.skill[2] = skill_lookup_exact (fread_word (fp));
                pObjIndex->v.scroll.skill[3] = skill_lookup_exact (fread_word (fp));
                break;

            default:
                pObjIndex->v.value[0] = fread_flag (fp);
                pObjIndex->v.value[1] = fread_flag (fp);
                pObjIndex->v.value[2] = fread_flag (fp);
                pObjIndex->v.value[3] = fread_flag (fp);
                pObjIndex->v.value[4] = fread_flag (fp);
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
                AFFECT_T *paf = affect_new ();

                location = fread_number (fp);
                modifier = fread_number (fp);
                affect_init (paf, AFF_TO_OBJECT, -1, pObjIndex->level, -1, location, modifier, 0);

                LIST_BACK (paf, next, pObjIndex->affected, AFFECT_T);
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
                affect_init (paf, paf->bit_type, -1, pObjIndex->level, -1,
                    location, modifier, bits);

                LIST_BACK (paf, next, pObjIndex->affected, AFFECT_T);
            }
            else if (letter == 'E') {
                EXTRA_DESCR_T *ed = extra_descr_new ();
                str_replace_dup (&ed->keyword,     fread_string (fp));
                str_replace_dup (&ed->description, fread_string (fp));
                LIST_BACK (ed, next, pObjIndex->extra_descr, EXTRA_DESCR_T);
            }
            else {
                ungetc (letter, fp);
                break;
            }
        }

        /* Perform post-processing on loaded obj. */
        db_finalize_obj (pObjIndex);

        /* Loading finished - register our object. */
        db_register_new_obj (pObjIndex);
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
        fprintf (file, "next:       %s\n",  area->next  ? "yes" : "no");
        fprintf (file, "helps:      %s\n",  area->helps ? "yes" : "no");
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
        fprintf (file, "next:        %s\n", room->next        ? "yes" : "no");
        fprintf (file, "people:      %s\n", room->people      ? "yes" : "no");
        fprintf (file, "contents:    %s\n", room->contents    ? "yes" : "no");
        fprintf (file, "extra_descr: %s\n", room->extra_descr ? "yes" : "no");
        for (ed = room->extra_descr; ed != NULL; ed = ed->next) {
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
        for (reset = room->reset_first; reset != NULL; reset = reset->next) {
            fprintf (file, "   next:      %s\n",   reset->next ? "yes" : "no");
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
        fprintf (file, "next:               %s\n",  mob->next ? "yes" : "no");
        fprintf (file, "spec_fun:           %s\n",  spec_function_name (mob->spec_fun));
        fprintf (file, "pShop:              %s\n",  mob->pShop ? "yes" : "no");
        if (mob->pShop) {
            SHOP_T *shop = mob->pShop;
            fprintf (file, "   next:         %s\n", shop->next ? "yes" : "no");
            fprintf (file, "   keeper:       %d\n", shop->keeper);
            for (i = 0; i < MAX_TRADE; i++)
                if (shop->buy_type[i] != 0)
                    fprintf (file, "   buy_type[%d]: %d\n", i, shop->buy_type[i]);
            fprintf (file, "   profit_buy:   %d\n", shop->profit_buy);
            fprintf (file, "   profit_sell:  %d\n", shop->profit_sell);
            fprintf (file, "   open_hour:    %d\n", shop->open_hour);
            fprintf (file, "   close_hour:   %d\n", shop->close_hour);
        }
        fprintf (file, "mprogs:             %s\n",  mob->mprogs ? "yes" : "no");
        fprintf (file, "area_str:           %s\n",  mob->area_str);
        fprintf (file, "area:               %s\n",  mob->area ? "yes" : "no");
        fprintf (file, "vnum:               %d\n",  mob->vnum);
        fprintf (file, "anum:               %d\n",  mob->anum);
        fprintf (file, "group:              %d\n",  mob->group);
        fprintf (file, "new_format:         %d\n",  mob->new_format);
        fprintf (file, "count:              %d\n",  mob->count);
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
        fprintf (file, "dam_type:           %d\n",  mob->dam_type);
        fprintf (file, "start_pos:          %d\n",  mob->start_pos);
        fprintf (file, "default_pos:        %d\n",  mob->default_pos);
        fprintf (file, "sex:                %d\n",  mob->sex);
        fprintf (file, "race:               %d\n",  mob->race);
        fprintf (file, "wealth:             %ld\n", mob->wealth);
        fprintf (file, "size:               %d\n",  mob->size);
        fprintf (file, "material:           %d\n",  mob->material);
        fprintf (file, "mprog_flags:        %ld\n", mob->mprog_flags);
        fprintf (file, "mob_plus:           %ld\n", mob->mob_plus);
        fprintf (file, "mob_final:          %ld\n", mob->mob_final);
        fprintf (file, "mob_minus:          %ld\n", mob->mob_minus);
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
        fprintf (file, "next:          %s\n",  obj->next ? "yes" : "no");
        fprintf (file, "extra_descr:   %s\n",  obj->extra_descr ? "yes" : "no");
        for (ed = obj->extra_descr; ed != NULL; ed = ed->next) {
            fprintf (file, "   keyword:     %s\n", ed->keyword);
            fprintf (file, "   description: %s\n", ed->description);
        }

        fprintf (file, "affected:      %s\n",  obj->affected ? "yes" : "no");
        for (aff = obj->affected; ed != NULL; ed = ed->next) {
            fprintf (file, "   next:     %s\n",  aff->next ? "yes" : "no");
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
        fprintf (file, "count:         %d\n",  obj->count);
        fprintf (file, "weight:        %d\n",  obj->weight);
        fprintf (file, "cost:          %d\n",  obj->cost);
        for (i = 0; i < OBJ_VALUE_MAX; i++)
            fprintf (file, "v.value[%d]:    %ld\n", i, obj->v.value[i]);
        fprintf (file, "-\n");
        count++;
    }

    for (had = had_get_first(); had; had = had_get_next (had)) {
        fprintf (file, "next:     %s\n", had->next  ? "yes" : "no");
        fprintf (file, "first:    %s\n", had->first ? "yes" : "no");
        fprintf (file, "last:     %s\n", had->last  ? "yes" : "no");
        fprintf (file, "area_str: %s\n", had->area_str);
        fprintf (file, "area:     %s\n", had->area  ? "yes" : "no");
        fprintf (file, "filename: %s\n", had->filename);
        fprintf (file, "name:     %s\n", had->name);
        fprintf (file, "changed:  %d\n", had->changed);
        fprintf (file, "-\n");
        count++;
    }

    for (help = help_get_first(); help; help = help_get_next (help)) {
        fprintf (file, "next:      %s\n", help->next      ? "yes" : "no");
        fprintf (file, "next_area: %s\n", help->next_area ? "yes" : "no");
        fprintf (file, "level:     %d\n", help->level);
        fprintf (file, "keyword:   %s\n", help->keyword);
        fprintf (file, "text:      %s\n", help->text);
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
        if (portal->generated)
            continue;
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
    LIST2_BACK (anum, prev, next, anum_first, anum_last);
    return anum;
}

void anum_free (ANUM_T *anum) {
    str_free (&(anum->area_str));
    LIST2_REMOVE (anum, prev, next, anum_first, anum_last);
    free (anum);
}
