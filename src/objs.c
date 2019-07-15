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
 *   ROM 2.4 is copyright 1993-1998 Russ Taylor                            *
 *   ROM has been brought to you by the ROM consortium                     *
 *       Russ Taylor (rtaylor@hypercube.org)                               *
 *       Gabrielle Taylor (gtaylor@hypercube.org)                          *
 *       Brian Moore (zump@rom.org)                                        *
 *   By using this code, you have agreed to follow the terms of the        *
 *   ROM license, in the file Rom24/doc/rom.license                        *
 ***************************************************************************/

#include <string.h>

#include "utils.h"
#include "chars.h"
#include "db.h"
#include "recycle.h"
#include "comm.h"
#include "lookup.h"

#include "objs.h"

/* TODO: lots of code can be put into tables. */
/* TODO: review the function names for consistency. */
/* TODO: remove any redundant functions, like simple lookup functions. */
/* TODO: char_wear_obj() is pretty awful :( */

/* returns number of people on an object */
int obj_count_users (OBJ_DATA * obj) {
    CHAR_DATA *fch;
    int count = 0;

    if (obj->in_room == NULL)
        return 0;
    for (fch = obj->in_room->people; fch != NULL; fch = fch->next_in_room)
        if (fch->on == obj)
            count++;
    return count;
}

/* Give an obj to a char. */
void obj_to_char (OBJ_DATA * obj, CHAR_DATA * ch) {
    LIST_FRONT (obj, next_content, ch->carrying);
    obj->carried_by = ch;
    obj->in_room = NULL;
    obj->in_obj = NULL;
    ch->carry_number += obj_get_carry_number (obj);
    ch->carry_weight += obj_get_weight (obj);
}

/* Take an obj from its character. */
void obj_from_char (OBJ_DATA * obj) {
    CHAR_DATA *ch;

    if ((ch = obj->carried_by) == NULL) {
        bug ("obj_from_char: null ch.", 0);
        return;
    }
    if (obj->wear_loc != WEAR_NONE)
        char_unequip (ch, obj);

    LIST_REMOVE (obj, next_content, ch->carrying, OBJ_DATA, NO_FAIL);
    obj->carried_by = NULL;
    ch->carry_number -= obj_get_carry_number (obj);
    ch->carry_weight -= obj_get_weight (obj);
}

/* Find the ac value of an obj, including position effect. */
int obj_get_ac_type (OBJ_DATA * obj, int iWear, int type) {
    if (obj->item_type != ITEM_ARMOR)
        return 0;

    switch (iWear) {
        case WEAR_BODY:    return 3 * obj->value[type];
        case WEAR_HEAD:    return 2 * obj->value[type];
        case WEAR_LEGS:    return 2 * obj->value[type];
        case WEAR_FEET:    return obj->value[type];
        case WEAR_HANDS:   return obj->value[type];
        case WEAR_ARMS:    return obj->value[type];
        case WEAR_SHIELD:  return obj->value[type];
        case WEAR_NECK_1:  return obj->value[type];
        case WEAR_NECK_2:  return obj->value[type];
        case WEAR_ABOUT:   return 2 * obj->value[type];
        case WEAR_WAIST:   return obj->value[type];
        case WEAR_WRIST_L: return obj->value[type];
        case WEAR_WRIST_R: return obj->value[type];
        case WEAR_HOLD:    return obj->value[type];
    }
    return 0;
}

/* Count occurrences of an obj in a list. */
int obj_index_count_in_list (OBJ_INDEX_DATA *pObjIndex, OBJ_DATA *list) {
    OBJ_DATA *obj;
    int nMatch;

    nMatch = 0;
    for (obj = list; obj != NULL; obj = obj->next_content)
        if (obj->pIndexData == pObjIndex)
            nMatch++;
    return nMatch;
}

/* Move an obj out of a room. */
void obj_from_room (OBJ_DATA * obj) {
    ROOM_INDEX_DATA *in_room;
    CHAR_DATA *ch;

    if ((in_room = obj->in_room) == NULL) {
        bug ("obj_from_room: NULL.", 0);
        return;
    }
    for (ch = in_room->people; ch != NULL; ch = ch->next_in_room)
        if (ch->on == obj)
            ch->on = NULL;

    LIST_REMOVE (obj, next_content, in_room->contents, OBJ_DATA, NO_FAIL);
    obj->in_room = NULL;
}

/* Move an obj into a room. */
void obj_to_room (OBJ_DATA * obj, ROOM_INDEX_DATA * pRoomIndex) {
    LIST_FRONT (obj, next_content, pRoomIndex->contents);
    obj->in_room = pRoomIndex;
    obj->carried_by = NULL;
    obj->in_obj = NULL;
}

