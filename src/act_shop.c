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

#include "act_shop.h"

#include "act_comm.h"
#include "chars.h"
#include "comm.h"
#include "find.h"
#include "globals.h"
#include "groups.h"
#include "interp.h"
#include "items.h"
#include "lookup.h"
#include "magic.h"
#include "materials.h"
#include "memory.h"
#include "mobiles.h"
#include "objs.h"
#include "players.h"
#include "rooms.h"
#include "tables.h"
#include "utils.h"

bool do_filter_get_keeper (CHAR_T *ch, CHAR_T **out_keeper) {
    CHAR_T *keeper;
    SHOP_T *shop;

    FILTER ((keeper = char_get_keeper_room (ch)) == NULL,
        "You can't do that here.\n\r", ch);
    FILTER ((shop = mobile_get_shop (keeper)) == NULL,
        "They don't have a shop.\n\r", ch);

    /* Undesirables. */
#if 0
    if (!IS_NPC(ch) && IS_SET(ch->plr, PLR_KILLER)) {
        do_function (keeper, &do_say, "Killers are not welcome!");
        sprintf (buf, "%s the KILLER is over here!\n\r", ch->name);
        do_function (keeper, &do_yell, buf);
        return NULL;
    }
    if (!IS_NPC(ch) && IS_SET(ch->plr, PLR_THIEF)) {
        do_function (keeper, &do_say, "Thieves are not welcome!");
        sprintf (buf, "%s the THIEF is over here!\n\r", ch->name);
        do_function (keeper, &do_yell, buf);
        return NULL;
    }
#endif

    /* Shop hours. */
    if (time_info.hour < shop->open_hour) {
        do_function (keeper, &do_say, "Sorry, I am closed. Come back later.");
        return TRUE;
    }
    if (time_info.hour > shop->close_hour) {
        do_function (keeper, &do_say, "Sorry, I am closed. Come back tomorrow.");
        return TRUE;
    }
    if (!char_can_see_anywhere (keeper, ch)) {
        do_function (keeper, &do_say, "I don't trade with folks I can't see.");
        return TRUE;
    }

    if (out_keeper)
        *out_keeper = keeper;
    return FALSE;
}

void do_buy_pet (CHAR_T *ch, char *argument) {
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    CHAR_T *pet;
    ROOM_INDEX_T *room_index_next;
    ROOM_INDEX_T *in_room;
    int cost, roll;

    str_smash_tilde (argument);
    if (IS_NPC (ch))
        return;
    argument = one_argument (argument, arg);

    /* hack to make new thalos pets work */
    if (ch->in_room->vnum == 9621)
        room_index_next = room_get_index (9706);
    else
        room_index_next = room_get_index (ch->in_room->vnum + 1);
    if (room_index_next == NULL) {
        bug ("do_buy: bad pet shop at vnum %d.", ch->in_room->vnum);
        send_to_char ("Sorry, you can't buy that here.\n\r", ch);
        return;
    }

    in_room = ch->in_room;
    ch->in_room = room_index_next;
    pet = find_char_same_room (ch, arg);
    ch->in_room = in_room;

    BAIL_IF (pet == NULL || !IS_PET (pet),
        "Sorry, you can't buy that here.\n\r", ch);
    BAIL_IF (ch->pet != NULL,
        "You already own a pet.\n\r", ch);

    cost = 10 * pet->level * pet->level;
    BAIL_IF ((ch->silver + 100 * ch->gold) < cost,
        "You can't afford it.\n\r", ch);
    BAIL_IF (ch->level < pet->level,
        "You're not powerful enough to master this pet.\n\r", ch);

    /* haggle */
    roll = number_percent ();
    if (roll < char_get_skill (ch, SN(HAGGLE))) {
        cost -= cost / 2 * roll / 100;
        printf_to_char (ch, "You haggle the price down to %d coins.\n\r", cost);
        player_try_skill_improve (ch, SN(HAGGLE), TRUE, 4);
    }

    char_reduce_money (ch, cost);
    pet = mobile_create (pet->mob_index);
    EXT_SET (pet->ext_mob, MOB_PET);
    SET_BIT (pet->affected_by, AFF_CHARM);
    pet->comm = COMM_NOTELL | COMM_NOSHOUT | COMM_NOCHANNELS;

    argument = one_argument (argument, arg);
    if (arg[0] != '\0') {
        sprintf (buf, "%s %s", pet->name, arg);
        str_replace_dup (&(pet->name), buf);
    }

    snprintf (buf, sizeof(buf), "%sA neck tag says 'I belong to %s'.\n\r",
        pet->description, ch->name);
    str_replace_dup (&(pet->description), buf);

    char_to_room (pet, ch->in_room);
    add_follower (pet, ch);
    pet->leader = ch;
    ch->pet = pet;
    send_to_char ("Enjoy your pet.\n\r", ch);
    act ("$n bought $N as a pet.", ch, NULL, pet, TO_NOTCHAR);
}

