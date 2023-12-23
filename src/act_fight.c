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

#include "act_fight.h"

#include "act_comm.h"
#include "affects.h"
#include "chars.h"
#include "comm.h"
#include "fight.h"
#include "find.h"
#include "groups.h"
#include "interp.h"
#include "lookup.h"
#include "mob_prog.h"
#include "players.h"
#include "tables.h"
#include "utils.h"

#include <stdio.h>
#include <stdlib.h>

bool do_fight_filter_skill_target (CHAR_T *ch, const char *argument,
    int sn, flag_t npc_flag, const char *cant_msg, const char *self_msg,
    int *out_chance, CHAR_T **out_victim)
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_T *victim;
    int chance;

    /* If a skill is available, make sure we can use it. */
    if (sn >= 0) {
        chance = char_get_skill (ch, sn);
        FILTER (chance == 0,
            cant_msg, ch);
    }
    else {
        chance = 100;
    }

    FILTER (IS_NPC (ch) && npc_flag != 0 && !IS_SET (ch->off_flags, npc_flag),
        cant_msg, ch);
    FILTER (sn >= 0 && !IS_NPC (ch) &&
            ch->level < skill_table[sn].classes[ch->class].level,
        cant_msg, ch);

    one_argument (argument, arg);
    if (arg[0] == '\0') {
        FILTER ((victim = ch->fighting) == NULL,
            "But you aren't fighting anyone!\n\r", ch);
    }
    else {
        FILTER ((victim = find_char_same_room (ch, arg)) == NULL,
            "They aren't here.\n\r", ch);
    }

    FILTER (self_msg != NULL && victim == ch,
        self_msg, ch);
    if (do_filter_can_attack (ch, victim))
        return TRUE;
    FILTER (IS_NPC (victim) && victim->fighting != NULL &&
             !is_same_group (ch, victim->fighting),
        "Kill stealing is not permitted.\n\r", ch);
    FILTER_ACT (IS_AFFECTED (ch, AFF_CHARM) && ch->master == victim,
        "But $N is your friend!", ch, NULL, victim);

    if (out_chance)
        *out_chance = chance;
    if (out_victim)
        *out_victim = victim;

    return FALSE;
}

DEFINE_DO_FUN (do_berserk) {
    AFFECT_T af;
    int chance, hp_percent;

    if ((chance = char_get_skill (ch, SN(BERSERK))) == 0      ||
        (IS_NPC (ch) && !IS_SET (ch->off_flags, OFF_BERSERK)) ||
        (!IS_NPC (ch) && ch->level <
            skill_table[SN(BERSERK)].classes[ch->class].level))
    {
        send_to_char ("You turn red in the face, but nothing happens.\n\r", ch);
        return;
    }

    BAIL_IF (IS_AFFECTED (ch, AFF_BERSERK)             ||
             affect_is_char_affected (ch, SN(BERSERK)) ||
             affect_is_char_affected (ch, SN(FRENZY)),
        "You get a little madder.\n\r", ch);
    BAIL_IF (IS_AFFECTED (ch, AFF_CALM),
        "You're feeling to mellow to berserk.\n\r", ch);
    BAIL_IF (ch->mana < 50,
        "You can't get up enough energy.\n\r", ch);

    /* modifiers */

    /* fighting */
    if (ch->position == POS_FIGHTING)
        chance += 10;

    /* damage -- below 50% of hp helps, above hurts */
    hp_percent = 100 * ch->hit / ch->max_hit;
    chance += 25 - hp_percent / 2;


    if (number_percent () >= chance) {
        WAIT_STATE (ch, 3 * PULSE_VIOLENCE);
        ch->mana -= 25;
        ch->move /= 2;

        send_to_char ("Your pulse speeds up, but nothing happens.\n\r", ch);
        player_try_skill_improve (ch, SN(BERSERK), FALSE, 2);
        return;
    }

    WAIT_STATE (ch, PULSE_VIOLENCE);
    ch->mana -= 50;
    ch->move /= 2;

    /* heal a little damage */
    ch->hit += ch->level * 2;
    ch->hit = UMIN (ch->hit, ch->max_hit);

    send_to_char ("Your pulse races as you are consumed by rage!\n\r", ch);
    act ("$n gets a wild look in $s eyes.", ch, NULL, NULL, TO_NOTCHAR);
    player_try_skill_improve (ch, SN(BERSERK), TRUE, 2);

    affect_init (&af, AFF_TO_AFFECTS, SN(BERSERK), ch->level, number_fuzzy (ch->level / 8), 0, UMAX (1, ch->level / 5), AFF_BERSERK);

    af.apply = APPLY_HITROLL;
    affect_copy_to_char (&af, ch);

    af.apply = APPLY_DAMROLL;
    affect_copy_to_char (&af, ch);

    af.modifier = UMAX (10, 10 * (ch->level / 5));
    af.apply = APPLY_AC;
    affect_copy_to_char (&af, ch);
}

