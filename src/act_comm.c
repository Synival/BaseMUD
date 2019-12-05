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
#include <stdlib.h>
#include <ctype.h> /* for isalpha() and isspace() -- JR */

#include "interp.h"
#include "recycle.h"
#include "lookup.h"
#include "comm.h"
#include "utils.h"
#include "mob_prog.h"
#include "db.h"
#include "do_sub.h"
#include "chars.h"
#include "find.h"
#include "chars.h"
#include "globals.h"

#include "act_comm.h"

bool do_comm_filter_quiet (CHAR_T *ch) {
    FILTER_ACT (IS_SET (ch->comm, COMM_QUIET),
        "You must turn off quiet mode first.", ch, NULL, NULL);
    return FALSE;
}

bool do_comm_filter_nochannels (CHAR_T *ch) {
    FILTER_ACT (IS_SET (ch->comm, COMM_NOCHANNELS),
        "The gods have revoked your channel priviliges.", ch, NULL, NULL);
    return FALSE;
}

bool do_comm_filter_quiet_nochannels (CHAR_T *ch) {
    return do_comm_filter_quiet (ch) || do_comm_filter_nochannels (ch);
}

bool do_comm_filter_emote (CHAR_T *ch, char *argument) {
    FILTER (!IS_NPC (ch) && IS_SET (ch->comm, COMM_NOEMOTE),
        "You can't show your emotions.\n\r", ch);
    FILTER (argument[0] == '\0',
        "Emote what?\n\r", ch);

    /* little hack to fix the ',{' bug posted to rom list
     * around 4/16/01 -- JR */
    FILTER (!(isalpha(argument[0])) || (isspace(argument[0])),
        "Moron!\n\r", ch);
    return FALSE;
}

void do_comm_channel_global (CHAR_T *ch, char *argument, flag_t channel,
    char *message_on, char *message_off, char *act_self, char *act_world,
    int max_pos)
{
    DESCRIPTOR_T *d;

    if (do_comm_toggle_channel_if_blank (ch, argument, channel,
            message_on, message_off))
        return;
    if (do_comm_filter_quiet_nochannels (ch))
        return;
    REMOVE_BIT (ch->comm, channel);

    printf_to_char (ch, act_self, argument);
    for (d = descriptor_list; d != NULL; d = d->next) {
        CHAR_T *victim = CH(d);
        if (victim == ch || d->connected != CON_PLAYING)
            continue;
        if (IS_SET (victim->comm, channel) || IS_SET (victim->comm, COMM_QUIET))
            continue;
        act_new (act_world, ch, argument, victim, TO_VICT, max_pos);
    }
}

bool do_comm_filter_can_tell_or_reply (CHAR_T *ch) {
    FILTER_ACT (IS_SET (ch->comm, COMM_NOTELL),
        "Your message didn't get through.", ch, NULL, NULL);
    FILTER_ACT (IS_SET (ch->comm, COMM_DEAF),
        "You must turn off deaf mode first.", ch, NULL, NULL);
    if (do_comm_filter_quiet (ch))
        return TRUE;
    return FALSE;
}

void do_comm_tell_to_buffer (CHAR_T *ch, CHAR_T *victim, char *msg) {
    char buf[MAX_STRING_LENGTH];
    sprintf (buf, "{k%s tells you '{K%s{k'{x\n\r", PERS_AW (ch, victim), msg);
    buf[2] = UPPER (buf[2]);
    add_buf (victim->pcdata->buffer, buf);
}

bool do_comm_filter_tell_target_can_receive_tells(CHAR_T *ch,
    CHAR_T *victim)
{
    FILTER_ACT (ch == victim,
        "Yeah, keep telling yourself that.", ch, NULL, NULL);

    /* Immortals can always send messages to other players. */
    if (IS_IMMORTAL (ch))
        return FALSE;

    /* We can't talk when we're awake, unless it's to immortals.
     * They can talk to us when we're sleeping, so it seems only fair! */
    FILTER_ACT (!IS_AWAKE (ch) && !IS_IMMORTAL (victim),
        "In your dreams, or what?", ch, NULL, NULL);
    FILTER_ACT (!IS_AWAKE (victim),
        "$E can't hear you.", ch, NULL, victim);
    FILTER_ACT (IS_SET (victim->comm, COMM_QUIET) ||
                IS_SET (victim->comm, COMM_DEAF),
        "$E is not receiving tells.", ch, NULL, victim);

    /* Filters passed. */
    return FALSE;
}

