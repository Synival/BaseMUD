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
#include "affects.h"
#include "comm.h"
#include "chars.h"
#include "fight.h"
#include "groups.h"
#include "skills.h"
#include "objs.h"
#include "recycle.h"
#include "lookup.h"

#include "spell_aff.h"

/* TODO: spell_enchant_weapon() and spell_enchant_armor() are
 *       almost identical. unify them! */

DEFINE_SPELL_FUN (spell_armor) {
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if (is_affected_with_act (victim, sn, 0, ch,
            "You are already armored.",
            "$N is already armored."))
        return;

    affect_init (&af, TO_AFFECTS, sn, level, 24, APPLY_AC, -20, 0);
    affect_to_char (victim, &af);

    send_to_char ("You feel someone protecting you.\n\r", victim);
    if (ch != victim)
        act ("$N is protected by your magic.", ch, NULL, victim, TO_CHAR);
}

DEFINE_SPELL_FUN (spell_bless_object) {
    OBJ_DATA *obj = (OBJ_DATA *) vo;
    AFFECT_DATA af;

    BAIL_IF_ACT (IS_OBJ_STAT (obj, ITEM_BLESS),
        "$p is already blessed.", ch, obj, NULL);

    if (IS_OBJ_STAT (obj, ITEM_EVIL)) {
        AFFECT_DATA *paf = affect_find (obj->affected, gsn_curse);
        int paf_level = (paf != NULL) ? paf->level : obj->level;

        BAIL_IF_ACT (saves_dispel (level, paf_level, 0),
            "The evil of $p is too powerful for you to overcome.",
                ch, obj, NULL);

        if (paf != NULL)
            affect_remove_obj (obj, paf);
        act ("$p glows a pale blue.", ch, obj, NULL, TO_ALL);
        REMOVE_BIT (obj->extra_flags, ITEM_EVIL);
        return;
    }

    affect_init (&af, TO_OBJECT, sn, level, 6 + level, APPLY_SAVES, -1, ITEM_BLESS);
    affect_to_obj (obj, &af);

    act ("$p glows with a holy aura.", ch, obj, NULL, TO_ALL);
    if (obj->wear_loc != WEAR_NONE)
        ch->saving_throw -= 1;
}

DEFINE_SPELL_FUN (spell_bless_char) {
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if (is_affected_with_act (victim, sn, 0, ch,
            "You are already blessed.",
            "$N already has divine favor."))
        return;

    affect_init (&af, TO_AFFECTS, sn, level, 6 + level, APPLY_HITROLL, level / 8, 0);
    affect_to_char (victim, &af);

    af.apply = APPLY_SAVING_SPELL;
    af.modifier = 0 - level / 8;
    affect_to_char (victim, &af);

    send_to_char ("You feel righteous.\n\r", victim);
    if (ch != victim)
        act ("You grant $N the favor of your god.", ch, NULL, victim, TO_CHAR);
}

DEFINE_SPELL_FUN (spell_bless) {
    if (target == TARGET_OBJ)
        spell_bless_object (sn, level, ch, vo, target);
    else
        spell_bless_char (sn, level, ch, vo, target);
}

DEFINE_SPELL_FUN (spell_blindness)
    { spell_perform_blindness (sn, level, ch, (CHAR_DATA *) vo, FALSE); }
DEFINE_SPELL_FUN (spell_blindness_quiet)
    { spell_perform_blindness (sn, level, ch, (CHAR_DATA *) vo, TRUE); }

void spell_perform_blindness (int sn, int level, CHAR_DATA *ch,
    CHAR_DATA *victim, bool quiet)
{
    AFFECT_DATA af;

    BAIL_IF_ACT (saves_spell (level, victim, DAM_OTHER),
        quiet ? NULL : "$E resists your spell!", ch, NULL, victim);
    BAIL_IF (IS_AFFECTED (victim, AFF_BLIND),
        quiet ? NULL : "It doesn't seem to have an effect...\n\r", ch);

    affect_init (&af, TO_AFFECTS, sn, level, 1 + level, APPLY_HITROLL, -4, AFF_BLIND);
    affect_to_char (victim, &af);

    send_to_char ("You are blinded!\n\r", victim);
    act ("$n appears to be blinded.", victim, NULL, NULL, TO_NOTCHAR);
}

/* RT calm spell stops all fighting in the room */
DEFINE_SPELL_FUN (spell_calm) {
    CHAR_DATA *vch;
    int mlevel = 0;
    int count;
    int high_level = 0;
    int chance;
    AFFECT_DATA af;

    /* get sum of all mobile levels in the room */
    count = 0;
    for (vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room) {
        if (!(vch->fighting || vch->position == POS_FIGHTING))
            continue;

        count++;
        if (IS_NPC (vch))
            mlevel += vch->level;
        else
            mlevel += vch->level / 2;
        high_level = UMAX (high_level, vch->level);
    }

    /* compute chance of stopping combat */
    chance = 4 * level - high_level + 2 * count;

    if (IS_IMMORTAL (ch)) /* always works */
        mlevel = 0;

    /* hard to stop large fights */
    if (number_range (0, chance) < mlevel)
        return;

    count = 0;
    for (vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room) {
        if (!(vch->fighting || vch->position == POS_FIGHTING))
            continue;
        if (IS_NPC (vch) && (IS_SET (vch->imm_flags, RES_MAGIC) ||
                             IS_SET (vch->mob, MOB_UNDEAD)))
            continue;
        if (IS_AFFECTED (vch, AFF_CALM) || IS_AFFECTED (vch, AFF_BERSERK) ||
                is_affected (vch, skill_lookup ("frenzy")))
            continue;

        send_to_char ("A wave of calm passes over you.\n\r", vch);
        act ("$n suddenly calms down.", vch, NULL, NULL, TO_NOTCHAR);

        if (vch->fighting || vch->position == POS_FIGHTING)
            stop_fighting (vch, FALSE);

        affect_init (&af, TO_AFFECTS, sn, level, level / 4, APPLY_HITROLL, (IS_NPC (vch) ? -2 : -5), AFF_CALM);
        affect_to_char (vch, &af);

        af.apply = APPLY_DAMROLL;
        affect_to_char (vch, &af);
        count++;
    }
    if (count == 0)
        send_to_char ("Nothing seems to happen.\n\r", ch);
}

