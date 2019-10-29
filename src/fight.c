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
 *    ROM 2.4 is copyright 1993-1998 Russ Taylor                           *
 *    ROM has been brought to you by the ROM consortium                    *
 *        Russ Taylor (rtaylor@hypercube.org)                              *
 *        Gabrielle Taylor (gtaylor@hypercube.org)                         *
 *        Brian Moore (zump@rom.org)                                       *
 *    By using this code, you have agreed to follow the terms of the       *
 *    ROM license, in the file Rom24/doc/rom.license                       *
 ***************************************************************************/

#include <string.h>

#include "interp.h"
#include "lookup.h"
#include "update.h"
#include "utils.h"
#include "db.h"
#include "effects.h"
#include "mob_prog.h"
#include "skills.h"
#include "comm.h"
#include "save.h"
#include "groups.h"
#include "magic.h"
#include "affects.h"
#include "act_fight.h"
#include "act_obj.h"
#include "act_comm.h"
#include "act_move.h"
#include "chars.h"
#include "objs.h"
#include "find.h"

#include "fight.h"

/* Nasty, nasty globals. */
char *damage_adj = NULL;

/* Advancement stuff. */
void advance_level (CHAR_DATA * ch, bool hide) {
    char buf[MAX_STRING_LENGTH];
    int add_hp;
    int add_mana;
    int add_move;
    int add_prac;

    if (IS_NPC (ch))
        return;

    ch->pcdata->last_level =
        (ch->played + (int) (current_time - ch->logon)) / 3600;

    sprintf (buf, "the %s",
        title_table[ch->class][ch->level][ch->sex == SEX_FEMALE ? 1 : 0]);
    char_set_title (ch, buf);

    add_hp = char_con_level_hp (ch) +
        number_range (class_table[ch->class].hp_min,
                      class_table[ch->class].hp_max);
    add_mana = number_range (2, (2 * char_get_curr_stat (ch, STAT_INT)
                                 + char_get_curr_stat (ch, STAT_WIS)) / 5);
    if (!class_table[ch->class].fMana)
        add_mana /= 2;
    add_move = number_range (1, (char_get_curr_stat (ch, STAT_CON)
                                 + char_get_curr_stat (ch, STAT_DEX)) / 6);
    add_prac = char_wis_level_practices (ch);

    add_hp   = add_hp   * 9 / 10;
    add_mana = add_mana * 9 / 10;
    add_move = add_move * 9 / 10;

    add_hp   = UMAX (2, add_hp);
    add_mana = UMAX (2, add_mana);
    add_move = UMAX (6, add_move);

    ch->max_hit  += add_hp;
    ch->max_mana += add_mana;
    ch->max_move += add_move;
    ch->practice += add_prac;
    ch->train += 1;

    ch->pcdata->perm_hit += add_hp;
    ch->pcdata->perm_mana += add_mana;
    ch->pcdata->perm_move += add_move;

    if (!hide) {
        printf_to_char (ch, "You gain %d hit point%s, %d mana, %d move, and "
            "%d practice%s.\n\r", add_hp, add_hp == 1 ? "" : "s", add_mana,
            add_move, add_prac, add_prac == 1 ? "" : "s");
    }
}

void gain_exp (CHAR_DATA * ch, int gain) {
    if (IS_NPC (ch) || ch->level >= LEVEL_HERO)
        return;

    ch->exp = UMAX (exp_per_level (ch, ch->pcdata->points), ch->exp + gain);
    while (ch->level < LEVEL_HERO && ch->exp >=
           exp_per_level (ch, ch->pcdata->points) * (ch->level + 1))
    {
        send_to_char ("{GYou raise a level!!  {x", ch);
        ch->level += 1;
        log_f ("%s gained level %d", ch->name, ch->level);
        wiznetf (ch, NULL, WIZ_LEVELS, 0, 0,
            "$N has attained level %d!", ch->level);
        advance_level (ch, FALSE);
        save_char_obj (ch);
    }
}

int should_assist_group (CHAR_DATA * bystander, CHAR_DATA * attacker,
    CHAR_DATA *victim)
{
    if (!is_same_group (bystander, attacker))
        return 0;
    if (is_same_group (bystander, victim))
        return 0;
    if (do_filter_can_attack (bystander, victim))
        return 0;
    if (!IS_NPC (bystander) && IS_SET (bystander->plr, PLR_AUTOASSIST))
        return 1;
    if (IS_AFFECTED (bystander, AFF_CHARM))
        return 1;
    return 0;
}

int npc_should_assist_player (CHAR_DATA * bystander, CHAR_DATA * player,
    CHAR_DATA *victim)
{
    if (!IS_NPC (bystander))
        return 0;
    if (IS_NPC (player))
        return 0;
    if (!IS_SET (bystander->off_flags, ASSIST_PLAYERS))
        return 0;

    /* bystander shouldn't assist any players fighting
     * something more than 5 levels above it. */
    if (bystander->level + 5 < victim->level)
        return 0;

    return 1;
}

bool npc_should_assist_attacker (CHAR_DATA * bystander, CHAR_DATA * attacker,
    CHAR_DATA * victim)
{
    if (!IS_NPC (bystander))
        return FALSE;

    /* assist group, everyone, race, or like-aligned. */
    if (bystander->group && bystander->group == attacker->group)
        return TRUE;
    if (IS_SET (bystander->off_flags, ASSIST_ALL))
        return TRUE;
    if (IS_SET (bystander->off_flags, ASSIST_RACE) &&
            bystander->race == attacker->race)
        return TRUE;
    if (IS_SET (bystander->off_flags, ASSIST_ALIGN) &&
            IS_SAME_ALIGN (bystander, attacker))
        return TRUE;

    /* programmed to assist a specific vnum? */
    if (bystander->pIndexData == attacker->pIndexData &&
            IS_SET (bystander->off_flags, ASSIST_VNUM))
        return TRUE;

    return FALSE;
}

CHAR_DATA * random_group_target_in_room (CHAR_DATA * bystander,
    CHAR_DATA * ch)
{
    CHAR_DATA *vch;
    int count, number;

    /* count suitable targets. */
    count = 0;
    for (vch = ch->in_room->people; vch; vch = vch->next)
        if (char_can_see_in_room (bystander, vch) && is_same_group (vch, ch))
            count++;
    if (count == 0)
        return NULL;

    number = number_range (0, count);
    for (vch = ch->in_room->people; vch; vch = vch->next)
        if (char_can_see_in_room (bystander, vch) && is_same_group (vch, ch) &&
            number-- == 0)
            return vch;

    return NULL;
}

/* for auto assisting */
void check_assist (CHAR_DATA * ch, CHAR_DATA * victim) {
    CHAR_DATA *rch, *rch_next;

    /* check everything in the room to see if it should assist
     * in this fight. */
    for (rch = ch->in_room->people; rch != NULL; rch = rch_next) {
        rch_next = rch->next_in_room;
        if (!IS_AWAKE (rch))
            continue;
        if (rch->fighting != NULL)
            continue;

        /* check players or charmed NPCs */
        if (!IS_NPC (ch) || IS_AFFECTED (ch, AFF_CHARM)) {
            if (should_assist_group (rch, ch, victim))
                multi_hit (rch, victim, TYPE_UNDEFINED);
            continue;
        }

        /* quick check for ASSIST_PLAYER */
        if (npc_should_assist_player (rch, ch, victim)) {
            do_function (rch, &do_emote, "screams and attacks!");
            multi_hit (rch, victim, TYPE_UNDEFINED);
            continue;
        }

        /* 25% chance to randomly assist attackers. */
        if (number_percent() < 25 &&
            npc_should_assist_attacker (rch, ch, victim))
        {
            CHAR_DATA * target = random_group_target_in_room (rch, victim);
            if (target == NULL)
                continue;

            /* attack! */
            do_function (rch, &do_emote, "screams and attacks!");
            multi_hit (rch, target, TYPE_UNDEFINED);
        }
    }
}

