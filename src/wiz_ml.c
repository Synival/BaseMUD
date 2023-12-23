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

#include "wiz_ml.h"

#include "act_info.h"
#include "boot.h"
#include "chars.h"
#include "comm.h"
#include "db.h"
#include "descs.h"
#include "fight.h"
#include "find.h"
#include "find.h"
#include "globals.h"
#include "interp.h"
#include "json.h"
#include "json_export.h"
#include "lookup.h"
#include "memory.h"
#include "mobiles.h"
#include "objs.h"
#include "players.h"
#include "quickmud.h"
#include "rooms.h"
#include "save.h"
#include "tables.h"
#include "utils.h"

#include <string.h>
#include <stdlib.h>
#include <unistd.h>

DEFINE_DO_FUN (do_advance) {
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    CHAR_T *victim;
    int i, level;

    argument = one_argument (argument, arg1);
    argument = one_argument (argument, arg2);

    BAIL_IF (arg1[0] == '\0' || arg2[0] == '\0' || !is_number (arg2),
        "Syntax: advance <char> <level>.\n\r", ch);
    BAIL_IF ((victim = find_char_world (ch, arg1)) == NULL,
        "That player is not here.\n\r", ch);
    BAIL_IF (IS_NPC (victim),
        "Not on NPC's.\n\r", ch);
    if ((level = atoi (arg2)) < 1 || level > MAX_LEVEL) {
        printf_to_char (ch, "Level must be 1 to %d.\n\r", MAX_LEVEL);
        return;
    }
    BAIL_IF (level > char_get_trust (ch),
        "Limited to your trust level.\n\r", ch);

    /* Lower level:
     *   Reset to level 1.
     *   Then raise again.
     *   Currently, an imp can lower another imp.
     *   -- Swiftest */
    if (level <= victim->level) {
        int temp_prac;
        send_to_char ("Lowering a player's level!\n\r", ch);
        send_to_char ("**** OOOOHHHHHHHHHH  NNNNOOOO ****\n\r", victim);
        temp_prac = victim->practice;
        victim->level = 1;
        victim->exp = player_get_exp_per_level (victim);
        victim->max_hit = 10;
        victim->max_mana = 100;
        victim->max_move = 100;
        victim->practice = 0;
        victim->hit = victim->max_hit;
        victim->mana = victim->max_mana;
        victim->move = victim->max_move;
        player_advance_level (victim, TRUE);
        victim->practice = temp_prac;
    }
    else {
        send_to_char ("Raising a player's level!\n\r", ch);
        send_to_char ("**** OOOOHHHHHHHHHH  YYYYEEEESSS ****\n\r", victim);
    }
    for (i = victim->level; i < level; i++) {
        victim->level += 1;
        player_advance_level (victim, TRUE);
    }
    printf_to_char (victim, "You are now level %d.\n\r", victim->level);
    victim->exp = player_get_exp_per_level (victim) * UMAX (1, victim->level);
    victim->trust = 0;
    save_char_obj (victim);
}

/*  Copyover - Original idea: Fusion of MUD++
 *  Adapted to Diku by Erwin S. Andreasen, <erwin@pip.dknet.dk>
 *  http://pip.dknet.dk/~pip1773
 *  Changed into a ROM patch after seeing the 100th request for it :) */
