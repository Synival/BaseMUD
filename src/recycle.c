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

#include "utils.h"
#include "lookup.h"
#include "db.h"
#include "comm.h"
#include "affects.h"
#include "objs.h"

#include "recycle.h"

/* TODO: move defines to recycle.h */
/* TODO: move non-recycle functions to appropriate places. */

/* Globals */
#define RECYCLE_GLOBALS(type, name, vtype)   \
    vtype * name ## _get_first (void)        \
        { return recycle_get_first_obj (type); } \
    vtype * name ## _get_next (const vtype * obj) \
        { return (obj->rec_data.next) ? obj->rec_data.next->obj : NULL; } \
    vtype * name ## _new (void)              \
        { return recycle_new (type); }       \
    void name ## _free (vtype *v)            \
        { recycle_free (type, v); }

RECYCLE_GLOBALS (RECYCLE_BAN_DATA,         ban,         BAN_DATA);
RECYCLE_GLOBALS (RECYCLE_AREA_DATA,        area,        AREA_DATA);
RECYCLE_GLOBALS (RECYCLE_EXTRA_DESCR_DATA, extra_descr, EXTRA_DESCR_DATA);
RECYCLE_GLOBALS (RECYCLE_EXIT_DATA,        exit,        EXIT_DATA);
RECYCLE_GLOBALS (RECYCLE_ROOM_INDEX_DATA,  room_index,  ROOM_INDEX_DATA);
RECYCLE_GLOBALS (RECYCLE_OBJ_INDEX_DATA,   obj_index,   OBJ_INDEX_DATA);
RECYCLE_GLOBALS (RECYCLE_SHOP_DATA,        shop,        SHOP_DATA);
RECYCLE_GLOBALS (RECYCLE_MOB_INDEX_DATA,   mob_index,   MOB_INDEX_DATA);
RECYCLE_GLOBALS (RECYCLE_RESET_DATA,       reset_data,  RESET_DATA);
RECYCLE_GLOBALS (RECYCLE_HELP_DATA,        help,        HELP_DATA);
RECYCLE_GLOBALS (RECYCLE_MPROG_CODE,       mpcode,      MPROG_CODE);
RECYCLE_GLOBALS (RECYCLE_DESCRIPTOR_DATA,  descriptor,  DESCRIPTOR_DATA);
RECYCLE_GLOBALS (RECYCLE_GEN_DATA,         gen_data,    GEN_DATA);
RECYCLE_GLOBALS (RECYCLE_AFFECT_DATA,      affect,      AFFECT_DATA);
RECYCLE_GLOBALS (RECYCLE_OBJ_DATA,         obj,         OBJ_DATA);
RECYCLE_GLOBALS (RECYCLE_CHAR_DATA,        char,        CHAR_DATA);
RECYCLE_GLOBALS (RECYCLE_PC_DATA,          pcdata,      PC_DATA);
RECYCLE_GLOBALS (RECYCLE_MEM_DATA,         mem_data,    MEM_DATA);
RECYCLE_GLOBALS (RECYCLE_BUFFER,           buf,         BUFFER);
RECYCLE_GLOBALS (RECYCLE_MPROG_LIST,       mprog,       MPROG_LIST);
RECYCLE_GLOBALS (RECYCLE_HELP_AREA,        had,         HELP_AREA);
RECYCLE_GLOBALS (RECYCLE_NOTE_DATA,        note,        NOTE_DATA);
RECYCLE_GLOBALS (RECYCLE_SOCIAL_TYPE,      social,      SOCIAL_TYPE);
RECYCLE_GLOBALS (RECYCLE_PORTAL_EXIT_TYPE, portal_exit, PORTAL_EXIT_TYPE);
RECYCLE_GLOBALS (RECYCLE_PORTAL_TYPE,      portal,      PORTAL_TYPE);

int mobile_count = 0;
long last_pc_id  = 0;
long last_mob_id = 0;
size_t new_buf_current_size = BASE_BUF;

