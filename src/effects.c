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
 **************************************************************************/

/***************************************************************************
 *   ROM 2.4 is copyright 1993-1998 Russ Taylor                            *
 *   ROM has been brought to you by the ROM consortium                     *
 *       Russ Taylor (rtaylor@hypercube.org)                               *
 *       Gabrielle Taylor (gtaylor@hypercube.org)                          *
 *       Brian Moore (zump@rom.org)                                        *
 *   By using this code, you have agreed to follow the terms of the        *
 *   ROM license, in the file Rom24/doc/rom.license                        *
 **************************************************************************/

#include "recycle.h"
#include "lookup.h"
#include "db.h"
#include "utils.h"
#include "comm.h"
#include "skills.h"
#include "magic.h"
#include "update.h"
#include "affects.h"
#include "objs.h"
#include "chars.h"

#include "effects.h"

void acid_effect (void *vo, int level, int dam, int target) {
    /* nail objects on the floor */
    if (target == TARGET_ROOM) {
        ROOM_INDEX_DATA *room = (ROOM_INDEX_DATA *) vo;
        OBJ_DATA *obj, *obj_next;

        for (obj = room->contents; obj != NULL; obj = obj_next) {
            obj_next = obj->next_content;
            acid_effect (obj, level, dam, TARGET_OBJ);
        }
        return;
    }

    /* do the effect on a victim */
    if (target == TARGET_CHAR) {
        CHAR_DATA *victim = (CHAR_DATA *) vo;
        OBJ_DATA *obj, *obj_next;

        /* let's toast some gear */
        for (obj = victim->carrying; obj != NULL; obj = obj_next) {
            obj_next = obj->next_content;
            acid_effect (obj, level, dam, TARGET_OBJ);
        }
        return;
    }

    /* toast an object */
    if (target == TARGET_OBJ) {
        OBJ_DATA *obj = (OBJ_DATA *) vo;
        OBJ_DATA *t_obj, *n_obj;
        int chance;
        char *msg;

        if (IS_OBJ_STAT (obj, ITEM_BURN_PROOF)
            || IS_OBJ_STAT (obj, ITEM_NOPURGE) || number_range (0, 4) == 0)
            return;

        chance = level / 4 + dam / 10;
        if (chance > 25)
            chance = (chance - 25) / 2 + 25;
        if (chance > 50)
            chance = (chance - 50) / 2 + 50;
        if (IS_OBJ_STAT (obj, ITEM_BLESS))
            chance -= 5;
        chance -= obj->level * 2;

        switch (obj->item_type) {
            default:
                return;
            case ITEM_CONTAINER:
            case ITEM_CORPSE_PC:
            case ITEM_CORPSE_NPC:
                msg = "$p fumes and dissolves.";
                break;
            case ITEM_ARMOR:
                msg = "$p is pitted and etched.";
                break;
            case ITEM_CLOTHING:
                msg = "$p is corroded into scrap.";
                break;
            case ITEM_STAFF:
            case ITEM_WAND:
                chance -= 10;
                msg = "$p corrodes and breaks.";
                break;
            case ITEM_SCROLL:
                chance += 10;
                msg = "$p is burned into waste.";
                break;
        }

        chance = URANGE (5, chance, 95);
        if (number_percent () > chance)
            return;

        if (obj->carried_by != NULL)
            act (msg, obj->carried_by, obj, NULL, TO_ALL);
        else if (obj->in_room != NULL && obj->in_room->people != NULL)
            act (msg, obj->in_room->people, obj, NULL, TO_ALL);
        if (obj->item_type == ITEM_ARMOR) { /* etch it */
            AFFECT_DATA *paf;
            bool af_found = FALSE;
            int i;

            obj_enchant (obj);
            for (paf = obj->affected; paf != NULL; paf = paf->next) {
                if (paf->apply == APPLY_AC) {
                    af_found = TRUE;
                    paf->type = -1;
                    paf->modifier += 1;
                    paf->level = UMAX (paf->level, level);
                    break;
                }
            }

            /* needs a new affect */
            if (!af_found) {
                paf = affect_new ();
                affect_init (paf, AFF_TO_AFFECTS, -1, level, -1, APPLY_AC, 1, 0);
                LIST_FRONT (paf, next, obj->affected);
            }
            if (obj->carried_by != NULL && obj->wear_loc != WEAR_NONE)
                for (i = 0; i < 4; i++)
                    obj->carried_by->armor[i] += 1;
            SET_BIT (obj->extra_flags, ITEM_CORRODED);
            return;
        }

        /* get rid of the object */
        if (obj->contains) { /* dump contents */
            for (t_obj = obj->contains; t_obj != NULL; t_obj = n_obj) {
                n_obj = t_obj->next_content;
                obj_from_obj (t_obj);
                if (obj->in_room != NULL)
                    obj_to_room (t_obj, obj->in_room);
                else if (obj->carried_by != NULL)
                    obj_to_room (t_obj, obj->carried_by->in_room);
                else {
                    obj_extract (t_obj);
                    continue;
                }
                acid_effect (t_obj, level / 2, dam / 2, TARGET_OBJ);
            }
        }

        obj_extract (obj);
        return;
    }
}

