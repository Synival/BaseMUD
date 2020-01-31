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

#include <string.h>

#include "interp.h"
#include "lookup.h"
#include "update.h"
#include "utils.h"
#include "db.h"
#include "effects.h"
#include "mob_prog.h"
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
#include "globals.h"
#include "memory.h"
#include "items.h"
#include "players.h"
#include "mobiles.h"

#include "fight.h"

int should_assist_group (CHAR_T *bystander, CHAR_T *attacker, CHAR_T *victim) {
    if (!is_same_group (bystander, attacker))
        return 0;
    if (is_same_group (bystander, victim))
        return 0;
    if (do_filter_can_attack (bystander, victim))
        return 0;
    if (!IS_NPC (bystander) && EXT_IS_SET (bystander->ext_plr, PLR_AUTOASSIST))
        return 1;
    if (IS_AFFECTED (bystander, AFF_CHARM))
        return 1;
    return 0;
}

CHAR_T *random_group_target_in_room (CHAR_T *bystander, CHAR_T *ch) {
    CHAR_T *vch;
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
void check_assist (CHAR_T *ch, CHAR_T *victim) {
    CHAR_T *rch, *rch_next;

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
                multi_hit (rch, victim, ATTACK_DEFAULT);
            continue;
        }

        /* quick check for ASSIST_PLAYER */
        if (mobile_should_assist_player (rch, ch, victim)) {
            do_function (rch, &do_emote, "screams and attacks!");
            multi_hit (rch, victim, ATTACK_DEFAULT);
            continue;
        }

        /* 25% chance to randomly assist attackers. */
        if (number_percent() < 25 &&
            mobile_should_assist_attacker (rch, ch, victim))
        {
            CHAR_T *target = random_group_target_in_room (rch, victim);
            if (target == NULL)
                continue;

            /* attack! */
            do_function (rch, &do_emote, "screams and attacks!");
            multi_hit (rch, target, ATTACK_DEFAULT);
        }
    }
}

/* Do one group of attacks. */
void multi_hit (CHAR_T *ch, CHAR_T *victim, int dt) {
    int chance;
    set_fighting_position_if_possible(ch);

    /* no sleep-fighting! */
    if (ch->position <= POS_SLEEPING)
        return;

    if (IS_NPC (ch)) {
        mobile_hit (ch, victim, dt);
        return;
    }

    one_hit (ch, victim, dt);
    if (ch->fighting != victim)
        return;

    if (IS_AFFECTED (ch, AFF_HASTE))
        one_hit (ch, victim, dt);
    if (ch->fighting != victim || dt == SN(BACKSTAB))
        return;

    chance = char_get_skill (ch, SN(SECOND_ATTACK)) / 2;
    if (IS_AFFECTED (ch, AFF_SLOW))
        chance /= 2;

    if (number_percent () < chance) {
        one_hit (ch, victim, dt);
        player_try_skill_improve (ch, SN(SECOND_ATTACK), TRUE, 5);
        if (ch->fighting != victim)
            return;
    }

    chance = char_get_skill (ch, SN(THIRD_ATTACK)) / 4;
    if (IS_AFFECTED (ch, AFF_SLOW))
        chance = 0;;

    if (number_percent () < chance) {
        one_hit (ch, victim, dt);
        player_try_skill_improve (ch, SN(THIRD_ATTACK), TRUE, 6);
        if (ch->fighting != victim)
            return;
    }
}

