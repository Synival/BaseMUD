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

/*   QuickMUD - The Lazy Man's ROM - $Id: act_obj.c,v 1.2 2000/12/01 10:48:33 ring0 Exp $ */

#include <stdlib.h>

#include "interp.h"
#include "affects.h"
#include "utils.h"
#include "comm.h"
#include "db.h"
#include "fight.h"
#include "groups.h"
#include "mob_prog.h"
#include "save.h"
#include "magic.h"
#include "act_group.h"
#include "act_comm.h"
#include "act_move.h"
#include "recycle.h"
#include "chars.h"
#include "objs.h"
#include "rooms.h"
#include "find.h"
#include "globals.h"
#include "lookup.h"
#include "items.h"
#include "players.h"

#include "act_obj.h"

bool do_filter_can_drop_item (CHAR_T *ch, OBJ_T *obj, bool msg) {
    FILTER (!char_can_see_obj (ch, obj),
        (!msg) ? NULL : "You can't find it.\n\r", ch);
    FILTER (obj->wear_loc != WEAR_LOC_NONE,
        (!msg) ? NULL : "You must remove it first.\n\r", ch);
    return FALSE;
}

bool do_filter_can_put_item (CHAR_T *ch, OBJ_T *obj, OBJ_T *container,
    bool msg)
{
    FILTER (obj == container,
        (!msg) ? NULL : "You can't fold it into itself.\n\r", ch);
    if (do_filter_can_drop_item (ch, obj, msg))
        return TRUE;
    return FALSE;
}

bool do_filter_put_or_get_valid_container (CHAR_T *ch, char *arg,
    OBJ_T **out_container)
{
    OBJ_T *container;
    FILTER (!str_cmp (arg, "all") || !str_prefix ("all.", arg),
        "You can't do that.\n\r", ch);
    FILTER_ACT ((container = find_obj_here (ch, arg)) == NULL,
        "You see no $T here.", ch, NULL, arg);
    FILTER (!item_is_container (container),
        "That's not a container.\n\r", ch);
    FILTER_ACT (item_is_closed (container),
        "The $p is closed.", ch, container, NULL);
    if (out_container)
        *out_container = container;
    return FALSE;
}

bool do_filter_can_give_item (CHAR_T *ch, OBJ_T *obj, CHAR_T *victim,
    bool msg)
{
    if (do_filter_can_drop_item (ch, obj, msg))
        return TRUE;
    return FALSE;
}

char *do_obj_parse_arg (const char *arg, int *out_type) {
    if (!str_cmp (arg, "all")) {
        *out_type = OBJ_ALL;
        return (char *) (arg + 3);
    }
    else if (!str_prefix ("all.", arg)) {
        *out_type = OBJ_ALL_OF;
        return (char *) (arg + 4);
    }
    else {
        *out_type = OBJ_SINGLE;
        return (char *) arg;
    }
}

void do_drop_single_item (CHAR_T *ch, OBJ_T *obj) {
    BAIL_IF_ACT (!char_can_drop_obj (ch, obj),
        "$p: You can't let go of it.", ch, obj, NULL);

    obj_give_to_room (obj, ch->in_room);
    act2 ("You drop $p.",
          "$n drops $p.", ch, obj, NULL, 0, POS_RESTING);

    if (IS_OBJ_STAT (obj, ITEM_MELT_DROP)) {
        act ("$p dissolves into smoke.", ch, obj, NULL, TO_ALL);
        obj_extract (obj);
    }
}

void do_put_single_item (CHAR_T *ch, OBJ_T *obj, OBJ_T *container) {
    const char **put_msgs;

    BAIL_IF_ACT (!char_can_drop_obj (ch, obj),
        "$p: You can't let go of it.", ch, obj, NULL);
    BAIL_IF_ACT (item_get_weight_mult (obj) != 100,
        "$p: You have a feeling that would be a bad idea.", ch, obj, NULL);
    BAIL_IF_ACT (!item_can_fit_obj_in (container, obj),
        "$p: It won't fit.", ch, obj, NULL);

    if (obj_is_donation_pit (container) && !obj_can_wear_flag (obj, ITEM_TAKE)) {
        if (obj->timer)
            SET_BIT (obj->extra_flags, ITEM_HAD_TIMER);
        else
            obj->timer = number_range (100, 200);
    }

    obj_give_to_obj (obj, container);

    put_msgs = item_get_put_messages (container);
    act2 (put_msgs[0], put_msgs[1], ch, obj, container, 0, POS_RESTING);
}

void do_get_container (CHAR_T *ch, char *arg1, char *arg2) {
    OBJ_T *container;
    OBJ_T *obj, *obj_start, *obj_next;
    int type;
    bool found = FALSE, failed = FALSE;

    /* get the container and make sure it's valid. */
    BAIL_IF (!str_cmp (arg2, "all") || !str_prefix ("all.", arg2),
        "You can't do that.\n\r", ch);
    if (do_filter_put_or_get_valid_container (ch, arg2, &container))
        return;
    arg1 = do_obj_parse_arg (arg1, &type);

    BAIL_IF_ACT (item_is_closed (container),
        "The $p is closed.", ch, container, NULL);
    BAIL_IF (!item_can_loot_as (container, ch),
        "You can't do that.\n\r", ch);
    BAIL_IF (type != OBJ_SINGLE && !IS_IMMORTAL (ch) &&
             obj_is_donation_pit (container),
        "Don't be so greedy!\n\r", ch);

    obj_start = (type != OBJ_SINGLE) ? container->content_first
        : find_obj_container (ch, container, arg1);

    for (obj = obj_start; obj != NULL; obj = obj_next) {
        obj_next = (type == OBJ_SINGLE) ? NULL : obj->content_next;
        if (type == OBJ_ALL_OF && !str_in_namelist (arg1, obj->name))
            continue;
        if (!char_can_see_obj (ch, obj))
            continue;
        char_take_obj (ch, obj, container);
        found = TRUE;
    }
    if (!found) {
        if (type == OBJ_ALL)
            act ("You see nothing in $P.", ch, NULL, container, TO_CHAR);
        else if (type == OBJ_ALL_OF || failed || obj_start == NULL)
            act ("You see no $t in $P.", ch, arg1, container, TO_CHAR);
    }
}