DEFINE_DO_FUN (do_bash) {
    CHAR_T *victim;
    int chance;

    if (do_fight_filter_skill_target (ch, argument, SN(BASH), OFF_BASH,
            "Bashing? What's that?\n\r",
            "You try to bash your brains out, but fail.\n\r",
            &chance, &victim))
        return;

    BAIL_IF_ACT (victim->position < POS_FIGHTING,
        "You'll have to let $M get back up first.", ch, NULL, victim);
    BAIL_IF (ch->position < POS_FIGHTING,
        "It's hard to bash when you're not standing up!\n\r", ch);

    /* modifiers */

    /* size and weight */
    chance += ch->carry_weight / 250;
    chance -= victim->carry_weight / 200;

    if (ch->size < victim->size)
        chance += (ch->size - victim->size) * 15;
    else
        chance += (ch->size - victim->size) * 10;

    /* stats */
    chance += char_get_curr_stat (ch, STAT_STR);
    chance -= (char_get_curr_stat (victim, STAT_DEX) * 4) / 3;
    chance -= GET_AC (victim, AC_BASH) / 25;

    /* speed */
    if (IS_SET (ch->off_flags, OFF_FAST) || IS_AFFECTED (ch, AFF_HASTE))
        chance += 10;
    if (IS_SET (victim->off_flags, OFF_FAST) || IS_AFFECTED (victim, AFF_HASTE))
        chance -= 30;

    /* level */
    chance += (ch->level - victim->level);
    if (!IS_NPC (victim) && chance < char_get_skill (victim, SN(DODGE)))
        chance -= 3 * (char_get_skill (victim, SN(DODGE)) - chance);

    /* now the attack */
    if (number_percent () < chance) {
        act3 ("{5You slam into $N, and send $M flying!{x",
              "{5$n sends you sprawling with a powerful bash!{x",
              "{5$n sends $N sprawling with a powerful bash.{x",
            ch, NULL, victim, 0, POS_RESTING);
        player_try_skill_improve (ch, SN(BASH), TRUE, 1);

        DAZE_STATE (victim, PULSE_VIOLENCE * 5 / 2);
        WAIT_STATE (ch, skill_table[SN(BASH)].beats);
        victim->position = POS_SITTING;
        damage_quiet (ch, victim, number_range (2, 2 + 2*ch->size + chance/20),
            SN(BASH), DAM_BASH);
    }
    else {
        act3 ("{5You fall flat on your face!{x",
              "{5You evade $n's bash, causing $m to fall flat on $s face.{x",
              "{5$n falls flat on $s face.{x",
            ch, NULL, victim, 0, POS_RESTING);
        player_try_skill_improve (ch, SN(BASH), FALSE, 1);

        WAIT_STATE (ch, skill_table[SN(BASH)].beats * 3 / 2);
        ch->position = POS_SITTING;
        damage_quiet (ch, victim, 0, SN(BASH), DAM_BASH);
    }
    check_killer (ch, victim);
}