DEFINE_DO_FUN (do_copyover) {
    FILE *fp;
    DESCRIPTOR_T *d, *d_next;
    char buf[100], buf2[100], buf3[100];

    fp = fopen (COPYOVER_FILE, "w");

    if (!fp) {
        send_to_char ("Copyover file not writeable, aborted.\n\r", ch);
        log_f ("Could not write to copyover file: %s", COPYOVER_FILE);
        perror ("do_copyover:fopen");
        return;
    }

    /* Consider changing all saved areas here, if you use OLC */

    /* do_asave (NULL, ""); - autosave changed areas */
    sprintf (buf, "\n\r *** COPYOVER by %s - please remain seated!\n\r",
        ch->name);

    /* For each playing descriptor, save its state */
    for (d = descriptor_first; d; d = d_next) {
        CHAR_T *och = CH (d);
        d_next = d->global_next; /* We delete from the list , so need to save this */

        /* drop those logging on */
        if (!d->character || d->connected < CON_PLAYING) {
            write_to_descriptor (d->descriptor,
                "\n\rSorry, we are rebooting. Come back in a few minutes.\n\r", 0);
            close_socket (d);    /* throw'em out */
        }
        else {
            fprintf (fp, "%d %s %s\n", d->descriptor, och->name, d->host);
#if 0                            /* This is not necessary for ROM */
            if (och->level == 1) {
                write_to_descriptor (d->descriptor,
                    "Since you are level one, and level one characters "
                    "do not save, you gain a free level!\n\r", 0);
                advance_level (och);
                och->level++;    /* Advance_level doesn't do that */
            }
#endif
            save_char_obj (och);
            write_to_descriptor (d->descriptor, buf, 0);
        }
    }

    fprintf (fp, "-1\n");
    fclose (fp);

    /* Close reserve and other always-open files and release other resources */
    fclose (reserve_file);

#ifdef IMC
    imc_hotboot();
#endif

    /* exec - descriptors are inherited */
    sprintf (buf, "%d", port);
    sprintf (buf2, "%d", control);
#ifdef IMC
    if (his_imcmud)
        snprintf (buf3, sizeof(buf3), "%d", this_imcmud->desc);
    else
        strncpy (buf3, "-1", sizeof(buf3));
#else
    strncpy (buf3, "-1", sizeof(buf3));
#endif
    execl (EXE_FILE, "rom", buf, "copyover", buf2, buf3, (char *) NULL);

    /* Failed - sucessful exec will not return */
    perror ("do_copyover: execl");
    send_to_char ("Copyover FAILED!\n\r", ch);

    /* Here you might want to reopen reserve_file */
    reserve_file = fopen (NULL_FILE, "r");
}

DEFINE_DO_FUN (do_trust) {
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    CHAR_T *victim;
    int level;

    argument = one_argument (argument, arg1);
    argument = one_argument (argument, arg2);

    BAIL_IF (arg1[0] == '\0' || arg2[0] == '\0' || !is_number (arg2),
        "Syntax: trust <char> <level>.\n\r", ch);
    BAIL_IF ((victim = find_char_world (ch, arg1)) == NULL,
        "That player is not here.\n\r", ch);
    if ((level = atoi (arg2)) < 0 || level > MAX_LEVEL) {
        printf_to_char (ch, "Level must be 0 (reset) or 1 to %d.\n\r", MAX_LEVEL);
        return;
    }
    BAIL_IF (level > char_get_trust (ch),
        "Limited to your trust.\n\r", ch);

    victim->trust = level;
}

#define DO_DUMP_SYNTAX \
    "Syntax: dump <stats | world <raw | json>>\n\r"
#define DO_DUMP_WORLD_SYNTAX \
    "Syntax: dump world <raw | json>\n\r"

DEFINE_DO_FUN (do_dump) {
    char arg1[MAX_STRING_LENGTH];
    char arg2[MAX_STRING_LENGTH];

    argument = one_argument (argument, arg1);
    BAIL_IF (arg1[0] == '\0', DO_DUMP_SYNTAX, ch);

    if (strcasecmp (arg1, "stats") == 0) {
        do_dump_stats (ch);
        return;
    }
    else if (strcasecmp (arg1, "world") == 0) {
        argument = one_argument (argument, arg2);
        BAIL_IF (arg2[0] == '\0', DO_DUMP_WORLD_SYNTAX, ch);

        if (strcasecmp (arg2, "raw") == 0) {
            do_dump_world_raw (ch);
            return;
        }
        else if (strcasecmp (arg2, "json") == 0) {
            do_dump_world_json (ch);
            return;
        }

        send_to_char (DO_DUMP_WORLD_SYNTAX, ch);
        return;
    }

    send_to_char (DO_DUMP_SYNTAX, ch);
}

