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
 *    ROM 2.4 is copyright 1993-1998 Russ Taylor                           *
 *    ROM has been brought to you by the ROM consortium                    *
 *        Russ Taylor (rtaylor@hypercube.org)                              *
 *        Gabrielle Taylor (gtaylor@hypercube.org)                         *
 *        Brian Moore (zump@rom.org)                                       *
 *    By using this code, you have agreed to follow the terms of the       *
 *    ROM license, in the file Rom24/doc/rom.license                       *
 ***************************************************************************/

#ifndef __ROM_MACROS_H
#define __ROM_MACROS_H

/* Thanks Dingo for making life a bit easier ;) */
#define CH(d)   ((d)->original ? (d)->original : (d)->character)
#define OCH(ch) (((ch)->desc) ? CH((ch)->desc) : (ch))

/* Utility macros. */
#define IS_VALID(data)       ((data) != NULL && (data)->rec_data.valid)
#define UMIN(a, b)           ((a) < (b) ? (a) : (b))
#define UMAX(a, b)           ((a) > (b) ? (a) : (b))
#define URANGE(a, b, c)      ((b) < (a) ? (a) : ((b) > (c) ? (c) : (b)))
#define LOWER(c)             ((c) >= 'A' && (c) <= 'Z' ? (c)+'a'-'A' : (c))
#define UPPER(c)             ((c) >= 'a' && (c) <= 'z' ? (c)+'A'-'a' : (c))
#define IS_SET(flag, bit)    ((flag) & (bit))
#define SET_BIT(var, bit)    ((var) |= (bit))
#define REMOVE_BIT(var, bit) ((var) &= ~(bit))
#define TOGGLE_BIT(var, bit) ((var) ^= (bit))
#define IS_NULLSTR(str)      ((str) == NULL || (str)[0] == '\0')
#define ENTRE(min,num,max)   (((min) < (num)) && ((num) < (max)))
#define ARE_SET(flag, bit)   (((flag) & (bit)) == (bit))
#define NONE_SET(flag, bit)  (((flag) & (bit)) == 0)

/* Character macros. */
#define IS_NPC(ch)           (IS_SET((ch)->mob, MOB_IS_NPC))
#define IS_IMMORTAL(ch)      (char_get_trust(ch) >= LEVEL_IMMORTAL)
#define IS_HERO(ch)          (char_get_trust(ch) >= LEVEL_HERO)
#define IS_TRUSTED(ch,level) (char_get_trust((ch)) >= (level))
#define IS_AFFECTED(ch, sn)  (IS_SET((ch)->affected_by, (sn)))

#define IS_SOBER(ch)         (!IS_NPC(ch) && (ch)->pcdata && \
                              (ch)->pcdata->condition[COND_DRUNK] <= 0)
#define IS_DRUNK(ch)         (!IS_NPC(ch) && (ch)->pcdata && \
                              (ch)->pcdata->condition[COND_DRUNK] > 10)
#define IS_THIRSTY(ch)       (!IS_NPC(ch) && (ch)->pcdata && \
                              (ch)->pcdata->condition[COND_THIRST] <= 0)
#define IS_QUENCHED(ch)      (!IS_NPC(ch) && (ch)->pcdata && \
                              (ch)->pcdata->condition[COND_THIRST] > 40)
#define IS_HUNGRY(ch)        (!IS_NPC(ch) && (ch)->pcdata && \
                              (ch)->pcdata->condition[COND_HUNGER] <= 0)
#define IS_FED(ch)           (!IS_NPC(ch) && (ch)->pcdata && \
                              (ch)->pcdata->condition[COND_HUNGER] > 40)
#define IS_FULL(ch)          (!IS_NPC(ch) && (ch)->pcdata && \
                              (ch)->pcdata->condition[COND_FULL] > 40)
#define IS_PET(ch)           (IS_NPC(ch) && IS_SET((ch)->mob, MOB_PET))