DEFINE_SPELL_FUN (spell_change_sex) {
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;
    int mod;

    if (is_affected_with_act (victim, sn, 0, ch,
            "You've already been changed.",
            "$N has already had $s(?) sex changed."))
        return;
    BAIL_IF_ACT (saves_spell (level, victim, DAM_OTHER),
        "$S body refuses to change form.\n\r", ch, NULL, victim);

    send_to_char ("You feel different.\n\r", victim);
    act ("$n doesn't look like $mself anymore...", victim, NULL, NULL,
         TO_NOTCHAR);

    /* TODO: WHAT IS THIS?! so we're *tweaking* the sex by a positive or
     * negative amount (see handlers.c)... what if we apply it multiple
     * times????  what if we check it here, but another affect (possibly
     * from an item) is removed afterwards?
     * -- Synival */
    do {
        mod = number_range (0, 2) - victim->sex;
    } while (mod == 0);

    affect_init (&af, TO_AFFECTS, sn, level, 2 * level, APPLY_SEX, mod, 0);
    affect_to_char (victim, &af);
}

DEFINE_SPELL_FUN (spell_charm_person) {
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if (do_filter_can_attack (ch, victim))
        return;

    BAIL_IF (victim == ch,
        "You like yourself even better!\n\r", ch);
    BAIL_IF (IS_AFFECTED (ch, AFF_CHARM),
        "I'm not sure your master would like that.\n\r", ch);
    BAIL_IF_ACT (IS_AFFECTED (victim, AFF_CHARM),
        "$E's already under someone else's influence.\n\r", ch, NULL, victim);
    BAIL_IF_ACT (level < victim->level ||
                IS_SET (victim->imm_flags, RES_CHARM) ||
                saves_spell (level, victim, DAM_CHARM),
        "$E resists your charms.\n\r", ch, NULL, victim);
    BAIL_IF (IS_SET (victim->in_room->room_flags, ROOM_LAW),
        "The mayor does not allow charming in the city limits.\n\r", ch);

    if (victim->master)
        stop_follower (victim);
    add_follower (victim, ch);
    victim->leader = ch;

    affect_init (&af, TO_AFFECTS, sn, level, number_fuzzy (level / 4), 0, 0, AFF_CHARM);
    affect_to_char (victim, &af);

    act ("Isn't $n just so nice?", ch, NULL, victim, TO_VICT);
    if (ch != victim)
        act ("$N looks at you with adoring eyes.", ch, NULL, victim, TO_CHAR);
}

DEFINE_SPELL_FUN (spell_curse_object) {
    OBJ_DATA *obj = (OBJ_DATA *) vo;
    AFFECT_DATA af;

    BAIL_IF_ACT (IS_OBJ_STAT (obj, ITEM_EVIL),
        "$p is already filled with evil.", ch, obj, NULL);

    if (IS_OBJ_STAT (obj, ITEM_BLESS)) {
        AFFECT_DATA *paf;

        paf = affect_find (obj->affected, skill_lookup ("bless"));
        BAIL_IF_ACT (saves_dispel (level, paf != NULL
                ? paf->level : obj->level, 0),
            "The holy aura of $p is too powerful for you to overcome.",
                ch, obj, NULL);

        if (paf != NULL)
            affect_remove_obj (obj, paf);
        act ("$p glows with a red aura.", ch, obj, NULL, TO_ALL);
        REMOVE_BIT (obj->extra_flags, ITEM_BLESS);
        return;
    }

    affect_init (&af, TO_OBJECT, sn, level, 2 * level, APPLY_SAVES, 1, ITEM_EVIL);
    affect_to_obj (obj, &af);

    act ("$p glows with a malevolent aura.", ch, obj, NULL, TO_ALL);
    if (obj->wear_loc != WEAR_NONE)
        ch->saving_throw += 1;
}

DEFINE_SPELL_FUN (spell_curse_char)
    { spell_perform_curse_char (sn, level, ch, vo, FALSE); }
DEFINE_SPELL_FUN (spell_curse_char_quiet)
    { spell_perform_curse_char (sn, level, ch, vo, TRUE); }