void do_dump_stats (CHAR_T *ch) {
    MOB_INDEX_T *mob_index;
    OBJ_INDEX_T *obj_index;
    FILE *fp;
    int vnum, matches = 0;

    /* lock writing? */
    fclose (reserve_file);

    /* standard memory dump */
    printf_to_char (ch, "Writing '%smemory.dump'...\n\r", DUMP_DIR);
    desc_flush_output (ch->desc);
    fp = fopen (DUMP_DIR "memory.dump", "w");
    fprintf (fp, "%s", mem_dump ("\n"));
    fclose (fp);

    /* start printing out mobile data */
    printf_to_char (ch, "Writing '%smob.dump'...\n\r", DUMP_DIR);
    desc_flush_output (ch->desc);
    fp = fopen (DUMP_DIR "mob.dump", "w");

    fprintf (fp, "Mobile Analysis\n");
    fprintf (fp, "---------------\n");
    matches = 0;
    for (vnum = 0; matches < TOP(RECYCLE_MOB_INDEX_T); vnum++) {
        if ((mob_index = mobile_get_index (vnum)) != NULL) {
            matches++;
            fprintf (fp, "#%-4d %3d active %3d killed     %s\n",
                     mob_index->vnum, mob_index->mob_count,
                     mob_index->killed, mob_index->short_descr);
        }
    }
    fclose (fp);

    /* start printing out object data */
    printf_to_char (ch, "Writing '%sobj.dump'...\n\r", DUMP_DIR);
    desc_flush_output (ch->desc);
    fp = fopen (DUMP_DIR "obj.dump", "w");
    fprintf (fp, "Object Analysis\n");
    fprintf (fp, "---------------\n");
    matches = 0;
    for (vnum = 0; matches < TOP(RECYCLE_OBJ_INDEX_T); vnum++) {
        if ((obj_index = obj_get_index (vnum)) != NULL) {
            matches++;
            fprintf (fp, "#%-4d %3d active %3d reset      %s\n",
                     obj_index->vnum, obj_index->obj_count,
                     obj_index->reset_num, obj_index->short_descr);
        }
    }
    fclose (fp);

    /* unlock writing? */
    reserve_file = fopen (NULL_FILE, "r");
    send_to_char ("Done.\n\r", ch);
}

void do_dump_world_raw (CHAR_T *ch) {
    /* lock writing, dump, unlock writing. */
    fclose (reserve_file);

    printf_to_char (ch, "Writing '%sworld.dump'...\n\r", DUMP_DIR);
    desc_flush_output (ch->desc);
    db_dump_world (DUMP_DIR "world.dump");

    reserve_file = fopen (NULL_FILE, "r");
    send_to_char ("Done.\n\r", ch);
}

void do_dump_world_json (CHAR_T *ch) {
    /* lock writing, dump, unlock writing. */
    fclose (reserve_file);

    printf_to_char (ch, "Writing '%sworld.json'...\n\r", DUMP_DIR);
    desc_flush_output (ch->desc);
    json_export_all (FALSE, DUMP_DIR "world.json");

    reserve_file = fopen (NULL_FILE, "r");
    send_to_char ("Done.\n\r", ch);
}

DEFINE_DO_FUN (do_violate) {
    ROOM_INDEX_T *location;
    CHAR_T *rch;

    BAIL_IF (argument[0] == '\0',
        "Goto where?\n\r", ch);
    BAIL_IF ((location = find_location (ch, argument)) == NULL,
        "No such location.\n\r", ch);
    BAIL_IF (!room_is_private (location),
        "That room isn't private, use goto.\n\r", ch);

    if (ch->fighting != NULL)
        stop_fighting (ch, TRUE);

    for (rch = ch->in_room->people_first; rch; rch = rch->room_next) {
        if (char_get_trust (rch) >= ch->invis_level) {
            if (ch->pcdata != NULL && ch->pcdata->bamfout[0] != '\0')
                act ("$t", ch, ch->pcdata->bamfout, rch, TO_VICT);
            else
                act ("$n leaves in a swirling mist.", ch, NULL, rch, TO_VICT);
        }
    }

    char_to_room (ch, location);
    for (rch = ch->in_room->people_first; rch; rch = rch->room_next) {
        if (char_get_trust (rch) >= ch->invis_level) {
            if (ch->pcdata != NULL && ch->pcdata->bamfin[0] != '\0')
                act ("$t", ch, ch->pcdata->bamfin, rch, TO_VICT);
            else
                act ("$n appears in a swirling mist.", ch, NULL, rch,
                     TO_VICT);
        }
    }
    do_function (ch, &do_look, "auto");
}

