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
*    ROM 2.4 is copyright 1993-1998 Russ Taylor                             *
*    ROM has been brought to you by the ROM consortium                      *
*        Russ Taylor (rtaylor@hypercube.org)                                *
*        Gabrielle Taylor (gtaylor@hypercube.org)                           *
*        Brian Moore (zump@rom.org)                                         *
*    By using this code, you have agreed to follow the terms of the         *
*    ROM license, in the file Rom24/doc/rom.license                         *
****************************************************************************/

#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdarg.h>
#include <errno.h>
#include <unistd.h>

#if defined(unix)
    #include <signal.h>
    #include <fcntl.h>
    #include <netdb.h>
    #include <arpa/inet.h>
#endif

#include "ban.h"
#include "chars.h"
#include "colour.h"
#include "comm.h"
#include "db.h"
#include "recycle.h"
#include "utils.h"

#include "descs.h"

#if defined(unix)
int init_socket (int port) {
    static struct sockaddr_in sa_zero;
    struct sockaddr_in sa;
    int x = 1;
    int fd;

    if ((fd = socket (AF_INET, SOCK_STREAM, 0)) < 0) {
        perror ("Init_socket: socket");
        exit (1);
    }

    if (setsockopt (fd, SOL_SOCKET, SO_REUSEADDR,
                    (char *) &x, sizeof (x)) < 0)
    {
        perror ("Init_socket: SO_REUSEADDR");
        close (fd);
        exit (1);
    }

    #if defined(SO_DONTLINGER) && !defined(SYSV)
    {
        struct linger ld;

        ld.l_onoff = 1;
        ld.l_linger = 1000;

        if (setsockopt (fd, SOL_SOCKET, SO_DONTLINGER,
                        (char *) &ld, sizeof (ld)) < 0)
        {
            perror ("Init_socket: SO_DONTLINGER");
            close (fd);
            exit (1);
        }
    }
    #endif

    sa = sa_zero;
    sa.sin_family = AF_INET;
    sa.sin_port = htons (port);
    sa.sin_addr.s_addr = inet_addr( mud_ipaddress );
    log_f("Set IP address to %s", mud_ipaddress);

    if (bind (fd, (struct sockaddr *) &sa, sizeof (sa)) < 0) {
        perror ("Init socket: bind");
        close (fd);
        exit (1);
    }

    if (listen (fd, 3) < 0) {
        perror ("Init socket: listen");
        close (fd);
        exit (1);
    }

    return fd;
}
#endif

#if defined(unix)
void init_descriptor (int control) {
    char buf[MAX_STRING_LENGTH];
    DESCRIPTOR_DATA *dnew;
    struct sockaddr_in sock;
    struct hostent *from;
    size_t desc;
    socklen_t size = sizeof(struct sockaddr_in);

    getsockname (control, (struct sockaddr *) &sock, &size);
    if ((desc = accept (control, (struct sockaddr *) &sock, &size)) < 0) {
        perror ("New_descriptor: accept");
        return;
    }

    #if !defined(FNDELAY)
        #define FNDELAY O_NDELAY
    #endif

    if (fcntl (desc, F_SETFL, FNDELAY) == -1) {
        perror ("New_descriptor: fcntl: FNDELAY");
        return;
    }

    /* Cons a new descriptor. */
    dnew = descriptor_new ();
    dnew->descriptor = desc;
    if (!mud_ansiprompt)
        dnew->connected = CON_GET_NAME;
    else
        dnew->connected = CON_ANSI;
    dnew->ansi = mud_ansicolor;
    dnew->showstr_head = NULL;
    dnew->showstr_point = NULL;
    dnew->outsize = 2000;
    dnew->pEdit = NULL;            /* OLC */
    dnew->pString = NULL;        /* OLC */
    dnew->editor = 0;            /* OLC */
    dnew->outbuf = alloc_mem (dnew->outsize);

    size = sizeof (sock);
    if (getpeername (desc, (struct sockaddr *) &sock, &size) < 0) {
        perror ("New_descriptor: getpeername");
        dnew->host = str_dup ("(unknown)");
    }
    else {
        /* Would be nice to use inet_ntoa here but it takes a struct arg,
         * which ain't very compatible between gcc and system libraries. */
        int addr;

        addr = ntohl (sock.sin_addr.s_addr);
        sprintf (buf, "%d.%d.%d.%d",
                 (addr >> 24) & 0xFF, (addr >> 16) & 0xFF,
                 (addr >> 8) & 0xFF, (addr) & 0xFF);
        sprintf (log_buf, "Sock.sinaddr:  %s", buf);
        log_string (log_buf);
        from = gethostbyaddr ((char *) &sock.sin_addr,
                              sizeof (sock.sin_addr), AF_INET);
        dnew->host = str_dup (from ? from->h_name : buf);
    }

    /* Swiftest: I added the following to ban sites.  I don't
     * endorse banning of sites, but Copper has few descriptors now
     * and some people from certain sites keep abusing access by
     * using automated 'autodialers' and leaving connections hanging.
     *
     * Furey: added suffix check by request of Nickel of HiddenWorlds. */

    if (check_ban (dnew->host, BAN_ALL)) {
        write_to_descriptor (desc, "Your site has been banned from this mud.\n\r", 0);
        close (desc);
        descriptor_free (dnew);
        return;
    }

    /* Init descriptor data. */
    LIST_FRONT (dnew, next, descriptor_list);

    /* First Contact! */
    if (!mud_ansiprompt) {
        extern char *help_greeting;
        if (help_greeting[0] == '.')
            send_to_desc (help_greeting+1, dnew);
        else
            send_to_desc (help_greeting  , dnew);
    }
    else
        send_to_desc ("Do you want ANSI? (Y/n) ", dnew);
}
#endif

