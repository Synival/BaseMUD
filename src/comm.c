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
 *  Thanks to abaddon for proof-reading our comm.c and pointing out bugs.  *
 *  Any remaining bugs are, of course, our work, not his.  :)              *
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
****************************************************************************/

/* This file contains all of the OS-dependent stuff:
 *   startup, signals, BSD sockets for tcp/ip, i/o, timing.
 *
 * The data flow for input is:
 *    Game_loop ---> Read_from_descriptor ---> Read
 *    Game_loop ---> Read_from_buffer
 *
 * The data flow for output is:
 *    Game_loop ---> Process_Output ---> Write_to_descriptor -> Write
 *
 * The OS-dependent functions are Read_from_descriptor and Write_to_descriptor.
 * -- Furey  26 Jan 1993 */

#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

#include "colour.h"
#include "recycle.h"
#include "utils.h"
#include "fight.h"
#include "interp.h"
#include "db.h"
#include "olc.h"
#include "save.h"
#include "mob_prog.h"
#include "lookup.h"
#include "act_info.h"
#include "chars.h"
#include "rooms.h"
#include "objs.h"
#include "descs.h"
#include "globals.h"

#include "comm.h"

/* Malloc debugging stuff. */
#if defined(sun)
    #undef MALLOC_DEBUG
#endif

#if defined(MALLOC_DEBUG)
    #include <malloc.h>
    extern int malloc_debug args ((int));
    extern int malloc_verify args ((void));
#endif

/* Bust a prompt (player settable prompt)
 * coded by Morgenes for Aldara Mud */
