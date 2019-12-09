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
#include "music.h"
#include "comm.h"
#include "affects.h"
#include "utils.h"
#include "skills.h"
#include "mob_prog.h"
#include "fight.h"
#include "save.h"
#include "magic.h"
#include "db.h"
#include "act_player.h"
#include "chars.h"
#include "objs.h"
#include "lookup.h"
#include "globals.h"

#include "update.h"

/* used for saving */
static int save_number = 0;

int recovery_in_position (int gain, int position) {
    switch (position) {
        case POS_SLEEPING: return gain * 3 / 2;
        case POS_RESTING:  return gain;
        case POS_SITTING:  return gain * 2 / 3;
        case POS_STANDING: return gain / 2;
        case POS_FIGHTING: return gain / 4;
        default:           return gain /8;
    }
}

/* Regeneration stuff. */
int hit_gain (CHAR_T *ch, bool apply_learning) {
    int gain;
    int number;

    if (ch->in_room == NULL)
        return 0;

    if (IS_NPC (ch)) {
        gain = 5 + ch->level;
        if (IS_AFFECTED (ch, AFF_REGENERATION))
            gain *= 2;
        gain = recovery_in_position(gain, ch->position);
    }
    else {
        gain = UMAX (3, char_get_curr_stat (ch, STAT_CON) - 3 + ch->level / 2);
        gain += class_table[ch->class].hp_max - 10;

        if (ch->position < POS_STANDING) {
            number = get_skill (ch, gsn_fast_healing);
            gain += number * gain / 100;
            if (apply_learning && ch->hit < ch->max_hit && number_percent() < number)
                check_improve (ch, gsn_fast_healing, TRUE, 8 * PULSE_DIVISOR);
        }

        gain = recovery_in_position (gain, ch->position);
        if (IS_THIRSTY (ch))
            gain /= 2;
        if (IS_HUNGRY (ch))
            gain /= 2;
    }

    gain = gain * ch->in_room->heal_rate / 100;
    if (ch->on != NULL && ch->on->item_type == ITEM_FURNITURE)
        gain = gain * ch->on->v.furniture.heal_rate / 100;

    if (IS_AFFECTED (ch, AFF_HASTE) || IS_AFFECTED (ch, AFF_SLOW))
        gain /= 2;
    if (IS_AFFECTED (ch, AFF_POISON))
        gain /= 4;
    if (IS_AFFECTED (ch, AFF_PLAGUE))
        gain /= 8;

    return gain;
}

int mana_gain (CHAR_T *ch, bool apply_learning) {
    int gain;
    int number;

    if (ch->in_room == NULL)
        return 0;

    if (IS_NPC (ch)) {
        gain = 5 + ch->level;
        switch (ch->position) {
            default:
                gain /= 2;
                break;
            case POS_SLEEPING:
                gain = 3 * gain / 2;
                break;
            case POS_RESTING:
                break;
            case POS_FIGHTING:
                gain /= 3;
                break;
        }
    }
    else {
        gain = (char_get_curr_stat (ch, STAT_WIS)
              + char_get_curr_stat (ch, STAT_INT) + ch->level) / 2;

        if (ch->position < POS_STANDING && ch->position > POS_SLEEPING &&
            ch->fighting == NULL)
        {
            number = get_skill (ch, gsn_meditation);
            gain += (number * gain) / 100;
            if (apply_learning && ch->hit < ch->max_hit && number_percent() < number)
                check_improve (ch, gsn_meditation, TRUE, 8 * PULSE_DIVISOR);
        }

        if (!class_table[ch->class].gains_mana)
            gain /= 2;
        gain = recovery_in_position (gain, ch->position);

        if (IS_HUNGRY (ch))
            gain /= 2;
        if (IS_THIRSTY (ch))
            gain /= 2;
    }

    gain = gain * ch->in_room->mana_rate / 100;
    if (ch->on != NULL && ch->on->item_type == ITEM_FURNITURE)
        gain = gain * ch->on->v.furniture.mana_rate / 100;

    if (IS_AFFECTED (ch, AFF_HASTE) || IS_AFFECTED (ch, AFF_SLOW))
        gain /= 2;
    if (IS_AFFECTED (ch, AFF_POISON))
        gain /= 4;
    if (IS_AFFECTED (ch, AFF_PLAGUE))
        gain /= 8;

    return gain;
}

