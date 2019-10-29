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

#include "magic.h"
#include "utils.h"
#include "fight.h"
#include "comm.h"
#include "affects.h"
#include "lookup.h"
#include "db.h"
#include "spell_aff.h"
#include "skills.h"
#include "chars.h"
#include "objs.h"

#include "spell_off.h"

DEFINE_SPELL_FUN (spell_acid_blast) {
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

    dam = dice (level, 12);
    if (saves_spell (level, victim, DAM_ACID))
        dam /= 2;
    damage (ch, victim, dam, sn, DAM_ACID, TRUE);
}

DEFINE_SPELL_FUN (spell_burning_hands) {
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    static const sh_int dam_each[] = {
         0,
         0,  0,  0,  0, 14, 17, 20, 23, 26, 29,
        29, 29, 30, 30, 31, 31, 32, 32, 33, 33,
        34, 34, 35, 35, 36, 36, 37, 37, 38, 38,
        39, 39, 40, 40, 41, 41, 42, 42, 43, 43,
        44, 44, 45, 45, 46, 46, 47, 47, 48, 48
    };
    int dam;

    level = UMIN (level, sizeof (dam_each) / sizeof (dam_each[0]) - 1);
    level = UMAX (0, level);
    dam = number_range (dam_each[level] / 2, dam_each[level] * 2);
    if (saves_spell (level, victim, DAM_FIRE))
        dam /= 2;
    damage (ch, victim, dam, sn, DAM_FIRE, TRUE);
}

DEFINE_SPELL_FUN (spell_call_lightning) {
    CHAR_DATA *vch;
    CHAR_DATA *vch_next;
    int dam;

    BAIL_IF (!IS_OUTSIDE (ch),
        "You must be out of doors.\n\r", ch);
    BAIL_IF (weather_info.sky < SKY_RAINING,
        "You need bad weather.\n\r", ch);

    dam = dice (level / 2, 8);
    send_to_char ("Mota's lightning strikes your foes!\n\r", ch);
    act ("$n calls Mota's lightning to strike $s foes!",
         ch, NULL, NULL, TO_NOTCHAR);

    for (vch = char_list; vch != NULL; vch = vch_next) {
        vch_next = vch->next;
        if (vch->in_room == NULL)
            continue;
        if (vch->in_room == ch->in_room) {
            if (vch != ch && (IS_NPC (ch) ? !IS_NPC (vch) : IS_NPC (vch)))
                damage (ch, vch, saves_spell (level, vch, DAM_LIGHTNING)
                        ? dam / 2 : dam, sn, DAM_LIGHTNING, TRUE);
            continue;
        }

        if (vch->in_room->area == ch->in_room->area && IS_OUTSIDE (vch)
            && IS_AWAKE (vch))
            send_to_char ("Lightning flashes in the sky.\n\r", vch);
    }
}

DEFINE_SPELL_FUN (spell_cause_light)
    { damage (ch, (CHAR_DATA *) vo, dice (1, 8) + level / 3, sn, DAM_HARM, TRUE); }
DEFINE_SPELL_FUN (spell_cause_serious)
    { damage (ch, (CHAR_DATA *) vo, dice (2, 8) + level / 2, sn, DAM_HARM, TRUE); }
DEFINE_SPELL_FUN (spell_cause_critical)
    { damage (ch, (CHAR_DATA *) vo, dice (3, 8) + level - 6, sn, DAM_HARM, TRUE); }