void do_buy_item (CHAR_T *ch, char *argument) {
    CHAR_T *keeper;
    OBJ_T *obj, *t_obj;
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    int number, count = 1;
    int cost, roll;

    if (do_filter_get_keeper (ch, &keeper))
        return;

    number = mult_argument (argument, arg);
    obj = find_obj_keeper (ch, keeper, arg);
    cost = mobile_get_obj_cost (keeper, obj, TRUE);

    BAIL_IF_ACT (number < 1 || number > 99,
        "$N tells you 'Get real!", ch, NULL, keeper);
    BAIL_IF_ACT (cost <= 0 || !char_can_see_obj (ch, obj),
        "$N tells you 'I don't sell that -- try 'list''.", ch, NULL, keeper);

    if (!IS_OBJ_STAT (obj, ITEM_INVENTORY)) {
        for (t_obj = obj->content_next;
             count < number && t_obj != NULL; t_obj = t_obj->content_next)
        {
            if (t_obj->obj_index == obj->obj_index &&
                    !str_cmp (t_obj->short_descr, obj->short_descr))
                count++;
            else
                break;
        }
        BAIL_IF_ACT (count < number,
            "$N tells you 'I don't have that many in stock.", ch, NULL, keeper);
    }

    if ((ch->silver + ch->gold * 100) < cost * number) {
        BAIL_IF_ACT (number > 1,
            "$N tells you 'You can't afford to buy that many.", ch, obj, keeper);
        BAIL_IF_ACT (TRUE,
            "$N tells you 'You can't afford to buy $p'.", ch, obj, keeper);
    }
    BAIL_IF_ACT (obj->level > ch->level,
        "$N tells you 'You can't use $p yet'.", ch, obj, keeper);
    BAIL_IF (ch->carry_number + number * obj_get_carry_number (obj) >
            char_get_max_carry_count (ch),
        "You can't carry that many items.\n\r", ch);
    BAIL_IF (ch->carry_weight + number * obj_get_weight (obj) >
            char_get_max_carry_weight (ch),
        "You can't carry that much weight.\n\r", ch);

    /* haggle */
    roll = number_percent ();
    if (!IS_OBJ_STAT (obj, ITEM_SELL_EXTRACT)
        && roll < char_get_skill (ch, SN(HAGGLE)))
    {
        cost -= obj->cost / 2 * roll / 100;
        act ("You haggle with $N.", ch, NULL, keeper, TO_CHAR);
        player_try_skill_improve (ch, SN(HAGGLE), TRUE, 4);
    }
    if (number > 1) {
        sprintf (buf, "You buy $p[%d] for %d silver.", number, cost * number);
        act (buf, ch, obj, NULL, TO_CHAR);
        sprintf (buf, "$n buys $p[%d].", number);
        act (buf, ch, obj, NULL, TO_NOTCHAR);
    }
    else {
        sprintf (buf, "You buy $p for %d silver.", cost);
        act (buf, ch, obj, NULL, TO_CHAR);
        act ("$n buys $p.", ch, obj, NULL, TO_NOTCHAR);
    }
    char_reduce_money (ch, cost * number);
    keeper->gold += cost * number / 100;
    keeper->silver += cost * number - (cost * number / 100) * 100;

    for (count = 0; count < number; count++) {
        if (IS_SET (obj->extra_flags, ITEM_INVENTORY))
            t_obj = obj_create (obj->obj_index, obj->level);
        else {
            t_obj = obj;
            obj = obj->content_next;
        }

        if (t_obj->timer > 0 && !IS_OBJ_STAT (t_obj, ITEM_HAD_TIMER))
            t_obj->timer = 0;
        REMOVE_BIT (t_obj->extra_flags, ITEM_HAD_TIMER);
        obj_give_to_char (t_obj, ch);
        if (cost < t_obj->cost)
            t_obj->cost = cost;
    }
}

DEFINE_DO_FUN (do_buy) {
    BAIL_IF (argument[0] == '\0',
        "Buy what?\n\r", ch);
    if (IS_SET (ch->in_room->room_flags, ROOM_PET_SHOP))
        do_buy_pet (ch, argument);
    else
        do_buy_item (ch, argument);
}