int move_gain (CHAR_T *ch, bool apply_learning) {
    int gain;

    if (ch->in_room == NULL)
        return 0;

    if (IS_NPC (ch))
        gain = ch->level;
    else {
        gain = UMAX (15, ch->level) + char_get_curr_stat (ch, STAT_DEX);
        gain = recovery_in_position (gain, ch->position);

        if (IS_HUNGRY (ch))
            gain /= 2;
        if (IS_THIRSTY (ch))
            gain /= 2;
    }

    gain = gain * ch->in_room->heal_rate / 100;
    if (ch->on != NULL && ch->on->item_type == ITEM_FURNITURE)
        gain = gain * ch->on->v.furniture.heal_rate / 100;

    if (IS_AFFECTED (ch, AFF_HASTE) || IS_AFFECTED (ch, AFF_SLOW))
        gain /= 2;
    if (IS_AFFECTED (ch, AFF_POISON))
        gain /= 4;
    if (IS_AFFECTED (ch, AFF_PLAGUE))
        gain /= 8;

    return gain;
}

void gain_condition (CHAR_T *ch, int cond, int value) {
    int condition;

    if (value == 0 || IS_NPC (ch) || ch->level >= LEVEL_IMMORTAL)
        return;

    condition = ch->pcdata->condition[cond];
    if (condition == -1)
        return;
    ch->pcdata->condition[cond] = URANGE (0, condition + value, 48);

    if (ch->pcdata->condition[cond] == 0) {
        switch (cond) {
            case COND_HUNGER:
                send_to_char ("You are hungry.\n\r", ch);
                break;
            case COND_THIRST:
                send_to_char ("You are thirsty.\n\r", ch);
                break;
            case COND_DRUNK:
                if (condition != 0)
                    send_to_char ("You are sober.\n\r", ch);
                break;
        }
    }
}

/* Repopulate areas periodically. */
void area_update (void) {
    AREA_T *area;

    for (area = area_first; area != NULL; area = area->next) {
        if (++area->age < 3)
            continue;

        /* Check age and reset.
         * Note: Mud School resets every 3 minutes (not 15). */
        if ((!area->empty && (area->nplayer == 0 || area->age >= 15))
            || area->age >= 31)
        {
            ROOM_INDEX_T *room_index;

            reset_area (area);
            wiznetf (NULL, NULL, WIZ_RESETS, 0, 0,
                "%s has just been reset.", area->title);

            area->age = number_range (0, 3);
            room_index = get_room_index (ROOM_VNUM_SCHOOL);
            if (room_index != NULL && area == room_index->area)
                area->age = 15 - 2;
            else if (area->nplayer == 0)
                area->empty = TRUE;
        }
    }
}

/* Mob autonomous action.
 * This function takes 25% to 35% of ALL Merc cpu time.
 * -- Furey */