/* Do one group of attacks. */
void multi_hit (CHAR_DATA * ch, CHAR_DATA * victim, int dt) {
    int chance;
    set_fighting_position_if_possible(ch);

    /* no sleep-fighting! */
    if (ch->position <= POS_SLEEPING)
        return;

    if (IS_NPC (ch)) {
        mob_hit (ch, victim, dt);
        return;
    }

    one_hit (ch, victim, dt);
    if (ch->fighting != victim)
        return;

    if (IS_AFFECTED (ch, AFF_HASTE))
        one_hit (ch, victim, dt);
    if (ch->fighting != victim || dt == gsn_backstab)
        return;

    chance = get_skill (ch, gsn_second_attack) / 2;
    if (IS_AFFECTED (ch, AFF_SLOW))
        chance /= 2;

    if (number_percent () < chance) {
        one_hit (ch, victim, dt);
        check_improve (ch, gsn_second_attack, TRUE, 5);
        if (ch->fighting != victim)
            return;
    }

    chance = get_skill (ch, gsn_third_attack) / 4;
    if (IS_AFFECTED (ch, AFF_SLOW))
        chance = 0;;

    if (number_percent () < chance) {
        one_hit (ch, victim, dt);
        check_improve (ch, gsn_third_attack, TRUE, 6);
        if (ch->fighting != victim)
            return;
    }
}

/* procedure for all mobile attacks */
void mob_hit (CHAR_DATA * ch, CHAR_DATA * victim, int dt) {
    int chance, number;
    CHAR_DATA *vch, *vch_next;

    one_hit (ch, victim, dt);
    if (ch->fighting != victim)
        return;

    /* Area attack -- BALLS nasty! */
    if (IS_SET (ch->off_flags, OFF_AREA_ATTACK)) {
        for (vch = ch->in_room->people; vch != NULL; vch = vch_next) {
            vch_next = vch->next;
            if ((vch != victim && vch->fighting == ch))
                one_hit (ch, vch, dt);
        }
    }

    if (IS_AFFECTED (ch, AFF_HASTE) ||
        (IS_SET (ch->off_flags, OFF_FAST) && !IS_AFFECTED (ch, AFF_SLOW)))
    {
        one_hit (ch, victim, dt);
    }
    if (ch->fighting != victim || dt == gsn_backstab)
        return;

    chance = get_skill (ch, gsn_second_attack) / 2;
    if (IS_AFFECTED (ch, AFF_SLOW) && !IS_SET (ch->off_flags, OFF_FAST))
        chance /= 2;

    if (number_percent () < chance) {
        one_hit (ch, victim, dt);
        if (ch->fighting != victim)
            return;
    }

    chance = get_skill (ch, gsn_third_attack) / 4;
    if (IS_AFFECTED (ch, AFF_SLOW) && !IS_SET (ch->off_flags, OFF_FAST))
        chance = 0;

    if (number_percent () < chance) {
        one_hit (ch, victim, dt);
        if (ch->fighting != victim)
            return;
    }

    /* oh boy!  Fun stuff! */
    if (ch->wait > 0 || ch->position < POS_STANDING)
        return;

    /* now for the skills */
    number = number_range (0, 8);
    switch (number) {
        case (0):
            if (IS_SET (ch->off_flags, OFF_BASH))
                do_function (ch, &do_bash, "");
            break;

        case (1):
            if (IS_SET (ch->off_flags, OFF_BERSERK)
                && !IS_AFFECTED (ch, AFF_BERSERK))
                do_function (ch, &do_berserk, "");
            break;

        case (2):
            if (IS_SET (ch->off_flags, OFF_DISARM)
                || (char_get_weapon_sn (ch) != gsn_hand_to_hand
                    && (IS_SET (ch->mob, MOB_WARRIOR) ||
                        IS_SET (ch->mob, MOB_THIEF))))
                do_function (ch, &do_disarm, "");
            break;

        case (3):
            if (IS_SET (ch->off_flags, OFF_KICK))
                do_function (ch, &do_kick, "");
            break;

        case (4):
            if (IS_SET (ch->off_flags, OFF_KICK_DIRT))
                do_function (ch, &do_dirt, "");
            break;

        case (5):
            if (IS_SET (ch->off_flags, OFF_TAIL))
                ; /* do_function(ch, &do_tail, "") */
            break;

        case (6):
            if (IS_SET (ch->off_flags, OFF_TRIP))
                do_function (ch, &do_trip, "");
            break;

        case (7):
            if (IS_SET (ch->off_flags, OFF_CRUSH))
                ; /* do_function(ch, &do_crush, "") */
            break;

        case (8):
            if (IS_SET (ch->off_flags, OFF_BACKSTAB))
                do_function (ch, &do_backstab, "");
            break;
    }
}