/* Move an object into an object. */
void obj_to_obj (OBJ_DATA * obj, OBJ_DATA * obj_to) {
    LIST_FRONT (obj, next_content, obj_to->contains);
    obj->in_obj = obj_to;
    obj->in_room = NULL;
    obj->carried_by = NULL;
    if (obj_to->pIndexData->vnum == OBJ_VNUM_PIT)
        obj->cost = 0;

    for (; obj_to != NULL; obj_to = obj_to->in_obj) {
        if (obj_to->carried_by != NULL) {
            obj_to->carried_by->carry_number += obj_get_carry_number (obj);
            obj_to->carried_by->carry_weight += obj_get_weight (obj)
                * WEIGHT_MULT (obj_to) / 100;
        }
    }
}

/* Move an object out of an object. */
void obj_from_obj (OBJ_DATA * obj) {
    OBJ_DATA *obj_from;
    if ((obj_from = obj->in_obj) == NULL) {
        bug ("obj_from_obj: null obj_from.", 0);
        return;
    }

    LIST_REMOVE (obj, next_content, obj_from->contains, OBJ_DATA, NO_FAIL);
    obj->in_obj = NULL;

    for (; obj_from != NULL; obj_from = obj_from->in_obj) {
        if (obj_from->carried_by != NULL) {
            obj_from->carried_by->carry_number -= obj_get_carry_number (obj);
            obj_from->carried_by->carry_weight -= obj_get_weight (obj)
                * WEIGHT_MULT (obj_from) / 100;
        }
    }
}

/* Extract an obj from the world. */
void obj_extract (OBJ_DATA * obj) {
    OBJ_DATA *obj_content;
    OBJ_DATA *obj_next;

    if (obj->in_room != NULL)
        obj_from_room (obj);
    else if (obj->carried_by != NULL)
        obj_from_char (obj);
    else if (obj->in_obj != NULL)
        obj_from_obj (obj);

    for (obj_content = obj->contains; obj_content; obj_content = obj_next) {
        obj_next = obj_content->next_content;
        obj_extract (obj_content);
    }

    LIST_REMOVE (obj, next, object_list, OBJ_DATA, return);
    --obj->pIndexData->count;
    obj_free (obj);
    return;
}

/* Create a 'money' obj. */
OBJ_DATA *obj_create_money (int gold, int silver) {
    char buf[MAX_STRING_LENGTH];
    OBJ_DATA *obj;

    if (gold < 0 || silver < 0 || (gold == 0 && silver == 0)) {
        bug ("obj_create_money: zero or negative money.", UMIN (gold, silver));
        gold = UMAX (1, gold);
        silver = UMAX (1, silver);
    }

    if (gold == 0 && silver == 1)
        obj = create_object (get_obj_index (OBJ_VNUM_SILVER_ONE), 0);
    else if (gold == 1 && silver == 0)
        obj = create_object (get_obj_index (OBJ_VNUM_GOLD_ONE), 0);
    else if (silver == 0) {
        obj = create_object (get_obj_index (OBJ_VNUM_GOLD_SOME), 0);
        sprintf (buf, obj->short_descr, gold);
        str_free (obj->short_descr);
        obj->short_descr = str_dup (buf);
        obj->value[1] = gold;
        obj->cost = gold;
        obj->weight = gold / 5;
    }
    else if (gold == 0) {
        obj = create_object (get_obj_index (OBJ_VNUM_SILVER_SOME), 0);
        sprintf (buf, obj->short_descr, silver);
        str_free (obj->short_descr);
        obj->short_descr = str_dup (buf);
        obj->value[0] = silver;
        obj->cost = silver;
        obj->weight = silver / 20;
    }
    else {
        obj = create_object (get_obj_index (OBJ_VNUM_COINS), 0);
        sprintf (buf, obj->short_descr, silver, gold);
        str_free (obj->short_descr);
        obj->short_descr = str_dup (buf);
        obj->value[0] = silver;
        obj->value[1] = gold;
        obj->cost = 100 * gold + silver;
        obj->weight = gold / 5 + silver / 20;
    }
    return obj;
}

/* Return # of objects which an object counts as.
 * Thanks to Tony Chamberlain for the correct recursive code here. */
int obj_get_carry_number (OBJ_DATA * obj) {
    int number;
    switch (obj->item_type) {
        case ITEM_CONTAINER:
        case ITEM_MONEY:
        case ITEM_GEM:
        case ITEM_JEWELRY:
            number = 0;
            break;
        default:
            number = 1;
    }
    for (obj = obj->contains; obj != NULL; obj = obj->next_content)
        number += obj_get_carry_number (obj);
    return number;
}

