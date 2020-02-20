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
#include <ctype.h>

#include "utils.h"
#include "memory.h"
#include "globals.h"

#include "fread.h"

/* Read a letter from a file.  */
char fread_letter (FILE *fp) {
    char c;
    do {
        c = getc (fp);
    } while (isspace (c));
    return c;
}

/* Read a number from a file. */
int fread_number (FILE *fp) {
    int number = 0;
    bool sign = FALSE;
    char c = fread_letter(fp);

    if (c == '+')
        c = getc (fp);
    else if (c == '-') {
        sign = TRUE;
        c = getc (fp);
    }

    EXIT_IF_BUG (!isdigit (c),
        "fread_number: bad format.", 0);

    while (isdigit (c)) {
        number = number * 10 + c - '0';
        c = getc (fp);
    }

    if (sign)
        number = 0 - number;

    if (c == '|')
        number += fread_number (fp);
    else if (c != ' ')
        ungetc (c, fp);

    return number;
}

flag_t fread_flag (FILE *fp) {
    int number = 0;
    bool negative = FALSE;
    char c = fread_letter(fp);

    if (c == '-') {
        negative = TRUE;
        c = getc (fp);
    }

    if (!isdigit (c)) {
        while (('A' <= c && c <= 'Z') || ('a' <= c && c <= 'z')) {
            number += fread_flag_convert (c);
            c = getc (fp);
        }
    }

    while (isdigit (c)) {
        number = number * 10 + c - '0';
        c = getc (fp);
    }

    if (c == '|')
        number += fread_flag (fp);
    else if (c != ' ')
        ungetc (c, fp);

    return negative ? -1 * number : number;
}

flag_t fread_flag_convert (char letter) {
    flag_t bitsum = 0;
    char i;

    if ('A' <= letter && letter <= 'Z') {
        bitsum = 1;
        for (i = letter; i > 'A'; i--)
            bitsum *= 2;
    }
    else if ('a' <= letter && letter <= 'z') {
        bitsum = 67108864;        /* 2^26 */
        for (i = letter; i > 'a'; i--)
            bitsum *= 2;
    }

    return bitsum;
}

EXT_FLAGS_T fread_ext_flag (FILE *fp, const EXT_FLAG_DEF_T *table) {
    char c;

    /* if the flag isn't in some sort of array format, read as an old flag. */
    while (1) {
        c = getc (fp);
        if (!(c == ' ' || c == '\n' || c == '\r'))
            break;
    }
    if (c != '[') {
        ungetc (c, fp);
        return EXT_FROM_FLAG_T (fread_flag (fp));
    }
    else {
        char buf[MAX_STRING_LENGTH];
        long start, count;

        /* look for a corresponding right bracket. */
        start = ftell (fp);
        count = 0;
        do {
            if ((c = getc (fp)) == ']')
                break;
            EXIT_IF_BUG (++count >= (MAX_STRING_LENGTH - 1),
                "fread_ext_flag: Extended flag list too long.", 0);
            EXIT_IF_BUG (feof (fp),
                "fread_ext_flag: Unterminated extended flag list. "
                "Expected ']', got EOF.", 0);
        } while (1);

        fseek (fp, start, SEEK_SET);

        /* Read the flags and the right bracket afterwards. */
        if (count > 0 && fread (buf, sizeof (char), count, fp) <= 0) {
            if (feof (fp))
                bugf ("fread_ext_flag: Premature EOF");
            else {
                int error = ferror (fp);
                bugf ("fread_ext_flag: File error - %s (%d)", strerror (error));
            }
        }
        buf[count] = '\0';
        getc (fp);

        /* Build flags from our string. */
        return ext_flags_from_string (table, buf);
    }
}

/* Read and allocate space for a string from a file.
 * These strings are read-only and shared.
 * Strings are hashed:
 *   each string prepended with hash pointer to prev string,
 *   hash code is simply the string length.
 *   this function takes 40% to 50% of boot-up time. */
char *fread_string_replace (FILE *fp, char **value) {
    const char *str = fread_string_static (fp);
    str_replace_dup (value, str);
    return *value;
}