DEFINE_DO_FUN (do_dirt) {
    CHAR_T *victim;
    int chance;

    if (do_fight_filter_skill_target (ch, argument, SN(DIRT), OFF_KICK_DIRT,
            "You get your feet dirty.\n\r",
            "Very funny.\n\r",
            &chance, &victim))
        return;

    BAIL_IF_ACT (IS_AFFECTED (victim, AFF_BLIND),
        "$E's already been blinded.", ch, NULL, victim);
    BAIL_IF (ch->position < POS_FIGHTING,
        "That would be tricky while you're off your feet.\n\r", ch);

    /* modifiers */

    /* dexterity */
    chance += char_get_curr_stat (ch, STAT_DEX);
    chance -= 2 * char_get_curr_stat (victim, STAT_DEX);

    /* speed  */
    if (IS_SET (ch->off_flags, OFF_FAST) || IS_AFFECTED (ch, AFF_HASTE))
        chance += 10;
    if (IS_SET (victim->off_flags, OFF_FAST)
        || IS_AFFECTED (victim, AFF_HASTE))
        chance -= 25;

    /* level */
    chance += (ch->level - victim->level) * 2;

    /* sloppy hack to prevent false zeroes */
    if (chance % 5 == 0)
        chance += 1;

    /* terrain */
    switch (ch->in_room->sector_type) {
        case (SECT_INSIDE):       chance -= 20; break;
        case (SECT_CITY):         chance -= 10; break;
        case (SECT_MOUNTAIN):     chance -= 10; break;
        case (SECT_FIELD):        chance +=  5; break;
        case (SECT_FOREST):       chance +=  0; break;
        case (SECT_HILLS):        chance +=  0; break;
        case (SECT_DESERT):       chance += 10; break;

        /* can't kick dirt in these areas: */
        case (SECT_WATER_SWIM):   chance  =  0; break;
        case (SECT_WATER_NOSWIM): chance  =  0; break;
        case (SECT_AIR):          chance  =  0; break;
    }

    BAIL_IF (chance == 0,
        "There isn't any dirt to kick.\n\r", ch);

    /* now the attack */
    if (number_percent () < chance) {
        AFFECT_T af;

        act3 ("{You kick dirt in $N's eyes!{x",
              "{5$n kicks dirt in your eyes!{x",
              "{5$n kicks dirt in $N's eyes!{x",
            ch, NULL, victim, 0, POS_RESTING);

        damage_quiet (ch, victim, number_range (2, 5), SN(DIRT), DAM_NONE);
        send_to_char ("{5You can't see a thing!{x\n\r", victim);

        player_try_skill_improve (ch, SN(DIRT), TRUE, 2);
        WAIT_STATE (ch, skill_table[SN(DIRT)].beats);

        affect_init (&af, AFF_TO_AFFECTS, SN(DIRT), ch->level, 0, APPLY_HITROLL, -4, AFF_BLIND);
        affect_copy_to_char (&af, victim);
    }
    else {
        damage_visible (ch, victim, 0, SN(DIRT), DAM_NONE, NULL);
        player_try_skill_improve (ch, SN(DIRT), FALSE, 2);
        WAIT_STATE (ch, skill_table[SN(DIRT)].beats);
    }
    check_killer (ch, victim);
}

DEFINE_DO_FUN (do_trip) {
    CHAR_T *victim;
    int chance;

    if (do_fight_filter_skill_target (ch, argument, SN(TRIP), OFF_TRIP,
            "Tripping? What's that?\n\r", NULL, &chance, &victim))
        return;

    BAIL_IF_ACT ((victim->parts & (PART_FEET | PART_LEGS)) == 0,
        "$N doesn't have any legs to trip.", ch, NULL, victim);
    BAIL_IF_ACT (IS_AFFECTED (victim, AFF_FLYING),
        "$S feet aren't on the ground.", ch, NULL, victim);
    BAIL_IF_ACT (victim->position < POS_FIGHTING,
        "$N is already down.", ch, NULL, victim);
    BAIL_IF (ch->position < POS_FIGHTING,
        "You'll need to stand up for that.\n\r", ch);
    BAIL_IF_ACT (IS_AFFECTED (ch, AFF_CHARM) && ch->master == victim,
        "$N is your beloved master.", ch, NULL, victim);

    if (victim == ch) {
        send_to_char ("{5You fall flat on your face!{x\n\r", ch);
        WAIT_STATE (ch, 2 * skill_table[SN(TRIP)].beats);
        act ("{5$n trips over $s own feet!{x", ch, NULL, NULL, TO_NOTCHAR);
        return;
    }

    /* modifiers */

    /* size */
    if (ch->size < victim->size)
        chance += (ch->size - victim->size) * 10; /* bigger = harder to trip */

    /* dex */
    chance += char_get_curr_stat (ch, STAT_DEX);
    chance -= char_get_curr_stat (victim, STAT_DEX) * 3 / 2;

    /* speed */
    if (IS_SET (ch->off_flags, OFF_FAST) || IS_AFFECTED (ch, AFF_HASTE))
        chance += 10;
    if (IS_SET (victim->off_flags, OFF_FAST) || IS_AFFECTED (victim, AFF_HASTE))
        chance -= 20;

    /* level */
    chance += (ch->level - victim->level) * 2;

    /* now the attack */
    if (number_percent () < chance) {
        act3 ("{5You trip $N and $N goes down!{x",
              "{5$n trips you and you go down!{x",
              "{5$n trips $N, sending $M to the ground.{x",
            ch, NULL, victim, 0, POS_RESTING);
        player_try_skill_improve (ch, SN(TRIP), TRUE, 1);

        DAZE_STATE (victim, PULSE_VIOLENCE * 3 / 2);
        WAIT_STATE (ch, skill_table[SN(TRIP)].beats);
        victim->position = POS_SITTING;
        damage_visible (ch, victim, number_range (2, 2 + 2 * victim->size),
            SN(TRIP), DAM_BASH, NULL);
    }
    else {
        player_try_skill_improve (ch, SN(TRIP), FALSE, 1);
        WAIT_STATE (ch, skill_table[SN(TRIP)].beats * 2 / 3);
        damage_visible (ch, victim, 0, SN(TRIP), DAM_BASH, NULL);
    }
    check_killer (ch, victim);
}