void spell_perform_curse_char (int sn, int level, CHAR_DATA *ch,
    CHAR_DATA *victim, bool quiet)
{
    AFFECT_DATA af;

    BAIL_IF_ACT (saves_spell (level, victim, DAM_NEGATIVE),
        quiet ? NULL : "$E resists your spell!", ch, NULL, victim);
    BAIL_IF (IS_AFFECTED (victim, AFF_CURSE),
        quiet ? NULL : "It doesn't seem to have an effect...\n\r", ch);

    affect_init (&af, TO_AFFECTS, sn, level, 2 * level, APPLY_HITROLL, -1 * (level / 8), AFF_CURSE);
    affect_to_char (victim, &af);

    af.apply  = APPLY_SAVING_SPELL;
    af.modifier  = level / 8;
    affect_to_char (victim, &af);

    send_to_char ("You feel unclean.\n\r", victim);
    act ("$n looks very uncomfortable.", victim, NULL, NULL, TO_NOTCHAR);
}

DEFINE_SPELL_FUN (spell_curse) {
    if (target == TARGET_OBJ)
        spell_curse_object (sn, level, ch, vo, target);
    else
        spell_curse_char (sn, level, ch, vo, target);
}

DEFINE_SPELL_FUN (spell_detect_evil) {
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if (is_affected_with_act (victim, -1, AFF_DETECT_EVIL, ch,
            "You can already sense evil.",
            "$N can already detect evil."))
        return;

    affect_init (&af, TO_AFFECTS, sn, level, level, APPLY_NONE, 0, AFF_DETECT_EVIL);
    affect_to_char (victim, &af);

    send_to_char ("Your eyes tingle.\n\r", victim);
    if (ch != victim)
        send_to_char ("Ok.\n\r", ch);
}

DEFINE_SPELL_FUN (spell_detect_good) {
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if (is_affected_with_act (victim, -1, AFF_DETECT_GOOD, ch,
            "You can already sense good.",
            "$N can already detect good."))
        return;

    affect_init (&af, TO_AFFECTS, sn, level, level, APPLY_NONE, 0, AFF_DETECT_GOOD);
    affect_to_char (victim, &af);

    send_to_char ("Your eyes tingle.\n\r", victim);
    if (ch != victim)
        send_to_char ("Ok.\n\r", ch);
}

DEFINE_SPELL_FUN (spell_detect_hidden) {
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if (is_affected_with_act (victim, -1, AFF_DETECT_HIDDEN, ch,
            "You are already as alert as you can be.",
            "$N can already sense hidden lifeforms."))
        return;

    affect_init (&af, TO_AFFECTS, sn, level, level, APPLY_NONE, 0, AFF_DETECT_HIDDEN);
    affect_to_char (victim, &af);

    send_to_char ("Your awareness improves.\n\r", victim);
    if (ch != victim)
        send_to_char ("Ok.\n\r", ch);
}

DEFINE_SPELL_FUN (spell_detect_invis) {
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if (is_affected_with_act (victim, -1, AFF_DETECT_INVIS, ch,
            "You can already see invisible.",
            "$N can already see invisible things."))
        return;

    affect_init (&af, TO_AFFECTS, sn, level, level, APPLY_NONE, 0, AFF_DETECT_INVIS);
    affect_to_char (victim, &af);

    send_to_char ("Your eyes tingle.\n\r", victim);
    if (ch != victim)
        send_to_char ("Ok.\n\r", ch);
}

DEFINE_SPELL_FUN (spell_detect_magic) {
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if (is_affected_with_act (victim, -1, AFF_DETECT_MAGIC, ch,
            "You can already sense magical auras.",
            "$N can already detect magic."))
        return;

    affect_init (&af, TO_AFFECTS, sn, level, level, APPLY_NONE, 0, AFF_DETECT_MAGIC);
    affect_to_char (victim, &af);

    send_to_char ("Your eyes tingle.\n\r", victim);
    if (ch != victim)
        send_to_char ("Ok.\n\r", ch);
}