bool do_comm_filter_tell_not_now (CHAR_T *ch, CHAR_T *victim, char *msg) {
    if (!IS_NPC (victim) && !victim->desc) {
        act_new ("$N seems to have misplaced $S link, but your tell will go "
                 "through when $E returns.", ch, NULL, victim, TO_CHAR, POS_DEAD);
        do_comm_tell_to_buffer (ch, victim, msg);
        return TRUE;
    }
    else if (IS_SET (victim->comm, COMM_AFK)) {
        FILTER_ACT (IS_NPC (victim),
            "$E is AFK, and not receiving tells.", ch, NULL, victim);

        act_new ("$E is AFK, but your tell will go through when $E "
                 "returns.", ch, NULL, victim, TO_CHAR, POS_DEAD);
        do_comm_tell_to_buffer (ch, victim, msg);
        return TRUE;
    }
    else if (victim->desc != NULL &&
             victim->desc->connected >= CON_NOTE_TO &&
             victim->desc->connected <= CON_NOTE_FINISH)
    {
        act_new ("$E is writing a note, but your tell will go through when "
                 "$E returns.", ch, NULL, victim, TO_CHAR, POS_DEAD);
        do_comm_tell_to_buffer (ch, victim, msg);
        return TRUE;
    }
    return FALSE;
}

void do_comm_tell_send_message (CHAR_T *ch, CHAR_T *victim, char *msg) {
    act_new ("{kYou tell $N '{K$t{k'{x",  ch, msg, victim, TO_CHAR, POS_DEAD);
    act_new ("{k$n tells you '{K$t{k'{x", ch, msg, victim, TO_VICT, POS_DEAD);
    victim->reply = ch;

    if (!IS_NPC (ch) && IS_NPC (victim) && HAS_TRIGGER (victim, TRIG_SPEECH))
        mp_act_trigger (msg, victim, ch, NULL, NULL, TRIG_SPEECH);
}

void do_comm_try_tell (CHAR_T *ch, CHAR_T *victim, char *msg) {
    BAIL_IF_ACT (msg[0] == '\0',
        "Tell $M what?", ch, NULL, victim);
    if (do_comm_filter_tell_target_can_receive_tells (ch, victim))
        return;
    if (do_comm_filter_tell_not_now (ch, victim, msg))
        return;
    do_comm_tell_send_message (ch, victim, msg);
}

/* RT does socials */
DEFINE_DO_FUN (do_socials) {
    SOCIAL_T *soc;
    int col;

    col = 0;
    for (soc = social_get_first(); soc != NULL; soc = social_get_next(soc)) {
        printf_to_char (ch, "%-12s", soc->name);
        if (++col % 6 == 0)
            send_to_char ("\n\r", ch);
    }
    if (col % 6 != 0)
        send_to_char ("\n\r", ch);
}

/* RT code to display channel status */
DEFINE_DO_FUN (do_channels) {
    /* lists all channels and their status */
    send_to_char ("   Channel        Status\n\r", ch);
    send_to_char ("---------------------------\n\r", ch);

    do_autolist_flag ("{dgossip{x",     ch, ~ch->comm, COMM_NOGOSSIP);
    do_autolist_flag ("{aauction{x",    ch, ~ch->comm, COMM_NOAUCTION);
    do_autolist_flag ("{emusic{x",      ch, ~ch->comm, COMM_NOMUSIC);
    do_autolist_flag ("{qQ{x/{fA{x",    ch, ~ch->comm, COMM_NOQUESTION);
    do_autolist_flag ("{hquote{x",      ch, ~ch->comm, COMM_NOQUOTE);
    do_autolist_flag ("{tgrats{x",      ch, ~ch->comm, COMM_NOGRATS);
    do_autolist_flag ("{tshouts{x",     ch, ~ch->comm, COMM_SHOUTSOFF);
    do_autolist_flag ("{ktells{x",      ch, ~ch->comm, COMM_DEAF);
    if (IS_IMMORTAL (ch))
        do_autolist_flag ("{igod channel{x", ch, ~ch->comm, COMM_NOWIZ);

    send_to_char ("---------------------------\n\r", ch);
    do_autolist_flag ("{tquiet mode{x", ch, ch->comm, COMM_QUIET);
    do_autolist_flag ("AFK", ch, ch->comm, COMM_AFK);

    send_to_char ("---------------------------\n\r", ch);
    if (ch->lines != PAGELEN) {
        if (ch->lines) {
            printf_to_char (ch, "You display %d lines of scroll.\n\r",
                ch->lines + 2);
        }
        else
            send_to_char ("Scroll buffering is off.\n\r", ch);
    }

    if (ch->prompt != NULL)
        printf_to_char (ch, "Your current prompt is: %s\n\r", ch->prompt);

    if (IS_SET (ch->comm, COMM_NOSHOUT))
        send_to_char ("You cannot shout.\n\r", ch);
    if (IS_SET (ch->comm, COMM_NOTELL))
        send_to_char ("You cannot use tell.\n\r", ch);
    if (IS_SET (ch->comm, COMM_NOCHANNELS))
        send_to_char ("You cannot use channels.\n\r", ch);
    if (IS_SET (ch->comm, COMM_NOEMOTE))
        send_to_char ("You cannot show emotions.\n\r", ch);
}

