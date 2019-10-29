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

#include <stdlib.h>
#include <stdarg.h>
#include <time.h>
#include <unistd.h>
#include <string.h>

#include "comm.h"
#include "db.h"
#include "interp.h"
#include "chars.h"

#include "utils.h"

/* Globals. */
int nAllocString;
int sAllocString;
int nAllocPerm;
int sAllocPerm;
char *string_space;
char *top_string;
void *rgFreeList[MAX_MEM_LIST];
const int rgSizeList[MAX_MEM_LIST] = {
    16, 32, 64, 128, 256, 1024, 2048, 4096, 8192, 16384, 32768 - 64
};
char str_empty[1];

/* Keep track of allocated memory blocks so Valgrind
 * doesn't report lost memory. -- Synival */
char *pMemPermArray[MAX_PERM_BLOCKS];
int nMemPermCount = 0;

/* External data we need to pull. */
extern FILE *fpArea;
extern char strArea[MAX_INPUT_LENGTH];

void init_string_space (void) {
    EXIT_IF_BUG ((string_space = calloc (1, MAX_STRING)) == NULL,
        "init_string_space: can't alloc %d string space.", MAX_STRING);
    top_string = string_space;
}

/* Returns an initial-capped string. */
char *capitalize (const char *str) {
    static char strcap[MAX_STRING_LENGTH];
    int i;
    for (i = 0; str[i] != '\0'; i++)
        strcap[i] = LOWER (str[i]);
    strcap[i] = '\0';
    strcap[0] = UPPER (strcap[0]);
    return strcap;
}

/* Stick a little fuzz on a number. */
int number_fuzzy (int number) {
    switch (number_bits(2)) {
        case 0:
            number -= 1;
            break;
        case 3:
            number += 1;
            break;
    }
    return UMAX (1, number);
}

/* I've gotten too many bad reports on OS-supplied random number generators.
 * This is the Mitchell-Moore algorithm from Knuth Volume II.
 * Best to leave the constants alone unless you've read Knuth.
 * -- Furey */

/* I noticed streaking with this random number generator, so I switched
   back to the system srandom call.  If this doesn't work for you,
   define OLD_RAND to use the old system -- Alander */

#if defined (OLD_RAND)
    static int rgiState[2 + 55];
#endif

void init_mm (void) {
#if defined (OLD_RAND)
    int *piState;
    int iState;

    piState = &rgiState[2];
    piState[-2] = 55 - 55;
    piState[-1] = 55 - 24;

    piState[0] = ((int) current_time) & ((1 << 30) - 1);
    piState[1] = 1;
    for (iState = 2; iState < 55; iState++) {
        piState[iState] = (piState[iState - 1] + piState[iState - 2])
            & ((1 << 30) - 1);
    }
#else
    srandom (time (NULL) ^ getpid ());
#endif
}

long number_mm (void) {
#if defined (OLD_RAND)
    int *piState;
    int iState1;
    int iState2;
    int iRand;

    piState = &rgiState[2];
    iState1 = piState[-2];
    iState2 = piState[-1];
    iRand = (piState[iState1] + piState[iState2]) & ((1 << 30) - 1);
    piState[iState1] = iRand;
    if (++iState1 == 55)
        iState1 = 0;
    if (++iState2 == 55)
        iState2 = 0;
    piState[-2] = iState1;
    piState[-1] = iState2;
    return iRand >> 6;
#else
    return random () >> 6;
#endif
}

/* Generate a random number. */
int number_range (int from, int to) {
    int power;
    int number;

    if (from == 0 && to == 0)
        return 0;
    if ((to = to - from + 1) <= 1)
        return from;
    for (power = 2; power < to; power <<= 1)
        ;
    while ((number = number_mm () & (power - 1)) >= to)
        ;
    return from + number;
}

/* Generate a percentile roll. */
int number_percent (void) {
    int percent;
    while ((percent = number_mm () & (128 - 1)) > 99);
    return 1 + percent;
}

/* Generate a random door. */
int number_door (void) {
    int door;
    while ((door = number_mm () & 0x07) > 5);
    return door;
}

