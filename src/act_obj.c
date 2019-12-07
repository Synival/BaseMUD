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

/*   QuickMUD - The Lazy Man's ROM - $Id: act_obj.c,v 1.2 2000/12/01 10:48:33 ring0 Exp $ */

#include <stdlib.h>

#include "interp.h"
#include "affects.h"
#include "utils.h"
#include "comm.h"
#include "db.h"
#include "skills.h"
#include "fight.h"
#include "groups.h"
#include "mob_prog.h"
#include "save.h"
#include "magic.h"
#include "update.h"
#include "act_group.h"
#include "act_comm.h"
#include "act_move.h"
#include "recycle.h"
#include "music.h"
#include "chars.h"
#include "objs.h"
#include "rooms.h"
#include "find.h"
#include "globals.h"

#include "act_obj.h"

bool do_filter_can_drop_item (CHAR_T *ch, OBJ_T *obj, bool msg) {
    FILTER (!char_can_see_obj (ch, obj),
        (!msg) ? NULL : "You can't find it.\n\r", ch);
    FILTER (obj->wear_loc != WEAR_NONE,
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
    FILTER (!obj_is_container (container),
        "That's not a container.\n\r", ch);
    FILTER_ACT (IS_SET (container->v.container.flags, CONT_CLOSED),
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

    obj_take_from_char (obj);
    obj_give_to_room (obj, ch->in_room);
    act2 ("You drop $p.",
          "$n drops $p.", ch, obj, NULL, 0, POS_RESTING);

    if (IS_OBJ_STAT (obj, ITEM_MELT_DROP)) {
        act ("$p dissolves into smoke.", ch, obj, NULL, TO_ALL);
        obj_extract (obj);
    }
}

void do_put_single_item (CHAR_T *ch, OBJ_T *obj, OBJ_T *container) {
    BAIL_IF_ACT (!char_can_drop_obj (ch, obj),
        "$p: You can't let go of it.", ch, obj, NULL);
    BAIL_IF_ACT (WEIGHT_MULT (obj) != 100,
        "$p: You have a feeling that would be a bad idea.", ch, obj, NULL);
    BAIL_IF_ACT (!obj_can_fit_in (obj, container),
        "$p: It won't fit.", ch, obj, NULL);

    if (container->index_data->vnum == OBJ_VNUM_PIT &&
        !CAN_WEAR_FLAG (obj, ITEM_TAKE))
    {
        if (obj->timer)
            SET_BIT (obj->extra_flags, ITEM_HAD_TIMER);
        else
            obj->timer = number_range (100, 200);
    }

    obj_take_from_char (obj);
    obj_give_to_obj (obj, container);

    if (container->item_type == ITEM_CONTAINER &&
            IS_SET (container->v.container.flags, CONT_PUT_ON))
        act2 ("You put $p on $P.",
              "$n puts $p on $P.", ch, obj, container, 0, POS_RESTING);
    else
        act2 ("You put $p in $P.",
              "$n puts $p in $P.", ch, obj, container, 0, POS_RESTING);
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

    /* special checks for getting only. */
    BAIL_IF (container->item_type == ITEM_CORPSE_PC &&
            !char_can_loot (ch, container),
        "You can't do that.\n\r", ch);
    BAIL_IF_ACT (container->item_type == ITEM_CONTAINER &&
                 IS_SET (container->v.container.flags, CONT_CLOSED),
        "The $p is closed.", ch, container, NULL);
    BAIL_IF (type != OBJ_SINGLE && !IS_IMMORTAL (ch) &&
             container->index_data->vnum == OBJ_VNUM_PIT,
        "Don't be so greedy!\n\r", ch);

    obj_start = (type != OBJ_SINGLE) ? container->contains
        : find_obj_container (ch, container, arg1);

    for (obj = obj_start; obj != NULL; obj = obj_next) {
        obj_next = (type == OBJ_SINGLE) ? NULL : obj->next_content;
        if (type == OBJ_ALL_OF && !is_name (arg1, obj->name))
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
    obj_start = (type != OBJ_SINGLE) ? ch->in_room->contents
        : find_obj_same_room (ch, argument);

    for (obj = obj_start; obj != NULL; obj = obj_next) {
        obj_next = (type == OBJ_SINGLE) ? NULL : obj->next_content;
        if (type == OBJ_ALL_OF && !is_name (argument, obj->name))
            continue;
        if (!char_can_see_obj (ch, obj))
            continue;
       char_take_obj (ch, obj, NULL);
        found = TRUE;
    }
    if (!found) {
        if (type == OBJ_ALL)
            send_to_char ("You see nothing here.\n\r", ch);
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
    BAIL_IF (container->item_type != ITEM_CONTAINER,
        "That's not a container.\n\r", ch);

    obj_start = (type != OBJ_SINGLE) ? ch->carrying
        : find_obj_own_inventory (ch, arg);

    for (obj = obj_start; obj != NULL; obj = obj_next) {
        obj_next = (type == OBJ_SINGLE) ? NULL : obj->next_content;
        if (type == OBJ_ALL_OF && !is_name (arg, obj->name))
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
            send_to_char ("You are not carrying anything.\n\r", ch);
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
    obj_start = (type != OBJ_SINGLE) ? ch->carrying
        : find_obj_own_inventory (ch, arg);

    for (obj = obj_start; obj != NULL; obj = obj_next) {
        obj_next = (type == OBJ_SINGLE) ? NULL : obj->next_content;
        if (type == OBJ_ALL_OF && !is_name (arg, obj->name))
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
            send_to_char ("You are not carrying anything.\n\r", ch);
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

    if (IS_NPC (victim) && IS_SET (victim->mob, MOB_IS_CHANGER)) {
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

    if (IS_NPC (victim) && victim->index_data->shop != NULL) {
        act ("$N tells you 'Sorry, you'll have to sell that.'",
             ch, NULL, victim, TO_CHAR);
        return;
    }

    obj_take_from_char (obj);
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
    AFFECT_T af;
    int percent, skill;

    /* find out what */
    BAIL_IF (argument[0] == '\0',
        "Envenom what item?\n\r", ch);
    BAIL_IF ((obj = find_obj_own_char (ch, argument)) == NULL,
        "You don't have that item.\n\r", ch);
    BAIL_IF ((skill = get_skill (ch, gsn_envenom)) < 1,
        "Are you crazy? You'd poison yourself!\n\r", ch);

    if (obj->item_type == ITEM_FOOD || obj->item_type == ITEM_DRINK_CON) {
        flag_t *pflag = NULL;
        switch (obj->item_type) {
            case ITEM_FOOD:      pflag = &(obj->v.food.poisoned);      break;
            case ITEM_DRINK_CON: pflag = &(obj->v.drink_con.poisoned); break;
        }

        BAIL_IF_ACT (IS_OBJ_STAT (obj, ITEM_BLESS) ||
                     IS_OBJ_STAT (obj, ITEM_BURN_PROOF),
            "You fail to poison $p.", ch, obj, NULL);
        if (number_percent () >= skill) {
            act ("You fail to poison $p.", ch, obj, NULL, TO_CHAR);
            if (*pflag == 0)
                check_improve (ch, gsn_envenom, FALSE, 4);
            WAIT_STATE (ch, skill_table[gsn_envenom].beats);
            return;
        }

        act ("You treat $p with deadly poison.", ch, obj, NULL, TO_CHAR);
        act ("$n treats $p with deadly poison.", ch, obj, NULL, TO_NOTCHAR);
        if (*pflag == 0) {
            *pflag = TRUE;
            check_improve (ch, gsn_envenom, TRUE, 4);
        }
        WAIT_STATE (ch, skill_table[gsn_envenom].beats);
        return;
    }

    if (obj->item_type == ITEM_WEAPON) {
        if (IS_WEAPON_STAT (obj, WEAPON_FLAMING)
            || IS_WEAPON_STAT (obj, WEAPON_FROST)
            || IS_WEAPON_STAT (obj, WEAPON_VAMPIRIC)
            || IS_WEAPON_STAT (obj, WEAPON_SHARP)
            || IS_WEAPON_STAT (obj, WEAPON_VORPAL)
            || IS_WEAPON_STAT (obj, WEAPON_SHOCKING)
            || IS_OBJ_STAT (obj, ITEM_BLESS)
            || IS_OBJ_STAT (obj, ITEM_BURN_PROOF))
        {
            act ("You can't seem to envenom $p.", ch, obj, NULL, TO_CHAR);
            return;
        }

        BAIL_IF (obj->v.weapon.attack_type < 0 ||
                 attack_table[obj->v.weapon.attack_type].damage == DAM_BASH,
            "You can only envenom edged weapons.\n\r", ch);
        BAIL_IF_ACT (IS_WEAPON_STAT (obj, WEAPON_POISON),
            "$p is already envenomed.", ch, obj, NULL);

        percent = number_percent ();
        if (percent >= skill) {
            act ("You fail to envenom $p.", ch, obj, NULL, TO_CHAR);
            check_improve (ch, gsn_envenom, FALSE, 3);
            WAIT_STATE (ch, skill_table[gsn_envenom].beats);
            return;
        }

        affect_init (&af, AFF_TO_WEAPON, gsn_poison, ch->level * percent / 100, ch->level / 2 * percent / 100, 0, 0, WEAPON_POISON);
        affect_to_obj (obj, &af);

        act ("You coat $p with venom.", ch, obj, NULL, TO_CHAR);
        act ("$n coats $p with deadly venom.", ch, obj, NULL, TO_NOTCHAR);
        check_improve (ch, gsn_envenom, TRUE, 3);
        WAIT_STATE (ch, skill_table[gsn_envenom].beats);
        return;
    }

    act ("You can't poison $p.", ch, obj, NULL, TO_CHAR);
}

DEFINE_DO_FUN (do_fill) {
    char arg[MAX_INPUT_LENGTH];
    char buf1[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    OBJ_T *obj;
    OBJ_T *fountain;
    char *liq;

    /* Find a valid container. */
    DO_REQUIRE_ARG (arg, "Fill what?\n\r");
    BAIL_IF ((obj = find_obj_own_inventory (ch, arg)) == NULL,
        "You do not have that item.\n\r", ch);
    BAIL_IF (obj->item_type != ITEM_DRINK_CON,
        "You can't fill that.\n\r", ch);

    /* Find the first fountain in the room. */
    for (fountain = ch->in_room->contents; fountain != NULL;
         fountain = fountain->next_content)
    {
        if (fountain->item_type == ITEM_FOUNTAIN)
            break;
    }
    BAIL_IF (fountain == NULL,
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

    liq = liq_table[fountain->v.fountain.liquid].name;
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
    char *liq;
    OBJ_T *out, *in;
    CHAR_T *vch = NULL;
    int amount;

    DO_REQUIRE_ARG (arg1, "Pour what into what?\n\r");
    DO_REQUIRE_ARG (arg2, "Pour it into what?\n\r");

    BAIL_IF ((out = find_obj_own_inventory (ch, arg1)) == NULL,
        "You don't have that item.\n\r", ch);
    BAIL_IF (out->item_type != ITEM_DRINK_CON,
        "That's not a drink container.\n\r", ch);
    liq = liq_table[out->v.drink_con.liquid].name;

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
        BAIL_IF ((in = char_get_eq_by_wear_loc (vch, WEAR_HOLD)) == NULL,
            "They aren't holding anything.", ch);
    }

    BAIL_IF (in->item_type != ITEM_DRINK_CON,
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

void do_change_conditions (CHAR_T *ch, int drunk, int full, int thirst,
    int hunger)
{
    if (IS_NPC (ch))
        return;

    if (drunk != 0) {
        int was_sober = IS_SOBER (ch);
        int was_drunk = IS_DRUNK (ch);
        gain_condition (ch, COND_DRUNK, drunk);

        if (!was_drunk && IS_DRUNK (ch))
            send_to_char ("You feel drunk.\n\r", ch);
        else if (was_sober && !IS_SOBER (ch))
            send_to_char ("You feel a little tispy...\n\r", ch);
    }

    if (thirst != 0) {
        int was_thirsty  = IS_THIRSTY (ch);
        int was_quenched = IS_QUENCHED (ch);
        gain_condition (ch, COND_THIRST, thirst);

        if (!was_quenched && IS_QUENCHED (ch))
            send_to_char ("Your thirst is quenched.\n\r", ch);
        else if (was_thirsty && !IS_THIRSTY (ch))
            send_to_char ("You are no longer thirsty.\n\r", ch);
    }

    if (hunger != 0) {
        int was_hungry = IS_HUNGRY (ch);
        int was_fed    = IS_FED (ch);
        gain_condition (ch, COND_HUNGER, hunger);

        if (!was_fed && IS_FED (ch))
            send_to_char ("You feel well-fed.\n\r", ch);
        else if (was_hungry && !IS_HUNGRY (ch))
            send_to_char ("You are no longer hungry.\n\r", ch);
    }

    if (full != 0) {
        int was_full = IS_FULL (ch);
        gain_condition (ch, COND_FULL, full);

        if (!was_full && IS_FULL (ch))
            send_to_char ("You are full.\n\r", ch);
    }
}

DEFINE_DO_FUN (do_drink) {
    char arg[MAX_INPUT_LENGTH];
    OBJ_T *obj;
    int amount;
    int liquid;
    const sh_int *affs;

    one_argument (argument, arg);
    if (arg[0] == '\0') {
        for (obj = ch->in_room->contents; obj; obj = obj->next_content)
            if (obj->item_type == ITEM_FOUNTAIN)
                break;
        BAIL_IF (obj == NULL,
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

    switch (obj->item_type) {
        case ITEM_FOUNTAIN:
            liquid = obj->v.fountain.liquid;
            amount = liq_table[liquid].affect[4] * 3;
            break;

        case ITEM_DRINK_CON:
            BAIL_IF (obj->v.drink_con.filled <= 0,
                "It is already empty.\n\r", ch);
            liquid = obj->v.drink_con.liquid;
            amount = UMIN (liq_table[liquid].affect[4], obj->v.drink_con.filled);
            break;

        default:
            send_to_char ("You can't drink from that.\n\r", ch);
            return;
    }

    if (liquid < 0) {
        bug ("do_drink: bad liquid number %d.", liquid);
        liquid = 0;
    }

    act2 ("You drink $T from $p.", "$n drinks $T from $p.",
         ch, obj, liq_table[liquid].name, 0, POS_RESTING);

    affs = liq_table[liquid].affect;
    do_change_conditions (ch,
        amount * affs[COND_DRUNK]  / 36, amount * affs[COND_FULL]   / 4,
        amount * affs[COND_THIRST] / 10, amount * affs[COND_HUNGER] / 2);

    if (obj->v.drink_con.poisoned != 0) {
        AFFECT_T af;

        /* The drink was poisoned! */
        send_to_char ("You choke and gag.\n\r", ch);
        act ("$n chokes and gags.", ch, NULL, NULL, TO_NOTCHAR);

        affect_init (&af, AFF_TO_AFFECTS, gsn_poison, number_fuzzy (amount),
            3 * amount, APPLY_NONE, 0, AFF_POISON);
        affect_join (ch, &af);
    }

    if (obj->v.drink_con.capacity > 0)
        obj->v.drink_con.filled -= amount;
}

DEFINE_DO_FUN (do_eat) {
    char arg[MAX_INPUT_LENGTH];
    OBJ_T *obj;

    DO_REQUIRE_ARG (arg, "Eat what?\n\r");
    BAIL_IF ((obj = find_obj_own_inventory (ch, arg)) == NULL,
        "You do not have that item.\n\r", ch);
    if (!IS_IMMORTAL (ch)) {
        BAIL_IF (obj->item_type != ITEM_FOOD && obj->item_type != ITEM_PILL,
            "That's not edible.\n\r", ch);
        BAIL_IF (IS_FULL (ch),
            "You're too full to eat more.\n\r", ch);
    }

    act2 ("You eat $p.", "$n eats $p.", ch, obj, NULL, 0, POS_RESTING);
    switch (obj->item_type) {
        case ITEM_FOOD:
            do_change_conditions (ch, 0, obj->v.food.hunger, 0,
                obj->v.food.fullness);
            if (obj->v.food.poisoned != 0) {
                AFFECT_T af;

                /* The food was poisoned! */
                send_to_char ("You choke and gag.\n\r", ch);
                act ("$n chokes and gags.", ch, NULL, NULL, TO_NOTCHAR);

                affect_init (&af, AFF_TO_AFFECTS, gsn_poison, number_fuzzy (
                    obj->v.food.hunger), 2 * obj->v.food.hunger, APPLY_NONE, 0,
                    AFF_POISON);
                affect_join (ch, &af);
            }
            break;

        case ITEM_PILL: {
            int i, level = obj->v.pill.level;
            for (i = 0; i < PILL_SKILL_MAX; i++)
                obj_cast_spell (obj->v.pill.skill[i], level, ch, ch, NULL);
            break;
        }
    }

    obj_extract (obj);
}

DEFINE_DO_FUN (do_wear) {
    char arg[MAX_INPUT_LENGTH];
    OBJ_T *obj;

    DO_REQUIRE_ARG (arg, "Wear, wield, or hold what?\n\r");
    if (!str_cmp (arg, "all")) {
        OBJ_T *obj_next;
        bool success;

        success = FALSE;
        for (obj = ch->carrying; obj != NULL; obj = obj_next) {
            obj_next = obj->next_content;
            if (obj->wear_loc == WEAR_NONE && char_can_see_obj (ch, obj))
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
    if (obj->item_type == ITEM_CORPSE_PC) {
        BAIL_IF (obj->contains,
            "Mota wouldn't like that.\n\r", ch);
    }
    BAIL_IF_ACT (!CAN_WEAR_FLAG (obj, ITEM_TAKE) ||
                  CAN_WEAR_FLAG (obj, ITEM_NO_SAC),
        "$p is not an acceptable sacrifice.", ch, obj, NULL);

    if (obj->in_room != NULL)
        for (gch = obj->in_room->people; gch; gch = gch->next_in_room)
            BAIL_IF_ACT (gch->on == obj,
                "$N appears to be using $p.", ch, obj, gch);

    silver = UMAX (1, obj->level * 3);
    if (obj->item_type != ITEM_CORPSE_NPC && obj->item_type != ITEM_CORPSE_PC)
        silver = UMIN (silver, obj->cost);

#ifdef BASEMUD_NO_WORTHLESS_SACRIFICES
    BAIL_IF (silver == 0,
        "Mota ignores your paltry offering.\n\r", ch);
#endif

    if (silver == 1)
        send_to_char ("Mota gives you one silver coin for your sacrifice.\n\r", ch);
    else {
        printf_to_char (ch, "Mota gives you %d silver coins for your sacrifice.\n\r",
            silver);
    }

    ch->silver += silver;
    if (IS_SET (ch->plr, PLR_AUTOSPLIT)) { /* AUTOSPLIT code */
        members = 0;
        for (gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room)
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
    int i, level;

    DO_REQUIRE_ARG (arg, "Quaff what?\n\r");
    BAIL_IF ((obj = find_obj_own_inventory (ch, arg)) == NULL,
        "You do not have that potion.\n\r", ch);
    BAIL_IF (obj->item_type != ITEM_POTION,
        "You can quaff only potions.\n\r", ch);
    BAIL_IF (ch->level < obj->level,
        "This liquid is too powerful for you to drink.\n\r", ch);

    act2 ("You quaff $p.", "$n quaffs $p.",
        ch, obj, NULL, 0, POS_RESTING);

    level = obj->v.potion.level;
    for (i = 0; i < POTION_SKILL_MAX; i++)
        obj_cast_spell (obj->v.potion.skill[i], level, ch, ch, NULL);

    obj_extract (obj);
}

DEFINE_DO_FUN (do_recite) {
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    CHAR_T *victim;
    OBJ_T *scroll;
    OBJ_T *obj;

    DO_REQUIRE_ARG (arg1, "Recite what?\n\r");
    argument = one_argument (argument, arg2);

    BAIL_IF ((scroll = find_obj_own_inventory (ch, arg1)) == NULL,
        "You do not have that scroll.\n\r", ch);
    BAIL_IF (scroll->item_type != ITEM_SCROLL,
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

    if (number_percent () >= 20 + get_skill (ch, gsn_scrolls) * 4 / 5) {
        send_to_char ("You mispronounce a syllable.\n\r", ch);
        check_improve (ch, gsn_scrolls, FALSE, 2);
    }
    else {
        int i, level = scroll->v.scroll.level;
        for (i = 0; i < SCROLL_SKILL_MAX; i++)
            obj_cast_spell (scroll->v.scroll.skill[i], level, ch, victim, obj);
        check_improve (ch, gsn_scrolls, TRUE, 2);
    }

    obj_extract (scroll);
}

DEFINE_DO_FUN (do_brandish) {
    CHAR_T *vch;
    CHAR_T *vch_next;
    OBJ_T *staff;
    int sn;

    BAIL_IF ((staff = char_get_eq_by_wear_loc (ch, WEAR_HOLD)) == NULL,
        "You hold nothing in your hand.\n\r", ch);
    BAIL_IF (staff->item_type != ITEM_STAFF,
        "You can brandish only with a staff.\n\r", ch);

    BAIL_IF_BUG ((sn = staff->v.staff.skill) < 0 ||
                  sn >= SKILL_MAX || skill_table[sn].spell_fun == 0,
        "do_brandish: bad sn %d.", sn);

    WAIT_STATE (ch, 2 * PULSE_VIOLENCE);
    if (staff->v.staff.charges > 0) {
        act2 ("You brandish $p.", "$n brandishes $p.",
            ch, staff, NULL, 0, POS_RESTING);

        if (ch->level < staff->level ||
            number_percent () >= 20 + get_skill (ch, gsn_staves) * 4 / 5)
        {
            act2 ("You fail to invoke $p.", "...and nothing happens.",
                ch, staff, NULL, 0, POS_RESTING);
            check_improve (ch, gsn_staves, FALSE, 2);
        }
        else {
            for (vch = ch->in_room->people; vch; vch = vch_next) {
                vch_next = vch->next_in_room;
                switch (skill_table[sn].target) {
                    case TAR_IGNORE:
                        if (vch != ch)
                            continue;
                        break;

                    case TAR_CHAR_OFFENSIVE:
                        if (IS_NPC (ch) ? IS_NPC (vch) : !IS_NPC (vch))
                            continue;
                        break;

                    case TAR_CHAR_DEFENSIVE:
                        if (IS_NPC (ch) ? !IS_NPC (vch) : IS_NPC (vch))
                            continue;
                        break;

                    case TAR_CHAR_SELF:
                        if (vch != ch)
                            continue;
                        break;

                    default:
                        bug ("do_brandish: bad target for sn %d.", sn);
                        return;
                }
                obj_cast_spell (staff->v.staff.skill, staff->v.staff.level,
                    ch, vch, NULL);
                check_improve (ch, gsn_staves, TRUE, 2);
            }
        }
    }
    if (--staff->v.staff.charges <= 0) {
        act ("Your $p blazes bright and is gone.", ch, staff, NULL, TO_CHAR);
        act ("$n's $p blazes bright and is gone.", ch, staff, NULL, TO_NOTCHAR);
        obj_extract (staff);
    }
}

DEFINE_DO_FUN (do_zap) {
    char arg[MAX_INPUT_LENGTH];
    CHAR_T *victim;
    OBJ_T *wand;
    OBJ_T *obj;

    one_argument (argument, arg);
    BAIL_IF (arg[0] == '\0' && ch->fighting == NULL,
        "Zap whom or what?\n\r", ch);
    BAIL_IF ((wand = char_get_eq_by_wear_loc (ch, WEAR_HOLD)) == NULL,
        "You hold nothing in your hand.\n\r", ch);
    BAIL_IF (wand->item_type != ITEM_WAND,
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
    if (wand->v.wand.charges > 0) {
        if (victim != NULL) {
            act3 ("You zap $N with $p.",
                  "$n zaps you with $p.",
                  "$n zaps $N with $p.",  ch, wand, victim, 0, POS_RESTING);
        }
        else {
            act2 ("You zap $P with $p.",
                  "$n zaps $P with $p.", ch, wand, obj, 0, POS_RESTING);
        }
        if (ch->level < wand->level
            || number_percent () >= 20 + get_skill (ch, gsn_wands) * 4 / 5)
        {
            act ("Your efforts with $p produce only smoke and sparks.",
                 ch, wand, NULL, TO_CHAR);
            act ("$n's efforts with $p produce only smoke and sparks.",
                 ch, wand, NULL, TO_NOTCHAR);
            check_improve (ch, gsn_wands, FALSE, 2);
        }
        else {
            obj_cast_spell (wand->v.wand.skill, wand->v.wand.level,
                ch, victim, obj);
            check_improve (ch, gsn_wands, TRUE, 2);
        }
    }
    if (--wand->v.wand.charges <= 0) {
        act2 ("Your $p explodes into fragments.",
              "$n's $p explodes into fragments.",
            ch, wand, NULL, 0, POS_RESTING);
        obj_extract (wand);
    }
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

    WAIT_STATE (ch, skill_table[gsn_steal].beats);
    percent = number_percent ();

    if (!IS_AWAKE (victim))
        percent -= 10;
    else if (!char_can_see_anywhere (victim, ch))
        percent += 25;
    else
        percent += 50;

    if (((ch->level + 7 < victim->level || ch->level - 7 > victim->level)
         && !IS_NPC (victim) && !IS_NPC (ch))
        || (!IS_NPC (ch) && percent > get_skill (ch, gsn_steal))
        || (!IS_NPC (ch) && !char_has_clan (ch)))
    {
        /* Failure. */
        send_to_char ("Oops.\n\r", ch);
        affect_strip (ch, gsn_sneak);
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
                check_improve (ch, gsn_steal, FALSE, 2);
                multi_hit (victim, ch, TYPE_UNDEFINED);
            }
            else {
                wiznetf (ch, NULL, WIZ_FLAGS, 0, 0,
                    "$N tried to steal from %s.", victim->name);
                if (!IS_SET (ch->plr, PLR_THIEF)) {
                    SET_BIT (ch->plr, PLR_THIEF);
                    send_to_char ("*** You are now a THIEF!! ***\n\r", ch);
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
        check_improve (ch, gsn_steal, TRUE, 2);
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

    obj_take_from_char (obj);
    obj_give_to_char (obj, ch);
    act ("You pocket $p.", ch, obj, NULL, TO_CHAR);
    check_improve (ch, gsn_steal, TRUE, 2);
    send_to_char ("Got it!\n\r", ch);
}

/* equips a character */
DEFINE_DO_FUN (do_outfit) {
    OBJ_T *obj;
    int i, sn, vnum;

    BAIL_IF (ch->level > 5 || IS_NPC (ch),
        "Find it yourself!\n\r", ch);

    if ((obj = char_get_eq_by_wear_loc (ch, WEAR_LIGHT)) == NULL) {
        obj = obj_create (get_obj_index (OBJ_VNUM_SCHOOL_BANNER), 0);
        obj->cost = 0;
        obj_give_to_char (obj, ch);
        char_equip_obj (ch, obj, WEAR_LIGHT);
    }

    if ((obj = char_get_eq_by_wear_loc (ch, WEAR_BODY)) == NULL) {
        obj = obj_create (get_obj_index (OBJ_VNUM_SCHOOL_VEST), 0);
        obj->cost = 0;
        obj_give_to_char (obj, ch);
        char_equip_obj (ch, obj, WEAR_BODY);
    }

    /* do the weapon thing */
    if ((obj = char_get_eq_by_wear_loc (ch, WEAR_WIELD)) == NULL) {
        sn = 0;
        vnum = OBJ_VNUM_SCHOOL_SWORD; /* just in case! */

        for (i = 0; weapon_table[i].name != NULL; i++) {
            if (ch->pcdata->learned[sn] <
                ch->pcdata->learned[*weapon_table[i].gsn])
            {
                sn = *weapon_table[i].gsn;
                vnum = weapon_table[i].newbie_vnum;
            }
        }

        obj = obj_create (get_obj_index (vnum), 0);
        obj_give_to_char (obj, ch);
        char_equip_obj (ch, obj, WEAR_WIELD);
    }

    if (((obj = char_get_eq_by_wear_loc (ch, WEAR_WIELD)) == NULL
         || !IS_WEAPON_STAT (obj, WEAPON_TWO_HANDS))
        && (obj = char_get_eq_by_wear_loc (ch, WEAR_SHIELD)) == NULL)
    {
        obj = obj_create (get_obj_index (OBJ_VNUM_SCHOOL_SHIELD), 0);
        obj->cost = 0;
        obj_give_to_char (obj, ch);
        char_equip_obj (ch, obj, WEAR_SHIELD);
    }

    send_to_char ("You have been equipped by Mota.\n\r", ch);
}

DEFINE_DO_FUN (do_play) {
    OBJ_T *juke;
    char *str, arg[MAX_INPUT_LENGTH];
    int song, i;
    bool global = FALSE;

    for (juke = ch->in_room->contents; juke != NULL; juke = juke->next_content)
        if (juke->item_type == ITEM_JUKEBOX && char_can_see_obj (ch, juke))
            break;
    BAIL_IF (juke == NULL,
        "You see nothing to play.\n\r", ch);

    str = one_argument (argument, arg);
    BAIL_IF (argument[0] == '\0',
        "Play what?\n\r", ch);

    if (!str_cmp (arg, "list")) {
        BUFFER_T *buffer;
        char buf[MAX_STRING_LENGTH];
        int col = 0;
        bool artist = FALSE, match = FALSE;

        buffer = buf_new ();
        argument = str;
        argument = one_argument (argument, arg);

        if (!str_cmp (arg, "artist"))
            artist = TRUE;
        if (argument[0] != '\0')
            match = TRUE;

        sprintf (buf, "%s has the following songs available:\n\r",
                 juke->short_descr);
        add_buf (buffer, capitalize (buf));

        for (i = 0; i < MAX_SONGS; i++) {
            if (song_table[i].name == NULL)
                break;

            if (artist && (!match
                           || !str_prefix (argument, song_table[i].group)))
                sprintf (buf, "%-39s %-39s\n\r",
                         song_table[i].group, song_table[i].name);
            else if (!artist && (!match
                                 || !str_prefix (argument,
                                                 song_table[i].name)))
                sprintf (buf, "%-35s ", song_table[i].name);
            else
                continue;

            add_buf (buffer, buf);
            if (!artist && ++col % 2 == 0)
                add_buf (buffer, "\n\r");
        }
        if (!artist && col % 2 != 0)
            add_buf (buffer, "\n\r");

        page_to_char (buf_string (buffer), ch);
        buf_free (buffer);
        return;
    }

    if (!str_cmp (arg, "loud")) {
        argument = str;
        global = TRUE;
    }
    BAIL_IF (argument[0] == '\0',
        "Play what?\n\r", ch);
    BAIL_IF ((global && channel_songs[MAX_SONG_GLOBAL] > -1) ||
             (!global && juke->v.value[4] > -1),
        "The jukebox is full up right now.\n\r", ch);

    for (song = 0; song < MAX_SONGS && song_table[song].name != NULL; song++)
        if (!str_prefix (argument, song_table[song].name))
            break;
    BAIL_IF (song >= MAX_SONGS,
        "That song isn't available.\n\r", ch);

    send_to_char ("Coming right up.\n\r", ch);
    if (global) {
        for (i = 1; i <= MAX_SONG_GLOBAL; i++) {
            if (channel_songs[i] >= 0)
                continue;
            if (i == 1)
                channel_songs[0] = -1;
            channel_songs[i] = song;
            return;
        }
    }
    else {
        for (i = 1; i < 5; i++) {
            if (juke->v.value[i] >= 0)
                continue;
            if (i == 1)
                juke->v.value[0] = -1;
            juke->v.value[i] = song;
            return;
        }
    }
}
