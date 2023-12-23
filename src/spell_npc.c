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

#include "spell_npc.h"

#include "chars.h"
#include "comm.h"
#include "effects.h"
#include "fight.h"
#include "lookup.h"
#include "magic.h"
#include "utils.h"

#include <stdio.h>

/* NPC spells. */
void perform_breath_attack (CHAR_T *ch, ROOM_INDEX_T *room,
    CHAR_T *victim, int dam_type, int level, int dam, int sn)
{
    const DAM_T *damt;
    CHAR_T *vch, *vch_next;
    EFFECT_FUN *effect;
    bool is_area_spell;

    /* get damage-type specific information. */
    damt = dam_get (dam_type);
    effect = damt ? effect_get (damt->effect)->effect_fun : effect_empty;

    /* effect and partial damage to room. */
    is_area_spell = (room != NULL) ? TRUE : FALSE;
    if (is_area_spell) {
        effect (room, level, dam / 2, TARGET_ROOM);
        for (vch = room->people_first; vch != NULL; vch = vch_next) {
            vch_next = vch->room_next;
            if (vch == victim)
                continue;

            if (IS_NPC (ch) && IS_NPC (vch) &&
                    (ch->fighting != vch || vch->fighting != ch))
                continue;
            if (!can_attack_spell (ch, vch, is_area_spell))
                continue;

            if (saves_spell (level - 2, vch, dam_type)) {
                effect (vch, level / 4, dam / 8, TARGET_CHAR);
                damage_visible (ch, vch, dam / 4, sn, dam_type, NULL);
            }
            else {
                effect (vch, level / 2, dam / 4, TARGET_CHAR);
                damage_visible (ch, vch, dam / 2, sn, dam_type, NULL);
            }
        }
    }

    /* full damage to victim. */
    if (victim != NULL) {
        do {
            if (IS_NPC (ch) && IS_NPC (victim) &&
                    (ch->fighting != victim || victim->fighting != ch))
                break;
            if (!can_attack_spell (ch, victim, is_area_spell))
                break;

            if (saves_spell (level, victim, dam_type)) {
                effect (victim, level / 2, dam / 4, TARGET_CHAR);
                damage_visible (ch, victim, dam / 2, sn, dam_type, NULL);
            }
            else {
                effect (victim, level, dam, TARGET_CHAR);
                damage_visible (ch, victim, dam, sn, dam_type, NULL);
            }
        } while (0);
    }
}

DEFINE_SPELL_FUN (spell_acid_breath) {
    CHAR_T *victim = (CHAR_T *) vo;
    int dam, hp_dam, dice_dam, hpch;

    act3 ("You spit acid at $N.",
          "$n spits a stream of corrosive acid at you.",
          "$n spits acid at $N.",
        ch, NULL, victim, 0, POS_RESTING);

    hpch = UMAX (12, ch->hit);
    hp_dam = number_range (hpch / 11 + 1, hpch / 6);
    dice_dam = dice (level, 16);
    dam = UMAX (hp_dam + dice_dam / 10, dice_dam + hp_dam / 10);

    perform_breath_attack (ch, NULL, victim, DAM_ACID,
        level, dam, sn);
}

DEFINE_SPELL_FUN (spell_fire_breath) {
    CHAR_T *victim = (CHAR_T *) vo;
    int dam, hp_dam, dice_dam;
    int hpch;

    act3 ("You breathe forth a cone of fire.",
          "$n breathes a cone of hot fire over you!",
          "$n breathes forth a cone of fire.",
        ch, NULL, victim, 0, POS_RESTING);

    hpch = UMAX (10, ch->hit);
    hp_dam = number_range (hpch / 9 + 1, hpch / 5);
    dice_dam = dice (level, 20);
    dam = UMAX (hp_dam + dice_dam / 10, dice_dam + hp_dam / 10);

    perform_breath_attack (ch, victim->in_room, victim, DAM_FIRE,
        level, dam, sn);
}

DEFINE_SPELL_FUN (spell_frost_breath) {
    CHAR_T *victim = (CHAR_T *) vo;
    int dam, hp_dam, dice_dam, hpch;

    act3 ("You breathe out a cone of frost.",
          "$n breathes a freezing cone of frost over you!",
          "$n breathes out a freezing cone of frost!",
        ch, NULL, victim, 0, POS_RESTING);

    hpch = UMAX (12, ch->hit);
    hp_dam = number_range (hpch / 11 + 1, hpch / 6);
    dice_dam = dice (level, 16);
    dam = UMAX (hp_dam + dice_dam / 10, dice_dam + hp_dam / 10);

    perform_breath_attack (ch, victim->in_room, victim, DAM_COLD,
        level, dam, sn);
}

DEFINE_SPELL_FUN (spell_gas_breath) {
    int dam, hp_dam, dice_dam, hpch;

    act2 ("You breathe out a cloud of poisonous gas.",
          "$n breathes out a cloud of poisonous gas!", ch, NULL, NULL,
        0, POS_RESTING);

    hpch = UMAX (16, ch->hit);
    hp_dam = number_range (hpch / 15 + 1, 8);
    dice_dam = dice (level, 12);
    dam = UMAX (hp_dam + dice_dam / 10, dice_dam + hp_dam / 10);

    perform_breath_attack (ch, ch->in_room, NULL, DAM_POISON,
        level, dam, sn);
}

DEFINE_SPELL_FUN (spell_lightning_breath) {
    CHAR_T *victim = (CHAR_T *) vo;
    int dam, hp_dam, dice_dam, hpch;

    act3 ("You breathe a bolt of lightning at $N.",
          "$n breathes a bolt of lightning at you!",
          "$n breathes a bolt of lightning at $N.", ch, NULL, victim,
        0, POS_RESTING);

    hpch = UMAX (10, ch->hit);
    hp_dam = number_range (hpch / 9 + 1, hpch / 5);
    dice_dam = dice (level, 20);
    dam = UMAX (hp_dam + dice_dam / 10, dice_dam + hp_dam / 10);

    perform_breath_attack (ch, NULL, victim, DAM_POISON,
        level, dam, sn);
}

/* Spells for mega1.are from Glop/Erkenbrand. */
DEFINE_SPELL_FUN (spell_general_purpose) {
    CHAR_T *victim = (CHAR_T *) vo;
    int dam;

    dam = number_range (25, 100);
    if (saves_spell (level, victim, DAM_PIERCE))
        dam /= 2;
    damage_visible (ch, victim, dam, sn, DAM_PIERCE, NULL);
}

DEFINE_SPELL_FUN (spell_high_explosive) {
    CHAR_T *victim = (CHAR_T *) vo;
    int dam;

    dam = number_range (30, 120);
    if (saves_spell (level, victim, DAM_PIERCE))
        dam /= 2;
    damage_visible (ch, victim, dam, sn, DAM_PIERCE, NULL);
}
