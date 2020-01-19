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

#ifndef __ROM_RECYCLE_H
#define __ROM_RECYCLE_H

#include "merc.h"

/* Some handy macros. */
#define RECYCLE_BUNDLE(type, name, vtype)   \
    vtype * name ## _get_first (void)        \
        { return recycle_get_first_obj (type); } \
    vtype * name ## _get_next (const vtype * obj) \
        { return (obj->rec_data.next) ? obj->rec_data.next->obj : NULL; } \
    vtype * name ## _new (void)              \
        { return recycle_new (type); }       \
    void name ## _free (vtype *v)            \
        { recycle_free (type, v); }

#define DEC_RECYCLE_BUNDLE(name, vtype)           \
    vtype * name ## _new (void);                  \
    void name ## _free (vtype *);                 \
    vtype * name ## _get_first (void);            \
    vtype * name ## _get_next (const vtype * obj) \

/* Recycle globals and necessary functions. */
DEC_RECYCLE_BUNDLE (ban,         BAN_T);
DEC_RECYCLE_BUNDLE (area,        AREA_T);
DEC_RECYCLE_BUNDLE (extra_descr, EXTRA_DESCR_T);
DEC_RECYCLE_BUNDLE (exit,        EXIT_T);
DEC_RECYCLE_BUNDLE (room_index,  ROOM_INDEX_T);
DEC_RECYCLE_BUNDLE (obj_index,   OBJ_INDEX_T);
DEC_RECYCLE_BUNDLE (shop,        SHOP_T);
DEC_RECYCLE_BUNDLE (mob_index,   MOB_INDEX_T);
DEC_RECYCLE_BUNDLE (reset_data,  RESET_T);
DEC_RECYCLE_BUNDLE (help,        HELP_T);
DEC_RECYCLE_BUNDLE (mpcode,      MPROG_CODE_T);
DEC_RECYCLE_BUNDLE (descriptor,  DESCRIPTOR_T);
DEC_RECYCLE_BUNDLE (gen_data,    GEN_T);
DEC_RECYCLE_BUNDLE (affect,      AFFECT_T);
DEC_RECYCLE_BUNDLE (obj,         OBJ_T);
DEC_RECYCLE_BUNDLE (char,        CHAR_T);
DEC_RECYCLE_BUNDLE (pcdata,      PC_T);
DEC_RECYCLE_BUNDLE (mem_data,    MEM_T);
DEC_RECYCLE_BUNDLE (buf,         BUFFER_T);
DEC_RECYCLE_BUNDLE (mprog,       MPROG_LIST_T);
DEC_RECYCLE_BUNDLE (had,         HELP_AREA_T);
DEC_RECYCLE_BUNDLE (note,        NOTE_T);
DEC_RECYCLE_BUNDLE (social,      SOCIAL_T);
DEC_RECYCLE_BUNDLE (portal_exit, PORTAL_EXIT_T);
DEC_RECYCLE_BUNDLE (portal,      PORTAL_T);

/* Function prototypes (recycle operations). */
void *recycle_new (int type);
void recycle_free (int type, void *obj);
int recycle_free_all (void);
int recycle_free_all_type (int type);
void *recycle_get_first_obj (int type);

/* Initialization / disposal functions. */
void ban_init (void *obj);
void ban_dispose (void *obj);
void descriptor_init (void *obj);
void descriptor_dispose (void *obj);
void extra_descr_init (void *obj);
void extra_descr_dispose (void *obj);
void obj_dispose (void *vobj);
void char_init (void *obj);
void char_dispose (void *vobj);
void pcdata_init (void *obj);
void pcdata_dispose (void *obj);
void buf_dispose (void *obj);
void mprog_init (void *obj);
void mprog_dispose (void *obj);
void had_dispose (void *obj);
void help_dispose (void *obj);
void reset_data_init (void *obj);
void area_init (void *obj);
void area_dispose (void *obj);
void exit_init (void *obj);
void exit_dispose (void *obj);
void room_index_init (void *obj);
void room_index_dispose (void *obj);
void shop_init (void *obj);
void obj_index_init (void *vobj);
void obj_index_dispose (void *vobj);
void mob_index_init (void *obj);
void mob_index_dispose (void *obj);
void mpcode_init (void *obj);
void mpcode_dispose (void *obj);
void buf_init (void *obj);
void note_dispose (void *obj);
void social_init (void *obj);
void social_dispose (void *obj);
void portal_exit_dispose (void *obj);
void portal_dispose (void *obj);

/* Functions related to specific recycleable objects. */
long get_pc_id (void);
long get_mob_id (void);

#endif