int number_bits (int width) {
    return number_mm () & ((1 << width) - 1);
}

/* Roll some dice. */
int dice (int number, int size) {
    int idice;
    int sum;

    switch (size) {
        case 0:
            return 0;
        case 1:
            return number;
    }

    for (idice = 0, sum = 0; idice < number; idice++)
        sum += number_range (1, size);
    return sum;
}

/* Simple linear interpolation. */
int interpolate (int level, int value_00, int value_32) {
    return value_00 + level * (value_32 - value_00) / 32;
}

/* Removes the tildes from a string.
 * Used for player-entered strings that go into disk files. */
void smash_tilde (char *str) {
    for (; *str != '\0'; str++)
        if (*str == '~')
            *str = '-';
}

/* Removes dollar signs to keep snerts from crashing us.
 * Posted to ROM list by Kyndig. JR -- 10/15/00 */
void smash_dollar( char *str ) {
    for (; *str != '\0'; str++)
        if (*str == '$')
            *str = 'S';
}

/* Compare strings, case insensitive.
 * Return TRUE if different
 *   (compatibility with historical functions). */
bool str_cmp (const char *astr, const char *bstr) {
    RETURN_IF_BUG (astr == NULL,
        "str_cmp: null astr.", 0, TRUE);
    RETURN_IF_BUG (bstr == NULL,
        "str_cmp: null bstr.", 0, TRUE);

    for (; *astr || *bstr; astr++, bstr++)
        if (LOWER (*astr) != LOWER (*bstr))
            return TRUE;
    return FALSE;
}

/* Compare strings, case insensitive, for prefix matching.
 * Return TRUE if astr not a prefix of bstr
 *   (compatibility with historical functions). */
bool str_prefix (const char *astr, const char *bstr) {
    RETURN_IF_BUG (astr == NULL,
        "str_prefix: null astr.", 0, TRUE);
    RETURN_IF_BUG (bstr == NULL,
        "str_prefix: null bstr.", 0, TRUE);

    for (; *astr; astr++, bstr++)
        if (LOWER (*astr) != LOWER (*bstr))
            return TRUE;
    return FALSE;
}

/* Compare strings, case insensitive, for match anywhere.
 * Returns TRUE is astr not part of bstr.
 *   (compatibility with historical functions). */
bool str_infix (const char *astr, const char *bstr) {
    int sstr1;
    int sstr2;
    int ichar;
    char c0;

    if ((c0 = LOWER (astr[0])) == '\0')
        return FALSE;

    sstr1 = strlen (astr);
    sstr2 = strlen (bstr);

    for (ichar = 0; ichar <= sstr2 - sstr1; ichar++)
        if (c0 == LOWER (bstr[ichar]) && !str_prefix (astr, bstr + ichar))
            return FALSE;

    return TRUE;
}

/* Compare strings, case insensitive, for suffix matching.
 * Return TRUE if astr not a suffix of bstr
 *   (compatibility with historical functions). */
bool str_suffix (const char *astr, const char *bstr) {
    int sstr1;
    int sstr2;

    sstr1 = strlen (astr);
    sstr2 = strlen (bstr);
    if (sstr1 <= sstr2 && !str_cmp (astr, bstr + sstr2 - sstr1))
        return FALSE;
    else
        return TRUE;
}

/* Append a string to a file.  */
void append_file (CHAR_DATA * ch, char *file, char *str) {
    FILE *fp;

    if (IS_NPC (ch) || str[0] == '\0')
        return;

    fclose (fpReserve);
    if ((fp = fopen (file, "a")) == NULL) {
        perror (file);
        send_to_char ("Could not open the file!\n\r", ch);
    }
    else {
        fprintf (fp, "[%5d] %s: %s\n", ch->in_room ? ch->in_room->vnum : 0, ch->name, str);
        fclose (fp);
    }

    fpReserve = fopen (NULL_FILE, "r");
}