DEFINE_SPELL_FUN (spell_chain_lightning) {
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    CHAR_DATA *tmp_vict, *last_vict, *next_vict;
    bool found;
    int dam;

    /* first strike */
    act3 ("A lightning bolt leaps from your hand and arcs to $N.",
          "A lightning bolt leaps from $n's hand and hits you!",
          "A lightning bolt leaps from $n's hand and arcs to $N.",
        ch, NULL, victim, 0, POS_RESTING);

    dam = dice (level, 6);
    if (saves_spell (level, victim, DAM_LIGHTNING))
        dam /= 3;
    damage (ch, victim, dam, sn, DAM_LIGHTNING, TRUE);
    last_vict = victim;
    level -= 4; /* decrement damage */

    /* new targets */
    while (level > 0) {
        found = FALSE;
        for (tmp_vict = ch->in_room->people;
             tmp_vict != NULL; tmp_vict = next_vict)
        {
            next_vict = tmp_vict->next_in_room;
            if (tmp_vict == last_vict || !can_attack_spell (ch, tmp_vict, TRUE))
                continue;

            found = TRUE;
            last_vict = tmp_vict;
            act2 ("The bolt hits you!", "The bolt arcs to $n!",
                tmp_vict, NULL, NULL, 0, POS_RESTING);
            dam = dice (level, 6);
            if (saves_spell (level, tmp_vict, DAM_LIGHTNING))
                dam /= 3;
            damage (ch, tmp_vict, dam, sn, DAM_LIGHTNING, TRUE);
            level -= 4; /* decrement damage */
        }

        /* no target found, hit the caster */
        if (!found) {
            if (ch == NULL)
                return;

            /* no double hits */
            if (last_vict == ch) {
                act2 ("The bolt grounds out through your body.",
                      "The bolt seems to have fizzled out.",
                    ch, NULL, NULL, 0, POS_RESTING);
                return;
            }

            last_vict = ch;
            send_to_char ("You are struck by your own lightning!\n\r", ch);
            act ("The bolt arcs to $n...whoops!", ch, NULL, NULL, TO_NOTCHAR);
            dam = dice (level, 6);
            if (saves_spell (level, ch, DAM_LIGHTNING))
                dam /= 3;
            damage (ch, ch, dam, sn, DAM_LIGHTNING, TRUE);
            level -= 4; /* decrement damage */
        }

        /* now go back and find more targets */
    }
}

DEFINE_SPELL_FUN (spell_chill_touch) {
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    static const sh_int dam_each[] = {
         0,
         0,  0,  6,  7,  8,  9, 12, 13, 13, 13,
        14, 14, 14, 15, 15, 15, 16, 16, 16, 17,
        17, 17, 18, 18, 18, 19, 19, 19, 20, 20,
        20, 21, 21, 21, 22, 22, 22, 23, 23, 23,
        24, 24, 24, 25, 25, 25, 26, 26, 26, 27
    };
    AFFECT_DATA af;
    int dam;

    level = UMIN (level, sizeof (dam_each) / sizeof (dam_each[0]) - 1);
    level = UMAX (0, level);
    dam = number_range (dam_each[level] / 2, dam_each[level] * 2);
    if (!saves_spell (level, victim, DAM_COLD)) {
        act2 ("You feel a sudden chill and shiver.",
              "$n turns blue and shivers.", victim, NULL, NULL,
            0, POS_RESTING);
        affect_init (&af, AFF_TO_AFFECTS, sn, level, 6, APPLY_STR, -1, 0);
        affect_join (victim, &af);
    }
    else
        dam /= 2;
    damage (ch, victim, dam, sn, DAM_COLD, TRUE);
}

DEFINE_SPELL_FUN (spell_colour_spray) {
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    static const sh_int dam_each[] = {
         0,
         0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
        30, 35, 40, 45, 50, 55, 55, 55, 56, 57,
        58, 58, 59, 60, 61, 61, 62, 63, 64, 64,
        65, 66, 67, 67, 68, 69, 70, 70, 71, 72,
        73, 73, 74, 75, 76, 76, 77, 78, 79, 79
    };
    int dam;

    level = UMIN (level, sizeof (dam_each) / sizeof (dam_each[0]) - 1);
    level = UMAX (0, level);
    dam = number_range (dam_each[level] / 2, dam_each[level] * 2);
    if (saves_spell (level, victim, DAM_LIGHT))
        dam /= 2;
    else
        spell_blindness_quiet (gsn_blindness, level / 2, ch,
            (void *) victim, TARGET_CHAR);

    damage (ch, victim, dam, sn, DAM_LIGHT, TRUE);
}