/* Return weight of an object, including weight of contents. */
int obj_get_weight (OBJ_DATA * obj) {
    int weight, mult;

    weight = obj->weight;
    mult = WEIGHT_MULT (obj);
    for (obj = obj->contains; obj != NULL; obj = obj->next_content)
        weight += obj_get_weight (obj) * mult / 100;

    return weight;
}

int obj_get_true_weight (OBJ_DATA * obj) {
    int weight;

    weight = obj->weight;
    for (obj = obj->contains; obj != NULL; obj = obj->next_content)
        weight += obj_get_true_weight (obj);

    return weight;
}

char *obj_format_to_char (OBJ_DATA * obj, CHAR_DATA * ch, bool fShort) {
    static char buf[MAX_STRING_LENGTH];
    buf[0] = '\0';

    if ((fShort && (obj->short_descr == NULL || obj->short_descr[0] == '\0'))
        || (obj->description == NULL || obj->description[0] == '\0'))
        return buf;

#ifndef VANILLA
    if (IS_OBJ_STAT (obj, ITEM_INVIS))
        strcat (buf, "({DInvis{x) ");
    if (IS_AFFECTED (ch, AFF_DETECT_EVIL) && IS_OBJ_STAT (obj, ITEM_EVIL))
        strcat (buf, "({rRed Aura{x) ");
    if (IS_AFFECTED (ch, AFF_DETECT_GOOD) && IS_OBJ_STAT (obj, ITEM_BLESS))
        strcat (buf, "({BBlue Aura{x) ");
    if (IS_AFFECTED (ch, AFF_DETECT_MAGIC) && IS_OBJ_STAT (obj, ITEM_MAGIC))
        strcat (buf, "({mMagical{x) ");
    if (IS_OBJ_STAT (obj, ITEM_GLOW))
        strcat (buf, "({WGlowing{x) ");
    if (IS_OBJ_STAT (obj, ITEM_HUM))
        strcat (buf, "({wHumming{x) ");
    if (IS_OBJ_STAT (obj, ITEM_CORRODED))
        strcat (buf, "({yCorroded{x) ");
#else
    if (IS_OBJ_STAT (obj, ITEM_INVIS))
        strcat (buf, "(Invis) ");
    if (IS_AFFECTED (ch, AFF_DETECT_EVIL) && IS_OBJ_STAT (obj, ITEM_EVIL))
        strcat (buf, "(Red Aura) ");
    if (IS_AFFECTED (ch, AFF_DETECT_GOOD) && IS_OBJ_STAT (obj, ITEM_BLESS))
        strcat (buf, "(Blue Aura) ");
    if (IS_AFFECTED (ch, AFF_DETECT_MAGIC) && IS_OBJ_STAT (obj, ITEM_MAGIC))
        strcat (buf, "(Magical) ");
    if (IS_OBJ_STAT (obj, ITEM_GLOW))
        strcat (buf, "(Glowing) ");
    if (IS_OBJ_STAT (obj, ITEM_HUM))
        strcat (buf, "(Humming) ");
    if (IS_OBJ_STAT (obj, ITEM_CORRODED))
        strcat (buf, "(Corroded) ");
#endif

    if (fShort) {
        if (obj->short_descr != NULL)
            strcat (buf, obj->short_descr);
    }
    else {
        if (obj->description != NULL)
            strcat (buf, obj->description);
    }

    return buf;
}

/* Show a list to a character.
 * Can coalesce duplicated items.  */