#define GET_AGE(ch) \
    ((int) (17 + ((ch)->played \ + current_time - (ch)->logon ) / 72000))

#define IS_GOOD(ch)         (ch->alignment >=  350)
#define IS_EVIL(ch)         (ch->alignment <= -350)
#define IS_REALLY_GOOD(ch)  (ch->alignment >=  750)
#define IS_REALLY_EVIL(ch)  (ch->alignment <= -750)
#define IS_NEUTRAL(ch)      (!IS_GOOD(ch) && !IS_EVIL(ch))
#define IS_OUTSIDE(ch)      (!IS_SET ((ch)->in_room->room_flags, ROOM_INDOORS))
#define IS_AWAKE(ch)        (ch->position > POS_SLEEPING)

#define IS_SAME_ALIGN(ch1, ch2) \
    (!IS_SET (ch1->mob, MOB_NOALIGN) && \
     !IS_SET (ch2->mob, MOB_NOALIGN) && \
     ((IS_GOOD(ch1) && IS_GOOD(ch2)) || \
      (IS_EVIL(ch1) && IS_EVIL(ch2)) || \
      (IS_NEUTRAL(ch1) && IS_NEUTRAL(ch2))))

#define GET_AC(ch,type) \
    ((ch)->armor[type] + (IS_AWAKE(ch) \
        ? dex_app[char_get_curr_stat(ch,STAT_DEX)].defensive : 0 ))
#define GET_HITROLL(ch) \
    ((ch)->hitroll + str_app[char_get_curr_stat(ch,STAT_STR)].tohit)
#define GET_DAMROLL(ch) \
    ((ch)->damroll + str_app[char_get_curr_stat(ch,STAT_STR)].todam)

#define WAIT_STATE(ch, npulse) \
    ((ch)->wait = (IS_TRUSTED(ch, IMPLEMENTOR) \
        ? (ch)->wait \
        : UMAX((ch)->wait, (npulse)) \
    ))
#define DAZE_STATE(ch, npulse) \
    ((ch)->daze = (IS_TRUSTED(ch, IMPLEMENTOR) \
        ? (ch)->daze \
        : UMAX((ch)->daze, (npulse)) \
    ))

#define act(format, ch, arg1, arg2, flags) \
    act_new((format), (ch), (arg1), (arg2), (flags), POS_RESTING)

#define HAS_TRIGGER(ch,trig)  (IS_SET((ch)->pIndexData->mprog_flags,(trig)))
#define HAS_ANY_TRIGGER(ch)   ((ch)->pIndexData->mprog_flags != 0)

#define IS_SWITCHED( ch )     ( ch->desc && ch->desc->original )
#define IS_BUILDER(ch, area) \
    ( !IS_NPC(ch) && !IS_SWITCHED(ch) && \
        ( ch->pcdata->security >= area->security \
          || strstr(area->builders, ch->name ) \
          || strstr(area->builders, "All" ) \
        ) \
    )

/* Object macros. */
#define CAN_WEAR(obj, part) \
    ((IS_SET((obj)->wear_flags, (part))) || \
     ((obj)->item_type == ITEM_LIGHT && (part) == ITEM_WEAR_LIGHT))
#define IS_OBJ_STAT(obj, stat)    (IS_SET((obj)->extra_flags, (stat)))
#define IS_WEAPON_STAT(obj,stat)  (IS_SET((obj)->value[4],(stat)))
#define WEIGHT_MULT(obj) \
    ((obj)->item_type == ITEM_CONTAINER ? (obj)->value[4] : 100)

/* Description macros. */
#define PERS(ch) \
    (IS_NPC(ch) ? (ch)->short_descr : (ch)->name)
#define PERS_AW(ch, looker) \
    (char_can_see_anywhere((looker), (ch)) ? (PERS(ch)) : "someone")
#define PERS_IR(ch, looker) \
    (char_can_see_in_room((looker), (ch)) ? (PERS(ch)) : "someone")

/* Read in a char. */
#if defined(KEY)
    #undef KEY