DEFINE_SPELL_FUN (spell_enchant_armor) {
    OBJ_DATA *obj = (OBJ_DATA *) vo;
    AFFECT_DATA *paf;
    int result, fail;
    int ac_bonus, added;
    bool ac_found = FALSE;

    BAIL_IF (obj->item_type != ITEM_ARMOR,
        "That isn't armor.\n\r", ch);
    BAIL_IF (obj->wear_loc != -1,
        "The item must be carried to be enchanted.\n\r", ch);

    ac_bonus = 0; /* this means they have no bonus */
    fail = 25;    /* base 25% chance of failure */

    /* find the bonuses */
    if (!obj->enchanted) {
        for (paf = obj->pIndexData->affected; paf != NULL; paf = paf->next) {
            if (paf->apply == APPLY_AC) {
                ac_bonus = paf->modifier;
                ac_found = TRUE;
                fail += 5 * (ac_bonus * ac_bonus);
            }
            else /* things get a little harder */
                fail += 20;
        }
    }

    for (paf = obj->affected; paf != NULL; paf = paf->next) {
        if (paf->apply == APPLY_AC) {
            ac_bonus = paf->modifier;
            ac_found = TRUE;
            fail += 5 * (ac_bonus * ac_bonus);
        }
        else /* things get a little harder */
            fail += 20;
    }

    /* apply other modifiers */
    fail -= level;
    if (IS_OBJ_STAT (obj, ITEM_BLESS))
        fail -= 15;
    if (IS_OBJ_STAT (obj, ITEM_GLOW))
        fail -= 5;
    fail = URANGE (5, fail, 85);

    /* the moment of truth */
    result = number_percent ();
    if (result < (fail / 5)) { /* item destroyed */
        act ("$p flares blindingly... and evaporates!", ch, obj, NULL, TO_ALL);
        obj_extract (obj);
        return;
    }

    /* item disenchanted */
    if (result < (fail / 3)) {
        AFFECT_DATA *paf_next;
        act ("$p glows brightly, then fades... oops.", ch, obj, NULL, TO_CHAR);
        act ("$p glows brightly, then fades.", ch, obj, NULL, TO_NOTCHAR);
        obj->enchanted = TRUE;

        /* remove all affects */
        for (paf = obj->affected; paf != NULL; paf = paf_next) {
            paf_next = paf->next;
            affect_free (paf);
        }
        obj->affected = NULL;

        /* clear all flags */
        obj->extra_flags = 0;
        return;
    }

    /* failed, no bad result */
    BAIL_IF (result <= fail,
        "Nothing seemed to happen.\n\r", ch);

    /* make sure our object is enchanted. */
    obj_enchant (obj);

    /* normal enchantment. */
    if (result <= (90 - level / 5)) { /* success! */
        act ("$p shimmers with a gold aura.", ch, obj, NULL, TO_ALL);
        SET_BIT (obj->extra_flags, ITEM_MAGIC);
        added = -1;
    }
    /* exceptional enchant */
    else {
        act ("$p glows a brillant gold!", ch, obj, NULL, TO_ALL);
        SET_BIT (obj->extra_flags, ITEM_MAGIC);
        SET_BIT (obj->extra_flags, ITEM_GLOW);
        added = -2;
    }

    /* now add the enchantments */
    if (obj->level < LEVEL_HERO - 1)
        obj->level = UMIN (LEVEL_HERO - 1, obj->level + 1);

    if (ac_found) {
        for (paf = obj->affected; paf != NULL; paf = paf->next) {
            if (paf->apply == APPLY_AC) {
                paf->type = sn;
                paf->modifier += added;
                paf->level = UMAX (paf->level, level);
            }
        }
    }
    /* add a new affect */
    else {
        paf = affect_new ();
        affect_init (paf, TO_OBJECT, sn, level, -1, APPLY_AC, added, 0);
        LIST_FRONT (paf, next, obj->affected);
    }
}

DEFINE_SPELL_FUN (spell_enchant_weapon) {
    OBJ_DATA *obj = (OBJ_DATA *) vo;
    AFFECT_DATA *paf;
    int result, fail;
    int hit_bonus, dam_bonus, added;
    bool hit_found = FALSE, dam_found = FALSE;

    BAIL_IF (obj->item_type != ITEM_WEAPON,
        "That isn't a weapon.\n\r", ch);
    BAIL_IF (obj->wear_loc != -1,
        "The item must be carried to be enchanted.\n\r", ch);

    hit_bonus = 0;  /* this means they have no bonus */
    dam_bonus = 0;
    fail      = 25; /* base 25% chance of failure */

    /* find the bonuses */
    if (!obj->enchanted) {
        for (paf = obj->pIndexData->affected; paf != NULL; paf = paf->next) {
            if (paf->apply == APPLY_HITROLL) {
                hit_bonus = paf->modifier;
                hit_found = TRUE;
                fail += 2 * (hit_bonus * hit_bonus);
            }
            else if (paf->apply == APPLY_DAMROLL) {
                dam_bonus = paf->modifier;
                dam_found = TRUE;
                fail += 2 * (dam_bonus * dam_bonus);
            }
            else /* things get a little harder */
                fail += 25;
        }
    }

    for (paf = obj->affected; paf != NULL; paf = paf->next) {
        if (paf->apply == APPLY_HITROLL) {
            hit_bonus = paf->modifier;
            hit_found = TRUE;
            fail += 2 * (hit_bonus * hit_bonus);
        }
        else if (paf->apply == APPLY_DAMROLL) {
            dam_bonus = paf->modifier;
            dam_found = TRUE;
            fail += 2 * (dam_bonus * dam_bonus);
        }
        else /* things get a little harder */
            fail += 25;
    }

    /* apply other modifiers */
    fail -= 3 * level / 2;
    if (IS_OBJ_STAT (obj, ITEM_BLESS))
        fail -= 15;
    if (IS_OBJ_STAT (obj, ITEM_GLOW))
        fail -= 5;
    fail = URANGE (5, fail, 95);

    /* the moment of truth */
    result = number_percent ();
    if (result < (fail / 5)) { /* item destroyed */
        act ("$p shivers violently and explodes!",  ch, obj, NULL, TO_ALL);
        obj_extract (obj);
        return;
    }

    /* item disenchanted */
    if (result < (fail / 2)) {
        AFFECT_DATA *paf_next;
        act ("$p glows brightly, then fades... oops.", ch, obj, NULL, TO_CHAR);
        act ("$p glows brightly, then fades.", ch, obj, NULL, TO_NOTCHAR);
        obj->enchanted = TRUE;

        /* remove all affects */
        for (paf = obj->affected; paf != NULL; paf = paf_next) {
            paf_next = paf->next;
            affect_free (paf);
        }
        obj->affected = NULL;

        /* clear all flags */
        obj->extra_flags = 0;
        return;
    }

    /* failed, no bad result */
    BAIL_IF (result <= fail,
        "Nothing seemed to happen.\n\r", ch);

    /* make sure our object is enchanted. */
    obj_enchant (obj);

    /* normal enchantment. */
    if (result <= (100 - level / 5)) { /* success! */
        act ("$p glows blue.", ch, obj, NULL, TO_ALL);
        SET_BIT (obj->extra_flags, ITEM_MAGIC);
        added = 1;
    }
    /* exceptional enchant */
    else {
        act ("$p glows a brillant blue!", ch, obj, NULL, TO_ALL);
        SET_BIT (obj->extra_flags, ITEM_MAGIC);
        SET_BIT (obj->extra_flags, ITEM_GLOW);
        added = 2;
    }

    /* now add the enchantments */
    if (obj->level < LEVEL_HERO - 1)
        obj->level = UMIN (LEVEL_HERO - 1, obj->level + 1);

    if (dam_found) {
        for (paf = obj->affected; paf != NULL; paf = paf->next) {
            if (paf->apply == APPLY_DAMROLL) {
                paf->type = sn;
                paf->modifier += added;
                paf->level = UMAX (paf->level, level);
                if (paf->modifier > 4)
                    SET_BIT (obj->extra_flags, ITEM_HUM);
            }
        }
    }
    /* add a new affect */
    else {
        paf = affect_new ();
        affect_init (paf, TO_OBJECT, sn, level, -1, APPLY_DAMROLL, added, 0);
        LIST_FRONT (paf, next, obj->affected);
    }

    if (hit_found) {
        for (paf = obj->affected; paf != NULL; paf = paf->next) {
            if (paf->apply == APPLY_HITROLL) {
                paf->type = sn;
                paf->modifier += added;
                paf->level = UMAX (paf->level, level);
                if (paf->modifier > 4)
                    SET_BIT (obj->extra_flags, ITEM_HUM);
            }
        }
    }
    /* add a new affect */
    else {
        paf = affect_new ();
        affect_init (paf, TO_OBJECT, sn, level, -1, APPLY_HITROLL, added, 0);
        LIST_FRONT (paf, next, obj->affected);
    }
}