void *recycle_new (int type) {
    RECYCLE_TYPE *rec;
    OBJ_RECYCLE_DATA *data;
    void *obj;

    /* This function is cool enough to disregard const.
     * It's BAD, but... This can be fixed in a later refactoring.
     * -- Synival */
    if ((rec = (RECYCLE_TYPE *) recycle_get (type)) == NULL) {
        bugf("new_recycle: Unknown type '%d'", type);
        return NULL;
    }

    /* If there's nothing to recycle, allocate a new object. */
    if (rec->free_front == NULL) {
        obj  = alloc_perm (rec->size);
        data = APPLY_OFFSET (OBJ_RECYCLE_DATA, obj, rec->obj_data_off);
        rec->top++;
    }
    /* There's a recycled object - get it and pull it off the stack. */
    else {
        obj  = rec->free_front->obj;
        data = rec->free_front;
        LIST2_REMOVE (data, prev, next, rec->free_front, rec->free_back);
        rec->free_count--;
    }

    /* Initialize with empty data and an optional initializer function. */
    memset (obj, 0, rec->size);
    data->obj = obj;

    /* Push it to the back of our valid object list. */
    LIST2_BACK (data, prev, next, rec->list_front, rec->list_back);
    rec->list_count++;

    /* 'Validate' our object so we know it's in use. */
    data->valid = TRUE;

    if (rec->init_fun) {
        RECYCLE_INIT_FUN *func = rec->init_fun;
        func (obj);
    }
    return obj;
}

void recycle_free (int type, void *obj) {
    RECYCLE_TYPE *rec;
    OBJ_RECYCLE_DATA *data;

    if (obj == NULL)
        return;

    /* This function is cool enough to disregard const.
     * It's BAD, but... This can be fixed in a later refactoring.
     * -- Synival */
    if ((rec = (RECYCLE_TYPE *) recycle_get (type)) == NULL) {
        bugf("free_recycle: Unknown type '%d'", type);
        return;
    }

    /* Don't free objects that are already invalidated. */
    data = APPLY_OFFSET (OBJ_RECYCLE_DATA, obj, rec->obj_data_off);
    if (!data->valid) {
        bugf("free_recycle: Attempted to free invalidated %s\n", rec->name);
        return;
    }

    /* Use a disposal function if we have one. */
    if (rec->dispose_fun) {
        RECYCLE_DISPOSE_FUN *func = rec->dispose_fun;
        func (obj);
    }

    /* Remove from our valid object list and add to the 'free' list. */
    LIST2_REMOVE (data, prev, next, rec->list_front, rec->list_back);
    LIST2_BACK   (data, prev, next, rec->free_front, rec->free_back);
    rec->list_count--;
    rec->free_count++;

    /* 'Invalidate' our object so we know it's no longer in use. */
    data->valid = FALSE;
}

void *recycle_get_first_obj (int type) {
    const RECYCLE_TYPE *rec = recycle_get (type);
    return (rec->list_front) ? rec->list_front->obj : NULL;
}

void ban_init (void *obj) {
    BAN_DATA *ban = obj;
    ban->name = &str_empty[0];
}

void ban_dispose (void *obj) {
    BAN_DATA *ban = obj;
    str_free (ban->name);
}

void descriptor_init (void *obj) {
    DESCRIPTOR_DATA *d = obj;
    d->connected     = CON_GET_NAME;
    d->showstr_head  = NULL;
    d->showstr_point = NULL;
    d->outsize       = 2000;
    d->outbuf        = alloc_mem (d->outsize);
}

void descriptor_dispose (void *obj) {
    DESCRIPTOR_DATA *d = obj;
    str_free (d->host);
    mem_free (d->outbuf, d->outsize);
}

void extra_descr_init (void *obj) {
    EXTRA_DESCR_DATA *ed = obj;
    ed->keyword = &str_empty[0];
    ed->description = &str_empty[0];
}