void cold_effect (void *vo, int level, int dam, int target) {
    /* nail objects on the floor */
    if (target == TARGET_ROOM) {
        ROOM_INDEX_DATA *room = (ROOM_INDEX_DATA *) vo;
        OBJ_DATA *obj, *obj_next;

        for (obj = room->contents; obj != NULL; obj = obj_next) {
            obj_next = obj->next_content;
            cold_effect (obj, level, dam, TARGET_OBJ);
        }
        return;
    }

    /* whack a character */
    if (target == TARGET_CHAR) {
        CHAR_DATA *victim = (CHAR_DATA *) vo;
        OBJ_DATA *obj, *obj_next;

        /* chill touch effect */
        if (!saves_spell (level / 4 + dam / 20, victim, DAM_COLD)) {
            AFFECT_DATA af;

            act ("A chill sinks deep into your bones.", victim, NULL, NULL, TO_CHAR);
            act ("$n turns blue and shivers.", victim, NULL, NULL, TO_NOTCHAR);

            affect_init (&af, AFF_TO_AFFECTS, skill_lookup ("chill touch"), level, 6, APPLY_STR, -1, 0);
            affect_join (victim, &af);
        }

        /* hunger! (warmth sucked out) */
        if (!IS_NPC (victim))
            gain_condition (victim, COND_HUNGER, -1 * dam / 20);

        /* let's toast some gear */
        for (obj = victim->carrying; obj != NULL; obj = obj_next) {
            obj_next = obj->next_content;
            cold_effect (obj, level, dam, TARGET_OBJ);
        }
        return;
    }

    /* toast an object */
    if (target == TARGET_OBJ) {
        OBJ_DATA *obj = (OBJ_DATA *) vo;
        int chance;
        char *msg;

        if (IS_OBJ_STAT (obj, ITEM_BURN_PROOF)
            || IS_OBJ_STAT (obj, ITEM_NOPURGE) || number_range (0, 4) == 0)
            return;

        chance = level / 4 + dam / 10;
        if (chance > 25)
            chance = (chance - 25) / 2 + 25;
        if (chance > 50)
            chance = (chance - 50) / 2 + 50;
        if (IS_OBJ_STAT (obj, ITEM_BLESS))
            chance -= 5;
        chance -= obj->level * 2;

        switch (obj->item_type) {
            default:
                return;
            case ITEM_POTION:
                msg = "$p freezes and shatters!";
                chance += 25;
                break;
            case ITEM_DRINK_CON:
                msg = "$p freezes and shatters!";
                chance += 5;
                break;
        }

        chance = URANGE (5, chance, 95);
        if (number_percent () > chance)
            return;

        if (obj->carried_by != NULL)
            act (msg, obj->carried_by, obj, NULL, TO_ALL);
        else if (obj->in_room != NULL && obj->in_room->people != NULL)
            act (msg, obj->in_room->people, obj, NULL, TO_ALL);

        obj_extract (obj);
        return;
    }
}