/* RT replacement demonfire spell */
DEFINE_SPELL_FUN (spell_demonfire) {
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

    if (!IS_NPC (ch) && !IS_EVIL (ch)) {
        victim = ch;
        send_to_char ("The demons turn upon you!\n\r", ch);
    }

    ch->alignment = UMAX (-1000, ch->alignment - 50);
    if (victim != ch) {
        act3 ("You conjure forth the demons of Hell!",
              "$n has assailed you with the demons of Hell!",
              "$n calls forth the demons of Hell upon $N!",
             ch, NULL, victim, 0, POS_RESTING);
    }

    dam = dice (level, 10);
    if (saves_spell (level, victim, DAM_NEGATIVE))
        dam /= 2;
    damage (ch, victim, dam, sn, DAM_NEGATIVE, TRUE);
    spell_curse_char_quiet (gsn_curse, 3 * level / 4, ch, (void *) victim,
        TARGET_CHAR);
}

DEFINE_SPELL_FUN (spell_dispel_evil) {
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

    if (!IS_NPC (ch) && IS_EVIL (ch))
        victim = ch;

    if (IS_GOOD (victim)) {
        act3 ("Mota protects $N from your spell.",
              "Mota protects you from $n's spell.",
              "Mota protects $N from $n's spell.",
            ch, NULL, victim, 0, POS_RESTING);
        return;
    }
    BAIL_IF_ACT (IS_NEUTRAL (victim),
        "$N does not seem to be affected.", ch, NULL, victim);

    if (victim->hit > (ch->level * 4))
        dam = dice (level, 4);
    else
        dam = UMAX (victim->hit, dice (level, 4));
    if (saves_spell (level, victim, DAM_HOLY))
        dam /= 2;
    damage (ch, victim, dam, sn, DAM_HOLY, TRUE);
}


DEFINE_SPELL_FUN (spell_dispel_good) {
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

    if (!IS_NPC (ch) && IS_GOOD (ch))
        victim = ch;

    if (IS_EVIL (victim)) {
        act3 ("$N is protected from your spell by $S evil.",
              "You are protected from $n's spell by your evil.",
              "$N is protected from $n's spell by $S evil.",
            ch, NULL, victim, 0, POS_RESTING);
        return;
    }
    BAIL_IF_ACT (IS_NEUTRAL (victim),
        "$N does not seem to be affected.", ch, NULL, victim);

    if (victim->hit > (ch->level * 4))
        dam = dice (level, 4);
    else
        dam = UMAX (victim->hit, dice (level, 4));
    if (saves_spell (level, victim, DAM_NEGATIVE))
        dam /= 2;
    damage (ch, victim, dam, sn, DAM_NEGATIVE, TRUE);
}

DEFINE_SPELL_FUN (spell_earthquake) {
    CHAR_DATA *vch;
    CHAR_DATA *vch_next;

    send_to_char ("The earth trembles beneath your feet!\n\r", ch);
    act ("$n makes the earth tremble and shiver.", ch, NULL, NULL, TO_NOTCHAR);

    for (vch = char_list; vch != NULL; vch = vch_next) {
        vch_next = vch->next;
        if (vch->in_room == NULL)
            continue;
        if (vch->in_room == ch->in_room) {
            if (vch != ch && can_attack_spell (ch, vch, TRUE)) {
                if (IS_AFFECTED (vch, AFF_FLYING))
                    damage (ch, vch, 0, sn, DAM_BASH, TRUE);
                else
                    damage (ch, vch, level + dice (2, 8), sn, DAM_BASH, TRUE);
            }
            continue;
        }
        if (vch->in_room->area == ch->in_room->area)
            send_to_char ("The earth trembles and shivers.\n\r", vch);
    }
}

/* Drain XP, MANA, HP.
 * Caster gains HP. */