void do_get_room (CHAR_T *ch, char *argument) {
    OBJ_T *obj, *obj_start, *obj_next;
    int type;
    bool found = FALSE, failed = FALSE;

    argument = do_obj_parse_arg (argument, &type);
    obj_start = (type != OBJ_SINGLE) ? ch->in_room->content_first
        : find_obj_same_room (ch, argument);

    for (obj = obj_start; obj != NULL; obj = obj_next) {
        obj_next = (type == OBJ_SINGLE) ? NULL : obj->content_next;
        if (type == OBJ_ALL_OF && !str_in_namelist (argument, obj->name))
            continue;
        if (!char_can_see_obj (ch, obj))
            continue;
        char_take_obj (ch, obj, NULL);
        found = TRUE;
    }
    if (!found) {
        if (type == OBJ_ALL)
            printf_to_char (ch, "You see nothing here.\n\r");
        else if (type == OBJ_ALL_OF || failed || obj_start == NULL)
            act ("You see no $T here.", ch, NULL, argument, TO_CHAR);
    }
}

DEFINE_DO_FUN (do_get) {
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];

    DO_REQUIRE_ARG (arg1, "Get what?\n\r");
    argument = one_argument (argument, arg2);
    if (!str_cmp (arg2, "from"))
        argument = one_argument (argument, arg2);

    /* if there's a second argument, we're getting from a container. */
    if (arg2[0] != '\0')
        do_get_container (ch, arg1, arg2);
    else
        do_get_room (ch, arg1);
}

DEFINE_DO_FUN (do_put) {
    OBJ_T *container;
    OBJ_T *obj, *obj_start, *obj_next;
    char arg1[MAX_INPUT_LENGTH], *arg;
    char arg2[MAX_INPUT_LENGTH];
    int type;
    bool found = FALSE, failed = FALSE;

    DO_REQUIRE_ARG (arg1, "Put what in what?\n\r");
    DO_REQUIRE_ARG (arg2, "Put it in what?\n\r");
    if (do_filter_put_or_get_valid_container (ch, arg2, &container))
        return;
    arg = do_obj_parse_arg (arg1, &type);

    /* putting cannot be done into corpses - only containers. */
    BAIL_IF (!item_can_put_objs (container),
        "That's not a container.\n\r", ch);

    obj_start = (type != OBJ_SINGLE) ? ch->content_first
        : find_obj_own_inventory (ch, arg);

    for (obj = obj_start; obj != NULL; obj = obj_next) {
        obj_next = (type == OBJ_SINGLE) ? NULL : obj->content_next;
        if (type == OBJ_ALL_OF && !str_in_namelist (arg, obj->name))
            continue;
        if (do_filter_can_put_item (ch, obj, container, type == OBJ_SINGLE)) {
            failed = TRUE;
            continue;
        }
        do_put_single_item (ch, obj, container);
        found = TRUE;
    }
    if (!found) {
        if (type == OBJ_ALL)
            printf_to_char (ch, "You are not carrying anything.\n\r");
        else if (type == OBJ_ALL_OF || failed || obj_start == NULL)
            act ("You are not carrying any $T.", ch, NULL, arg, TO_CHAR);
    }
}

void do_drop_money (CHAR_T *ch, char *arg1, char *argument) {
    char arg[MAX_INPUT_LENGTH];
    int gold = 0, silver = 0;
    bool is_silver;
    int amount = atoi (arg1);

    argument = one_argument (argument, arg);
    BAIL_IF (amount <= 0
            || (str_cmp (arg, "coins") && str_cmp (arg, "coin") &&
                str_cmp (arg, "gold")  && str_cmp (arg, "silver")),
        "Sorry, you can't do that.\n\r", ch);

    is_silver = str_cmp (arg, "gold");
    if (is_silver) {
        BAIL_IF (ch->silver < amount,
            "You don't have that much silver.\n\r", ch);
        ch->silver -= amount;
        silver = amount;
    }
    else {
        BAIL_IF (ch->gold < amount,
            "You don't have that much gold.\n\r", ch);
        ch->gold -= amount;
        gold = amount;
    }

    room_add_money (ch->in_room, gold, silver);
    printf_to_char (ch, "You drop %d %s coins.\n\r", amount,
        is_silver ? "silver" : "gold");
    act ("$n drops some coins.", ch, NULL, NULL, TO_NOTCHAR);
}