DEFINE_DO_FUN (do_kick) {
    CHAR_T *victim;
    int chance;

    if (do_fight_filter_skill_target (ch, argument, SN(KICK), OFF_KICK,
            "You better leave the martial arts to fighters.\n\r",
            "You're having trouble connecting your foot to your torso.\n\r",
            &chance, &victim))
        return;
    BAIL_IF (ch->position < POS_FIGHTING,
        "It's hard to kick when you're on the ground.\n\r", ch);

    WAIT_STATE (ch, skill_table[SN(KICK)].beats);
    if (chance > number_percent ()) {
        damage_visible (ch, victim, number_range (1, ch->level), SN(KICK),
            DAM_BASH, NULL);
        player_try_skill_improve (ch, SN(KICK), TRUE, 1);
    }
    else {
        damage_visible (ch, victim, 0, SN(KICK), DAM_BASH, NULL);
        player_try_skill_improve (ch, SN(KICK), FALSE, 1);
    }
    check_killer (ch, victim);
}

DEFINE_DO_FUN (do_kill) {
    char arg[MAX_INPUT_LENGTH];
    CHAR_T *victim;

    DO_REQUIRE_ARG (arg, "Kill whom?\n\r");
    BAIL_IF ((victim = find_char_same_room (ch, arg)) == NULL,
        "They aren't here.\n\r", ch);

/*  Allow player killing
    if ( !IS_NPC(victim) ) {
        if ( !IS_SET(victim->plr, PLR_KILLER)
        &&   !IS_SET(victim->plr, PLR_THIEF) )
        {
            send_to_char( "You must MURDER a player.\n\r", ch );
            return;
        }
    }
*/

    if (victim == ch) {
        send_to_char ("You hit yourself.  Ouch!\n\r", ch);
        multi_hit (ch, ch, ATTACK_DEFAULT);
        return;
    }
    if (do_filter_can_attack (ch, victim))
        return;
    BAIL_IF (victim->fighting != NULL && !is_same_group (ch, victim->fighting),
        "Kill stealing is not permitted.\n\r", ch);
    BAIL_IF_ACT (IS_AFFECTED (ch, AFF_CHARM) && ch->master == victim,
        "$N is your beloved master.", ch, NULL, victim);
    BAIL_IF (ch->position == POS_FIGHTING,
        "You do the best you can!\n\r", ch);

    WAIT_STATE (ch, 1 * PULSE_VIOLENCE);
    check_killer (ch, victim);
    multi_hit (ch, victim, ATTACK_DEFAULT);
}

DEFINE_DO_FUN (do_murde) {
    send_to_char ("If you want to MURDER, spell it out.\n\r", ch);
}

