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
#include <stdarg.h>

#include "utils.h"
#include "globals.h"
#include "recycle.h"

#include "memory.h"

/* Memory management.
 * Increase MAX_STRING if you have too.
 * Tune the others only if you understand what you're doing. */
#define MAGIC_NUM           52571214
#define MAX_STRING_SPACE    2097152 /* 2^21 */
#define MAX_PERM_BLOCK      131072
#define MAX_PERM_BLOCKS     1024
#define MAX_MEM_LIST        11

#ifndef BASEMUD_DEBUG_DISABLE_MEMORY_MANAGEMENT
    /* Magic number for memory allocation */
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
#endif

void string_space_init (void) {
#ifndef BASEMUD_DEBUG_DISABLE_MEMORY_MANAGEMENT
    EXIT_IF_BUG ((string_space = calloc (1, MAX_STRING_SPACE)) == NULL,
        "string_space_init: can't alloc %d string space. Please decrease "
        "MAX_STRING_SPACE.", MAX_STRING_SPACE);
    top_string = string_space;
#endif
}

int string_space_dispose (void) {
#ifdef BASEMUD_DEBUG_DISABLE_MEMORY_MANAGEMENT
    return 0;
#else
    if (string_space == NULL)
        return 0;
    free (string_space);
    string_space = NULL;
    top_string = NULL;
    return MAX_STRING_SPACE;
#endif
}

char *string_space_next (void) {
#ifdef BASEMUD_DEBUG_DISABLE_MEMORY_MANAGEMENT
    static char fake_string_space[MAX_STRING_LENGTH];
    return fake_string_space;
#else
    char *str = top_string + sizeof (char *);
    EXIT_IF_BUG (str > &string_space[MAX_STRING_SPACE - MAX_STRING_LENGTH],
        "string_space_next: MAX_STRING_SPACE %d exceeded. Please increase.",
        MAX_STRING_SPACE);
    return str;
#endif
}

/* Frees a potentially already-allocated string and
 * replaces it with a newly created on. */
void str_replace_dup (char **old, const char *str) {
    if (old != NULL && *old == str)
        return;
    str_free (old);
    *old = str_dup (str);
}

/* Duplicate a string into dynamic memory (unless booting).
 * Fread_strings are read-only and shared. */
char *str_dup (const char *str) {
#ifndef BASEMUD_DEBUG_DISABLE_MEMORY_MANAGEMENT
    char *str_new;
#endif

    if (str == NULL)
        return NULL;
    if (str[0] == '\0')
        return str_empty;
#ifdef BASEMUD_DEBUG_DISABLE_MEMORY_MANAGEMENT
    return strdup (str);
#else
    if (str >= string_space && str < top_string)
        return (char *) str;
    if (in_boot_db)
        return str_register (str);

    str_new = mem_alloc (strlen (str) + 1);
    strcpy (str_new, str);
    return str_new;
#endif
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
#ifdef BASEMUD_DEBUG_DISABLE_MEMORY_MANAGEMENT
    free (*pstr);
    *pstr = NULL;
#else
    if (*pstr >= string_space && *pstr < top_string) {
        *pstr = NULL;
        return;
    }
    mem_free (*pstr, strlen (*pstr) + 1);
    *pstr = NULL;
#endif
}

/* Add a string to the global hash of read-only, permanent strings. */
char *str_register (const char *str) {
#ifdef BASEMUD_DEBUG_DISABLE_MEMORY_MANAGEMENT
    return NULL;
#else
    union {
        char *pc;
        char rgc[sizeof (char *)];
    } hash_ptr;

    unsigned int i, hash_index;
    unsigned int hash_value;
    char *new, *new_str;
    unsigned int *new_hash;
    size_t new_len;
    char *existing_str;

    /* Ignore special cases. */
    if (str == NULL)
        return NULL;
    if (*str == '\0')
        return str_empty;

    /* Check to see if this exact string is already loaded. Each string
     * is stored in the following structure:
     * ----------------------------------------------------------------
     *      (char *)                 (int)           (char x len) '\0'
     *      ^^ pointer to next hash, ^^ string hash, ^^ data
     * ----------------------------------------------------------------
     * Each hash is traversed back to the first string loaded similar to
     * traversing through a linked list.
     *
     * Hashes are generated like so:
     * 1) Set 'hash_value' to a generated non-unique hash.
     *    This will be stored in the 'string_hash' table data.
     * 2) set 'hash_index' to 'hash_value % MAX_KEY_HASH'.
     *    This is used to divide the hash table into smaller chunks
     *    to speed up lookup by MAX_KEY_HASH times.
     * 3) Store each string in 'string_hash[hash_index]'. */

    /* Generate a hash VALUE for the string for comparison. */
    /* Method is 'djb2' by Dan Bernstein, c/o:
     *    https://stackoverflow.com/a/7666577/7270901 */
    hash_value = 0;
    for (i = 0; str[i] != '\0'; i++)
        hash_value = ((hash_value << 5) + hash_value) + str[i];
    hash_index = hash_value % MAX_KEY_HASH;

    /* See if there's a string already registered. */
    if ((existing_str = str_get_registered (str, hash_index, hash_value)))
        return existing_str;

    /* No matching string was found - so register it into 'string_space'.
       If this string isn't already loaded into 'string_space', do it now. */
    new_len  = strlen (str) + sizeof (char *) + sizeof (unsigned int) + 1;
    EXIT_IF_BUG (string_bytes_allocated + new_len > MAX_STRING_SPACE,
        "str_register: MAX_STRING_SPACE %d exceeded. Please increase.",
        MAX_STRING_SPACE);

    new      = top_string;
    new_hash = (void *) new + sizeof (char *);
    new_str  = (void *) new_hash + sizeof (unsigned int);

    /* Set the pointer to the next string in the hash. */
    hash_ptr.pc = string_hash[hash_index];
    for (i = 0; i < sizeof (char *); i++)
        new[i] = hash_ptr.rgc[i];
    string_hash[hash_index] = new;

    /* Set the hash VALUE. */
    *new_hash = hash_value;

    /* Finally copy the string. */
    strcpy (new_str, str);

    strings_allocated      += 1;
    top_string             += new_len;
    string_bytes_allocated += new_len;

    /* Return the string in its 'string_space' location, without the pointer
     * to the next hashed string. */
    return new_str;
#endif
}