/* This _should_ encompass all the QuickMUD config commands */
/* -- JR 11/24/00                                           */
#define DO_QMCONFIG_SYNTAX \
    "Valid qmconfig options are:\n\r" \
    "    show       (shows current status of toggles)\n\r" \
    "    ansiprompt [on|off]\n\r" \
    "    ansicolor  [on|off]\n\r" \
    "    telnetga   [on|off]\n\r" \
    "    read\n\r"

DEFINE_DO_FUN (do_qmconfig) {
    char arg1[MSL];
    char arg2[MSL];

    if (IS_NPC(ch))
        return;

    BAIL_IF (argument[0] == '\0', DO_QMCONFIG_SYNTAX, ch);

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if (!str_prefix(arg1, "read")) {
        qmconfig_read();
        return;
    }

    if (!str_prefix(arg1, "show")) {
        printf_to_char (ch,
            "ANSI prompt: %s\n\r"
            "ANSI color : %s\n\r"
            "IP Address : %s\n\r"
            "Telnet GA  : %s\n\r",
            mud_ansiprompt ? "{GON{x" : "{ROFF{x",
            mud_ansicolor  ? "{GON{x" : "{ROFF{x",
            mud_ipaddress,
            mud_telnetga   ? "{GON{x" : "{ROFF{x"
        );
        return;
    }

    if (!str_prefix(arg1, "ansiprompt")) {
        if (!str_prefix(arg2, "on")) {
            mud_ansiprompt = TRUE;
            printf_to_char(ch, "New logins will now get an ANSI color prompt.\n\r");
            return;
        }
        else if(!str_prefix(arg2, "off")) {
            mud_ansiprompt = FALSE;
            printf_to_char(ch, "New logins will not get an ANSI color prompt.\n\r");
            return;
        }
        printf_to_char(ch, "Valid arguments are \"on\" and \"off\".\n\r");
        return;
    }
    if (!str_prefix(arg1, "ansicolor")) {
        if (!str_prefix(arg2, "on")) {
            mud_ansicolor = TRUE;
            printf_to_char(ch, "New players will have color enabled.\n\r");
            return;
        }
        else if (!str_prefix(arg2, "off")) {
            mud_ansicolor = FALSE;
            printf_to_char(ch, "New players will not have color enabled.\n\r");
            return;
        }
        printf_to_char(ch, "Valid arguments are \"on\" and \"off\".\n\r");
        return;
    }
    if (!str_prefix(arg1, "telnetga")) {
        if (!str_prefix(arg2, "on")) {
            mud_telnetga = TRUE;
            printf_to_char(ch, "Telnet GA will be enabled for new players.\n\r");
            return;
        }
        else if (!str_prefix(arg2, "off")) {
            mud_telnetga = FALSE;
            printf_to_char(ch, "Telnet GA will be disabled for new players.\n\r");
            return;
        }
        printf_to_char(ch, "Valid arguments are \"on\" and \"off\".\n\r");
        return;
    }
    printf_to_char(ch, "I have no clue what you are trying to do...\n\r");
}

#define DO_JSAVE_SYNTAX \
    "Syntax for 'jsave':\n\r" \
    "    jsave all\n\r" \
    "    jsave socials\n\r" \
    "    jsave portals\n\r" \
    "    jsave area (...)\n\r" \
    "    jsave help (...)\n\r" \
    "    jsave table (...)\n\r"

