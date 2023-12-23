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

#include "affects.h"
#include "board.h"
#include "chars.h"
#include "db.h"
#include "ext_flags.h"
#include "extra_descrs.h"
#include "globals.h"
#include "help.h"
#include "lookup.h"
#include "memory.h"
#include "mob_prog.h"
#include "mobiles.h"
#include "objs.h"
#include "objs.h"
#include "portals.h"
#include "resets.h"
#include "rooms.h"
#include "tables.h"
#include "utils.h"

#include <stdarg.h>
#include <string.h>
#include <stdlib.h>

/* Bundles of recycle functions. */
RECYCLE_BUNDLE (RECYCLE_BAN_T,         ban,         BAN_T);
RECYCLE_BUNDLE (RECYCLE_AREA_T,        area,        AREA_T);
RECYCLE_BUNDLE (RECYCLE_EXTRA_DESCR_T, extra_descr, EXTRA_DESCR_T);
RECYCLE_BUNDLE (RECYCLE_EXIT_T,        exit,        EXIT_T);
RECYCLE_BUNDLE (RECYCLE_ROOM_INDEX_T,  room_index,  ROOM_INDEX_T);
RECYCLE_BUNDLE (RECYCLE_OBJ_INDEX_T,   obj_index,   OBJ_INDEX_T);
RECYCLE_BUNDLE (RECYCLE_SHOP_T,        shop,        SHOP_T);
RECYCLE_BUNDLE (RECYCLE_MOB_INDEX_T,   mob_index,   MOB_INDEX_T);
RECYCLE_BUNDLE (RECYCLE_RESET_T,       reset_data,  RESET_T);
RECYCLE_BUNDLE (RECYCLE_HELP_T,        help,        HELP_T);
RECYCLE_BUNDLE (RECYCLE_MPROG_CODE_T,  mpcode,      MPROG_CODE_T);
RECYCLE_BUNDLE (RECYCLE_DESCRIPTOR_T,  descriptor,  DESCRIPTOR_T);
RECYCLE_BUNDLE (RECYCLE_GEN_T,         gen_data,    GEN_T);
RECYCLE_BUNDLE (RECYCLE_AFFECT_T,      affect,      AFFECT_T);
RECYCLE_BUNDLE (RECYCLE_OBJ_T,         obj,         OBJ_T);
RECYCLE_BUNDLE (RECYCLE_CHAR_T,        char,        CHAR_T);
RECYCLE_BUNDLE (RECYCLE_PC_T,          pcdata,      PC_T);
RECYCLE_BUNDLE (RECYCLE_MEM_T,         mem_data,    MEM_T);
RECYCLE_BUNDLE (RECYCLE_BUFFER_T,      buf,         BUFFER_T);
RECYCLE_BUNDLE (RECYCLE_MPROG_LIST_T,  mprog,       MPROG_LIST_T);
RECYCLE_BUNDLE (RECYCLE_HELP_AREA_T,   had,         HELP_AREA_T);
RECYCLE_BUNDLE (RECYCLE_NOTE_T,        note,        NOTE_T);
RECYCLE_BUNDLE (RECYCLE_SOCIAL_T,      social,      SOCIAL_T);
RECYCLE_BUNDLE (RECYCLE_PORTAL_EXIT_T, portal_exit, PORTAL_EXIT_T);
RECYCLE_BUNDLE (RECYCLE_PORTAL_T,      portal,      PORTAL_T);

void *recycle_new (int type) {
    RECYCLE_T *rec;
    OBJ_RECYCLE_T *data;
    void *obj;

    /* This function is cool enough to disregard const.
     * It's BAD, but... This can be fixed in a later refactoring.
     * -- Synival */
    RETURN_IF_BUG ((rec = (RECYCLE_T *) recycle_get (type)) == NULL,
        "new_recycle: Unknown type '%d'", type, NULL);

    /* If there's nothing to recycle, allocate a new object. */
    if (rec->free_first == NULL) {
        obj  = mem_alloc_perm (rec->size);
        data = APPLY_OFFSET (OBJ_RECYCLE_T, obj, rec->obj_data_off);
        rec->top++;
    }
    /* There's a recycled object - get it and pull it off the stack. */
    else {
        obj  = rec->free_first->obj;
        data = rec->free_first;
        LIST2_REMOVE (data, prev, next, rec->free_first, rec->free_last);
        rec->free_count--;
    }

    /* Initialize with empty data and an optional initializer function. */
    memset (obj, 0, rec->size);
    data->obj = obj;

    /* Push it to the back of our valid object list. */
    LIST2_BACK (data, prev, next, rec->list_first, rec->list_last);
    rec->list_count++;

    /* 'Validate' our object so we know it's in use. */
    data->valid = TRUE;

    if (rec->init_fun) {
        INIT_FUN *func = rec->init_fun;
        func (obj);
    }
    return obj;
}

