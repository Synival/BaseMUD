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
#include "skills.h"
#include "comm.h"
#include "utils.h"
#include "lookup.h"
#include "fight.h"
#include "chars.h"
#include "objs.h"

#include "spell_cure.h"

DEFINE_SPELL_FUN (spell_cure_blindness) {
    CHAR_T *victim = (CHAR_T *) vo;

    if (isnt_affected_with_act (victim, SN(BLINDNESS), 0, ch,
            "You aren't blind.",
            "$N doesn't appear to be blinded."))
        return;

    BAIL_IF (!check_dispel (level, victim, SN(BLINDNESS)),
        "Spell failed.\n\r", ch);

    send_to_char ("Your vision returns!\n\r", victim);
    act ("$n is no longer blinded.", victim, NULL, NULL, TO_NOTCHAR);
}

DEFINE_SPELL_FUN (spell_cure_critical) {
    CHAR_T *victim = (CHAR_T *) vo;
    int heal;

    heal = dice (3, 8) + level - 6;
    victim->hit = UMIN (victim->hit + heal, victim->max_hit);
    update_pos (victim);

    send_to_char ("You feel much better!\n\r", victim);
    act ("$n's wounds look much better!", victim, NULL, NULL, TO_NOTCHAR);
}

/* RT added to cure plague */
DEFINE_SPELL_FUN (spell_cure_disease) {
    CHAR_T *victim = (CHAR_T *) vo;

    if (isnt_affected_with_act (victim, SN(PLAGUE), 0, ch,
            "You aren't ill.",
            "$N doesn't appear to be diseased."))
        return;

    BAIL_IF (!check_dispel (level, victim, SN(PLAGUE)),
        "Spell failed.\n\r", ch);

    send_to_char ("Your sores vanish.\n\r", victim);
    act ("$n looks relieved as $s sores vanish.", victim, NULL, NULL,
         TO_NOTCHAR);
}

DEFINE_SPELL_FUN (spell_cure_light) {
    CHAR_T *victim = (CHAR_T *) vo;
    int heal;

    heal = dice (1, 8) + level / 3;
    victim->hit = UMIN (victim->hit + heal, victim->max_hit);
    update_pos (victim);

    send_to_char ("You feel a little better!\n\r", victim);
    act ("$n's wounds look a little better!", victim, NULL, NULL, TO_NOTCHAR);
}

DEFINE_SPELL_FUN (spell_cure_poison) {
    CHAR_T *victim = (CHAR_T *) vo;

    if (isnt_affected_with_act (victim, SN(POISON), 0, ch,
            "You aren't poisoned.",
            "$N doesn't appear to be poisoned."))
        return;

    BAIL_IF (!check_dispel (level, victim, SN(POISON)),
        "Spell failed.\n\r", ch);

    send_to_char ("A warm feeling runs through your body.\n\r", victim);
    act ("$n looks much healthier.", victim, NULL, NULL, TO_NOTCHAR);
}

DEFINE_SPELL_FUN (spell_cure_serious) {
    CHAR_T *victim = (CHAR_T *) vo;
    int heal;

    heal = dice (2, 8) + level / 2;
    victim->hit = UMIN (victim->hit + heal, victim->max_hit);
    update_pos (victim);

    send_to_char ("You feel better!\n\r", victim);
    act ("$n's wounds look better!", victim, NULL, NULL, TO_NOTCHAR);
}

DEFINE_SPELL_FUN (spell_heal) {
    CHAR_T *victim = (CHAR_T *) vo;
    victim->hit = UMIN (victim->hit + 100, victim->max_hit);
    update_pos (victim);

    send_to_char ("A warm feeling fills your body and your injuries heal!\n\r", victim);
    act ("$n glows and recovers from $s injuries!", victim,
        NULL, NULL, TO_NOTCHAR);
}

DEFINE_SPELL_FUN (spell_mass_healing) {
    CHAR_T *gch;
    int heal_num, refresh_num;

    heal_num    = skill_lookup ("heal");
    refresh_num = skill_lookup ("refresh");

    for (gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room) {
        if ((IS_NPC (ch) && IS_NPC (gch)) || (!IS_NPC (ch) && !IS_NPC (gch))) {
            spell_heal (heal_num, level, ch, (void *) gch, TARGET_CHAR,
                target_name);
            spell_refresh (refresh_num, level, ch, (void *) gch, TARGET_CHAR,
                target_name);
        }
    }
}

DEFINE_SPELL_FUN (spell_refresh) {
    CHAR_T *victim = (CHAR_T *) vo;
    victim->move = UMIN (victim->move + level, victim->max_move);
    if (victim->max_move == victim->move) {
        send_to_char ("You feel fully refreshed!\n\r", victim);
        act ("$n looks fully refreshed.\n\r", victim, NULL, NULL, TO_NOTCHAR);
    }
    else {
        send_to_char ("You feel less tired.\n\r", victim);
        act ("$n looks less tired.\n\r", victim, NULL, NULL, TO_NOTCHAR);
    }
}

DEFINE_SPELL_FUN (spell_remove_curse_object) {
    OBJ_T *obj = (OBJ_T *) vo;

    if (IS_OBJ_STAT (obj, ITEM_NODROP)
        || IS_OBJ_STAT (obj, ITEM_NOREMOVE))
    {
        if (!IS_OBJ_STAT (obj, ITEM_NOUNCURSE) &&
            !saves_dispel (level + 2, obj->level, 0))
        {
            REMOVE_BIT (obj->extra_flags, ITEM_NODROP);
            REMOVE_BIT (obj->extra_flags, ITEM_NOREMOVE);
            act ("$p glows blue.", ch, obj, NULL, TO_ALL);
            return;
        }

        act ("The curse on $p is beyond your power.", ch, obj, NULL, TO_CHAR);
        return;
    }
    act ("There doesn't seem to be a curse on $p.", ch, obj, NULL, TO_CHAR);
}

DEFINE_SPELL_FUN (spell_remove_curse_char) {
    CHAR_T *victim = (CHAR_T *) vo;
    OBJ_T *obj;
    bool found = FALSE;

    if (check_dispel (level, victim, SN(CURSE))) {
        found = TRUE;
        send_to_char ("You feel better.\n\r", victim);
        act ("$n looks more relaxed.", victim, NULL, NULL, TO_NOTCHAR);
    }

    for (obj = victim->carrying; (obj != NULL && !found);
         obj = obj->next_content)
    {
        if (!(IS_OBJ_STAT (obj, ITEM_NODROP) ||
              IS_OBJ_STAT (obj, ITEM_NOREMOVE)))
            continue;
        if (IS_OBJ_STAT (obj, ITEM_NOUNCURSE))
            continue;
        if (saves_dispel (level, obj->level, 0))
            continue;

        found = TRUE;
        REMOVE_BIT (obj->extra_flags, ITEM_NODROP);
        REMOVE_BIT (obj->extra_flags, ITEM_NOREMOVE);
        act ("Your $p glows blue.", victim, obj, NULL, TO_CHAR);
        act ("$n's $p glows blue.", victim, obj, NULL, TO_NOTCHAR);
    }

    if (!found)
        send_to_char ("Nothing happens.\n\r", ch);
}

DEFINE_SPELL_FUN (spell_remove_curse) {
    if (target == TARGET_OBJ)
        spell_remove_curse_object (sn, level, ch, vo, target, target_name);
    else
        spell_remove_curse_char (sn, level, ch, vo, target, target_name);
}