DEFINE_DO_FUN (do_jsave) {
    char arg1[MSL];
    char arg2[MSL];

    if (IS_NPC(ch))
        return;

    argument = one_argument (argument, arg1);
    argument = one_argument (argument, arg2);

    BAIL_IF (arg1[0] == '\0',
        DO_JSAVE_SYNTAX, ch);

    /* Save everything! */
    if (!str_prefix (arg1, "all")) {
        printf_to_char (ch, "Saving '%ssocials.json'.\n\r", JSON_CONFIG_DIR);
        json_export_socials (JSON_EXPORT_MODE_SAVE);

        printf_to_char (ch, "Saving '%sportals.json'.\n\r", JSON_CONFIG_DIR);
        json_export_portals (JSON_EXPORT_MODE_SAVE);

        do_jsave_area (ch, "all");
        do_jsave_help (ch, "all");
        do_jsave_table (ch, "all");
        return;
    }

    /* Save socials. */
    if (!str_prefix (arg1, "socials")) {
        printf_to_char (ch, "Saving '%ssocials.json'.\n\r", JSON_CONFIG_DIR);
        json_export_socials (JSON_EXPORT_MODE_SAVE);
        return;
    }

    /* Save portals. */
    if (!str_prefix (arg1, "portals")) {
        printf_to_char (ch, "Saving '%sportals.json'.\n\r", JSON_CONFIG_DIR);
        json_export_portals (JSON_EXPORT_MODE_SAVE);
        return;
    }

    /* Pass over some other parameters to sub-commands. */
    if (!str_prefix (arg1, "area"))
        { do_jsave_area (ch, arg2); return; }
    if (!str_prefix (arg1, "help"))
        { do_jsave_help (ch, arg2); return; }
    if (!str_prefix (arg1, "table"))
        { do_jsave_table (ch, arg2); return; }

    /* No command found. */
    printf_to_char (ch, "Invalid command '%s'\n\r", arg1);
}

void do_jsave_area (CHAR_T *ch, const char *arg) {
    const AREA_T *area;

    BAIL_IF (arg[0] == '\0',
        "Syntax for 'jsave area':\n\r"
        "    jsave area list\n\r"
        "    jsave area all\n\r"
        "    jsave area <name>\n\r", ch);

    /* Listing possibilities? */
    if (!str_cmp ("list", arg)) {
        int col = 0;
        send_to_char ("Areas that can be saved:\n\r", ch);

        for (area = area_first; area; area = area->global_next) {
            if (col % 7 == 0)
                send_to_char ("  ", ch);
            printf_to_char (ch, "%-10s", area->name);
            if (++col % 7 == 0)
                send_to_char ("\n\r", ch);
        }
        if (col % 7 != 0)
            send_to_char ("\n\r", ch);

        return;
    }

    /* Are we saving all? */
    if (!str_cmp ("all", arg)) {
        send_to_char ("Saving all areas...\n\r", ch);
        desc_flush_output (ch->desc);

        for (area = area_first; area; area = area->global_next) {
            printf_to_char (ch, "   - %s%s/*\n\r", JSON_AREAS_DIR, area->name);
            desc_flush_output (ch->desc);
            json_export_area (area, JSON_EXPORT_MODE_SAVE);
        }

        send_to_char ("Done.\n\r", ch);
        return;
    }

    /* We're saving one - look for it. */
    BAIL_IF ((area = area_get_by_name_exact (arg)) == NULL,
        "Area not found.\n\r", ch);

    printf_to_char (ch, "Saving '%s%s/*'.\n\r", JSON_AREAS_DIR, area->name);
    json_export_area (area, JSON_EXPORT_MODE_SAVE);
}

void do_jsave_help (CHAR_T *ch, const char *arg) {
    const HELP_AREA_T *had;

    BAIL_IF (arg[0] == '\0',
        "Syntax for 'jsave help':\n\r"
        "    jsave help list\n\r"
        "    jsave help all\n\r"
        "    jsave help <name>\n\r", ch);

    /* Listing possibilities? */
    if (!str_cmp ("list", arg)) {
        int col = 0;
        send_to_char ("Helps that can be saved:\n\r", ch);

        for (had = had_first; had; had = had->global_next) {
            if (col % 7 == 0)
                send_to_char ("  ", ch);
            printf_to_char (ch, "%-10s", had->name);
            if (++col % 7 == 0)
                send_to_char ("\n\r", ch);
        }
        if (col % 7 != 0)
            send_to_char ("\n\r", ch);

        return;
    }

    /* Are we saving all? */
    if (!str_cmp ("all", arg)) {
        send_to_char ("Saving all helps...\n\r", ch);
        desc_flush_output (ch->desc);

        for (had = had_first; had; had = had->global_next) {
            printf_to_char (ch, "   - %s%s.json\n\r", JSON_HELP_DIR, had->name);
            desc_flush_output (ch->desc);
            json_export_help_area (had, JSON_EXPORT_MODE_SAVE);
        }

        send_to_char ("Done.\n\r", ch);
        return;
    }

    /* We're saving one - look for it. */
    BAIL_IF ((had = had_get_by_name_exact (arg)) == NULL,
        "Helps not found.\n\r", ch);

    printf_to_char (ch, "Saving '%s%s.json'.\n\r", JSON_HELP_DIR, had->name);
    json_export_help_area (had, JSON_EXPORT_MODE_SAVE);
}