/* Hit one guy once. */
void one_hit (CHAR_DATA * ch, CHAR_DATA * victim, int dt) {
    OBJ_DATA *wield;
    char *extra_adj = NULL;
    int victim_ac;
    int thac0;
    int thac0_00;
    int thac0_32;
    int dam;
    int diceroll;
    int sn, skill;
    int dam_type;
    int missed;
    int skill_val;
    bool result;

    sn = -1;

    /* just in case */
    if (victim == ch || ch == NULL || victim == NULL)
        return;

    /* Can't beat a dead char!
     * Guard against weird room-leavings. */
    if (victim->position == POS_DEAD || ch->in_room != victim->in_room)
        return;

    /* Figure out the type of damage message. */
    wield = char_get_eq_by_wear_loc (ch, WEAR_WIELD);
    if (dt == TYPE_UNDEFINED) {
        dt = TYPE_HIT;
        if (wield != NULL && wield->item_type == ITEM_WEAPON)
            dt += wield->v.weapon.attack_type;
        else
            dt += ch->dam_type;
    }

    if (dt < TYPE_HIT) {
        if (wield != NULL)
            dam_type = attack_table[wield->v.weapon.attack_type].damage;
        else
            dam_type = attack_table[ch->dam_type].damage;
    }
    else
        dam_type = attack_table[dt - TYPE_HIT].damage;

    if (dam_type == -1)
        dam_type = DAM_BASH;

    /* get the weapon skill */
    sn = char_get_weapon_sn (ch);
    skill = 20 + char_get_weapon_skill (ch, sn);

    /* Calculate to-hit-armor-class-0 versus armor. */
    if (IS_NPC (ch)) {
        thac0_00 = 20;
        thac0_32 = -4; /* as good as a thief */
        if (IS_SET (ch->mob, MOB_WARRIOR))
            thac0_32 = -10;
        else if (IS_SET (ch->mob, MOB_THIEF))
            thac0_32 = -4;
        else if (IS_SET (ch->mob, MOB_CLERIC))
            thac0_32 = 2;
        else if (IS_SET (ch->mob, MOB_MAGE))
            thac0_32 = 6;
    }
    else {
        thac0_00 = class_table[ch->class].thac0_00;
        thac0_32 = class_table[ch->class].thac0_32;
    }
    thac0 = interpolate (ch->level, thac0_00, thac0_32);

    if (thac0 < 0)
        thac0 = thac0 / 2;
    if (thac0 < -5)
        thac0 = -5 + (thac0 + 5) / 2;

    thac0 -= GET_HITROLL (ch) * skill / 100;
    thac0 += 5 * (100 - skill) / 100;

    if (dt == gsn_backstab)
        thac0 -= 10 * (100 - get_skill (ch, gsn_backstab));

    switch (dam_type) {
        case (DAM_PIERCE): victim_ac = GET_AC (victim, AC_PIERCE) / 10; break;
        case (DAM_BASH):   victim_ac = GET_AC (victim, AC_BASH)   / 10; break;
        case (DAM_SLASH):  victim_ac = GET_AC (victim, AC_SLASH)  / 10; break;
        default:           victim_ac = GET_AC (victim, AC_EXOTIC) / 10; break;
    };

    if (victim_ac < -15)
        victim_ac = (victim_ac + 15) / 5 - 15;
    if (!char_can_see_in_room (ch, victim))
        victim_ac -= 4;

    switch (victim->position) {
        case POS_FIGHTING: victim_ac += 0; break;
        case POS_STANDING: victim_ac += 1; break;
        case POS_SITTING:  victim_ac += 2; break;
        case POS_RESTING:  victim_ac += 3; break;
        default:           victim_ac += 4; break;
    }

    /* The moment of excitement! */
    while ((diceroll = number_bits (5)) >= 20)
        ;

    /* Check for misses. We want to calculate the damage, but not deal it -
     * just display it using dam_message. */
    missed = FALSE;
    if (diceroll == 0 || (diceroll != 19 && diceroll < thac0 - victim_ac))
        missed = TRUE;

    /* Hit.  Calc damage. */
    if (IS_NPC (ch) && (!ch->pIndexData->new_format || wield == NULL)) {
        if (!ch->pIndexData->new_format) {
            dam = number_range (ch->level / 2, ch->level * 3 / 2);
            if (wield != NULL)
                dam += dam / 2;
        }
        else
            dam = dice (ch->damage[DICE_NUMBER], ch->damage[DICE_TYPE]);
    }
    else {
        if (sn != -1)
            check_improve (ch, sn, TRUE, 5);
        if (wield != NULL) {
            if (wield->pIndexData->new_format)
                dam = dice (wield->v.weapon.dice_num,
                            wield->v.weapon.dice_size) * skill / 100;
            else
                dam = number_range (wield->v.weapon.dice_num  * skill / 100,
                                    wield->v.weapon.dice_size * skill / 100);

            if (char_get_eq_by_wear_loc (ch, WEAR_SHIELD) == NULL)    /* no shield = more */
                dam = dam * 11 / 10;

            /* sharpness! */
            if (IS_WEAPON_STAT (wield, WEAPON_SHARP)) {
                int percent;
                if ((percent = number_percent ()) <= (skill / 8))
                    dam = 2 * dam + (dam * 2 * percent / 100);
            }
        }
        else
            dam = number_range (1 + 4 * skill / 100,
                                2 * ch->level / 3 * skill / 100);
    }

    /* Bonuses. */
    skill_val = get_skill (ch, gsn_enhanced_damage) * 2 / 3;
    if (skill_val > 0 && number_percent() < skill_val) {
#ifndef VANILLA
        extra_adj = "heavy";
#endif
        check_improve (ch, gsn_enhanced_damage, TRUE, 6);
        dam += (dam * 3) / 4;
    }

    if (!IS_AWAKE (victim))
        dam *= 2;
    else if (victim->position < POS_FIGHTING)
        dam = dam * 3 / 2;

    if (dt == gsn_backstab && wield != NULL) {
        if (wield->v.weapon.weapon_type != WEAPON_DAGGER)
            dam *= 2 + (ch->level / 10);
        else
            dam *= 2 + (ch->level / 8);
    }

    dam += GET_DAMROLL (ch) * UMIN (100, skill) / 100;
    if (dam <= 0)
        dam = 1;

    damage_adj = extra_adj;
    if (missed) {
        damage (ch, victim, 0, dt, dam_type, FALSE);
        dam_message (ch, victim, 0, dt, FALSE, dam);
        return;
    }
    result = damage (ch, victim, dam, dt, dam_type, TRUE);

    /* but do we have a funky weapon? */
    if (result && wield != NULL) {
        int dam;
        if (ch->fighting == victim && IS_WEAPON_STAT (wield, WEAPON_POISON)) {
            int level;
            AFFECT_DATA *poison, af;

            if ((poison = affect_find (wield->affected, gsn_poison)) == NULL)
                level = wield->level;
            else
                level = poison->level;

            if (!saves_spell (level / 2, victim, DAM_POISON)) {
                send_to_char ("You feel poison coursing through your veins.",
                              victim);
                act ("$n is poisoned by the venom on $p.",
                     victim, wield, NULL, TO_NOTCHAR);

                affect_init (&af, AFF_TO_AFFECTS, gsn_poison, level * 3 / 4, level / 2, APPLY_STR, -1, AFF_POISON);
                affect_join (victim, &af);
            }

            /* weaken the poison if it's temporary */
            if (poison != NULL) {
                poison->level = UMAX (0, poison->level - 2);
                poison->duration = UMAX (0, poison->duration - 1);

                if (poison->level == 0 || poison->duration == 0)
                    act ("The poison on $p has worn off.", ch, wield, NULL,
                         TO_CHAR);
            }
        }

        if (ch->fighting == victim && IS_WEAPON_STAT (wield, WEAPON_VAMPIRIC)) {
            dam = number_range (1, wield->level / 5 + 1);
            act ("You feel $p drawing your life away.",
                 victim, wield, NULL, TO_CHAR);
            act ("$p draws life from $n.", victim, wield, NULL, TO_NOTCHAR);
            damage (ch, victim, dam, 0, DAM_NEGATIVE, FALSE);
            ch->alignment = UMAX (-1000, ch->alignment - 1);
            ch->hit += dam / 2;
        }

        if (ch->fighting == victim && IS_WEAPON_STAT (wield, WEAPON_FLAMING)) {
            dam = number_range (1, wield->level / 4 + 1);
            act ("$p sears your flesh.", victim, wield, NULL, TO_CHAR);
            act ("$n is burned by $p.", victim, wield, NULL, TO_NOTCHAR);
            fire_effect ((void *) victim, wield->level / 2, dam, TARGET_CHAR);
            damage (ch, victim, dam, 0, DAM_FIRE, FALSE);
        }

        if (ch->fighting == victim && IS_WEAPON_STAT (wield, WEAPON_FROST)) {
            dam = number_range (1, wield->level / 6 + 2);
            act ("The cold touch of $p surrounds you with ice.",
                 victim, wield, NULL, TO_CHAR);
            act ("$p freezes $n.", victim, wield, NULL, TO_NOTCHAR);
            cold_effect (victim, wield->level / 2, dam, TARGET_CHAR);
            damage (ch, victim, dam, 0, DAM_COLD, FALSE);
        }

        if (ch->fighting == victim && IS_WEAPON_STAT (wield, WEAPON_SHOCKING)) {
            dam = number_range (1, wield->level / 5 + 2);
            act ("You are shocked by $p.", victim, wield, NULL, TO_CHAR);
            act ("$n is struck by lightning from $p.", victim, wield, NULL,
                 TO_NOTCHAR);
            shock_effect (victim, wield->level / 2, dam, TARGET_CHAR);
            damage (ch, victim, dam, 0, DAM_LIGHTNING, FALSE);
        }
    }

    tail_chain ();
}