DEFINE_DO_FUN (do_drop) {
    char arg1[MAX_INPUT_LENGTH], *arg;
    OBJ_T *obj, *obj_start, *obj_next;
    int type;
    bool found = FALSE, failed = FALSE;

    DO_REQUIRE_ARG (arg1, "Drop what?\n\r");

    /* 'drop NNNN coins' */
    if (is_number (arg1)) {
        do_drop_money (ch, arg1, argument);
        return;
    }

    arg = do_obj_parse_arg (arg1, &type);
    obj_start = (type != OBJ_SINGLE) ? ch->content_first
        : find_obj_own_inventory (ch, arg);

    for (obj = obj_start; obj != NULL; obj = obj_next) {
        obj_next = (type == OBJ_SINGLE) ? NULL : obj->content_next;
        if (type == OBJ_ALL_OF && !str_in_namelist (arg, obj->name))
            continue;
        if (do_filter_can_drop_item (ch, obj, type == OBJ_SINGLE)) {
            failed = TRUE;
            continue;
        }
        do_drop_single_item (ch, obj);
        found = TRUE;
    }
    if (!found) {
        if (type == OBJ_ALL)
            printf_to_char (ch, "You are not carrying anything.\n\r");
        else if (type == OBJ_ALL_OF || failed || obj_start == NULL)
            act ("You are not carrying any $T.", ch, NULL, arg, TO_CHAR);
    }
}

void do_give_money (CHAR_T *ch, char *arg1, char *arg2, char *argument) {
    /* 'give NNNN coins victim' */
    char buf[MAX_STRING_LENGTH];
    CHAR_T *victim;
    bool is_silver;
    int amount = atoi (arg1);

    BAIL_IF (amount <= 0 ||
            (str_cmp (arg2, "coins") && str_cmp (arg2, "coin") &&
             str_cmp (arg2, "gold") && str_cmp (arg2, "silver")),
        "Sorry, you can't do that.\n\r", ch);

    is_silver = str_cmp (arg2, "gold");
    argument = one_argument (argument, arg2);

    BAIL_IF (arg2[0] == '\0',
        "Give what to whom?\n\r", ch);
    BAIL_IF ((victim = find_char_same_room (ch, arg2)) == NULL,
        "They aren't here.\n\r", ch);
    BAIL_IF ((!is_silver && ch->gold   < amount) ||
              (is_silver && ch->silver < amount),
        "You haven't got that much.\n\r", ch);

    if (is_silver) {
        ch->silver -= amount;
        victim->silver += amount;
    }
    else {
        ch->gold -= amount;
        victim->gold += amount;
    }

    sprintf (buf, "$n gives you %d %s.", amount,
             is_silver ? "silver" : "gold");
    act (buf, ch, NULL, victim, TO_VICT);
    act ("$n gives $N some coins.", ch, NULL, victim, TO_OTHERS);
    sprintf (buf, "You give $N %d %s.", amount,
             is_silver ? "silver" : "gold");
    act (buf, ch, NULL, victim, TO_CHAR);

    /* Bribe trigger */
    if (IS_NPC (victim) && HAS_TRIGGER (victim, TRIG_BRIBE))
        mp_bribe_trigger (victim, ch, is_silver ? amount : amount * 100);

    if (IS_NPC (victim) && EXT_IS_SET (victim->ext_mob, MOB_IS_CHANGER)) {
        int change;
        int can_see;

        change = (is_silver ? (95 * amount / 100 / 100) : (95 * amount));
        if (!is_silver && change > victim->silver)
            victim->silver += change;
        if (is_silver && change > victim->gold)
            victim->gold += change;

        can_see = char_can_see_anywhere (victim, ch);
        if (change < 1 && can_see) {
            act ("$n tells you 'I'm sorry, you did not give me enough to change.'",
                 victim, NULL, ch, TO_VICT);
            ch->reply = victim;
            sprintf (buf, "%d %s %s",
                     amount, is_silver ? "silver" : "gold", ch->name);
            do_function (victim, &do_give, buf);
        }
        else if (can_see) {
            sprintf (buf, "%d %s %s",
                     change, is_silver ? "gold" : "silver", ch->name);
            do_function (victim, &do_give, buf);
            if (is_silver) {
                sprintf (buf, "%d silver %s",
                         (95 * amount / 100 - change * 100), ch->name);
                do_function (victim, &do_give, buf);
            }
            act ("$n tells you 'Thank you, come again.'",
                 victim, NULL, ch, TO_VICT);
            ch->reply = victim;
        }
    }
}

void do_give_single_item (CHAR_T *ch, OBJ_T *obj, CHAR_T *victim) {
    BAIL_IF_ACT (victim->carry_number + obj_get_carry_number (obj) >
            char_get_max_carry_count (victim),
        "$N has $S hands full.", ch, NULL, victim);
    BAIL_IF_ACT (char_get_carry_weight (victim) + obj_get_weight (obj) >
            char_get_max_carry_weight (victim),
        "$N can't carry that much weight.", ch, NULL, victim);
    BAIL_IF_ACT (!char_can_see_obj (victim, obj),
        "$N can't see it.", ch, NULL, victim);

    if (IS_NPC (victim) && victim->mob_index->shop != NULL) {
        act ("$N tells you 'Sorry, you'll have to sell that.'",
             ch, NULL, victim, TO_CHAR);
        return;
    }

    obj_give_to_char (obj, victim);
    trigger_mobs = FALSE;
    act3 ("$n gives $p to $N.",
          "$n gives you $p.",
          "You give $p to $N.", ch, obj, victim, 0, POS_RESTING);
    trigger_mobs = TRUE;

    /* Give trigger */
    if (IS_NPC (victim) && HAS_TRIGGER (victim, TRIG_GIVE))
        mp_give_trigger (victim, ch, obj);
}

