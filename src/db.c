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

#include "utils.h"
#include "recycle.h"
#include "affects.h"
#include "lookup.h"
#include "json.h"
#include "json_obj.h"
#include "music.h"
#include "ban.h"
#include "board.h"
#include "comm.h"
#include "db2.h"
#include "interp.h"
#include "magic.h"
#include "olc.h"
#include "portals.h"
#include "chars.h"
#include "rooms.h"
#include "objs.h"

#include "db.h"

/* TODO: my JSON-saving code is the worst thing ever produced. rewrite :( */
/* TODO: this file is giant, and lots of the loading routines are ugly.
 *       split it up, and clean it up! */
/* TODO: integrate db2.c into this, move old stuff to db_old.c */
/* TODO: move code below into header files */
/* TODO: evaluate what globals belong here */

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

/* Globals.  */
HELP_DATA *help_first, *help_last;
SHOP_DATA *shop_first, *shop_last;
AREA_DATA *area_first, *area_last;
BAN_DATA  *ban_first,  *ban_last;
HELP_AREA *had_first,  *had_last;

CHAR_DATA *char_list;
MPROG_CODE *mprog_list;
OBJ_DATA *object_list;

int newmobs = 0;
int newobjs = 0;

char bug_buf[2 * MAX_INPUT_LENGTH];
char *help_greeting;
char log_buf[2 * MAX_INPUT_LENGTH];
KILL_DATA kill_table[MAX_LEVEL];
TIME_INFO_DATA time_info;
WEATHER_DATA weather_info;

/* Locals. */
MOB_INDEX_DATA *mob_index_hash[MAX_KEY_HASH];
OBJ_INDEX_DATA *obj_index_hash[MAX_KEY_HASH];
ROOM_INDEX_DATA *room_index_hash[MAX_KEY_HASH];
char *string_hash[MAX_KEY_HASH];
AREA_DATA *current_area;

/* Semi-locals.  */
bool fBootDb;
FILE *fpArea;
char strArea[MAX_INPUT_LENGTH];

void db_export_json (void) {
    JSON_T *jarea, *jgrp, *json;
    AREA_DATA *area;
    char buf[256], fbuf[256];

    /* Write all areas. */
    for (area = area_get_first(); area != NULL; area = area_get_next(area)) {
        snprintf (fbuf, sizeof(fbuf), "%s/", area->name);

        snprintf (buf, sizeof(buf), "areas/%s", area->name);
        jarea = json_root_area (buf);
        jgrp = json_prop_array (jarea, NULL);

        json = json_wrap_obj(json_new_obj_area (NULL, area), "area");
        json_attach_under (json, jgrp);

        log_f("Exporting JSON: %sareas/%s*", JSON_DIR, fbuf);
        snprintf (buf, sizeof(buf), "%sareas/%sarea.json", JSON_DIR, fbuf);
        json_mkdir_to (buf);
        json_write_to_file (jgrp, buf);

        #define ADD_AREA_JSON(oname, fname, btype, vtype, func) \
            do { \
                vtype *obj; \
                jgrp = json_prop_array (jarea, NULL); \
                \
                for (obj = btype ## _get_first(); obj != NULL; \
                     obj = btype ## _get_next(obj)) \
                { \
                    if (obj->area != area) \
                        continue; \
                    if (obj->vnum < area->min_vnum || obj->vnum > area->max_vnum) \
                        bugf("Warning: " #btype " #%d should be >= %d and <= %d", \
                        obj->vnum, area->min_vnum, area->max_vnum);\
                    json = json_wrap_obj (func (NULL, obj), oname); \
                    json_attach_under (json, jgrp); \
                } \
                if (jgrp->first_child) { \
                    snprintf (buf, sizeof (buf), "%sareas/%s" fname, JSON_DIR, fbuf); \
                    json_mkdir_to (buf); \
                    json_write_to_file (jgrp, buf); \
                } \
            } while (0)

        ADD_AREA_JSON ("room",   "rooms.json",   room_index, ROOM_INDEX_DATA, json_new_obj_room);
        ADD_AREA_JSON ("object", "objects.json", obj_index,  OBJ_INDEX_DATA,  json_new_obj_object);
        ADD_AREA_JSON ("mobile", "mobiles.json", mob_index,  MOB_INDEX_DATA,  json_new_obj_mobile);
    }

    /* Write json that doesn't need subdirectories. */
    #define ADD_META_JSON(oname, fname, btype, vtype, func, check) \
        jarea = json_root_area (fname); \
        log_f("Exporting JSON: %s%s.json", JSON_DIR, fname); \
        do { \
            vtype *obj; \
            \
            for (obj = (vtype *) btype ## _get_first(); obj != NULL; \
                 obj = (vtype *) btype ## _get_next((const vtype *) obj)) \
            { \
                if (!(check)) \
                    continue; \
                json = json_wrap_obj (func (NULL, obj), oname); \
                json_attach_under (json, jarea); \
            } \
            \
            if (jarea->first_child) { \
                snprintf (buf, sizeof (buf), "%s" fname ".json", JSON_DIR); \
                json_mkdir_to (buf); \
                json_write_to_file (jarea, buf); \
            } \
        } while (0)

    ADD_META_JSON ("social", "config/socials", social, SOCIAL_TYPE,
        json_new_obj_social, 1);
    ADD_META_JSON ("portal", "config/portals", portal, PORTAL_TYPE,
        json_new_obj_portal,
           (obj->opposite == NULL ||
            obj->from->dir == DIR_NORTH ||
            obj->from->dir == DIR_EAST ||
            obj->from->dir == DIR_UP));

    ADD_META_JSON ("table", "meta/flags", master, TABLE_TYPE,
        json_new_obj_table, ARE_SET (obj->flags, TABLE_FLAG_TYPE | TABLE_BITS));
    ADD_META_JSON ("table", "meta/types", master, TABLE_TYPE,
        json_new_obj_table, IS_SET (obj->flags, TABLE_FLAG_TYPE) &&
                           !IS_SET (obj->flags, TABLE_BITS));
    ADD_META_JSON ("table", "meta/tables", master, TABLE_TYPE,
        json_new_obj_table, !IS_SET (obj->flags, TABLE_FLAG_TYPE) &&
                             obj->json_write_func);

    /* Add help areas. */
    do {
        HELP_AREA *had;
        for (had = had_get_first(); had; had = had_get_next (had)) {
            snprintf (buf, sizeof(buf), "help/%s", had->name);
            jarea = json_root_area (buf);
            snprintf (fbuf, sizeof (fbuf), "%shelp/%s.json", JSON_DIR, had->name);
            log_f("Exporting JSON: %s", fbuf);

            json = json_wrap_obj (json_new_obj_help_area (NULL, had), "help");
            json_attach_under (json, jarea);
            json_mkdir_to (fbuf);
            json_write_to_file (jarea, fbuf);
        }
    } while (0);

    json_write_to_file (json_root(), JSON_DIR "everything.json");
    json_free (json_root());
}

/* Big mama top level function. */
void boot_db (void) {
    /* Declare that we're booting the database. */
    fBootDb = TRUE;

    init_string_space ();
    init_mm ();
    init_time_weather ();
    init_gsns ();
    init_areas ();

    fix_exits ();
    portal_create_missing ();
    fix_mobprogs ();

    /* Boot process is over(?) */
    fBootDb = FALSE;

    convert_objects (); /* ROM OLC */
    db_export_json ();
    area_update ();
    load_boards ();
    save_notes ();
    load_bans ();
    load_songs ();
}

void init_time_weather (void) {
    long lhour, lday, lmonth;

    lhour = (current_time - 650336715) / (PULSE_TICK / PULSE_PER_SECOND);
    time_info.hour = lhour % 24;
    lday = lhour / 24;
    time_info.day = lday % 35;
    lmonth = lday / 35;
    time_info.month = lmonth % 17;
    time_info.year = lmonth / 17;

         if (time_info.hour <  5) weather_info.sunlight = SUN_DARK;
    else if (time_info.hour <  6) weather_info.sunlight = SUN_RISE;
    else if (time_info.hour < 19) weather_info.sunlight = SUN_LIGHT;
    else if (time_info.hour < 20) weather_info.sunlight = SUN_SET;
    else                          weather_info.sunlight = SUN_DARK;

    weather_info.change = 0;
    weather_info.mmhg = 960;
    if (time_info.month >= 7 && time_info.month <= 12)
        weather_info.mmhg += number_range (1, 50);
    else
        weather_info.mmhg += number_range (1, 80);

    if (weather_info.mmhg <= 980)
        weather_info.sky = SKY_LIGHTNING;
    else if (weather_info.mmhg <= 1000)
        weather_info.sky = SKY_RAINING;
    else if (weather_info.mmhg <= 1020)
        weather_info.sky = SKY_CLOUDY;
    else
        weather_info.sky = SKY_CLOUDLESS;
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
            if ((fpArea = fopen (fname, "r")) == NULL) {
                perror (fname);
                exit (1);
            }
        }

        current_area = NULL;
        while (1) {
            char *word;

            if (fread_letter (fpArea) != '#') {
                bug ("boot_db: # not found.", 0);
                exit (1);
            }

            word = fread_word (fpArea);
            if (word[0] == '$')
                break;

                 if (!str_cmp (word, "AREA"))     load_area (fpArea);
            else if (!str_cmp (word, "AREADATA")) new_load_area (fpArea); /* OLC */
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
                bug ("boot_db: bad section name.", 0);
                exit (1);
            }
        }

        if (fpArea != stdin)
            fclose (fpArea);
        fpArea = NULL;
    }
    fclose (fpList);
}