void recycle_free (int type, void *obj) {
    RECYCLE_T *rec;
    OBJ_RECYCLE_T *data;

    if (obj == NULL)
        return;

    /* This function is cool enough to disregard const.
     * It's BAD, but... This can be fixed in a later refactoring.
     * -- Synival */
    BAIL_IF_BUG ((rec = (RECYCLE_T *) recycle_get (type)) == NULL,
        "recycle_free: Unknown type '%d'", type);

    /* Don't free objects that are already invalidated. */
    data = APPLY_OFFSET (OBJ_RECYCLE_T, obj, rec->obj_data_off);
    BAIL_IF_BUGF (!data->valid,
        "recycle_free: Attempted to free invalidated %s", rec->name);

    /* Use a disposal function if we have one. */
    if (rec->dispose_fun) {
        DISPOSE_FUN *func = rec->dispose_fun;
        func (obj);
    }

    /* 'Invalidate' our object so we know it's no longer in use. */
    if (data->obj == NULL) {
        bugf ("recycle_free: Warning - recycle data of entity with type '%s' "
            "lost its 'rec->obj' reference at some point! Was it accidentally "
            "zeroed-out somewhere?", rec->name);
        data->obj = obj;
    }
    data->valid = FALSE;

    /* Remove from our valid object list and add to the 'free' list. */
    LIST2_REMOVE (data, prev, next, rec->list_first, rec->list_last);
    rec->list_count--;

#ifdef BASEMUD_DEBUG_DISABLE_RECYCLE
    mem_free (data->obj, rec->size);
#else
    LIST2_BACK (data, prev, next, rec->free_first, rec->free_last);
    rec->free_count++;
#endif
}

int recycle_free_all (void) {
    int i, count = 0;

    /* Free some high-level objects first. */
    recycle_free_all_type (RECYCLE_AFFECT_T);
    recycle_free_all_type (RECYCLE_OBJ_T);
    recycle_free_all_type (RECYCLE_CHAR_T);
    recycle_free_all_type (RECYCLE_OBJ_INDEX_T);
    recycle_free_all_type (RECYCLE_MOB_INDEX_T);
    recycle_free_all_type (RECYCLE_ROOM_INDEX_T);
    recycle_free_all_type (RECYCLE_HELP_T);
    recycle_free_all_type (RECYCLE_HELP_AREA_T);
    recycle_free_all_type (RECYCLE_AREA_T);

    /* Free anything else we may have missed. */
    for (i = 0; i < RECYCLE_MAX; i++)
        count += recycle_free_all_type (i);

    return count;
}

int recycle_free_all_type (int type) {
    RECYCLE_T *rec;
    OBJ_RECYCLE_T *data;
    void *obj;
    int count = 0, free_count = 0;

    rec = &(recycle_table[type]);
    if ((rec->list_count == 0 || !rec->list_first) &&
        (rec->free_count == 0 || !rec->free_first))
        return 0;

    if (rec->list_count > 0 || rec->list_first) {
        log_f ("Freeing %d recyclable '%s(s)'...", rec->list_count, rec->name);
        while (rec->list_count > 0 || rec->list_first) {
            recycle_free (type, rec->list_first->obj);
            count++;
        }
    }
    while (rec->free_count > 0 || rec->free_first) {
        obj = rec->free_first->obj;
        data = APPLY_OFFSET (OBJ_RECYCLE_T, obj, rec->obj_data_off);
        LIST2_REMOVE (data, prev, next, rec->free_first, rec->free_last);
#ifdef BASEMUD_DEBUG_DISABLE_MEMORY_MANAGEMENT
        mem_free (obj, rec->size);
#endif
        rec->free_count--;
        free_count++;
    }

    return count;
}