/* Inflict damage from a hit. */
bool damage (CHAR_DATA * ch, CHAR_DATA * victim, int dam, int dt,
             int dam_type, bool show)
{
    OBJ_DATA *corpse;
    bool immune;
    int orig_dam = dam, last_pos;
    int log_target;

    if (victim->position == POS_DEAD)
        return FALSE;

    /* Stop up any residual loopholes. */
    if (dam > 1200 && dt >= TYPE_HIT) {
        bug ("damage: %d: more than 1200 points!", dam);
        dam = 1200;
        if (!IS_IMMORTAL (ch)) {
            OBJ_DATA *obj;
            obj = char_get_eq_by_wear_loc (ch, WEAR_WIELD);
            send_to_char ("You really shouldn't cheat.\n\r", ch);
            if (obj != NULL)
                obj_extract (obj);
        }

    }

    /* damage reduction */
    if (dam > 35)
        dam = (dam - 35) / 2 + 35;
    if (dam > 80)
        dam = (dam - 80) / 2 + 80;

    if (victim != ch) {
        /* Certain attacks are forbidden.
         * Most other attacks are returned. */
        if (do_filter_can_attack (ch, victim))
            return FALSE;
        check_killer (ch, victim);

        /* Make sure we're fighting the victim. */
        if (victim->position > POS_STUNNED) {
            if (ch->fighting == NULL)
                set_fighting_one (ch, victim);
            set_fighting_position_if_possible(ch);
        }

        /* Tell the victim to fight back! */
        if (victim->position > POS_STUNNED) {
            if (victim->fighting == NULL) {
                set_fighting_one (victim, ch);
                if (IS_NPC (victim) && HAS_TRIGGER (victim, TRIG_KILL))
                    mp_percent_trigger (victim, ch, NULL, NULL, TRIG_KILL);
            }
            set_fighting_position_if_possible(victim);
        }

        /* More charm stuff. */
        if (victim->master == ch)
            stop_follower (victim);
    }

    /* Inviso attacks ... not. */
    if (IS_AFFECTED (ch, AFF_INVISIBLE)) {
        affect_strip (ch, gsn_invis);
        affect_strip (ch, gsn_mass_invis);
        REMOVE_BIT (ch->affected_by, AFF_INVISIBLE);
        act ("$n fades into existence.", ch, NULL, NULL, TO_NOTCHAR);
    }

    /* Damage modifiers. */
    if (dam > 1 && IS_DRUNK (victim))
        dam = 9 * dam / 10;
    if (dam > 1 && IS_AFFECTED (victim, AFF_SANCTUARY))
        dam /= 2;
    if (dam > 1 &&
        ((IS_AFFECTED (victim, AFF_PROTECT_EVIL) && IS_EVIL (ch)) ||
         (IS_AFFECTED (victim, AFF_PROTECT_GOOD) && IS_GOOD (ch))))
        dam -= dam / 4;

    /* Check for parry, and dodge. */
    if (dt >= TYPE_HIT && ch != victim) {
        if (check_parry (ch, victim))
            return FALSE;
        if (check_dodge (ch, victim))
            return FALSE;
        if (check_shield_block (ch, victim))
            return FALSE;
    }

    immune = FALSE;
    switch (check_immune (victim, dam_type)) {
        case (IS_IMMUNE):
            immune = TRUE;
            dam = 0;
            break;
        case (IS_RESISTANT):
            dam -= dam / 3;
            break;
        case (IS_VULNERABLE):
            dam += dam / 2;
            break;
    }

    if (show)
        dam_message (ch, victim, dam, dt, immune, orig_dam);
    if (dam == 0)
        return FALSE;

    /* Hurt the victim.
     * Inform the victim of his new state. */
    victim->hit -= dam;
    if (!IS_NPC (victim) && victim->level >= LEVEL_IMMORTAL && victim->hit < 1)
        victim->hit = 1;
    last_pos = victim->position;
    update_pos (victim);

    if (last_pos != victim->position) {
        switch (victim->position) {
            case POS_MORTAL:
                act ("$n is mortally wounded, and will die soon, if not aided.", victim, NULL, NULL, TO_NOTCHAR);
                send_to_char ("You are mortally wounded, and will die soon, if not aided.\n\r", victim);
                break;

            case POS_INCAP:
                act ("$n is incapacitated and will slowly die, if not aided.", victim, NULL, NULL, TO_NOTCHAR);
                send_to_char ("You are incapacitated and will slowly die, if not aided.\n\r",
                     victim);
                break;

            case POS_STUNNED:
                act ("$n is stunned, but will probably recover.", victim, NULL, NULL, TO_NOTCHAR);
                send_to_char ("You are stunned, but will probably recover.\n\r", victim);
                break;

            case POS_DEAD:
                act ("{R$n is DEAD!!{x", victim, NULL, NULL, TO_NOTCHAR);
                send_to_char ("{RYou have been KILLED!!{x\n\r\n\r", victim);
                break;
        }
    }

    /* Message for serious damage! You'd better run! */
    if (victim->position > POS_STUNNED) {
        if (dam > victim->max_hit / 4)
            send_to_char ("{RThat really did HURT!{x\n\r", victim);
        if (victim->hit < victim->max_hit / 4)
            send_to_char ("{RYou sure are BLEEDING!{x\n\r", victim);
    }

    /* Sleep spells and extremely wounded folks. */
    if (!IS_AWAKE (victim))
        stop_fighting (victim, FALSE);

    /* Payoff for killing things. */
    if (victim->position == POS_DEAD) {
        stop_fighting (victim, TRUE);
        group_gain (ch, victim);
        if (!IS_NPC (victim)) {
            log_f ("%s killed by %s at %d",
                victim->name, PERS (ch), ch->in_room->vnum);

            /* Dying penalty: 2/3 way back to previous level. */
            if (victim->exp > exp_per_level (victim, victim->pcdata->points)
                * victim->level)
            {
                gain_exp (victim, (2 *
                   (exp_per_level (victim, victim->pcdata->points) *
                    victim->level - victim->exp) / 3) + 50);
            }
        }

        log_target = (IS_NPC (victim)) ? WIZ_MOBDEATHS : WIZ_DEATHS;
        wiznetf (NULL, NULL, log_target, 0, 0,
            "%s got toasted by %s at %s [room %d]",
            PERS (victim), PERS (ch), ch->in_room->name, ch->in_room->vnum);

        /* Death trigger */
        if (IS_NPC (victim) && HAS_TRIGGER (victim, TRIG_DEATH)) {
            victim->position = POS_STANDING;
            mp_percent_trigger (victim, ch, NULL, NULL, TRIG_DEATH);
        }

        raw_kill (victim);

        /* dump the flags */
        if (ch != victim && !IS_NPC (ch) && !char_in_same_clan (ch, victim)) {
            if (IS_SET (victim->plr, PLR_KILLER))
                REMOVE_BIT (victim->plr, PLR_KILLER);
            else
                REMOVE_BIT (victim->plr, PLR_THIEF);
        }

        /* RT new auto commands */
        if (!IS_NPC (ch)
            && (corpse = find_obj_same_room (ch, "corpse")) != NULL
            && corpse->item_type == ITEM_CORPSE_NPC
            && char_can_see_obj (ch, corpse))
        {
            OBJ_DATA *coins;

            corpse = find_obj_same_room (ch, "corpse");

            /* exists and not empty */
            if (IS_SET (ch->plr, PLR_AUTOLOOT) && corpse && corpse->contains)
                do_function (ch, &do_get, "all corpse");

            /* exists and not empty */
            if (IS_SET (ch->plr, PLR_AUTOGOLD) && corpse && corpse->contains &&
                !IS_SET (ch->plr, PLR_AUTOLOOT))
            {
                if ((coins = find_obj_container (ch, corpse, "gcash")) != NULL)
                    do_function (ch, &do_get, "all.gcash corpse");
            }

            if (IS_SET (ch->plr, PLR_AUTOSAC)) {
                if (IS_SET (ch->plr, PLR_AUTOLOOT) && corpse && corpse->contains)
                    return TRUE; /* leave if corpse has treasure */
                else
                    do_function (ch, &do_sacrifice, "corpse");
            }
        }
        return TRUE;
    }

    if (victim == ch)
        return TRUE;

    /* Take care of link dead people. */
    if (!IS_NPC (victim) && victim->desc == NULL) {
        if (number_range (0, victim->wait) == 0) {
            do_function (victim, &do_recall, "");
            return TRUE;
        }
    }

    /* Wimp out? */
    if (IS_NPC (victim) && dam > 0 && victim->wait < PULSE_VIOLENCE / 2) {
        if ((IS_SET (victim->mob, MOB_WIMPY) && number_bits (2) == 0
             && victim->hit < victim->max_hit / 5)
            || (IS_AFFECTED (victim, AFF_CHARM) && victim->master != NULL
                && victim->master->in_room != victim->in_room))
        {
            do_function (victim, &do_flee, "");
        }
    }

    if (!IS_NPC (victim)
        && victim->hit > 0
        && victim->hit <= victim->wimpy && victim->wait < PULSE_VIOLENCE / 2)
    {
        do_function (victim, &do_flee, "");
    }

    tail_chain ();
    return TRUE;
}

bool set_fighting_position_if_possible (CHAR_DATA * ch) {
    if (ch->timer > 4)
        return FALSE;
    else if (ch->position < POS_FIGHTING && (ch->daze > 0 || ch->wait > 0))
        return FALSE;
    else if (ch->fighting == NULL)
        return FALSE;
    if (ch->position != POS_FIGHTING) {
        position_change_send_message(ch, ch->position, POS_FIGHTING, ch->on);
        ch->position = POS_FIGHTING;
    }
    return TRUE;
}

bool do_filter_can_attack (CHAR_DATA *ch, CHAR_DATA *victim)
    { return do_filter_can_attack_real (ch, victim, FALSE, FALSE); }
bool do_filter_can_attack_spell (CHAR_DATA * ch, CHAR_DATA * victim, bool area)
    { return do_filter_can_attack_real (ch, victim, area, FALSE); }

bool can_attack (CHAR_DATA *ch, CHAR_DATA *victim) {
    return do_filter_can_attack_real (ch, victim, FALSE, TRUE) ? FALSE : TRUE;
}
bool can_attack_spell (CHAR_DATA *ch, CHAR_DATA *victim, bool area) {
    return do_filter_can_attack_real (ch, victim, area, TRUE) ? FALSE : TRUE;
}