void bust_a_prompt (CHAR_T *ch) {
    char buf[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    const char *str;
    const char *i;
    char *point;
    char *pbuff;
    char buffer[MAX_STRING_LENGTH * 2];

    if (ch == NULL || ch->desc == NULL)
        return;

#ifdef BASEMUD_SHOW_OLC_IN_PROMPT
    if (ch->desc->editor != ED_NONE)
        printf_to_char (ch, "[%s %s] ", olc_ed_name (ch), olc_ed_vnum (ch));
#endif

    point = buf;
    str = ch->prompt;
    if (str == NULL || str[0] == '\0') {
        printf_to_char (ch, "{p<%dhp %dm %dmv>{x %s",
            ch->hit, ch->mana, ch->move, ch->prefix);
        return;
    }

    if (IS_SET (ch->comm, COMM_AFK)) {
        send_to_char ("{p<AFK>{x ", ch);
        return;
    }

    while (*str != '\0') {
        if (*str != '%') {
            *point++ = *str++;
            continue;
        }
        ++str;
        switch (*str) {
            case '%': buf2[0] = '%'; buf2[1] = '\0';           i = buf2; break;
            case 'c': sprintf (buf2, "%s",  "\n\r");           i = buf2; break;
            case 'h': sprintf (buf2, "%d",  ch->hit);          i = buf2; break;
            case 'H': sprintf (buf2, "%d",  ch->max_hit);      i = buf2; break;
            case 'm': sprintf (buf2, "%d",  ch->mana);         i = buf2; break;
            case 'M': sprintf (buf2, "%d",  ch->max_mana);     i = buf2; break;
            case 'v': sprintf (buf2, "%d",  ch->move);         i = buf2; break;
            case 'V': sprintf (buf2, "%d",  ch->max_move);     i = buf2; break;
            case 'x': sprintf (buf2, "%d",  ch->exp);          i = buf2; break;
            case 'g': sprintf (buf2, "%ld", ch->gold);         i = buf2; break;
            case 's': sprintf (buf2, "%ld", ch->silver);       i = buf2; break;
            case 'o': sprintf (buf2, "%s",  olc_ed_name (ch)); i = buf2; break;
            case 'O': sprintf (buf2, "%s",  olc_ed_vnum (ch)); i = buf2; break;

            case 'e':
                char_exit_string (ch, ch->in_room, EXITS_PROMPT,
                    buf2, sizeof (buf2));
                i = buf2;
                break;

            case 'X':
                sprintf (buf2, "%d", IS_NPC (ch) ? 0 : (ch->level + 1)
                    * exp_per_level (ch, ch->pcdata->points) - ch->exp);
                i = buf2;
                break;

            case 'a':
                if (ch->level > 9)
                    sprintf (buf2, "%d", ch->alignment);
                else {
                    sprintf (buf2, "%s",
                        IS_GOOD (ch) ? "good" :
                        IS_EVIL (ch) ? "evil" :
                                       "neutral");
                }
                i = buf2;
                break;

            case 'r':
                if (ch->in_room != NULL) {
                    sprintf (buf2, "%s",
                        ((!IS_NPC (ch) && IS_SET (ch->plr, PLR_HOLYLIGHT)) ||
                         (!IS_AFFECTED (ch, AFF_BLIND) &&
                             !room_is_dark (ch->in_room))
                        ) ? ch->in_room-> name : "darkness"
                    );
                    i = buf2;
                }
                else
                    i = " ";
                break;

            case 'R':
                if (IS_IMMORTAL (ch) && ch->in_room != NULL) {
                    sprintf (buf2, "%d", ch->in_room->vnum);
                    i = buf2;
                }
                else
                    i = " ";
                break;

            case 'z':
                if (IS_IMMORTAL (ch) && ch->in_room != NULL)
                    sprintf (buf2, "%s", ch->in_room->area->title);
                else
                    sprintf (buf2, " ");
                i = buf2;
                break;

            case 'p':
                sprintf (buf2, "%s%s", ch->fighting ? "!" : "",
                    char_get_position_str (ch, ch->position, ch->on, FALSE));
                i = buf2;
                break;

            default:
                i = " ";
                break;
        }
        ++str;
        while ((*point = *i) != '\0')
            ++point, ++i;
    }

    *point = '\0';
    pbuff = buffer;
    colour_puts (ch, ch->desc->ansi, buf, pbuff, MAX_STRING_LENGTH);
    send_to_char ("{p", ch);
    write_to_buffer (ch->desc, buffer, 0);
    send_to_char ("{x", ch);

    if (ch->prefix[0] != '\0')
        write_to_buffer (ch->desc, ch->prefix, 0);
}

/* Write to one char. */
void send_to_char_bw (const char *txt, CHAR_T *ch) {
    if (txt == NULL || ch == NULL || ch->desc == NULL)
        return;
    write_to_buffer (ch->desc, txt, strlen (txt));
}

/* Send a page to one char. */
void page_to_char_bw (const char *txt, CHAR_T *ch) {
    if (txt == NULL || ch == NULL || ch->desc == NULL)
        return;
    if (ch->lines == 0) {
        send_to_char_bw (txt, ch);
        return;
    }

#if defined(macintosh)
    send_to_char_bw (txt, ch);
#else
    append_to_page (ch->desc, txt);
#endif
}

/* Page to one char, new colour version, by Lope. */
void send_to_char (const char *txt, CHAR_T *ch) {
    char buf[MAX_STRING_LENGTH * 4];
    int len;

    if (txt == NULL || ch == NULL || ch->desc == NULL)
        return;
    buf[0] = '\0';
    len = colour_puts (ch, ch->desc->ansi, txt, buf, sizeof(buf));
    write_to_buffer (ch->desc, buf, len);
}

void page_to_char (const char *txt, CHAR_T *ch) {
#if !defined(macintosh)
    char buf[MAX_STRING_LENGTH * 4];
#endif

    if (txt == NULL || ch == NULL || ch->desc == NULL)
        return;
    if (ch->lines == 0) {
        send_to_char (txt, ch);
        return;
    }

#if defined(macintosh)
    send_to_char (txt, ch);
#else
    buf[0] = '\0';
    colour_puts (ch, ch->desc->ansi, txt, buf, sizeof(buf));
    append_to_page (ch->desc, buf);
#endif
}

void act2 (const char *to_char, const char *to_room, CHAR_T *ch,
           const void *arg1, const void *arg2, flag_t flags, int min_pos)
{
    if (to_char)
        act_new (to_char, ch, arg1, arg2, flags | TO_CHAR, min_pos);
    if (to_room)
        act_new (to_room, ch, arg1, arg2, flags | TO_NOTCHAR, min_pos);
}

void act3 (const char *to_char, const char *to_vict, const char *to_room,
           CHAR_T *ch, const void *arg1, const void *arg2, flag_t flags,
           int min_pos)
{
    if (to_char)
        act_new (to_char, ch, arg1, arg2, flags | TO_CHAR, min_pos);
    if (to_vict && arg2) /* arg2 represents the victim */
        act_new (to_vict, ch, arg1, arg2, flags | TO_VICT, min_pos);
    if (to_room)
        act_new (to_room, ch, arg1, arg2, flags | TO_OTHERS, min_pos);
}

bool act_is_valid_recipient (CHAR_T *to, flag_t flags,
    CHAR_T *ch, CHAR_T *vch)
{
    if ((flags & TO_CHAR) && to == ch)
        return TRUE;
    if ((flags & TO_VICT) && to == vch && to != ch)
        return TRUE;
    if ((flags & TO_OTHERS) && (to != ch && to != vch))
        return TRUE;
    return FALSE;
}

char *act_code_pronoun (const CHAR_T *ch, char code) {
    static char *const he_she[]  = { "it",  "he",  "she" };
    static char *const him_her[] = { "it",  "him", "her" };
    static char *const his_her[] = { "its", "his", "her" };

    switch (code) {
        case 'e': case 'E': return he_she [URANGE (0, ch->sex, 2)];
        case 'm': case 'M': return him_her[URANGE (0, ch->sex, 2)];
        case 's': case 'S': return his_her[URANGE (0, ch->sex, 2)];
        default:            return "???";
    }
}

char *act_code (char code, CHAR_T *ch, CHAR_T *vch, CHAR_T *to,
    OBJ_T *obj1, OBJ_T *obj2, const void *arg1, const void *arg2,
    char *out_buf, size_t size)
{
    #define FILTER_BAD_CODE(true_cond, message) \
        do { \
            RETURN_IF_BUG (!(true_cond), \
                "act: " message, 0, " <@@@> "); \
        } while (0)

    switch (code) {
        /* Added checking of pointers to each case after reading about the
         * bug on Edwin's page. JR -- 10/15/00 */

        /* Thx alex for 't' idea */
        case 't':
            FILTER_BAD_CODE (arg1, "bad code $t for 'arg1'");
            return (char *) arg1;
        case 'T':
            FILTER_BAD_CODE (arg2, "bad code $T for 'arg2'");
            return (char *) arg2;
        case 'n':
            FILTER_BAD_CODE (ch && to, "bad code $n for 'ch' or 'to'");
            return PERS_AW (ch, to);
        case 'N':
            FILTER_BAD_CODE (vch && to, "bad code $N for 'vch' or 'to'");
            return PERS_AW (vch, to);
        case 'e':
            FILTER_BAD_CODE (ch, "bad code $e for 'ch'");
            return act_code_pronoun (ch, 'e');
        case 'E':
            FILTER_BAD_CODE (vch, "bad code $E for 'vch'");
            return act_code_pronoun (vch, 'E');
        case 'm':
            FILTER_BAD_CODE (ch, "bad code $m for 'ch'");
            return act_code_pronoun (ch, 'm');
        case 'M':
            FILTER_BAD_CODE (vch, "bad code $M for 'vch'");
            return act_code_pronoun (vch, 'm');
        case 's':
            FILTER_BAD_CODE (ch, "bad code $s for 'ch'");
            return act_code_pronoun (ch, 's');
        case 'S':
            FILTER_BAD_CODE (vch, "bad code $S for 'vch'");
            return act_code_pronoun (vch, 's');
        case 'p':
            FILTER_BAD_CODE (to && obj1, "bad code $p for 'to' or 'obj1'");
            return char_can_see_obj (to, obj1) ? obj1->short_descr : "something";
        case 'P':
            FILTER_BAD_CODE (to && obj2, "bad code $P for 'to' or 'obj2'");
            return char_can_see_obj (to, obj2) ? obj2->short_descr : "something";
        case 'd':
            return room_get_door_name ((char *) arg2, out_buf, size);

        default:
            bug ("bad code %d.", code);
            return " <@@@> ";
    }
}

void act_new (const char *format, CHAR_T *ch, const void *arg1,
              const void *arg2, flag_t flags, int min_pos)
{
    char buf[MAX_STRING_LENGTH];
    char code_buf[MAX_INPUT_LENGTH];
    CHAR_T *to;
    CHAR_T *vch = (CHAR_T *) arg2;
    OBJ_T *obj1 = (OBJ_T *) arg1;
    OBJ_T *obj2 = (OBJ_T *) arg2;
    const char *str;
    const char *i;
    char *point;
    char *pbuff;
    char buffer[MSL * 2];

    /* Discard null and zero-length messages. */
    if (format == NULL || format[0] == '\0')
        return;

    /* discard null rooms and chars */
    if (ch == NULL || ch->in_room == NULL)
        return;

    to = ch->in_room->people;
    if (flags == TO_VICT) {
        BAIL_IF_BUG (vch == NULL,
            "act: null vch with TO_VICT.", 0);
        if (vch->in_room == NULL)
            return;
        to = vch->in_room->people;
    }

    for (; to != NULL; to = to->next_in_room) {
        if (!IS_NPC (to) && to->desc == NULL)
            continue;
        if (IS_NPC (to) && to->desc == NULL && !HAS_TRIGGER (to, TRIG_ACT))
            continue;
        if (to->position < min_pos)
            continue;
        if (!act_is_valid_recipient (to, flags, ch, vch))
            continue;

        point = buf;
        str = format;
        while (*str != '\0') {
            if (*str != '$') {
                *point++ = *str++;
                continue;
            }
            ++str;
            i = " <@@@> ";

            if (arg2 == NULL && *str >= 'A' && *str <= 'Z')
                bug ("act: missing arg2 for code %d.", *str);
            else
                i = act_code (*str, ch, vch, to, obj1, obj2, arg1, arg2,
                    code_buf, sizeof (code_buf));

            ++str;
            while ((*point = *i) != '\0')
                ++point, ++i;
        }

        *point++ = '\n';
        *point++ = '\r';
        *point = '\0';

        /* Kludge to capitalize first letter of buffer, trying
         * to account for { color codes. -- JR 09/09/00 */
        if (buf[0] == '{' && buf[1] != '{')
            buf[2] = UPPER (buf[2]);
        else
            buf[0] = UPPER (buf[0]);
        pbuff = buffer;
        colour_puts (to, to->desc ? to->desc->ansi : 0,
            buf, pbuff, MAX_STRING_LENGTH);
        if (to->desc && (to->desc->connected == CON_PLAYING))
            write_to_buffer (to->desc, buffer, 0); /* changed to buffer to reflect prev. fix */
        else if (trigger_mobs)
            mp_act_trigger (buf, to, ch, arg1, arg2, TRIG_ACT);
    }
}

void printf_to_char (CHAR_T *ch, const char *fmt, ...) {
    char buf[MAX_STRING_LENGTH];
    va_list args;
    va_start (args, fmt);
    vsnprintf (buf, sizeof(buf), fmt, args);
    va_end (args);

    send_to_char (buf, ch);
}

void wiznet (const char *string, CHAR_T *ch, OBJ_T *obj,
             flag_t flag, flag_t flag_skip, int min_level)
{
    DESCRIPTOR_T *d;

    for (d = descriptor_list; d != NULL; d = d->next) {
        if (d->connected == CON_PLAYING && IS_IMMORTAL (d->character)
            && IS_SET (d->character->wiznet, WIZ_ON)
            && (!flag || IS_SET (d->character->wiznet, flag))
            && (!flag_skip || !IS_SET (d->character->wiznet, flag_skip))
            && char_get_trust (d->character) >= min_level && d->character != ch)
        {
            if (IS_SET (d->character->wiznet, WIZ_PREFIX))
                send_to_char ("{Z--> ", d->character);
            else
                send_to_char ("{Z", d->character);
            act_new (string, d->character, obj, ch, TO_CHAR, POS_DEAD);
            send_to_char ("{x", d->character);
        }
    }
}

void wiznetf (CHAR_T *ch, OBJ_T *obj, flag_t flag, flag_t flag_skip,
    int min_level, const char *fmt, ...)
{
    char buf[2 * MSL];
    va_list args;
    va_start (args, fmt);
    vsnprintf (buf, sizeof(buf), fmt, args);
    va_end (args);
    wiznet (buf, ch, obj, flag, flag_skip, min_level);
}

bool position_change_send_message (CHAR_T *ch, int from, int to,
    OBJ_T *obj)
{
    switch (to) {
        case POS_SLEEPING:
            return position_change_send_message_to_sleeping (ch, from, obj);
        case POS_RESTING:
            return position_change_send_message_to_resting (ch, from, obj);
        case POS_SITTING:
            return position_change_send_message_to_sitting (ch, from, obj);
        case POS_STANDING:
            return position_change_send_message_to_standing (ch, from, obj);
        case POS_FIGHTING:
            return position_change_send_message_to_fighting (ch, from, obj);
    }
    return FALSE;
}

bool position_change_send_message_to_standing (CHAR_T *ch, int from,
    OBJ_T *obj)
{
    const char *prep = obj_furn_preposition (obj, POS_STANDING);
    switch (from) {
        case POS_SLEEPING:
            if (obj == NULL) {
                send_to_char ("You wake and stand up.\n\r", ch);
                act ("$n wakes and stands up.", ch, NULL, NULL, TO_NOTCHAR);
            }
            else {
                act_new ("You wake and stand $T $p.", ch, obj, prep, TO_CHAR, POS_DEAD);
                act ("$n wakes and stands $T $p.", ch, obj, prep, TO_NOTCHAR);
            }
            return TRUE;

        case POS_RESTING:
            if (obj == NULL) {
                send_to_char ("You stop resting and stand up.\n\r", ch);
                act ("$n stops resting and stands up.", ch, NULL, NULL, TO_NOTCHAR);
            }
            else {
                act ("You stop resting and stand $T $p.", ch, obj, prep, TO_CHAR);
                act ("$n stops resting and stands $T $p.", ch, obj, prep, TO_NOTCHAR);
            }
            return TRUE;

        case POS_SITTING:
            if (obj == NULL) {
                send_to_char ("You stop sitting and stand up.\n\r", ch);
                act ("$n stops sitting and stands up.", ch, NULL, NULL, TO_NOTCHAR);
            }
            else {
                act ("You stop sitting and stand $T $p.", ch, obj, prep, TO_CHAR);
                act ("$n stops sitting and stands $T $p.", ch, obj, prep, TO_NOTCHAR);
            }
            return TRUE;

        case POS_STANDING:
            if (ch->on == obj)
                break;
            if (obj == NULL && ch->on != NULL) {
                prep = obj_furn_preposition_base (ch->on, POS_STANDING,
                    "away from", "off of", "outside of", "away from");
                act ("You step $T $p.", ch, ch->on, prep, TO_CHAR);
                act ("$n steps $T $p.", ch, ch->on, prep, TO_NOTCHAR);
            }
            else if (obj != NULL) {
                prep = obj_furn_preposition_base (obj, POS_STANDING,
                    "toward", "onto", "inside", "beside");
                act ("You step $T $p.", ch, obj, prep, TO_CHAR);
                act ("$n steps $T $p.", ch, obj, prep, TO_NOTCHAR);
            }
            else
                return FALSE;
            return TRUE;
    }
    return FALSE;
}

bool position_change_send_message_to_fighting (CHAR_T *ch, int from,
    OBJ_T *obj)
{
    switch (from) {
        case POS_SLEEPING:
            send_to_char ("You wake up, stand up, and fight!\n\r", ch);
            act ("$n wakes up, stands up, and fights!", ch, NULL, NULL, TO_NOTCHAR);
            return TRUE;

        case POS_RESTING:
            send_to_char ("You stop resting, stand up, and fight!\n\r", ch);
            act ("$n stops resting, stands up, and fights!", ch, NULL, NULL, TO_NOTCHAR);
            return TRUE;

        case POS_SITTING:
            send_to_char ("You stand up and fight!\n\r", ch);
            act ("$n stands up and fights!", ch, NULL, NULL, TO_NOTCHAR);
            return TRUE;
    }
    return FALSE;
}

bool position_change_send_message_to_resting (CHAR_T *ch, int from,
    OBJ_T *obj)
{
    const char *prep = obj_furn_preposition (obj, POS_RESTING);
    switch (from) {
        case POS_SLEEPING:
            if (obj == NULL) {
                send_to_char ("You wake up and start resting.\n\r", ch);
                act ("$n wakes up and starts resting.", ch, NULL, NULL, TO_NOTCHAR);
            }
            else {
                act_new ("You wake up and rest $T $p.", ch, obj, prep, TO_CHAR, POS_SLEEPING);
                act ("$n wakes up and rests $T $p.", ch, obj, prep, TO_NOTCHAR);
            }
            return TRUE;

        case POS_SITTING:
            if (obj == NULL) {
                send_to_char ("You rest.\n\r", ch);
                act ("$n rests.", ch, NULL, NULL, TO_NOTCHAR);
            }
            else {
                act ("You rest $T $p.", ch, obj, prep, TO_CHAR);
                act ("$n rests $T $p.", ch, obj, prep, TO_NOTCHAR);
            }
            return TRUE;

        case POS_STANDING:
            if (obj == NULL) {
                send_to_char ("You sit down and rest.\n\r", ch);
                act ("$n sits down and rests.", ch, NULL, NULL, TO_NOTCHAR);
            }
            else {
                act ("You rest $T $p.", ch, obj, prep, TO_CHAR);
                act ("$n rests $T $p.", ch, obj, prep, TO_NOTCHAR);
            }
            return TRUE;
    }
    return FALSE;
}

bool position_change_send_message_to_sitting (CHAR_T *ch, int from,
    OBJ_T *obj)
{
    const char *prep = obj_furn_preposition (obj, POS_RESTING);
    switch (from) {
        case POS_SLEEPING:
            if (obj == NULL) {
                send_to_char ("You wake and sit up.\n\r", ch);
                act ("$n wakes and sits up.", ch, NULL, NULL, TO_NOTCHAR);
            }
            else {
                act_new ("You wake and sit $T $p.", ch, obj, prep, TO_CHAR, POS_DEAD);
                act ("$n wakes and sits $T $p.", ch, obj, prep, TO_NOTCHAR);
            }
            return TRUE;

        case POS_RESTING:
            if (obj == NULL)
                send_to_char ("You stop resting.\n\r", ch);
            else {
                act_new ("You sit $T $p.", ch, obj, prep, TO_CHAR, POS_DEAD);
                act ("$n sits $T $p.", ch, obj, prep, TO_NOTCHAR);
            }
            return TRUE;

        case POS_STANDING:
            if (obj == NULL) {
                send_to_char ("You sit down.\n\r", ch);
                act ("$n sits down on the ground.", ch, NULL, NULL, TO_NOTCHAR);
            }
            else {
                act ("You sit down $T $p.", ch, obj, prep, TO_CHAR);
                act ("$n sits down $T $p.", ch, obj, prep, TO_NOTCHAR);
            }
            return TRUE;
    }
    return FALSE;
}

bool position_change_send_message_to_sleeping (CHAR_T *ch, int from,
    OBJ_T *obj)
{
    const char *prep = obj_furn_preposition (obj, POS_SLEEPING);
    switch (from) {
        case POS_RESTING:
        case POS_SITTING:
        case POS_STANDING:
            if (obj == NULL) {
                send_to_char ("You lie down and go to sleep.\n\r", ch);
                act ("$n lies down and goes to sleep.", ch, NULL, NULL, TO_NOTCHAR);
            }
            else {
                act ("You lie down and go to sleep $T $p.", ch, obj, prep, TO_CHAR);
                act ("$n lies down and goes to sleep $T $p.", ch, obj, prep, TO_NOTCHAR);
            }
            return TRUE;
    }
    return FALSE;
}

void echo_to_char (CHAR_T *to, CHAR_T *from, const char *type,
    const char *msg)
{
    if (char_get_trust (to) >= char_get_trust (from)) {
        send_to_char (type, to);
        send_to_char ("> ", to);
    }
    send_to_char (msg, to);
    send_to_char ("\n\r", to);
}