/* Reports a bug. */
void bug (const char *str, int param) {
    char buf[MAX_STRING_LENGTH];

    if (fpArea != NULL) {
        int iLine;
        int iChar;

        if (fpArea == stdin)
            iLine = 0;
        else {
            iChar = ftell (fpArea);
            fseek (fpArea, 0, 0);
            for (iLine = 0; ftell (fpArea) < iChar; iLine++)
                while (getc (fpArea) != '\n');
            fseek (fpArea, iChar, 0);
        }

        log_f ("[*****] FILE: %s LINE: %d", strArea, iLine);

/* RT removed because we don't want bugs shutting the mud
        if ((fp = fopen("shutdown.txt", "a")) != NULL) {
            fprintf (fp, "[*****] %s\n", buf);
            fclose (fp);
        }
*/
    }

    strcpy (buf, "[*****] BUG: ");
    sprintf (buf + strlen (buf), str, param);
    log_string (buf);

/* RT removed due to bug-file spamming
    fclose (fpReserve);
    if ((fp = fopen(BUG_FILE, "a")) != NULL) {
        fprintf(fp, "%s\n", buf);
        fclose(fp);
    }
    fpReserve = fopen (NULL_FILE, "r");
*/
}

void bugf (const char *fmt, ...) {
    char buf[2 * MSL];
    va_list args;
    va_start (args, fmt);
    vsnprintf (buf, sizeof(buf), fmt, args);
    va_end (args);
    bug (buf, 0);
}

/* Writes a string to the log. */
void log_string (const char *str) {
    char *strtime;
    strtime = ctime (&current_time);
    strtime[strlen (strtime) - 1] = '\0';
    fprintf (stderr, "%s :: %s\n", strtime, str);
}

void log_f (const char *fmt, ...) {
    char buf[2 * MSL];
    va_list args;
    va_start (args, fmt);
    vsnprintf (buf, sizeof(buf), fmt, args);
    va_end (args);
    log_string (buf);
}

/* This function is here to aid in debugging.
 * If the last expression in a function is another function call,
 *   gcc likes to generate a JMP instead of a CALL.
 * This is called "tail chaining."
 * It hoses the debugger call stack for that call.
 * So I make this the last call in certain critical functions,
 *   where I really need the call stack to be right for debugging!
 *
 * If you don't understand this, then LEAVE IT ALONE.
 * Don't remove any calls to tail_chain anywhere.
 *
 * -- Furey */
void tail_chain (void) {
    return;
}

/* Allocate some ordinary memory,
 * with the expectation of freeing it someday. */
void *alloc_mem (int sMem) {
    void *pMem;
    int *magic;
    int iList;

    sMem += sizeof (*magic);
    for (iList = 0; iList < MAX_MEM_LIST; iList++)
        if (sMem <= rgSizeList[iList])
            break;

    EXIT_IF_BUG (iList == MAX_MEM_LIST,
        "alloc_mem: size %d too large.", sMem);

    if (rgFreeList[iList] == NULL)
        pMem = alloc_perm (rgSizeList[iList]);
    else {
        pMem = rgFreeList[iList];
        rgFreeList[iList] = *((void **) rgFreeList[iList]);
    }

    magic  = (int *) pMem;
    *magic = MAGIC_NUM;
    pMem   += sizeof (*magic);

    return pMem;
}

/* Free some memory.
 * Recycle it back onto the free list for blocks of that size. */
void mem_free (void *pMem, int sMem) {
    int iList;
    int *magic;

    pMem -= sizeof (*magic);
    magic = (int *) pMem;

    if (*magic != MAGIC_NUM) {
        bug ("Attempt to recyle invalid memory of size %d.", sMem);
        bug ((char *) pMem + sizeof (*magic), 0);
        return;
    }

    *magic = 0;
    sMem += sizeof (*magic);

    for (iList = 0; iList < MAX_MEM_LIST; iList++)
        if (sMem <= rgSizeList[iList])
            break;

    EXIT_IF_BUG (iList == MAX_MEM_LIST,
        "mem_free: size %d too large.", sMem);

    *((void **) pMem) = rgFreeList[iList];
    rgFreeList[iList] = pMem;
}

/* Allocate some permanent memory.
 * Permanent memory is never freed,
 *   pointers into it may be copied safely. */