DEFINE_SPELL_FUN (spell_fireproof) {
    OBJ_DATA *obj = (OBJ_DATA *) vo;
    AFFECT_DATA af;

    BAIL_IF_ACT (IS_OBJ_STAT (obj, ITEM_BURN_PROOF),
        "$p is already protected from burning.", ch, obj, NULL);

    affect_init (&af, TO_OBJECT, sn, level, number_fuzzy (level / 4), APPLY_NONE, 0, ITEM_BURN_PROOF);
    affect_to_obj (obj, &af);

    act ("You protect $p from fire.", ch, obj, NULL, TO_CHAR);
    act ("$p is surrounded by a protective aura.", ch, obj, NULL, TO_NOTCHAR);
}

DEFINE_SPELL_FUN (spell_faerie_fire) {
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    BAIL_IF_ACT (IS_AFFECTED (victim, AFF_FAERIE_FIRE),
        "The pink aura around $N seems unaffected.", ch, NULL, victim);
    affect_init (&af, TO_AFFECTS, sn, level, level, APPLY_AC, 2 * level, AFF_FAERIE_FIRE);
    affect_to_char (victim, &af);

    send_to_char ("You are surrounded by a pink outline.\n\r", victim);
    act ("$n is surrounded by a pink outline.", victim, NULL, NULL, TO_NOTCHAR);
}

DEFINE_SPELL_FUN (spell_faerie_fog) {
    CHAR_DATA *ich;

    send_to_char ("You conjure a cloud of purple smoke.\n\r", ch);
    act ("$n conjures a cloud of purple smoke.", ch, NULL, NULL, TO_NOTCHAR);

    for (ich = ch->in_room->people; ich != NULL; ich = ich->next_in_room) {
        if (ich->invis_level > 0)
            continue;
        if (ich == ch || saves_spell (level, ich, DAM_OTHER))
            continue;

        affect_strip (ich, gsn_invis);
        affect_strip (ich, gsn_mass_invis);
        affect_strip (ich, gsn_sneak);
        REMOVE_BIT (ich->affected_by, AFF_HIDE);
        REMOVE_BIT (ich->affected_by, AFF_INVISIBLE);
        REMOVE_BIT (ich->affected_by, AFF_SNEAK);
        send_to_char ("You are revealed!\n\r", ich);
        act ("$n is revealed!", ich, NULL, NULL, TO_NOTCHAR);
    }
}

DEFINE_SPELL_FUN (spell_fly) {
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if (is_affected_with_act (victim, -1, AFF_FLYING, ch,
            "You are already airborne.",
            "$N doesn't need your help to fly."))
        return;

    affect_init (&af, TO_AFFECTS, sn, level, level + 3, 0, 0, AFF_FLYING);
    affect_to_char (victim, &af);

    send_to_char ("Your feet rise off the ground.\n\r", victim);
    act ("$n's feet rise off the ground.", victim, NULL, NULL, TO_NOTCHAR);
}

/* RT clerical berserking spell */
DEFINE_SPELL_FUN (spell_frenzy) {
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if (is_affected_with_act (victim, sn, AFF_BERSERK, ch,
            "You are already in a frenzy.",
            "$N is already in a frenzy."))
        return;

    if (is_affected_with_act (victim, skill_lookup ("calm"), 0, ch,
            "Why don't you just relax for a while?",
            "$N doesn't look like $e wants to fight anymore."))
        return;

    BAIL_IF_ACT (!IS_SAME_ALIGN (ch, victim),
        "Your god doesn't seem to like $N.", ch, NULL, victim);

    affect_init (&af, TO_AFFECTS, sn, level, level / 3, APPLY_HITROLL, level / 6, 0);
    affect_to_char (victim, &af);

    af.apply = APPLY_DAMROLL;
    affect_to_char (victim, &af);

    af.modifier = 10 * (level / 12);
    af.apply = APPLY_AC;
    affect_to_char (victim, &af);

    send_to_char ("You are filled with holy wrath!\n\r", victim);
    act ("$n gets a wild look in $s eyes!", victim, NULL, NULL, TO_NOTCHAR);
}

