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

#include <stdlib.h>

#include "interp.h"
#include "groups.h"
#include "utils.h"
#include "comm.h"
#include "db.h"
#include "chars.h"
#include "find.h"

#include "act_group.h"

void do_group_show (CHAR_DATA *ch) {
    CHAR_DATA *gch;
    CHAR_DATA *leader;

    leader = (ch->leader != NULL) ? ch->leader : ch;
    printf_to_char (ch, "%s's group:\n\r", PERS_AW (leader, ch));

    for (gch = char_list; gch != NULL; gch = gch->next) {
        if (!is_same_group (gch, ch))
            continue;
        printf_to_char (ch,
            "[%2d %s] %-16s %4d/%4d hp %4d/%4d mana %4d/%4d mv %5d xp\n\r",
            gch->level, IS_NPC (gch) ? "Mob" : class_table[gch->class].who_name,
            capitalize (PERS_AW (gch, ch)), gch->hit, gch->max_hit,
            gch->mana, gch->max_mana, gch->move, gch->max_move, gch->exp);
    }
}

void do_order_all (CHAR_DATA * ch, char *argument) {
    char buf[MAX_STRING_LENGTH];
    snprintf(buf, sizeof(buf), "all %s", argument);
    do_order(ch, buf);
}

void do_order (CHAR_DATA * ch, char *argument) {
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    CHAR_DATA *och;
    CHAR_DATA *och_next;
    bool found;
    bool fAll;

    argument = one_argument (argument, arg);
    one_argument (argument, arg2);

    BAIL_IF (!str_cmp (arg2, "delete") || !str_cmp (arg2, "mob"),
        "That will NOT be done.\n\r", ch);
    BAIL_IF (arg[0] == '\0' || argument[0] == '\0',
        "Order whom to do what?\n\r", ch);
    BAIL_IF (IS_AFFECTED (ch, AFF_CHARM),
        "You feel like taking, not giving, orders.\n\r", ch);

    if (!str_cmp (arg, "all")) {
        fAll = TRUE;
        victim = NULL;
    }
    else {
        fAll = FALSE;
        BAIL_IF ((victim = find_char_same_room (ch, arg)) == NULL,
            "They aren't here.\n\r", ch);
        BAIL_IF (victim == ch,
            "Aye aye, right away!\n\r", ch);
        BAIL_IF (!IS_AFFECTED (victim, AFF_CHARM) || victim->master != ch ||
                 (IS_IMMORTAL (victim) && victim->trust >= ch->trust),
            "Do it yourself!\n\r", ch);
    }

    found = FALSE;
    for (och = ch->in_room->people; och != NULL; och = och_next) {
        och_next = och->next_in_room;
        if (!(IS_AFFECTED (och, AFF_CHARM) &&
              och->master == ch && (fAll || och == victim)))
            continue;

        found = TRUE;
        sprintf (buf, "$n orders you to '%s'.", argument);
        act (buf, ch, NULL, och, TO_VICT);
        interpret (och, argument);
    }

    if (found) {
        WAIT_STATE (ch, PULSE_VIOLENCE);
        send_to_char ("Ok.\n\r", ch);
    }
    else
        send_to_char ("You have no followers here.\n\r", ch);
}

void do_group (CHAR_DATA * ch, char *argument) {
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument (argument, arg);
    if (arg[0] == '\0') {
        do_group_show (ch);
        return;
    }

    BAIL_IF ((victim = find_char_same_room (ch, arg)) == NULL,
        "They aren't here.\n\r", ch);
    BAIL_IF (ch->master != NULL || (ch->leader != NULL && ch->leader != ch),
        "But you are following someone else!\n\r", ch);
    BAIL_IF_ACT (victim->master != ch && ch != victim,
        "$N isn't following you.", ch, NULL, victim);
    BAIL_IF (IS_AFFECTED (victim, AFF_CHARM),
        "You can't remove charmed mobs from your group.\n\r", ch);
    BAIL_IF_ACT (IS_AFFECTED (ch, AFF_CHARM),
        "You like your master too much to leave $m!", ch, NULL, victim);

    if (is_same_group (victim, ch) && ch != victim) {
        victim->leader = NULL;
        act3 ("You remove $N from your group.",
              "$n removes you from $s group.",
              "$n removes $N from $s group.",
            ch, NULL, victim, 0, POS_SLEEPING);
        return;
    }

    victim->leader = ch;
    act3 ("$N joins your group.",
          "You join $n's group.",
          "$N joins $n's group.",
          ch, NULL, victim, TO_CHAR, POS_SLEEPING);
}

