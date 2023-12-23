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

#include "affects.h"

#include "chars.h"
#include "lookup.h"
#include "recycle.h"
#include "utils.h"

#include <stdio.h>
#include <string.h>

void affect_modify_char_bits (AFFECT_T *paf, CHAR_T *ch, bool on) {
    if (on) {
        switch (paf->bit_type) {
            case AFF_TO_AFFECTS: SET_BIT (ch->affected_by, paf->bits); break;
            case AFF_TO_IMMUNE:  SET_BIT (ch->imm_flags,   paf->bits); break;
            case AFF_TO_RESIST:  SET_BIT (ch->res_flags,   paf->bits); break;
            case AFF_TO_VULN:    SET_BIT (ch->vuln_flags,  paf->bits); break;
        }
    }
    else {
        switch (paf->bit_type) {
            case AFF_TO_AFFECTS: REMOVE_BIT (ch->affected_by, paf->bits); break;
            case AFF_TO_IMMUNE:  REMOVE_BIT (ch->imm_flags,   paf->bits); break;
            case AFF_TO_RESIST:  REMOVE_BIT (ch->res_flags,   paf->bits); break;
            case AFF_TO_VULN:    REMOVE_BIT (ch->vuln_flags,  paf->bits); break;
        }
    }
}

void affect_modify_char_apply (AFFECT_T *paf, CHAR_T *ch, bool on) {
    int mod, i;

    mod = on ? (paf->modifier) : (0 - paf->modifier);
    switch (paf->apply) {
        case APPLY_NONE:          break;
        case APPLY_STR:           ch->mod_stat[STAT_STR] += mod; break;
        case APPLY_DEX:           ch->mod_stat[STAT_DEX] += mod; break;
        case APPLY_INT:           ch->mod_stat[STAT_INT] += mod; break;
        case APPLY_WIS:           ch->mod_stat[STAT_WIS] += mod; break;
        case APPLY_CON:           ch->mod_stat[STAT_CON] += mod; break;
        case APPLY_SEX:           ch->sex += mod; break;
        case APPLY_CLASS:         break;
        case APPLY_LEVEL:         break;
        case APPLY_AGE:           break;
        case APPLY_HEIGHT:        break;
        case APPLY_WEIGHT:        break;
        case APPLY_MANA:          ch->max_mana += mod; break;
        case APPLY_HIT:           ch->max_hit += mod; break;
        case APPLY_MOVE:          ch->max_move += mod; break;
        case APPLY_GOLD:          break;
        case APPLY_EXP:           break;
        case APPLY_HITROLL:       ch->hitroll += mod; break;
        case APPLY_DAMROLL:       ch->damroll += mod; break;
        case APPLY_SAVES:         ch->saving_throw += mod; break;
        case APPLY_SAVING_ROD:    ch->saving_throw += mod; break;
        case APPLY_SAVING_PETRI:  ch->saving_throw += mod; break;
        case APPLY_SAVING_BREATH: ch->saving_throw += mod; break;
        case APPLY_SAVING_SPELL:  ch->saving_throw += mod; break;
        case APPLY_SPELL_AFFECT:  break;

        case APPLY_AC:
            for (i = 0; i < 4; i++)
                ch->armor[i] += mod;
            break;

        default:
            bug ("affect_modify_char_apply: unknown apply %d.", paf->apply);
            return;
    }
}

/* Apply or remove an affect to a character. */
void affect_modify_char (AFFECT_T *paf, CHAR_T *ch, bool on) {
    /* Modifiy various bits and other stats from 'apply'. */
    affect_modify_char_bits  (paf, ch, on);
    affect_modify_char_apply (paf, ch, on);

    /* Drop our weapon if it's suddenly too heavy. */
    char_drop_weapon_if_too_heavy (ch);
}

void affect_modify_obj (AFFECT_T *paf, OBJ_T *obj) {
    /* are there bits to apply? */
    if (paf->bits) {
        switch (paf->bit_type) {
            /* apply any object affect bits to the object's extra_flags */
            case AFF_TO_OBJECT:
                SET_BIT (obj->extra_flags, paf->bits);
                break;
            /* apply any weapon affect bits to the weapon-specific flags */
            case AFF_TO_WEAPON:
                if (obj->item_type == ITEM_WEAPON)
                    SET_BIT (obj->v.weapon.flags, paf->bits);
                break;
        }
    }
}

/* find an effect in an affect list */
AFFECT_T *affect_find (AFFECT_T *paf, int sn) {
    AFFECT_T *paf_find;
    for (paf_find = paf; paf_find != NULL; paf_find = paf_find->on_next)
        if (paf_find->type == sn)
            return paf_find;
    return NULL;
}

