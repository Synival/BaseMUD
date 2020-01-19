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
#include <stdlib.h>

#include "affects.h"
#include "utils.h"
#include "chars.h"
#include "db.h"
#include "recycle.h"
#include "comm.h"
#include "lookup.h"
#include "materials.h"
#include "globals.h"
#include "memory.h"
#include "items.h"

#include "objs.h"

/* Create an instance of an object. */
OBJ_T *obj_create (OBJ_INDEX_T *obj_index, int level) {
    AFFECT_T *paf;
    OBJ_T *obj;
    int i;

    EXIT_IF_BUG (obj_index == NULL,
        "obj_create: NULL obj_index.", 0);

    obj = obj_new ();

    obj->index_data = obj_index;
    obj->in_room = NULL;
    obj->enchanted = FALSE;

    if (obj_index->new_format)
        obj->level = obj_index->level;
    else
        obj->level = UMAX (0, level);
    obj->wear_loc = -1;

    str_replace_dup (&obj->name,        obj_index->name);        /* OLC */
    str_replace_dup (&obj->short_descr, obj_index->short_descr); /* OLC */
    str_replace_dup (&obj->description, obj_index->description); /* OLC */
    obj->material    = obj_index->material;
    obj->item_type   = obj_index->item_type;
    obj->extra_flags = obj_index->extra_flags;
    obj->wear_flags  = obj_index->wear_flags;
    obj->weight      = obj_index->weight;
    for (i = 0; i < OBJ_VALUE_MAX; i++)
        obj->v.value[i] = obj_index->v.value[i];

    if (level == -1 || obj_index->new_format)
        obj->cost = obj_index->cost;
    else
        obj->cost = number_fuzzy (10)
            * number_fuzzy (level) * number_fuzzy (level);

    /* Mess with object properties. */
    item_init (obj, obj_index, level);

    for (paf = obj_index->affected; paf != NULL; paf = paf->next)
        if (paf->apply == APPLY_SPELL_AFFECT)
            affect_to_obj (obj, paf);

    LIST_FRONT (obj, next, object_list);
    obj_index->count++;
    return obj;
}

/* duplicate an object exactly -- except contents */
void obj_clone (OBJ_T *parent, OBJ_T *clone) {
    int i;
    AFFECT_T *paf;
    EXTRA_DESCR_T *ed, *ed_new;

    if (parent == NULL || clone == NULL)
        return;

    /* start fixing the object */
    str_replace_dup (&clone->name,        parent->name);
    str_replace_dup (&clone->short_descr, parent->short_descr);
    str_replace_dup (&clone->description, parent->description);
    clone->item_type   = parent->item_type;
    clone->extra_flags = parent->extra_flags;
    clone->wear_flags  = parent->wear_flags;
    clone->weight      = parent->weight;
    clone->cost        = parent->cost;
    clone->level       = parent->level;
    clone->condition   = parent->condition;
    clone->material    = parent->material;
    clone->timer       = parent->timer;

    for (i = 0; i < OBJ_VALUE_MAX; i++)
        clone->v.value[i] = parent->v.value[i];

    /* affects */
    clone->enchanted = parent->enchanted;
    for (paf = parent->affected; paf != NULL; paf = paf->next)
        affect_to_obj (clone, paf);

    /* extended desc */
    for (ed = parent->extra_descr; ed != NULL; ed = ed->next) {
        ed_new = extra_descr_new ();
        str_replace_dup (&ed_new->keyword, ed->keyword);
        str_replace_dup (&ed_new->description, ed->description);
        LIST_BACK (ed_new, next, clone->extra_descr, EXTRA_DESCR_T);
    }
}

/* returns number of people on an object */
int obj_count_users (const OBJ_T *obj) {
    CHAR_T *fch;
    int count = 0;

    if (obj->in_room == NULL)
        return 0;
    for (fch = obj->in_room->people; fch != NULL; fch = fch->next_in_room)
        if (fch->on == obj)
            count++;
    return count;
}

/* Give an obj to a char. */
void obj_give_to_char (OBJ_T *obj, CHAR_T *ch) {
    LIST_FRONT (obj, next_content, ch->carrying);
    obj->carried_by = ch;
    obj->in_room    = NULL;
    obj->in_obj     = NULL;

    ch->carry_number += obj_get_carry_number (obj);
    ch->carry_weight += obj_get_weight (obj);
}

