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

#include <string.h>
#include <stdlib.h>

#include "utils.h"
#include "globals.h"

#include "memory.h"

/* Magic number for memory allocation */
#define MAGIC_NUM 52571214
typedef int magic_t;

/* Globals for strings. */
static int strings_allocated;
static int string_bytes_allocated;
static char *string_space;
static char *top_string;
static char *string_hash[MAX_KEY_HASH];

/* Globals for memory pages. */
static int mem_blocks_allocated;
static int mem_block_bytes_allocated;
static void *mem_blocks_free[MAX_MEM_LIST];
static const magic_t mem_block_sizes[MAX_MEM_LIST] = {
    16, 32, 64, 128, 256, 1024, 2048, 4096, 8192, 16384, 32768 - 64
};

/* Keep track of allocated memory blocks so Valgrind
 * doesn't report lost memory. -- Synival */
static char *mem_pages[MAX_PERM_BLOCKS];
static int mem_pages_allocated = 0;

void string_space_init (void) {
    EXIT_IF_BUG ((string_space = calloc (1, MAX_STRING_SPACE)) == NULL,
        "init_string_space: can't alloc %d string space. Please decrease "
        "MAX_STRING_SPACE.", MAX_STRING_SPACE);
    top_string = string_space;
}

int string_space_dispose (void) {
    if (string_space == NULL)
        return 0;
    free (string_space);
    string_space = NULL;
    top_string = NULL;
    return MAX_STRING_SPACE;
}

char *string_space_next (void) {
    char *str = top_string + sizeof (char *);
    EXIT_IF_BUG (str > &string_space[MAX_STRING_SPACE - MAX_STRING_LENGTH],
        "string_space_next: MAX_STRING_SPACE %d exceeded. Please increase.",
        MAX_STRING_SPACE);
    return str;
}

/* Frees a potentially already-allocated string and
 * replaces it with a newly created on. */
void str_replace_dup (char **old, const char *str) {
    str_free (old);
    *old = str_dup (str);
}

/* Duplicate a string into dynamic memory (unless booting).
 * Fread_strings are read-only and shared. */
char *str_dup (const char *str) {
    char *str_new;

    if (str == NULL)
        return NULL;
    if (str[0] == '\0')
        return str_empty;
    if (str >= string_space && str < top_string)
        return (char *) str;
    if (in_boot_db)
        return str_register (str, strlen (str));

    str_new = mem_alloc (strlen (str) + 1);
    strcpy (str_new, str);
    return str_new;
}

/* Free a string and set its poitner to NULL.
 * Null is legal here to simplify callers.
 * Read-only shared strings are not touched. */
void str_free (char **pstr) {
    if (pstr == NULL || *pstr == NULL)
        return;
    if (*pstr == str_empty) {
        *pstr = NULL;
        return;
    }
    if (*pstr >= string_space && *pstr < top_string) {
        *pstr = NULL;
        return;
    }
    mem_free (*pstr, strlen (*pstr) + 1);
    *pstr = NULL;
}

/* Add a string to the global hash of read-only, permanent strings. */
char *str_register (const char *str, int len) {
    union {
        char *pc;
        char rgc[sizeof (char *)];
    } u1;

    int i, hash_index;
    char *hash, *hash_str, *next_hash;
    char *new = top_string, *new_str = top_string + sizeof (char *);

    /* Ignore special cases. */
    if (str == NULL)
        return NULL;
    if (*str == '\0')
        return str_empty;

    /* Check to see if this exact string is already loaded.
     * Each string is hashed by its length. hash_index[n] points to the
     * latest string loaded of the same size. Each string is stored in
     * 'string_space' with the following format:
     * ----------------------------------------------------------------
     *      (char *)                 (char x len) '\0'
     *      ^^ pointer to next hash, ^^ data
     * ----------------------------------------------------------------
     * Each hash is traversed back to the first string loaded similar to
     * traversing through a linked list. */

    hash_index = UMIN (MAX_KEY_HASH - 1, len);
    for (hash = string_hash[hash_index]; hash; hash = next_hash) {
        for (i = 0; i < sizeof (char *); i++)
            u1.rgc[i] = hash[i];
        next_hash = u1.pc;

        hash_str = hash + sizeof (char *);
        if (str[0] == hash_str[0] && !strcmp (str + 1, hash_str + 1))
            return hash + sizeof (char *);
    }

    /* No matching string was found - so register it into 'string_space'.
       If this string isn't already loaded into 'string_space', do it now. */
    if (str != new_str) {
        strcpy (new_str, str);
        str = new_str;
    }

    strings_allocated += 1;
    top_string   += len + 1 + sizeof (char *);
    string_bytes_allocated += len + 1 + sizeof (char *);

    u1.pc = string_hash[hash_index];
    for (i = 0; i < sizeof (char *); i++)
        new[i] = u1.rgc[i];
    string_hash[hash_index] = new;

    /* Return the string in its 'string_space' location, without the pointer
     * to the next hashed string. */
    return new_str;
}