void close_socket (DESCRIPTOR_DATA * dclose) {
    CHAR_DATA *ch;

    if (dclose->outtop > 0)
        process_output (dclose, FALSE);
    if (dclose->snoop_by != NULL)
        write_to_buffer (dclose->snoop_by, "Your victim has left the game.\n\r", 0);

    {
        DESCRIPTOR_DATA *d;
        for (d = descriptor_list; d != NULL; d = d->next)
            if (d->snoop_by == dclose)
                d->snoop_by = NULL;
    }

    if ((ch = dclose->character) != NULL) {
        sprintf (log_buf, "Closing link to %s.", ch->name);
        log_string (log_buf);
        /* cut down on wiznet spam when rebooting */
        /* If ch is writing note or playing, just lose link otherwise clear char */
        if ((dclose->connected == CON_PLAYING && !merc_down)
                || ((dclose->connected >= CON_NOTE_TO)
                        && (dclose->connected <= CON_NOTE_FINISH)))
        {
            act ("$n has lost $s link.", ch, NULL, NULL, TO_NOTCHAR);
            wiznet ("Net death has claimed $N.", ch, NULL, WIZ_LINKS, 0, 0);
            ch->desc = NULL;
        }
        else
            char_free (dclose->original ? dclose->original : dclose->character);
    }

    if (d_next == dclose)
        d_next = d_next->next;

    LIST_REMOVE (dclose, next, descriptor_list, DESCRIPTOR_DATA, NO_FAIL);

    close (dclose->descriptor);
    descriptor_free (dclose);
    #if defined(MSDOS) || defined(macintosh)
        exit (1);
    #endif
}

bool read_from_descriptor (DESCRIPTOR_DATA * d) {
    int iStart;

    /* Hold horses if pending command already. */
    if (d->incomm[0] != '\0')
        return TRUE;

    /* Check for overflow. */
    iStart = strlen (d->inbuf);
    if (iStart >= sizeof (d->inbuf) - 10) {
        sprintf (log_buf, "%s input overflow!", d->host);
        log_string (log_buf);
        write_to_descriptor (d->descriptor,
            "\n\r*** PUT A LID ON IT!!! ***\n\r", 0);
        return FALSE;
    }

    /* Snarf input. */
    #if defined(macintosh)
    while (1) {
        int c;
        c = getc (stdin);
        if (c == '\0' || c == EOF)
            break;
        putc (c, stdout);
        if (c == '\r')
            putc ('\n', stdout);
        d->inbuf[iStart++] = c;
        if (iStart > sizeof (d->inbuf) - 10)
            break;
    }
    #endif

#if defined(MSDOS) || defined(unix)
    while (1) {
        int nRead;
        nRead = read (d->descriptor, d->inbuf + iStart,
                      sizeof (d->inbuf) - 10 - iStart);
        if (nRead > 0) {
            iStart += nRead;
            if (d->inbuf[iStart - 1] == '\n' || d->inbuf[iStart - 1] == '\r')
                break;
        }
        else if (nRead == 0) {
            log_string ("EOF encountered on read.");
            return FALSE;
        }
        else if (errno == EWOULDBLOCK)
            break;
        else {
            perror ("Read_from_descriptor");
            return FALSE;
        }
    }
#endif

    d->inbuf[iStart] = '\0';
    return TRUE;
}

