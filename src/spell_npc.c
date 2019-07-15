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
#include "lookup.h"
#include "effects.h"
#include "fight.h"
#include "comm.h"

#include "spell_npc.h"

/* NPC spells. */
void perform_breath_attack (CHAR_DATA * ch, ROOM_INDEX_DATA * room,
    CHAR_DATA * victim, int dam_type, int level, int dam, int sn)
{
    const DAM_TYPE *damt;
    CHAR_DATA *vch, *vch_next;
    EFFECT_FUN *effect;
    bool is_area_spell;

    /* get damage-type specific information. */
    damt = dam_get (dam_type);
    effect = (damt && damt->effect) ? damt->effect : empty_effect;

    /* effect and partial damage to room. */
    is_area_spell = (room != NULL) ? TRUE : FALSE;
    if (is_area_spell) {
        effect (room, level, dam / 2, TARGET_ROOM);
        for (vch = room->people; vch != NULL; vch = vch_next) {
            vch_next = vch->next_in_room;
            if (vch == victim)
                continue;

            if (is_safe_spell (ch, vch, is_area_spell)
                || (IS_NPC (vch) && IS_NPC (ch)
                    && (ch->fighting != vch || vch->fighting != ch)))
                continue;

            if (saves_spell (level - 2, vch, dam_type)) {
                effect (vch, level / 4, dam / 8, TARGET_CHAR);
                damage (ch, vch, dam / 4, sn, dam_type, TRUE);
            }
            else {
                effect (vch, level / 2, dam / 4, TARGET_CHAR);
                damage (ch, vch, dam / 2, sn, dam_type, TRUE);
            }
        }
    }

    /* full damage to victim. */
    if (victim != NULL) {
        if (is_safe_spell (ch, victim, is_area_spell)
            || (IS_NPC (victim) && IS_NPC (ch)
                && (ch->fighting != victim || victim->fighting != ch)))
            ;
        else {
            if (saves_spell (level, victim, dam_type)) {
                effect (victim, level / 2, dam / 4, TARGET_CHAR);
                damage (ch, victim, dam / 2, sn, dam_type, TRUE);
            }
            else {
                effect (victim, level, dam, TARGET_CHAR);
                damage (ch, victim, dam, sn, dam_type, TRUE);
            }
        }
    }
}

DEFINE_SPELL_FUN (spell_acid_breath) {
    CHAR_DATA *victim = (CHAR_DATA *) vo;
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
    CHAR_DATA *victim = (CHAR_DATA *) vo;
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
    CHAR_DATA *victim = (CHAR_DATA *) vo;
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
    CHAR_DATA *victim = (CHAR_DATA *) vo;
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
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

    dam = number_range (25, 100);
    if (saves_spell (level, victim, DAM_PIERCE))
        dam /= 2;
    damage (ch, victim, dam, sn, DAM_PIERCE, TRUE);
}

DEFINE_SPELL_FUN (spell_high_explosive) {
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

    dam = number_range (30, 120);
    if (saves_spell (level, victim, DAM_PIERCE))
        dam /= 2;
    damage (ch, victim, dam, sn, DAM_PIERCE, TRUE);
}