void *recycle_get_first_obj (int type) {
    const RECYCLE_T *rec = recycle_get (type);
    return (rec->list_first) ? rec->list_first->obj : NULL;
}

DEFINE_INIT_FUN (ban_init) {
    BAN_T *ban = obj;
    ban->name = &str_empty[0];
}

DEFINE_DISPOSE_FUN (ban_dispose) {
    BAN_T *ban = obj;
    str_free (&(ban->name));
    LIST2_REMOVE (ban, global_prev, global_next,
        ban_first, ban_last);
}

DEFINE_INIT_FUN (descriptor_init) {
    DESCRIPTOR_T *d = obj;
    d->connected     = CON_GET_NAME;
    d->showstr_head  = NULL;
    d->showstr_point = NULL;
    d->outsize       = 2000;
    d->outbuf        = mem_alloc (d->outsize);
}

DEFINE_DISPOSE_FUN (descriptor_dispose) {
    DESCRIPTOR_T *d = obj;
    str_free (&(d->host));
    mem_free (d->outbuf, d->outsize);
    LIST2_REMOVE (d, global_prev, global_next,
        descriptor_first, descriptor_last);
}

DEFINE_INIT_FUN (extra_descr_init) {
    EXTRA_DESCR_T *ed = obj;
    ed->keyword = &str_empty[0];
    ed->description = &str_empty[0];
}

DEFINE_DISPOSE_FUN (extra_descr_dispose) {
    EXTRA_DESCR_T *ed = obj;
    str_free (&(ed->keyword));
    str_free (&(ed->description));
    extra_descr_unlink (ed);
}

DEFINE_DISPOSE_FUN (obj_dispose) {
    OBJ_T *o = obj;
    bool obj_in_world;

    /* Objects loaded into the playable world should use obj_extract(). */
    obj_in_world = obj_take (obj);
    if (obj_in_world && in_game_loop)
        bugf ("obj_free: %s is still in the real world.", o->short_descr);

    str_free (&(o->name));
    str_free (&(o->description));
    str_free (&(o->short_descr));
    str_free (&(o->owner));

    while (o->content_first)
        obj_free (o->content_first);
    while (o->extra_descr_first)
        extra_descr_free (o->extra_descr_first);
    while (o->affect_first)
        affect_free (o->affect_first);
    while (o->content_first)
        obj_free (o->content_first);

    obj_to_obj_index (o, NULL);
    LIST2_REMOVE (o, global_prev, global_next,
        object_first, object_last);
}

DEFINE_INIT_FUN (char_init) {
    CHAR_T *ch = obj;
    int i;

    ch->name        = &str_empty[0];
    ch->short_descr = &str_empty[0];
    ch->long_descr  = &str_empty[0];
    ch->description = &str_empty[0];
    ch->prompt      = &str_empty[0];
    ch->prefix      = &str_empty[0];
    ch->logon       = current_time;
    ch->lines       = PAGELEN;
    ch->position    = POS_STANDING;
    ch->hit         = 20;
    ch->max_hit     = 20;
    ch->mana        = 100;
    ch->max_mana    = 100;
    ch->move        = 100;
    ch->max_move    = 100;

    for (i = 0; i < 4; i++)
        ch->armor[i] = 100;
    for (i = 0; i < STAT_MAX; i++) {
        ch->perm_stat[i] = 13;
        ch->mod_stat[i] = 0;
    }
}