DEFINE_DO_FUN (do_murder) {
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    CHAR_T *victim;

    DO_REQUIRE_ARG (arg, "Murder whom?\n\r");

    BAIL_IF (IS_AFFECTED (ch, AFF_CHARM) || IS_PET (ch),
        "Only out of your own free will.\n\r", ch);
    BAIL_IF ((victim = find_char_same_room (ch, arg)) == NULL,
        "They aren't here.\n\r", ch);
    BAIL_IF (victim == ch,
        "Suicide is a mortal sin.\n\r", ch);
    if (do_filter_can_attack (ch, victim))
        return;
    BAIL_IF (IS_NPC (victim) && victim->fighting != NULL &&
             !is_same_group (ch, victim->fighting),
        "Kill stealing is not permitted.\n\r", ch);
    BAIL_IF_ACT (IS_AFFECTED (ch, AFF_CHARM) && ch->master == victim,
        "$N is your beloved master.", ch, NULL, victim);
    BAIL_IF (ch->position == POS_FIGHTING,
        "You do the best you can!\n\r", ch);

    WAIT_STATE (ch, 1 * PULSE_VIOLENCE);
    if (IS_NPC (ch))
        sprintf (buf, "Help! I am being attacked by %s!", ch->short_descr);
    else
        sprintf (buf, "Help! I am being attacked by %s!", ch->name);

    do_function (victim, &do_yell, buf);
    check_killer (ch, victim);
    multi_hit (ch, victim, ATTACK_DEFAULT);
}

DEFINE_DO_FUN (do_backstab) {
    char arg[MAX_INPUT_LENGTH];
    CHAR_T *victim;
    OBJ_T *obj;

    DO_REQUIRE_ARG (arg, "Backstab whom?\n\r");

    BAIL_IF (ch->fighting != NULL,
        "You're facing the wrong end.\n\r", ch);
    BAIL_IF ((victim = find_char_same_room (ch, arg)) == NULL,
        "They aren't here.\n\r", ch);
    BAIL_IF (victim == ch,
        "How can you sneak up on yourself?\n\r", ch);
    if (do_filter_can_attack (ch, victim))
        return;
    BAIL_IF (IS_NPC (victim) && victim->fighting != NULL &&
             !is_same_group (ch, victim->fighting),
        "Kill stealing is not permitted.\n\r", ch);
    BAIL_IF (ch->position < POS_FIGHTING,
        "You can't even reach their back on the ground!\n\r", ch);
    BAIL_IF ((obj = char_get_eq_by_wear_loc (ch, WEAR_LOC_WIELD)) == NULL,
        "You need to wield a weapon to backstab.\n\r", ch);
    BAIL_IF_ACT (victim->hit < victim->max_hit / 3,
        "$N is hurt and suspicious ... you can't sneak up.", ch, NULL, victim);

    check_killer (ch, victim);
    WAIT_STATE (ch, skill_table[SN(BACKSTAB)].beats);
    if (number_percent () < char_get_skill (ch, SN(BACKSTAB))
        || (char_get_skill (ch, SN(BACKSTAB)) >= 2 && !IS_AWAKE (victim)))
    {
        player_try_skill_improve (ch, SN(BACKSTAB), TRUE, 1);
        multi_hit (ch, victim, SN(BACKSTAB));
    }
    else {
        player_try_skill_improve (ch, SN(BACKSTAB), FALSE, 1);
        damage_visible (ch, victim, 0, SN(BACKSTAB), DAM_NONE, NULL);
    }
}

DEFINE_DO_FUN (do_flee) {
    ROOM_INDEX_T *was_in;
    ROOM_INDEX_T *now_in;
    CHAR_T *victim;
    int attempt;

    if ((victim = ch->fighting) == NULL) {
        if (ch->position == POS_FIGHTING)
            ch->position = POS_STANDING;
        send_to_char ("You aren't fighting anyone.\n\r", ch);
        return;
    }
    BAIL_IF (ch->daze > 0,
        "You're too dazed to get your bearings!\n\r", ch);

    was_in = ch->in_room;
    for (attempt = 0; attempt < 6; attempt++) {
        EXIT_T *pexit;
        int door;

        door = number_door ();
        if ((pexit = was_in->exit[door]) == NULL)
            continue;
        if (pexit->to_room == NULL)
            continue;
        if (IS_SET (pexit->exit_flags, EX_CLOSED))
            continue;
        if (number_range (0, ch->daze) != 0)
            continue;
        if ((IS_NPC (ch) && IS_SET (pexit->to_room->room_flags, ROOM_NO_MOB)))
            continue;

        char_move (ch, door, FALSE);
        if ((now_in = ch->in_room) == was_in)
            continue;

        ch->in_room = was_in;
        act ("$n has fled!", ch, NULL, NULL, TO_NOTCHAR);
        ch->in_room = now_in;

        if (!IS_NPC (ch)) {
            const CLASS_T *class;
            send_to_char ("You flee from combat!\n\r", ch);

            class = class_get (ch->class);
            if (class != NULL && class->can_sneak_away &&
                    (number_percent () < 3 * (ch->level / 2)))
                send_to_char ("You snuck away safely.\n\r", ch);
            else {
                send_to_char ("You lost 10 exp.\n\r", ch);
                player_gain_exp (ch, -10);
            }
        }

        stop_fighting (ch, TRUE);
        return;
    }
    send_to_char ("PANIC! You couldn't escape!\n\r", ch);
}

