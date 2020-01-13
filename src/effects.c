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

#include "recycle.h"
#include "lookup.h"
#include "db.h"
#include "utils.h"
#include "comm.h"
#include "magic.h"
#include "update.h"
#include "affects.h"
#include "objs.h"
#include "chars.h"
#include "items.h"

#include "effects.h"

DEFINE_EFFECT_FUN (effect_acid) {
    /* nail objects on the floor */
    if (target == TARGET_ROOM) {
        ROOM_INDEX_T *room = (ROOM_INDEX_T *) vo;
        OBJ_T *obj, *obj_next;

        for (obj = room->contents; obj != NULL; obj = obj_next) {
            obj_next = obj->next_content;
            effect_acid (obj, level, dam, TARGET_OBJ);
        }
        return;
    }

    /* do the effect on a victim */
    if (target == TARGET_CHAR) {
        CHAR_T *victim = (CHAR_T *) vo;
        OBJ_T *obj, *obj_next;

        /* let's toast some gear */
        for (obj = victim->carrying; obj != NULL; obj = obj_next) {
            obj_next = obj->next_content;
            effect_acid (obj, level, dam, TARGET_OBJ);
        }
        return;
    }

    /* toast an object */
    if (target == TARGET_OBJ) {
        OBJ_T *obj = (OBJ_T *) vo;
        OBJ_T *t_obj, *n_obj;
        const char *msg;
        int chance;

        if (IS_OBJ_STAT (obj, ITEM_BURN_PROOF) ||
            IS_OBJ_STAT (obj, ITEM_NOPURGE)    ||
            number_range (0, 4) == 0)
        {
            return;
        }
        if ((msg = item_get_corrode_message (obj)) == NULL)
            return;

        chance = level / 4 + dam / 10;
        if (chance > 25)
            chance = (chance - 25) / 2 + 25;
        if (chance > 50)
            chance = (chance - 50) / 2 + 50;
        if (IS_OBJ_STAT (obj, ITEM_BLESS))
            chance -= 5;
        chance -= obj->level * 2;

        chance += item_get_corrode_chance_modifier (obj);
        chance = URANGE (5, chance, 95);
        if (number_percent () > chance)
            return;

        if (*msg != '\0') {
            if (obj->carried_by != NULL)
                act (msg, obj->carried_by, obj, NULL, TO_ALL);
            else if (obj->in_room != NULL && obj->in_room->people != NULL)
                act (msg, obj->in_room->people, obj, NULL, TO_ALL);
        }

        /* perform any special corrosion effects. */
        if (item_corrode_effect (obj, level))
            return;

        /* no special effect - get rid of the object */
        if (obj->contains) { /* dump contents */
            for (t_obj = obj->contains; t_obj != NULL; t_obj = n_obj) {
                n_obj = t_obj->next_content;
                obj_take_from_obj (t_obj);
                if (obj->in_room != NULL)
                    obj_give_to_room (t_obj, obj->in_room);
                else if (obj->carried_by != NULL)
                    obj_give_to_room (t_obj, obj->carried_by->in_room);
                else {
                    obj_extract (t_obj);
                    continue;
                }
                effect_acid (t_obj, level / 2, dam / 2, TARGET_OBJ);
            }
        }

        obj_extract (obj);
        return;
    }
}

DEFINE_EFFECT_FUN (effect_cold) {
    /* nail objects on the floor */
    if (target == TARGET_ROOM) {
        ROOM_INDEX_T *room = (ROOM_INDEX_T *) vo;
        OBJ_T *obj, *obj_next;

        for (obj = room->contents; obj != NULL; obj = obj_next) {
            obj_next = obj->next_content;
            effect_cold (obj, level, dam, TARGET_OBJ);
        }
        return;
    }

    /* whack a character */
    if (target == TARGET_CHAR) {
        CHAR_T *victim = (CHAR_T *) vo;
        OBJ_T *obj, *obj_next;

        /* chill touch effect */
        if (!saves_spell (level / 4 + dam / 20, victim, DAM_COLD)) {
            AFFECT_T af;

            act2 ("A chill sinks deep into your bones.",
                  "$n turns blue and shivers.",
                  victim, NULL, NULL, 0, POS_RESTING);

            affect_init (&af, AFF_TO_AFFECTS, skill_lookup ("chill touch"), level, 6, APPLY_STR, -1, 0);
            affect_join (victim, &af);
        }

        /* hunger! (warmth sucked out) */
        if (!IS_NPC (victim))
            gain_condition (victim, COND_HUNGER, -1 * dam / 20);

        /* let's toast some gear */
        for (obj = victim->carrying; obj != NULL; obj = obj_next) {
            obj_next = obj->next_content;
            effect_cold (obj, level, dam, TARGET_OBJ);
        }
        return;
    }

    /* toast an object */
    if (target == TARGET_OBJ) {
        OBJ_T *obj = (OBJ_T *) vo;
        const char *msg;
        int chance;

        if (IS_OBJ_STAT (obj, ITEM_BURN_PROOF) ||
            IS_OBJ_STAT (obj, ITEM_NOPURGE)    ||
            number_range (0, 4) == 0)
        {
            return;
        }
        if ((msg = item_get_freeze_message (obj)) == NULL)
            return;

        chance = level / 4 + dam / 10;
        if (chance > 25)
            chance = (chance - 25) / 2 + 25;
        if (chance > 50)
            chance = (chance - 50) / 2 + 50;
        if (IS_OBJ_STAT (obj, ITEM_BLESS))
            chance -= 5;
        chance -= obj->level * 2;

        chance += item_get_freeze_chance_modifier (obj);
        chance = URANGE (5, chance, 95);
        if (number_percent () > chance)
            return;

        if (*msg != '\0') {
            if (obj->carried_by != NULL)
                act (msg, obj->carried_by, obj, NULL, TO_ALL);
            else if (obj->in_room != NULL && obj->in_room->people != NULL)
                act (msg, obj->in_room->people, obj, NULL, TO_ALL);
        }

        /* perform any special freeze effects. */
        if (item_freeze_effect (obj, level))
            return;

        obj_extract (obj);
        return;
    }
}