/* fix object affects when removing one */
void affect_check_char (CHAR_T *ch, int bit_type, flag_t bits) {
    AFFECT_T *paf;
    OBJ_T *obj;

    if (ch == NULL)
        return;
    if (bit_type == AFF_TO_OBJECT || bit_type == AFF_TO_WEAPON || bits == 0)
        return;

    for (paf = ch->affect_first; paf != NULL; paf = paf->on_next) {
        if (paf->bit_type == bit_type && paf->bits == bits) {
            affect_modify_char_bits (paf, ch, TRUE);
            return;
        }
    }

    for (obj = ch->content_first; obj != NULL; obj = obj->content_next) {
        if (obj->wear_loc == WEAR_LOC_NONE)
            continue;

        for (paf = obj->affect_first; paf != NULL; paf = paf->on_next)
            if (paf->bit_type == bit_type && paf->bits == bits)
                affect_modify_char_bits (paf, ch, TRUE);

        if (obj->enchanted)
            continue;
        for (paf = obj->obj_index->affect_first; paf; paf = paf->on_next)
            if (paf->bit_type == bit_type && paf->bits == bits)
                affect_modify_char_bits (paf, ch, TRUE);
    }
}

void affect_copy (AFFECT_T *dest, AFFECT_T *src) {
    dest->bit_type = src->bit_type;
    dest->type     = src->type;
    dest->level    = src->level;
    dest->duration = src->duration;
    dest->apply    = src->apply;
    dest->modifier = src->modifier;
    dest->bits     = src->bits;
}

/* Give an affect to a char. */
void affect_copy_to_char (AFFECT_T *paf, CHAR_T *ch) {
    AFFECT_T *paf_new;

    paf_new = affect_new ();
    affect_copy (paf_new, paf);
    affect_to_char_front (paf_new, ch);
    affect_modify_char (paf_new, ch, TRUE);
}

/* give an affect to an object */
void affect_copy_to_obj (AFFECT_T *paf, OBJ_T *obj) {
    AFFECT_T *paf_new;

    paf_new = affect_new ();
    affect_copy (paf_new, paf);
    affect_to_obj_front (paf_new, obj);
    affect_modify_obj (paf_new, obj);
}

void affect_to_char_front (AFFECT_T *paf, CHAR_T *ch) {
    paf->parent_type = AFFECT_CHAR;
    paf->parent = ch;
    LIST2_FRONT (paf, on_prev, on_next,
        ch->affect_first, ch->affect_last);
}

void affect_to_char_back (AFFECT_T *paf, CHAR_T *ch) {
    paf->parent_type = AFFECT_CHAR;
    paf->parent = ch;
    LIST2_BACK (paf, on_prev, on_next,
        ch->affect_first, ch->affect_last);
}

void affect_to_obj_front (AFFECT_T *paf, OBJ_T *obj) {
    paf->parent_type = AFFECT_OBJ;
    paf->parent = obj;
    LIST2_FRONT (paf, on_prev, on_next,
        obj->affect_first, obj->affect_last);
}

void affect_to_obj_back (AFFECT_T *paf, OBJ_T *obj) {
    paf->parent_type = AFFECT_OBJ;
    paf->parent = obj;
    LIST2_BACK (paf, on_prev, on_next,
        obj->affect_first, obj->affect_last);
}

void affect_to_obj_index_back (AFFECT_T *paf, OBJ_INDEX_T *obj_index) {
    paf->parent_type = AFFECT_OBJ_INDEX;
    paf->parent = obj_index;
    LIST2_BACK (paf, on_prev, on_next,
        obj_index->affect_first, obj_index->affect_last);
}

void affect_unlink (AFFECT_T *paf) {
    switch (paf->parent_type) {
        case AFFECT_CHAR: {
            CHAR_T *ch = paf->parent;
            if (ch != NULL) {
                paf->parent = NULL;
                LIST2_REMOVE (paf, on_prev, on_next,
                    ch->affect_first, ch->affect_last);
            }
            break;
        }

        case AFFECT_OBJ: {
            OBJ_T *obj = paf->parent;
            if (obj != NULL) {
                paf->parent = NULL;
                LIST2_REMOVE (paf, on_prev, on_next,
                    obj->affect_first, obj->affect_last);
            }
            break;
        }

        case AFFECT_OBJ_INDEX: {
            OBJ_INDEX_T *obj_index = paf->parent;
            if (obj_index != NULL) {
                paf->parent = NULL;
                LIST2_REMOVE (paf, on_prev, on_next,
                    obj_index->affect_first, obj_index->affect_last);
            }
            break;
        }

        default:
            paf->parent = NULL;
    }

    paf->parent_type = AFFECT_NONE;
}

/* Removes from affect from anything. */
void affect_remove (AFFECT_T *paf) {
    switch (paf->parent_type) {
        case AFFECT_CHAR:
            affect_remove_char (paf);
            break;

        case AFFECT_OBJ:
            affect_remove_obj (paf);
            break;

        default:
            affect_free (paf);
    }
}