/* 'Split' originally by Gnort, God of Chaos. */
void do_split (CHAR_DATA * ch, char *argument) {
    char buf[MAX_STRING_LENGTH];
    char arg1[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];
    CHAR_DATA *gch;
    int members;
    int amount_gold = 0, amount_silver = 0;
    int share_gold, share_silver;
    int extra_gold, extra_silver;

    DO_REQUIRE_ARG (arg1, "Split how much?\n\r");
    one_argument (argument, arg2);

    amount_silver = atoi (arg1);
    if (arg2[0] != '\0')
        amount_gold = atoi (arg2);

    BAIL_IF (amount_gold < 0 || amount_silver < 0,
        "Your group wouldn't like that.\n\r", ch);
    BAIL_IF (amount_gold == 0 && amount_silver == 0,
        "You hand out zero coins, but no one notices.\n\r", ch);
    BAIL_IF (ch->gold < amount_gold || ch->silver < amount_silver,
        "You don't have that much to split.\n\r", ch);

    members = 0;
    for (gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room)
        if (is_same_group (gch, ch) && !IS_AFFECTED (gch, AFF_CHARM))
            members++;

    BAIL_IF (members < 2,
        "Just keep it all.\n\r", ch);

    share_silver = amount_silver / members;
    extra_silver = amount_silver % members;

    share_gold = amount_gold / members;
    extra_gold = amount_gold % members;

    BAIL_IF (share_gold == 0 && share_silver == 0,
        "Don't even bother, cheapskate.\n\r", ch);

    ch->silver -= amount_silver;
    ch->silver += share_silver + extra_silver;
    ch->gold -= amount_gold;
    ch->gold += share_gold + extra_gold;

    if (share_gold == 0) {
        printf_to_char (ch, "You split %d silver coins. Your share is %d "
             "silver.\n\r", amount_silver, share_silver + extra_silver);
        sprintf (buf, "$n splits %d silver coins. Your share is %d silver.",
             amount_silver, share_silver);
    }
    else if (share_silver == 0) {
        printf_to_char (ch, "You split %d goin coins. Your share is %d "
             "gold.\n\r", amount_gold, share_gold + extra_gold);
        sprintf (buf, "$n splits %d gold coins. Your share is %d gold.",
             amount_gold, share_gold);
    }
    else {
        printf_to_char (ch, "You split %d silver and %d goin coins. "
            "Your share is %d silver and %d gold.\n\r",
            amount_silver, amount_gold,
            share_silver + extra_silver, share_gold + extra_gold);
        sprintf (buf, "$n splits %d silver and %d gold coins. "
            "Your share is %d silver and %d gold.",
            amount_silver, amount_gold, share_silver, share_gold);
    }

    for (gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room) {
        if (gch != ch && is_same_group (gch, ch)
            && !IS_AFFECTED (gch, AFF_CHARM))
        {
            act (buf, ch, NULL, gch, TO_VICT);
            gch->gold += share_gold;
            gch->silver += share_silver;
        }
    }
}

void do_gtell (CHAR_DATA * ch, char *argument) {
    CHAR_DATA *gch;

    BAIL_IF (argument[0] == '\0',
        "Tell your group what?\n\r", ch);
    BAIL_IF (IS_SET (ch->comm, COMM_NOTELL),
        "Your message didn't get through!\n\r", ch);

    for (gch = char_list; gch != NULL; gch = gch->next)
        if (is_same_group (gch, ch))
            act_new ("$n tells the group '$t'",
                     ch, argument, gch, TO_VICT, POS_SLEEPING);
}

void do_follow (CHAR_DATA * ch, char *argument) {
    /* RT changed to allow unlimited following and follow the NOFOLLOW rules */
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    DO_REQUIRE_ARG (arg, "Follow whom?\n\r");

    BAIL_IF ((victim = find_char_same_room (ch, arg)) == NULL,
        "They aren't here.\n\r", ch);
    BAIL_IF_ACT (IS_AFFECTED (ch, AFF_CHARM) && ch->master != NULL,
        "But you'd rather follow $N!", ch, NULL, ch->master);
    if (victim == ch) {
        BAIL_IF (ch->master == NULL,
            "You already follow yourself.\n\r", ch);
        stop_follower (ch);
        return;
    }

    BAIL_IF_ACT (!IS_NPC (victim) && IS_SET (victim->plr, PLR_NOFOLLOW) &&
                 !IS_IMMORTAL (ch),
        "$N doesn't seem to want any followers.\n\r", ch, NULL, victim);

    REMOVE_BIT (ch->plr, PLR_NOFOLLOW);
    if (ch->master != NULL)
        stop_follower (ch);

    add_follower (ch, victim);
}