char *str_get_registered (const char *str, unsigned int hash_index,
    unsigned int hash_value)
{
#ifdef BASEMUD_DEBUG_DISABLE_MEMORY_MANAGEMENT
    return NULL;
#else
    union {
        char *pc;
        char rgc[sizeof (char *)];
    } hash_ptr;

    int i;
    char *hash, *hash_str, *next_hash;
    unsigned int *hash_hash; /* This is the stupidest name ever, but...
                              * it's the hash in the hash. /shrug */

    for (hash = string_hash[hash_index]; hash; hash = next_hash) {
        /* Get the pointer to the next string. */
        for (i = 0; i < sizeof (char *); i++)
            hash_ptr.rgc[i] = hash[i];
        next_hash = hash_ptr.pc;

        /* Compare hash VALUES between strings. */
        hash_hash = (void *) hash + sizeof (char *);
        if (*hash_hash != hash_value)
            continue;

        /* Compare the string values. If it already exists, we're done. */
        hash_str = (void *) hash_hash + sizeof (unsigned int);
        if (!strcmp (str, hash_str))
            return hash_str;
    }
    return NULL;
#endif
}

int str_get_strings_allocated (void)
    { return strings_allocated; }

/* Allocate some ordinary memory,
 * with the expectation of freeing it someday. */
void *mem_alloc (size_t size) {
#ifdef BASEMUD_DEBUG_DISABLE_MEMORY_MANAGEMENT
    return calloc (1, size);
#else
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
#endif
}

/* Allocate some permanent memory.
 * Permanent memory is never freed,
 *   pointers into it may be copied safely. */
void *mem_alloc_perm (size_t size) {
#ifdef BASEMUD_DEBUG_DISABLE_MEMORY_MANAGEMENT
    return calloc (1, size);
#else
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
#endif
}

/* Free some memory.
 * Recycle it back onto the free list for blocks of that size. */
void mem_free (void *mem, size_t size) {
#ifdef BASEMUD_DEBUG_DISABLE_MEMORY_MANAGEMENT
    free (mem);
    return;
#else
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
#endif
}

int mem_pages_dispose (void) {
#ifdef BASEMUD_DEBUG_DISABLE_MEMORY_MANAGEMENT
    return 0;
#else
    int i, freed;

    freed = mem_pages_allocated;
    for (i = 0; i < mem_pages_allocated; i++) {
        free (mem_pages[i]);
        mem_pages[i] = NULL;
    }
    mem_pages_allocated = 0;

    return freed * MAX_PERM_BLOCK;
#endif
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

#ifndef BASEMUD_DEBUG_DISABLE_MEMORY_MANAGEMENT
    len += snprintf (buf + len, size - len,
        "strings     %5d (%8d / %d bytes)%s",
        strings_allocated, string_bytes_allocated, MAX_STRING_SPACE, eol);
    len += snprintf (buf + len, size - len,
        "blocks      %5d (%8d bytes, %d pages)%s",
        mem_blocks_allocated, mem_block_bytes_allocated, mem_pages_allocated, eol);
#endif

    return buf;
}

/* buffer sizes */
static const int buf_size[MAX_BUF_LIST] = {
    16, 32, 64, 128, 256, 1024, 2048, 4096, 8192, 16384
};

/* local procedure for finding the next acceptable size */
/* -1 indicates out-of-boundary error */
int buf_get_size (int val) {
    int i;
    for (i = 0; i < MAX_BUF_LIST; i++)
        if (buf_size[i] >= val)
            return buf_size[i];
    return -1;
}

BUFFER_T *buf_new_size (int size) {
    size_t last_size;
    BUFFER_T *buf;

    last_size = new_buf_size;
    new_buf_size = size;
    buf = buf_new ();
    new_buf_size = last_size;

    return buf;
}

bool buf_cat (BUFFER_T *buffer, char *string) {
    int len;
    char *oldstr;
    int oldsize;

    oldstr = buffer->string;
    oldsize = buffer->size;

    if (buffer->state == BUFFER_OVERFLOW)    /* don't waste time on bad strings! */
        return FALSE;

    len = strlen (buffer->string) + strlen (string) + 1;
    while (len >= buffer->size) { /* increase the buffer size */
        buffer->size = buf_get_size (buffer->size + 1); {
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

void buf_clear (BUFFER_T *buffer) {
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

    buf_cat (buffer, buf);
}