DEFINE_DO_FUN (do_rescue) {
    char arg[MAX_INPUT_LENGTH];
    CHAR_T *victim;
    CHAR_T *fch;

    DO_REQUIRE_ARG (arg, "Rescue whom?\n\r");

    BAIL_IF ((victim = find_char_same_room (ch, arg)) == NULL,
        "They aren't here.\n\r", ch);
    BAIL_IF (victim == ch,
        "What about fleeing instead?\n\r", ch);
    /* Can't rescue pets? That would be nice! -- Synival */
    BAIL_IF_ACT (!IS_NPC (ch) && IS_NPC (victim),
        "$E doesn't need your help!\n\r", ch, NULL, victim);
    BAIL_IF (ch->fighting == victim,
        "Too late.\n\r", ch);
    BAIL_IF_ACT ((fch = victim->fighting) == NULL,
        "$N is not fighting right now.\n\r", ch, NULL, victim);
    BAIL_IF (IS_NPC (fch) && !is_same_group (ch, victim),
        "Kill stealing is not permitted.\n\r", ch);

    WAIT_STATE (ch, skill_table[SN(RESCUE)].beats);
    if (number_percent () > char_get_skill (ch, SN(RESCUE))) {
        send_to_char ("You fail the rescue.\n\r", ch);
        player_try_skill_improve (ch, SN(RESCUE), FALSE, 1);
        return;
    }

    act3 ("{5You rescue $N!{x",
          "{5$n rescues you!{x",
          "{5$n rescues $N!{x",
        ch, NULL, victim, 0, POS_RESTING);
    player_try_skill_improve (ch, SN(RESCUE), TRUE, 1);

    stop_fighting (fch, FALSE);
    stop_fighting (victim, FALSE);

    check_killer (ch, fch);
    if (ch->fighting == NULL)
        set_fighting_one (ch, fch);
    set_fighting_one (fch, ch);
}

DEFINE_DO_FUN (do_disarm) {
    CHAR_T *victim;
    OBJ_T *obj;
    int chance, hth, ch_weapon, vict_weapon, ch_vict_weapon;
    hth = 0;

    BAIL_IF ((chance = char_get_skill (ch, SN(DISARM))) == 0,
        "You don't know how to disarm opponents.\n\r", ch);
    BAIL_IF (char_get_eq_by_wear_loc (ch, WEAR_LOC_WIELD) == NULL &&
        ((hth = char_get_skill (ch, SN(HAND_TO_HAND))) == 0 ||
         (IS_NPC (ch) && !IS_SET (ch->off_flags, OFF_DISARM))),
        "You must wield a weapon to disarm.\n\r", ch);
    BAIL_IF ((victim = ch->fighting) == NULL,
        "You aren't fighting anyone.\n\r", ch);
    BAIL_IF ((obj = char_get_eq_by_wear_loc (victim, WEAR_LOC_WIELD)) == NULL,
        "Your opponent is not wielding a weapon.\n\r", ch);

    /* find weapon skills */
    ch_weapon      = char_get_weapon_skill (ch, char_get_weapon_sn (ch));
    vict_weapon    = char_get_weapon_skill (victim, char_get_weapon_sn (victim));
    ch_vict_weapon = char_get_weapon_skill (ch, char_get_weapon_sn (victim));

    /* modifiers */

    /* skill */
    if (char_get_eq_by_wear_loc (ch, WEAR_LOC_WIELD) == NULL)
        chance = chance * hth / 150;
    else
        chance = chance * ch_weapon / 100;

    chance += (ch_vict_weapon / 2 - vict_weapon) / 2;

    /* dex vs. strength */
    chance += char_get_curr_stat (ch, STAT_DEX);
    chance -= 2 * char_get_curr_stat (victim, STAT_STR);

    /* level */
    chance += (ch->level - victim->level) * 2;

    /* and now the attack */
    if (number_percent () < chance) {
        WAIT_STATE (ch, skill_table[SN(DISARM)].beats);
        disarm (ch, victim);
        player_try_skill_improve (ch, SN(DISARM), TRUE, 1);
    }
    else {
        WAIT_STATE (ch, skill_table[SN(DISARM)].beats);
        act3 ("{5You fail to disarm $N.{x",
              "{5$n tries to disarm you, but fails.{x",
              "{5$n tries to disarm $N, but fails.{x",
            ch, NULL, victim, 0, POS_RESTING);
        player_try_skill_improve (ch, SN(DISARM), FALSE, 1);
    }
    check_killer (ch, victim);
}