void extra_descr_dispose (void *obj) {
    EXTRA_DESCR_DATA *ed = obj;
    str_free (ed->keyword);
    str_free (ed->description);
}

void obj_dispose (void *vobj) {
    OBJ_DATA *obj = vobj;
    AFFECT_DATA *paf, *paf_next;
    EXTRA_DESCR_DATA *ed, *ed_next;

    for (paf = obj->affected; paf != NULL; paf = paf_next) {
        paf_next = paf->next;
        affect_free (paf);
    }
    obj->affected = NULL;

    for (ed = obj->extra_descr; ed != NULL; ed = ed_next) {
        ed_next = ed->next;
        extra_descr_free (ed);
    }
    obj->extra_descr = NULL;

    str_free (obj->name);
    str_free (obj->description);
    str_free (obj->short_descr);
    str_free (obj->owner);
}

void char_init (void *obj) {
    CHAR_DATA *ch = obj;
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

void char_dispose (void *vobj) {
    CHAR_DATA *ch = vobj;
    OBJ_DATA *obj;
    OBJ_DATA *obj_next;
    AFFECT_DATA *paf;
    AFFECT_DATA *paf_next;

    if (IS_NPC (ch))
        mobile_count--;

    for (obj = ch->carrying; obj != NULL; obj = obj_next) {
        obj_next = obj->next_content;
        obj_extract (obj);
    }
    for (paf = ch->affected; paf != NULL; paf = paf_next) {
        paf_next = paf->next;
        affect_remove (ch, paf);
    }

    str_free (ch->name);
    str_free (ch->short_descr);
    str_free (ch->long_descr);
    str_free (ch->description);
    str_free (ch->prompt);
    str_free (ch->prefix);
/*  note_free (ch->pnote); */
#ifdef IMC
    imc_freechardata( ch );
#endif
    pcdata_free (ch->pcdata);
}

void pcdata_init (void *obj) {
    PC_DATA *pcdata = obj;
    int alias;
    for (alias = 0; alias < MAX_ALIAS; alias++) {
        pcdata->alias[alias] = NULL;
        pcdata->alias_sub[alias] = NULL;
    }
    pcdata->buffer = buf_new ();
}

void pcdata_dispose (void *obj) {
    PC_DATA *pcdata = obj;
    int alias;

    str_free (pcdata->pwd);
    str_free (pcdata->bamfin);
    str_free (pcdata->bamfout);
    str_free (pcdata->title);
    buf_free (pcdata->buffer);

    for (alias = 0; alias < MAX_ALIAS; alias++) {
        str_free (pcdata->alias[alias]);
        str_free (pcdata->alias_sub[alias]);
    }
}

void buf_dispose (void *obj) {
    BUFFER *buffer = obj;
    mem_free (buffer->string, buffer->size);
    buffer->string = NULL;
    buffer->size = 0;
    buffer->state = BUFFER_FREED;
}

void mprog_init (void *obj) {
    MPROG_LIST *mp = obj;
    mp->code = str_dup ("");
}

void mprog_dispose (void *obj) {
    MPROG_LIST *mp = obj;
    str_free (mp->code);
}

void had_dispose (void *obj) {
    HELP_AREA *had = obj;
    str_free (had->filename);
    str_free (had->name);
}

void help_dispose (void *obj) {
    HELP_DATA *help = obj;
    str_free (help->keyword);
    str_free (help->text);
}

void reset_data_init (void *obj) {
    RESET_DATA *reset = obj;
    reset->command = 'X';
}

void area_init (void *obj) {
    AREA_DATA *area = obj;
    char buf[MAX_INPUT_LENGTH];
    area->title      = str_dup ("New area");
/*  area->recall     = ROOM_VNUM_TEMPLE;      ROM OLC */
    area->area_flags = AREA_ADDED;
    area->security   = 1;
    area->builders   = str_dup ("None");
    area->empty      = TRUE;        /* ROM patch */
    sprintf (buf, "area%d.are", area->vnum);
    area->filename   = str_dup (buf);
    area->vnum       = TOP(RECYCLE_AREA_DATA) - 1;
}

void area_dispose (void *obj) {
    AREA_DATA *area = obj;
    str_free (area->title);
    str_free (area->name);
    str_free (area->filename);
    str_free (area->builders);
    str_free (area->credits);
}

void exit_init (void *obj) {
    EXIT_DATA *exit = obj;
    exit->keyword     = &str_empty[0];
    exit->description = &str_empty[0];
}

void exit_dispose (void *obj) {
    EXIT_DATA *exit = obj;
    str_free (exit->keyword);
    str_free (exit->description);
}

void room_index_init (void *obj) {
    ROOM_INDEX_DATA *room = obj;
    room->name        = &str_empty[0];
    room->description = &str_empty[0];
    room->owner       = &str_empty[0];
    room->heal_rate   = 100;
    room->mana_rate   = 100;
}

void room_index_dispose (void *obj) {
    ROOM_INDEX_DATA *room = obj;
    int door;
    EXTRA_DESCR_DATA *extra;
    RESET_DATA *reset;


    str_free (room->name);
    str_free (room->description);
    str_free (room->owner);

    for (door = 0; door < DIR_MAX; door++)
        if (room->exit[door])
            exit_free (room->exit[door]);
    for (extra = room->extra_descr; extra; extra = extra->next)
        extra_descr_free (extra);
    for (reset = room->reset_first; reset; reset = reset->next)
        reset_data_free (reset);
}

void shop_init (void *obj) {
    SHOP_DATA *shop = obj;
    shop->profit_buy = 100;
    shop->profit_sell = 100;
    shop->open_hour = 0;
    shop->close_hour = 23;
}

void obj_index_init (void *vobj) {
    OBJ_INDEX_DATA *obj = vobj;
    obj->name          = str_dup ("no name");
    obj->short_descr   = str_dup ("(no short description)");
    obj->description   = str_dup ("(no description)");
    obj->item_type     = ITEM_TRASH;
    obj->condition     = 100;
    obj->material_str  = str_dup (material_get_name(0));
    obj->new_format    = TRUE;
}

void obj_index_dispose (void *vobj) {
    OBJ_INDEX_DATA *obj = vobj;
    EXTRA_DESCR_DATA *extra, *extra_next;
    AFFECT_DATA *af, *af_next;

    str_free (obj->name);
    str_free (obj->short_descr);
    str_free (obj->description);
    str_free (obj->item_type_str);

    for (af = obj->affected; af; af = af_next) {
        af_next = af->next;
        affect_free (af);
    }
    obj->affected = NULL;

    for (extra = obj->extra_descr; extra; extra = extra_next) {
        extra_next = extra->next;
        extra_descr_free (extra);
    }
    obj->extra_descr = NULL;
}

void mob_index_init (void *obj) {
    MOB_INDEX_DATA *mob = obj;
    mob->name         = str_dup ("no name");
    mob->short_descr  = str_dup ("(no short description)");
    mob->long_descr   = str_dup ("(no long description)\n\r");
    mob->description  = &str_empty[0];
    mob->act          = ACT_IS_NPC;
    mob->race         = race_lookup ("human"); /* -- Hugin */
    mob->material_str = str_dup (material_get_name(0));
    mob->size         = SIZE_MEDIUM;           /* ROM patch -- Hugin */
    mob->start_pos    = POS_STANDING;          /* -- Hugin */
    mob->default_pos  = POS_STANDING;          /* -- Hugin */
    mob->new_format   = TRUE;                  /* ROM */
}

void mob_index_dispose (void *obj) {
    MOB_INDEX_DATA *mob = obj;
    str_free (mob->name);
    str_free (mob->short_descr);
    str_free (mob->long_descr);
    str_free (mob->description);
    str_free (mob->race_str);
    str_free (mob->dam_type_str);
    str_free (mob->size_str);
    str_free (mob->start_pos_str);
    str_free (mob->default_pos_str);
    str_free (mob->sex_str);
    mprog_free (mob->mprogs);
    shop_free (mob->pShop);
}

void mpcode_init (void *obj) {
    MPROG_CODE *mpcode = obj;
    mpcode->code = str_dup ("");
}

void mpcode_dispose (void *obj) {
    MPROG_CODE *mpcode = obj;
    str_free (mpcode->code);
}

void buf_init (void *obj) {
    BUFFER *buffer = obj;
    buffer->state = BUFFER_SAFE;
    buffer->size  = get_size (new_buf_current_size);
    if (buffer->size == -1) {
        bug ("new_buf: buffer size %d too large.", new_buf_current_size);
        exit (1);
    }
    buffer->string = alloc_mem (buffer->size);
    buffer->string[0] = '\0';
}

void note_dispose (void *obj) {
    NOTE_DATA *note = obj;
    str_free (note->sender);
    str_free (note->to_list);
    str_free (note->subject);
    str_free (note->date);
    str_free (note->text);
}

void social_dispose (void *obj) {
    SOCIAL_TYPE *soc = obj;
    str_free (soc->char_no_arg);
    str_free (soc->others_no_arg);
    str_free (soc->char_found);
    str_free (soc->others_found);
    str_free (soc->vict_found);
    str_free (soc->char_not_found);
    str_free (soc->char_auto);
    str_free (soc->others_auto);
}

void portal_exit_dispose (void *obj) {
    PORTAL_EXIT_TYPE *pex = obj;
    str_free (pex->name);
}

void portal_dispose (void *obj) {
    PORTAL_TYPE *portal = obj;
    if (portal->opposite)
        portal->opposite->opposite = NULL;
    str_free (portal->name_from);
    str_free (portal->name_to);
}

long get_pc_id (void) {
    int val;
    val = (current_time <= last_pc_id) ? last_pc_id + 1 : current_time;
    last_pc_id = val;
    return val;
}

long get_mob_id (void) {
    last_mob_id++;
    return last_mob_id;
}

/* buffer sizes */
const int buf_size[MAX_BUF_LIST] = {
    16, 32, 64, 128, 256, 1024, 2048, 4096, 8192, 16384
};

/* local procedure for finding the next acceptable size */
/* -1 indicates out-of-boundary error */
int get_size (int val) {
    int i;
    for (i = 0; i < MAX_BUF_LIST; i++)
        if (buf_size[i] >= val)
            return buf_size[i];
    return -1;
}

BUFFER *new_buf_size (int size) {
    BUFFER *buf;
    new_buf_current_size = size;
    buf = buf_new ();
    new_buf_current_size = BASE_BUF;
    return buf;
}

bool add_buf (BUFFER * buffer, char *string) {
    int len;
    char *oldstr;
    int oldsize;

    oldstr = buffer->string;
    oldsize = buffer->size;

    if (buffer->state == BUFFER_OVERFLOW)    /* don't waste time on bad strings! */
        return FALSE;

    len = strlen (buffer->string) + strlen (string) + 1;
    while (len >= buffer->size) { /* increase the buffer size */
        buffer->size = get_size (buffer->size + 1); {
            if (buffer->size == -1) { /* overflow */
                buffer->size = oldsize;
                buffer->state = BUFFER_OVERFLOW;
                bug ("buffer overflow past size %d", buffer->size);
                return FALSE;
            }
        }
    }

    if (buffer->size != oldsize) {
        buffer->string = alloc_mem (buffer->size);
        strcpy (buffer->string, oldstr);
        mem_free (oldstr, oldsize);
    }

    strcat (buffer->string, string);
    return TRUE;
}

void clear_buf (BUFFER * buffer) {
    buffer->string[0] = '\0';
    buffer->state = BUFFER_SAFE;
}

char *buf_string (BUFFER * buffer) {
    return buffer->string;
}