/* Transfer one line from input buffer to input line. */
void read_from_buffer (DESCRIPTOR_DATA * d) {
    int i, j, k;

    /* Hold horses if pending command already. */
    if (d->incomm[0] != '\0')
        return;

    /* Look for at least one new line. */
    for (i = 0; d->inbuf[i] != '\n' && d->inbuf[i] != '\r'; i++)
        if (d->inbuf[i] == '\0')
            return;

    /* Canonical input processing. */
    for (i = 0, k = 0; d->inbuf[i] != '\n' && d->inbuf[i] != '\r'; i++) {
        if (k >= MAX_INPUT_LENGTH - 2) {
            write_to_descriptor (d->descriptor, "Line too long.\n\r", 0);

            /* skip the rest of the line */
            for (; d->inbuf[i] != '\0'; i++)
                if (d->inbuf[i] == '\n' || d->inbuf[i] == '\r')
                    break;

            d->inbuf[i] = '\n';
            d->inbuf[i + 1] = '\0';
            break;
        }

        if (d->inbuf[i] == '\b' && k > 0)
            --k;
        else if (isascii (d->inbuf[i]) && isprint (d->inbuf[i]))
            d->incomm[k++] = d->inbuf[i];
    }

    /* Finish off the line. */
    if (k == 0)
        d->incomm[k++] = ' ';
    d->incomm[k] = '\0';

    /* Deal with bozos with #repeat 1000 ... */
    if (k > 1 || d->incomm[0] == '!') {
        if (d->incomm[0] != '!' && strcmp (d->incomm, d->inlast))
            d->repeat = 0;
        else {
            if (++d->repeat >= 25 && d->character
                && d->connected == CON_PLAYING)
            {
                sprintf (log_buf, "%s input spamming!", d->host);
                log_string (log_buf);
                wiznet ("Spam spam spam $N spam spam spam spam spam!",
                        d->character, NULL, WIZ_SPAM, 0,
                        char_get_trust (d->character));
                if (d->incomm[0] == '!')
                    wiznet (d->inlast, d->character, NULL, WIZ_SPAM, 0,
                            char_get_trust (d->character));
                else
                    wiznet (d->incomm, d->character, NULL, WIZ_SPAM, 0,
                            char_get_trust (d->character));

                d->repeat = 0;
            }
        }
    }

    /* Do '!' substitution. */
    if (d->incomm[0] == '!')
        strcpy (d->incomm, d->inlast);
    else
        strcpy (d->inlast, d->incomm);

    /* Shift the input buffer. */
    while (d->inbuf[i] == '\n' || d->inbuf[i] == '\r')
        i++;
    for (j = 0; (d->inbuf[j] = d->inbuf[i + j]) != '\0'; j++)
        ;
}