DEFINE_SPELL_FUN (spell_energy_drain) {
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

    if (victim != ch)
        ch->alignment = UMAX (-1000, ch->alignment - 50);

    if (saves_spell (level, victim, DAM_NEGATIVE)) {
        send_to_char ("You are left unsatisfied.\n\r", ch);
        send_to_char ("You feel a momentary chill.\n\r", victim);
        return;
    }

    if (victim->level <= 2)
        dam = ch->hit + 1;
    else {
        gain_exp (victim, 0 - number_range (level / 2, 3 * level / 2));
        victim->mana /= 2;
        victim->move /= 2;
        dam = dice (1, level);
        ch->hit += dam;
    }

    send_to_char ("You feel your life slipping away!\n\r", victim);
    send_to_char ("Wow....what a rush!\n\r", ch);
    damage (ch, victim, dam, sn, DAM_NEGATIVE, TRUE);
}

DEFINE_SPELL_FUN (spell_fireball) {
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    static const sh_int dam_each[] = {
          0,
          0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
          0,   0,   0,   0,  30,  35,  40,  45,  50,  55,
         60,  65,  70,  75,  80,  82,  84,  86,  88,  90,
         92,  94,  96,  98, 100, 102, 104, 106, 108, 110,
        112, 114, 116, 118, 120, 122, 124, 126, 128, 130
    };
    int dam;

    level = UMIN (level, sizeof (dam_each) / sizeof (dam_each[0]) - 1);
    level = UMAX (0, level);
    dam = number_range (dam_each[level] / 2, dam_each[level] * 2);
    if (saves_spell (level, victim, DAM_FIRE))
        dam /= 2;
    damage (ch, victim, dam, sn, DAM_FIRE, TRUE);
}

DEFINE_SPELL_FUN (spell_flamestrike) {
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

    dam = dice (6 + level / 2, 8);
    if (saves_spell (level, victim, DAM_FIRE))
        dam /= 2;
    damage (ch, victim, dam, sn, DAM_FIRE, TRUE);
}

DEFINE_SPELL_FUN (spell_harm) {
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

    dam = UMAX (20, victim->hit - dice (1, 4));
    if (saves_spell (level, victim, DAM_HARM))
        dam = UMIN (50, dam / 2);
    dam = UMIN (100, dam);
    damage (ch, victim, dam, sn, DAM_HARM, TRUE);
}