DEFINE_EFFECT_FUN (effect_fire) {
    /* nail objects on the floor */
    if (target == TARGET_ROOM) {
        ROOM_INDEX_T *room = (ROOM_INDEX_T *) vo;
        OBJ_T *obj, *obj_next;

        for (obj = room->contents; obj != NULL; obj = obj_next) {
            obj_next = obj->next_content;
            effect_fire (obj, level, dam, TARGET_OBJ);
        }
        return;
    }

    /* do the effect on a victim */
    if (target == TARGET_CHAR) {
        CHAR_T *victim = (CHAR_T *) vo;
        OBJ_T *obj, *obj_next;

        /* chance of blindness */
        if (!IS_AFFECTED (victim, AFF_BLIND)
            && !saves_spell (level / 4 + dam / 20, victim, DAM_FIRE))
        {
            AFFECT_T af;
            act2 ("Your eyes tear up from smoke...you can't see a thing!",
                  "$n is blinded by smoke!",
                  victim, NULL, NULL, 0, POS_RESTING);

            affect_init (&af, AFF_TO_AFFECTS, skill_lookup ("fire breath"), level, number_range (0, level / 10), APPLY_HITROLL, -4, AFF_BLIND);
            affect_to_char (victim, &af);
        }

        /* getting thirsty */
        if (!IS_NPC (victim))
            gain_condition (victim, COND_THIRST, -1 * dam / 20);

        /* let's toast some gear! */
        for (obj = victim->carrying; obj != NULL; obj = obj_next) {
            obj_next = obj->next_content;
            effect_fire (obj, level, dam, TARGET_OBJ);
        }
        return;
    }

    /* toast an object */
    if (target == TARGET_OBJ) {
        OBJ_T *obj = (OBJ_T *) vo;
        OBJ_T *t_obj, *n_obj;
        const char *msg;
        int chance;

        if (IS_OBJ_STAT (obj, ITEM_BURN_PROOF) ||
            IS_OBJ_STAT (obj, ITEM_NOPURGE)    ||
            number_range (0, 4) == 0)
        {
            return;
        }
        if ((msg = item_get_burn_message (obj)) == NULL)
            return;

        chance = level / 4 + dam / 10;
        if (chance > 25)
            chance = (chance - 25) / 2 + 25;
        if (chance > 50)
            chance = (chance - 50) / 2 + 50;
        if (IS_OBJ_STAT (obj, ITEM_BLESS))
            chance -= 5;
        chance -= obj->level * 2;

        chance += item_get_burn_chance_modifier (obj);
        chance = URANGE (5, chance, 95);
        if (number_percent () > chance)
            return;

        if (*msg != '\0') {
            if (obj->carried_by != NULL)
                act (msg, obj->carried_by, obj, NULL, TO_ALL);
            else if (obj->in_room != NULL && obj->in_room->people != NULL)
                act (msg, obj->in_room->people, obj, NULL, TO_ALL);
        }

        /* perform any special burn effects. */
        if (item_burn_effect (obj, level))
            return;

        /* dump the contents */
        if (obj->contains) {
            for (t_obj = obj->contains; t_obj != NULL; t_obj = n_obj) {
                n_obj = t_obj->next_content;
                obj_take_from_obj (t_obj);
                if (obj->in_room != NULL)
                    obj_give_to_room (t_obj, obj->in_room);
                else if (obj->carried_by != NULL)
                    obj_give_to_room (t_obj, obj->carried_by->in_room);
                else {
                    obj_extract (t_obj);
                    continue;
                }
                effect_fire (t_obj, level / 2, dam / 2, TARGET_OBJ);
            }
        }

        obj_extract (obj);
        return;
    }
}

