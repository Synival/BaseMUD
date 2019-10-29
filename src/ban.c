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
 **************************************************************************/

/***************************************************************************
 *   ROM 2.4 is copyright 1993-1998 Russ Taylor                            *
 *   ROM has been brought to you by the ROM consortium                     *
 *       Russ Taylor (rtaylor@hypercube.org)                               *
 *       Gabrielle Taylor (gtaylor@hypercube.org)                          *
 *       Brian Moore (zump@rom.org)                                        *
 *   By using this code, you have agreed to follow the terms of the        *
 *   ROM license, in the file Rom24/doc/rom.license                        *
 **************************************************************************/

#include <string.h>

#include "recycle.h"
#include "utils.h"
#include "db.h"
#include "save.h"
#include "interp.h"
#include "comm.h"
#include "chars.h"

#include "ban.h"

void save_bans (void) {
    BAN_DATA *pban;
    FILE *fp;
    bool found = FALSE;

    fclose (fpReserve);
    if ((fp = fopen (BAN_FILE, "w")) == NULL)
        perror (BAN_FILE);

    for (pban = ban_first; pban != NULL; pban = pban->next) {
        if (IS_SET (pban->ban_flags, BAN_PERMANENT)) {
            found = TRUE;
            fprintf (fp, "%-20s %-2d %s\n", pban->name, pban->level,
                     print_flags (pban->ban_flags));
        }
    }

    fclose (fp);
    fpReserve = fopen (NULL_FILE, "r");
    if (!found)
        unlink (BAN_FILE);
}

void load_bans (void) {
    FILE *fp;

    if ((fp = fopen (BAN_FILE, "r")) == NULL)
        return;

    while (1) {
        BAN_DATA *pban;
        if (feof (fp)) {
            fclose (fp);
            return;
        }

        pban = ban_new ();
        str_replace_dup (&pban->name, fread_word (fp));
        pban->level = fread_number (fp);
        pban->ban_flags = fread_flag (fp);
        fread_to_eol (fp);

        LISTB_BACK (pban, next, ban_first, ban_last);
    }
}

bool check_ban (char *site, int type) {
    BAN_DATA *pban;
    char host[MAX_STRING_LENGTH];

    strcpy (host, capitalize (site));
    host[0] = LOWER (host[0]);

    for (pban = ban_first; pban != NULL; pban = pban->next) {
        if (!IS_SET (pban->ban_flags, type))
            continue;

        if (IS_SET (pban->ban_flags, BAN_PREFIX)
            && IS_SET (pban->ban_flags, BAN_SUFFIX)
            && strstr (pban->name, host) != NULL)
            return TRUE;

        if (IS_SET (pban->ban_flags, BAN_PREFIX)
            && !str_suffix (pban->name, host))
            return TRUE;

        if (IS_SET (pban->ban_flags, BAN_SUFFIX)
            && !str_prefix (pban->name, host))
            return TRUE;
    }

    return FALSE;
}

void ban_site (CHAR_DATA * ch, char *argument, bool fPerm) {
    char buf[MAX_STRING_LENGTH], buf2[MAX_STRING_LENGTH];
    char arg1[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];
    char *name;
    BUFFER *buffer;
    BAN_DATA *pban, *prev, *pban_next;
    bool prefix = FALSE, suffix = FALSE;
    int type;

    argument = one_argument (argument, arg1);
    argument = one_argument (argument, arg2);

    if (arg1[0] == '\0') {
        BAIL_IF (ban_first == NULL,
            "No sites banned at this time.\n\r", ch);
        buffer = buf_new ();

        add_buf (buffer, "Banned sites  level  type     status\n\r");
        for (pban = ban_first; pban != NULL; pban = pban->next) {
            sprintf (buf2, "%s%s%s",
                     IS_SET (pban->ban_flags, BAN_PREFIX) ? "*" : "",
                     pban->name,
                     IS_SET (pban->ban_flags, BAN_SUFFIX) ? "*" : "");
            sprintf (buf, "%-12s    %-3d  %-7s  %s\n\r",
                     buf2, pban->level,
                     IS_SET (pban->ban_flags, BAN_NEWBIES) ? "newbies" :
                     IS_SET (pban->ban_flags, BAN_PERMIT) ? "permit" :
                     IS_SET (pban->ban_flags, BAN_ALL) ? "all" : "",
                     IS_SET (pban->ban_flags,
                             BAN_PERMANENT) ? "perm" : "temp");
            add_buf (buffer, buf);
        }

        page_to_char (buf_string (buffer), ch);
        buf_free (buffer);
        return;
    }

    /* find out what type of ban */
    if (arg2[0] == '\0' || !str_prefix (arg2, "all"))
        type = BAN_ALL;
    else if (!str_prefix (arg2, "newbies"))
        type = BAN_NEWBIES;
    else if (!str_prefix (arg2, "permit"))
        type = BAN_PERMIT;
    else {
        send_to_char
            ("Acceptable ban types are all, newbies, and permit.\n\r", ch);
        return;
    }

    name = arg1;

    if (name[0] == '*') {
        prefix = TRUE;
        name++;
    }
    if (name[strlen (name) - 1] == '*') {
        suffix = TRUE;
        name[strlen (name) - 1] = '\0';
    }
    BAIL_IF (strlen (name) == 0,
        "You have to ban SOMETHING.\n\r", ch);

    prev = NULL;
    for (pban = ban_first; pban != NULL; prev = pban, pban = pban_next) {
        pban_next = pban->next;
        if (str_cmp (name, pban->name))
            continue;
        BAIL_IF (pban->level > char_get_trust (ch),
            "That ban was set by a higher power.\n\r", ch);

        LISTB_REMOVE_WITH_PREV (pban, prev, next, ban_first, ban_last);
        ban_free (pban);
    }

    pban = ban_new ();
    pban->name = str_dup (name);
    pban->level = char_get_trust (ch);

    /* set ban type */
    pban->ban_flags = type;

    if (prefix)
        SET_BIT (pban->ban_flags, BAN_PREFIX);
    if (suffix)
        SET_BIT (pban->ban_flags, BAN_SUFFIX);
    if (fPerm)
        SET_BIT (pban->ban_flags, BAN_PERMANENT);

    LISTB_FRONT (pban, next, ban_first, ban_last);
    save_bans ();

    printf_to_char (ch, "%s has been banned.\n\r", pban->name);
}