bool do_filter_can_attack_real (CHAR_DATA *ch, CHAR_DATA *victim, bool area,
    bool quiet)
{
    if (victim->in_room == NULL || ch->in_room == NULL)
        return TRUE;
    if (victim == ch && area)
        return TRUE;
    if (victim->fighting == ch || victim == ch)
        return FALSE;
    if (IS_IMMORTAL (ch) && ch->level > LEVEL_IMMORTAL && !area)
        return FALSE;

#define QU(x) ((quiet) ? NULL : (x))
    /* killing mobiles */
    if (IS_NPC (victim)) {
        /* safe room? */
        FILTER (IS_SET (victim->in_room->room_flags, ROOM_SAFE),
            QU("Not in this room.\n\r"), ch);

        /* no killing shopkeepers or healers, trainers, etc */
        FILTER (victim->pIndexData->pShop != NULL,
            QU("The shopkeeper wouldn't like that.\n\r"), ch);
        FILTER ((victim->mob & MOB_FRIENDLY_BITS) != 0,
            QU("I don't think Mota would approve.\n\r"), ch);

        if (!IS_NPC (ch)) {
            /* no pets or charmed creatures (unless owner) */
            FILTER_ACT (IS_PET (victim),
                QU("But $N looks so cute and cuddly..."), ch, NULL, victim);
            FILTER (IS_AFFECTED (victim, AFF_CHARM) &&
                    (area || ch != victim->master),
                QU("You don't own that monster.\n\r"), ch);

            /* legal kill? -- cannot hit mob fighting non-group member */
            FILTER (victim->fighting != NULL &&
                    !is_same_group (ch, victim->fighting),
                QU("Kill-stealing is not permitted.\n\r"), ch);
        }
        else {
            /* area effect spells do not hit other mobs */
            FILTER (area && !is_same_group (victim, ch->fighting),
                QU("Kill-stealing is not permitted.\n\r"), ch);
        }
    }
    /* killing players */
    else {
        if (area && IS_IMMORTAL (victim) && victim->level > LEVEL_IMMORTAL)
            return TRUE;

        /* NPC doing the killing */
        if (IS_NPC (ch)) {
            /* safe room check */
            FILTER (IS_SET (victim->in_room->room_flags, ROOM_SAFE),
                QU("Not in this room.\n\r"), ch);

            /* charmed mobs and pets cannot attack players while owned */
            FILTER (IS_AFFECTED (ch, AFF_CHARM) && ch->master != NULL &&
                    ch->master->fighting != victim,
                 QU("Players are your friends!\n\r"), ch);

            /* legal kill? -- mobs only hit players grouped with opponent */
            FILTER (ch->fighting != NULL && !is_same_group (ch->fighting,
                    victim),
                QU("Maybe finish fighting your current foes first?!\n\r"), ch);
        }
        /* player doing the killing */
        else {
            FILTER (!char_has_clan (ch),
                QU("Join a clan if you want to kill players.\n\r"), ch);

            /* Killing undesirables is allowed. */
            if (IS_SET (victim->plr, PLR_KILLER) ||
                IS_SET (victim->plr, PLR_THIEF))
                return FALSE;

            FILTER (!char_has_clan (victim),
                QU("They aren't in a clan, leave them alone.\n\r"), ch);
            FILTER (ch->level > victim->level + 8,
                QU("Pick on someone your own size.\n\r"), ch);
        }
    }
#undef QU

    /* All checks passed - don't filter. */
    return FALSE;
}

/* See if an attack justifies a KILLER flag. */
void check_killer (CHAR_DATA * ch, CHAR_DATA * victim) {
    /* Follow charm thread to responsible character.
     * Attacking someone's charmed char is hostile! */
    while (IS_AFFECTED (victim, AFF_CHARM) && victim->master != NULL)
        victim = victim->master;

    /* NPC's are fair game.
     * So are killers and thieves. */
    if (IS_NPC (victim)
        || IS_SET (victim->plr, PLR_KILLER)
        || IS_SET (victim->plr, PLR_THIEF))
        return;

    /* Charm-o-rama. */
    if (IS_SET (ch->affected_by, AFF_CHARM)) {
        if (ch->master == NULL) {
            bugf ("Check_killer: %s bad AFF_CHARM", PERS (ch));
            affect_strip (ch, gsn_charm_person);
            REMOVE_BIT (ch->affected_by, AFF_CHARM);
            return;
        }
/*
    send_to_char( "*** You are now a KILLER!! ***\n\r", ch->master );
      SET_BIT(ch->master->plr, PLR_KILLER);
*/

        stop_follower (ch);
        return;
    }

    /* NPC's are cool of course (as long as not charmed).
     * Hitting yourself is cool too (bleeding).
     * So is being immortal (Alander's idea).
     * And current killers stay as they are. */
    if (IS_NPC (ch)
        || ch == victim || ch->level >= LEVEL_IMMORTAL || !char_has_clan (ch)
        || IS_SET (ch->plr, PLR_KILLER) || ch->fighting == victim)
        return;

    send_to_char ("*** You are now a KILLER!! ***\n\r", ch);
    SET_BIT (ch->plr, PLR_KILLER);
    wiznetf (ch, NULL, WIZ_FLAGS, 0, 0,
        "$N is attempting to murder %s", victim->name);
    save_char_obj (ch);
}

/* Check for parry. */
bool check_parry (CHAR_DATA * ch, CHAR_DATA * victim) {
    int chance;

    if (!IS_AWAKE (victim))
        return FALSE;

    chance = get_skill (victim, gsn_parry) / 2;
    if (char_get_eq_by_wear_loc (victim, WEAR_WIELD) == NULL) {
        if (IS_NPC (victim))
            chance /= 2;
        else
            return FALSE;
    }
    if (!char_can_see_in_room (ch, victim))
        chance /= 2;
    if (number_percent () >= chance + victim->level - ch->level)
        return FALSE;

    act ("You parry $n's attack.", ch, NULL, victim, TO_VICT);
    act ("$N parries your attack.", ch, NULL, victim, TO_CHAR);
    check_improve (victim, gsn_parry, TRUE, 6);
    return TRUE;
}

/* Check for shield block. */
bool check_shield_block (CHAR_DATA * ch, CHAR_DATA * victim) {
    int chance;

    if (!IS_AWAKE (victim))
        return FALSE;
    if (char_get_eq_by_wear_loc (victim, WEAR_SHIELD) == NULL)
        return FALSE;

    chance = get_skill (victim, gsn_shield_block) / 5 + 3;
    if (number_percent () >= chance + victim->level - ch->level)
        return FALSE;

    act ("You block $n's attack with your shield.", ch, NULL, victim, TO_VICT);
    act ("$N blocks your attack with a shield.", ch, NULL, victim, TO_CHAR);
    check_improve (victim, gsn_shield_block, TRUE, 6);
    return TRUE;
}

/* Check for dodge. */
bool check_dodge (CHAR_DATA * ch, CHAR_DATA * victim) {
    int chance;
    if (!IS_AWAKE (victim))
        return FALSE;

    chance = get_skill (victim, gsn_dodge) / 2;
    if (!char_can_see_in_room (victim, ch))
        chance /= 2;
    if (number_percent () >= chance + victim->level - ch->level)
        return FALSE;

    act ("You dodge $n's attack.", ch, NULL, victim, TO_VICT);
    act ("$N dodges your attack.", ch, NULL, victim, TO_CHAR);
    check_improve (victim, gsn_dodge, TRUE, 6);
    return TRUE;
}

/* Set position of a victim. */
void update_pos (CHAR_DATA * victim) {
    if (victim->hit > 0) {
        if (victim->position <= POS_STUNNED)
            victim->position = POS_RESTING;
    }
    else {
        int percent = victim->hit * 100 / victim->max_hit;

#ifndef VANILLA
        if (percent <= -30)
            victim->position = POS_DEAD;
#else
        if (IS_NPC (victim) || percent <= -30)
            victim->position = POS_DEAD;
#endif
        else if (percent <= -20)
            victim->position = POS_MORTAL;
        else if (percent <= -10)
            victim->position = POS_INCAP;
        else
            victim->position = POS_STUNNED;
    }
}

/* Start fights. */
void set_fighting_both (CHAR_DATA * ch, CHAR_DATA * victim) {
    if (ch->fighting == NULL)
        set_fighting_one (ch, victim);
    if (victim->fighting == NULL)
        set_fighting_one (victim, ch);
}

void set_fighting_one (CHAR_DATA * ch, CHAR_DATA * victim) {
    int daze_mult = 0;

    BAIL_IF_BUG (ch->fighting != NULL,
        "set_fighting: already fighting", 0);

    if (IS_AFFECTED (ch, AFF_SLEEP))
        affect_strip (ch, gsn_sleep);

    if (victim->fighting == NULL && victim->daze == 0 &&
        victim->position < POS_FIGHTING)
    {
        switch (victim->position) {
            case POS_SLEEPING:
                send_to_char ("You were caught sleeping!\n\r", victim);
                act ("$n was caught sleeping!", victim, NULL, NULL, TO_NOTCHAR);
                daze_mult = 3;
                break;

            case POS_RESTING:
                send_to_char ("You were caught resting!\n\r", victim);
                act ("$n was caught resting!", victim, NULL, NULL, TO_NOTCHAR);
                daze_mult = 2;
                break;

            case POS_SITTING:
                send_to_char ("You were caught sitting down!\n\r", victim);
                act ("$n was caught sitting down!", victim, NULL, NULL, TO_NOTCHAR);
                daze_mult = 1;
                break;
        }
        if (daze_mult > 0) {
            do_function (victim, &do_sit, "");
            DAZE_STATE (victim, daze_mult * PULSE_VIOLENCE + PULSE_VIOLENCE / 2);
        }
    }

    ch->fighting = victim;
    set_fighting_position_if_possible (ch);
}