void mobile_update (void) {
    CHAR_T *ch;
    CHAR_T *ch_next;
    EXIT_T *pexit;
    int door;

    /* Examine all mobs. */
    for (ch = char_list; ch != NULL; ch = ch_next) {
        ch_next = ch->next;
        if (!IS_NPC (ch) || ch->in_room == NULL
            || IS_AFFECTED (ch, AFF_CHARM))
            continue;
        if (ch->in_room->area->empty && !IS_SET (ch->mob, MOB_UPDATE_ALWAYS))
            continue;

        /* Examine call for special procedure */
        if (ch->spec_fun != 0)
            if ((*ch->spec_fun) (ch))
                continue;

        /* give him some gold */
        if (ch->index_data->shop != NULL) {
            if ((ch->gold * 100 + ch->silver) < ch->index_data->wealth) {
                ch->gold   += ch->index_data->wealth * number_range (1, 20) / 5000000;
                ch->silver += ch->index_data->wealth * number_range (1, 20) / 50000;
            }
        }

        /* Check triggers only if mobile still in default position */
        if (ch->position == ch->index_data->default_pos) {
            /* Delay */
            if (HAS_TRIGGER (ch, TRIG_DELAY) && ch->mprog_delay > 0) {
                if (--ch->mprog_delay <= 0) {
                    mp_percent_trigger (ch, NULL, NULL, NULL, TRIG_DELAY);
                    continue;
                }
            }
            if (HAS_TRIGGER (ch, TRIG_RANDOM)) {
                if (mp_percent_trigger (ch, NULL, NULL, NULL, TRIG_RANDOM))
                    continue;
            }
        }

        /* That's all for sleeping / busy monster, and empty zones */
        if (ch->position != POS_STANDING)
            continue;

        /* Scavenge */
        if (IS_SET (ch->mob, MOB_SCAVENGER)
            && ch->in_room->contents != NULL && number_bits (6) == 0)
        {
            OBJ_T *obj;
            OBJ_T *obj_best;
            int max;

            max = 1;
            obj_best = 0;
            for (obj = ch->in_room->contents; obj; obj = obj->next_content) {
                if (CAN_WEAR_FLAG (obj, ITEM_TAKE) && char_can_loot (ch, obj)
                    && obj->cost > max && obj->cost > 0)
                {
                    obj_best = obj;
                    max = obj->cost;
                }
            }
            if (obj_best) {
                obj_take_from_room (obj_best);
                obj_give_to_char (obj_best, ch);
                act ("$n gets $p.", ch, obj_best, NULL, TO_NOTCHAR);
            }
        }

        /* Wander */
        if (!IS_SET (ch->mob, MOB_SENTINEL)
            && number_bits (3) == 0
            && (door = number_bits (5)) <= 5
            && (pexit = ch->in_room->exit[door]) != NULL
            && pexit->to_room != NULL
            && !IS_SET (pexit->exit_flags, EX_CLOSED)
            && !IS_SET (pexit->to_room->room_flags, ROOM_NO_MOB)
            && (!IS_SET (ch->mob, MOB_STAY_AREA)
                || pexit->to_room->area == ch->in_room->area)
            && (!IS_SET (ch->mob, MOB_OUTDOORS)
                || !IS_SET (pexit->to_room->room_flags, ROOM_INDOORS))
            && (!IS_SET (ch->mob, MOB_INDOORS)
                || IS_SET (pexit->to_room->room_flags, ROOM_INDOORS)))
        {
            char_move (ch, door, FALSE);
        }
    }
}

