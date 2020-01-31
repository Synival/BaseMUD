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

#ifndef __ROM_MACROS_H
#define __ROM_MACROS_H

/* Utility macros. */
#define IS_VALID(data)       ((data) != NULL && (data)->rec_data.valid)
#define UMIN(a, b)           ((a) < (b) ? (a) : (b))
#define UMAX(a, b)           ((a) > (b) ? (a) : (b))
#define URANGE(a, b, c)      ((b) < (a) ? (a) : ((b) > (c) ? (c) : (b)))
#define LOWER(c)             ((c) >= 'A' && (c) <= 'Z' ? (c)+'a'-'A' : (c))
#define UPPER(c)             ((c) >= 'a' && (c) <= 'z' ? (c)+'A'-'a' : (c))
#define IS_NULLSTR(str)      ((str) == NULL || (str)[0] == '\0')
#define ENTRE(min, num, max) (((min) < (num)) && ((num) < (max)))

/* Bit macros. */
#define IS_SET(_var, _bit)       ((_var) & (_bit))
#define SET_BIT(_var, _bit)      ((_var) |= (_bit))
#define REMOVE_BIT(_var, _bit)   ((_var) &= ~(_bit))
#define TOGGLE_BIT(_var, _bit)   ((_var) ^= (_bit))

#define ARE_SET(_var, _bit)      (((_var) & (_bit)) == (_bit))
#define NONE_SET(_var, _bit)     (((_var) & (_bit)) == 0)

/* Extended bit macros. */
#define EXT_ZERO                     ((EXT_FLAGS_T) {{ 0 }})
#define EXT_IS_ZERO(_bits)           (ext_flags_is_zero(_bits))
#define EXT_IS_NONZERO(_bits)        (!EXT_IS_ZERO(_bits))
#define EXT_BITS(...)                (ext_flags_build(__VA_ARGS__, -1))
#define EXT_IS_SET(_var, _flag)      (ext_flags_is_set     ((_var), (_flag)))
#define EXT_EQUALS(_bits1, _bits2)   (ext_flags_equals     ((_bits1), (_bits2)))
#define EXT_SET(_var, _flag)         (ext_flags_set        (&(_var), (_flag)))
#define EXT_SET_MANY(_var, _var2)    (ext_flags_set_many   (&(_var), (_var2)))
#define EXT_UNSET(_var, _flag)       (ext_flags_unset      (&(_var), (_flag)))
#define EXT_UNSET_MANY(_var, _var2)  (ext_flags_unset_many (&(_var), (_var2)))
#define EXT_TOGGLE(_var, _flag)      (ext_flags_toggle     (&(_var), (_flag)))
#define EXT_TOGGLE_MANY(_var, _var2) (ext_flags_toggle_many(&(_var), (_var2)))

#define EXT_TO_FLAG_T(_bits)         (ext_flags_to_flag_t(_bits))

#define EXT_INIT_ZERO                ((EXT_INIT_FLAGS_T) {(int[]) { -1 }})
#define EXT_INIT_BITS(...)           ((EXT_INIT_FLAGS_T) {(int[]) { __VA_ARGS__, -1 }})
#define EXT_FROM_FLAG_T(_flags)      (ext_flags_from_flag_t(_flags))
#define EXT_FROM_INIT(_bits)         (ext_flags_from_init(&(_bits)))

#define EXT_WITH(_bits, _flag)           (ext_flags_with (_bits, _flag))
#define EXT_WITHOUT(_bits, _flag)        (ext_flags_without (_bits, _flag))
#define EXT_WITH_MANY(_bits1, _bits2)    (ext_flags_with_many (_bits1, _bits2))
#define EXT_WITHOUT_MANY(_bits1, _bits2) (ext_flags_without_many (_bits1, _bits2))

/* Skill macros. */
#define SN(map)      (skill_map_table[SKILL_MAP_ ## map].skill_index)

/* Alias for "new" act function. */
#define act(format, ch, arg1, arg2, flags) \
    act_new((format), (ch), (arg1), (arg2), (flags), POS_RESTING)

/* Mob program macros. */
#define HAS_TRIGGER(ch, trig) (IS_SET((ch)->index_data->mprog_flags,(trig)))
#define HAS_ANY_TRIGGER(ch)   ((ch)->index_data->mprog_flags != 0)

/* Read in a char. */
#if defined(KEY)
    #undef KEY
#endif
#define KEY(literal, field, value) \
    if (!str_cmp(word, literal)) { \
        field = value;             \
        match = TRUE;              \
        break;                     \
    }

/* provided to free strings */
#if defined(KEYS)
    #undef KEYS
#endif
#define KEYS(literal, field, value) \
    if (!str_cmp(word, literal)) {  \
        str_free (&(field));        \
        field = value;              \
        match = TRUE;               \
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
            if ((ch) && (msg)) { \
                send_to_char ((msg), (ch)); \
            } \
            return rval; \
        } \
    } while (0)

#define RETURN_IF_BUG(cond, msg, param, rval) \
    do { \
        if (cond) { \
            if (msg) { \
                bug ((msg), (param)); \
            } \
            return rval; \
        } \
    } while (0)

#define RETURN_IF_BUGF(cond, rval, ...) \
    do { \
        if (cond) { \
            bugf (__VA_ARGS__); \
            return rval; \
        } \
    } while (0)

#define RETURN_IF_ACT(cond, msg, ch, arg1, arg2, rval) \
    do { \
        if (cond) { \
            if ((ch) && (msg)) { \
                act_new ((msg), (ch), (arg1), (arg2), TO_CHAR, POS_DEAD); \
            } \
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

#define EXIT_IF_BUG(cond, msg, param) \
    do { \
        if (cond) { \
            if (msg) { \
                bug ((msg), (param)); \
            } \
            exit (1); \
        } \
    } while (0)

#define EXIT_IF_BUGF(cond, ...) \
    do { \
        if (cond) { \
            bugf (__VA_ARGS__); \
            exit (1); \
        } \
    } while (0)

#define FILTER(cond, msg, ch) \
    RETURN_IF(cond, msg, ch, TRUE)
#define FILTER_ACT(cond, msg, ch, arg1, arg2) \
    RETURN_IF_ACT(cond, msg, ch, arg1, arg2, TRUE)

#define BAIL_IF(cond, msg, ch) \
    RETURN_IF(cond, msg, ch, )

#define BAIL_IF_BUG(cond, msg, param) \
    RETURN_IF_BUG(cond, msg, param, )

#define BAIL_IF_BUGF(cond, ...) \
    RETURN_IF_BUGF(cond, , __VA_ARGS__)

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