DEFINE_DISPOSE_FUN (char_dispose) {
    CHAR_T *ch = obj;
    CHAR_T *wch;

    /* Characters loaded into the playable world should use char_extract(). */
    if (ch->in_room != NULL) {
        char_from_room (ch);
        if (in_game_loop)
            bugf ("char_dispose: %s is still in the real world.", ch->short_descr);
    }

    for (wch = char_first; wch != NULL; wch = wch->global_next) {
        if (wch->reply == ch)
            wch->reply = NULL;
        if (ch->mprog_target == wch)
            wch->mprog_target = NULL;
    }

    if (ch->desc != NULL)
        ch->desc->character = NULL;

    mobile_to_mob_index (ch, NULL);
    if (IS_NPC (ch))
        mobile_count--;

    str_free (&(ch->name));
    str_free (&(ch->short_descr));
    str_free (&(ch->long_descr));
    str_free (&(ch->description));
    str_free (&(ch->prompt));
    str_free (&(ch->prefix));
/*  note_free (ch->pnote); */

    while (ch->affect_first)
        affect_free (ch->affect_first);
    while (ch->content_first)
        obj_free (ch->content_first);

#ifdef IMC
    imc_freechardata( ch );
#endif
    pcdata_free (ch->pcdata);

    LIST2_REMOVE (ch, global_prev, global_next,
        char_first, char_last);
}

DEFINE_INIT_FUN (pcdata_init) {
    PC_T *pcdata = obj;
    int alias;
    for (alias = 0; alias < MAX_ALIAS; alias++) {
        pcdata->alias[alias]     = NULL;
        pcdata->alias_sub[alias] = NULL;
    }
    pcdata->buffer = buf_new ();
}

DEFINE_DISPOSE_FUN (pcdata_dispose) {
    PC_T *pcdata = obj;
    int alias;

    str_free (&(pcdata->pwd));
    str_free (&(pcdata->bamfin));
    str_free (&(pcdata->bamfout));
    str_free (&(pcdata->title));
    buf_free (pcdata->buffer);

    for (alias = 0; alias < MAX_ALIAS; alias++) {
        str_free (&(pcdata->alias[alias]));
        str_free (&(pcdata->alias_sub[alias]));
    }
}

DEFINE_DISPOSE_FUN (buf_dispose) {
    BUFFER_T *buffer = obj;
    mem_free (buffer->string, buffer->size);
    buffer->string = NULL;
    buffer->size = 0;
    buffer->state = BUFFER_FREED;
}

DEFINE_INIT_FUN (mprog_init) {
    MPROG_LIST_T *mp = obj;
    mp->code = str_dup ("");
}

DEFINE_DISPOSE_FUN (mprog_dispose) {
    MPROG_LIST_T *mp = obj;
    str_free (&(mp->code));
    mprog_to_area (mp, NULL);
}

DEFINE_DISPOSE_FUN (had_dispose) {
    HELP_AREA_T *had = obj;

    str_free (&(had->filename));
    str_free (&(had->name));
    str_free (&(had->area_str));

    while (had->help_first)
        help_free (had->help_first);

    help_area_to_area (had, NULL);
    LIST2_REMOVE (had, global_prev, global_next,
        had_first, had_last);
}

DEFINE_DISPOSE_FUN (help_dispose) {
    HELP_T *help = obj;

    str_free (&(help->keyword));
    str_free (&(help->text));

    help_to_help_area (help, NULL);
    LIST2_REMOVE (help, global_prev, global_next,
        help_first, help_last);
}

DEFINE_INIT_FUN (reset_data_init) {
    RESET_T *reset = obj;
    reset->command = 'X';
}

DEFINE_DISPOSE_FUN (reset_data_dispose) {
    RESET_T *reset = obj;
    reset_to_area (reset, NULL);
    reset_to_room (reset, NULL);
}

DEFINE_INIT_FUN (area_init) {
    AREA_T *area = obj;
    char buf[MAX_INPUT_LENGTH];

    area->title      = str_dup ("New area");
/*  area->recall     = ROOM_VNUM_TEMPLE;      ROM OLC */
    area->area_flags = AREA_ADDED;
    area->security   = 1;
    area->builders   = str_dup ("None");
    area->vnum       = TOP(RECYCLE_AREA_T);

    sprintf (buf, "area%d", area->vnum);
    area->name = str_dup (buf);

    sprintf (buf, "area%d.are", area->vnum);
    area->filename = str_dup (buf);
}