DEFINE_SPELL_FUN (spell_giant_strength) {
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if (is_affected_with_act (victim, sn, 0, ch,
            "You are already as strong as you can get!",
            "$N can't get any stronger."))
        return;

    affect_init (&af, TO_AFFECTS, sn, level, level, APPLY_STR, 1 + (level >= 18) + (level >= 25) + (level >= 32), 0);
    affect_to_char (victim, &af);

    send_to_char ("Your muscles surge with heightened power!\n\r", victim);
    act ("$n's muscles surge with heightened power.", victim, NULL, NULL, TO_NOTCHAR);
}

/* RT haste spell */
DEFINE_SPELL_FUN (spell_haste) {
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if (is_affected (victim, sn) || IS_AFFECTED (victim, AFF_HASTE)
        || IS_SET (victim->off_flags, OFF_FAST))
    {
        if (victim == ch)
            send_to_char ("You can't move any faster!\n\r", ch);
        else
            act ("$N is already moving as fast as $E can.", ch, NULL, victim, TO_CHAR);
        return;
    }

    if (IS_AFFECTED (victim, AFF_SLOW)) {
        if (!check_dispel (level, victim, skill_lookup ("slow"))) {
            if (victim != ch)
                send_to_char ("Spell failed.\n\r", ch);
            send_to_char ("You feel momentarily faster, then it passes.\n\r", victim);
            return;
        }
        if (victim == ch)
            send_to_char ("You are moving less slowly.\n\r", ch);
        act ("$n is moving less slowly.", victim, NULL, NULL, TO_NOTCHAR);
        return;
    }

    affect_init (&af, TO_AFFECTS, sn, level, level / ((victim == ch) ? 2 : 4), APPLY_DEX, 1 + (level >= 18) + (level >= 25) + (level >= 32), AFF_HASTE);
    affect_to_char (victim, &af);

    send_to_char ("You feel yourself moving more quickly.\n\r", victim);
    act ("$n is moving more quickly.", victim, NULL, NULL, TO_NOTCHAR);
}

DEFINE_SPELL_FUN (spell_infravision) {
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if (is_affected_with_act (victim, -1, AFF_INFRARED, ch,
            "You can already see in the dark.",
            "$N already has infravision."))
        return;

    affect_init (&af, TO_AFFECTS, sn, level, 2 * level, APPLY_NONE, 0, AFF_INFRARED);
    affect_to_char (victim, &af);

    send_to_char ("Your eyes glow red.\n\r", victim);
    act ("$n's eyes glow red.\n\r", ch, NULL, NULL, TO_NOTCHAR);
}

DEFINE_SPELL_FUN (spell_invis_object) {
    OBJ_DATA *obj = (OBJ_DATA *) vo;
    AFFECT_DATA af;

    BAIL_IF_ACT (IS_OBJ_STAT (obj, ITEM_INVIS),
        "$p is already invisible.", ch, obj, NULL);

    act ("$p fades out of sight.", ch, obj, NULL, TO_ALL);
    affect_init (&af, TO_OBJECT, sn, level, level + 12, APPLY_NONE, 0, ITEM_INVIS);
    affect_to_obj (obj, &af);
}

DEFINE_SPELL_FUN (spell_invis_char) {
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    victim = (CHAR_DATA *) vo;
    if (is_affected_with_act (victim, -1, AFF_INVISIBLE, ch,
            "You are already invisible.",
            "$N is already invisible."))
        return;

    send_to_char ("You fade out of existence.\n\r", victim);
    act ("$n fades out of existence.", victim, NULL, NULL, TO_NOTCHAR);

    affect_init (&af, TO_AFFECTS, sn, level, level + 12, APPLY_NONE, 0, AFF_INVISIBLE);
    affect_to_char (victim, &af);
}

DEFINE_SPELL_FUN (spell_invis) {
    if (target == TARGET_OBJ)
        spell_invis_object (sn, level, ch, vo, target);
    else
        spell_invis_char (sn, level, ch, vo, target);
}

DEFINE_SPELL_FUN (spell_mass_invis) {
    AFFECT_DATA af;
    CHAR_DATA *gch;
    bool found = FALSE;

    for (gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room) {
        if (!is_same_group (gch, ch) || IS_AFFECTED (gch, AFF_INVISIBLE))
            continue;
        send_to_char ("You slowly fade out of existence.\n\r", gch);
        act ("$n slowly fades out of existence.", gch, NULL, NULL, TO_NOTCHAR);

        affect_init (&af, TO_AFFECTS, sn, level / 2, 24, APPLY_NONE, 0, AFF_INVISIBLE);
        affect_to_char (gch, &af);
        found = TRUE;
    }
    if (!found)
        send_to_char ("Nothing happens.\n\r", ch);
}

