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

/*   QuickMUD - The Lazy Man's ROM - $Id: act_comm.c,v 1.2 2000/12/01 10:48:33 ring0 Exp $ */

#include <string.h>
#include <ctype.h>

#include "db.h"
#include "comm.h"
#include "save.h"
#include "utils.h"
#include "fight.h"
#include "interp.h"
#include "recycle.h"
#include "chars.h"
#include "descs.h"

#include "act_player.h"

/* RT code to delete yourself */
DEFINE_DO_FUN (do_delet) {
    send_to_char ("You must type the full command to delete yourself.\n\r", ch);
}

DEFINE_DO_FUN (do_delete) {
    char strsave[MAX_INPUT_LENGTH];

    if (IS_NPC (ch))
        return;

    if (ch->pcdata->confirm_delete) {
        if (argument[0] != '\0') {
            send_to_char ("Delete status removed.\n\r", ch);
            ch->pcdata->confirm_delete = FALSE;
            return;
        }
        else {
            wiznet ("$N turns $Mself into line noise.", ch, NULL, 0, 0, 0);
            sprintf (strsave, "%s%s", PLAYER_DIR, capitalize (ch->name));
            stop_fighting (ch, TRUE);
            do_function (ch, &do_quit, "");
            unlink (strsave);
            return;
        }
    }

    BAIL_IF (argument[0] != '\0',
        "Just type delete. No argument.\n\r", ch);

    send_to_char (
        "Type delete again to confirm this command.\n\r"
        "WARNING: this command is irreversible.\n\r"
        "Typing delete with an argument will undo delete status.\n\r", ch);

    ch->pcdata->confirm_delete = TRUE;
    wiznet ("$N is contemplating deletion.", ch, NULL, 0, 0,
        char_get_trust (ch));
}

DEFINE_DO_FUN (do_rent) {
    send_to_char ("There is no rent here.  Just save and quit.\n\r", ch);
}

DEFINE_DO_FUN (do_qui) {
    send_to_char ("If you want to QUIT, you have to spell it out.\n\r", ch);
}

DEFINE_DO_FUN (do_quit) {
    DESCRIPTOR_DATA *d, *d_next;
    int id;

    if (IS_NPC (ch))
        return;

    BAIL_IF (ch->position == POS_FIGHTING,
        "No way! You are fighting.\n\r", ch);
    BAIL_IF (ch->position <= POS_STUNNED,
        "You're not DEAD yet.\n\r", ch);

    send_to_char ("Alas, all good things must come to an end.\n\r", ch);
    act ("$n has left the game.", ch, NULL, NULL, TO_NOTCHAR);
    log_f ("%s has quit.", ch->name);
    wiznet ("$N rejoins the real world.", ch, NULL, WIZ_LOGINS, 0,
        char_get_trust (ch));

    /* After char_extract the ch is no longer valid! */
    save_char_obj (ch);

    /* Free note that might be there somehow */
    if (ch->pcdata->in_progress)
        note_free (ch->pcdata->in_progress);

    id = ch->id;
    d = ch->desc;
    char_extract (ch, TRUE);
    if (d != NULL)
        close_socket (d);

    /* toast evil cheating bastards */
    for (d = descriptor_list; d != NULL; d = d_next) {
        CHAR_DATA *tch;
        d_next = d->next;

        tch = d->original ? d->original : d->character;
        if (tch && tch->id == id) {
            char_extract (tch, TRUE);
            close_socket (d);
        }
    }
}

DEFINE_DO_FUN (do_save) {
    if (IS_NPC (ch))
        return;

    save_char_obj (ch);
    send_to_char ("Saving. Remember that ROM has automatic saving now.\n\r", ch);
    WAIT_STATE (ch, PULSE_VIOLENCE);
}

DEFINE_DO_FUN (do_password) {
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char *pArg;
    char *pwdnew;
    char *p;
    char cEnd;

    if (IS_NPC (ch))
        return;

    /* Can't use one_argument here because it smashes case.
     * So we just steal all its code. Bleagh. */
    pArg = arg1;
    while (isspace (*argument))
        argument++;

    cEnd = ' ';
    if (*argument == '\'' || *argument == '"')
        cEnd = *argument++;

    while (*argument != '\0') {
        if (*argument == cEnd) {
            argument++;
            break;
        }
        *pArg++ = *argument++;
    }
    *pArg = '\0';

    pArg = arg2;
    while (isspace (*argument))
        argument++;

    cEnd = ' ';
    if (*argument == '\'' || *argument == '"')
        cEnd = *argument++;

    while (*argument != '\0') {
        if (*argument == cEnd) {
            argument++;
            break;
        }
        *pArg++ = *argument++;
    }
    *pArg = '\0';

    BAIL_IF (arg1[0] == '\0' || arg2[0] == '\0',
        "Syntax: password <old> <new>.\n\r", ch);

    if (strcmp (crypt (arg1, ch->pcdata->pwd), ch->pcdata->pwd)) {
        WAIT_STATE (ch, 40);
        send_to_char ("Wrong password.  Wait 10 seconds.\n\r", ch);
        return;
    }
    BAIL_IF (strlen (arg2) < 5,
        "New password must be at least five characters long.\n\r", ch);

    /* No tilde allowed because of player file format. */
    pwdnew = crypt (arg2, ch->name);
    for (p = pwdnew; *p != '\0'; p++)
        BAIL_IF (*p == '~',
            "New password not acceptable, try again.\n\r", ch);

    str_free (ch->pcdata->pwd);
    ch->pcdata->pwd = str_dup (pwdnew);
    save_char_obj (ch);
    send_to_char ("Ok.\n\r", ch);
}