void fire_effect (void *vo, int level, int dam, int target) {
    /* nail objects on the floor */
    if (target == TARGET_ROOM) {
        ROOM_INDEX_DATA *room = (ROOM_INDEX_DATA *) vo;
        OBJ_DATA *obj, *obj_next;

        for (obj = room->contents; obj != NULL; obj = obj_next) {
            obj_next = obj->next_content;
            fire_effect (obj, level, dam, TARGET_OBJ);
        }
        return;
    }

    /* do the effect on a victim */
    if (target == TARGET_CHAR) {
        CHAR_DATA *victim = (CHAR_DATA *) vo;
        OBJ_DATA *obj, *obj_next;

        /* chance of blindness */
        if (!IS_AFFECTED (victim, AFF_BLIND)
            && !saves_spell (level / 4 + dam / 20, victim, DAM_FIRE))
        {
            AFFECT_DATA af;
            act ("Your eyes tear up from smoke...you can't see a thing!",
                 victim, NULL, NULL, TO_CHAR);
            act ("$n is blinded by smoke!", victim, NULL, NULL, TO_NOTCHAR);

            affect_init (&af, AFF_TO_AFFECTS, skill_lookup ("fire breath"), level, number_range (0, level / 10), APPLY_HITROLL, -4, AFF_BLIND);
            affect_to_char (victim, &af);
        }

        /* getting thirsty */
        if (!IS_NPC (victim))
            gain_condition (victim, COND_THIRST, -1 * dam / 20);

        /* let's toast some gear! */
        for (obj = victim->carrying; obj != NULL; obj = obj_next) {
            obj_next = obj->next_content;
            fire_effect (obj, level, dam, TARGET_OBJ);
        }
        return;
    }

    /* toast an object */
    if (target == TARGET_OBJ) {
        OBJ_DATA *obj = (OBJ_DATA *) vo;
        OBJ_DATA *t_obj, *n_obj;
        int chance;
        char *msg;

        if (IS_OBJ_STAT (obj, ITEM_BURN_PROOF)
            || IS_OBJ_STAT (obj, ITEM_NOPURGE) || number_range (0, 4) == 0)
            return;

        chance = level / 4 + dam / 10;
        if (chance > 25)
            chance = (chance - 25) / 2 + 25;
        if (chance > 50)
            chance = (chance - 50) / 2 + 50;
        if (IS_OBJ_STAT (obj, ITEM_BLESS))
            chance -= 5;
        chance -= obj->level * 2;

        switch (obj->item_type) {
            default:
                return;
            case ITEM_CONTAINER:
                msg = "$p ignites and burns!";
                break;
            case ITEM_POTION:
                chance += 25;
                msg = "$p bubbles and boils!";
                break;
            case ITEM_SCROLL:
                chance += 50;
                msg = "$p crackles and burns!";
                break;
            case ITEM_STAFF:
                chance += 10;
                msg = "$p smokes and chars!";
                break;
            case ITEM_WAND:
                msg = "$p sparks and sputters!";
                break;
            case ITEM_FOOD:
                msg = "$p blackens and crisps!";
                break;
            case ITEM_PILL:
                msg = "$p melts and drips!";
                break;
        }

        chance = URANGE (5, chance, 95);
        if (number_percent () > chance)
            return;

        if (obj->carried_by != NULL)
            act (msg, obj->carried_by, obj, NULL, TO_ALL);
        else if (obj->in_room != NULL && obj->in_room->people != NULL)
            act (msg, obj->in_room->people, obj, NULL, TO_ALL);

        /* dump the contents */
        if (obj->contains) {
            for (t_obj = obj->contains; t_obj != NULL; t_obj = n_obj) {
                n_obj = t_obj->next_content;
                obj_from_obj (t_obj);
                if (obj->in_room != NULL)
                    obj_to_room (t_obj, obj->in_room);
                else if (obj->carried_by != NULL)
                    obj_to_room (t_obj, obj->carried_by->in_room);
                else {
                    obj_extract (t_obj);
                    continue;
                }
                fire_effect (t_obj, level / 2, dam / 2, TARGET_OBJ);
            }
        }
        obj_extract (obj);
        return;
    }
}