DEFINE_SPELL_FUN (spell_heat_metal) {
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    OBJ_DATA *obj_lose, *obj_next;
    int dam = 0;
    bool success = FALSE;
    bool is_weapon, is_worn, can_drop;
    bool fumbled, drop_item;

    if (saves_spell (level + 2, victim, DAM_FIRE)
        || IS_SET (victim->imm_flags, RES_FIRE))
    {
        send_to_char ("Your spell had no effect.\n\r", ch);
        send_to_char ("You feel momentarily warmer.\n\r", victim);
        return;
    }

    for (obj_lose = victim->carrying; obj_lose != NULL; obj_lose = obj_next) {
        obj_next = obj_lose->next_content;
        if (number_range (1, 2 * level) <= obj_lose->level)
            continue;
        if (saves_spell (level, victim, DAM_FIRE))
            continue;
        if (IS_OBJ_STAT (obj_lose, ITEM_NONMETAL))
            continue;
        if (IS_OBJ_STAT (obj_lose, ITEM_BURN_PROOF))
            continue;

        /* Only heat weapons and armor. */
        if (obj_lose->item_type != ITEM_ARMOR &&
            obj_lose->item_type != ITEM_WEAPON)
            continue;

        /* Flaming weapons being wielded are ignored. */
        is_weapon = (obj_lose->item_type == ITEM_WEAPON);
        is_worn   = (obj_lose->wear_loc != -1);
        if (is_worn && is_weapon && IS_WEAPON_STAT (obj_lose, WEAPON_FLAMING))
            continue;

        success = TRUE;
        can_drop = char_can_drop_obj (victim, obj_lose);
        drop_item = FALSE;

        /* Different conditions + damage + messages for worn items. */
        if (is_worn) {
            fumbled = FALSE;
            if (is_weapon)
                fumbled = TRUE;
            else {
                int dex        = char_get_curr_stat (victim, STAT_DEX);
                int obj_weight = obj_lose->weight / 10;
                int heaviest   = number_range (1, 2 * dex);
                if (obj_weight > heaviest)
                    fumbled = TRUE;
            }

            if (can_drop && fumbled &&
                char_remove_obj (victim, obj_lose->wear_loc, TRUE, TRUE))
            {
                act2 (
                    is_weapon ? "You throw your red-hot weapon to the ground!"
                              : "You remove and drop $p before it burns you!",
                    is_weapon ? "$n is burned by $p, and throws it to the ground!"
                              : "$n yelps and throws $p to the ground!",
                    victim, obj_lose, NULL, 0, POS_RESTING);

                dam += is_weapon ? 1 : (number_range (1, obj_lose->level) / 3);
                drop_item = TRUE;
            }
            else {
                act (is_weapon ? "Your weapon sears your flesh!"
                               : "Your skin is seared by $p!",
                    victim, obj_lose, NULL, TO_CHAR);
                dam += is_weapon ?  number_range (1, obj_lose->level)
                                 : (number_range (1, obj_lose->level) / 2);
            }
        }
        /* Non-worn items. */
        else {
            if (can_drop) {
                act2 (is_weapon ? "You drop $p before it burns you."
                                : "You drop $p before it burns you.",
                      is_weapon ? "$n throws a burning hot $p to the ground!"
                                : "$n yelps and throws $p to the ground!",
                    victim, obj_lose, NULL, 0, POS_RESTING);

                dam += (number_range (1, obj_lose->level) / 6);
                drop_item = TRUE;
            }
            else {
                act (is_weapon ? "Your skin is seared by $p!"
                               : "Your skin is seared by $p!",
                    victim, obj_lose, NULL, TO_CHAR);
                dam += (number_range (1, obj_lose->level) / 2);
            }
        }

        /* Checks passed to drop item - so drop it! */
        if (drop_item) {
            obj_from_char (obj_lose);
            obj_to_room (obj_lose, victim->in_room);
        }
    }

    if (!success) {
        send_to_char ("Your spell had no effect.\n\r", ch);
        send_to_char ("You feel momentarily warmer.\n\r", victim);
        return;
    }

    /* damage! */
    if (saves_spell (level, victim, DAM_FIRE))
        dam = 2 * dam / 3;
    damage (ch, victim, dam, sn, DAM_FIRE, TRUE);
}

/* RT really nasty high-level attack spell */
DEFINE_SPELL_FUN (spell_holy_word) {
    CHAR_DATA *vch;
    CHAR_DATA *vch_next;
    int dam;
    int bless_num, curse_num, frenzy_num;

    bless_num  = skill_lookup ("bless");
    curse_num  = skill_lookup ("curse");
    frenzy_num = skill_lookup ("frenzy");

    send_to_char ("You utter a word of divine power.\n\r", ch);
    act ("$n utters a word of divine power!", ch, NULL, NULL, TO_NOTCHAR);

    for (vch = ch->in_room->people; vch != NULL; vch = vch_next) {
        vch_next = vch->next_in_room;

        if (IS_SAME_ALIGN (ch, vch)) {
            send_to_char ("You feel even more powerful.\n\r", vch);
            spell_frenzy (frenzy_num, level, ch, (void *) vch, TARGET_CHAR);
            spell_bless (bless_num, level, ch, (void *) vch, TARGET_CHAR);
        }
        else if ((IS_GOOD (ch) && IS_EVIL (vch)) ||
                 (IS_EVIL (ch) && IS_GOOD (vch)))
        {
            if (can_attack_spell (ch, vch, TRUE)) {
                send_to_char ("You are struck down!\n\r", vch);
                dam = dice (level, 6);
                damage (ch, vch, dam, sn, DAM_ENERGY, TRUE);
                spell_curse_char_quiet (curse_num, level, ch, (void *) vch,
                    TARGET_CHAR);
            }
        }
        else if (IS_NEUTRAL (ch)) {
            if (can_attack_spell (ch, vch, TRUE)) {
                send_to_char ("You are struck down!\n\r", vch);
                dam = dice (level, 4);
                damage (ch, vch, dam, sn, DAM_ENERGY, TRUE);
                spell_curse_char_quiet (curse_num, level / 2, ch, (void *) vch,
                    TARGET_CHAR);
            }
        }
    }

    send_to_char ("You feel drained.\n\r", ch);
    ch->move = 0;
    ch->hit /= 2;
}