/* Stop fights. */
void stop_fighting_one (CHAR_DATA * ch) {
    ch->fighting = NULL;
    if (ch->position == POS_FIGHTING)
        ch->position = IS_NPC (ch) ? ch->default_pos : POS_STANDING;
    update_pos (ch);
}

void stop_fighting (CHAR_DATA * ch, bool fBoth) {
    if (!fBoth)
        stop_fighting_one(ch);
    else {
        CHAR_DATA *fch;
        for (fch = char_list; fch != NULL; fch = fch->next)
            if (fch == ch || (fBoth && fch->fighting == ch))
                stop_fighting_one (fch);
    }
}

/* Make a corpse out of a character. */
void make_corpse (CHAR_DATA * ch) {
    char buf[MAX_STRING_LENGTH];
    OBJ_DATA *corpse;
    OBJ_DATA *obj;
    OBJ_DATA *obj_next;
    char *name;

    if (IS_NPC (ch)) {
        name = ch->short_descr;
        corpse = obj_create (get_obj_index (OBJ_VNUM_CORPSE_NPC), 0);
        corpse->timer = number_range (3, 6);
        if (ch->gold > 0) {
            obj_to_obj (obj_create_money (ch->gold, ch->silver), corpse);
            ch->gold = 0;
            ch->silver = 0;
        }
        corpse->cost = 0;
    }
    else {
        name = ch->name;
        corpse = obj_create (get_obj_index (OBJ_VNUM_CORPSE_PC), 0);
        corpse->timer = number_range (25, 40);
        REMOVE_BIT (ch->plr, PLR_CANLOOT);
        if (!char_has_clan (ch))
            corpse->owner = str_dup (ch->name);
        else {
            corpse->owner = NULL;
            if (ch->gold > 1 || ch->silver > 1) {
                obj_to_obj (obj_create_money (ch->gold / 2, ch->silver / 2),
                            corpse);
                ch->gold -= ch->gold / 2;
                ch->silver -= ch->silver / 2;
            }
        }
        corpse->cost = 0;
    }

    corpse->level = ch->level;

    sprintf (buf, corpse->short_descr, name);
    str_free (corpse->short_descr);
    corpse->short_descr = str_dup (buf);

    sprintf (buf, corpse->description, name);
    str_free (corpse->description);
    corpse->description = str_dup (buf);

    for (obj = ch->carrying; obj != NULL; obj = obj_next) {
        bool floating = FALSE;

        obj_next = obj->next_content;
        if (obj->wear_loc == WEAR_FLOAT)
            floating = TRUE;
        obj_from_char (obj);
        if (obj->item_type == ITEM_POTION)
            obj->timer = number_range (500, 1000);
        if (obj->item_type == ITEM_SCROLL)
            obj->timer = number_range (1000, 2500);
        if (IS_SET (obj->extra_flags, ITEM_ROT_DEATH) && !floating) {
            obj->timer = number_range (5, 10);
            REMOVE_BIT (obj->extra_flags, ITEM_ROT_DEATH);
        }
        REMOVE_BIT (obj->extra_flags, ITEM_VIS_DEATH);

        if (IS_SET (obj->extra_flags, ITEM_INVENTORY))
            obj_extract (obj);
        else if (floating) {
            if (IS_OBJ_STAT (obj, ITEM_ROT_DEATH)) { /* get rid of it! */
                if (obj->contains != NULL) {
                    OBJ_DATA *in, *in_next;

                    act ("$p evaporates, scattering its contents.",
                         ch, obj, NULL, TO_NOTCHAR);
                    for (in = obj->contains; in != NULL; in = in_next) {
                        in_next = in->next_content;
                        obj_from_obj (in);
                        obj_to_room (in, ch->in_room);
                    }
                }
                else
                    act ("$p evaporates.", ch, obj, NULL, TO_NOTCHAR);
                obj_extract (obj);
            }
            else {
                act ("$p falls to the floor.", ch, obj, NULL, TO_NOTCHAR);
                obj_to_room (obj, ch->in_room);
            }
        }
        else
            obj_to_obj (obj, corpse);
    }

    obj_to_room (corpse, ch->in_room);
}

/* Improved Death_cry contributed by Diavolo. */
void death_cry (CHAR_DATA * ch) {
    ROOM_INDEX_DATA *was_in_room;
    char *msg;
    int door;
    int vnum;

    vnum = 0;
    msg = "You hear $n's death cry.";

    switch (number_bits (4)) {
        case 0:
            msg = "$n hits the ground ... DEAD.";
            break;
        case 1:
            if (char_get_eq_by_wear_loc (ch, WEAR_BODY))
                msg = "$n splatters blood on your armor.";
            break;
        case 2:
            if (IS_SET (ch->parts, PART_GUTS)) {
                msg = "$n spills $s guts all over the floor.";
                vnum = OBJ_VNUM_GUTS;
            }
            break;
        case 3:
            if (IS_SET (ch->parts, PART_HEAD)) {
                msg = "$n's severed head plops on the ground.";
                vnum = OBJ_VNUM_SEVERED_HEAD;
            }
            break;
        case 4:
            if (IS_SET (ch->parts, PART_HEART)) {
                msg = "$n's heart is torn from $s chest.";
                vnum = OBJ_VNUM_TORN_HEART;
            }
            break;
        case 5:
            if (IS_SET (ch->parts, PART_ARMS)) {
                msg = "$n's arm is sliced from $s dead body.";
                vnum = OBJ_VNUM_SLICED_ARM;
            }
            break;
        case 6:
            if (IS_SET (ch->parts, PART_LEGS)) {
                msg = "$n's leg is sliced from $s dead body.";
                vnum = OBJ_VNUM_SLICED_LEG;
            }
            break;
        case 7:
            if (IS_SET (ch->parts, PART_BRAINS)) {
                msg = "$n's head is shattered, and $s brains splash all over you.";
                vnum = OBJ_VNUM_BRAINS;
            }
            break;
    }
    act (msg, ch, NULL, NULL, TO_NOTCHAR);

    if (vnum != 0) {
        char buf[MAX_STRING_LENGTH];
        OBJ_DATA *obj;
        char *name;

        name = PERS (ch);
        obj = obj_create (get_obj_index (vnum), 0);
        obj->timer = number_range (4, 7);

        sprintf (buf, obj->short_descr, name);
        str_free (obj->short_descr);
        obj->short_descr = str_dup (buf);

        sprintf (buf, obj->description, name);
        str_free (obj->description);
        obj->description = str_dup (buf);

        if (obj->item_type == ITEM_FOOD) {
            if (IS_SET (ch->form, FORM_POISON))
                obj->v.food.poisoned = 1;
            else if (!IS_SET (ch->form, FORM_EDIBLE))
                obj->item_type = ITEM_TRASH;
        }
        obj_to_room (obj, ch->in_room);
    }

    if (IS_NPC (ch))
        msg = "You hear something's death cry.";
    else
        msg = "You hear someone's death cry.";

    was_in_room = ch->in_room;
    for (door = 0; door <= 5; door++) {
        EXIT_DATA *pexit;

        if ((pexit = was_in_room->exit[door]) != NULL
            && pexit->to_room != NULL && pexit->to_room != was_in_room)
        {
            ch->in_room = pexit->to_room;
            act (msg, ch, NULL, NULL, TO_NOTCHAR);
        }
    }
    ch->in_room = was_in_room;
}

void raw_kill (CHAR_DATA * victim) {
    int i;

    stop_fighting (victim, TRUE);
    death_cry (victim);
    make_corpse (victim);

    if (IS_NPC (victim)) {
        victim->pIndexData->killed++;
        kill_table[URANGE (0, victim->level, MAX_LEVEL - 1)].killed++;
        char_extract (victim, TRUE);
        return;
    }

    char_extract (victim, FALSE);
    while (victim->affected)
        affect_remove (victim, victim->affected);
    victim->affected_by = race_table[victim->race].aff;
    for (i = 0; i < 4; i++)
        victim->armor[i] = 100;
    victim->position = POS_RESTING;
    victim->hit = UMAX (1, victim->hit);
    victim->mana = UMAX (1, victim->mana);
    victim->move = UMAX (1, victim->move);

    /* we're stable enough to not need this :) */
    /* save_char_obj (victim); */
}