/* Move an obj into a room. */
void obj_give_to_room (OBJ_T *obj, ROOM_INDEX_T *room_index) {
    LIST_FRONT (obj, next_content, room_index->contents);
    obj->carried_by = NULL;
    obj->in_room    = room_index;
    obj->in_obj     = NULL;
}

/* Move an object into an object. */
void obj_give_to_obj (OBJ_T *obj, OBJ_T *obj_to) {
    LIST_FRONT (obj, next_content, obj_to->contains);
    obj->carried_by = NULL;
    obj->in_room    = NULL;
    obj->in_obj     = obj_to;

    if (obj_to->index_data->vnum == OBJ_VNUM_PIT)
        obj->cost = 0;

    for (; obj_to != NULL; obj_to = obj_to->in_obj) {
        if (obj_to->carried_by != NULL) {
            obj_to->carried_by->carry_number += obj_get_carry_number (obj);
            obj_to->carried_by->carry_weight += obj_get_weight (obj)
                * item_get_weight_mult (obj_to) / 100;
        }
    }
}

/* insert an object at the right spot for the keeper */
void obj_give_to_keeper (OBJ_T *obj, CHAR_T *ch) {
    OBJ_T *t_obj, *t_obj_next;

    /* see if any duplicates are found */
    for (t_obj = ch->carrying; t_obj != NULL; t_obj = t_obj_next) {
        t_obj_next = t_obj->next_content;
        if (obj->index_data != t_obj->index_data)
            continue;
        if (str_cmp (obj->short_descr, t_obj->short_descr) != 0)
            continue;

        /* if this is an unlimited item, destroy the new one */
        if (IS_OBJ_STAT (t_obj, ITEM_INVENTORY)) {
            obj_extract (obj);
            return;
        }
        obj->cost = t_obj->cost; /* keep it standard */
    }

    LIST_INSERT_AFTER (obj, t_obj, next_content, ch->carrying);
    obj->carried_by = ch;
    obj->in_room    = NULL;
    obj->in_obj     = NULL;

    ch->carry_number += obj_get_carry_number (obj);
    ch->carry_weight += obj_get_weight (obj);
}

/* Take the object from whoever or whatever is holding it. */
void obj_take (OBJ_T *obj) {
    if (obj->carried_by) {
        obj_take_from_char (obj);
        return;
    }
    else if (obj->in_room) {
        obj_take_from_room (obj);
        return;
    }
    else if (obj->in_obj) {
        obj_take_from_obj (obj);
        return;
    }
    bug ("obj_take: object not carried, in room, or in object.", 0);
}

/* Take an obj from its character. */
void obj_take_from_char (OBJ_T *obj) {
    CHAR_T *ch;

    BAIL_IF_BUG ((ch = obj->carried_by) == NULL,
        "obj_take_from_char: null ch.", 0);
    if (obj->wear_loc != WEAR_LOC_NONE)
        char_unequip_obj (ch, obj);

    LIST_REMOVE (obj, next_content, ch->carrying, OBJ_T, NO_FAIL);
    obj->carried_by = NULL;
    ch->carry_number -= obj_get_carry_number (obj);
    ch->carry_weight -= obj_get_weight (obj);
}

/* Move an obj out of a room. */
void obj_take_from_room (OBJ_T *obj) {
    ROOM_INDEX_T *in_room;
    CHAR_T *ch;

    BAIL_IF_BUG ((in_room = obj->in_room) == NULL,
        "obj_take_from_room: NULL.", 0);
    for (ch = in_room->people; ch != NULL; ch = ch->next_in_room)
        if (ch->on == obj)
            ch->on = NULL;

    LIST_REMOVE (obj, next_content, in_room->contents, OBJ_T, NO_FAIL);
    obj->in_room = NULL;
}