#endif
#define KEY(literal, field, value)  \
    if (!str_cmp(word, literal)) {  \
        field  = value;             \
        fMatch = TRUE;              \
        break;                      \
    }

/* provided to free strings */
#if defined(KEYS)
    #undef KEYS
#endif
#define KEYS(literal, field, value) \
    if (!str_cmp(word, literal)) {  \
        str_free(field);            \
        field  = value;             \
        fMatch = TRUE;              \
        break;                      \
    }

#define GET_OFFSET(type, member) \
    (offsetof(type, member))
#define APPLY_OFFSET(type, ptr, offset) \
    ((type *) ((ptr) + (offset)))
#define DEREF_OFFSET(type, ptr, offset) \
    (*(APPLY_OFFSET(type, ptr, offset)))
#define ASSIGN_OFFSET(type, ptr, offset, value) \
    ((DEREF_OFFSET(type, ptr, offset)) = (value))

#define LIST_FIND(cond, nxt, lst, out_obj) \
    do { \
        for (out_obj = lst; out_obj; out_obj = out_obj->nxt) \
            if (cond) \
                break; \
    } while (0)

#define LIST_FIND_WITH_PREV(cond, nxt, lst, out_obj, out_pobj) \
    do { \
        out_pobj = NULL; \
        for (out_obj = lst; out_obj; out_pobj=out_obj, out_obj=out_obj->nxt) \
            if (cond) \
                break; \
    } while (0)

#define LIST_FRONT(obj, nxt, lst) \
    do { \
        obj->nxt = lst; \
        lst = obj; \
    } while (0)

#define LIST_INSERT_AFTER(obj, after, nxt, lst) \
    do { \
        if (after == NULL) { \
            obj->nxt = lst; \
            lst = obj; \
        } \
        else { \
            obj->nxt = after->nxt; \
            after->nxt = obj; \
        } \
    } while (0)

#define LIST_BACK(obj, nxt, lst, vtype) \
    do { \
        if (lst == NULL) \
            lst = obj; \
        else { \
            vtype *o; \
            for (o = lst; o->nxt; o = o->nxt) \
                ; \
            o->nxt = obj; \
        } \
        obj->nxt = NULL; \
    } while (0)

#define NO_FAIL