void group_gain (CHAR_DATA * ch, CHAR_DATA * victim) {
    CHAR_DATA *gch;
 /* CHAR_DATA *lch; */
    int xp;
    int members;
    int group_levels;

    /* Monsters don't get kill xp's or alignment changes.
     * P-killing doesn't help either.
     * Dying of mortal wounds or poison doesn't give xp to anyone! */
    if (victim == ch)
        return;

    members = 0;
    group_levels = 0;
    for (gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room) {
        if (is_same_group (gch, ch)) {
            members++;
            group_levels += IS_NPC (gch) ? gch->level / 2 : gch->level;
        }
    }

    if (members == 0) {
        bug ("group_gain: members.", members);
        members = 1;
        group_levels = ch->level;
    }

 /* lch = (ch->leader != NULL) ? ch->leader : ch; */
    for (gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room) {
        OBJ_DATA *obj;
        OBJ_DATA *obj_next;
        if (!is_same_group (gch, ch) || IS_NPC (gch))
            continue;

        /* Taken out, add it back if you want it */
#if 0
        if (gch->level - lch->level >= 5) {
            send_to_char ("You are too high for this group.\n\r", gch);
            continue;
        }

        if (gch->level - lch->level <= -5) {
            send_to_char ("You are too low for this group.\n\r", gch);
            continue;
        }
#endif

        xp = xp_compute (gch, victim, group_levels);
        printf_to_char (gch, "You receive %d experience points.\n\r", xp);
        gain_exp (gch, xp);

        for (obj = ch->carrying; obj != NULL; obj = obj_next) {
            obj_next = obj->next_content;
            if (obj->wear_loc == WEAR_NONE)
                continue;

            if ((IS_OBJ_STAT (obj, ITEM_ANTI_EVIL)    && IS_EVIL (ch)) ||
                (IS_OBJ_STAT (obj, ITEM_ANTI_GOOD)    && IS_GOOD (ch)) ||
                (IS_OBJ_STAT (obj, ITEM_ANTI_NEUTRAL) && IS_NEUTRAL (ch)))
            {
                act ("You are zapped by $p.", ch, obj, NULL, TO_CHAR);
                act ("$n is zapped by $p.", ch, obj, NULL, TO_NOTCHAR);
                obj_from_char (obj);
                obj_to_room (obj, ch->in_room);
            }
        }
    }
}

/* Compute xp for a kill.
 * Also adjust alignment of killer.
 * Edit this function to change xp computations. */
int xp_compute (CHAR_DATA * gch, CHAR_DATA * victim, int total_levels) {
    int xp, base_exp;
    int align, level_range;
    int change;
 // int time_per_level;

    /* compute the base exp */
    level_range = victim->level - gch->level;
    switch (level_range) {
        case -10: base_exp = 1;   break;
        case  -9: base_exp = 2;   break;
        case  -8: base_exp = 5;   break;
        case  -7: base_exp = 8;   break;
        case  -6: base_exp = 11;  break;
        case  -5: base_exp = 22;  break;
        case  -4: base_exp = 33;  break;
        case  -3: base_exp = 50;  break;
        case  -2: base_exp = 66;  break;
        case  -1: base_exp = 83;  break;
        case   0: base_exp = 100; break;
        default:
            base_exp = (level_range < -10) ? 0 : 100 + 20 * level_range;
    }

    /* do alignment computations */
    align = victim->alignment - gch->alignment;

    /* no change */
    if (IS_SET (victim->mob, MOB_NOALIGN))
        ;
    /* monster is more good than slayer */
    else if (align > 500) {
        change = (align - 500) * base_exp / 500 * gch->level / total_levels;
        change = UMAX (1, change);
        gch->alignment = UMAX (-1000, gch->alignment - change);
    }
    /* monster is more evil than slayer */
    else if (align < -500) {
        change = (-1 * align - 500) * base_exp / 500 * gch->level / total_levels;
        change = UMAX (1, change);
        gch->alignment = UMIN (1000, gch->alignment + change);
    }
    /* improve this someday */
    else {
        change = gch->alignment * base_exp / 500 * gch->level / total_levels;
        gch->alignment -= change;
    }

    /* calculate exp multiplier */
    if (IS_SET (victim->mob, MOB_NOALIGN))
        xp = base_exp;
    /* for goodie two shoes */
    else if (gch->alignment > 500) {
        /* a LOT more for evil */
             if (victim->alignment < -750) xp = (base_exp * 4) / 3;
        else if (victim->alignment < -500) xp = (base_exp * 5) / 4;
        /* a LOT less for good */
        else if (victim->alignment >  750) xp = (base_exp * 1) / 4;
        else if (victim->alignment >  500) xp = (base_exp * 1) / 2;
        else if (victim->alignment >  250) xp = (base_exp * 3) / 4;
        /* don't care */
        else                               xp = (base_exp);
    }
    /* a little good */
    else if (gch->alignment > 200) {
        /* should kill evil */
             if (victim->alignment < -500) xp = (base_exp * 6) / 5;
        /* shouldn't kill good */
        else if (victim->alignment >  750) xp = (base_exp * 1) / 2;
        else if (victim->alignment >    0) xp = (base_exp * 3) / 4;
        /* don't care */
        else                               xp = (base_exp);
    }
    /* for baddies */
    else if (gch->alignment < -500) {
        /* a bit more for good */
             if (victim->alignment >  750) xp = (base_exp * 5) / 4;
        else if (victim->alignment >  500) xp = (base_exp * 11) / 10;
        /* a bit less for evil */
        else if (victim->alignment < -750) xp = (base_exp / 2);
        else if (victim->alignment < -500) xp = (base_exp * 3) / 4;
        else if (victim->alignment < -250) xp = (base_exp * 9) / 10;
        /* don't care */
        else                               xp = (base_exp);
    }
    /* a little bad */
    else if (gch->alignment < -200) {
        if (victim->alignment > 500) xp = (base_exp * 6) / 5;
        else if (victim->alignment < -750) xp = base_exp / 2;
        else if (victim->alignment < 0) xp = (base_exp * 3) / 4;
        /* don't care */
        else xp = base_exp;
    }
    /* neutral */
    else {
        /* a little more for extreme alignments */
        if (victim->alignment > 500 || victim->alignment < -500)
            xp = (base_exp * 4) / 3;
        /* a little less for fellow neutrals */
        else if (victim->alignment < 200 && victim->alignment > -200)
            xp = (base_exp * 2) / 3;
        /* don't care */
        else
            xp = base_exp;
    }

    /* more exp at the low levels */
    if (gch->level < 6)
        xp = 10 * xp / (gch->level + 4);

    /* less at high */
    if (gch->level > 35)
        xp = 15 * xp / (gch->level - 25);

    /* HOW DARE YOU!! */
#if 0
    /* reduce for playing time */
    {
        /* compute quarter-hours per level */
        time_per_level = 4 *
            (gch->played + (int) (current_time - gch->logon)) / 3600
            / gch->level;
        time_per_level = URANGE (2, time_per_level, 12);
        if (gch->level < 15)    /* make it a curve */
            time_per_level = UMAX (time_per_level, (15 - gch->level));
        xp = xp * time_per_level / 12;
    }
#endif

    /* randomize the rewards */
    xp = number_range (xp * 3 / 4, xp * 5 / 4);

    /* adjust for grouping */
    xp = xp * gch->level / (UMAX (1, total_levels - 1));
    return xp;
}