/* Hit one guy once. */
void one_hit (CHAR_T *ch, CHAR_T *victim, int dt) {
    OBJ_T *wield;
    char *damage_adj = NULL;
    int victim_ac, thac0, thac0_00, thac0_32;
    int dam, dam_type, diceroll, missed, fight_attack;
    int sn, skill, skill_val;
    bool result;

    sn = -1;

    /* just in case */
    if (victim == ch || ch == NULL || victim == NULL)
        return;

    /* Can't beat a dead char!
     * Guard against weird room-leavings. */
    if (victim->position == POS_DEAD || ch->in_room != victim->in_room)
        return;

    /* Determine if there is a weapon in use. */
    wield = char_get_weapon (ch);
    fight_attack = (wield) ? wield->v.weapon.attack_type : ch->attack_type;

    /* If the attack is 'ATTACK_DEFAULT', use a non-skill 'fight' attack with
     * the character's weapon or, if not available, the innate attack. */
    if (dt == ATTACK_DEFAULT) {
        dt = ATTACK_FIGHTING + fight_attack;
        dam_type = attack_table[fight_attack].dam_type;
    }
    /* If the attack is an explicit type (slash, pierce, etc) and a non-skill
     * (greater than or equal to ATTACK_FIGHTING), use the explicitly-defined
     * damage type in the attack table. */
    else if (dt >= ATTACK_FIGHTING)
        dam_type = attack_table[dt - ATTACK_FIGHTING].dam_type;
    /* If the attack is a skill, use the character's weapon/innate attack to
     * determine the type of damage. */
    else
        dam_type = attack_table[fight_attack].dam_type;

    /* Default to bashing if no damage type is available. */
    if (dam_type < 0)
        dam_type = DAM_BASH;

    /* get the weapon skill */
    sn = char_get_weapon_sn (ch);
    skill = 20 + char_get_weapon_skill (ch, sn);

    /* Calculate to-hit-armor-class-0 versus armor. */
    if (IS_NPC (ch)) {
        thac0_00 = 20;
        thac0_32 = -4; /* as good as a thief */
             if (EXT_IS_SET (ch->ext_mob, MOB_WARRIOR)) thac0_32 = -10;
        else if (EXT_IS_SET (ch->ext_mob, MOB_THIEF))   thac0_32 =  -4;
        else if (EXT_IS_SET (ch->ext_mob, MOB_CLERIC))  thac0_32 =   2;
        else if (EXT_IS_SET (ch->ext_mob, MOB_MAGE))    thac0_32 =   6;
    }
    else {
        thac0_00 = class_table[ch->class].thac0_00;
        thac0_32 = class_table[ch->class].thac0_32;
    }
    thac0 = int_interpolate (ch->level, thac0_00, thac0_32);

    if (thac0 < 0)
        thac0 = thac0 / 2;
    if (thac0 < -5)
        thac0 = -5 + (thac0 + 5) / 2;

    thac0 -= GET_HITROLL (ch) * skill / 100;
    thac0 += 5 * (100 - skill) / 100;

    if (dt == SN(BACKSTAB))
        thac0 -= 10 * (100 - char_get_skill (ch, SN(BACKSTAB)));

    /* TODO: damage type specify what kind of AC should be used. */
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

    /* TODO: position can determine how much AC is added. */
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
    if (IS_NPC (ch) && (!ch->index_data->new_format || wield == NULL)) {
        if (!ch->index_data->new_format) {
            dam = number_range (ch->level / 2, ch->level * 3 / 2);
            if (wield != NULL)
                dam += dam / 2;
        }
        else
            dam = dice (ch->damage.number, ch->damage.size);
    }
    else {
        if (sn != -1)
            player_try_skill_improve (ch, sn, TRUE, 5);
        if (wield != NULL) {
            if (wield->index_data->new_format)
                dam = dice (wield->v.weapon.dice_num,
                            wield->v.weapon.dice_size) * skill / 100;
            else
                dam = number_range (wield->v.weapon.dice_num  * skill / 100,
                                    wield->v.weapon.dice_size * skill / 100);

            if (char_get_eq_by_wear_loc (ch, WEAR_LOC_SHIELD) == NULL)    /* no shield = more */
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
    skill_val = char_get_skill (ch, SN(ENHANCED_DAMAGE)) * 2 / 3;
    if (skill_val > 0 && number_percent() < skill_val) {
#ifdef BASEMUD_SHOW_ENHANCED_DAMAGE
        damage_adj = "heavy";
#endif
        player_try_skill_improve (ch, SN(ENHANCED_DAMAGE), TRUE, 6);
        dam += (dam * 3) / 4;
    }

    if (!IS_AWAKE (victim))
        dam *= 2;
    else if (victim->position < POS_FIGHTING)
        dam = dam * 3 / 2;

    if (dt == SN(BACKSTAB) && wield != NULL) {
        if (wield->v.weapon.weapon_type != WEAPON_DAGGER)
            dam *= 2 + (ch->level / 10);
        else
            dam *= 2 + (ch->level / 8);
    }

    dam += GET_DAMROLL (ch) * UMIN (100, skill) / 100;
    if (dam <= 0)
        dam = 1;

    if (missed) {
        damage_quiet (ch, victim, 0, dt, dam_type);
        dam_message (ch, victim, 0, dt, FALSE, dam, damage_adj);
        return;
    }
    result = damage_visible (ch, victim, dam, dt, dam_type, damage_adj);

    /* but do we have a funky weapon? */
    if (result && wield != NULL) {
        int dam;
        if (ch->fighting == victim && IS_WEAPON_STAT (wield, WEAPON_POISON)) {
            int level;
            AFFECT_T *poison, af;

            if ((poison = affect_find (wield->affected, SN(POISON))) == NULL)
                level = wield->level;
            else
                level = poison->level;

            if (!saves_spell (level / 2, victim, DAM_POISON)) {
                send_to_char ("You feel poison coursing through your veins.",
                              victim);
                act ("$n is poisoned by the venom on $p.",
                     victim, wield, NULL, TO_NOTCHAR);

                affect_init (&af, AFF_TO_AFFECTS, SN(POISON), level * 3 / 4, level / 2, APPLY_STR, -1, AFF_POISON);
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

        /* TODO: use act2 */
        if (ch->fighting == victim) {
            if (IS_WEAPON_STAT (wield, WEAPON_VAMPIRIC)) {
                dam = number_range (1, wield->level / 5 + 1);
                act ("You feel $p drawing your life away.",
                     victim, wield, NULL, TO_CHAR);
                act ("$p draws life from $n.", victim, wield, NULL, TO_NOTCHAR);
                damage_quiet (ch, victim, dam, 0, DAM_NEGATIVE);
                ch->alignment = UMAX (-1000, ch->alignment - 1);
                ch->hit += dam / 2;
            }

            /* TODO: use act2 */
            if (IS_WEAPON_STAT (wield, WEAPON_FLAMING)) {
                dam = number_range (1, wield->level / 4 + 1);
                act ("$p sears your flesh.", victim, wield, NULL, TO_CHAR);
                act ("$n is burned by $p.", victim, wield, NULL, TO_NOTCHAR);
                effect_fire ((void *) victim, wield->level / 2, dam, TARGET_CHAR);
                damage_quiet (ch, victim, dam, 0, DAM_FIRE);
            }

            /* TODO: use act2 */
            if (IS_WEAPON_STAT (wield, WEAPON_FROST)) {
                dam = number_range (1, wield->level / 6 + 2);
                act ("The cold touch of $p surrounds you with ice.",
                     victim, wield, NULL, TO_CHAR);
                act ("$p freezes $n.", victim, wield, NULL, TO_NOTCHAR);
                effect_cold (victim, wield->level / 2, dam, TARGET_CHAR);
                damage_quiet (ch, victim, dam, 0, DAM_COLD);
            }

            /* TODO: use act2 */
            if (IS_WEAPON_STAT (wield, WEAPON_SHOCKING)) {
                dam = number_range (1, wield->level / 5 + 2);
                act ("You are shocked by $p.", victim, wield, NULL, TO_CHAR);
                act ("$n is struck by lightning from $p.", victim, wield, NULL,
                     TO_NOTCHAR);
                effect_shock (victim, wield->level / 2, dam, TARGET_CHAR);
                damage_quiet (ch, victim, dam, 0, DAM_LIGHTNING);
            }
        }
    }

    tail_chain ();
}

/* Inflict damage from a hit. */
bool damage_quiet (CHAR_T *ch, CHAR_T *victim, int dam, int dt, int dam_type) {
    return damage_real (ch, victim, dam, dt, dam_type, FALSE, NULL);
}

bool damage_visible (CHAR_T *ch, CHAR_T *victim, int dam, int dt, int dam_type,
    const char *damage_adj)
{
    return damage_real (ch, victim, dam, dt, dam_type, TRUE, damage_adj);
}

bool damage_real (CHAR_T *ch, CHAR_T *victim, int dam, int dt, int dam_type,
    bool show, const char *damage_adj)
{
    OBJ_T *corpse;
    bool immune;
    int orig_dam = dam, last_pos;
    int log_target;

    if (victim->position == POS_DEAD)
        return FALSE;

    /* Stop up any residual loopholes. */
    if (dam > 1200 && dt >= ATTACK_FIGHTING) {
        bug ("damage: %d: more than 1200 points!", dam);
        dam = 1200;
        if (!IS_IMMORTAL (ch)) {
            OBJ_T *obj;
            obj = char_get_eq_by_wear_loc (ch, WEAR_LOC_WIELD);
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
        affect_strip (ch, SN(INVIS));
        affect_strip (ch, SN(MASS_INVIS));
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
    if (dt >= ATTACK_FIGHTING && ch != victim) {
        if (check_parry (ch, victim))
            return FALSE;
        if (check_dodge (ch, victim))
            return FALSE;
        if (check_shield_block (ch, victim))
            return FALSE;
    }

    immune = FALSE;
    switch (char_get_immunity (victim, dam_type)) {
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
        dam_message (ch, victim, dam, dt, immune, orig_dam, damage_adj);
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
            int exp;
            log_f ("%s killed by %s at %d",
                victim->name, PERS (ch), ch->in_room->vnum);

            /* Dying penalty: 2/3 way back to previous level. */
            exp = player_get_exp_per_level (victim);
            if (victim->exp > exp * victim->level) {
                player_gain_exp (victim, (2 *
                    (exp * victim->level - victim->exp) / 3) + 50);
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

        corpse = raw_kill (victim);

        /* dump the flags */
        if (ch != victim && !IS_NPC (ch) && !player_in_same_clan (ch, victim)) {
            if (EXT_IS_SET (victim->ext_plr, PLR_KILLER))
                EXT_UNSET (victim->ext_plr, PLR_KILLER);
            else
                EXT_UNSET (victim->ext_plr, PLR_THIEF);
        }

        /* RT new auto commands */
        if (!IS_NPC (ch) && corpse != NULL &&
             item_can_loot_as (corpse, ch) &&
             corpse->item_type != ITEM_CORPSE_PC &&
             char_can_see_obj (ch, corpse))
        {
            OBJ_T *coins;

            if (corpse->contains) {
                if (EXT_IS_SET (ch->ext_plr, PLR_AUTOLOOT))
                    do_function (ch, &do_get, "all corpse");
                else if (EXT_IS_SET (ch->ext_plr, PLR_AUTOGOLD)) {
                    if ((coins = find_obj_container (ch, corpse, "gcash")) != NULL)
                        do_function (ch, &do_get, "all.gcash corpse");
                }
            }

            if (EXT_IS_SET (ch->ext_plr, PLR_AUTOSAC)) {
                /* don't sacrifice if we intend to loot but could not. */
                if (!(EXT_IS_SET (ch->ext_plr, PLR_AUTOLOOT) && corpse->contains))
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
        if ((EXT_IS_SET (victim->ext_mob, MOB_WIMPY) && number_bits (2) == 0
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

bool set_fighting_position_if_possible (CHAR_T *ch) {
    if (ch->timer > 4)
        return FALSE;
    else if (ch->position < POS_FIGHTING && (ch->daze > 0 || ch->wait > 0))
        return FALSE;
    else if (ch->fighting == NULL)
        return FALSE;
    if (ch->position != POS_FIGHTING) {
        position_change_send_message (ch, ch->position, POS_FIGHTING, ch->on);
        ch->position = POS_FIGHTING;
    }
    return TRUE;
}

bool do_filter_can_attack (CHAR_T *ch, CHAR_T *victim)
    { return do_filter_can_attack_real (ch, victim, FALSE, FALSE); }
bool do_filter_can_attack_spell (CHAR_T *ch, CHAR_T *victim, bool area)
    { return do_filter_can_attack_real (ch, victim, area, FALSE); }

bool can_attack (CHAR_T *ch, CHAR_T *victim) {
    return do_filter_can_attack_real (ch, victim, FALSE, TRUE) ? FALSE : TRUE;
}
bool can_attack_spell (CHAR_T *ch, CHAR_T *victim, bool area) {
    return do_filter_can_attack_real (ch, victim, area, TRUE) ? FALSE : TRUE;
}

bool do_filter_can_attack_real (CHAR_T *ch, CHAR_T *victim, bool area,
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
        FILTER (victim->index_data->shop != NULL,
            QU("The shopkeeper wouldn't like that.\n\r"), ch);
        FILTER (mobile_is_friendly (victim),
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
            FILTER (!player_has_clan (ch),
                QU("Join a clan if you want to kill players.\n\r"), ch);

            /* Killing undesirables is allowed. */
            if (player_is_undesirable (victim))
                return FALSE;

            FILTER (!player_has_clan (victim),
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
void check_killer (CHAR_T *ch, CHAR_T *victim) {
    /* Follow charm thread to responsible character.
     * Attacking someone's charmed char is hostile! */
    while (IS_AFFECTED (victim, AFF_CHARM) && victim->master != NULL)
        victim = victim->master;

    /* NPC's are fair game.
     * So are killers and thieves. */
    if (IS_NPC (victim) || player_is_undesirable (victim))
        return;

    /* Charm-o-rama. */
    if (IS_SET (ch->affected_by, AFF_CHARM)) {
        if (ch->master == NULL) {
            bugf ("Check_killer: %s bad AFF_CHARM", PERS (ch));
            affect_strip (ch, SN(CHARM_PERSON));
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
    if (IS_NPC (ch))
        return;
    if (ch == victim || ch->fighting == victim)
        return;
    if (ch->level >= LEVEL_IMMORTAL || !player_has_clan (ch))
        return;
    if (EXT_IS_SET (ch->ext_plr, PLR_KILLER))
        return;

    send_to_char ("*** You are now a KILLER!! ***\n\r", ch);
    EXT_SET (ch->ext_plr, PLR_KILLER);
    wiznetf (ch, NULL, WIZ_FLAGS, 0, 0,
        "$N is attempting to murder %s", victim->name);
    save_char_obj (ch);
}

/* Check for parry. */
bool check_parry (CHAR_T *ch, CHAR_T *victim) {
    int chance;

    if (!IS_AWAKE (victim))
        return FALSE;

    chance = char_get_skill (victim, SN(PARRY)) / 2;
    if (char_get_eq_by_wear_loc (victim, WEAR_LOC_WIELD) == NULL) {
        if (IS_NPC (victim))
            chance /= 2;
        else
            return FALSE;
    }
    if (!char_can_see_in_room (ch, victim))
        chance /= 2;
    if (number_percent () >= chance + victim->level - ch->level)
        return FALSE;

    /* TODO: act2 */
    act ("You parry $n's attack.", ch, NULL, victim, TO_VICT);
    act ("$N parries your attack.", ch, NULL, victim, TO_CHAR);
    player_try_skill_improve (victim, SN(PARRY), TRUE, 6);
    return TRUE;
}

/* Check for shield block. */
bool check_shield_block (CHAR_T *ch, CHAR_T *victim) {
    int chance;

    if (!IS_AWAKE (victim))
        return FALSE;
    if (char_get_eq_by_wear_loc (victim, WEAR_LOC_SHIELD) == NULL)
        return FALSE;

    chance = char_get_skill (victim, SN(SHIELD_BLOCK)) / 5 + 3;
    if (number_percent () >= chance + victim->level - ch->level)
        return FALSE;

    /* TODO: act2 */
    act ("You block $n's attack with your shield.", ch, NULL, victim, TO_VICT);
    act ("$N blocks your attack with a shield.", ch, NULL, victim, TO_CHAR);
    player_try_skill_improve (victim, SN(SHIELD_BLOCK), TRUE, 6);
    return TRUE;
}

/* Check for dodge. */
bool check_dodge (CHAR_T *ch, CHAR_T *victim) {
    int chance;
    if (!IS_AWAKE (victim))
        return FALSE;

    chance = char_get_skill (victim, SN(DODGE)) / 2;
    if (!char_can_see_in_room (victim, ch))
        chance /= 2;
    if (number_percent () >= chance + victim->level - ch->level)
        return FALSE;

    /* TODO: act2 */
    act ("You dodge $n's attack.", ch, NULL, victim, TO_VICT);
    act ("$N dodges your attack.", ch, NULL, victim, TO_CHAR);
    player_try_skill_improve (victim, SN(DODGE), TRUE, 6);
    return TRUE;
}

/* Set position of a victim. */
void update_pos (CHAR_T *victim) {
    if (victim->hit > 0) {
        if (victim->position <= POS_STUNNED)
            victim->position = POS_RESTING;
    }
    else {
        int percent = victim->hit * 100 / victim->max_hit;

#ifdef BASEMUD_ALLOW_STUNNED_MOBS
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
void set_fighting_both (CHAR_T *ch, CHAR_T *victim) {
    if (ch->fighting == NULL)
        set_fighting_one (ch, victim);
    if (victim->fighting == NULL)
        set_fighting_one (victim, ch);
}

void set_fighting_one (CHAR_T *ch, CHAR_T *victim) {
    int daze_mult = 0;

    BAIL_IF_BUG (ch->fighting != NULL,
        "set_fighting: already fighting", 0);

    if (IS_AFFECTED (ch, AFF_SLEEP))
        affect_strip (ch, SN(SLEEP));

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
void stop_fighting_one (CHAR_T *ch) {
    ch->fighting = NULL;
    if (ch->position == POS_FIGHTING)
        ch->position = IS_NPC (ch) ? ch->default_pos : POS_STANDING;
    update_pos (ch);
}

void stop_fighting (CHAR_T *ch, bool both) {
    if (!both)
        stop_fighting_one(ch);
    else {
        CHAR_T *fch;
        for (fch = char_list; fch != NULL; fch = fch->next)
            if (fch == ch || (both && fch->fighting == ch))
                stop_fighting_one (fch);
    }
}

/* Make a corpse out of a character. */
OBJ_T *make_corpse (CHAR_T *ch) {
    char buf[MAX_STRING_LENGTH];
    OBJ_T *corpse;
    OBJ_T *obj, *obj_next;
    char *name;

    if (IS_NPC (ch)) {
        name = ch->short_descr;
        corpse = obj_create (obj_get_index (OBJ_VNUM_CORPSE_NPC), 0);
        corpse->timer = number_range (3, 6);
        if (ch->gold > 0) {
            obj_give_to_obj (obj_create_money (ch->gold, ch->silver), corpse);
            ch->gold = 0;
            ch->silver = 0;
        }
        corpse->cost = 0;
    }
    else {
        name = ch->name;
        corpse = obj_create (obj_get_index (OBJ_VNUM_CORPSE_PC), 0);
        corpse->timer = number_range (25, 40);
        EXT_UNSET (ch->ext_plr, PLR_CANLOOT);
        if (!player_has_clan (ch))
            corpse->owner = str_dup (ch->name);
        else {
            corpse->owner = NULL;
            if (ch->gold > 1 || ch->silver > 1) {
                obj_give_to_obj (obj_create_money (ch->gold / 2, ch->silver / 2),
                            corpse);
                ch->gold -= ch->gold / 2;
                ch->silver -= ch->silver / 2;
            }
        }
        corpse->cost = 0;
    }

    corpse->level = ch->level;

    sprintf (buf, corpse->short_descr, name);
    str_replace_dup (&(corpse->short_descr), buf);

    sprintf (buf, corpse->description, name);
    str_replace_dup (&(corpse->description), buf);

    for (obj = ch->carrying; obj != NULL; obj = obj_next) {
        bool floating = FALSE;
        int timer_min, timer_max;

        obj_next = obj->next_content;
        if (obj->wear_loc == WEAR_LOC_FLOAT)
            floating = TRUE;
        obj_take_from_char (obj);

        if (item_get_corpse_timer_range (obj, &timer_min, &timer_max))
            obj->timer = number_range (timer_min, timer_max);
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
                    OBJ_T *in, *in_next;

                    act ("$p evaporates, scattering its contents.",
                         ch, obj, NULL, TO_NOTCHAR);
                    for (in = obj->contains; in != NULL; in = in_next) {
                        in_next = in->next_content;
                        obj_take_from_obj (in);
                        obj_give_to_room (in, ch->in_room);
                    }
                }
                else
                    act ("$p evaporates.", ch, obj, NULL, TO_NOTCHAR);
                obj_extract (obj);
            }
            else {
                act ("$p falls to the floor.", ch, obj, NULL, TO_NOTCHAR);
                obj_give_to_room (obj, ch->in_room);
            }
        }
        else
            obj_give_to_obj (obj, corpse);
    }

    obj_give_to_room (corpse, ch->in_room);
    return corpse;
}

/* Improved Death_cry contributed by Diavolo. */
void death_cry (CHAR_T *ch) {
    ROOM_INDEX_T *was_in_room;
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
            if (char_get_eq_by_wear_loc (ch, WEAR_LOC_BODY))
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
        OBJ_T *obj;
        char *name;

        name = PERS (ch);
        obj = obj_create (obj_get_index (vnum), 0);
        obj->timer = number_range (4, 7);

        sprintf (buf, obj->short_descr, name);
        str_replace_dup (&(obj->short_descr), buf);

        sprintf (buf, obj->description, name);
        str_replace_dup (&(obj->description), buf);

        /* Convert food to poison or trash. */
        if (obj->item_type == ITEM_FOOD) {
            if (IS_SET (ch->form, FORM_POISON))
                obj->v.food.poisoned = TRUE;
            else if (!IS_SET (ch->form, FORM_EDIBLE))
                obj->item_type = ITEM_TRASH;
        }

        obj_give_to_room (obj, ch->in_room);
    }

    if (IS_NPC (ch))
        msg = "You hear something's death cry.";
    else
        msg = "You hear someone's death cry.";

    was_in_room = ch->in_room;
    for (door = 0; door < DIR_MAX; door++) {
        EXIT_T *pexit;

        if ((pexit = was_in_room->exit[door]) != NULL
            && pexit->to_room != NULL && pexit->to_room != was_in_room)
        {
            ch->in_room = pexit->to_room;
            act (msg, ch, NULL, NULL, TO_NOTCHAR);
        }
    }
    ch->in_room = was_in_room;
}

OBJ_T *raw_kill (CHAR_T *victim) {
    OBJ_T *corpse;
    const RACE_T *race;
    int i;

    stop_fighting (victim, TRUE);
    death_cry (victim);
    corpse = make_corpse (victim);

    if (IS_NPC (victim)) {
        victim->index_data->killed++;
        kill_table[URANGE (0, victim->level, MAX_LEVEL - 1)].killed++;
        char_extract (victim, TRUE);
        return corpse;
    }

    char_extract (victim, FALSE);
    while (victim->affected)
        affect_remove (victim, victim->affected);

    race = race_get (victim->race);
    victim->affected_by = race->aff;
    for (i = 0; i < 4; i++)
        victim->armor[i] = 100;

    victim->position = POS_RESTING;
    victim->hit  = UMAX (1, victim->hit);
    victim->mana = UMAX (1, victim->mana);
    victim->move = UMAX (1, victim->move);

    /* we're stable enough to not need this :) */
    /* save_char_obj (victim); */

    return corpse;
}

void group_gain (CHAR_T *ch, CHAR_T *victim) {
    CHAR_T *gch;
 /* CHAR_T *lch; */
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
        OBJ_T *obj;
        OBJ_T *obj_next;
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

        xp = fight_compute_kill_exp (gch, victim, group_levels);
        printf_to_char (gch, "You receive %d experience points.\n\r", xp);
        player_gain_exp (gch, xp);

        for (obj = ch->carrying; obj != NULL; obj = obj_next) {
            obj_next = obj->next_content;
            if (obj->wear_loc == WEAR_LOC_NONE)
                continue;

            if ((IS_OBJ_STAT (obj, ITEM_ANTI_EVIL)    && IS_EVIL (ch)) ||
                (IS_OBJ_STAT (obj, ITEM_ANTI_GOOD)    && IS_GOOD (ch)) ||
                (IS_OBJ_STAT (obj, ITEM_ANTI_NEUTRAL) && IS_NEUTRAL (ch)))
            {
                act ("You are zapped by $p.", ch, obj, NULL, TO_CHAR);
                act ("$n is zapped by $p.", ch, obj, NULL, TO_NOTCHAR);
                obj_take_from_char (obj);
                obj_give_to_room (obj, ch->in_room);
            }
        }
    }
}

/* Compute xp for a kill.
 * Also adjust alignment of killer.
 * Edit this function to change xp computations. */
int fight_compute_kill_exp (CHAR_T *gch, CHAR_T *victim, int total_levels) {
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
    if (EXT_IS_SET (victim->ext_mob, MOB_NOALIGN))
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
    if (EXT_IS_SET (victim->ext_mob, MOB_NOALIGN))
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

void dam_message (CHAR_T *ch, CHAR_T *victim, int dam, int dt,
    bool immune, int orig_dam, const char *damage_adj)
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
    /* TODO: into a table! */
#ifdef BASEMUD_SHOW_ABSOLUTE_HIT_DAMAGE
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
    /* TODO: into a table! */
#ifdef BASEMUD_MORE_PRECISE_RELATIVE_DAMAGE
         if (dam         == 0)  { vs = "miss";               vp = "misses"; }
    else if (dam_percent <= 1)  { vs = "annoy";              vp = "annoys"; }
    else if (dam_percent <= 3)  { vs = "scratch";            vp = "scratches"; }
    else if (dam_percent <= 9)  { vs = "graze";              vp = "grazes"; }
    else if (dam_percent <= 6)  { vs = "scrape";             vp = "scrapes"; }
    else if (dam_percent <= 12) { vs = "bruise";             vp = "bruises"; }
#else
         if (dam         == 0)  { vs = "miss";               vp = "misses"; }
    else if (dam_percent <= 5)  { vs = "scratch";            vp = "scratches"; }
    else if (dam_percent <= 10) { vs = "graze";              vp = "grazes"; }
#endif
    else if (dam_percent <= 15) { vs = "hit";                vp = "hits"; }
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
#ifdef BASEMUD_MORE_PRECISE_RELATIVE_DAMAGE
    else if (dam_percent < 100) { vs = "### DESTROY ###";    vp = "### DESTROYS ###"; }
#endif
    else { vs = "do UNSPEAKABLE things to"; vp = "does UNSPEAKABLE things to"; }

    /* Use an exclamation point for big attacks! */
    punct = (dam_percent <= 45) ? '.' : '!';

    /* Special message for non-skill damage without an specific attack. */
    if (dt == ATTACK_FIGHTING) {
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
    /* No more special cases - determine messages. */
    else {
        /* If this is a skill, use the skill's noun. */
        if (dt >= 0 && dt < SKILL_MAX)
            attack = skill_table[dt].noun_damage;
        /* If this is an explicit attack type, use the attack's noun. */
        else if (dt >= ATTACK_FIGHTING && dt < ATTACK_FIGHTING + ATTACK_MAX)
            attack = attack_table[dt - ATTACK_FIGHTING].noun;
        /* Unknown attack type - report it. */
        else {
            bug ("dam_message: bad dt %d.", dt);
            dt = ATTACK_FIGHTING;
            attack = attack_table[0].name;
        }

        /* Was an adjective applied to the attack type? (heavy, strong, etc) */
        if (damage_adj != NULL && damage_adj[0] != '\0') {
            sprintf (buf_hit, "%s %s", damage_adj, attack);
            attack = buf_hit;
        }

        /* Special messages for immunity. */
        if (immune) {
            if (ch == victim) {
                sprintf (buf1, "{2Luckily, you are immune to that.{x");
                sprintf (buf3, "{3$n is unaffected by $s own %s.{x", attack);
            }
            else {
                sprintf (buf1, "{2$N is unaffected by your %s!{x", attack);
                sprintf (buf2, "{4$n's %s is powerless against you.{x", attack);
                sprintf (buf3, "{3$N is unaffected by $n's %s!{x", attack);
            }
        }
        /* The attack was successful - show a message. */
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

    /* Show different messages depending on whether the attack was
     * self-inflicted or not. */
    if (ch == victim)
        act2 (buf1, buf3, ch, NULL, NULL, 0, POS_RESTING);
    else
        act3 (buf1, buf2, buf3, ch, NULL, victim, 0, POS_RESTING);
}

/* Disarm a creature.
 * Caller must check for successful attack. */
void disarm (CHAR_T *ch, CHAR_T *victim) {
    OBJ_T *obj;

    if ((obj = char_get_eq_by_wear_loc (victim, WEAR_LOC_WIELD)) == NULL)
        return;

    if (IS_OBJ_STAT (obj, ITEM_NOREMOVE)) {
        /* TODO: act3 */
        act ("{5$S weapon won't budge!{x", ch, NULL, victim, TO_CHAR);
        act ("{5$n tries to disarm you, but your weapon won't budge!{x",
             ch, NULL, victim, TO_VICT);
        act ("{5$n tries to disarm $N, but fails.{x", ch, NULL, victim,
             TO_OTHERS);
        return;
    }

    /* TODO: act3 */
    act ("{5$n DISARMS you and sends your weapon flying!{x",
         ch, NULL, victim, TO_VICT);
    act ("{5You disarm $N!{x", ch, NULL, victim, TO_CHAR);
    act ("{5$n disarms $N!{x", ch, NULL, victim, TO_OTHERS);

    obj_take_from_char (obj);
    if (IS_OBJ_STAT (obj, ITEM_NODROP) || IS_OBJ_STAT (obj, ITEM_INVENTORY))
        obj_give_to_char (obj, victim);
    else {
        obj_give_to_room (obj, victim->in_room);
        if (IS_NPC (victim) && victim->wait == 0 && char_can_see_obj (victim, obj))
            char_take_obj (victim, obj, NULL);
    }
}