/* Allocate some ordinary memory,
 * with the expectation of freeing it someday. */
void *mem_alloc (size_t size) {
    size_t mem_size;
    void *mem;
    magic_t *magic;
    int i;

    /* */
    mem_size = size + sizeof (*magic);
    for (i = 0; i < MAX_MEM_LIST; i++)
        if (mem_size <= mem_block_sizes[i])
            break;

    EXIT_IF_BUG (i == MAX_MEM_LIST,
        "mem_alloc: size %d too large.", size);

    if (mem_blocks_free[i] == NULL)
        mem = mem_alloc_perm (mem_block_sizes[i]);
    else {
        mem = mem_blocks_free[i];
        mem_blocks_free[i] = *((void **) mem_blocks_free[i]);
    }

    magic  = (magic_t *) mem;
    *magic = (magic_t) MAGIC_NUM;
    return mem + sizeof (*magic);
}

/* Allocate some permanent memory.
 * Permanent memory is never freed,
 *   pointers into it may be copied safely. */
void *mem_alloc_perm (size_t size) {
    static char *mem_perm;
    static int mem_perm_size;
    void *mem;

    while (size % sizeof (long) != 0)
        size++;
    EXIT_IF_BUG (size > MAX_PERM_BLOCK,
        "mem_alloc_perm: %d too large.", size);

    if (mem_perm == NULL || mem_perm_size + size > MAX_PERM_BLOCK) {
        mem_perm_size = 0;
        if ((mem_perm = calloc (1, MAX_PERM_BLOCK)) == NULL) {
            perror ("mem_alloc_perm");
            exit (1);
        }
        if (mem_pages_allocated >= MAX_PERM_BLOCKS)
            bug ("mem_alloc_perm: Warning - exceeeded trackable permanent "
                 "memory blocks! Please increase MAX_PERM_BLOCKS.", 0);
        else
            mem_pages[mem_pages_allocated] = mem_perm;
        mem_pages_allocated++;
    }

    mem                        = mem_perm + mem_perm_size;
    mem_perm_size             += size;
    mem_blocks_allocated      += 1;
    mem_block_bytes_allocated += size;
    return mem;
}

/* Free some memory.
 * Recycle it back onto the free list for blocks of that size. */
void mem_free (void *mem, size_t size) {
    size_t mem_size;
    magic_t *magic;
    int i;

    mem_size = size + sizeof (*magic);
    mem -= sizeof (*magic);
    magic = (magic_t *) mem;

    if (*magic != MAGIC_NUM) {
        bug ("Attempt to recycle invalid memory of size %d.", size);
        bugf ("[%s]\n", (char *) mem + sizeof (*magic));
        return;
    }
    *magic = (magic_t) 0;

    for (i = 0; i < MAX_MEM_LIST; i++)
        if (mem_size <= mem_block_sizes[i])
            break;

    EXIT_IF_BUG (i == MAX_MEM_LIST,
        "mem_free: size %ld too large.", size);

    *((void **) mem) = mem_blocks_free[i];
    mem_blocks_free[i] = mem;
}

int mem_pages_dispose (void) {
    int i, freed;

    freed = mem_pages_allocated;
    for (i = 0; i < mem_pages_allocated; i++) {
        free (mem_pages[i]);
        mem_pages[i] = NULL;
    }
    mem_pages_allocated = 0;

    return freed * MAX_PERM_BLOCK;
}

char *mem_dump (char *eol) {
    static char buf[MAX_STRING_LENGTH];
    const RECYCLE_T *rec;
    int i;
    size_t size, len;

    buf[0] = '\0';
    size = sizeof(buf);
    len = 0;

    for (i = 0; i < RECYCLE_MAX; i++) {
        rec = &(recycle_table[i]);
        len += snprintf (buf + len, size - len,
            "%-11s %5d (%8ld bytes, %d:%d in use:freed)%s", rec->name,
            rec->top, rec->top * rec->size, rec->list_count, rec->free_count,
            eol);
    }
    len += snprintf (buf + len, size - len,
        "strings     %5d (%8d / %d bytes)%s",
        strings_allocated, string_bytes_allocated, MAX_STRING_SPACE, eol);
    len += snprintf (buf + len, size - len,
        "blocks      %5d (%8d bytes, %d pages)%s",
        mem_blocks_allocated, mem_block_bytes_allocated, mem_pages_allocated, eol);

    return buf;
}