/* RT deaf blocks out all shouts */
DEFINE_DO_FUN (do_deaf) {
    do_flag_toggle (ch, FALSE, &(ch->comm), COMM_DEAF,
        "{kYou can now hear tells again.{x\n\r",
        "{kFrom now on, you won't hear tells.{x\n\r");
}

/* RT quiet blocks out all communication */
DEFINE_DO_FUN (do_quiet) {
    do_flag_toggle (ch, FALSE, &(ch->comm), COMM_QUIET,
        "{tQuiet mode removed.{x\n\r",
        "{tFrom now on, you will only hear says and emotes.{x\n\r");
}

/* afk command */
DEFINE_DO_FUN (do_afk) {
    do_flag_toggle (ch, FALSE, &(ch->comm), COMM_AFK,
        "AFK mode removed. Type 'replay' to see tells.\n\r",
        "You are now in AFK mode.\n\r");
}

DEFINE_DO_FUN (do_replay) {
    BAIL_IF (IS_NPC (ch),
        "You can't replay.\n\r", ch);
    BAIL_IF (buf_string (ch->pcdata->buffer)[0] == '\0',
        "You have no tells to replay.\n\r", ch);
    page_to_char (buf_string (ch->pcdata->buffer), ch);
    clear_buf (ch->pcdata->buffer);
}

DEFINE_DO_FUN (do_auction) {
    do_comm_channel_global (ch, argument, COMM_NOAUCTION,
        "{aAuction channel is now ON.{x\n\r",
        "{aAuction channel is now OFF.{x\n\r",
        "{aYou auction '{A%s{a'{x\n\r",
        "{a$n auctions '{A$t{a'{x",
        POS_DEAD);
}

DEFINE_DO_FUN (do_gossip) {
    do_comm_channel_global (ch, argument, COMM_NOGOSSIP,
        "{dGossip channel is now ON.{x\n\r",
        "{dGossip channel is now OFF.{x\n\r",
        "{dYou gossip '{9%s{d'{x\n\r",
        "{d$n gossips '{9$t{d'{x", POS_SLEEPING);
}

DEFINE_DO_FUN (do_grats) {
    do_comm_channel_global (ch, argument, COMM_NOGRATS,
        "{tGrats channel is now ON.{x\n\r",
        "{tGrats channel is now OFF.{x\n\r",
        "{tYou grats '%s'{x\n\r",
        "{t$n grats '$t'{x", POS_SLEEPING);
}

DEFINE_DO_FUN (do_quote) {
    do_comm_channel_global (ch, argument, COMM_NOQUOTE,
        "{hQuote channel is now ON.{x\n\r",
        "{hQuote channel is now OFF.{x\n\r",
        "{hYou quote '{H%s{h'{x\n\r",
        "{h$n quotes '{H$t{h'{x", POS_SLEEPING);
}

/* RT question channel */
DEFINE_DO_FUN (do_question) {
    do_comm_channel_global (ch, argument, COMM_NOQUESTION,
        "{qQ{x/{fA{x channel is now ON.\n\r",
        "{qQ{x/{fA{x channel is now OFF.\n\r",
        "{qYou question '{Q%s{q'{x\n\r",
        "{q$n questions '{Q$t{q'{x", POS_SLEEPING);
}