/* Snarf an 'area' header line. */
void load_area (FILE * fp) {
    AREA_DATA *pArea, *pLast;

    pArea = area_new ();
    str_replace_dup (&pArea->filename, fread_string (fp));
    str_replace_dup (&pArea->name, trim_extension (pArea->filename));

    /* Pretty up the log a little */
    log_f("Loading area %s", pArea->filename);

    pArea->area_flags = AREA_LOADING;           /* OLC */
    pArea->security = 9;                        /* OLC 9 -- Hugin */
    str_replace_dup (&pArea->builders, "None"); /* OLC */
    pArea->vnum = TOP(RECYCLE_AREA_DATA);       /* OLC */

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
 * support on your MUD.  See the new_load_area format below for
 * a short example. */
#if defined(KEY)
    #undef KEY
#endif

#define KEY(literal, field, value)  \
    if (!str_cmp (word, literal)) { \
        field = value;              \
        break;                      \
    }

#define SKEY(string, field)        \
    if (!str_cmp (word, string)) { \
        str_free (field);          \
        field = fread_string (fp); \
        break;                     \
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
void new_load_area (FILE * fp) {
    AREA_DATA *pArea;
    char *word;

    pArea = area_new ();
    pArea->age = 15;
    pArea->nplayer = 0;
    str_replace_dup (&pArea->filename, strArea);
    pArea->vnum = TOP(RECYCLE_AREA_DATA);
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
void assign_area_vnum (int vnum) {
    if (area_last->min_vnum == 0 && area_last->max_vnum == 0)
        area_last->min_vnum = area_last->max_vnum = vnum;
    if (vnum != URANGE (area_last->min_vnum, vnum, area_last->max_vnum)) {
        if (vnum < area_last->min_vnum)
            area_last->min_vnum = vnum;
        else
            area_last->max_vnum = vnum;
    }
}

/* Snarf a help section. */
void load_helps (FILE * fp, char *fname) {
    HELP_DATA *pHelp;
    int level;
    char *keyword;

    while (1) {
        HELP_AREA *had;

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

        if (!str_cmp (pHelp->keyword, "greeting"))
            help_greeting = pHelp->text;

        LISTB_BACK (pHelp, next, help_first, help_last);
        LISTB_BACK (pHelp, next_area, had->first, had->last);
    }
}

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

        pMobIndex->act = fread_flag (fp) | ACT_IS_NPC;
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

void fix_bogus_obj (OBJ_INDEX_DATA * obj) {
    switch (obj->item_type) {
        case ITEM_FURNITURE: {
            int min_occupants = 0;
            int min_hit       = 100;
            int min_mana      = 100;

            if (is_name ("tent", obj->name) ||
                is_name ("cabin", obj->name))
            {
                SET_BIT (obj->value[2], REST_IN);
                SET_BIT (obj->value[2], SIT_IN);
                SET_BIT (obj->value[2], SLEEP_IN);
                SET_BIT (obj->value[2], STAND_IN);
                min_occupants = 1;
                min_hit = 250;
            }
            if (is_name ("bed", obj->name)) {
                SET_BIT (obj->value[2], REST_ON);
                SET_BIT (obj->value[2], SIT_ON);
                SET_BIT (obj->value[2], SLEEP_IN);
                min_occupants = 1;
                min_hit = 200;
            }
            if (is_name ("sofa", obj->name) ||
                is_name ("couch", obj->name))
            {
                SET_BIT (obj->value[2], REST_ON);
                SET_BIT (obj->value[2], SIT_ON);
                SET_BIT (obj->value[2], SLEEP_ON);
                min_occupants = 1;
                min_hit = 150;
            }
            if (is_name ("bench", obj->name)) {
                SET_BIT (obj->value[2], REST_ON);
                SET_BIT (obj->value[2], SIT_ON);
                SET_BIT (obj->value[2], SLEEP_ON);
                min_occupants = 1;
                min_hit = 125;
            }
            if (is_name ("chair", obj->name) ||
                is_name ("stool", obj->name))
            {
                SET_BIT (obj->value[2], STAND_ON);
                SET_BIT (obj->value[2], REST_ON);
                SET_BIT (obj->value[2], SIT_ON);
                SET_BIT (obj->value[2], SLEEP_ON);
                min_occupants = 1;
                min_hit = 125;
            }

            if (obj->value[0] < min_occupants) obj->value[0] = min_occupants;
            if (obj->value[3] < min_hit)       obj->value[3] = min_hit;
            if (obj->value[4] < min_mana)      obj->value[4] = min_mana;
            break;
        }
    }
}

/* Adds a reset to a room.  OLC
 * Similar to add_reset in olc.c */
void room_take_reset (ROOM_INDEX_DATA *room, RESET_DATA *reset) {
    if (!room || !reset)
        return;
    reset->area = room->area;
    LISTB_BACK (reset, next, room->reset_first, room->reset_last);
}

/* Snarf a reset section. */
void load_resets (FILE * fp) {
    RESET_DATA *pReset;
    EXIT_DATA *pexit;
    ROOM_INDEX_DATA *pRoomIndex;
    int rVnum = -1;

    if (!area_last) {
        bug ("load_resets: no #AREA seen yet.", 0);
        exit (1);
    }

    while (1) {
        char letter;
        if ((letter = fread_letter (fp)) == 'S')
            break;

        if (letter == '*') {
            fread_to_eol (fp);
            continue;
        }

        pReset = reset_data_new ();
        pReset->area  = area_last;
        pReset->command = letter;
        pReset->value[0] = fread_number (fp);
        pReset->value[1] = fread_number (fp);
        pReset->value[2] = fread_number (fp);
        pReset->value[3] = (letter == 'G' || letter == 'R') ? 0 : fread_number (fp);
        pReset->value[4] = (letter == 'P' || letter == 'M') ? fread_number (fp) : 0;
        fread_to_eol (fp);

        switch (pReset->command) {
            case 'M':
            case 'O':
                rVnum = pReset->value[3];
                break;

            case 'P':
            case 'G':
            case 'E':
                break;

            case 'D':
                pRoomIndex = get_room_index ((rVnum = pReset->value[1]));
                if (pReset->value[2] < 0
                    || pReset->value[2] >= DIR_MAX
                    || !pRoomIndex
                    || !(pexit = pRoomIndex->exit[pReset->value[2]])
                    || !IS_SET (pexit->rs_flags, EX_ISDOOR))
                {
                    bugf ("Load_resets: 'D': exit %d, room %d not door.",
                          pReset->value[2], pReset->value[1]);
                    exit (1);
                }

                switch (pReset->value[3]) {
                    case 0:
                        break;
                    case 1:
                        SET_BIT (pexit->exit_flags, EX_CLOSED);
                        break;
                    case 2:
                        SET_BIT (pexit->exit_flags, EX_CLOSED | EX_LOCKED);
                        break;
                    default:
                        bug ("load_resets: 'D': bad 'locks': %d.",
                             pReset->value[3]);
                        break;
                }
                pexit->rs_flags = pexit->exit_flags;
                break;

            case 'R':
                rVnum = pReset->value[1];
                break;
        }

        if (rVnum == -1) {
            bugf ("load_resets : rVnum == -1");
            exit (1);
        }
        if (pReset->command == 'D') {
            reset_data_free (pReset);
            continue;
        }
        room_take_reset (get_room_index (rVnum), pReset);
    }
}

/* Snarf a room section. */
void load_rooms (FILE * fp) {
    ROOM_INDEX_DATA *pRoomIndex;

    if (area_last == NULL) {
        bug ("load_resets: no #AREA seen yet.", 0);
        exit (1);
    }

    while (1) {
        sh_int vnum;
        char letter;
        int door;
        int iHash;

        letter = fread_letter (fp);
        if (letter != '#') {
            bug ("load_rooms: # not found.", 0);
            exit (1);
        }

        vnum = fread_number (fp);
        if (vnum == 0)
            break;

        fBootDb = FALSE;
        if (get_room_index (vnum) != NULL) {
            bug ("load_rooms: vnum %d duplicated.", vnum);
            exit (1);
        }
        fBootDb = TRUE;

        pRoomIndex = room_index_new ();
        str_replace_dup (&pRoomIndex->owner, "");
        pRoomIndex->people = NULL;
        pRoomIndex->contents = NULL;
        pRoomIndex->extra_descr = NULL;
        pRoomIndex->area = area_last;
        pRoomIndex->vnum = vnum;
        pRoomIndex->anum = vnum - area_last->min_vnum;
        str_replace_dup (&pRoomIndex->name, fread_string (fp));
        str_replace_dup (&pRoomIndex->description, fread_string (fp));
        /* Area number */ fread_number (fp);
        pRoomIndex->room_flags = fread_flag (fp);
        /* horrible hack */
        if (3000 <= vnum && vnum < 3400)
            SET_BIT (pRoomIndex->room_flags, ROOM_LAW);
        pRoomIndex->sector_type = fread_number (fp);
        pRoomIndex->light = 0;
        for (door = 0; door <= 5; door++)
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
                if (pRoomIndex->clan_str) {
                    bug ("load_rooms: duplicate clan fields.", 0);
                    exit (1);
                }
                str_replace_dup (&pRoomIndex->clan_str, clan_str);
                pRoomIndex->clan = lookup_backup (clan_lookup_exact, clan_str,
                    "Unknown clan '%s'", 0);
            }
            else if (letter == 'D') {
                EXIT_DATA *pexit;
                int locks;

                door = fread_number (fp);
                if (door < 0 || door > 5) {
                    bug ("fread_rooms: vnum %d has bad door number.", vnum);
                    exit (1);
                }

                pexit = exit_new ();
                str_replace_dup (&pexit->description, fread_string (fp));
                str_replace_dup (&pexit->keyword,     fread_string (fp));
                locks = fread_number (fp);
                pexit->key = fread_number (fp);
                pexit->vnum = fread_number (fp);
                pexit->area_vnum = -1;
                pexit->room_anum = -1;
                pexit->orig_door = door;    /* OLC */

                switch (locks) {
                    case 0:
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
                EXTRA_DESCR_DATA *ed = extra_descr_new ();
                ed = alloc_perm (sizeof (*ed));
                str_replace_dup (&ed->keyword,     fread_string (fp));
                str_replace_dup (&ed->description, fread_string (fp));
                LIST_BACK (ed, next, pRoomIndex->extra_descr, EXTRA_DESCR_DATA);
            }
            else if (letter == 'O') {
                if (pRoomIndex->owner[0] != '\0') {
                    bug ("load_rooms: duplicate owner.", 0);
                    exit (1);
                }
                str_replace_dup (&pRoomIndex->owner, fread_string (fp));
            }
            else {
                bug ("load_rooms: vnum %d has flag not 'DES'.", vnum);
                exit (1);
            }
        }

        iHash = vnum % MAX_KEY_HASH;
        LIST_FRONT (pRoomIndex, next, room_index_hash[iHash]);
        top_vnum_room = top_vnum_room < vnum ? vnum : top_vnum_room; /* OLC */
        assign_area_vnum (vnum); /* OLC */
    }
}

/* Snarf a shop section. */
void load_shops (FILE * fp) {
    SHOP_DATA *pShop;
    int keeper = 0;

    while (1) {
        MOB_INDEX_DATA *pMobIndex;
        int iTrade;

        // ROM mem leak fix, check the keeper before allocating the memory
        // to the SHOP_DATA variable.  -Rhien
        keeper = fread_number(fp);
        if (keeper == 0)
            break;

        // Now that we have a non zero keeper number we can allocate
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
void load_specials (FILE * fp) {
    char *func_name;
    while (1) {
        MOB_INDEX_DATA *pMobIndex;
        char letter;

        switch (letter = fread_letter (fp)) {
            default:
                bug ("load_specials: letter '%c' not *MS.", letter);
                exit (1);

            case 'S':
                return;

            case '*':
                break;

            case 'M':
                pMobIndex = get_mob_index (fread_number (fp));
                func_name = fread_word (fp);
                pMobIndex->spec_fun = spec_lookup_function (func_name);
                if (pMobIndex->spec_fun == NULL) {
                    bugf ("Unknown special function '%s'", func_name);
                    exit (1);
                }
                break;
        }

        fread_to_eol (fp);
    }
}

void fix_exit_doors (ROOM_INDEX_DATA *room_from, int dir_from,
                     ROOM_INDEX_DATA *room_to,   int dir_to)
{
    EXIT_DATA *exit_from = room_from->exit[dir_from];
    EXIT_DATA *exit_to   = room_to  ->exit[dir_to];
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
        exit_from->key != -1 &&
        exit_to->key != -1)
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
    char buf[MAX_STRING_LENGTH];
    ROOM_INDEX_DATA *pRoomIndex;
    ROOM_INDEX_DATA *to_room;
    EXIT_DATA *pexit;
    EXIT_DATA *pexit_rev;
    RESET_DATA *pReset;
    ROOM_INDEX_DATA *iLastRoom, *iLastObj;
    int iHash;
    int door;

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
                switch (pReset->command) {
                    case 'M':
                        get_mob_index (pReset->value[1]);
                        iLastRoom = get_room_index (pReset->value[3]);
                        break;

                    case 'O':
                        get_obj_index (pReset->value[1]);
                        iLastObj = get_room_index (pReset->value[3]);
                        break;

                    case 'P':
                        get_obj_index (pReset->value[1]);
                        if (iLastObj == NULL) {
                            bugf
                                ("fix_exits : reset in room %d with iLastObj NULL",
                                 pRoomIndex->vnum);
                            exit (1);
                        }
                        break;

                    case 'G':
                    case 'E':
                        get_obj_index (pReset->value[1]);
                        if (iLastRoom == NULL) {
                            bugf ("fix_exits : reset in room %d with iLastRoom NULL",
                                 pRoomIndex->vnum);
                            exit (1);
                        }
                        iLastObj = iLastRoom;
                        break;

                    case 'D':
                        bugf ("???");
                        break;

                    case 'R':
                        get_room_index (pReset->value[1]);
                        if (pReset->value[2] < 0 || pReset->value[2] > DIR_MAX) {
                            bugf ("fix_exits : reset in room %d with values[2] %d > DIR_MAX",
                                 pRoomIndex->vnum, pReset->value[2]);
                            exit (1);
                        }
                        break;

                    default:
                        bugf ("fix_exits : room %d with reset cmd %c",
                              pRoomIndex->vnum, pReset->command);
                        exit (1);
                        break;
                }
            }

            fexit = FALSE;
            for (door = 0; door <= 5; door++) {
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
            }
            if (!fexit)
                SET_BIT (pRoomIndex->room_flags, ROOM_NO_MOB);
        }
    }

    for (iHash = 0; iHash < MAX_KEY_HASH; iHash++) {
        for (pRoomIndex = room_index_hash[iHash];
             pRoomIndex != NULL; pRoomIndex = pRoomIndex->next)
        {
            for (door = 0; door <= 5; door++) {
                int rev_dir;
                if ((pexit = pRoomIndex->exit[door]) == NULL)
                    continue;
                if ((to_room = pexit->to_room) == NULL)
                    continue;
                rev_dir = REV_DIR(door);
                if ((pexit_rev = to_room->exit[rev_dir]) == NULL)
                    continue;

                fix_exit_doors (pRoomIndex, door, to_room, rev_dir);

                if (pexit_rev->to_room != pRoomIndex &&
                    (pRoomIndex->vnum < 1200 || pRoomIndex->vnum > 1299))
                {
                    sprintf (buf, "Fix_exits: %d:%d -> %d:%d -> %d.",
                             pRoomIndex->vnum, door, to_room->vnum, rev_dir,
                             (pexit_rev->to_room == NULL)
                             ? 0 : pexit_rev->to_room->vnum);
                    bug (buf, 0);
                }
            }
        }
    }
}

