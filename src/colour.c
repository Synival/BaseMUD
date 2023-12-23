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

/*   QuickMUD - The Lazy Man's ROM - $Id: act_comm.c,v 1.2 2000/12/01 10:48:33 ring0 Exp $ */

#include "colour.h"

#include "chars.h"
#include "tables.h"

#include <stdio.h>
#include <string.h>

int colour_to_ansi (flag_t colour, char *buf_out, size_t size) {
    const char *flag_str = "", *fg_str = "", *bg_str = "", *beep_str = "";
    if (buf_out == NULL || size <= 0)
        return 0;

    /* Beeping adds a non-ANSI escape char. */
    if (colour & CB_BEEP)
        beep_str = "\a";

    /* Handle our flag bits. */
    if ((colour & CB_DEFAULT) && (colour & CB_BRIGHT))
        flag_str = "0;1";
    else if (colour & CB_BRIGHT)
        flag_str = "1";
    else
        flag_str = "0";

    if (!(colour & CB_DEFAULT)) {
        switch (colour & CM_FORECOLOUR & ~CB_BRIGHT) {
            case CC_BLACK:   fg_str = ";30"; break;
            case CC_RED:     fg_str = ";31"; break;
            case CC_GREEN:   fg_str = ";32"; break;
            case CC_YELLOW:  fg_str = ";33"; break;
            case CC_BLUE:    fg_str = ";34"; break;
            case CC_MAGENTA: fg_str = ";35"; break;
            case CC_CYAN:    fg_str = ";36"; break;
            case CC_WHITE:   fg_str = ";37"; break;
        }
    }

    if (!(colour & CB_BACK_DEFAULT)) {
        switch (colour & CM_BACKCOLOUR & ~CB_BACK_BRIGHT) {
            case CC_BACK_BLACK:   bg_str = ";40"; break;
            case CC_BACK_RED:     bg_str = ";41"; break;
            case CC_BACK_GREEN:   bg_str = ";42"; break;
            case CC_BACK_YELLOW:  bg_str = ";43"; break;
            case CC_BACK_BLUE:    bg_str = ";44"; break;
            case CC_BACK_MAGENTA: bg_str = ";45"; break;
            case CC_BACK_CYAN:    bg_str = ";46"; break;
            case CC_BACK_WHITE:   bg_str = ";47"; break;
        }
    }

    /* return the number of characters written to buf_out. */
    return snprintf (buf_out, size, "\x1b[%s%s%sm%s",
        flag_str, fg_str, bg_str, beep_str);
}

int colour_code_to_ansi (CHAR_T *ch, bool use_colour,
    char type, char *buf_out, size_t size)
{
    char code[32];
    if (ch != NULL) {
        ch = REAL_CH (ch);
        use_colour = EXT_IS_SET (ch->ext_plr, PLR_COLOUR) ? TRUE : FALSE;
    }

    /* Look for a hard-coded colour code. */
    code[0] = '\0';
    #define BD CC_BACK_DEFAULT
    #define CTA(c) (colour_to_ansi (c, code, sizeof(code)))
    switch (type) {
        case '0': CTA(BD | CC_BLACK);   break;
        case 'r': CTA(BD | CC_RED);     break;
        case 'b': CTA(BD | CC_BLUE);    break;
        case 'c': CTA(BD | CC_CYAN);    break;
        case 'g': CTA(BD | CC_GREEN);   break;
        case 'm': CTA(BD | CC_MAGENTA); break;
        case 'y': CTA(BD | CC_YELLOW);  break;
        case 'w': CTA(BD | CC_WHITE);   break;

        case 'D': CTA(BD | CC_DARK_GREY);      break;
        case 'R': CTA(BD | CC_BRIGHT_RED);     break;
        case 'B': CTA(BD | CC_BRIGHT_BLUE);    break;
        case 'C': CTA(BD | CC_BRIGHT_CYAN);    break;
        case 'G': CTA(BD | CC_BRIGHT_GREEN);   break;
        case 'M': CTA(BD | CC_BRIGHT_MAGENTA); break;
        case 'Y': CTA(BD | CC_BRIGHT_YELLOW);  break;
        case 'W': CTA(BD | CC_BRIGHT_WHITE);   break;

        case 'x': CTA(CC_CLEAR);   break;

        case '*': strcpy (code, "\a");   break;
        case '/': strcpy (code, "\n\r"); break;
        case '-': strcpy (code, "~");    break;
        case '{': strcpy (code, "{");    break;
    }

    /* if we didn't find a code, look one up. */
    if (code[0] == '\0' && ch != NULL && ch->pcdata) {
        const COLOUR_SETTING_T *setting;
        setting = colour_setting_get_by_char(type);
        if (setting != NULL)
            CTA(ch->pcdata->colour[setting->index]);
    }

    /* if we STILL didn't find a code, use CLEAR. */
    if (code[0] == '\0')
        CTA(CC_CLEAR);

    /* return the number of characters written to buf_out. */
    return snprintf (buf_out, size, "%s", code);
}

int colour_puts (CHAR_T *ch, bool use_colour, const char *buf_in,
    char *buf_out, size_t size)
{
    const char *buf_start, *point;
    int skip;

    if (buf_in == NULL || buf_out == NULL || size <= 0)
        return 0;
    if (ch != NULL) {
        ch = REAL_CH (ch);
        use_colour = EXT_IS_SET (ch->ext_plr, PLR_COLOUR) ? TRUE : FALSE;
    }

    buf_start = buf_out;
    for (point = buf_in; size > 1 && *point; point++) {
        skip = 0;
        if (*point == '{') {
            skip = 1;
            point++;
        }
        if (!*point)
            break;

        if (!skip || *point == '\r' || *point == '\n') {
            size--;
            *(buf_out++) = *point;
            continue;
        }
        if (use_colour) {
            skip = colour_code_to_ansi (ch, use_colour, *point, buf_out, size);
            buf_out += skip;
            size -= skip;
        }
    }
    *buf_out = '\0';
    return buf_out - buf_start;
}

int colour_to_full_name (flag_t colour, char *buf_out, size_t size) {
    int i, len;
    if (buf_out == NULL || size <= 0)
        return 0;
    buf_out[0] = '\0';

    len = 0;
    for (i = 0; colour_table[i].name != NULL; i++) {
        const COLOUR_T *c = &(colour_table[i]);
        flag_t mask = colour & c->mask;
        if (mask == c->code) {
            len += snprintf (buf_out + len, size - len, "%s%s",
                (*buf_out) ? " " : "", c->name);
        }
    }
    return len;
}

const COLOUR_SETTING_T *colour_setting_get_by_char (char ch) {
    int i;
    for (i = 0; i < COLOUR_SETTING_MAX; i++)
        if (colour_setting_table[i].act_char == ch)
            return &(colour_setting_table[i]);
    return NULL;
}