void dam_message (CHAR_DATA * ch, CHAR_DATA * victim, int dam, int dt,
                  bool immune, int orig_dam)
{
    char buf1[256], buf2[256], buf3[256], buf_hit[256];
    const char *vs;
    const char *vp;
    const char *attack;
    const char *str;
    char punct;
    int dam_percent = ((100 * dam) / victim->max_hit);

    if (ch == NULL || victim == NULL)
        return;

    /* Determine strength from overall damage. */
#ifndef VANILLA
         if (orig_dam ==   0) str = "";
    else if (orig_dam ==   1) str = "pathetic ";
    else if (orig_dam <=   2) str = "wimpy ";
    else if (orig_dam <=   3) str = "weak ";
    else if (orig_dam <=   5) str = "mediocre ";
    else if (orig_dam <=   8) str = "decent ";
    else if (orig_dam <=  13) str = "good ";
    else if (orig_dam <=  21) str = "great ";
    else if (orig_dam <=  34) str = "excellent ";
    else if (orig_dam <=  55) str = "magnificent ";
    else if (orig_dam <=  89) str = "epic ";
    else if (orig_dam <= 144) str = "legendary ";
    else                      str = "godly ";
#else
    str = "";
#endif

    /* Determine strength from damage percent. */
#ifdef VANILLA
         if (dam         == 0)  { vs = "miss";               vp = "misses"; }
    else if (dam_percent <= 5)  { vs = "scratch";            vp = "scratches"; }
    else if (dam_percent <= 10) { vs = "graze";              vp = "grazes"; }
    else if (dam_percent <= 15) { vs = "hit";                vp = "hits"; }
#else
         if (dam         == 0)  { vs = "miss";               vp = "misses"; }
    else if (dam_percent <= 3)  { vs = "scratch";            vp = "scratches"; }
    else if (dam_percent <= 6)  { vs = "scrape";             vp = "scrapes"; }
    else if (dam_percent <= 10) { vs = "graze";              vp = "grazes"; }
    else if (dam_percent <= 15) { vs = "bruise";             vp = "bruises"; }
#endif
    else if (dam_percent <= 20) { vs = "injure";             vp = "injures"; }
    else if (dam_percent <= 25) { vs = "wound";              vp = "wounds"; }
    else if (dam_percent <= 30) { vs = "maul";               vp = "mauls"; }
    else if (dam_percent <= 35) { vs = "decimate";           vp = "decimates"; }
    else if (dam_percent <= 40) { vs = "devastate";          vp = "devastates"; }
    else if (dam_percent <= 45) { vs = "maim";               vp = "maims"; }
    else if (dam_percent <= 50) { vs = "MUTILATE";           vp = "MUTILATES"; }
    else if (dam_percent <= 55) { vs = "DISEMBOWEL";         vp = "DISEMBOWELS"; }
    else if (dam_percent <= 60) { vs = "DISMEMBER";          vp = "DISMEMBERS"; }
    else if (dam_percent <= 65) { vs = "MASSACRE";           vp = "MASSACRES"; }
    else if (dam_percent <= 70) { vs = "MANGLE";             vp = "MANGLES"; }
    else if (dam_percent <= 75) { vs = "*** DEMOLISH ***";   vp = "*** DEMOLISHES ***"; }
    else if (dam_percent <= 80) { vs = "*** DEVASTATE ***";  vp = "*** DEVASTATES ***"; }
    else if (dam_percent <= 85) { vs = "=== OBLITERATE ==="; vp = "=== OBLITERATES ==="; }
    else if (dam_percent <= 90) { vs = ">>> ANNIHILATE <<<"; vp = ">>> ANNIHILATES <<<"; }
    else if (dam_percent <= 95) { vs = "<<< ERADICATE >>>";  vp = "<<< ERADICATES >>>"; }
#ifndef VANILLA
    else if (dam_percent < 100) { vs = "### DESTROY ###";    vp = "### DESTROYS ###"; }
#endif
    else { vs = "do UNSPEAKABLE things to"; vp = "does UNSPEAKABLE things to"; }

    punct = (dam_percent <= 45) ? '.' : '!';

    if (dt == TYPE_HIT) {
        if (damage_adj == NULL)
            strcpy (buf_hit, "hit");
        else
            sprintf (buf_hit, "%s hit", damage_adj);
        attack = buf_hit;

        if (ch == victim) {
            sprintf (buf1, "{2You %s yourself with your %s%s%c{x", vs, str, attack, punct);
            sprintf (buf3, "{3$n %s $melf with $s %s%s%c{x", vp, str, attack, punct);
        }
        else {
            sprintf (buf1, "{2You %s $N with your %s%s%c{x", vs, str, attack, punct);
            sprintf (buf2, "{4$n %s you with $s %s%s%c{x", vp, str, attack, punct);
            sprintf (buf3, "{3$n %s $N with $s %s%s%c{x", vp, str, attack, punct);
        }
    }
    else {
        if (dt >= 0 && dt < SKILL_MAX)
            attack = skill_table[dt].noun_damage;
        else if (dt >= TYPE_HIT && dt < TYPE_HIT + ATTACK_MAX)
            attack = attack_table[dt - TYPE_HIT].noun;
        else {
            bug ("dam_message: bad dt %d.", dt);
            dt = TYPE_HIT;
            attack = attack_table[0].name;
        }
        if (damage_adj != NULL) {
            sprintf (buf_hit, "%s %s", damage_adj, attack);
            attack = buf_hit;
        }

        if (immune) {
            if (ch == victim) {
                sprintf (buf1, "{2Luckily, you are immune to that.{x");
                sprintf (buf3, "{3$n is unaffected by $s own %s.{x", attack);
            }
            else {
                sprintf (buf1, "{2$N is unaffected by your %s!{x", attack);
                sprintf (buf2, "{4$n's %s is powerless against you.{x",
                         attack);
                sprintf (buf3, "{3$N is unaffected by $n's %s!{x", attack);
            }
        }
        else {
            if (ch == victim) {
                sprintf (buf1, "{2Your %s%s %s you%c{x", str, attack, vp, punct);
                sprintf (buf3, "{3$n's %s%s %s $m%c{x", str, attack, vp, punct);
            }
            else {
                sprintf (buf1, "{2Your %s%s %s $N%c{x", str, attack, vp, punct);
                sprintf (buf2, "{4$n's %s%s %s you%c{x", str, attack, vp, punct);
                sprintf (buf3, "{3$n's %s%s %s $N%c{x", str, attack, vp, punct);
            }
        }
    }

    if (ch == victim) {
        act (buf1, ch, NULL, NULL, TO_CHAR);
        act (buf3, ch, NULL, NULL, TO_NOTCHAR);
    }
    else {
        act (buf1, ch, NULL, victim, TO_CHAR);
        act (buf2, ch, NULL, victim, TO_VICT);
        act (buf3, ch, NULL, victim, TO_OTHERS);
    }
    damage_adj = NULL;
}

/* Disarm a creature.
 * Caller must check for successful attack. */
void disarm (CHAR_DATA * ch, CHAR_DATA * victim) {
    OBJ_DATA *obj;

    if ((obj = char_get_eq_by_wear_loc (victim, WEAR_WIELD)) == NULL)
        return;

    if (IS_OBJ_STAT (obj, ITEM_NOREMOVE)) {
        act ("{5$S weapon won't budge!{x", ch, NULL, victim, TO_CHAR);
        act ("{5$n tries to disarm you, but your weapon won't budge!{x",
             ch, NULL, victim, TO_VICT);
        act ("{5$n tries to disarm $N, but fails.{x", ch, NULL, victim,
             TO_OTHERS);
        return;
    }

    act ("{5$n DISARMS you and sends your weapon flying!{x",
         ch, NULL, victim, TO_VICT);
    act ("{5You disarm $N!{x", ch, NULL, victim, TO_CHAR);
    act ("{5$n disarms $N!{x", ch, NULL, victim, TO_OTHERS);

    obj_from_char (obj);
    if (IS_OBJ_STAT (obj, ITEM_NODROP) || IS_OBJ_STAT (obj, ITEM_INVENTORY))
        obj_to_char (obj, victim);
    else {
        obj_to_room (obj, victim->in_room);
        if (IS_NPC (victim) && victim->wait == 0 && char_can_see_obj (victim, obj))
            char_take_obj (victim, obj, NULL);
    }
}

int get_exp_to_level (CHAR_DATA * ch) {
    if (IS_NPC (ch) || ch->level >= LEVEL_HERO)
        return 1000;
    return (ch->level + 1) * exp_per_level (ch, ch->pcdata->points) - ch->exp;
}

int exp_per_level (CHAR_DATA * ch, int points) {
    int expl, inc;

    if (IS_NPC (ch))
        return 1000;

    expl = 1000;
    inc = 500;

    if (points < 40)
        return 1000 * (pc_race_table[ch->race].class_mult[ch->class] ?
                       pc_race_table[ch->race].class_mult[ch->class] /
                       100 : 1);

    /* processing */
    points -= 40;

    while (points > 9) {
        expl += inc;
        points -= 10;
        if (points > 9) {
            expl += inc;
            inc *= 2;
            points -= 10;
        }
    }

    expl += points * inc / 10;
    return expl * pc_race_table[ch->race].class_mult[ch->class] / 100;
}
