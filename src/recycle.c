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

#include <stdarg.h>
#include <string.h>
#include <stdlib.h>

#include "utils.h"
#include "lookup.h"
#include "db.h"
#include "affects.h"
#include "objs.h"
#include "chars.h"
#include "globals.h"
#include "memory.h"

#include "recycle.h"

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
    if (rec->free_front == NULL) {
        obj  = mem_alloc_perm (rec->size);
        data = APPLY_OFFSET (OBJ_RECYCLE_T, obj, rec->obj_data_off);
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
    RECYCLE_T *rec;
    OBJ_RECYCLE_T *data;

    if (obj == NULL)
        return;

    /* This function is cool enough to disregard const.
     * It's BAD, but... This can be fixed in a later refactoring.
     * -- Synival */
    BAIL_IF_BUG  ((rec = (RECYCLE_T *) recycle_get (type)) == NULL,
        "free_recycle: Unknown type '%d'", type);

    /* Don't free objects that are already invalidated. */
    data = APPLY_OFFSET (OBJ_RECYCLE_T, obj, rec->obj_data_off);
    BAIL_IF_BUGF (!data->valid,
        "free_recycle: Attempted to free invalidated %s", rec->name);

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

int recycle_free_all (void) {
    int i, count = 0;

    /* Free some high-level objects first. */
    recycle_free_all_type (RECYCLE_AREA_T);
    recycle_free_all_type (RECYCLE_MOB_INDEX_T);
    recycle_free_all_type (RECYCLE_CHAR_T);
    recycle_free_all_type (RECYCLE_OBJ_T);
    recycle_free_all_type (RECYCLE_HELP_AREA_T);
    recycle_free_all_type (RECYCLE_HELP_T);
    recycle_free_all_type (RECYCLE_ROOM_INDEX_T);
    recycle_free_all_type (RECYCLE_OBJ_INDEX_T);

    /* Free anything else we may have missed. */
    for (i = 0; i < RECYCLE_MAX; i++)
        count += recycle_free_all_type (i);

    return count;
}

int recycle_free_all_type (int type) {
    RECYCLE_T *rec;
    int count = 0;

    rec = &(recycle_table[type]);
    if (rec->list_count == 0 && !rec->list_front)
        return 0;

    log_f ("Freeing %d recyclable '%s(s)'...", rec->list_count, rec->name);
    while (rec->list_count > 0 || rec->list_front) {
        recycle_free (type, rec->list_front->obj);
        count++;
    }

    return count;
}

void *recycle_get_first_obj (int type) {
    const RECYCLE_T *rec = recycle_get (type);
    return (rec->list_front) ? rec->list_front->obj : NULL;
}

void ban_init (void *obj) {
    BAN_T *ban = obj;
    ban->name = &str_empty[0];
}

void ban_dispose (void *obj) {
    BAN_T *ban = obj;
    str_free (&(ban->name));
}

void descriptor_init (void *obj) {
    DESCRIPTOR_T *d = obj;
    d->connected     = CON_GET_NAME;
    d->showstr_head  = NULL;
    d->showstr_point = NULL;
    d->outsize       = 2000;
    d->outbuf        = mem_alloc (d->outsize);
}

void descriptor_dispose (void *obj) {
    DESCRIPTOR_T *d = obj;
    str_free (&(d->host));
    mem_free (d->outbuf, d->outsize);
}

void extra_descr_init (void *obj) {
    EXTRA_DESCR_T *ed = obj;
    ed->keyword = &str_empty[0];
    ed->description = &str_empty[0];
}

void extra_descr_dispose (void *obj) {
    EXTRA_DESCR_T *ed = obj;
    str_free (&(ed->keyword));
    str_free (&(ed->description));
}

void obj_dispose (void *vobj) {
    OBJ_T *obj = vobj;
    AFFECT_T *paf, *paf_next;
    EXTRA_DESCR_T *ed, *ed_next;

    for (ed = obj->extra_descr; ed != NULL; ed = ed_next) {
        ed_next = ed->next;
        extra_descr_free (ed);
    }
    obj->extra_descr = NULL;

    for (paf = obj->affected; paf != NULL; paf = paf_next) {
        paf_next = paf->next;
        affect_free (paf);
    }
    obj->affected = NULL;

    str_free (&(obj->name));
    str_free (&(obj->description));
    str_free (&(obj->short_descr));
    str_free (&(obj->owner));
}

void char_init (void *obj) {
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

void char_dispose (void *vobj) {
    CHAR_T *ch = vobj;
    OBJ_T *obj;
    OBJ_T *obj_next;
    AFFECT_T *paf;
    AFFECT_T *paf_next;

    if (IS_NPC (ch))
        mobile_count--;

    for (obj = ch->carrying; obj != NULL; obj = obj_next) {
        obj_next = obj->next_content;
        obj_extract (obj);
    }
    ch->carrying = NULL;

    for (paf = ch->affected; paf != NULL; paf = paf_next) {
        paf_next = paf->next;
        affect_remove (ch, paf);
    }
    ch->affected = NULL;

    str_free (&(ch->name));
    str_free (&(ch->short_descr));
    str_free (&(ch->long_descr));
    str_free (&(ch->description));
    str_free (&(ch->prompt));
    str_free (&(ch->prefix));
/*  note_free (ch->pnote); */

#ifdef IMC
    imc_freechardata( ch );
#endif
    pcdata_free (ch->pcdata);
}

void pcdata_init (void *obj) {
    PC_T *pcdata = obj;
    int alias;
    for (alias = 0; alias < MAX_ALIAS; alias++) {
        pcdata->alias[alias] = NULL;
        pcdata->alias_sub[alias] = NULL;
    }
    pcdata->buffer = buf_new ();
}

void pcdata_dispose (void *obj) {
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

void buf_dispose (void *obj) {
    BUFFER_T *buffer = obj;
    mem_free (buffer->string, buffer->size);
    buffer->string = NULL;
    buffer->size = 0;
    buffer->state = BUFFER_FREED;
}

void mprog_init (void *obj) {
    MPROG_LIST_T *mp = obj;
    mp->code = str_dup ("");
}

void mprog_dispose (void *obj) {
    MPROG_LIST_T *mp = obj;
    str_free (&(mp->code));
}

void had_dispose (void *obj) {
    HELP_AREA_T *had = obj;
    HELP_T *help, *help_next;

    str_free (&(had->filename));
    str_free (&(had->name));
    str_free (&(had->area_str));

    for (help = had->first; help != NULL; help = help_next) {
        help_next = help->next_area;
        help_free (help);
    }
    had->first = NULL;
    had->next = NULL;
}

void help_dispose (void *obj) {
    HELP_T *help = obj;
    str_free (&(help->keyword));
    str_free (&(help->text));
}

void reset_data_init (void *obj) {
    RESET_T *reset = obj;
    reset->command = 'X';
}

void area_init (void *obj) {
    AREA_T *area = obj;
    char buf[MAX_INPUT_LENGTH];
    area->title      = str_dup ("New area");
/*  area->recall     = ROOM_VNUM_TEMPLE;      ROM OLC */
    area->area_flags = AREA_ADDED;
    area->security   = 1;
    area->builders   = str_dup ("None");
    area->empty      = TRUE;        /* ROM patch */
    sprintf (buf, "area%d.are", area->vnum);
    area->filename   = str_dup (buf);
    area->vnum       = TOP(RECYCLE_AREA_T);
}

void area_dispose (void *obj) {
    AREA_T *area = obj;
    str_free (&(area->title));
    str_free (&(area->name));
    str_free (&(area->filename));
    str_free (&(area->builders));
    str_free (&(area->credits));
}

void exit_init (void *obj) {
    EXIT_T *exit = obj;
    exit->keyword     = &str_empty[0];
    exit->description = &str_empty[0];
    exit->vnum        = -1;
    exit->area_vnum   = -1;
    exit->key         = KEY_NOKEYHOLE;
}

void exit_dispose (void *obj) {
    EXIT_T *exit = obj;
    str_free (&(exit->keyword));
    str_free (&(exit->description));
}

void room_index_init (void *obj) {
    ROOM_INDEX_T *room = obj;
    room->name        = &str_empty[0];
    room->description = &str_empty[0];
    room->owner       = &str_empty[0];
    room->heal_rate   = 100;
    room->mana_rate   = 100;
}

void room_index_dispose (void *obj) {
    ROOM_INDEX_T *room = obj;
    int door;
    EXTRA_DESCR_T *extra, *extra_next;
    RESET_T *reset, *reset_next;

    str_free (&(room->name));
    str_free (&(room->description));
    str_free (&(room->owner));
    str_free (&(room->area_str));

    for (door = 0; door < DIR_MAX; door++) {
        if (room->exit[door])
            exit_free (room->exit[door]);
        room->exit[door] = NULL;
    }

    for (extra = room->extra_descr; extra; extra = extra_next) {
        extra_next = extra->next;
        extra_descr_free (extra);
    }
    room->extra_descr = NULL;

    for (reset = room->reset_first; reset; reset = reset_next) {
        reset_next = reset->next;
        reset_data_free (reset);
    }
    room->reset_first = NULL;
    room->reset_last = NULL;
}

void shop_init (void *obj) {
    SHOP_T *shop = obj;
    shop->profit_buy  = 100;
    shop->profit_sell = 100;
    shop->open_hour   = 0;
    shop->close_hour  = 23;
}

void obj_index_init (void *vobj) {
    OBJ_INDEX_T *obj = vobj;
    obj->name        = str_dup ("no name");
    obj->short_descr = str_dup ("(no short description)");
    obj->description = str_dup ("(no description)");
    obj->item_type   = ITEM_TRASH;
    obj->condition   = 100;
    obj->material    = MATERIAL_GENERIC;
    obj->new_format  = TRUE;
}

void obj_index_dispose (void *vobj) {
    OBJ_INDEX_T *obj = vobj;
    EXTRA_DESCR_T *extra, *extra_next;
    AFFECT_T *af, *af_next;

    str_free (&(obj->name));
    str_free (&(obj->short_descr));
    str_free (&(obj->description));
    str_free (&(obj->area_str));

    for (extra = obj->extra_descr; extra; extra = extra_next) {
        extra_next = extra->next;
        extra_descr_free (extra);
    }
    obj->extra_descr = NULL;

    for (af = obj->affected; af; af = af_next) {
        af_next = af->next;
        affect_free (af);
    }
    obj->affected = NULL;
}

void mob_index_init (void *obj) {
    MOB_INDEX_T *mob = obj;
    mob->name         = str_dup ("no name");
    mob->short_descr  = str_dup ("(no short description)");
    mob->long_descr   = str_dup ("(no long description)\n\r");
    mob->description  = &str_empty[0];
    mob->mob_plus     = MOB_IS_NPC;
    mob->race         = race_lookup_exact ("human"); /* -- Hugin */
    mob->size         = SIZE_MEDIUM;  /* ROM patch -- Hugin */
    mob->start_pos    = POS_STANDING; /* -- Hugin */
    mob->default_pos  = POS_STANDING; /* -- Hugin */
    mob->sex          = SEX_EITHER;
    mob->material     = MATERIAL_GENERIC;
    mob->new_format   = TRUE;         /* ROM */
    db_finalize_mob (mob);
}

void mob_index_dispose (void *obj) {
    MOB_INDEX_T *mob = obj;
    str_free (&(mob->name));
    str_free (&(mob->short_descr));
    str_free (&(mob->long_descr));
    str_free (&(mob->description));
    str_free (&(mob->area_str));
    mprog_free (mob->mprogs);
    shop_free (mob->shop);
}

void mpcode_init (void *obj) {
    MPROG_CODE_T *mpcode = obj;
    mpcode->code = str_dup ("");
}

void mpcode_dispose (void *obj) {
    MPROG_CODE_T *mpcode = obj;
    str_free (&(mpcode->code));
}

static size_t new_buf_current_size = BASE_BUF;
void buf_init (void *obj) {
    BUFFER_T *buffer = obj;
    buffer->state = BUFFER_SAFE;

    buffer->size  = get_size (new_buf_current_size);
    EXIT_IF_BUG (buffer->size == -1,
        "new_buf: buffer size %d too large.", new_buf_current_size);

    buffer->string = mem_alloc (buffer->size);
    buffer->string[0] = '\0';
}

void note_dispose (void *obj) {
    NOTE_T *note = obj;
    str_free (&(note->sender));
    str_free (&(note->to_list));
    str_free (&(note->subject));
    str_free (&(note->date));
    str_free (&(note->text));
}

void social_init (void *obj) {
    SOCIAL_T *social = obj;
    social->min_pos = POS_RESTING;
}

void social_dispose (void *obj) {
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

void portal_exit_dispose (void *obj) {
    PORTAL_EXIT_T *pex = obj;
    str_free (&(pex->name));
}

void portal_dispose (void *obj) {
    PORTAL_T *portal = obj;
    str_free (&(portal->name_from));
    str_free (&(portal->name_to));
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

/* buffer sizes */
static const int buf_size[MAX_BUF_LIST] = {
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

BUFFER_T *new_buf_size (int size) {
    BUFFER_T *buf;
    new_buf_current_size = size;
    buf = buf_new ();
    new_buf_current_size = BASE_BUF;
    return buf;
}

bool add_buf (BUFFER_T *buffer, char *string) {
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
        buffer->string = mem_alloc (buffer->size);
        strcpy (buffer->string, oldstr);
        mem_free (oldstr, oldsize);
    }

    strcat (buffer->string, string);
    return TRUE;
}

void clear_buf (BUFFER_T *buffer) {
    buffer->string[0] = '\0';
    buffer->state = BUFFER_SAFE;
}

char *buf_string (BUFFER_T *buffer) {
    return buffer->string;
}

void printf_to_buf (BUFFER_T *buffer, const char *fmt, ...) {
    char buf[MAX_STRING_LENGTH];
    va_list args;
    va_start (args, fmt);
    vsnprintf (buf, sizeof(buf), fmt, args);
    va_end (args);

    add_buf (buffer, buf);
}