void poison_effect (void *vo, int level, int dam, int target) {
    /* nail objects on the floor */
    if (target == TARGET_ROOM) {
        ROOM_INDEX_DATA *room = (ROOM_INDEX_DATA *) vo;
        OBJ_DATA *obj, *obj_next;

        for (obj = room->contents; obj != NULL; obj = obj_next) {
            obj_next = obj->next_content;
            poison_effect (obj, level, dam, TARGET_OBJ);
        }
        return;
    }

    /* do the effect on a victim */
    if (target == TARGET_CHAR) {
        CHAR_DATA *victim = (CHAR_DATA *) vo;
        OBJ_DATA *obj, *obj_next;

        /* chance of poisoning */
        if (!saves_spell (level / 4 + dam / 20, victim, DAM_POISON)) {
            AFFECT_DATA af;
            send_to_char ("You feel poison coursing through your veins.\n\r",
                          victim);
            act ("$n looks very ill.", victim, NULL, NULL, TO_NOTCHAR);

            affect_init (&af, AFF_TO_AFFECTS, gsn_poison, level, level / 2, APPLY_STR, -1, AFF_POISON);
            affect_join (victim, &af);
        }

        /* equipment */
        for (obj = victim->carrying; obj != NULL; obj = obj_next) {
            obj_next = obj->next_content;
            poison_effect (obj, level, dam, TARGET_OBJ);
        }
        return;
    }

    if (target == TARGET_OBJ) {
        /* do some poisoning */
        OBJ_DATA *obj = (OBJ_DATA *) vo;
        int chance;

        if (IS_OBJ_STAT (obj, ITEM_BURN_PROOF)
            || IS_OBJ_STAT (obj, ITEM_NOPURGE) || number_range (0, 4) == 0)
            return;

        chance = level / 4 + dam / 10;
        if (chance > 25)
            chance = (chance - 25) / 2 + 25;
        if (chance > 50)
            chance = (chance - 50) / 2 + 50;
        if (IS_OBJ_STAT (obj, ITEM_BLESS))
            chance -= 5;
        chance -= obj->level * 2;

        switch (obj->item_type) {
            default:
                return;
            case ITEM_FOOD:
                break;
            case ITEM_DRINK_CON:
                if (obj->v.drink_con.capacity == obj->v.drink_con.filled)
                    return;
                break;
        }

        chance = URANGE (5, chance, 95);
        if (number_percent () > chance)
            return;

        switch (obj->item_type) {
            case ITEM_FOOD:
                obj->v.food.poisoned = 1;
                break;

            case ITEM_DRINK_CON:
                obj->v.drink_con.poisoned = 1;
                break;
        }
        return;
    }
}

void shock_effect (void *vo, int level, int dam, int target) {
    /* nail objects on the floor */
    if (target == TARGET_ROOM) {
        ROOM_INDEX_DATA *room = (ROOM_INDEX_DATA *) vo;
        OBJ_DATA *obj, *obj_next;

        for (obj = room->contents; obj != NULL; obj = obj_next) {
            obj_next = obj->next_content;
            shock_effect (obj, level, dam, TARGET_OBJ);
        }
        return;
    }

    /* do the effect on a victim */
    if (target == TARGET_CHAR) {
        CHAR_DATA *victim = (CHAR_DATA *) vo;
        OBJ_DATA *obj, *obj_next;

        /* daze and confused? */
        if (!saves_spell (level / 4 + dam / 20, victim, DAM_LIGHTNING)) {
            send_to_char ("Your muscles stop responding.\n\r", victim);
            DAZE_STATE (victim, UMAX (12, level / 4 + dam / 20));
        }

        /* toast some gear */
        for (obj = victim->carrying; obj != NULL; obj = obj_next) {
            obj_next = obj->next_content;
            shock_effect (obj, level, dam, TARGET_OBJ);
        }
        return;
    }

    if (target == TARGET_OBJ) {
        OBJ_DATA *obj = (OBJ_DATA *) vo;
        int chance;
        char *msg;

        if (IS_OBJ_STAT (obj, ITEM_BURN_PROOF)
            || IS_OBJ_STAT (obj, ITEM_NOPURGE) || number_range (0, 4) == 0)
            return;

        chance = level / 4 + dam / 10;
        if (chance > 25)
            chance = (chance - 25) / 2 + 25;
        if (chance > 50)
            chance = (chance - 50) / 2 + 50;
        if (IS_OBJ_STAT (obj, ITEM_BLESS))
            chance -= 5;
        chance -= obj->level * 2;

        switch (obj->item_type) {
            default:
                return;
            case ITEM_WAND:
            case ITEM_STAFF:
                chance += 10;
                msg = "$p overloads and explodes!";
                break;
            case ITEM_JEWELRY:
                chance -= 10;
                msg = "$p is fused into a worthless lump.";
        }

        chance = URANGE (5, chance, 95);
        if (number_percent () > chance)
            return;

        if (obj->carried_by != NULL)
            act (msg, obj->carried_by, obj, NULL, TO_ALL);
        else if (obj->in_room != NULL && obj->in_room->people != NULL)
            act (msg, obj->in_room->people, obj, NULL, TO_ALL);

        obj_extract (obj);
        return;
    }
}

void empty_effect (void *vo, int level, int dam, int target) {
    /* Do nothing! :D
     * This is here to avoid null function pointer crashes. */
}
