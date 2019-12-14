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
#include "globals.h"

#include "utils.h"

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

/* Removes the tildes from a string.
 * Used for player-entered strings that go into disk files. */
void smash_tilde (char *str) {
    for (; *str != '\0'; str++)
        if (*str == '~')
            *str = '-';
}

/* Removes dollar signs to keep snerts from crashing us.
 * Posted to ROM list by Kyndig. JR -- 10/15/00 */
void smash_dollar (char *str) {
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
bool is_name (const char *str, char *namelist) {
    char name[MAX_INPUT_LENGTH], part[MAX_INPUT_LENGTH];
    const char *list, *string;

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

bool is_exact_name (const char *str, char *namelist) {
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

/* Simple linear interpolation. */
int interpolate (int level, int value_00, int value_32) {
    return value_00 + level * (value_32 - value_00) / 32;
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
    static int mm_state[2 + 55];
#endif

void init_mm (void) {
#if defined (OLD_RAND)
    int *state_ptr;
    int state;

    state_ptr = &mm_state[2];
    state_ptr[-2] = 55 - 55;
    state_ptr[-1] = 55 - 24;

    state_ptr[0] = ((int) current_time) & ((1 << 30) - 1);
    state_ptr[1] = 1;
    for (state = 2; state < 55; state++) {
        state_ptr[state] = (state_ptr[state - 1] + state_ptr[state - 2])
            & ((1 << 30) - 1);
    }
#else
    srandom (time (NULL) ^ getpid ());
#endif
}

long number_mm (void) {
#if defined (OLD_RAND)
    int *state_ptr;
    int state1;
    int state2;
    int rand_int;

    state_ptr = &mm_state[2];
    state1 = state_ptr[-2];
    state2 = state_ptr[-1];
    rand_int = (state_ptr[state1] + state_ptr[state2]) & ((1 << 30) - 1);
    state_ptr[state1] = rand_int;
    if (++state1 == 55)
        state1 = 0;
    if (++state2 == 55)
        state2 = 0;
    state_ptr[-2] = state1;
    state_ptr[-1] = state2;
    return rand_int >> 6;
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

/* Append a string to a file.  */
void append_file (CHAR_T *ch, char *file, char *str) {
    FILE *fp;

    if (IS_NPC (ch) || str[0] == '\0')
        return;

    fclose (reserve_file);
    if ((fp = fopen (file, "a")) == NULL) {
        perror (file);
        send_to_char ("Could not open the file!\n\r", ch);
    }
    else {
        fprintf (fp, "[%5d] %s: %s\n", ch->in_room ? ch->in_room->vnum : 0, ch->name, str);
        fclose (fp);
    }

    reserve_file = fopen (NULL_FILE, "r");
}

/* Reports a bug. */
void bug (const char *str, int param) {
    char buf[MAX_STRING_LENGTH];

    if (current_area_file != NULL) {
        int line, ch;
        if (current_area_file == stdin)
            line = 0;
        else {
            ch = ftell (current_area_file);
            fseek (current_area_file, 0, 0);
            for (line = 0; ftell (current_area_file) < ch; line++)
                while (getc (current_area_file) != '\n');
            fseek (current_area_file, ch, 0);
        }

        log_f ("[*****] FILE: %s LINE: %d", current_area_filename, line);

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
    fclose (reserve_file);
    if ((fp = fopen(BUG_FILE, "a")) != NULL) {
        fprintf(fp, "%s\n", buf);
        fclose(fp);
    }
    reserve_file = fopen (NULL_FILE, "r");
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

char *ctime_fixed (const time_t *timep) {
    char *rval, *nl;
    static char buf[256];

    /* get the value from ctime() and remove and trailing newline. */
    rval = ctime (timep);
    snprintf (buf, sizeof (buf), "%s", rval);
    if ((nl = strchr (buf, '\n')) != NULL)
        *nl = '\0';

    return buf;
}

/* Writes a string to the log. */
void log_string (const char *str) {
    char *strtime;
    strtime = ctime_fixed (&current_time);
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