DEFINE_SPELL_FUN (spell_pass_door) {
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if (is_affected_with_act (victim, -1, AFF_PASS_DOOR, ch,
            "You are already out of phase.",
            "$N is already shifted out of phase."))
        return;

    affect_init (&af, TO_AFFECTS, sn, level, number_fuzzy (level / 4), APPLY_NONE, 0, AFF_PASS_DOOR);
    affect_to_char (victim, &af);

    send_to_char ("You turn translucent.\n\r", victim);
    act ("$n turns translucent.", victim, NULL, NULL, TO_NOTCHAR);
}

/* RT plague spell, very nasty */
DEFINE_SPELL_FUN (spell_plague) {
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if (saves_spell (level, victim, DAM_DISEASE) ||
        (IS_NPC (victim) && IS_SET (victim->mob, MOB_UNDEAD)))
    {
        if (ch == victim)
            send_to_char ("You feel momentarily ill, but it passes.\n\r", ch);
        else
            act ("$N seems to be unaffected.", ch, NULL, victim, TO_CHAR);
        return;
    }

    affect_init (&af, TO_AFFECTS, sn, level * 3 / 4, level, APPLY_STR, -5, AFF_PLAGUE);
    affect_join (victim, &af);

    send_to_char (
        "You scream in agony as plague sores erupt from your skin.\n\r",
        victim);
    act ("$n screams in agony as plague sores erupt from $s skin.", victim,
        NULL, NULL, TO_NOTCHAR);
}

DEFINE_SPELL_FUN (spell_poison_object) {
    OBJ_DATA *obj = (OBJ_DATA *) vo;
    AFFECT_DATA af;

    if (obj->item_type == ITEM_FOOD || obj->item_type == ITEM_DRINK_CON) {
        BAIL_IF_ACT (IS_OBJ_STAT (obj, ITEM_BLESS) ||
                     IS_OBJ_STAT (obj, ITEM_BURN_PROOF),
            "Your spell fails to corrupt $p.", ch, obj, NULL);
        BAIL_IF_ACT (obj->value[3] == 1,
            "$p is already poisoned.", ch, obj, NULL);

        obj->value[3] = 1;
        act ("$p is infused with poisonous vapors.", ch, obj, NULL, TO_ALL);
        return;
    }

    if (obj->item_type == ITEM_WEAPON) {
        BAIL_IF_ACT (IS_WEAPON_STAT (obj, WEAPON_FLAMING) ||
                     IS_WEAPON_STAT (obj, WEAPON_FROST) ||
                     IS_WEAPON_STAT (obj, WEAPON_VAMPIRIC) ||
                     IS_WEAPON_STAT (obj, WEAPON_SHARP) ||
                     IS_WEAPON_STAT (obj, WEAPON_VORPAL) ||
                     IS_WEAPON_STAT (obj, WEAPON_SHOCKING) ||
                     IS_OBJ_STAT (obj, ITEM_BLESS) ||
                     IS_OBJ_STAT (obj, ITEM_BURN_PROOF),
            "You can't seem to envenom $p.", ch, obj, NULL);
        BAIL_IF_ACT (IS_WEAPON_STAT (obj, WEAPON_POISON),
            "$p is already envenomed.", ch, obj, NULL);

        affect_init (&af, TO_WEAPON, sn, level / 2, level / 8, 0, 0, WEAPON_POISON);
        affect_to_obj (obj, &af);

        act ("$p is coated with deadly venom.", ch, obj, NULL, TO_ALL);
        return;
    }

    act ("You can't poison $p.", ch, obj, NULL, TO_CHAR);
}

DEFINE_SPELL_FUN (spell_poison_char) {
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if (saves_spell (level, victim, DAM_POISON)) {
        send_to_char ("You feel momentarily ill, but it passes.\n\r", victim);
        act ("$n turns slightly green, but it passes.", victim, NULL, NULL,
             TO_NOTCHAR);
        return;
    }

    affect_init (&af, TO_AFFECTS, sn, level, level, APPLY_STR, -2, AFF_POISON);
    affect_join (victim, &af);

    send_to_char ("You feel very sick.\n\r", victim);
    act ("$n looks very ill.", victim, NULL, NULL, TO_NOTCHAR);
}

DEFINE_SPELL_FUN (spell_poison) {
    if (target == TARGET_OBJ)
        spell_poison_object (sn, level, ch, vo, target);
    else
        spell_poison_char (sn, level, ch, vo, target);
}

DEFINE_SPELL_FUN (spell_protection_evil) {
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if (IS_AFFECTED (victim, AFF_PROTECT_EVIL) ||
        IS_AFFECTED (victim, AFF_PROTECT_GOOD))
    {
        if (victim == ch)
            send_to_char ("You are already protected.\n\r", ch);
        else
            act ("$N is already protected.", ch, NULL, victim, TO_CHAR);
        return;
    }

    affect_init (&af, TO_AFFECTS, sn, level, 24, APPLY_SAVING_SPELL, -1, AFF_PROTECT_EVIL);
    affect_to_char (victim, &af);

    send_to_char ("You feel holy and pure.\n\r", victim);
    if (ch != victim)
        act ("$N is protected from evil.", ch, NULL, victim, TO_CHAR);
}