void *alloc_perm (int sMem) {
    static char *pMemPerm;
    static int iMemPerm;
    void *pMem;

    while (sMem % sizeof (long) != 0)
        sMem++;
    EXIT_IF_BUG (sMem > MAX_PERM_BLOCK,
        "alloc_perm: %d too large.", sMem);

    if (pMemPerm == NULL || iMemPerm + sMem > MAX_PERM_BLOCK) {
        iMemPerm = 0;
        if ((pMemPerm = calloc (1, MAX_PERM_BLOCK)) == NULL) {
            perror ("Alloc_perm");
            exit (1);
        }
        if (nMemPermCount >= MAX_PERM_BLOCKS)
            bug ("alloc_perm: Warning - exceeeded trackable permanent memory "
                 "blocks! Please increase MAX_PERM_BLOCKS.", 0);
        else
            pMemPermArray[nMemPermCount] = pMemPerm;
        nMemPermCount++;
    }

    pMem = pMemPerm + iMemPerm;
    iMemPerm += sMem;
    nAllocPerm += 1;
    sAllocPerm += sMem;
    return pMem;
}

/* Frees a potentially already-allocated string and
 * replaces it with a newly created on. */
void str_replace_dup (char **old, const char *str) {
    str_free (*old);
    *old = str_dup (str);
}

/* Duplicate a string into dynamic memory.
 * Fread_strings are read-only and shared. */
char *str_dup (const char *str) {
    char *str_new;

    if (str == NULL)
        return NULL;
    if (str[0] == '\0')
        return &str_empty[0];
    if (str >= string_space && str < top_string)
        return (char *) str;

    str_new = alloc_mem (strlen (str) + 1);
    strcpy (str_new, str);
    return str_new;
}

/* Free a string.
 * Null is legal here to simplify callers.
 * Read-only shared strings are not touched. */
void str_free (char *pstr) {
    if (pstr == NULL || pstr == &str_empty[0])
        return;
    if (pstr >= string_space && pstr < top_string)
        return;
    mem_free (pstr, strlen (pstr) + 1);
}

/* Returns 'str' if non-null, otherwise 'ifnull'. */
const char *if_null_str (const char *str, const char *ifnull) {
    return str ? str : ifnull;
}

char *trim_extension (char *input) {
    static char fbuf[256], *period;
    snprintf (fbuf, sizeof (fbuf), "%s", input);
    if ((period = strrchr (fbuf, '.')) != NULL)
        *period = '\0';
    return fbuf;
}

/* See if a string is one of the names of an object. */
bool is_name (char *str, char *namelist) {
    char name[MAX_INPUT_LENGTH], part[MAX_INPUT_LENGTH];
    char *list, *string;

    /* fix crash on NULL namelist */
    if (namelist == NULL || namelist[0] == '\0')
        return FALSE;

    /* fixed to prevent is_name on "" returning TRUE */
    if (str == NULL || str[0] == '\0')
        return FALSE;

    /* we need ALL parts of string to match part of namelist */
    string = str;
    while (1) { /* start parsing string */
        str = one_argument (str, part);
        if (part[0] == '\0')
            return TRUE;

        /* check to see if this is part of namelist */
        list = namelist;
        while (1) { /* start parsing namelist */
            list = one_argument (list, name);
            if (name[0] == '\0') /* this name was not found */
                return FALSE;
            if (!str_prefix (string, name))
                return TRUE; /* full pattern match */
            if (!str_prefix (part, name))
                break;
        }
    }
}

bool is_exact_name (char *str, char *namelist) {
    char name[MAX_INPUT_LENGTH];

    if (namelist == NULL)
        return FALSE;
    while (1) {
        namelist = one_argument (namelist, name);
        if (name[0] == '\0')
            return FALSE;
        if (!str_cmp (str, name))
            return TRUE;
    }
}

/* See if a string is one of the names of an object. */
bool is_full_name (const char *str, char *namelist) {
    char name[MIL];
    while (1) {
        namelist = one_argument (namelist, name);
        if (name[0] == '\0')
            return FALSE;
        if (!str_cmp (str, name))
            return TRUE;
    }
}