/* Move an object out of an object. */
void obj_take_from_obj (OBJ_T *obj) {
    OBJ_T *obj_from;
    BAIL_IF_BUG ((obj_from = obj->in_obj) == NULL,
        "obj_take_from_obj: null obj_from.", 0);

    LIST_REMOVE (obj, next_content, obj_from->contains, OBJ_T, NO_FAIL);
    obj->in_obj = NULL;

    for (; obj_from != NULL; obj_from = obj_from->in_obj) {
        if (obj_from->carried_by != NULL) {
            obj_from->carried_by->carry_number -= obj_get_carry_number (obj);
            obj_from->carried_by->carry_weight -= obj_get_weight (obj)
                * item_get_weight_mult (obj_from) / 100;
        }
    }
}

/* Find the ac value of an obj, including position effect. */
int obj_get_ac_type (const OBJ_T *obj, int wear_loc, int type) {
    const WEAR_LOC_T *wear_loc_t;
    int ac_value;

    if ((ac_value = item_get_ac (obj, type)) == 0)
        return 0;
    if ((wear_loc_t = wear_loc_get (wear_loc)) == NULL)
        return 0;
    if (wear_loc_t->ac_bonus == 0)
        return 0;
    return (ac_value * wear_loc_t->ac_bonus) / 100;
}

/* Count occurrences of an obj in a list. */
int obj_index_count_in_list (const OBJ_INDEX_T *obj_index, const OBJ_T *list) {
    const OBJ_T *obj;
    int matches;

    matches = 0;
    for (obj = list; obj != NULL; obj = obj->next_content)
        if (obj->index_data == obj_index)
            matches++;
    return matches;
}

/* Extract an obj from the world. */
void obj_extract (OBJ_T *obj) {
    OBJ_T *obj_content;
    OBJ_T *obj_next;

    if (obj->in_room != NULL)
        obj_take_from_room (obj);
    else if (obj->carried_by != NULL)
        obj_take_from_char (obj);
    else if (obj->in_obj != NULL)
        obj_take_from_obj (obj);

    for (obj_content = obj->contains; obj_content; obj_content = obj_next) {
        obj_next = obj_content->next_content;
        obj_extract (obj_content);
    }

    LIST_REMOVE (obj, next, object_list, OBJ_T, return);
    --obj->index_data->count;
    obj_free (obj);
}

/* Create a 'money' obj. */
OBJ_T *obj_create_money (int gold, int silver) {
    char buf[MAX_STRING_LENGTH];
    OBJ_T *obj;

    if (gold < 0 || silver < 0 || (gold == 0 && silver == 0)) {
        bug ("obj_create_money: zero or negative money.", UMIN (gold, silver));
        gold = UMAX (1, gold);
        silver = UMAX (1, silver);
    }

    if (gold == 0 && silver == 1)
        obj = obj_create (get_obj_index (OBJ_VNUM_SILVER_ONE), 0);
    else if (gold == 1 && silver == 0)
        obj = obj_create (get_obj_index (OBJ_VNUM_GOLD_ONE), 0);
    else if (silver == 0) {
        obj = obj_create (get_obj_index (OBJ_VNUM_GOLD_SOME), 0);
        sprintf (buf, obj->short_descr, gold);
        str_replace_dup (&(obj->short_descr), buf);
        obj->v.money.gold = gold;
        obj->cost = gold;
        obj->weight = gold / 5;
    }
    else if (gold == 0) {
        obj = obj_create (get_obj_index (OBJ_VNUM_SILVER_SOME), 0);
        sprintf (buf, obj->short_descr, silver);
        str_replace_dup (&(obj->short_descr), buf);
        obj->v.money.silver = silver;
        obj->cost = silver;
        obj->weight = silver / 20;
    }
    else {
        obj = obj_create (get_obj_index (OBJ_VNUM_COINS), 0);
        sprintf (buf, obj->short_descr, silver, gold);
        str_replace_dup (&(obj->short_descr), buf);
        obj->v.money.silver = silver;
        obj->v.money.gold   = gold;
        obj->cost = 100 * gold + silver;
        obj->weight = gold / 5 + silver / 20;
    }
    return obj;
}

/* Return # of objects which an object counts as.
 * Thanks to Tony Chamberlain for the correct recursive code here. */
int obj_get_carry_number (const OBJ_T *obj) {
    int number = item_get_carry_number (obj);
    for (obj = obj->contains; obj != NULL; obj = obj->next_content)
        number += obj_get_carry_number (obj);
    return number;
}

