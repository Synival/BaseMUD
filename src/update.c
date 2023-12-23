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

#include "update.h"

#include "areas.h"
#include "chars.h"
#include "comm.h"
#include "fight.h"
#include "globals.h"
#include "items.h"
#include "lookup.h"
#include "mob_prog.h"
#include "mobiles.h"
#include "music.h"
#include "objs.h"
#include "players.h"
#include "tables.h"
#include "utils.h"

#include <string.h>

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
        gain = recovery_in_position (gain, ch->position);
    }
    else {
        gain = UMAX (3, char_get_curr_stat (ch, STAT_CON) - 3 + ch->level / 2);
        gain += class_table[ch->class].hp_max - 10;

        if (ch->position < POS_STANDING) {
            number = char_get_skill (ch, SN(FAST_HEALING));
            gain += number * gain / 100;
            if (apply_learning && ch->hit < ch->max_hit && number_percent() < number)
                player_try_skill_improve (ch, SN(FAST_HEALING), TRUE,
                    8 * PULSE_DIVISOR);
        }

        gain = recovery_in_position (gain, ch->position);
        if (IS_THIRSTY (ch))
            gain /= 2;
        if (IS_HUNGRY (ch))
            gain /= 2;
    }

    gain = gain * ch->in_room->heal_rate / 100;
    if (ch->on != NULL && item_can_position_at (ch->on, ch->position))
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
            number = char_get_skill (ch, SN(MEDITATION));
            gain += (number * gain) / 100;
            if (apply_learning && ch->hit < ch->max_hit && number_percent() < number)
                player_try_skill_improve (ch, SN(MEDITATION), TRUE,
                    8 * PULSE_DIVISOR);
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
    if (ch->on != NULL && item_can_position_at (ch->on, ch->position))
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
    if (ch->on != NULL && item_can_position_at (ch->on, ch->position))
        gain = gain * ch->on->v.furniture.heal_rate / 100;

    if (IS_AFFECTED (ch, AFF_HASTE) || IS_AFFECTED (ch, AFF_SLOW))
        gain /= 2;
    if (IS_AFFECTED (ch, AFF_POISON))
        gain /= 4;
    if (IS_AFFECTED (ch, AFF_PLAGUE))
        gain /= 8;

    return gain;
}

void song_update (void) {
    OBJ_T *obj, *obj_next;

    /* Update global songs, if there are any */
    music_update_global ();

    /* Update all jukeboxes */
    for (obj = object_first; obj != NULL; obj = obj_next) {
        obj_next = obj->global_next;
        if (!item_is_playing (obj))
            continue;
        item_play_continue (obj);
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
    sun = sun_get_by_hour (time_info.hour);
    if (weather_info.sunlight != sun->type) {
        weather_info.sunlight = sun->type;
        strcat (buf, sun->message);
        strcat (buf, "\n\r");
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
        for (d = descriptor_first; d != NULL; d = d->global_next) {
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
    for (ch = char_first; ch != NULL; ch = ch_next) {
        ch_next = ch->global_next;
        if (ch->position >= POS_STUNNED)
            health_update_ch(ch);
    }
}

void health_update_ch(CHAR_T *ch) {
    health_update_ch_stat (ch, &(ch->hit), &(ch->max_hit),
        &(ch->gain_hit_remainder), hit_gain);
    health_update_ch_stat (ch, &(ch->mana), &(ch->max_mana),
        &(ch->gain_mana_remainder), mana_gain);
    health_update_ch_stat (ch, &(ch->move), &(ch->max_move),
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
    CHAR_T *wch, *wch_next;
    CHAR_T *ch,  *ch_next;
    CHAR_T *vch, *vch_next;
    CHAR_T *victim;

    for (wch = char_first; wch != NULL; wch = wch_next) {
        wch_next = wch->global_next;
        if (IS_NPC (wch))
            continue;
        if (wch->level >= LEVEL_IMMORTAL)
            continue;
        if (wch->in_room == NULL)
            continue;
        if (!wch->in_room->area->had_players)
            continue;

        for (ch = wch->in_room->people_first; ch != NULL; ch = ch_next) {
            int count;
            ch_next = ch->room_next;

            if (!IS_NPC (ch)
                || !EXT_IS_SET (ch->ext_mob, MOB_AGGRESSIVE)
                || IS_SET (ch->in_room->room_flags, ROOM_SAFE)
                || IS_AFFECTED (ch, AFF_CALM)
                || ch->fighting != NULL || IS_AFFECTED (ch, AFF_CHARM)
                || !IS_AWAKE (ch)
                || (EXT_IS_SET (ch->ext_mob, MOB_WIMPY) && IS_AWAKE (wch))
                || !char_can_see_in_room (ch, wch) || number_bits (1) == 0)
                continue;

            /*
             * Ok we have a 'wch' player character and a 'ch' npc aggressor.
             * Now make the aggressor fight a RANDOM pc victim in the room,
             *   giving each 'vch' an equal chance of selection.
             */
            count = 0;
            victim = NULL;
            for (vch = wch->in_room->people_first; vch != NULL; vch = vch_next) {
                vch_next = vch->room_next;

                if (!IS_NPC (vch)
                    && vch->level < LEVEL_IMMORTAL
                    && ch->level >= vch->level - 5
                    && (!EXT_IS_SET (ch->ext_mob, MOB_WIMPY) || !IS_AWAKE (vch))
                    && char_can_see_in_room (ch, vch))
                {
                    if (number_range (0, count) == 0)
                        victim = vch;
                    count++;
                }
            }

            if (victim == NULL)
                continue;

            multi_hit (ch, victim, ATTACK_DEFAULT);
        }
    }
}

/* Control the fights going on.
 * Called periodically by update_handler. */
void violence_update (void) {
    CHAR_T *ch;
    CHAR_T *ch_next;
    CHAR_T *victim;

    for (ch = char_first; ch != NULL; ch = ch_next) {
        ch_next = ch->global_next;

        if ((victim = ch->fighting) == NULL || ch->in_room == NULL)
            continue;

        if (IS_AWAKE (ch) && ch->in_room == victim->in_room)
            multi_hit (ch, victim, ATTACK_DEFAULT);
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

    for (ch = char_first; ch != NULL; ch = ch_next) {
        ch_next = ch->global_next;

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
    static int pulse_area     = PULSE_AREA; /* Already run at boot */
    static int pulse_mobile   = 0;
    static int pulse_violence = 0;
    static int pulse_point    = 0;
    static int pulse_music    = 0;
    static int pulse_health   = 0;

    while (--pulse_area <= 0) {
        pulse_area += PULSE_AREA;
        area_update_all ();
    }

    while (--pulse_music <= 0) {
        pulse_music += PULSE_MUSIC;
        song_update ();
    }

    while (--pulse_mobile <= 0) {
        pulse_mobile += PULSE_MOBILE;
        mobile_update_all ();
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
        char_update_all ();
        obj_update_all ();
    }

    aggr_update ();
    pulse_update ();
    tail_chain ();
}