#define LIST_REMOVE(obj, nxt, lst, vtype, fail) \
    do { \
        if (obj == lst) \
            lst = obj->nxt; \
        else { \
            vtype *pobj; \
            for (pobj = lst; pobj && pobj->nxt != obj; pobj = pobj->nxt) \
                ; \
            if (pobj == NULL) { \
                bug ("LIST_REMOVE: Couldn't find " #vtype " '" #obj \
                     "' in list '" #lst "'.", 0); \
                fail; \
            } \
            else \
                pobj->nxt = obj->nxt; \
        } \
        obj->nxt = NULL; \
    } while (0)

#define LIST_REMOVE_WITH_PREV(obj, pobj, nxt, lst) \
    do { \
        if (pobj) \
            pobj->nxt = obj->nxt; \
        else \
            lst = obj->nxt; \
        obj->nxt = NULL; \
    } while (0)

#define LISTB_FRONT(obj, nxt, f, b) \
    do { \
        obj->nxt = f; \
        f = obj; \
        if (obj->nxt == NULL) \
            b = obj; \
    } while (0)

#define LISTB_INSERT_AFTER(obj, after, nxt, f, b) \
    do { \
        if (after == NULL) { \
            obj->nxt = f; \
            f = obj; \
        } \
        else { \
            obj->nxt = after->nxt; \
            after->nxt = obj; \
        } \
        if (obj->nxt == NULL) \
            b = obj; \
    } while (0)

#define LISTB_BACK(obj, nxt, f, b) \
    do { \
        if (f == NULL) \
            f = obj; \
        else \
            b->nxt = obj; \
        obj->nxt = NULL; \
        b = obj; \
    } while (0)

#define LISTB_REMOVE(obj, nxt, f, b, vtype, fail) \
    do { \
        if (obj == f) { \
            f = obj->nxt; \
            if (obj == b) \
                b = NULL; \
        } \
        else { \
            vtype *pobj; \
            for (pobj = f; pobj && pobj->nxt != obj; pobj = pobj->nxt) \
                ; \
            if (pobj == NULL) { \
                bug ("LISTB_REMOVE: Couldn't find " #vtype " '" #obj \
                     "' in list '" #f "'.", 0); \
                fail; \
            } \
            else { \
                pobj->nxt = obj->nxt; \
                if (pobj->nxt == NULL) \
                    b = pobj; \
            } \
        } \
        obj->nxt = NULL; \
    } while (0)

#define LISTB_REMOVE_WITH_PREV(obj, pobj, nxt, f, b) \
    do { \
        if (pobj) \
            pobj->nxt = obj->nxt; \
        else \
            f = obj->nxt; \
        obj->nxt = NULL; \
        if (b == obj) \
            b = pobj; \
    } while (0)

#define LIST2_FRONT(obj, prv, nxt, f, b) \
    do { \
        obj->prv = NULL; \
        obj->nxt = f; \
        if (f == NULL) \
            b = obj; \
        else \
            f->prv = obj; \
        f = obj; \
    } while (0)

#define LIST2_BACK(obj, prv, nxt, f, b) \
    do { \
        obj->nxt = NULL; \
        obj->prv = b; \
        if (b == NULL) \
            f = obj; \
        else \
            b->nxt = obj; \
        b = obj; \
    } while (0)

#define LIST2_INSERT_AFTER(obj, after, prv, nxt, f, b) \
    do { \
        obj->prv = after; \
        obj->nxt = after ? after->nxt : f; \
        if (obj->prv) obj->prv->nxt = obj; else f = obj; \
        if (obj->nxt) obj->nxt->prv = obj; else b = obj; \
    } while (0)

#define LIST2_REMOVE(obj, prv, nxt, f, b) \
    do { \
        if (obj == f) f = f->nxt; \
        if (obj == b) b = b->prv; \
        if (obj->prv) obj->prv->nxt = obj->nxt; \
        if (obj->nxt) obj->nxt->prv = obj->prv; \
        obj->prv = NULL; \
        obj->nxt = NULL; \
    } while (0)

#define TOP(type) \
    (recycle_table[type].top)

#define REV_DIR(x) \
    (door_get(x)->reverse)

#define RETURN_IF(cond, msg, ch, rval) \
    do { \
        if (cond) { \
            if (ch && msg) \
                send_to_char (msg, ch); \
            return rval; \
        } \
    } while (0)

#define RETURN_IF_ACT(cond, msg, ch, arg1, arg2, rval) \
    do { \
        if (cond) { \
            if (ch && msg) \
                act_new (msg, ch, arg1, arg2, TO_CHAR, POS_DEAD); \
            return rval; \
        } \
    } while (0)

#define RETURN_IF_EXPR(cond, expr, rval) \
    do { \
        if (cond) { \
            (expr); \
            return rval; \
        } \
    } while (0)

#define FILTER(cond, msg, ch) \
    RETURN_IF(cond, msg, ch, TRUE)
#define FILTER_ACT(cond, msg, ch, arg1, arg2) \
    RETURN_IF_ACT(cond, msg, ch, arg1, arg2, TRUE)

#define BAIL_IF(cond, msg, ch) \
    RETURN_IF(cond, msg, ch, )
#define BAIL_IF_ACT(cond, msg, ch, arg1, arg2) \
    RETURN_IF_ACT(cond, msg, ch, arg1, arg2, )
#define BAIL_IF_EXPR(cond, expr) \
    RETURN_IF_EXPR(cond, expr, )

#define DO_REQUIRE_ARG(buf, msg) \
    do { \
        argument = one_argument (argument, buf); \
        BAIL_IF (buf[0] == '\0', msg, ch); \
    } while(0)

#endif