/* Return weight of an object, including weight of contents. */
int obj_get_weight (const OBJ_T *obj) {
    int weight, mult;

    weight = obj->weight;
    mult = item_get_weight_mult (obj);
    for (obj = obj->contains; obj != NULL; obj = obj->next_content)
        weight += obj_get_weight (obj) * mult / 100;

    return weight;
}

int obj_get_true_weight (const OBJ_T *obj) {
    int weight;

    weight = obj->weight;
    for (obj = obj->contains; obj != NULL; obj = obj->next_content)
        weight += obj_get_true_weight (obj);

    return weight;
}

char *obj_format_to_char (const OBJ_T *obj, const CHAR_T *ch, bool is_short) {
    static char buf[MAX_STRING_LENGTH];
    buf[0] = '\0';

    if ((is_short && (obj->short_descr == NULL || obj->short_descr[0] == '\0'))
        || (obj->description == NULL || obj->description[0] == '\0'))
        return buf;

#ifdef BASEMUD_COLOR_STATUS_EFFECTS
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

    if (IS_SET (ch->comm, COMM_MATERIALS))
        material_strcat (buf, material_get (obj->material));

    if (is_short) {
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
void obj_list_show_to_char (const OBJ_T *list, CHAR_T *ch, bool is_short,
    bool show_nothing)
{
    const OBJ_T *obj;
    char buf[MAX_STRING_LENGTH];
    BUFFER_T *output;
    char **show_strings;
    int *show_string_counts;
    char *obj_show_string;
    int show_string_num;
    int i;
    int count;
    bool combine;

    if (ch->desc == NULL)
        return;

    /* Alloc space for output lines.  */
    output = buf_new ();

    count = 0;
    for (obj = list; obj != NULL; obj = obj->next_content)
        count++;
    show_strings = mem_alloc (count * sizeof (char *));
    show_string_counts = mem_alloc (count * sizeof (int));
    show_string_num = 0;

    /* Format the list of objects.  */
    for (obj = list; obj != NULL; obj = obj->next_content) {
        if (obj->wear_loc != WEAR_LOC_NONE)
            continue;
        if (!char_can_see_obj (ch, obj))
            continue;

        obj_show_string= obj_format_to_char (obj, ch, is_short);
        combine = FALSE;

        if (IS_NPC (ch) || IS_SET (ch->comm, COMM_COMBINE)) {
            /* Look for duplicates, case sensitive.
             * Matches tend to be near end so run loop backwords. */
            for (i = show_string_num - 1; i >= 0; i--) {
                if (!strcmp (show_strings[i], obj_show_string)) {
                    show_string_counts[i]++;
                    combine = TRUE;
                    break;
                }
            }
        }

        /* Couldn't combine, or didn't want to. */
        if (!combine) {
            show_strings[show_string_num] = str_dup (obj_show_string);
            show_string_counts[show_string_num] = 1;
            show_string_num++;
        }
    }

    /* Output the formatted list. */
    for (i = 0; i < show_string_num; i++) {
        if (show_strings[i][0] == '\0') {
            str_free (&(show_strings[i]));
            continue;
        }

        if (IS_NPC (ch) || IS_SET (ch->comm, COMM_COMBINE)) {
            if (show_string_counts[i] != 1) {
                sprintf (buf, "(%2d) ", show_string_counts[i]);
                buf_cat (output, buf);
            }
            else
                buf_cat (output, "     ");
        }
        buf_cat (output, show_strings[i]);
        buf_cat (output, "\n\r");
        str_free (&(show_strings[i]));
    }

    if (show_nothing && show_string_num == 0) {
        if (IS_NPC (ch) || IS_SET (ch->comm, COMM_COMBINE))
            send_to_char ("     ", ch);
        send_to_char ("Nothing.\n\r", ch);
    }
    page_to_char (buf_string (output), ch);

    /* Clean up. */
    buf_free (output);
    mem_free (show_strings, count * sizeof (char *));
    mem_free (show_string_counts, count * sizeof (int));
}

const char *obj_furn_preposition_base (const OBJ_T *obj, int position,
    const char *at, const char *on, const char *in, const char *by)
{
    int pos_type;
    pos_type = item_get_furn_preposition_type (obj, position);
    switch (pos_type) {
        case POS_PREP_NO_OBJECT:     return "(no object)";
        case POS_PREP_NOT_FURNITURE: return "(not furniture)";
        case POS_PREP_BAD_POSITION:  return "(bad position)";
        case POS_PREP_AT:            return at;
        case POS_PREP_ON:            return on;
        case POS_PREP_IN:            return in;
        case POS_PREP_BY:            return by;
        default:                     return "(unknown)";
    }
}

const char *obj_furn_preposition (const OBJ_T *obj, int position) {
    return obj_furn_preposition_base (obj, position, "at", "on", "in", "by");
}

/* Find some object with a given index data.
 * Used by area-reset 'P' command. */
OBJ_T *obj_get_by_index (const OBJ_INDEX_T *obj_index) {
    OBJ_T *obj;
    for (obj = object_list; obj != NULL; obj = obj->next)
        if (obj->index_data == obj_index)
            return obj;
    return NULL;
}

void obj_enchant (OBJ_T *obj) {
    AFFECT_T *paf, *af_new;

    if (obj->enchanted)
        return;
    obj->enchanted = TRUE;

    for (paf = obj->index_data->affected; paf != NULL; paf = paf->next) {
        af_new = affect_new ();
        affect_copy (af_new, paf);
        af_new->type = UMAX (0, af_new->type);
        LIST_FRONT (af_new, next, obj->affected);
    }
}

/* Former macros. */
bool obj_can_wear_flag (const OBJ_T *obj, flag_t wear_flag) {
    if (IS_SET ((obj)->wear_flags, wear_flag))
        return TRUE;
    if (item_can_wear_flag (obj, wear_flag))
        return TRUE;
    return FALSE;
}

bool obj_index_can_wear_flag (const OBJ_INDEX_T *obj, flag_t wear_flag) {
    if (IS_SET ((obj)->wear_flags, wear_flag))
        return TRUE;
    if (item_index_can_wear_flag (obj, wear_flag))
        return TRUE;
    return FALSE;
}

bool obj_should_spill_contents_when_poofed (const OBJ_T *obj) {
    if (obj->wear_loc == WEAR_LOC_FLOAT)
        return TRUE;
    if (item_should_spill_contents_when_poofed (obj))
        return TRUE;
    return FALSE;
}

void obj_poof (OBJ_T *obj) {
    CHAR_T *rch;
    const char *message;

    message = item_get_poof_message (obj);

    if (obj->carried_by != NULL) {
        if (IS_NPC (obj->carried_by)
                && obj->carried_by->index_data->shop != NULL)
            obj->carried_by->silver += obj->cost / 5;
        else {
            act (message, obj->carried_by, obj, NULL, TO_CHAR);
            if (obj->wear_loc == WEAR_LOC_FLOAT)
                act (message, obj->carried_by, obj, NULL, TO_NOTCHAR);
        }
    }
    else if (obj->in_room != NULL && (rch = obj->in_room->people) != NULL) {
        if (!(obj->in_obj && obj->in_obj->index_data->vnum == OBJ_VNUM_PIT
              && !obj_can_wear_flag (obj->in_obj, ITEM_TAKE)))
        {
            act (message, rch, obj, NULL, TO_CHAR);
            act (message, rch, obj, NULL, TO_NOTCHAR);
        }
    }

    /* save the contents */
    if (obj->contains && obj_should_spill_contents_when_poofed (obj)) {
        OBJ_T *t_obj, *next_obj;
        for (t_obj = obj->contains; t_obj != NULL; t_obj = next_obj) {
            next_obj = t_obj->next_content;
            obj_take_from_obj (t_obj);

            /* in another object */
            if (obj->in_obj)
                obj_give_to_obj (t_obj, obj->in_obj);
            /* carried */
            else if (obj->carried_by) {
                if (obj->wear_loc == WEAR_LOC_FLOAT) {
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

ROOM_INDEX_T *obj_get_room (const OBJ_T *obj) {
    const OBJ_T *o;
    for (o = obj; o != NULL; o = o->in_obj) {
        if (o->carried_by)
            return o->carried_by->in_room;
        if (o->in_room)
            return o->in_room;
    }
    return NULL;
}