/* Remove an affect from a char. */
void affect_remove_char (AFFECT_T *paf) {
    CHAR_T *ch;
    int bit_type;
    flag_t bits;

    BAIL_IF_BUG ((ch = paf->parent) == NULL,
        "affect_remove_char: no affect.", 0);

    affect_modify_char (paf, ch, FALSE);
    bit_type = paf->bit_type;
    bits     = paf->bits;

    affect_free (paf);
    affect_check_char (ch, bit_type, bits);
}

void affect_remove_obj (AFFECT_T *paf) {
    OBJ_T *obj;
    int bit_type, bits;

    BAIL_IF_BUG ((obj = paf->parent) == NULL,
        "affect_remove_obj: no affect.", 0);

    paf->parent = NULL;
    if (obj->carried_by != NULL && obj->wear_loc != WEAR_LOC_NONE)
        affect_modify_char (paf, obj->carried_by, FALSE);

    bit_type = paf->bit_type;
    bits     = paf->bits;

    /* remove flags from the object if needed */
    if (paf->bits) {
        switch (paf->bit_type) {
            /* apply any object affect bits to the object's extra_flags */
            case AFF_TO_OBJECT:
                REMOVE_BIT (obj->extra_flags, paf->bits);
                break;

            /* apply any weapon affect bits to the weapon-specific flags */
            case AFF_TO_WEAPON:
                if (obj->item_type == ITEM_WEAPON)
                    REMOVE_BIT (obj->v.weapon.flags, paf->bits);
                break;
        }
    }

    affect_free (paf);
    if (obj->carried_by != NULL && obj->wear_loc != WEAR_LOC_NONE)
        affect_check_char (obj->carried_by, bit_type, bits);
}

/* Strip all affects of a given sn. */
void affect_strip_char (CHAR_T *ch, int sn) {
    AFFECT_T *paf, *paf_next;
    for (paf = ch->affect_first; paf != NULL; paf = paf_next) {
        paf_next = paf->on_next;
        if (paf->type == sn)
            affect_remove (paf);
    }
}

/* Return true if a char is affected by a spell. */
bool affect_is_char_affected (CHAR_T *ch, int sn) {
    AFFECT_T *paf;
    for (paf = ch->affect_first; paf != NULL; paf = paf->on_next)
        if (paf->type == sn)
            return TRUE;
    return FALSE;
}

/* Add or enhance an affect. */
void affect_join_char (AFFECT_T *paf, CHAR_T *ch) {
    AFFECT_T *paf_old;
#ifdef BASEMUD_CAP_JOINED_AFFECTS
    int lower, higher, sum;
#endif

    for (paf_old = ch->affect_first; paf_old != NULL; paf_old = paf_old->on_next) {
        if (paf_old->type != paf->type)
            continue;

        paf->level = (paf->level + paf_old->level) / 2;
        paf->duration += paf_old->duration;

#ifdef BASEMUD_CAP_JOINED_AFFECTS
        /* we can't be weaker or stronger than either one of the affects
         * we're merging.  otherwise, something like chill touch could
         * cause -10 str penalty upon multiple hits, which is crazy OP. */
        lower  = UMIN (paf->modifier, paf_old->modifier);
        higher = UMAX (paf->modifier, paf_old->modifier);
        sum    = paf->modifier + paf_old->modifier;
        paf->modifier = URANGE (lower, sum, higher);
#else
        paf->modifier += paf_old->modifier;
#endif

        affect_remove (paf_old);
        break;
    }
    affect_copy_to_char (paf, ch);
}

void affect_init (AFFECT_T *af, sh_int bit_type, sh_int type, sh_int level,
    sh_int duration, sh_int apply, sh_int modifier, flag_t bits)
{
    af->bit_type  = bit_type;
    af->type      = type;
    af->level     = level;
    af->duration  = duration;
    af->apply     = apply;
    af->modifier  = modifier;
    af->bits      = bits;

    /* presumably, we aren't initializing invalidated
     * objects... but this could break something if we did! */
    af->rec_data.valid = TRUE;
}

char *affect_bit_message (int bit_type, flag_t bits) {
    static char buf[MAX_STRING_LENGTH];
    switch (bit_type) {
        case AFF_TO_AFFECTS:
            sprintf (buf, "Adds %s affect.\n\r", affect_bit_name (bits));
            break;
        case AFF_TO_OBJECT:
            sprintf (buf, "Adds %s object flag.\n\r", extra_bit_name (bits));
            break;
        case AFF_TO_IMMUNE:
            sprintf (buf, "Adds immunity to %s.\n\r", res_bit_name (bits));
            break;
        case AFF_TO_RESIST:
            sprintf (buf, "Adds resistance to %s.\n\r", res_bit_name (bits));
            break;
        case AFF_TO_VULN:
            sprintf (buf, "Adds vulnerability to %s.\n\r", res_bit_name (bits));
            break;
        default:
            sprintf (buf, "Unknown bit %d: %ld\n\r", bit_type, bits);
            break;
    }
    return buf;
}