DEFINE_SPELL_FUN (spell_protection_good) {
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if (IS_AFFECTED (victim, AFF_PROTECT_GOOD)
     || IS_AFFECTED (victim, AFF_PROTECT_EVIL))
    {
        if (victim == ch)
            send_to_char ("You are already protected.\n\r", ch);
        else
            act ("$N is already protected.", ch, NULL, victim, TO_CHAR);
        return;
    }

    affect_init (&af, TO_AFFECTS, sn, level, 24, APPLY_SAVING_SPELL, -1, AFF_PROTECT_GOOD);
    affect_to_char (victim, &af);

    send_to_char ("You feel aligned with darkness.\n\r", victim);
    if (ch != victim)
        act ("$N is protected from good.", ch, NULL, victim, TO_CHAR);
    return;
}

DEFINE_SPELL_FUN (spell_sanctuary) {
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if (is_affected_with_act(victim, -1, AFF_SANCTUARY, ch,
            "You are already in sanctuary.",
            "$N is already in sanctuary."))
        return;

    affect_init (&af, TO_AFFECTS, sn, level, level / 6, APPLY_NONE, 0, AFF_SANCTUARY);
    affect_to_char (victim, &af);

    send_to_char ("You are surrounded by a white aura.\n\r", victim);
    act ("$n is surrounded by a white aura.", victim, NULL, NULL, TO_NOTCHAR);
}

DEFINE_SPELL_FUN (spell_shield) {
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if (is_affected_with_act (victim, sn, -1, ch,
            "You are already shielded from harm.",
            "$N is already protected by a shield."))
        return;

    affect_init (&af, TO_AFFECTS, sn, level, 8 + level, APPLY_AC, -20, 0);
    affect_to_char (victim, &af);

    act ("$n is surrounded by a force shield.", victim, NULL, NULL, TO_NOTCHAR);
    send_to_char ("You are surrounded by a force shield.\n\r", victim);
}

DEFINE_SPELL_FUN (spell_sleep) {
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    BAIL_IF_ACT (saves_spell (level - 4, victim, DAM_CHARM),
        "$e resists your spell!", ch, NULL, NULL);
    BAIL_IF (IS_AFFECTED (victim, AFF_SLEEP) ||
            (IS_NPC (victim) && IS_SET (victim->mob, MOB_UNDEAD)) ||
            (level + 2) < victim->level,
        "Nothing happens.\n\r", ch);

    affect_init (&af, TO_AFFECTS, sn, level, 4 + level, APPLY_NONE, 0, AFF_SLEEP);
    affect_join (victim, &af);

    if (!IS_AWAKE (victim)) {
        act ("$n falls into a deeper sleep.", victim, NULL, NULL, TO_NOTCHAR);
        return;
    }

    send_to_char ("You feel very sleepy ..... zzzzzz.\n\r", victim);
    act ("$n goes to sleep.", victim, NULL, NULL, TO_NOTCHAR);
    victim->position = POS_SLEEPING;
}

DEFINE_SPELL_FUN (spell_slow) {
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if (is_affected_with_act (victim, sn, AFF_SLOW, ch,
            "You can't move any slower!",
            "$N can't get any slower than that."))
        return;

    if (saves_spell (level, victim, DAM_OTHER)
        || IS_SET (victim->imm_flags, RES_MAGIC))
    {
        if (victim != ch)
            send_to_char ("Nothing seemed to happen.\n\r", ch);
        send_to_char ("You feel momentarily lethargic.\n\r", victim);
        return;
    }

    if (IS_AFFECTED (victim, AFF_HASTE)) {
        if (!check_dispel (level, victim, skill_lookup ("haste"))) {
            if (victim != ch)
                send_to_char ("Spell failed.\n\r", ch);
            send_to_char ("You feel momentarily slower, then is passes.\n\r", victim);
            return;
        }

        if (victim == ch)
            send_to_char ("You are moving less quickly.\n\r", ch);
        act ("$n is moving less quickly.", victim, NULL, NULL, TO_NOTCHAR);
        return;
    }

    affect_init (&af, TO_AFFECTS, sn, level, level / 2, APPLY_DEX, -1 - (level >= 18) - (level >= 25) - (level >= 32), AFF_SLOW);
    affect_to_char (victim, &af);

    send_to_char ("You feel yourself slowing d o w n...\n\r", victim);
    act ("$n starts to move in slow motion.", victim, NULL, NULL, TO_NOTCHAR);
}

DEFINE_SPELL_FUN (spell_stone_skin) {
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if (is_affected_with_act (victim, sn, 0, ch,
            "Your skin is already as hard as a rock.",
            "$N is already as hard as can be."))
        return;

    affect_init (&af, TO_AFFECTS, sn, level, level, APPLY_AC, -40, 0);
    affect_to_char (victim, &af);

    send_to_char ("Your skin turns to stone.\n\r", victim);
    act ("$n's skin turns to stone.", victim, NULL, NULL, TO_NOTCHAR);
}

DEFINE_SPELL_FUN (spell_weaken) {
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if (is_affected_with_act (victim, sn, 0, ch,
            "You're weak enough as it is!",
            "$N is weak enough already."))
        return;

    BAIL_IF (saves_spell (level, victim, DAM_OTHER),
        "Spell failed.\n\r", ch);

    affect_init (&af, TO_AFFECTS, sn, level, level / 2, APPLY_STR, -1 * (level / 5), AFF_WEAKEN);
    affect_to_char (victim, &af);

    send_to_char ("You feel your strength slip away.\n\r", victim);
    act ("$n looks tired and weak.", victim, NULL, NULL, TO_NOTCHAR);
}