/* RT answer channel - uses same line as questions */
DEFINE_DO_FUN (do_answer) {
    do_comm_channel_global (ch, argument, COMM_NOQUESTION,
        "{qQ{x/{fA{x channel is now ON.\n\r",
        "{qQ{x/{fA{x channel is now OFF.\n\r",
        "{fYou answer '{F%s{f'{x\n\r",
        "{f$n answers '{F$t{f'{x", POS_SLEEPING);
}

/* RT music channel */
DEFINE_DO_FUN (do_music) {
    do_comm_channel_global (ch, argument, COMM_NOMUSIC,
        "Music channel is now ON.\n\r",
        "Music channel is now OFF.\n\r",
        "{eYou MUSIC: '{E%s{e'{x\n\r",
        "{e$n MUSIC: '{E$t{e'{x", POS_SLEEPING);
}

/* clan channels */
DEFINE_DO_FUN (do_clantalk) {
    DESCRIPTOR_T *d;

    BAIL_IF (!char_has_clan (ch) || clan_table[ch->clan].independent,
        "You aren't in a clan.\n\r", ch);
    if (do_comm_toggle_channel_if_blank (ch, argument, COMM_NOCLAN,
            "Clan channel is now ON\n\r",
            "Clan channel is now OFF\n\r"))
        return;
    if (do_comm_filter_quiet_nochannels (ch))
        return;
    REMOVE_BIT (ch->comm, COMM_NOCLAN);

    printf_to_char (ch, "You clan '%s'{x\n\r", argument);
    for (d = descriptor_list; d != NULL; d = d->next) {
        CHAR_T *victim = CH(d);
        if (victim == ch || d->connected != CON_PLAYING)
            continue;
        if (IS_SET (victim->comm, COMM_NOCLAN) || IS_SET (victim->comm, COMM_QUIET))
            continue;
        if (!char_in_same_clan (ch, d->character))
            continue;
        act_new ("$n clans '$t'{x", ch, argument, victim, TO_VICT, POS_DEAD);
    }
}

DEFINE_DO_FUN (do_say) {
    BAIL_IF (argument[0] == '\0',
        "Say what?\n\r", ch);

    act2 ("{6You say '{7$T{6'{x",
          "{6$n says '{7$T{6'{x",
        ch, NULL, argument, 0, POS_RESTING);

    if (!IS_NPC (ch)) {
        CHAR_T *mob, *mob_next;
        for (mob = ch->in_room->people; mob != NULL; mob = mob_next) {
            mob_next = mob->next_in_room;
            if (IS_NPC (mob) && HAS_TRIGGER (mob, TRIG_SPEECH)
                && mob->position == mob->pIndexData->default_pos)
                mp_act_trigger (argument, mob, ch, NULL, NULL, TRIG_SPEECH);
        }
    }
}

DEFINE_DO_FUN (do_shout) {
    DESCRIPTOR_T *d;

    if (do_comm_toggle_channel_if_blank (ch, argument, COMM_SHOUTSOFF,
            "{tYou can hear shouts again.{x\n\r",
            "{tYou will no longer hear shouts.{x\n\r"))
        return;
    BAIL_IF (IS_SET (ch->comm, COMM_NOSHOUT),
        "You can't shout.\n\r", ch);
    REMOVE_BIT (ch->comm, COMM_SHOUTSOFF);
    WAIT_STATE (ch, 12);

    act ("You shout '$T'", ch, NULL, argument, TO_CHAR);
    for (d = descriptor_list; d != NULL; d = d->next) {
        CHAR_T *victim = CH(d);
        if (victim == ch || d->connected != CON_PLAYING)
            continue;
        if (IS_SET (victim->comm, COMM_SHOUTSOFF) || IS_SET (victim->comm, COMM_QUIET))
            continue;
        act ("$n shouts '$t'", ch, argument, victim, TO_VICT);
    }
}

DEFINE_DO_FUN (do_tell) {
    char arg[MAX_INPUT_LENGTH];
    CHAR_T *victim;

    if (do_comm_filter_can_tell_or_reply (ch))
        return;

    DO_REQUIRE_ARG (arg, "Tell whom what?\n\r");

    /* Can tell to PC's anywhere, but NPC's only in same room.
     * -- Furey */
    BAIL_IF ((victim = find_char_world (ch, arg)) == NULL ||
             (IS_NPC (victim) && victim->in_room != ch->in_room),
        "They aren't here.\n\r", ch);
    do_comm_try_tell (ch, victim, argument);
}