DEFINE_SPELL_FUN (spell_lightning_bolt) {
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    static const sh_int dam_each[] = {
         0,
         0,  0,  0,  0,  0,  0,  0,  0, 25, 28,
        31, 34, 37, 40, 40, 41, 42, 42, 43, 44,
        44, 45, 46, 46, 47, 48, 48, 49, 50, 50,
        51, 52, 52, 53, 54, 54, 55, 56, 56, 57,
        58, 58, 59, 60, 60, 61, 62, 62, 63, 64
    };
    int dam;

    level = UMIN (level, sizeof (dam_each) / sizeof (dam_each[0]) - 1);
    level = UMAX (0, level);
    dam = number_range (dam_each[level] / 2, dam_each[level] * 2);
    if (saves_spell (level, victim, DAM_LIGHTNING))
        dam /= 2;
    damage (ch, victim, dam, sn, DAM_LIGHTNING, TRUE);
}

DEFINE_SPELL_FUN (spell_magic_missile) {
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    static const sh_int dam_each[] = {
         0,
         3,  3,  4,  4,  5,  6,  6,  6,  6,  6,
         7,  7,  7,  7,  7,  8,  8,  8,  8,  8,
         9,  9,  9,  9,  9, 10, 10, 10, 10, 10,
        11, 11, 11, 11, 11, 12, 12, 12, 12, 12,
        13, 13, 13, 13, 13, 14, 14, 14, 14, 14
    };
    int dam;

    level = UMIN (level, sizeof (dam_each) / sizeof (dam_each[0]) - 1);
    level = UMAX (0, level);
    dam = number_range (dam_each[level] / 2, dam_each[level] * 2);
    if (saves_spell (level, victim, DAM_ENERGY))
        dam /= 2;
    damage (ch, victim, dam, sn, DAM_ENERGY, TRUE);
}

DEFINE_SPELL_FUN (spell_ray_of_truth) {
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam, align;

    if (IS_EVIL (ch)) {
        victim = ch;
        send_to_char ("The energy explodes inside you!\n\r", ch);
    }

    if (victim != ch) {
        act2 ("You raise your hand and a blinding ray of light shoots forth!",
              "$n raises $s hand, and a blinding ray of light shoots forth!",
             ch, NULL, NULL, 0, POS_RESTING);
    }

    if (IS_GOOD (victim)) {
        act2 ("The light seems powerless to affect you.",
              "$n seems unharmed by the light.",
            victim, NULL, NULL, 0, POS_RESTING);
        return;
    }

    dam = dice (level, 10);
    if (saves_spell (level, victim, DAM_HOLY))
        dam /= 2;

    align = victim->alignment;
    align -= 350;
    if (align < -1000)
        align = -1000 + (align + 1000) / 3;
    dam = (dam * align * align) / 1000000;

    damage (ch, victim, dam, sn, DAM_HOLY, TRUE);
    spell_blindness_quiet (gsn_blindness, 3 * level / 4, ch, (void *) victim,
        TARGET_CHAR);
}

DEFINE_SPELL_FUN (spell_shocking_grasp) {
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    static const int dam_each[] = {
         0,
         0,  0,  0,  0,  0,  0, 20, 25, 29, 33,
        36, 39, 39, 39, 40, 40, 41, 41, 42, 42,
        43, 43, 44, 44, 45, 45, 46, 46, 47, 47,
        48, 48, 49, 49, 50, 50, 51, 51, 52, 52,
        53, 53, 54, 54, 55, 55, 56, 56, 57, 57
    };
    int dam;

    level = UMIN (level, sizeof (dam_each) / sizeof (dam_each[0]) - 1);
    level = UMAX (0, level);
    dam = number_range (dam_each[level] / 2, dam_each[level] * 2);
    if (saves_spell (level, victim, DAM_LIGHTNING))
        dam /= 2;
    damage (ch, victim, dam, sn, DAM_LIGHTNING, TRUE);
}