/* Update the weather. */
void weather_update (void) {
    const SUN_T *sun;
    const SKY_T *sky;
    char buf[MAX_STRING_LENGTH];
    DESCRIPTOR_T *d;
    int diff;

    int mmhg, mmhg_min, mmhg_max;
    bool mmhg_rare;
    bool sky_better, sky_better_rare;
    bool sky_worse,  sky_worse_rare;

    /* Update the clock. */
    ++time_info.hour;
    while (time_info.hour >= HOURS_PER_DAY) {
        time_info.hour -= HOURS_PER_DAY;
        time_info.day++;
    }
    while (time_info.day >= DAYS_PER_MONTH) {
        time_info.day -= DAYS_PER_MONTH;
        time_info.month++;
    }
    while (time_info.month >= MONTH_MAX) {
        time_info.month -= MONTH_MAX;
        time_info.year++;
    }

    /* Update our sun position. */
    buf[0] = '\0';
    sun = sun_get_by_hour (++time_info.hour);
    if (weather_info.sunlight != sun->type) {
        weather_info.sunlight = sun->type;
        strcat (buf, sun->message);
    }

    /* Weather change. */
    if (time_info.month >= 9 && time_info.month <= 16)
        diff = (weather_info.mmhg) > 985 ? -2 : 2;
    else
        diff = (weather_info.mmhg) > 1015 ? -2 : 2;

    weather_info.change += diff * dice (1, 4) + dice (2, 6) - dice (2, 6);
    weather_info.change = UMAX (weather_info.change, -12);
    weather_info.change = UMIN (weather_info.change,  12);

    weather_info.mmhg += weather_info.change;
    weather_info.mmhg = UMAX (weather_info.mmhg, 960);
    weather_info.mmhg = UMIN (weather_info.mmhg, 1040);

    sky = sky_get_current ();
    mmhg      = weather_info.mmhg;
    mmhg_min  = sky->mmhg_min;
    mmhg_max  = sky->mmhg_max;
    mmhg_rare = (number_bits(2) == 0) ? TRUE : FALSE;

    /* Determine if the weather can get better. */
    if (mmhg_max == -1) {
        sky_better      = FALSE;
        sky_better_rare = FALSE;
    }
    else {
        sky_better      = (mmhg > (mmhg_max + 30));
        sky_better_rare = (mmhg > (mmhg_max + 10) && mmhg_rare);
    }

    /* Determine if the weather can get worse. */
    if (mmhg_min == -1) {
        sky_worse      = FALSE;
        sky_worse_rare = FALSE;
    }
    else {
        sky_worse      = (mmhg < (mmhg_min - 30));
        sky_worse_rare = (mmhg < (mmhg_min - 10) && mmhg_rare);
    }

    /* Update our sky! */
    switch (weather_info.sky) {
        case SKY_CLOUDLESS:
            if (sky_worse || sky_worse_rare) {
                strcat (buf, "The sky is getting cloudy.\n\r");
                weather_info.sky = SKY_CLOUDY;
            }
            break;

        case SKY_CLOUDY:
            if (sky_worse || sky_worse_rare) {
                strcat (buf, "It starts to rain.\n\r");
                weather_info.sky = SKY_RAINING;
            }
            else if (sky_better_rare) {
                strcat (buf, "The clouds disappear.\n\r");
                weather_info.sky = SKY_CLOUDLESS;
            }
            break;

        case SKY_RAINING:
            if (sky_worse_rare) {
                strcat (buf, "Lightning flashes in the sky.\n\r");
                weather_info.sky = SKY_LIGHTNING;
            }
            else if (sky_better || sky_better_rare) {
                strcat (buf, "The rain stopped.\n\r");
                weather_info.sky = SKY_CLOUDY;
            }
            break;

        case SKY_LIGHTNING:
            if (sky_better || sky_better_rare) {
                strcat (buf, "The lightning has stopped.\n\r");
                weather_info.sky = SKY_RAINING;
            }
            break;

        default:
            bug ("weather_update: bad sky %d.", weather_info.sky);
            weather_info.sky = SKY_CLOUDLESS;
            break;
    }

    if (buf[0] != '\0') {
        for (d = descriptor_list; d != NULL; d = d->next) {
            if (d->connected != CON_PLAYING)
                continue;
            if (!IS_OUTSIDE (d->character))
                continue;
            if (!IS_AWAKE (d->character))
                continue;
            send_to_char (buf, d->character);
        }
    }
}

void health_update(void) {
    CHAR_T *ch, *ch_next;
    for (ch = char_list; ch != NULL; ch = ch_next) {
        ch_next = ch->next;
        if (ch->position >= POS_STUNNED)
            health_update_ch(ch);
    }
}

void health_update_ch(CHAR_T *ch) {
    health_update_ch_stat(ch, &(ch->hit), &(ch->max_hit),
        &(ch->gain_hit_remainder), hit_gain);
    health_update_ch_stat(ch, &(ch->mana), &(ch->max_mana),
        &(ch->gain_mana_remainder), mana_gain);
    health_update_ch_stat(ch, &(ch->move), &(ch->max_move),
        &(ch->gain_move_remainder), move_gain);
}

void health_update_ch_stat(CHAR_T *ch, sh_int *cur, sh_int *max,
    sh_int *rem, int (*func) (CHAR_T *, bool))
{
    if (*cur == *max)
        return;
    if (*cur > *max) {
        *cur = *max;
        return;
    }
    if (*cur < *max) {
        int amount    = func(ch, TRUE);
        int portion   = amount / PULSE_DIVISOR;
        int remainder = amount % PULSE_DIVISOR + *rem;

        while (remainder >= PULSE_DIVISOR) {
            remainder -= PULSE_DIVISOR;
            portion++;
        }

        *cur += portion;
        *rem = remainder;

        /* Correct for overflow. */
        if (*cur >= *max) {
            *cur = *max;
            *rem = 0;
        }
    }
}