/* Low level output function. */
bool process_output (DESCRIPTOR_DATA * d, bool fPrompt) {
    extern bool merc_down;

    /* Bust a prompt. */
    if (!merc_down) {
        if (d->showstr_point && *d->showstr_point == '\0') {
            d->lines_written = 0;
            clear_page (d);
        }

        if (d->showstr_point)
            write_to_buffer (d, "[Hit Return to continue] ", 0);
        else if (fPrompt && d->pString && d->connected == CON_PLAYING)
            write_to_buffer (d, "> ", 2);
        else if (fPrompt && d->connected == CON_PLAYING) {
            CHAR_DATA *ch;
            CHAR_DATA *victim;

            ch = d->character;

            /* battle prompt */
            if ((victim = ch->fighting) != NULL && char_can_see_in_room (ch, victim)) {
                int percent;
                char *pbuff;
                char buf[MSL];
                char buffer[MSL*2];

                if (victim->max_hit > 0)
                    percent = victim->hit * 100 / victim->max_hit;
                else
                    percent = -1;

                sprintf (buf, "%s %s.\n\r", PERS_IR (victim, ch),
                    condition_string(percent));
                buf[0] = UPPER (buf[0]);
                pbuff = buffer;
                colour_puts (CH(d), d->ansi, buf, pbuff, MAX_STRING_LENGTH);
                write_to_buffer (d, buffer, 0);
            }

            ch = d->original ? d->original : d->character;
            if (!IS_SET (ch->comm, COMM_COMPACT))
                write_to_buffer (d, "\n\r", 2);
            if (IS_SET (ch->comm, COMM_PROMPT))
                bust_a_prompt (d->character);
            if (IS_SET (ch->comm, COMM_TELNET_GA))
                write_to_buffer (d, go_ahead_str, 0);
        }
    }

    /* Short-circuit if nothing to write. */
    if (d->outtop == 0)
        return TRUE;

    /* Snoop-o-rama. */
    if (d->snoop_by != NULL) {
        if (d->character != NULL)
            write_to_buffer (d->snoop_by, d->character->name, 0);
        write_to_buffer (d->snoop_by, "> ", 2);
        write_to_buffer (d->snoop_by, d->outbuf, d->outtop);
    }

    /* OS-dependent output. */
    if (!write_to_descriptor (d->descriptor, d->outbuf, d->outtop)) {
        d->outtop = 0;
        return FALSE;
    }
    else {
        d->outtop = 0;
        return TRUE;
    }
}

/* Append onto an output buffer. */
void write_to_buffer (DESCRIPTOR_DATA * d, const char *txt, int length) {
    /* Find length in case caller didn't. */
    if (length <= 0)
        length = strlen (txt);

    /* Initial \n\r if needed. */
    if (d->outtop == 0 && !d->fcommand) {
        d->outbuf[0] = '\n';
        d->outbuf[1] = '\r';
        d->outtop = 2;
    }

    /* Expand the buffer as needed. */
    while (d->outtop + length >= d->outsize) {
        char *outbuf;

        if (d->outsize >= 32000) {
            bug ("Buffer overflow. Closing.\n\r", 0);
            close_socket (d);
            return;
        }
        outbuf = alloc_mem (2 * d->outsize);
        strncpy (outbuf, d->outbuf, d->outtop);
        mem_free (d->outbuf, d->outsize);
        d->outbuf = outbuf;
        d->outsize *= 2;
    }

    /* Copy. */
    strncpy (d->outbuf + d->outtop, txt, length);
    d->outtop += length;
}

/* Lowest level output function.
 * Write a block of text to the file descriptor.
 * If this gives errors on very long blocks (like 'ofind all'),
 *   try lowering the max block size. */
bool write_to_descriptor (int desc, char *txt, int length) {
    int iStart;
    int nWrite;
    int nBlock;

#if defined(macintosh) || defined(MSDOS)
    if (desc == 0)
        desc = 1;
#endif

    if (length <= 0)
        length = strlen (txt);

    for (iStart = 0; iStart < length; iStart += nWrite) {
        nBlock = UMIN (length - iStart, 4096);
        if ((nWrite = write (desc, txt + iStart, nBlock)) < 0) {
            perror ("Write_to_descriptor");
            return FALSE;
        }
    }

    return TRUE;
}