void do_jsave_table (CHAR_T *ch, const char *arg) {
    const TABLE_T *table;
    bool save_all;
    int restrict_type;

    BAIL_IF (arg[0] == '\0',
        "Syntax for 'jsave table':\n\r"
        "    jsave table list\n\r"
        "    jsave table flags\n\r"
        "    jsave table ext_flags\n\r"
        "    jsave table types\n\r"
        "    jsave table unique\n\r"
        "    jsave table all\n\r"
        "    jsave table <name>\n\r", ch);

    /* Listing possibilities? */
    if (!str_cmp ("list", arg)) {
        const char *type_name;
        int col;

        for (restrict_type = 0; restrict_type < TABLE_INTERNAL; restrict_type++) {
            switch (restrict_type) {
                case TABLE_FLAGS:     type_name = "flags";     break;
                case TABLE_TYPES:     type_name = "types";     break;
                case TABLE_EXT_FLAGS: type_name = "ext_flags"; break;
                case TABLE_UNIQUE:    type_name = "unique";    break;
                default:              type_name = "<error>";   break;
            }

            if (restrict_type > 0)
                send_to_char ("\n\r", ch);
            printf_to_char (ch, "Tables of type '%s' can be saved:\n\r",
                type_name);

            col = 0;
            for (table = master_get_first(); table; table = master_get_next (table)) {
                if (table->type != restrict_type)
                    continue;
                if (!json_can_export_table (table))
                    continue;
                if (col % 4 == 0)
                    send_to_char ("  ", ch);
                printf_to_char (ch, "%-19s", table->name);
                if (++col % 4 == 0)
                    send_to_char ("\n\r", ch);
            }
            if (col % 4 != 0)
                send_to_char ("\n\r", ch);
        }

        return;
    }

    /* Are we saving all? */
    if (!str_cmp ("flags", arg)) {
        send_to_char ("Saving all tables of type 'flags'...\n\r", ch);
        save_all = TRUE;
        restrict_type = TABLE_FLAGS;
    }
    else if (!str_cmp ("ext_flags", arg)) {
        send_to_char ("Saving all tables of type 'ext_flags'...\n\r", ch);
        save_all = TRUE;
        restrict_type = TABLE_EXT_FLAGS;
    }
    else if (!str_cmp ("types", arg)) {
        send_to_char ("Saving all tables of type 'types'...\n\r", ch);
        save_all = TRUE;
        restrict_type = TABLE_TYPES;
    }
    else if (!str_cmp ("unique", arg)) {
        send_to_char ("Saving all tables of type 'unique'...\n\r", ch);
        save_all = TRUE;
        restrict_type = TABLE_UNIQUE;
    }
    else if (!str_cmp ("all", arg)) {
        send_to_char ("Saving all tables...\n\r", ch);
        save_all = TRUE;
        restrict_type = -1;
    }
    else {
        save_all = FALSE;
        restrict_type = -1;
    }

    if (save_all) {
        for (table = master_get_first(); table; table = master_get_next (table)) {
            if (restrict_type >= 0 && table->type != restrict_type)
                continue;
            if (!json_can_export_table (table))
                continue;

            printf_to_char (ch, "   - %s%s/%s.json\n\r", JSON_DIR,
                table->json_path, table->name);
            desc_flush_output (ch->desc);
            json_export_table (table, JSON_EXPORT_MODE_SAVE);
        }

        send_to_char ("Done.\n\r", ch);
        return;
    }

    /* We're saving one - look for it. */
    table = master_get_by_name_exact (arg);
    BAIL_IF (table == NULL || !json_can_export_table (table),
        "Table not found.\n\r", ch);

    printf_to_char (ch, "Saving '%s%s/%s.json'.\n\r", JSON_DIR,
        table->json_path, table->name);
    json_export_table (table, JSON_EXPORT_MODE_SAVE);
}