void damage_if_wounded (CHAR_T *ch) {
    CHAR_T *rch;
    int div = 2;

    if (ch->position > POS_INCAP)
        return;
    if (ch->position == POS_INCAP)
        div *= 2;

    /* NPC's shouldn't get damaged during stun while somebody is
     * fighting them. Too much kill theft!! */
    if (IS_NPC (ch)) {
        for (rch = ch->in_room->people; rch; rch = rch->next_in_room)
            if (rch->fighting == ch)
                break;
        if (rch)
            return;
    }

    damage_quiet (ch, ch, (ch->level / div) + 1, TYPE_UNDEFINED, DAM_NONE);
}

/* Update all chars, including mobs. */
void char_update (void) {
    CHAR_T *ch;
    CHAR_T *ch_next;
    CHAR_T *ch_quit;

    ch_quit = NULL;

    /* update save counter */
    save_number++;

    if (save_number > 29)
        save_number = 0;

    for (ch = char_list; ch != NULL; ch = ch_next) {
        AFFECT_T *paf;
        AFFECT_T *paf_next;

        ch_next = ch->next;

        if (ch->timer > 30)
            ch_quit = ch;

        if (ch->position >= POS_STUNNED) {
            /* check to see if we need to go home */
            if (IS_NPC (ch) && ch->zone != NULL
                && ch->zone != ch->in_room->area && ch->desc == NULL
                && ch->fighting == NULL && !IS_AFFECTED (ch, AFF_CHARM)
                && number_percent () < 5)
            {
                act ("$n wanders on home.", ch, NULL, NULL, TO_NOTCHAR);
                char_extract (ch, TRUE);
                continue;
            }
        }

        if (ch->position == POS_STUNNED)
            update_pos (ch);

        if (!IS_NPC (ch) && ch->level < LEVEL_IMMORTAL) {
            OBJ_T *obj;

            if ((obj = char_get_eq_by_wear_loc (ch, WEAR_LIGHT)) != NULL
                && obj->item_type == ITEM_LIGHT && obj->v.light.duration > 0)
            {
                if (--obj->v.light.duration == 0 && ch->in_room != NULL) {
                    --ch->in_room->light;
                    act ("$p flickers and goes out.", ch, obj, NULL, TO_CHAR);
                    act ("$p goes out.", ch, obj, NULL, TO_NOTCHAR);
                    obj_extract (obj);
                }
                else if (obj->v.light.duration <= 5 && ch->in_room != NULL)
                    act ("$p flickers.", ch, obj, NULL, TO_CHAR);
            }

            if (IS_IMMORTAL (ch))
                ch->timer = 0;

            if (++ch->timer >= 12) {
                if (ch->was_in_room == NULL && ch->in_room != NULL) {
                    ch->was_in_room = ch->in_room;
                    if (ch->fighting != NULL)
                        stop_fighting (ch, TRUE);
                    send_to_char ("You disappear into the void.\n\r", ch);
                    act ("$n disappears into the void.",
                         ch, NULL, NULL, TO_NOTCHAR);
                    if (ch->level > 1)
                        save_char_obj (ch);
                    char_from_room (ch);
                    char_to_room (ch, get_room_index (ROOM_VNUM_LIMBO));
                }
            }

            gain_condition (ch, COND_DRUNK,  -1);
            gain_condition (ch, COND_FULL,   ch->size > SIZE_MEDIUM ? -4 : -2);
            gain_condition (ch, COND_THIRST, -1);
            gain_condition (ch, COND_HUNGER, ch->size > SIZE_MEDIUM ? -2 : -1);
        }

        for (paf = ch->affected; paf != NULL; paf = paf_next) {
            paf_next = paf->next;
            if (paf->duration > 0) {
                paf->duration--;
                if (number_range (0, 4) == 0 && paf->level > 0)
                    paf->level--;    /* spell strength fades with time */
            }
            else if (paf->duration < 0)
                ;
            else {
                if (paf_next == NULL
                    || paf_next->type != paf->type || paf_next->duration > 0)
                {
                    if (paf->type > 0 && skill_table[paf->type].msg_off) {
                        send_to_char (skill_table[paf->type].msg_off, ch);
                        send_to_char ("\n\r", ch);
                    }
                }
                affect_remove (ch, paf);
            }
        }

        /* Careful with the damages here,
         * MUST NOT refer to ch after damage taken,
         * as it may be lethal damage (on NPC). */
        if (is_affected (ch, gsn_plague) && ch != NULL) {
            AFFECT_T *af, plague;
            CHAR_T *vch;
            int dam;

            if (ch->in_room == NULL)
                continue;

            send_to_char ("You writhe in agony from the plague.\n\r", ch);
            act ("$n writhes in agony as plague sores erupt from $s skin.",
                 ch, NULL, NULL, TO_NOTCHAR);
            for (af = ch->affected; af != NULL; af = af->next) {
                if (af->type == gsn_plague)
                    break;
            }

            if (af == NULL) {
                REMOVE_BIT (ch->affected_by, AFF_PLAGUE);
                continue;
            }

            if (af->level == 1)
                continue;

            affect_init (&plague, AFF_TO_AFFECTS, gsn_plague, af->level - 1, number_range (1, 2 * plague.level), APPLY_STR, -5, AFF_PLAGUE);

            for (vch = ch->in_room->people; vch != NULL;
                 vch = vch->next_in_room)
            {
                if (!saves_spell (plague.level - 2, vch, DAM_DISEASE)
                    && !IS_IMMORTAL (vch)
                    && !IS_AFFECTED (vch, AFF_PLAGUE) && number_bits (4) == 0)
                {
                    send_to_char ("You feel hot and feverish.\n\r", vch);
                    act ("$n shivers and looks very ill.", vch, NULL, NULL,
                         TO_NOTCHAR);
                    affect_join (vch, &plague);
                }
            }

            dam = UMIN (ch->level, af->level / 5 + 1);
            ch->mana -= dam;
            ch->move -= dam;
            damage_quiet (ch, ch, dam, gsn_plague, DAM_DISEASE);
        }
        else if (IS_AFFECTED (ch, AFF_POISON) && ch != NULL
                 && !IS_AFFECTED (ch, AFF_SLOW))
        {
            AFFECT_T *poison;

            poison = affect_find (ch->affected, gsn_poison);

            if (poison != NULL) {
                send_to_char ("You shiver and suffer.\n\r", ch);
                act ("$n shivers and suffers.", ch, NULL, NULL, TO_NOTCHAR);
                damage_quiet (ch, ch, poison->level / 10 + 1, gsn_poison,
                        DAM_POISON);
            }
        }
        else
            damage_if_wounded (ch);
    }

    /* Autosave and autoquit.
     * Check that these chars still exist. */
    for (ch = char_list; ch != NULL; ch = ch_next) {
        /* Edwin's fix for possible pet-induced problem
         * JR -- 10/15/00 */
        BAIL_IF_BUG (!IS_VALID (ch),
            "update_char: Trying to work with an invalidated character.\n", 0);

        ch_next = ch->next;
        if (ch->desc != NULL && ch->desc->descriptor % 30 == save_number)
            save_char_obj (ch);
        if (ch == ch_quit)
            do_function (ch, &do_quit, "");
    }
}