void obj_list_show_to_char (OBJ_DATA * list, CHAR_DATA * ch, bool fShort,
    bool fShowNothing)
{
    char buf[MAX_STRING_LENGTH];
    BUFFER *output;
    char **prgpstrShow;
    int *prgnShow;
    char *pstrShow;
    OBJ_DATA *obj;
    int nShow;
    int iShow;
    int count;
    bool fCombine;

    if (ch->desc == NULL)
        return;

    /* Alloc space for output lines.  */
    output = buf_new ();

    count = 0;
    for (obj = list; obj != NULL; obj = obj->next_content)
        count++;
    prgpstrShow = alloc_mem (count * sizeof (char *));
    prgnShow = alloc_mem (count * sizeof (int));
    nShow = 0;

    /* Format the list of objects.  */
    for (obj = list; obj != NULL; obj = obj->next_content) {
        if (obj->wear_loc == WEAR_NONE && char_can_see_obj (ch, obj)) {
            pstrShow = obj_format_to_char (obj, ch, fShort);
            fCombine = FALSE;

            if (IS_NPC (ch) || IS_SET (ch->comm, COMM_COMBINE)) {
                /* Look for duplicates, case sensitive.
                 * Matches tend to be near end so run loop backwords. */
                for (iShow = nShow - 1; iShow >= 0; iShow--) {
                    if (!strcmp (prgpstrShow[iShow], pstrShow)) {
                        prgnShow[iShow]++;
                        fCombine = TRUE;
                        break;
                    }
                }
            }

            /* Couldn't combine, or didn't want to. */
            if (!fCombine) {
                prgpstrShow[nShow] = str_dup (pstrShow);
                prgnShow[nShow] = 1;
                nShow++;
            }
        }
    }

    /* Output the formatted list. */
    for (iShow = 0; iShow < nShow; iShow++) {
        if (prgpstrShow[iShow][0] == '\0') {
            str_free (prgpstrShow[iShow]);
            continue;
        }

        if (IS_NPC (ch) || IS_SET (ch->comm, COMM_COMBINE)) {
            if (prgnShow[iShow] != 1) {
                sprintf (buf, "(%2d) ", prgnShow[iShow]);
                add_buf (output, buf);
            }
            else
                add_buf (output, "     ");
        }
        add_buf (output, prgpstrShow[iShow]);
        add_buf (output, "\n\r");
        str_free (prgpstrShow[iShow]);
    }

    if (fShowNothing && nShow == 0) {
        if (IS_NPC (ch) || IS_SET (ch->comm, COMM_COMBINE))
            send_to_char ("     ", ch);
        send_to_char ("Nothing.\n\r", ch);
    }
    page_to_char (buf_string (output), ch);

    /* Clean up. */
    buf_free (output);
    mem_free (prgpstrShow, count * sizeof (char *));
    mem_free (prgnShow, count * sizeof (int));
}

const char *obj_furn_preposition (OBJ_DATA * obj, int position) {
    const FURNITURE_BITS *bits;
    if (obj == NULL)
        return NULL;
    if (obj->item_type != ITEM_FURNITURE)
        return "(not obj) ";
    if ((bits = furniture_get (position)) == NULL)
        return "(bad pos) ";

         if (obj->value[2] & bits->bit_at) return "at";
    else if (obj->value[2] & bits->bit_on) return "on";
    else if (obj->value[2] & bits->bit_in) return "in";
    else                                   return "by";
}

bool obj_is_container (OBJ_DATA *obj) {
    return obj->item_type == ITEM_CONTAINER ||
           obj->item_type == ITEM_CORPSE_NPC ||
           obj->item_type == ITEM_CORPSE_PC;
}

bool obj_can_fit_in (OBJ_DATA *obj, OBJ_DATA *container) {
    int weight;
    if (!obj_is_container (container))
        return FALSE;
    if (container->item_type != ITEM_CONTAINER)
        return TRUE;

    weight = obj_get_weight (obj);
    if (weight + obj_get_true_weight (container) > (container->value[0] * 10))
        return FALSE;
    if (weight > (container->value[3] * 10))
        return FALSE;
    return TRUE;
}

/* Find some object with a given index data.
 * Used by area-reset 'P' command. */
OBJ_DATA *obj_get_by_index (OBJ_INDEX_DATA * pObjIndex) {
    OBJ_DATA *obj;
    for (obj = object_list; obj != NULL; obj = obj->next)
        if (obj->pIndexData == pObjIndex)
            return obj;
    return NULL;
}

/* insert an object at the right spot for the keeper */
void obj_to_keeper (OBJ_DATA * obj, CHAR_DATA * ch) {
    OBJ_DATA *t_obj, *t_obj_next;

    /* see if any duplicates are found */
    for (t_obj = ch->carrying; t_obj != NULL; t_obj = t_obj_next) {
        t_obj_next = t_obj->next_content;

        if (obj->pIndexData == t_obj->pIndexData
            && !str_cmp (obj->short_descr, t_obj->short_descr))
        {
            /* if this is an unlimited item, destroy the new one */
            if (IS_OBJ_STAT (t_obj, ITEM_INVENTORY)) {
                obj_extract (obj);
                return;
            }
            obj->cost = t_obj->cost;    /* keep it standard */
            break;
        }
    }

    LIST_INSERT_AFTER (obj, t_obj, next_content, ch->carrying);
    obj->carried_by = ch;

    obj->in_room = NULL;
    obj->in_obj = NULL;
    ch->carry_number += obj_get_carry_number (obj);
    ch->carry_weight += obj_get_weight (obj);
}