char *fread_string_dup (FILE *fp) {
    return str_dup (fread_string_static (fp));
}

char *fread_string_static (FILE *fp) {
    static char buf[MAX_STRING_LENGTH * 4];
    return fread_string (fp, buf, sizeof (buf));
}

char *fread_string (FILE *fp, char *buf, size_t size) {
    char *plast;
    char c;

    plast = buf;
    c = fread_letter (fp);
    if ((*plast++ = c) == '~')
        return str_empty;

    /* TODO: mind 'size'. */
    while (1) {
        /* Back off the char type lookup,
         * it was too dirty for portability.
         *   -- Furey */

        switch (*plast = getc (fp)) {
            case EOF:
                /* temp fix */
                bug ("fread_string: EOF", 0);
                return NULL;

            case '\n':
                plast++;
                *plast++ = '\r';
                break;

            case '\r':
                break;

            case '~':
                *plast = '\0';
                return buf;

            default:
                plast++;
                break;
        }
    }
}

char *fread_string_eol_replace (FILE *fp, char **value) {
    const char *str = fread_string_eol_static (fp);
    str_replace_dup (value, str);
    return *value;
}

char *fread_string_eol_dup (FILE *fp) {
    return str_dup (fread_string_eol_static (fp));
}

char *fread_string_eol_static (FILE *fp) {
    static char buf[MAX_STRING_LENGTH * 4];
    return fread_string_eol (fp, buf, sizeof (buf));
}

char *fread_string_eol (FILE *fp, char *buf, size_t size) {
    static bool char_special[256 - EOF];
    char *plast;

    if (char_special[EOF - EOF] != TRUE) {
        char_special[EOF - EOF]  = TRUE;
        char_special['\n' - EOF] = TRUE;
        char_special['\r' - EOF] = TRUE;
    }

    plast = buf;
    *plast++ = fread_letter (fp);
    if (plast[-1] == '\n' || plast[-1] == '\r')
        return str_empty;

    while (1) {
        if (!char_special[(*plast++ = getc (fp)) - EOF])
            continue;

        switch (plast[-1]) {
            case EOF:
                bug ("fread_string_eol: EOF", 0);
                exit (1);
                break;

            case '\n':
            case '\r':
                plast--;
                *plast = '\0';
                return buf;

            default:
                break;
        }
    }
}

/* Read one word (into static buffer). */
char *fread_word_replace (FILE *fp, char **value) {
    const char *str = fread_word_static (fp);
    str_replace_dup (value, str);
    return *value;
}

char *fread_word_dup (FILE *fp) {
    return str_dup (fread_word_static (fp));
}

char *fread_word_static (FILE *fp) {
    static char word[MAX_INPUT_LENGTH];
    return fread_word (fp, word, sizeof (word));
}

char *fread_word (FILE *fp, char *buf, size_t size) {
    char *pword;
    char end_ch;

    end_ch = fread_letter (fp);
    if (end_ch == '\'' || end_ch == '"')
        pword = buf;
    else {
        buf[0] = end_ch;
        pword = buf + 1;
        end_ch = ' ';
    }

    for (; pword < buf + size; pword++) {
        *pword = getc (fp);
        if (end_ch == ' ' ? isspace (*pword) : *pword == end_ch) {
            if (end_ch == ' ')
                ungetc (*pword, fp);
            *pword = '\0';
            return buf;
        }
    }

    EXIT_IF_BUG (TRUE,
        "fread_word: word too long.", 0);
    return NULL;
}

/* Read to end of line (for comments). */
void fread_to_eol (FILE *fp) {
    char c;

    do {
        c = getc (fp);
    } while (c != '\n' && c != '\r');

    do {
        c = getc (fp);
    } while (c == '\n' || c == '\r');

    ungetc (c, fp);
}

void fread_dice (FILE *fp, DICE_T *out) {
    out->number = fread_number (fp);
    /* 'd'     */ fread_letter (fp);
    out->size   = fread_number (fp);
    /* '+'     */ fread_letter (fp);
    out->bonus  = fread_number (fp);
}