/* Update all objs.
 * This function is performance sensitive. */
void obj_update (void) {
    OBJ_T *obj;
    OBJ_T *obj_next;
    AFFECT_T *paf, *paf_next;

    for (obj = object_list; obj != NULL; obj = obj_next) {
        CHAR_T *rch;
        char *message;

        obj_next = obj->next;

        /* go through affects and decrement */
        for (paf = obj->affected; paf != NULL; paf = paf_next) {
            paf_next = paf->next;
            if (paf->duration > 0) {
                paf->duration--;
                if (number_range (0, 4) == 0 && paf->level > 0)
                    paf->level--;    /* spell strength fades with time */
            }
            else if (paf->duration < 0)
                ; /* empty */
            else {
                if (paf_next == NULL
                    || paf_next->type != paf->type || paf_next->duration > 0)
                {
                    if (paf->type > 0 && skill_table[paf->type].msg_obj) {
                        if (obj->carried_by != NULL) {
                            rch = obj->carried_by;
                            act (skill_table[paf->type].msg_obj,
                                 rch, obj, NULL, TO_CHAR);
                        }
                        if (obj->in_room != NULL
                            && obj->in_room->people != NULL)
                        {
                            rch = obj->in_room->people;
                            act (skill_table[paf->type].msg_obj,
                                 rch, obj, NULL, TO_ALL);
                        }
                    }
                }
                affect_remove_obj (obj, paf);
            }
        }

        if (obj->timer <= 0 || --obj->timer > 0)
            continue;

        switch (obj->item_type) {
            default:
                message = "$p crumbles into dust.";
                break;
            case ITEM_FOUNTAIN:
                message = "$p dries up.";
                break;
            case ITEM_CORPSE_NPC:
            case ITEM_CORPSE_PC:
                message = "$p decays into dust.";
                break;
            case ITEM_FOOD:
                message = "$p decomposes.";
                break;
            case ITEM_POTION:
                message = "$p has evaporated from disuse.";
                break;
            case ITEM_PORTAL:
                message = "$p fades out of existence.";
                break;
            case ITEM_CONTAINER:
                if (CAN_WEAR_FLAG (obj, ITEM_WEAR_FLOAT))
                    if (obj->contains)
                        message =
                            "$p flickers and vanishes, spilling its contents on the floor.";
                    else
                        message = "$p flickers and vanishes.";
                else
                    message = "$p crumbles into dust.";
                break;
        }

        if (obj->carried_by != NULL) {
            if (IS_NPC (obj->carried_by)
                    && obj->carried_by->index_data->shop != NULL)
                obj->carried_by->silver += obj->cost / 5;
            else {
                act (message, obj->carried_by, obj, NULL, TO_CHAR);
                if (obj->wear_loc == WEAR_FLOAT)
                    act (message, obj->carried_by, obj, NULL, TO_NOTCHAR);
            }
        }
        else if (obj->in_room != NULL && (rch = obj->in_room->people) != NULL) {
            if (!(obj->in_obj && obj->in_obj->index_data->vnum == OBJ_VNUM_PIT
                  && !CAN_WEAR_FLAG (obj->in_obj, ITEM_TAKE)))
            {
                act (message, rch, obj, NULL, TO_CHAR);
                act (message, rch, obj, NULL, TO_NOTCHAR);
            }
        }

        /* save the contents */
        if ((obj->item_type == ITEM_CORPSE_PC || obj->wear_loc == WEAR_FLOAT)
            && obj->contains)
        {
            OBJ_T *t_obj, *next_obj;
            for (t_obj = obj->contains; t_obj != NULL; t_obj = next_obj) {
                next_obj = t_obj->next_content;
                obj_take_from_obj (t_obj);

                /* in another object */
                if (obj->in_obj)
                    obj_give_to_obj (t_obj, obj->in_obj);
                /* carried */
                else if (obj->carried_by) {
                    if (obj->wear_loc == WEAR_FLOAT) {
                        if (obj->carried_by->in_room == NULL)
                            obj_extract (t_obj);
                        else
                            obj_give_to_room (t_obj, obj->carried_by->in_room);
                    }
                    else
                        obj_give_to_char (t_obj, obj->carried_by);
                }
                /* to a room */
                else if (obj->in_room)
                    obj_give_to_room (t_obj, obj->in_room);
                /* nowhere - destroy it! */
                else
                    obj_extract (t_obj);
            }
        }
        obj_extract (obj);
    }
}