DEFINE_DO_FUN (do_give) {
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    CHAR_T *victim;
    OBJ_T *obj;

    DO_REQUIRE_ARG (arg1, "Give what to whom?\n\r");
    DO_REQUIRE_ARG (arg2, "Give it to whom?\n\r");

    if (is_number (arg1)) {
        do_give_money (ch, arg1, arg2, argument);
        return;
    }

    BAIL_IF ((obj = find_obj_own_inventory (ch, arg1)) == NULL,
        "You do not have that item.\n\r", ch);
    BAIL_IF ((victim = find_char_same_room (ch, arg2)) == NULL,
        "They aren't here.\n\r", ch);
    if (do_filter_can_give_item (ch, obj, victim, TRUE))
        return;

    do_give_single_item (ch, obj, victim);
}

/* for poisoning weapons and food/drink */
DEFINE_DO_FUN (do_envenom) {
    OBJ_T *obj;
    int skill;

    /* find out what */
    BAIL_IF (argument[0] == '\0',
        "Envenom what item?\n\r", ch);
    BAIL_IF ((obj = find_obj_own_char (ch, argument)) == NULL,
        "You don't have that item.\n\r", ch);
    BAIL_IF ((skill = char_get_skill (ch, SN(ENVENOM))) < 1,
        "Are you crazy? You'd poison yourself!\n\r", ch);

    if (!item_envenom_effect (obj, ch, skill, SN(ENVENOM), FALSE))
        act ("You can't poison $p.", ch, obj, NULL, TO_CHAR);

    WAIT_STATE (ch, skill_table[SN(ENVENOM)].beats);
}