DEFINE_DO_FUN (do_surrender) {
    CHAR_T *mob;
    BAIL_IF ((mob = ch->fighting) == NULL,
        "But you're not fighting!\n\r", ch);
    act3 ("You surrender to $N!",
          "$n surrenders to you!",
          "$n tries to surrender to $N!",
        ch, NULL, mob, 0, POS_RESTING);

    BAIL_IF_ACT (!IS_NPC (ch) && IS_NPC (mob) &&
            (!HAS_TRIGGER (mob, TRIG_SURR) ||
             !mp_percent_trigger (mob, ch, NULL, NULL, TRIG_SURR)),
        "$N seems to ignore your cowardly act!", ch, NULL, mob);

    stop_fighting (ch, TRUE);
}

DEFINE_DO_FUN (do_disengage) {
    CHAR_T *rch;

    BAIL_IF (ch->fighting == NULL,
        "You're not fighting anybody.\n\r", ch);

    for (rch = ch->in_room->people_first; rch != NULL; rch = rch->room_next)
        BAIL_IF (rch->fighting == ch,
            "You can't disengage while someone is fighting you!\n\r", ch);

    act2 ("You disengage from combat.",
          "$n disengages from combat.", ch, NULL, NULL, 0, POS_RESTING);
    stop_fighting (ch, FALSE);
    WAIT_STATE (ch, PULSE_VIOLENCE);
}

/* 'Wimpy' originally by Dionysos. */
DEFINE_DO_FUN (do_wimpy) {
    char arg[MAX_INPUT_LENGTH];
    int wimpy;

    one_argument (argument, arg);
    wimpy = (arg[0] == '\0') ? (ch->max_hit / 5) : atoi (arg);

    BAIL_IF (wimpy < 0,
        "Your courage exceeds your wisdom.\n\r", ch);
    BAIL_IF (wimpy > ch->max_hit / 2,
        "Such cowardice ill becomes you.\n\r", ch);

    ch->wimpy = wimpy;
    printf_to_char (ch, "Wimpy set to %d hit points.\n\r", wimpy);
}

DEFINE_DO_FUN (do_consider) {
    char arg[MAX_INPUT_LENGTH];
    CHAR_T *victim;
    char *msg;
    int diff;

    DO_REQUIRE_ARG (arg, "Consider killing whom?\n\r");
    BAIL_IF ((victim = find_char_same_room (ch, arg)) == NULL,
        "They're not here.\n\r", ch);
    BAIL_IF (do_filter_can_attack (ch, victim),
        "Don't even think about it.\n\r", ch);

    diff = victim->level - ch->level;
         if (diff <= -10) msg = "You can kill $N naked and weaponless.";
    else if (diff <=  -5) msg = "$N is no match for you.";
    else if (diff <=  -2) msg = "$N looks like an easy kill.";
    else if (diff <=   1) msg = "The perfect match!";
    else if (diff <=   4) msg = "$N says 'Do you feel lucky, punk?'.";
    else if (diff <=   9) msg = "$N laughs at you mercilessly.";
    else                  msg = "Death will thank you for your gift.";
    act (msg, ch, NULL, victim, TO_CHAR);
}