/* Aggress.
 *
 * for each mortal PC
 *     for each mob in room
 *         aggress on some random PC
 *
 * This function takes 25% to 35% of ALL Merc cpu time.
 * Unfortunately, checking on each PC move is too tricky,
 *   because we don't the mob to just attack the first PC
 *   who leads the party into the room.
 *
 * -- Furey */
void aggr_update (void) {
    CHAR_T *wch;
    CHAR_T *wch_next;
    CHAR_T *ch;
    CHAR_T *ch_next;
    CHAR_T *vch;
    CHAR_T *vch_next;
    CHAR_T *victim;

    for (wch = char_list; wch != NULL; wch = wch_next) {
        wch_next = wch->next;
        if (IS_NPC (wch)
            || wch->level >= LEVEL_IMMORTAL
            || wch->in_room == NULL || wch->in_room->area->empty) continue;

        for (ch = wch->in_room->people; ch != NULL; ch = ch_next) {
            int count;
            ch_next = ch->next_in_room;

            if (!IS_NPC (ch)
                || !IS_SET (ch->mob, MOB_AGGRESSIVE)
                || IS_SET (ch->in_room->room_flags, ROOM_SAFE)
                || IS_AFFECTED (ch, AFF_CALM)
                || ch->fighting != NULL || IS_AFFECTED (ch, AFF_CHARM)
                || !IS_AWAKE (ch)
                || (IS_SET (ch->mob, MOB_WIMPY) && IS_AWAKE (wch))
                || !char_can_see_in_room (ch, wch) || number_bits (1) == 0)
                continue;

            /*
             * Ok we have a 'wch' player character and a 'ch' npc aggressor.
             * Now make the aggressor fight a RANDOM pc victim in the room,
             *   giving each 'vch' an equal chance of selection.
             */
            count = 0;
            victim = NULL;
            for (vch = wch->in_room->people; vch != NULL; vch = vch_next) {
                vch_next = vch->next_in_room;

                if (!IS_NPC (vch)
                    && vch->level < LEVEL_IMMORTAL
                    && ch->level >= vch->level - 5
                    && (!IS_SET (ch->mob, MOB_WIMPY) || !IS_AWAKE (vch))
                    && char_can_see_in_room (ch, vch))
                {
                    if (number_range (0, count) == 0)
                        victim = vch;
                    count++;
                }
            }

            if (victim == NULL)
                continue;

            multi_hit (ch, victim, TYPE_UNDEFINED);
        }
    }
}