DEFINE_DO_FUN (do_fill) {
    char arg[MAX_INPUT_LENGTH];
    char buf1[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    OBJ_T *obj;
    OBJ_T *fountain;
    const char *liq;

    /* Find a valid container. */
    DO_REQUIRE_ARG (arg, "Fill what?\n\r");
    BAIL_IF ((obj = find_obj_own_inventory (ch, arg)) == NULL,
        "You do not have that item.\n\r", ch);
    BAIL_IF (!item_can_fill (obj),
        "You can't fill that.\n\r", ch);
    BAIL_IF ((fountain = room_get_obj_of_type (ch->in_room, ch,
            ITEM_FOUNTAIN)) == NULL,
        "There is no fountain here!\n\r", ch);

    /* Can we fill our container with this fountain? */
    BAIL_IF (obj->v.drink_con.filled != 0 &&
             obj->v.drink_con.liquid != fountain->v.fountain.liquid,
        "There is already another liquid in it.\n\r", ch);
    BAIL_IF (obj->v.drink_con.filled >= obj->v.drink_con.capacity,
        "Your container is full.\n\r", ch);

    obj->v.drink_con.liquid   = fountain->v.fountain.liquid;
    obj->v.drink_con.poisoned = fountain->v.fountain.poisoned;
    obj->v.drink_con.filled   = obj->v.drink_con.capacity;

    liq = liq_get_name (fountain->v.fountain.liquid);
    snprintf (buf1, sizeof(buf1), "You fill $p with %s from $P.", liq);
    snprintf (buf2, sizeof(buf2), "$n fills $p with %s from $P.", liq);
    act2 (buf1, buf2, ch, obj, fountain, 0, POS_RESTING);
}

DEFINE_DO_FUN (do_pour) {
    char arg1[MAX_STRING_LENGTH];
    char arg2[MAX_STRING_LENGTH];
    char buf1[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    char buf3[MAX_STRING_LENGTH];
    const char *liq;
    OBJ_T *out, *in;
    CHAR_T *vch = NULL;
    int amount;

    DO_REQUIRE_ARG (arg1, "Pour what into what?\n\r");
    DO_REQUIRE_ARG (arg2, "Pour it into what?\n\r");

    BAIL_IF ((out = find_obj_own_inventory (ch, arg1)) == NULL,
        "You don't have that item.\n\r", ch);
    BAIL_IF (!item_can_fill (out),
        "That's not a drink container.\n\r", ch);
    liq = liq_get_name (out->v.drink_con.liquid);

    if (!str_cmp (arg2, "out")) {
        BAIL_IF (out->v.drink_con.filled == 0,
            "It's already empty.\n\r", ch);

        out->v.drink_con.filled   = 0;
        out->v.drink_con.poisoned = 0;

        snprintf (buf1, sizeof(buf1),
            "You invert $p, spilling %s all over the ground.", liq);
        snprintf (buf2, sizeof(buf2),
            "$n inverts $p, spilling %s all over the ground.", liq);
        act2 (buf1, buf2, ch, out, NULL, 0, POS_RESTING);
        return;
    }

    if ((in = find_obj_here (ch, arg2)) == NULL) {
        BAIL_IF ((vch = find_char_same_room (ch, arg2)) == NULL,
            "Pour into what?\n\r", ch);
        BAIL_IF ((in = char_get_eq_by_wear_loc (vch, WEAR_LOC_HOLD)) == NULL,
            "They aren't holding anything.", ch);
    }

    BAIL_IF (!item_can_fill (in),
        "You can only pour into other drink containers.\n\r", ch);
    BAIL_IF (in == out,
        "You cannot change the laws of physics!\n\r", ch);
    BAIL_IF (in->v.drink_con.filled != 0 &&
             in->v.drink_con.liquid != out->v.drink_con.liquid,
        "They don't hold the same liquid.\n\r", ch);
    BAIL_IF_ACT (out->v.drink_con.filled == 0,
        "There's nothing in $p to pour.", ch, out, NULL);
    BAIL_IF_ACT (in->v.drink_con.filled >= in->v.drink_con.capacity,
        "$p is already filled to the top.", ch, in, NULL);

    amount = UMIN (out->v.drink_con.filled,
        in->v.drink_con.capacity - in->v.drink_con.filled);

    in ->v.drink_con.filled += amount;
    out->v.drink_con.filled -= amount;
    in ->v.drink_con.liquid  = out->v.drink_con.liquid;

    if (vch == NULL) {
        snprintf (buf1, sizeof(buf1), "You pour %s from $p into $P.", liq);
        snprintf (buf2, sizeof(buf2), "$n pours %s from $p into $P.", liq);
        act2 (buf1, buf2, ch, out, in, 0, POS_RESTING);
    }
    else {
        snprintf (buf1, sizeof(buf1), "You pour some %s for $N.", liq);
        snprintf (buf2, sizeof(buf2), "$n pours you some %s.",    liq);
        snprintf (buf3, sizeof(buf3), "$n pours some %s for $N.", liq);
        act3 (buf1, buf2, buf3, ch, NULL, vch, 0, POS_RESTING);
    }
}

DEFINE_DO_FUN (do_drink) {
    const LIQ_T *liq;
    OBJ_T *obj;
    char arg[MAX_INPUT_LENGTH];

    one_argument (argument, arg);
    if (arg[0] == '\0') {
        BAIL_IF ((obj = room_get_obj_of_type (ch->in_room, ch,
                ITEM_FOUNTAIN)) == NULL,
            "Drink what?\n\r", ch);
    }
    else {
        BAIL_IF ((obj = find_obj_here (ch, arg)) == NULL,
            "You can't find it.\n\r", ch);
    }

    BAIL_IF (IS_DRUNK (ch),
        "You fail to reach your mouth. *Hic*\n\r", ch);
    BAIL_IF (IS_FULL (ch) && !IS_IMMORTAL (ch),
        "You're too full to drink more.\n\r", ch);
    BAIL_IF (!item_is_drinkable (obj),
        "You can't drink from that.\n\r", ch);
    BAIL_IF (!item_has_liquid (obj),
        "It is already empty.\n\r", ch);
    BAIL_IF ((liq = item_get_liquid (obj)) == NULL,
        "It is already empty.\n\r", ch);

    act2 ("You drink $T from $p.", "$n drinks $T from $p.",
         ch, obj, liq->name, 0, POS_RESTING);

    if (!item_drink_effect (obj, ch))
        printf_to_char (ch, "...but nothing happens.\n\r");
}

DEFINE_DO_FUN (do_eat) {
    char arg[MAX_INPUT_LENGTH];
    OBJ_T *obj;

    DO_REQUIRE_ARG (arg, "Eat what?\n\r");
    BAIL_IF ((obj = find_obj_own_inventory (ch, arg)) == NULL,
        "You do not have that item.\n\r", ch);
    if (!IS_IMMORTAL (ch)) {
        BAIL_IF (!item_is_edible (obj),
            "That's not edible.\n\r", ch);
        BAIL_IF (IS_FULL (ch),
            "You're too full to eat more.\n\r", ch);
    }

    act2 ("You eat $p.", "$n eats $p.", ch, obj, NULL, 0, POS_RESTING);
    if (!item_eat_effect (obj, ch))
        printf_to_char (ch, "...but nothing happens.\n\r");
}

DEFINE_DO_FUN (do_wear) {
    char arg[MAX_INPUT_LENGTH];
    OBJ_T *obj;

    DO_REQUIRE_ARG (arg, "Wear, wield, or hold what?\n\r");
    if (!str_cmp (arg, "all")) {
        OBJ_T *obj_next;
        bool success;

        success = FALSE;
        for (obj = ch->content_first; obj != NULL; obj = obj_next) {
            obj_next = obj->content_next;
            if (obj->wear_loc == WEAR_LOC_NONE && char_can_see_obj (ch, obj))
                if (char_wear_obj (ch, obj, FALSE))
                    success = TRUE;
        }
        if (!success)
            send_to_char ("You find nothing new to wear, wield, "
                          "or hold.\n\r", ch);
        return;
    }

    BAIL_IF ((obj = find_obj_own_inventory (ch, arg)) == NULL,
        "You do not have that item.\n\r", ch);
    char_wear_obj (ch, obj, TRUE);
}

DEFINE_DO_FUN (do_remove) {
    char arg[MAX_INPUT_LENGTH];
    OBJ_T *obj;

    DO_REQUIRE_ARG (arg, "Remove what?\n\r");
    BAIL_IF ((obj = find_obj_own_worn (ch, arg)) == NULL,
        "You do not have that item.\n\r", ch);
    char_remove_obj (ch, obj->wear_loc, TRUE, FALSE);
}

DEFINE_DO_FUN (do_sacrifice) {
    char arg[MAX_INPUT_LENGTH];
    OBJ_T *obj;
    int silver;

    /* variables for AUTOSPLIT */
    CHAR_T *gch;
    int members;
    char buffer[100];

    DO_REQUIRE_ARG (arg, "Sacrifice what to Mota?\n\r");

    /* This was the common 'sacrifice' no-argument message, which didn't
     * make much sense, so the joke was lost. */
    if (!str_cmp (arg, ch->name) || !str_cmp (arg, "self")) {
        act2 ("Mota appreciates your offer and may accept it later.",
              "$n offers $mself to Mota, who graciously declines.",
             ch, NULL, NULL, 0, POS_RESTING);
        return;
    }

    BAIL_IF ((obj = find_obj_same_room (ch, arg)) == NULL,
        "You can't find it.\n\r", ch);
    BAIL_IF (!item_can_sacrifice (obj),
        "Mota wouldn't like that.\n\r", ch);
    BAIL_IF_ACT (!obj_can_wear_flag (obj, ITEM_TAKE) ||
                  obj_can_wear_flag (obj, ITEM_NO_SAC),
        "$p is not an acceptable sacrifice.", ch, obj, NULL);

    if (obj->in_room != NULL)
        for (gch = obj->in_room->people_first; gch; gch = gch->room_next)
            BAIL_IF_ACT (gch->on == obj,
                "$N appears to be using $p.", ch, obj, gch);

    silver = item_get_sacrifice_silver (obj);
#ifdef BASEMUD_NO_WORTHLESS_SACRIFICES
    BAIL_IF (silver == 0,
        "Mota ignores your paltry offering.\n\r", ch);
#endif

    if (silver == 1)
        printf_to_char (ch, "Mota gives you one silver coin for your sacrifice.\n\r");
    else {
        printf_to_char (ch, "Mota gives you %d silver coins for your sacrifice.\n\r",
            silver);
    }

    ch->silver += silver;
    if (EXT_IS_SET (ch->ext_plr, PLR_AUTOSPLIT)) { /* AUTOSPLIT code */
        members = 0;
        for (gch = ch->in_room->people_first; gch != NULL; gch = gch->room_next)
            if (is_same_group (gch, ch))
                members++;
        if (members > 1 && silver > 1) {
            sprintf (buffer, "%d", silver);
            do_function (ch, &do_split, buffer);
        }
    }

    wiznet ("$N sends up $p as a burnt offering.", ch, obj, WIZ_SACCING, 0, 0);
    act ("$n sacrifices $p to Mota.", ch, obj, NULL, TO_NOTCHAR);
    obj_extract (obj);
}

DEFINE_DO_FUN (do_quaff) {
    char arg[MAX_INPUT_LENGTH];
    OBJ_T *obj;

    DO_REQUIRE_ARG (arg, "Quaff what?\n\r");
    BAIL_IF ((obj = find_obj_own_inventory (ch, arg)) == NULL,
        "You do not have that potion.\n\r", ch);
    BAIL_IF (!item_can_quaff (obj),
        "You can quaff only potions.\n\r", ch);
    BAIL_IF (ch->level < obj->level,
        "This liquid is too powerful for you to drink.\n\r", ch);

    act2 ("You quaff $p.", "$n quaffs $p.",
        ch, obj, NULL, 0, POS_RESTING);

    if (!item_quaff_effect (obj, ch))
        printf_to_char (ch, "...but nothing happens.\n\r");
}

DEFINE_DO_FUN (do_recite) {
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    CHAR_T *victim;
    OBJ_T *scroll, *obj;

    DO_REQUIRE_ARG (arg1, "Recite what?\n\r");
    argument = one_argument (argument, arg2);

    BAIL_IF ((scroll = find_obj_own_inventory (ch, arg1)) == NULL,
        "You do not have that scroll.\n\r", ch);
    BAIL_IF (!item_can_recite (scroll),
        "You can recite only scrolls.\n\r", ch);
    BAIL_IF (ch->level < scroll->level,
        "This scroll is too complex for you to comprehend.\n\r", ch);

    obj = NULL;
    if (arg2[0] == '\0')
        victim = ch;
    else {
        BAIL_IF ((victim = find_char_same_room (ch, arg2)) == NULL &&
                 (obj = find_obj_here (ch, arg2)) == NULL,
            "You can't find it.\n\r", ch);
    }

    act2 ("You recite $p.", "$n recites $p.",
        ch, scroll, NULL, 0, POS_RESTING);

    if (number_percent () >= 20 + char_get_skill (ch, SN(SCROLLS)) * 4 / 5) {
        printf_to_char (ch, "You mispronounce a syllable.\n\r");
        player_try_skill_improve (ch, SN(SCROLLS), FALSE, 2);
        obj_extract (scroll);
        return;
    }

    if (!item_recite_effect (scroll, ch, victim, obj)) {
        printf_to_char (ch, "...but nothing happens.\n\r");
        return;
    }

    player_try_skill_improve (ch, SN(SCROLLS), TRUE, 2);
}

DEFINE_DO_FUN (do_brandish) {
    OBJ_T *staff;
    bool success;

    BAIL_IF ((staff = char_get_eq_by_wear_loc (ch, WEAR_LOC_HOLD)) == NULL,
        "You hold nothing in your hand.\n\r", ch);
    BAIL_IF (!item_can_brandish (staff),
        "You can brandish only with a staff.\n\r", ch);

    WAIT_STATE (ch, 2 * PULSE_VIOLENCE);

    if (item_has_charges (staff)) {
        act2 ("You brandish $p.", "$n brandishes $p.",
            ch, staff, NULL, 0, POS_RESTING);

        success = TRUE;
        if (ch->level < staff->level)
            success = FALSE;
        else if (number_percent () >= 20 + char_get_skill (ch, SN(STAVES)) * 4 / 5)
            success = FALSE;
        else if (!item_brandish_effect (staff, ch, TRUE))
            success = FALSE;

        if (!success) {
            act2 ("You fail to invoke $p.", "...and nothing happens.",
                ch, staff, NULL, 0, POS_RESTING);
            player_try_skill_improve (ch, SN(STAVES), FALSE, 2);
        }
    }

    item_consume_charge_as (staff, ch);
}

DEFINE_DO_FUN (do_zap) {
    bool success;
    char arg[MAX_INPUT_LENGTH];
    CHAR_T *victim;
    OBJ_T *wand;
    OBJ_T *obj;

    one_argument (argument, arg);
    BAIL_IF (arg[0] == '\0' && ch->fighting == NULL,
        "Zap whom or what?\n\r", ch);
    BAIL_IF ((wand = char_get_eq_by_wear_loc (ch, WEAR_LOC_HOLD)) == NULL,
        "You hold nothing in your hand.\n\r", ch);
    BAIL_IF (!item_can_zap (wand),
        "You can zap only with a wand.\n\r", ch);

    obj = NULL;
    if (arg[0] == '\0') {
        BAIL_IF ((victim = ch->fighting) == NULL,
            "Zap whom or what?\n\r", ch);
    }
    else {
        BAIL_IF ((victim = find_char_same_room (ch, arg)) == NULL &&
                 (obj = find_obj_here (ch, arg)) == NULL,
            "You can't find it.\n\r", ch);
    }

    WAIT_STATE (ch, 2 * PULSE_VIOLENCE);

    if (item_has_charges (wand)) {
        if (victim != NULL) {
            act3 ("You zap $N with $p.",
                  "$n zaps you with $p.",
                  "$n zaps $N with $p.",  ch, wand, victim, 0, POS_RESTING);
        }
        else {
            act2 ("You zap $P with $p.",
                  "$n zaps $P with $p.", ch, wand, obj, 0, POS_RESTING);
        }

        success = TRUE;
        if (ch->level < wand->level)
            success = FALSE;
        else if (number_percent () >= 20 + char_get_skill (ch, SN(WANDS)) * 4 / 5)
            success = FALSE;
        else if (!item_zap_effect (wand, ch, victim, obj))
            success = FALSE;

        if (!success) {
            act ("Your efforts with $p produce only smoke and sparks.",
                 ch, wand, NULL, TO_CHAR);
            act ("$n's efforts with $p produce only smoke and sparks.",
                 ch, wand, NULL, TO_NOTCHAR);
            player_try_skill_improve (ch, SN(WANDS), FALSE, 2);
        }
        else
            player_try_skill_improve (ch, SN(WANDS), TRUE, 2);
    }

    item_consume_charge_as (wand, ch);
}

DEFINE_DO_FUN (do_steal) {
    char buf[MAX_STRING_LENGTH];
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    CHAR_T *victim;
    OBJ_T *obj;
    int percent;

    DO_REQUIRE_ARG (arg1, "Steal what from whom?\n\r");
    DO_REQUIRE_ARG (arg2, "Steal it from whom?\n\r");

    BAIL_IF ((victim = find_char_same_room (ch, arg2)) == NULL,
        "They aren't here.\n\r", ch);
    BAIL_IF (victim == ch,
        "That's pointless.\n\r", ch);
    if (do_filter_can_attack (ch, victim))
        return;
    BAIL_IF (IS_NPC (victim) && (victim->fighting ||
            victim->position == POS_FIGHTING),
        "You'd better not -- you might get hit.\n\r", ch);

    WAIT_STATE (ch, skill_table[SN(STEAL)].beats);
    percent = number_percent ();

    if (!IS_AWAKE (victim))
        percent -= 10;
    else if (!char_can_see_anywhere (victim, ch))
        percent += 25;
    else
        percent += 50;

    if (((ch->level + 7 < victim->level || ch->level - 7 > victim->level)
         && !IS_NPC (victim) && !IS_NPC (ch))
        || (!IS_NPC (ch) && percent > char_get_skill (ch, SN(STEAL)))
        || (!IS_NPC (ch) && !player_has_clan (ch)))
    {
        /* Failure. */
        printf_to_char (ch, "Oops.\n\r");
        affect_strip_char (ch, SN(SNEAK));
        REMOVE_BIT (ch->affected_by, AFF_SNEAK);

        act ("$n tried to steal from you.\n\r", ch, NULL, victim, TO_VICT);
        act ("$n tried to steal from $N.\n\r",  ch, NULL, victim, TO_OTHERS);
        switch (number_range (0, 3)) {
            case 0:
                sprintf (buf, "%s is a lousy thief!", ch->name);
                break;
            case 1:
                sprintf (buf, "%s couldn't rob %s way out of a paper bag!",
                         ch->name, (ch->sex == 2) ? "her" : "his");
                break;
            case 2:
                sprintf (buf, "%s tried to rob me!", ch->name);
                break;
            case 3:
                sprintf (buf, "Keep your hands out of there, %s!", ch->name);
                break;
        }
        if (!IS_AWAKE (victim))
            do_function (victim, &do_wake, "");
        if (IS_AWAKE (victim))
            do_function (victim, &do_yell, buf);
        if (!IS_NPC (ch)) {
            if (IS_NPC (victim)) {
                player_try_skill_improve (ch, SN(STEAL), FALSE, 2);
                multi_hit (victim, ch, ATTACK_DEFAULT);
            }
            else {
                wiznetf (ch, NULL, WIZ_FLAGS, 0, 0,
                    "$N tried to steal from %s.", victim->name);
                if (!EXT_IS_SET (ch->ext_plr, PLR_THIEF)) {
                    EXT_SET (ch->ext_plr, PLR_THIEF);
                    printf_to_char (ch, "*** You are now a THIEF!! ***\n\r");
                    save_char_obj (ch);
                }
            }
        }
        return;
    }

    if (!str_cmp (arg1, "coin") || !str_cmp (arg1, "coins") ||
        !str_cmp (arg1, "gold") || !str_cmp (arg1, "silver"))
    {
        int gold, silver;

        gold = victim->gold * number_range (1, ch->level) / MAX_LEVEL;
        silver = victim->silver * number_range (1, ch->level) / MAX_LEVEL;
        BAIL_IF (gold <= 0 && silver <= 0,
            "You couldn't get any coins.\n\r", ch);

        ch->gold += gold;
        ch->silver += silver;
        victim->silver -= silver;
        victim->gold -= gold;
        if (silver <= 0)
            sprintf (buf, "Bingo!  You got %d gold coins.\n\r", gold);
        else if (gold <= 0)
            sprintf (buf, "Bingo!  You got %d silver coins.\n\r", silver);
        else
            sprintf (buf, "Bingo!  You got %d silver and %d gold coins.\n\r",
                     silver, gold);

        send_to_char (buf, ch);
        player_try_skill_improve (ch, SN(STEAL), TRUE, 2);
        return;
    }

    BAIL_IF ((obj = find_obj_inventory (ch, victim, arg1)) == NULL,
        "You can't find it.\n\r", ch);
    BAIL_IF (!char_can_drop_obj (ch, obj) ||
            IS_SET (obj->extra_flags, ITEM_INVENTORY) ||
            obj->level > ch->level,
        "You can't pry it away.\n\r", ch);
    BAIL_IF (ch->carry_number + obj_get_carry_number (obj) >
                char_get_max_carry_count (ch),
        "You have your hands full.\n\r", ch);
    BAIL_IF (ch->carry_weight + obj_get_weight (obj) >
                char_get_max_carry_weight (ch),
        "You can't carry that much weight.\n\r", ch);

    obj_give_to_char (obj, ch);
    act ("You pocket $p.", ch, obj, NULL, TO_CHAR);
    player_try_skill_improve (ch, SN(STEAL), TRUE, 2);
    printf_to_char (ch, "Got it!\n\r");
}

/* equips a character */
DEFINE_DO_FUN (do_outfit) {
    OBJ_T *obj;
    int i, sn, vnum;

    BAIL_IF (ch->level > 5 || IS_NPC (ch),
        "Find it yourself!\n\r", ch);

    if ((obj = char_get_eq_by_wear_loc (ch, WEAR_LOC_LIGHT)) == NULL) {
        obj = obj_create (obj_get_index (OBJ_VNUM_SCHOOL_BANNER), 0);
        obj->cost = 0;
        obj_give_to_char (obj, ch);
        char_equip_obj (ch, obj, WEAR_LOC_LIGHT);
    }

    if ((obj = char_get_eq_by_wear_loc (ch, WEAR_LOC_BODY)) == NULL) {
        obj = obj_create (obj_get_index (OBJ_VNUM_SCHOOL_VEST), 0);
        obj->cost = 0;
        obj_give_to_char (obj, ch);
        char_equip_obj (ch, obj, WEAR_LOC_BODY);
    }

    /* do the weapon thing */
    if ((obj = char_get_eq_by_wear_loc (ch, WEAR_LOC_WIELD)) == NULL) {
        int weapon_sn;
        sn = 0;
        vnum = OBJ_VNUM_SCHOOL_SWORD; /* just in case! */

        for (i = 0; weapon_table[i].name != NULL; i++) {
            weapon_sn = weapon_table[i].skill_index;
            if (ch->pcdata->learned[sn] < ch->pcdata->learned[weapon_sn]) {
                sn   = weapon_sn;
                vnum = weapon_table[i].newbie_vnum;
            }
        }

        obj = obj_create (obj_get_index (vnum), 0);
        obj_give_to_char (obj, ch);
        char_equip_obj (ch, obj, WEAR_LOC_WIELD);
    }

    if (((obj = char_get_eq_by_wear_loc (ch, WEAR_LOC_WIELD)) == NULL
         || !IS_WEAPON_STAT (obj, WEAPON_TWO_HANDS))
        && (obj = char_get_eq_by_wear_loc (ch, WEAR_LOC_SHIELD)) == NULL)
    {
        obj = obj_create (obj_get_index (OBJ_VNUM_SCHOOL_SHIELD), 0);
        obj->cost = 0;
        obj_give_to_char (obj, ch);
        char_equip_obj (ch, obj, WEAR_LOC_SHIELD);
    }

    printf_to_char (ch, "You have been equipped by Mota.\n\r");
}

DEFINE_DO_FUN (do_play) {
    OBJ_T *juke;

    BAIL_IF ((juke = room_get_obj_with_condition (ch->in_room, ch,
            &item_can_play)) == NULL,
        "You see nothing to play.\n\r", ch);

    if (!item_play_effect (juke, ch, argument))
        printf_to_char (ch, "Nothing happens.\n\r");
}