void do_list_pets (CHAR_T *ch, char *argument) {
    ROOM_INDEX_T *room_index_next;
    CHAR_T *pet;
    bool found;
    char *material_str;

    /* hack to make new thalos pets work */
    if (ch->in_room->vnum == 9621)
        room_index_next = room_get_index (9706);
    else
        room_index_next = room_get_index (ch->in_room->vnum + 1);
    if (room_index_next == NULL) {
        bug ("do_list: bad pet shop at vnum %d.", ch->in_room->vnum);
        send_to_char ("You can't do that here.\n\r", ch);
        return;
    }

    found = FALSE;
    for (pet = room_index_next->people_first; pet; pet = pet->room_next) {
        if (!IS_PET (pet))
            continue;
        if (!found) {
            found = TRUE;
            send_to_char ("Pets for sale:\n\r", ch);
        }
        material_str = IS_SET (ch->comm, COMM_MATERIALS)
            ? material_format_part (material_get (pet->material)) : "";
        printf_to_char (ch, "[%2d] %8d - %s%s\n\r", pet->level,
            10 * pet->level * pet->level, material_str, pet->short_descr);
    }
    if (!found)
        send_to_char ("Sorry, we're out of pets right now.\n\r", ch);
}

void do_list_items (CHAR_T *ch, char *argument) {
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    CHAR_T *keeper;
    OBJ_T *obj;
    int cost, count;
    bool found;
    char *material_str;

    if (do_filter_get_keeper (ch, &keeper))
        return;
    one_argument (argument, arg);

    found = FALSE;
    for (obj = keeper->content_first; obj; obj = obj->content_next) {
        if (obj->wear_loc != WEAR_LOC_NONE)
            continue;
        if (!char_can_see_obj (ch, obj))
            continue;
        if ((cost = mobile_get_obj_cost (keeper, obj, TRUE)) <= 0)
            continue;
        if (!(arg[0] == '\0' || str_in_namelist (arg, obj->name)))
            continue;

        if (!found) {
            found = TRUE;
            send_to_char ("[Lv Price Qty] Item\n\r", ch);
        }

        material_str = IS_SET (ch->comm, COMM_MATERIALS)
            ? material_format_part (material_get (obj->material)) : "";

        if (IS_OBJ_STAT (obj, ITEM_INVENTORY)) {
            sprintf (buf, "[%2d %5d -- ] %s%s\n\r",
                obj->level, cost, material_str, obj->short_descr);
        }
        else {
            count = 1;
            while (obj->content_next != NULL
                   && obj->obj_index == obj->content_next->obj_index
                   && !str_cmp (obj->short_descr,
                                obj->content_next->short_descr))
            {
                obj = obj->content_next;
                count++;
            }
            sprintf (buf, "[%2d %5d %2d ] %s%s\n\r",
                obj->level, cost, count, material_str, obj->short_descr);
        }
        send_to_char (buf, ch);
    }

    if (!found)
        send_to_char ("You can't buy anything here.\n\r", ch);
}

DEFINE_DO_FUN (do_list) {
    if (IS_SET (ch->in_room->room_flags, ROOM_PET_SHOP))
        do_list_pets (ch, argument);
    else
        do_list_items (ch, argument);
}

DEFINE_DO_FUN (do_sell) {
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    CHAR_T *keeper;
    OBJ_T *obj;
    int cost, roll;

    DO_REQUIRE_ARG (arg, "Sell what?\n\r");
    if (do_filter_get_keeper (ch, &keeper))
        return;

    BAIL_IF_ACT ((obj = find_obj_own_inventory (ch, arg)) == NULL,
        "$N tells you 'You don't have that item'.", ch, NULL, keeper);
    BAIL_IF (!char_can_drop_obj (ch, obj),
        "You can't let go of it.\n\r", ch);
    BAIL_IF_ACT (!char_can_see_obj (keeper, obj),
        "$N doesn't see what you are offering.", ch, NULL, keeper);
    BAIL_IF_ACT ((cost = mobile_get_obj_cost (keeper, obj, FALSE)) <= 0,
        "$N looks uninterested in $p.", ch, obj, keeper);
    BAIL_IF_ACT (cost > (keeper->silver + 100 * keeper->gold),
        "$N tells you 'I'm afraid I don't have enough wealth to buy $p.",
             ch, obj, keeper);

    act ("$n sells $p.", ch, obj, NULL, TO_NOTCHAR);

    /* haggle */
    roll = number_percent ();
    if (!IS_OBJ_STAT (obj, ITEM_SELL_EXTRACT)
        && roll < char_get_skill (ch, SN(HAGGLE)))
    {
        send_to_char ("You haggle with the shopkeeper.\n\r", ch);
        cost += obj->cost / 2 * roll / 100;
        cost = UMIN (cost, 95 * mobile_get_obj_cost (keeper, obj, TRUE) / 100);
        cost = UMIN (cost, (keeper->silver + 100 * keeper->gold));
        player_try_skill_improve (ch, SN(HAGGLE), TRUE, 4);
    }

    sprintf (buf, "You sell $p for %d silver and %d gold piece%s.",
             cost - (cost / 100) * 100, cost / 100, cost == 1 ? "" : "s");
    act (buf, ch, obj, NULL, TO_CHAR);

    ch->gold += cost / 100;
    ch->silver += cost - (cost / 100) * 100;
    char_reduce_money (keeper, cost);

    if (keeper->gold < 0)
        keeper->gold = 0;
    if (keeper->silver < 0)
        keeper->silver = 0;

    if (!item_can_sell (obj) || IS_OBJ_STAT (obj, ITEM_SELL_EXTRACT))
        obj_extract (obj);
    else {
        if (obj->timer)
            SET_BIT (obj->extra_flags, ITEM_HAD_TIMER);
        else
            obj->timer = number_range (50, 100);
        obj_give_to_keeper (obj, keeper);
    }
}