DEFINE_DISPOSE_FUN (area_dispose) {
    AREA_T *area = obj;

    str_free (&(area->title));
    str_free (&(area->name));
    str_free (&(area->filename));
    str_free (&(area->builders));
    str_free (&(area->credits));

    while (area->had_first)
        had_free (area->had_first);
    while (area->room_first)
        room_index_free (area->room_first);
    while (area->mprog_first)
        mprog_free (area->mprog_first);
    while (area->mpcode_first)
        mpcode_free (area->mpcode_first);
    while (area->reset_first)
        reset_data_free (area->reset_first);
    while (area->mob_first)
        mob_index_free (area->mob_first);
    while (area->obj_first)
        obj_index_free (area->obj_first);

    LIST2_REMOVE (area, global_prev, global_next,
        area_first, area_last);
}

DEFINE_INIT_FUN (exit_init) {
    EXIT_T *exit = obj;
    exit->keyword      = &str_empty[0];
    exit->description  = &str_empty[0];
    exit->to_vnum      = -1;
    exit->to_area_vnum = -1;
    exit->to_anum      = -1;
    exit->key          = KEY_NOKEYHOLE;
}

DEFINE_DISPOSE_FUN (exit_dispose) {
    EXIT_T *exit = obj;

    str_free (&(exit->keyword));
    str_free (&(exit->description));

    if (exit->portal) {
        portal_exit_free (exit->portal);
        exit->portal = NULL;
    }

    exit_to_room_index_from (exit, NULL, DIR_NONE);
    exit_to_room_index_to (exit, NULL);
}

DEFINE_INIT_FUN (room_index_init) {
    ROOM_INDEX_T *room = obj;
    room->name        = &str_empty[0];
    room->description = &str_empty[0];
    room->owner       = &str_empty[0];
    room->heal_rate   = 100;
    room->mana_rate   = 100;
}

DEFINE_DISPOSE_FUN (room_index_dispose) {
    ROOM_INDEX_T *room = obj;
    int door;

    str_free (&(room->name));
    str_free (&(room->description));
    str_free (&(room->owner));
    str_free (&(room->area_str));

    for (door = 0; door < DIR_MAX; door++) {
        if (room->exit[door])
            exit_free (room->exit[door]);
        if (room->exit[door]) {
            bugf ("room_index_dispose: Room '%s' exit %d not "
                "properly unlinked.", room->name, door);
            room->exit[door] = NULL;
        }
    }

    if (room->portal) {
        portal_exit_free (room->portal);
        room->portal = NULL;
    }

    while (room->extra_descr_first)
        extra_descr_free (room->extra_descr_first);
    while (room->reset_first)
        reset_data_free (room->reset_first);
    while (room->content_first)
        obj_free (room->content_first);
    while (room->people_first)
        char_free (room->people_first);

    room_to_area (room, NULL);
    room_index_from_hash (room);
}

DEFINE_INIT_FUN (shop_init) {
    SHOP_T *shop = obj;
    shop->profit_buy  = 100;
    shop->profit_sell = 100;
    shop->open_hour   = 0;
    shop->close_hour  = 23;
}

DEFINE_DISPOSE_FUN (shop_dispose) {
    SHOP_T *shop = obj;
    LIST2_REMOVE (shop, global_prev, global_next,
        shop_first, shop_last);
}

DEFINE_INIT_FUN (obj_index_init) {
    OBJ_INDEX_T *o = obj;
    o->name        = str_dup ("no name");
    o->short_descr = str_dup ("(no short description)");
    o->description = str_dup ("(no description)");
    o->item_type   = ITEM_TRASH;
    o->condition   = 100;
    o->material    = MATERIAL_GENERIC;
    o->new_format  = TRUE;
}

DEFINE_DISPOSE_FUN (obj_index_dispose) {
    OBJ_INDEX_T *o = obj;

    while (o->obj_first)
        obj_free (o->obj_first);

    str_free (&(o->name));
    str_free (&(o->short_descr));
    str_free (&(o->description));
    str_free (&(o->area_str));

    while (o->extra_descr_first)
        extra_descr_free (o->extra_descr_first);
    while (o->affect_first)
        affect_free (o->affect_first);

    obj_index_to_area (o, NULL);
    obj_index_from_hash (o);
}