DEFINE_DO_FUN (do_reply) {
    CHAR_T *victim;
    if (do_comm_filter_can_tell_or_reply (ch))
        return;
    BAIL_IF ((victim = ch->reply) == NULL,
        "They aren't here.\n\r", ch);
    do_comm_try_tell (ch, victim, argument);
}

DEFINE_DO_FUN (do_yell) {
    DESCRIPTOR_T *d;

    BAIL_IF (IS_SET (ch->comm, COMM_NOSHOUT),
        "You can't yell.\n\r", ch);
    BAIL_IF (argument[0] == '\0',
        "Yell what?\n\r", ch);

    act ("You yell '$t'", ch, argument, NULL, TO_CHAR);
    WAIT_STATE (ch, 12);

    for (d = descriptor_list; d != NULL; d = d->next) {
        CHAR_T *victim = CH(d);
        if (victim == ch || d->connected != CON_PLAYING)
            continue;
        if (IS_SET (victim->comm, COMM_SHOUTSOFF) || IS_SET (victim->comm, COMM_QUIET))
            continue;
        if (victim->in_room == NULL || ch->in_room == NULL)
            continue;
        if (victim->in_room->area != ch->in_room->area)
            continue;
        act ("$n yells '$t'", ch, argument, victim, TO_VICT);
    }
}

DEFINE_DO_FUN (do_emote) {
    if (do_comm_filter_emote (ch, argument))
        return;

    MOBtrigger = FALSE;
    act ("$n $T", ch, NULL, argument, TO_CHAR);
    act ("$n $T", ch, NULL, argument, TO_NOTCHAR);
    MOBtrigger = TRUE;
}

DEFINE_DO_FUN (do_pmote) {
    CHAR_T *vch;
    char *letter, *name;
    char last[MAX_INPUT_LENGTH], temp[MAX_STRING_LENGTH];
    int matches = 0;

    if (do_comm_filter_emote (ch, argument))
        return;

    act ("$n $t", ch, argument, NULL, TO_CHAR);
    for (vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room) {
        if (vch->desc == NULL || vch == ch)
            continue;

        if ((letter = strstr (argument, vch->name)) == NULL) {
            MOBtrigger = FALSE;
            act ("$N $t", vch, argument, ch, TO_CHAR);
            MOBtrigger = TRUE;
            continue;
        }

        strcpy (temp, argument);
        temp[strlen (argument) - strlen (letter)] = '\0';
        last[0] = '\0';
        name = vch->name;

        for (; *letter != '\0'; letter++) {
            if (*letter == '\'' && matches == strlen (vch->name)) {
                strcat (temp, "r");
                continue;
            }
            if (*letter == 's' && matches == strlen (vch->name)) {
                matches = 0;
                continue;
            }
            if (matches == strlen (vch->name))
                matches = 0;

            if (*letter == *name) {
                matches++;
                name++;
                if (matches == strlen (vch->name)) {
                    strcat (temp, "you");
                    last[0] = '\0';
                    name = vch->name;
                    continue;
                }
                strncat (last, letter, 1);
                continue;
            }

            matches = 0;
            strcat (temp, last);
            strncat (temp, letter, 1);
            last[0] = '\0';
            name = vch->name;
        }

        MOBtrigger = FALSE;
        act ("$N $t", vch, temp, ch, TO_CHAR);
        MOBtrigger = TRUE;
    }
}

DEFINE_DO_FUN (do_pose) {
    int level, pose, message;

    BAIL_IF (IS_NPC (ch) || ch->class < 0 || ch->class >= CLASS_MAX,
        "You have no profession to demonstrate. Sorry!\n\r", ch);

    level = UMIN (ch->level, POS_MAX - 1);
    pose  = number_range (0, level);
    message = 2 * ch->class;

    act2 (pose_table[pose].message[message + 0],
          pose_table[pose].message[message + 1],
          ch, NULL, NULL, 0, POS_RESTING);
}

DEFINE_DO_FUN (do_bug) {
    append_file (ch, BUG_FILE, argument);
    send_to_char ("Bug logged.\n\r", ch);
}

DEFINE_DO_FUN (do_typo) {
    append_file (ch, TYPO_FILE, argument);
    send_to_char ("Typo logged.\n\r", ch);
}