DEFINE_DO_FUN (do_value) {
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    CHAR_T *keeper;
    OBJ_T *obj;
    int cost;

    DO_REQUIRE_ARG (arg, "Value what?\n\r");
    if (do_filter_get_keeper (ch, &keeper))
        return;

    BAIL_IF_ACT ((obj = find_obj_own_inventory (ch, arg)) == NULL,
        "$N tells you 'You don't have that item'.", ch, NULL, keeper);
    BAIL_IF_ACT (!char_can_see_obj (keeper, obj),
        "$N doesn't see what you are offering.", ch, NULL, keeper);
    BAIL_IF (!char_can_drop_obj (ch, obj),
        "You can't let go of it.\n\r", ch);
    BAIL_IF_ACT ((cost = mobile_get_obj_cost (keeper, obj, FALSE)) <= 0,
        "$N looks uninterested in $p.", ch, obj, keeper);

    sprintf (buf, "$N tells you 'I'll give you %d silver and "
                  "%d gold coins for $p'.",
             cost - (cost / 100) * 100, cost / 100);
    act (buf, ch, obj, keeper, TO_CHAR);
}

DEFINE_DO_FUN (do_heal) {
    CHAR_T *mob;
    char arg[MAX_INPUT_LENGTH];
    int sn, cost;
    const HEAL_SPELL_T *spell;

    /* check for healer */
    for (mob = ch->in_room->people_first; mob; mob = mob->room_next)
        if (IS_NPC (mob) && EXT_IS_SET (mob->ext_mob, MOB_IS_HEALER))
            break;
    BAIL_IF (mob == NULL,
        "You can't do that here.\n\r", ch);

    /* if no argument is provided, display the price list */
    one_argument (argument, arg);
    if (arg[0] == '\0') {
        act ("$N says 'I offer the following spells:'", ch, NULL, mob, TO_CHAR);
        for (spell = heal_spell_table; spell->keywords != NULL; spell++) {
            char keyword[32];
            const char *kw;
            int i;

            /* get the first keyword of the available keywords. */
            for (i = 0, kw = spell->keywords;
                 i < sizeof(keyword) - 2 && *kw != '\0' && *kw != ' ';
                 i++, kw++)
            {
                keyword[i] = *kw;
            }
            keyword[i] = ':';
            keyword[i + 1] = '\0';

            /* show the spell keyword, description, and cost */
            printf_to_char (ch, "   %-9s%-21s%2d gold\n\r",
                keyword, spell->description, spell->cost_gold);
        }
        return;
    }

    /* look for a matching spell. */
    for (spell = heal_spell_table; spell->keywords != NULL; spell++)
        if (str_in_namelist (arg, spell->keywords))
            break;
    if (spell->keywords == NULL) {
        act ("$N says 'Type 'heal' for a list of spells.'", ch, NULL, mob, TO_CHAR);
        return;
    }

    /* spell found - can we afford it? */
    cost = spell->cost_gold * 100;
    if (cost > (ch->gold * 100 + ch->silver)) {
        act ("$N says 'You do not have enough gold for my services.'",
             ch, NULL, mob, TO_CHAR);
        return;
    }

    /* look up the skill. if it's not available, we have a problem! */
    sn = spell->skill_name ? skill_lookup (spell->skill_name) : -1;
    if (sn == -1) {
        act ("$N seems to have forgotten how to cast it...", ch, NULL, mob, TO_CHAR);
        return;
    }

    /* all checks passed - wait a moment and dispense with services. */
    WAIT_STATE (ch, PULSE_VIOLENCE);

    char_reduce_money (ch, cost);
    mob->gold += cost / 100;
    mob->silver += cost % 100;

    say_spell (mob, sn, class_lookup_exact ("cleric"));
    spell->spell_func (sn, mob->level, mob, ch, TARGET_CHAR, "");
}