/* Load mobprogs section */
void load_mobprogs (FILE * fp) {
    MPROG_CODE *pMprog;

    if (area_last == NULL) {
        bug ("load_mobprogs: no #AREA seen yet.", 0);
        exit (1);
    }

    while (1) {
        sh_int vnum;
        char letter;

        letter = fread_letter (fp);
        if (letter != '#') {
            bug ("load_mobprogs: # not found.", 0);
            exit (1);
        }

        vnum = fread_number (fp);
        if (vnum == 0)
            break;

        fBootDb = FALSE;
        if (get_mprog_index (vnum) != NULL) {
            bug ("load_mobprogs: vnum %d duplicated.", vnum);
            exit (1);
        }
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
    MOB_INDEX_DATA *pMobIndex;
    MPROG_LIST *list;
    MPROG_CODE *prog;
    int iHash;

    for (iHash = 0; iHash < MAX_KEY_HASH; iHash++) {
        for (pMobIndex = mob_index_hash[iHash];
             pMobIndex != NULL; pMobIndex = pMobIndex->next)
        {
            for (list = pMobIndex->mprogs; list != NULL; list = list->next) {
                if ((prog = get_mprog_index (list->vnum)) != NULL)
                    list->code = prog->code;
                else {
                    bug ("fix_mobprogs: code vnum %d not found.", list->vnum);
                    exit (1);
                }
            }
        }
    }
}

/* Repopulate areas periodically. */
void area_update (void) {
    AREA_DATA *pArea;
    char buf[MAX_STRING_LENGTH];

    for (pArea = area_first; pArea != NULL; pArea = pArea->next) {
        if (++pArea->age < 3)
            continue;

        /* Check age and reset.
         * Note: Mud School resets every 3 minutes (not 15). */
        if ((!pArea->empty && (pArea->nplayer == 0 || pArea->age >= 15))
            || pArea->age >= 31)
        {
            ROOM_INDEX_DATA *pRoomIndex;

            reset_area (pArea);
            sprintf (buf, "%s has just been reset.", pArea->title);
            wiznet (buf, NULL, NULL, WIZ_RESETS, 0, 0);

            pArea->age = number_range (0, 3);
            pRoomIndex = get_room_index (ROOM_VNUM_SCHOOL);
            if (pRoomIndex != NULL && pArea == pRoomIndex->area)
                pArea->age = 15 - 2;
            else if (pArea->nplayer == 0)
                pArea->empty = TRUE;
        }
    }
}

/* OLC
 * Reset one room.  Called by reset_area and olc. */
void reset_room (ROOM_INDEX_DATA * pRoom) {
    RESET_DATA *pReset;
    CHAR_DATA *pMob;
    CHAR_DATA *mob;
    OBJ_DATA *pObj;
    CHAR_DATA *LastMob = NULL;
    OBJ_DATA *LastObj = NULL;
    int iExit;
    int level = 0;
    bool last;

    if (!pRoom)
        return;
    pMob = NULL;
    last = FALSE;

    for (iExit = 0; iExit < DIR_MAX; iExit++) {
        EXIT_DATA *pExit, *pRev;
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

    for (pReset = pRoom->reset_first; pReset != NULL; pReset = pReset->next) {
        MOB_INDEX_DATA *pMobIndex;
        OBJ_INDEX_DATA *pObjIndex;
        OBJ_INDEX_DATA *pObjToIndex;
        ROOM_INDEX_DATA *pRoomIndex;
        ROOM_INDEX_DATA *pRoomIndexPrev;
        char buf[MAX_STRING_LENGTH];
        int count, limit = 0;

        switch (pReset->command) {
            default:
                bug ("reset_room: bad command %c.", pReset->command);
                break;

            case 'M':
                if (!(pMobIndex = get_mob_index (pReset->value[1]))) {
                    bug ("reset_room: 'M': bad vnum %d.", pReset->value[1]);
                    continue;
                }
                if ((pRoomIndex = get_room_index (pReset->value[3])) == NULL) {
                    bug ("reset_room: 'M': bad vnum %d.", pReset->value[3]);
                    continue;
                }
                if (pMobIndex->count >= pReset->value[2]) {
                    last = FALSE;
                    break;
                }

                count = 0;
                for (mob = pRoomIndex->people; mob != NULL;
                     mob = mob->next_in_room) if (mob->pIndexData == pMobIndex)
                {
                    count++;
                    if (count >= pReset->value[4]) {
                        last = FALSE;
                        break;
                    }
                }
                if (count >= pReset->value[4])
                    break;

                pMob = create_mobile (pMobIndex);

                /* Some more hard coding. */
                if (room_is_dark (pRoom))
                    SET_BIT (pMob->affected_by, AFF_INFRARED);

                /* Pet shop mobiles get ACT_PET set. */
                pRoomIndexPrev = get_room_index (pRoom->vnum - 1);
                if (pRoomIndexPrev
                    && IS_SET (pRoomIndexPrev->room_flags, ROOM_PET_SHOP))
                    SET_BIT (pMob->act, ACT_PET);

                char_to_room (pMob, pRoom);
                LastMob = pMob;
                level = URANGE (0, pMob->level - 2, LEVEL_HERO - 1);    /* -1 ROM */
                last = TRUE;
                break;

            case 'O': {
                int obj_count = UMAX(1, pReset->value[0]);

                if (!(pObjIndex = get_obj_index (pReset->value[1]))) {
                    bug ("reset_room: 'O' 1 : bad vnum %d", pReset->value[1]);
                    sprintf (buf, "%d %d %d %d", pReset->value[1], pReset->value[2],
                             pReset->value[3], pReset->value[4]);
                    bug (buf, 1);
                    continue;
                }
                if (!(pRoomIndex = get_room_index (pReset->value[3]))) {
                    bug ("reset_room: 'O' 2 : bad vnum %d.", pReset->value[3]);
                    sprintf (buf, "%d %d %d %d", pReset->value[1], pReset->value[2],
                             pReset->value[3], pReset->value[4]);
                    bug (buf, 1);
                    continue;
                }

                if (pRoom->area->nplayer > 0 ||
                    obj_index_count_in_list (pObjIndex, pRoom->contents) >= obj_count)
                {
                    last = FALSE;
                    break;
                }

                pObj = create_object (pObjIndex,    /* UMIN - ROM OLC */
                   UMIN (number_fuzzy (level), LEVEL_HERO - 1));
                if (pObj->item_type != ITEM_TREASURE)
                    pObj->cost = 0;
                obj_to_room (pObj, pRoom);
                last = TRUE;
                break;
            }

            case 'P':
                if (!(pObjIndex = get_obj_index (pReset->value[1]))) {
                    bug ("reset_room: 'P': bad vnum %d.", pReset->value[1]);
                    continue;
                }

                if (!(pObjToIndex = get_obj_index (pReset->value[3]))) {
                    bug ("reset_room: 'P': bad vnum %d.", pReset->value[3]);
                    continue;
                }

                if (pReset->value[2] > 50) /* old format */
                    limit = 6;
                else if (pReset->value[2] == -1) /* no limit */
                    limit = 999;
                else
                    limit = pReset->value[2];

                if (pRoom->area->nplayer > 0
                    || (LastObj = obj_get_by_index (pObjToIndex)) == NULL
                    || (LastObj->in_room == NULL && !last)
                    || (pObjIndex->count >=
                        limit /* && number_range(0,4) != 0 */ )
                    || (count =
                        obj_index_count_in_list (pObjIndex,
                                        LastObj->contains)) > pReset->value[4])
                {
                    last = FALSE;
                    break;
                }
                /* lastObj->level  -  ROM */

                while (count < pReset->value[4]) {
                    pObj = create_object (pObjIndex,
                                       number_fuzzy (LastObj->level));
                    obj_to_obj (pObj, LastObj);
                    count++;
                    if (pObjIndex->count >= limit)
                        break;
                }

                /* fix object lock state! */
                LastObj->value[1] = LastObj->pIndexData->value[1];
                last = TRUE;
                break;

            case 'G':
            case 'E':
                if (!(pObjIndex = get_obj_index (pReset->value[1]))) {
                    bug ("reset_room: 'E' or 'G': bad vnum %d.",
                         pReset->value[1]);
                    continue;
                }

                if (!last)
                    break;

                if (!LastMob) {
                    bug ("reset_room: 'E' or 'G': null mob for vnum %d.",
                         pReset->value[1]);
                    last = FALSE;
                    break;
                }

                if (LastMob->pIndexData->pShop) { /* Shop-keeper? */
                    int olevel = 0, i, j;
                    if (!pObjIndex->new_format) {
                        switch (pObjIndex->item_type) {
                            default:
                                olevel = 0;
                                break;

                            case ITEM_PILL:
                            case ITEM_POTION:
                            case ITEM_SCROLL:
                                olevel = 53;
                                for (i = 1; i < 5; i++) {
                                    if (pObjIndex->value[i] > 0) {
                                        for (j = 0; j < CLASS_MAX; j++) {
                                            olevel = UMIN (olevel,
                                                skill_table[pObjIndex->value[i]].skill_level[j]);
                                        }
                                    }
                                }
                                olevel = UMAX (0, (olevel * 3 / 4) - 2);
                                break;

                            case ITEM_WAND:
                                olevel = number_range (10, 20);
                                break;
                            case ITEM_STAFF:
                                olevel = number_range (15, 25);
                                break;
                            case ITEM_ARMOR:
                                olevel = number_range (5, 15);
                                break;
                            /* ROM patch weapon, treasure */
                            case ITEM_WEAPON:
                                olevel = number_range (5, 15);
                                break;
                            case ITEM_TREASURE:
                                olevel = number_range (10, 20);
                                break;

#if 0                       /* envy version */
                            case ITEM_WEAPON:
                                if (pReset->command == 'G')
                                    olevel = number_range (5, 15);
                                else
                                    olevel = number_fuzzy (level);
#endif /* envy version */

                                break;
                        }
                    }

                    pObj = create_object (pObjIndex, olevel);
                    SET_BIT (pObj->extra_flags, ITEM_INVENTORY);    /* ROM OLC */

#if 0               /* envy version */
                    if (pReset->command == 'G')
                        SET_BIT (pObj->extra_flags, ITEM_INVENTORY);
#endif /* envy version */

                }
                else { /* ROM OLC else version */
                    int limit;
                    if (pReset->value[2] > 50) /* old format */
                        limit = 6;
                    else if (pReset->value[2] == -1 || pReset->value[2] == 0) /* no limit */
                        limit = 999;
                    else
                        limit = pReset->value[2];

                    if (pObjIndex->count < limit || number_range (0, 4) == 0) {
                        pObj = create_object (pObjIndex,
                                              UMIN (number_fuzzy (level),
                                                    LEVEL_HERO - 1));
                        /* error message if it is too high */
                        if (pObj->level > LastMob->level + 3
                            || (pObj->item_type == ITEM_WEAPON
                                && pReset->command == 'E'
                                && pObj->level < LastMob->level - 5
                                && pObj->level < 45)) {
                            log_f ("Check Levels:");
                            log_f ("  Object: (VNUM %5d)(Level %2d) %s",
                                     pObj->pIndexData->vnum,
                                     pObj->level,
                                     pObj->short_descr);
                            log_f ("     Mob: (VNUM %5d)(Level %2d) %s",
                                     LastMob->pIndexData->vnum,
                                     LastMob->level,
                                     LastMob->short_descr);
                        }
                    }
                    else
                        break;
                }

                obj_to_char (pObj, LastMob);
                if (pReset->command == 'E')
                    char_equip (LastMob, pObj, pReset->value[3]);
                last = TRUE;
                break;

            case 'D':
                break;

            case 'R':
                if (!(pRoomIndex = get_room_index (pReset->value[1]))) {
                    bug ("reset_room: 'R': bad vnum %d.", pReset->value[1]);
                    continue;
                }

                {
                    EXIT_DATA *pExit;
                    int d0;
                    int d1;

                    for (d0 = 0; d0 < pReset->value[2] - 1; d0++) {
                        d1 = number_range (d0, pReset->value[2] - 1);
                        pExit = pRoomIndex->exit[d0];
                        pRoomIndex->exit[d0] = pRoomIndex->exit[d1];
                        pRoomIndex->exit[d1] = pExit;
                    }
                }
                break;
        }
    }
}

/* OLC
 * Reset one area. */
void reset_area (AREA_DATA * pArea) {
    ROOM_INDEX_DATA *pRoom;
    int vnum;
    for (vnum = pArea->min_vnum; vnum <= pArea->max_vnum; vnum++)
        if ((pRoom = get_room_index (vnum)))
            reset_room (pRoom);
}

/* Create an instance of a mobile. */
CHAR_DATA *create_mobile (MOB_INDEX_DATA * pMobIndex) {
    CHAR_DATA *mob;
    int i;
    AFFECT_DATA af;

    mobile_count++;
    if (pMobIndex == NULL) {
        bug ("create_mobile: NULL pMobIndex.", 0);
        exit (1);
    }

    mob = char_new ();

    mob->pIndexData   = pMobIndex;
    str_replace_dup (&mob->name,        pMobIndex->name);        /* OLC */
    str_replace_dup (&mob->short_descr, pMobIndex->short_descr); /* OLC */
    str_replace_dup (&mob->long_descr,  pMobIndex->long_descr);  /* OLC */
    str_replace_dup (&mob->description, pMobIndex->description); /* OLC */
    mob->id           = get_mob_id ();
    mob->spec_fun     = pMobIndex->spec_fun;
    mob->prompt       = NULL;
    mob->mprog_target = NULL;
    mob->class        = CLASS_NONE;

    if (pMobIndex->wealth == 0) {
        mob->silver = 0;
        mob->gold = 0;
    }
    else {
        long wealth;
        wealth = number_range (pMobIndex->wealth / 2, 3 * pMobIndex->wealth / 2);
        mob->gold = number_range (wealth / 200, wealth / 100);
        mob->silver = wealth - (mob->gold * 100);
    }

    /* load in new style */
    if (pMobIndex->new_format) {
        /* read from prototype */
        mob->group    = pMobIndex->group;
        mob->act      = pMobIndex->act;
        mob->comm     = COMM_NOCHANNELS | COMM_NOSHOUT | COMM_NOTELL;
        mob->affected_by = pMobIndex->affected_by;
        mob->alignment = pMobIndex->alignment;
        mob->level    = pMobIndex->level;
        mob->hitroll  = pMobIndex->hitroll;
        mob->damroll  = pMobIndex->damage[DICE_BONUS];
        mob->max_hit  = dice (pMobIndex->hit[DICE_NUMBER],
                             pMobIndex->hit[DICE_TYPE]) + pMobIndex->hit[DICE_BONUS];
        mob->hit      = mob->max_hit;
        mob->max_mana = dice (pMobIndex->mana[DICE_NUMBER],
                              pMobIndex->mana[DICE_TYPE]) + pMobIndex->mana[DICE_BONUS];
        mob->mana     = mob->max_mana;
        mob->damage[DICE_NUMBER] = pMobIndex->damage[DICE_NUMBER];
        mob->damage[DICE_TYPE] = pMobIndex->damage[DICE_TYPE];
        mob->dam_type = pMobIndex->dam_type;

        if (mob->dam_type == 0) {
            switch (number_range (1, 3)) {
                case (1): mob->dam_type = 3;  break; /* slash */
                case (2): mob->dam_type = 7;  break; /* pound */
                case (3): mob->dam_type = 11; break; /* pierce */
            }
        }
        for (i = 0; i < 4; i++)
            mob->armor[i] = pMobIndex->ac[i];
        mob->off_flags   = pMobIndex->off_flags;
        mob->imm_flags   = pMobIndex->imm_flags;
        mob->res_flags   = pMobIndex->res_flags;
        mob->vuln_flags  = pMobIndex->vuln_flags;
        mob->start_pos   = pMobIndex->start_pos;
        mob->default_pos = pMobIndex->default_pos;

        mob->sex = pMobIndex->sex;
        if (mob->sex == 3) /* random sex */
            mob->sex = number_range (1, 2);
        mob->race     = pMobIndex->race;
        mob->form     = pMobIndex->form;
        mob->parts    = pMobIndex->parts;
        mob->size     = pMobIndex->size;
        mob->material = pMobIndex->material;

        /* computed on the spot */
        for (i = 0; i < STAT_MAX; i++)
            mob->perm_stat[i] = UMIN (25, 11 + mob->level / 4);

        if (IS_SET (mob->act, ACT_WARRIOR)) {
            mob->perm_stat[STAT_STR] += 3;
            mob->perm_stat[STAT_INT] -= 1;
            mob->perm_stat[STAT_CON] += 2;
        }
        if (IS_SET (mob->act, ACT_THIEF)) {
            mob->perm_stat[STAT_DEX] += 3;
            mob->perm_stat[STAT_INT] += 1;
            mob->perm_stat[STAT_WIS] -= 1;
        }
        if (IS_SET (mob->act, ACT_CLERIC)) {
            mob->perm_stat[STAT_WIS] += 3;
            mob->perm_stat[STAT_DEX] -= 1;
            mob->perm_stat[STAT_STR] += 1;
        }
        if (IS_SET (mob->act, ACT_MAGE)) {
            mob->perm_stat[STAT_INT] += 3;
            mob->perm_stat[STAT_STR] -= 1;
            mob->perm_stat[STAT_DEX] += 1;
        }
        if (IS_SET (mob->off_flags, OFF_FAST))
            mob->perm_stat[STAT_DEX] += 2;

        mob->perm_stat[STAT_STR] += mob->size - SIZE_MEDIUM;
        mob->perm_stat[STAT_CON] += (mob->size - SIZE_MEDIUM) / 2;

        /* let's get some spell action */
        if (IS_AFFECTED (mob, AFF_SANCTUARY)) {
            affect_init (&af, TO_AFFECTS, skill_lookup_exact ("sanctuary"), mob->level, -1, APPLY_NONE, 0, AFF_SANCTUARY);
            affect_to_char (mob, &af);
        }
        if (IS_AFFECTED (mob, AFF_HASTE)) {
            affect_init (&af, TO_AFFECTS, skill_lookup_exact ("haste"), mob->level, -1, APPLY_DEX, 1 + (mob->level >= 18) + (mob->level >= 25) + (mob->level >= 32), AFF_HASTE);
            affect_to_char (mob, &af);
        }
        if (IS_AFFECTED (mob, AFF_PROTECT_EVIL)) {
            affect_init (&af, TO_AFFECTS, skill_lookup_exact ("protection evil"), mob->level, -1, APPLY_SAVES, -1, AFF_PROTECT_EVIL);
            affect_to_char (mob, &af);
        }
        if (IS_AFFECTED (mob, AFF_PROTECT_GOOD)) {
            affect_init (&af, TO_AFFECTS, skill_lookup_exact ("protection good"), mob->level, -1, APPLY_SAVES, -1, AFF_PROTECT_GOOD);
            affect_to_char (mob, &af);
        }
    }
    /* read in old format and convert */
    else {
        mob->act         = pMobIndex->act;
        mob->affected_by = pMobIndex->affected_by;
        mob->alignment   = pMobIndex->alignment;
        mob->level       = pMobIndex->level;
        mob->hitroll     = pMobIndex->hitroll;
        mob->damroll     = 0;
        mob->max_hit     = (mob->level * 8 + number_range
            (mob->level * mob->level / 4, mob->level * mob->level)) * 9 / 10;
        mob->hit         = mob->max_hit;
        mob->max_mana    = 100 + dice (mob->level, 10);
        mob->mana        = mob->max_mana;
        switch (number_range (1, 3)) {
            case (1): mob->dam_type = 3;  break; /* slash */
            case (2): mob->dam_type = 7;  break; /* pound */
            case (3): mob->dam_type = 11; break; /* pierce */
        }
        for (i = 0; i < 3; i++)
            mob->armor[i] = interpolate (mob->level, 100, -100);
        mob->armor[3]    = interpolate (mob->level, 100, 0);
        mob->race        = pMobIndex->race;
        mob->off_flags   = pMobIndex->off_flags;
        mob->imm_flags   = pMobIndex->imm_flags;
        mob->res_flags   = pMobIndex->res_flags;
        mob->vuln_flags  = pMobIndex->vuln_flags;
        mob->start_pos   = pMobIndex->start_pos;
        mob->default_pos = pMobIndex->default_pos;
        mob->sex         = pMobIndex->sex;
        mob->form        = pMobIndex->form;
        mob->parts       = pMobIndex->parts;
        mob->size        = SIZE_MEDIUM;

        for (i = 0; i < STAT_MAX; i++)
            mob->perm_stat[i] = 11 + mob->level / 4;
    }
    mob->position = mob->start_pos;

    /* link the mob to the world list */
    LIST_FRONT (mob, next, char_list);
    pMobIndex->count++;
    return mob;
}

/* duplicate a mobile exactly -- except inventory */
void clone_mobile (CHAR_DATA * parent, CHAR_DATA * clone) {
    int i;
    AFFECT_DATA *paf;

    if (parent == NULL || clone == NULL || !IS_NPC (parent))
        return;

    /* start fixing values */
    str_replace_dup (&clone->name, parent->name);
    clone->version     = parent->version;
    str_replace_dup (&clone->short_descr, parent->short_descr);
    str_replace_dup (&clone->long_descr,  parent->long_descr);
    str_replace_dup (&clone->description, parent->description);
    clone->group       = parent->group;
    clone->sex         = parent->sex;
    clone->class       = parent->class;
    clone->race        = parent->race;
    clone->level       = parent->level;
    clone->trust       = 0;
    clone->timer       = parent->timer;
    clone->wait        = parent->wait;
    clone->hit         = parent->hit;
    clone->max_hit     = parent->max_hit;
    clone->mana        = parent->mana;
    clone->max_mana    = parent->max_mana;
    clone->move        = parent->move;
    clone->max_move    = parent->max_move;
    clone->gold        = parent->gold;
    clone->silver      = parent->silver;
    clone->exp         = parent->exp;
    clone->act         = parent->act;
    clone->comm        = parent->comm;
    clone->imm_flags   = parent->imm_flags;
    clone->res_flags   = parent->res_flags;
    clone->vuln_flags  = parent->vuln_flags;
    clone->invis_level = parent->invis_level;
    clone->affected_by = parent->affected_by;
    clone->position    = parent->position;
    clone->practice    = parent->practice;
    clone->train       = parent->train;
    clone->saving_throw = parent->saving_throw;
    clone->alignment   = parent->alignment;
    clone->hitroll     = parent->hitroll;
    clone->damroll     = parent->damroll;
    clone->wimpy       = parent->wimpy;
    clone->form        = parent->form;
    clone->parts       = parent->parts;
    clone->size        = parent->size;
    clone->material    = parent->material;
    clone->off_flags   = parent->off_flags;
    clone->dam_type    = parent->dam_type;
    clone->start_pos   = parent->start_pos;
    clone->default_pos = parent->default_pos;
    clone->spec_fun    = parent->spec_fun;

    for (i = 0; i < 4; i++)
        clone->armor[i] = parent->armor[i];

    for (i = 0; i < STAT_MAX; i++) {
        clone->perm_stat[i] = parent->perm_stat[i];
        clone->mod_stat[i]  = parent->mod_stat[i];
    }

    for (i = 0; i < 3; i++)
        clone->damage[i] = parent->damage[i];

    /* now add the affects */
    for (paf = parent->affected; paf != NULL; paf = paf->next)
        affect_to_char (clone, paf);
}

/* Create an instance of an object. */
OBJ_DATA *create_object (OBJ_INDEX_DATA * pObjIndex, int level) {
    AFFECT_DATA *paf;
    OBJ_DATA *obj;
    int i;

    if (pObjIndex == NULL) {
        bug ("create_object: NULL pObjIndex.", 0);
        exit (1);
    }

    obj = obj_new ();

    obj->pIndexData = pObjIndex;
    obj->in_room = NULL;
    obj->enchanted = FALSE;

    if (pObjIndex->new_format)
        obj->level = pObjIndex->level;
    else
        obj->level = UMAX (0, level);
    obj->wear_loc = -1;

    str_replace_dup (&obj->name,        pObjIndex->name);        /* OLC */
    str_replace_dup (&obj->short_descr, pObjIndex->short_descr); /* OLC */
    str_replace_dup (&obj->description, pObjIndex->description); /* OLC */
    obj->material    = pObjIndex->material;
    obj->item_type   = pObjIndex->item_type;
    obj->extra_flags = pObjIndex->extra_flags;
    obj->wear_flags  = pObjIndex->wear_flags;
    obj->value[0]    = pObjIndex->value[0];
    obj->value[1]    = pObjIndex->value[1];
    obj->value[2]    = pObjIndex->value[2];
    obj->value[3]    = pObjIndex->value[3];
    obj->value[4]    = pObjIndex->value[4];
    obj->weight      = pObjIndex->weight;

    if (level == -1 || pObjIndex->new_format)
        obj->cost = pObjIndex->cost;
    else
        obj->cost = number_fuzzy (10)
            * number_fuzzy (level) * number_fuzzy (level);

    /* Mess with object properties. */
    switch (obj->item_type) {
        default:
            bug ("read_object: vnum %d bad type.", pObjIndex->vnum);
            break;

        case ITEM_LIGHT:
            if (obj->value[2] == 999)
                obj->value[2] = -1;
            break;

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
        case ITEM_PORTAL:
            if (!pObjIndex->new_format)
                obj->cost /= 5;
            break;

        case ITEM_TREASURE:
        case ITEM_WARP_STONE:
        case ITEM_ROOM_KEY:
        case ITEM_GEM:
        case ITEM_JEWELRY:
            break;

        case ITEM_JUKEBOX:
            for (i = 0; i < 5; i++)
                obj->value[i] = -1;
            break;

        case ITEM_SCROLL:
            if (level != -1 && !pObjIndex->new_format)
                obj->value[0] = number_fuzzy (obj->value[0]);
            break;

        case ITEM_WAND:
        case ITEM_STAFF:
            if (level != -1 && !pObjIndex->new_format) {
                obj->value[0] = number_fuzzy (obj->value[0]);
                obj->value[1] = number_fuzzy (obj->value[1]);
                obj->value[2] = obj->value[1];
            }
            if (!pObjIndex->new_format)
                obj->cost *= 2;
            break;

        case ITEM_WEAPON:
            if (level != -1 && !pObjIndex->new_format) {
                obj->value[1] = number_fuzzy (number_fuzzy (1 * level / 4 + 2));
                obj->value[2] = number_fuzzy (number_fuzzy (3 * level / 4 + 6));
            }
            break;

        case ITEM_ARMOR:
            if (level != -1 && !pObjIndex->new_format) {
                obj->value[0] = number_fuzzy (level / 5 + 3);
                obj->value[1] = number_fuzzy (level / 5 + 3);
                obj->value[2] = number_fuzzy (level / 5 + 3);
            }
            break;

        case ITEM_POTION:
        case ITEM_PILL:
            if (level != -1 && !pObjIndex->new_format)
                obj->value[0] = number_fuzzy (number_fuzzy (obj->value[0]));
            break;

        case ITEM_MONEY:
            if (!pObjIndex->new_format)
                obj->value[0] = obj->cost;
            break;
    }

    for (paf = pObjIndex->affected; paf != NULL; paf = paf->next)
        if (paf->apply == APPLY_SPELL_AFFECT)
            affect_to_obj (obj, paf);

    LIST_FRONT (obj, next, object_list);
    pObjIndex->count++;
    return obj;
}

/* duplicate an object exactly -- except contents */
void clone_object (OBJ_DATA * parent, OBJ_DATA * clone) {
    int i;
    AFFECT_DATA *paf;
    EXTRA_DESCR_DATA *ed, *ed_new;

    if (parent == NULL || clone == NULL)
        return;

    /* start fixing the object */
    str_replace_dup (&clone->name,        parent->name);
    str_replace_dup (&clone->short_descr, parent->short_descr);
    str_replace_dup (&clone->description, parent->description);
    clone->item_type   = parent->item_type;
    clone->extra_flags = parent->extra_flags;
    clone->wear_flags  = parent->wear_flags;
    clone->weight      = parent->weight;
    clone->cost        = parent->cost;
    clone->level       = parent->level;
    clone->condition   = parent->condition;
    clone->material    = parent->material;
    clone->timer       = parent->timer;

    for (i = 0; i < 5; i++)
        clone->value[i] = parent->value[i];

    /* affects */
    clone->enchanted = parent->enchanted;

    for (paf = parent->affected; paf != NULL; paf = paf->next)
        affect_to_obj (clone, paf);

    /* extended desc */
    for (ed = parent->extra_descr; ed != NULL; ed = ed->next) {
        ed_new = extra_descr_new ();
        str_replace_dup (&ed_new->keyword, ed->keyword);
        str_replace_dup (&ed_new->description, ed->description);
        LIST_BACK (ed_new, next, clone->extra_descr, EXTRA_DESCR_DATA);
    }
}

/* Clear a new character. */
void clear_char (CHAR_DATA * ch) {
    static CHAR_DATA ch_zero;
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
char *get_extra_descr (const char *name, EXTRA_DESCR_DATA * ed) {
    for (; ed != NULL; ed = ed->next)
        if (is_name ((char *) name, ed->keyword))
            return ed->description;
    return NULL;
}

/* Translates mob virtual number to its mob index struct.
 * Hash table lookup. */
MOB_INDEX_DATA *get_mob_index (int vnum) {
    MOB_INDEX_DATA *pMobIndex;

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
        return NULL;
    }

    return NULL;
}

/* Translates mob virtual number to its obj index struct.
 * Hash table lookup. */
OBJ_INDEX_DATA *get_obj_index (int vnum) {
    OBJ_INDEX_DATA *pObjIndex;

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
        return NULL;
    }

    return NULL;
}

/* Translates mob virtual number to its room index struct.
 * Hash table lookup. */
ROOM_INDEX_DATA *get_room_index (int vnum) {
    ROOM_INDEX_DATA *pRoomIndex;

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
        return NULL;
    }

    return NULL;
}