/* Look for link-dead player to reconnect. */
bool check_reconnect (DESCRIPTOR_DATA * d, char *name, bool fConn) {
    CHAR_DATA *ch;

    for (ch = char_list; ch != NULL; ch = ch->next) {
        if (!IS_NPC (ch)
            && (!fConn || ch->desc == NULL)
            && !str_cmp (d->character->name, ch->name))
        {
            if (fConn == FALSE) {
                str_free (d->character->pcdata->pwd);
                d->character->pcdata->pwd = str_dup (ch->pcdata->pwd);
            }
            else {
                char_free (d->character);
                d->character = ch;
                ch->desc = d;
                ch->timer = 0;
                send_to_char ("Reconnecting. Type replay to see missed tells.\n\r", ch);
                act ("$n has reconnected.", ch, NULL, NULL, TO_NOTCHAR);

                sprintf (log_buf, "%s@%s reconnected.", ch->name, d->host);
                log_string (log_buf);
                wiznet ("$N groks the fullness of $S link.",
                        ch, NULL, WIZ_LINKS, 0, 0);
                d->connected = CON_PLAYING;

                /* Inform the character of a note in progress and the possbility
                 * of continuation! */
                if (ch->pcdata->in_progress)
                    send_to_char ("You have a note in progress. Type NWRITE to continue it.\n\r", ch);
            }
            return TRUE;
        }
    }

    return FALSE;
}

/* Check if already playing. */
bool check_playing (DESCRIPTOR_DATA * d, char *name) {
    DESCRIPTOR_DATA *dold;

    for (dold = descriptor_list; dold; dold = dold->next) {
        if (dold != d
            && dold->character != NULL
            && dold->connected != CON_GET_NAME
            && dold->connected != CON_GET_OLD_PASSWORD
            && !str_cmp (name, dold->original
                         ? dold->original->name : dold->character->name))
        {
            write_to_buffer (d, "That character is already playing.\n\r", 0);
            write_to_buffer (d, "Do you wish to connect anyway (Y/N)?", 0);
            d->connected = CON_BREAK_CONNECT;
            return TRUE;
        }
    }

    return FALSE;
}

/* Page to one descriptor using Lope's color. */
void send_to_desc (const char *txt, DESCRIPTOR_DATA * d) {
    char buf[MAX_STRING_LENGTH * 4];
    int len;

    if (txt == NULL || d == NULL)
        return;
    buf[0] = '\0';
    len = colour_puts (NULL, d->ansi, txt, buf, sizeof(buf));
    write_to_buffer (d, buf, len);
}

void clear_page (DESCRIPTOR_DATA *d) {
    if (d->showstr_head) {
        mem_free (d->showstr_head, strlen (d->showstr_head));
        d->showstr_head = NULL;
    }
    d->showstr_point = NULL;
}

void append_to_page (DESCRIPTOR_DATA *d, const char *txt) {
    int len;
    if (d == NULL || txt == NULL)
        return;

    len = strlen (txt);
    if (d->showstr_head == NULL) {
        d->showstr_head  = alloc_mem (len + 1);
        d->showstr_point = d->showstr_head;
        strcpy (d->showstr_head, txt);
    }
    else {
        int offset = d->showstr_point - d->showstr_head;
        char *new_buf = alloc_mem (strlen (d->showstr_head) + len + 1);
        strcpy (new_buf, d->showstr_head);
        strcat (new_buf, txt);

        clear_page (d);
        d->showstr_head = new_buf;
        d->showstr_point = d->showstr_head + offset;
        strcpy (d->showstr_head, txt);
    }

    /* write what we can immediately. */
    show_page (d);
}

/* string pager */
int show_page (DESCRIPTOR_DATA *d) {
    char buffer[4 * MAX_STRING_LENGTH];
    register char *scan;
    int crlf = 0;
    int show_lines;

    show_lines = d->character ? d->character->lines : 0;
    for (scan = buffer; *d->showstr_point != '\0';
         scan++, d->showstr_point++)
    {
        if (show_lines != 0 && d->lines_written >= show_lines)
            break;
        *scan = *d->showstr_point;
        if (*scan == '\n') crlf |= 0x01;
        if (*scan == '\r') crlf |= 0x02;
        if (crlf == 0x03) {
            crlf = 0;
            d->lines_written++;
        }
    }
    *scan = '\0';
    write_to_buffer (d, buffer, strlen (buffer));
    return (*d->showstr_point) == '\0';
}

/* source: EOD, by John Booth <???> */
void printf_to_desc (DESCRIPTOR_DATA * d, char *fmt, ...) {
    char buf[MSL];
    va_list args;
    va_start (args, fmt);
    vsprintf (buf, fmt, args);
    va_end (args);

    send_to_desc (buf, d);
}