/* Control the fights going on.
 * Called periodically by update_handler. */
void violence_update (void) {
    CHAR_T *ch;
    CHAR_T *ch_next;
    CHAR_T *victim;

    for (ch = char_list; ch != NULL; ch = ch_next) {
        ch_next = ch->next;

        if ((victim = ch->fighting) == NULL || ch->in_room == NULL)
            continue;

        if (IS_AWAKE (ch) && ch->in_room == victim->in_room)
            multi_hit (ch, victim, TYPE_UNDEFINED);
        else
            stop_fighting (ch, FALSE);

        if ((victim = ch->fighting) == NULL)
            continue;

        /* Fun for the whole family! */
        check_assist (ch, victim);
        if (IS_NPC (ch)) {
            if (HAS_TRIGGER (ch, TRIG_FIGHT))
                mp_percent_trigger (ch, victim, NULL, NULL, TRIG_FIGHT);
            if (HAS_TRIGGER (ch, TRIG_HPCNT))
                mp_hprct_trigger (ch, victim);
        }
    }
}

void pulse_update(void) {
    CHAR_T *ch, *ch_next;
    int dazed;

    for (ch = char_list; ch != NULL; ch = ch_next) {
        ch_next = ch->next;

        dazed = 0;
        if (ch->daze > 0) {
            dazed = 1;
            --ch->daze;
        }
        if (ch->wait > 0) {
            dazed = 1;
            --ch->wait;
        }

        /* Attempt to stand back up and fight! */
        if (dazed && ch->daze == 0 && ch->wait == 0)
            set_fighting_position_if_possible (ch);
    }
}

/* Handle all kinds of updates.
 * Called once per pulse from game loop.
 * Random times to defeat tick-timing clients and players. */
void update_handler (void) {
    static int pulse_area = 0;
    static int pulse_mobile = 0;
    static int pulse_violence = 0;
    static int pulse_point = 0;
    static int pulse_music = 0;
    static int pulse_health = 0;

    while (--pulse_area <= 0) {
        pulse_area += PULSE_AREA;
        area_update ();
    }

    while (--pulse_music <= 0) {
        pulse_music += PULSE_MUSIC;
        song_update ();
    }

    while (--pulse_mobile <= 0) {
        pulse_mobile += PULSE_MOBILE;
        mobile_update ();
    }

    while (--pulse_violence <= 0) {
        pulse_violence += PULSE_VIOLENCE;
        violence_update ();
    }

    while (--pulse_health <= 0) {
        pulse_health += PULSE_HEALTH;
        health_update ();
    }

    while (--pulse_point <= 0) {
        wiznet ("TICK!", NULL, NULL, WIZ_TICKS, 0, 0);
        pulse_point += PULSE_TICK;
        weather_update ();
        char_update ();
        obj_update ();
    }

    aggr_update ();
    pulse_update ();
    tail_chain ();
}