MPROG_CODE *get_mprog_index (int vnum) {
    MPROG_CODE *prg;
    for (prg = mprog_list; prg; prg = prg->next)
        if (prg->vnum == vnum)
            return (prg);
    return NULL;
}

/* Read a letter from a file.  */
char fread_letter (FILE * fp) {
    char c;
    do {
        c = getc (fp);
    } while (isspace (c));
    return c;
}

/* Read a number from a file. */
int fread_number (FILE * fp) {
    int number = 0;
    bool sign = FALSE;
    char c = fread_letter(fp);

    if (c == '+')
        c = getc (fp);
    else if (c == '-') {
        sign = TRUE;
        c = getc (fp);
    }

    if (!isdigit (c)) {
        bug ("fread_number: bad format.", 0);
        exit (1);
    }

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

flag_t fread_flag (FILE * fp) {
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
char *fread_string (FILE * fp) {
    char *plast;
    char c;

    plast = top_string + sizeof (char *);
    if (plast > &string_space[MAX_STRING - MAX_STRING_LENGTH]) {
        bug ("fread_string: MAX_STRING %d exceeded.", MAX_STRING);
        exit (1);
    }

    c = fread_letter(fp);
    if ((*plast++ = c) == '~')
        return &str_empty[0];

    while (1) {
        /* Back off the char type lookup,
         * it was too dirty for portability.
         *   -- Furey */

        switch (*plast = getc (fp)) {
            default:
                plast++;
                break;

            case EOF:
                /* temp fix */
                bug ("fread_string: EOF", 0);
                return NULL;
                /* exit( 1 ); */
                break;

            case '\n':
                plast++;
                *plast++ = '\r';
                break;

            case '\r':
                break;

            case '~': {
                union {
                    char *pc;
                    char rgc[sizeof (char *)];
                } u1;

                int ic;
                int iHash;
                char *pHash;
                char *pHashPrev;
                char *pString;

                plast++;
                plast[-1] = '\0';
                iHash = UMIN (MAX_KEY_HASH - 1, plast - 1 - top_string);
                for (pHash = string_hash[iHash]; pHash; pHash = pHashPrev) {
                    for (ic = 0; ic < sizeof (char *); ic++)
                        u1.rgc[ic] = pHash[ic];
                    pHashPrev = u1.pc;
                    pHash += sizeof (char *);

                    if (top_string[sizeof (char *)] == pHash[0]
                        && !strcmp (top_string + sizeof (char *) + 1,
                                    pHash + 1))
                        return pHash;
                }

                if (fBootDb) {
                    pString = top_string;
                    top_string = plast;
                    u1.pc = string_hash[iHash];
                    for (ic = 0; ic < sizeof (char *); ic++)
                        pString[ic] = u1.rgc[ic];
                    string_hash[iHash] = pString;

                    nAllocString += 1;
                    sAllocString += top_string - pString;
                    return pString + sizeof (char *);
                }
                else
                    return str_dup (top_string + sizeof (char *));
                break;
            }
        }
    }
}

char *fread_string_eol (FILE * fp) {
    static bool char_special[256 - EOF];
    char *plast;
    char c;

    if (char_special[EOF - EOF] != TRUE) {
        char_special[EOF - EOF]  = TRUE;
        char_special['\n' - EOF] = TRUE;
        char_special['\r' - EOF] = TRUE;
    }

    plast = top_string + sizeof (char *);
    if (plast > &string_space[MAX_STRING - MAX_STRING_LENGTH]) {
        bug ("fread_string: MAX_STRING %d exceeded.", MAX_STRING);
        exit (1);
    }

    c = fread_letter(fp);
    if ((*plast++ = c) == '\n')
        return &str_empty[0];

    while (1) {
        if (!char_special[(*plast++ = getc (fp)) - EOF])
            continue;

        switch (plast[-1]) {
            default:
                break;

            case EOF:
                bug ("fread_string_eol  EOF", 0);
                exit (1);
                break;

            case '\n':
            case '\r': {
                union {
                    char *pc;
                    char rgc[sizeof (char *)];
                } u1;

                int ic;
                int iHash;
                char *pHash;
                char *pHashPrev;
                char *pString;

                plast[-1] = '\0';
                iHash = UMIN (MAX_KEY_HASH - 1, plast - 1 - top_string);
                for (pHash = string_hash[iHash]; pHash; pHash = pHashPrev) {
                    for (ic = 0; ic < sizeof (char *); ic++)
                        u1.rgc[ic] = pHash[ic];
                    pHashPrev = u1.pc;
                    pHash += sizeof (char *);

                    if (top_string[sizeof (char *)] == pHash[0]
                        && !strcmp (top_string + sizeof (char *) + 1,
                                    pHash + 1))
                        return pHash;
                }

                if (fBootDb) {
                    pString = top_string;
                    top_string = plast;
                    u1.pc = string_hash[iHash];
                    for (ic = 0; ic < sizeof (char *); ic++)
                        pString[ic] = u1.rgc[ic];
                    string_hash[iHash] = pString;

                    nAllocString += 1;
                    sAllocString += top_string - pString;
                    return pString + sizeof (char *);
                }
                else
                    return str_dup (top_string + sizeof (char *));
                break;
            }
        }
    }
}

/* Read to end of line (for comments). */
void fread_to_eol (FILE * fp) {
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
char *fread_word (FILE * fp) {
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

    bug ("fread_word: word too long.", 0);
    exit (1);
    return NULL;
}

void fread_dice (FILE *fp, sh_int *out) {
    out[DICE_NUMBER] = fread_number (fp);
    /* 'd'          */ fread_letter (fp);
    out[DICE_TYPE]   = fread_number (fp);
    /* '+'          */ fread_letter (fp);
    out[DICE_BONUS]  = fread_number (fp);
}

char *memory_dump (char *eol) {
    static char buf[MAX_STRING_LENGTH];
    const RECYCLE_TYPE *rec;
    int i;
    size_t size, len;

    buf[0] = '\0';
    size = sizeof(buf);
    len = 0;

    for (i = 0; i < RECYCLE_MAX; i++) {
        rec = &(recycle_table[i]);
        len += snprintf (buf + len, size - len,
            "%-11s %5d (%8ld bytes, %d:%d in use:freed)%s", rec->name,
            rec->top, rec->top * rec->size, rec->list_count, rec->free_count,
            eol);
    }
    len += snprintf (buf + len, size - len,
        "strings     %5d of %7d bytes (max %d)%s",
        nAllocString, sAllocString, MAX_STRING, eol);
    len += snprintf (buf + len, size - len,
        "blocks      %5d of %7d bytes (%d pages)%s",
        nAllocPerm, sAllocPerm, nMemPermCount, eol);

    return buf;
}

/* Added this as part of a bugfix suggested by Chris Litchfield (of
 * "The Mage's Lair" fame). Pets were being loaded with any saved
 * affects, then having those affects loaded again. -- JR 2002/01/31 */
bool check_pet_affected(int vnum, AFFECT_DATA *paf) {
    MOB_INDEX_DATA *petIndex;

    petIndex = get_mob_index(vnum);
    if (petIndex == NULL)
        return FALSE;
    if (paf->bit_type == TO_AFFECTS && IS_AFFECTED (petIndex, paf->bitvector))
        return TRUE;
    return FALSE;
}

/* random room generation procedure */
ROOM_INDEX_DATA *get_random_room (CHAR_DATA * ch) {
    ROOM_INDEX_DATA *room;

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