DEFINE_EFFECT_FUN (effect_poison) {
    /* nail objects on the floor */
    if (target == TARGET_ROOM) {
        ROOM_INDEX_T *room = (ROOM_INDEX_T *) vo;
        OBJ_T *obj, *obj_next;

        for (obj = room->contents; obj != NULL; obj = obj_next) {
            obj_next = obj->next_content;
            effect_poison (obj, level, dam, TARGET_OBJ);
        }
        return;
    }

    /* do the effect on a victim */
    if (target == TARGET_CHAR) {
        CHAR_T *victim = (CHAR_T *) vo;
        OBJ_T *obj, *obj_next;

        /* chance of poisoning */
        if (!saves_spell (level / 4 + dam / 20, victim, DAM_POISON)) {
            AFFECT_T af;
            send_to_char ("You feel poison coursing through your veins.\n\r",
                          victim);
            act2 ("You feel poison coursing through your veins.",
                  "$n looks very ill.",
                  victim, NULL, NULL, 0, POS_RESTING);

            affect_init (&af, AFF_TO_AFFECTS, SN(POISON), level, level / 2, APPLY_STR, -1, AFF_POISON);
            affect_join (victim, &af);
        }

        /* equipment */
        for (obj = victim->carrying; obj != NULL; obj = obj_next) {
            obj_next = obj->next_content;
            effect_poison (obj, level, dam, TARGET_OBJ);
        }
        return;
    }

    if (target == TARGET_OBJ) {
        /* do some poisoning */
        OBJ_T *obj = (OBJ_T *) vo;
        const char *msg;
        int chance;

        if (IS_OBJ_STAT (obj, ITEM_BURN_PROOF) ||
            IS_OBJ_STAT (obj, ITEM_NOPURGE)    ||
            number_range (0, 4) == 0)
        {
            return;
        }
        if ((msg = item_get_poison_message (obj)) == NULL)
            return;

        chance = level / 4 + dam / 10;
        if (chance > 25)
            chance = (chance - 25) / 2 + 25;
        if (chance > 50)
            chance = (chance - 50) / 2 + 50;
        if (IS_OBJ_STAT (obj, ITEM_BLESS))
            chance -= 5;
        chance -= obj->level * 2;

        chance += item_get_poison_chance_modifier (obj);
        chance = URANGE (5, chance, 95);
        if (number_percent () > chance)
            return;

        if (*msg != '\0') {
            if (obj->carried_by != NULL)
                act (msg, obj->carried_by, obj, NULL, TO_ALL);
            else if (obj->in_room != NULL && obj->in_room->people != NULL)
                act (msg, obj->in_room->people, obj, NULL, TO_ALL);
        }

        /* perform any special poison effects. */
        if (item_poison_effect (obj, level))
            return;

        return;
    }
}

DEFINE_EFFECT_FUN (effect_shock) {
    /* nail objects on the floor */
    if (target == TARGET_ROOM) {
        ROOM_INDEX_T *room = (ROOM_INDEX_T *) vo;
        OBJ_T *obj, *obj_next;

        for (obj = room->contents; obj != NULL; obj = obj_next) {
            obj_next = obj->next_content;
            effect_shock (obj, level, dam, TARGET_OBJ);
        }
        return;
    }

    /* do the effect on a victim */
    if (target == TARGET_CHAR) {
        CHAR_T *victim = (CHAR_T *) vo;
        OBJ_T *obj, *obj_next;

        /* daze and confused? */
        if (!saves_spell (level / 4 + dam / 20, victim, DAM_LIGHTNING)) {
            send_to_char ("Your muscles stop responding.\n\r", victim);
            DAZE_STATE (victim, UMAX (12, level / 4 + dam / 20));
        }

        /* toast some gear */
        for (obj = victim->carrying; obj != NULL; obj = obj_next) {
            obj_next = obj->next_content;
            effect_shock (obj, level, dam, TARGET_OBJ);
        }
        return;
    }

    if (target == TARGET_OBJ) {
        OBJ_T *obj = (OBJ_T *) vo;
        const char *msg;
        int chance;

        if (IS_OBJ_STAT (obj, ITEM_BURN_PROOF) ||
            IS_OBJ_STAT (obj, ITEM_NOPURGE)    ||
            number_range (0, 4) == 0)
        {
            return;
        }
        if ((msg = item_get_shock_message (obj)) == NULL)
            return;

        chance = level / 4 + dam / 10;
        if (chance > 25)
            chance = (chance - 25) / 2 + 25;
        if (chance > 50)
            chance = (chance - 50) / 2 + 50;
        if (IS_OBJ_STAT (obj, ITEM_BLESS))
            chance -= 5;
        chance -= obj->level * 2;

        chance += item_get_shock_chance_modifier (obj);
        chance = URANGE (5, chance, 95);
        if (number_percent () > chance)
            return;

        if (*msg != '\0') {
            if (obj->carried_by != NULL)
                act (msg, obj->carried_by, obj, NULL, TO_ALL);
            else if (obj->in_room != NULL && obj->in_room->people != NULL)
                act (msg, obj->in_room->people, obj, NULL, TO_ALL);
        }

        /* perform any special shock effects. */
        if (item_shock_effect (obj, level))
            return;

        obj_extract (obj);
        return;
    }
}

DEFINE_EFFECT_FUN (effect_empty) {
    /* Do nothing! :D
     * This is here to avoid null function pointer crashes. */
}