DEFINE_INIT_FUN (mob_index_init) {
    MOB_INDEX_T *mob = obj;
    mob->name         = str_dup ("no name");
    mob->short_descr  = str_dup ("(no short description)");
    mob->long_descr   = str_dup ("(no long description)\n\r");
    mob->description  = &str_empty[0];
    mob->ext_mob_plus = EXT_BITS (MOB_IS_NPC);
    mob->race         = race_lookup_exact ("human"); /* -- Hugin */
    mob->size         = SIZE_MEDIUM;  /* ROM patch -- Hugin */
    mob->start_pos    = POS_STANDING; /* -- Hugin */
    mob->default_pos  = POS_STANDING; /* -- Hugin */
    mob->sex          = SEX_EITHER;
    mob->material     = MATERIAL_GENERIC;
    mob->new_format   = TRUE;         /* ROM */
    db_finalize_mob (mob);
}

DEFINE_DISPOSE_FUN (mob_index_dispose) {
    MOB_INDEX_T *mob = obj;

    while (mob->mob_first)
        char_free (mob->mob_first);

    str_free (&(mob->name));
    str_free (&(mob->short_descr));
    str_free (&(mob->long_descr));
    str_free (&(mob->description));
    str_free (&(mob->area_str));

    while (mob->mprog_first)
        mprog_free (mob->mprog_first);
    if (mob->shop)
        shop_free (mob->shop);

    mob_index_to_area (mob, NULL);
    mob_index_from_hash (mob);
}

DEFINE_INIT_FUN (mpcode_init) {
    MPROG_CODE_T *mpcode = obj;
    mpcode->code = str_dup ("");
}

DEFINE_DISPOSE_FUN (mpcode_dispose) {
    MPROG_CODE_T *mpcode = obj;
    str_free (&(mpcode->code));

    mpcode_to_area (mpcode, NULL);
    LIST2_REMOVE (mpcode, global_prev, global_next,
        mpcode_first, mpcode_last);
}

DEFINE_INIT_FUN (buf_init) {
    BUFFER_T *buffer = obj;
    buffer->state = BUFFER_SAFE;

    buffer->size = buf_get_size (new_buf_size);
    EXIT_IF_BUG (buffer->size == -1,
        "new_buf: buffer size %d too large.", new_buf_size);

    buffer->string = mem_alloc (buffer->size);
    buffer->string[0] = '\0';
}

DEFINE_DISPOSE_FUN (note_dispose) {
    NOTE_T *note = obj;

    str_free (&(note->sender));
    str_free (&(note->to_list));
    str_free (&(note->subject));
    str_free (&(note->date));
    str_free (&(note->text));

    note_to_board (note, NULL);
}

DEFINE_INIT_FUN (social_init) {
    SOCIAL_T *social = obj;
    social->min_pos = POS_RESTING;
}

DEFINE_DISPOSE_FUN (social_dispose) {
    SOCIAL_T *soc = obj;
    str_free (&(soc->name));
    str_free (&(soc->char_no_arg));
    str_free (&(soc->others_no_arg));
    str_free (&(soc->char_found));
    str_free (&(soc->others_found));
    str_free (&(soc->vict_found));
    str_free (&(soc->char_not_found));
    str_free (&(soc->char_auto));
    str_free (&(soc->others_auto));
}

DEFINE_DISPOSE_FUN (portal_exit_dispose) {
    PORTAL_EXIT_T *pex = obj;

    portal_free_all_with_portal_exit (pex);
    portal_exit_to_exit (pex, NULL);

    str_free (&(pex->name));
}

DEFINE_DISPOSE_FUN (portal_dispose) {
    PORTAL_T *portal = obj;

    str_free (&(portal->name_from));
    str_free (&(portal->name_to));
}

DEFINE_DISPOSE_FUN (affect_dispose) {
    AFFECT_T *affect = obj;
    affect_unlink (affect);
}

static long last_pc_id  = 0;
long get_pc_id (void) {
    int val;
    val = (current_time <= last_pc_id) ? last_pc_id + 1 : current_time;
    last_pc_id = val;
    return val;
}

static long last_mob_id = 0;
long get_mob_id (void) {
    last_mob_id++;
    return last_mob_id;
}
