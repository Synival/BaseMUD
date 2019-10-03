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

#include <stdlib.h>
#include <time.h>
#include <string.h>

#if defined(unix)
    #include <signal.h>
#endif

#include "interp.h"
#include "comm.h"
#include "db.h"
#include "utils.h"
#include "signal.h"
#include "descs.h"
#include "signal.h"
#include "string.h"
#include "save.h"
#include "olc.h"
#include "nanny.h"
#include "update.h"

/* TODO: feature - better command-line arguments? */

#if defined(macintosh) || defined(MSDOS)
    void game_loop_mac_msdos (void);
#endif

#if defined(unix)
    void game_loop_unix (int control);
#endif

int main (int argc, char **argv) {
    struct timeval now_time;
    bool fCopyOver = FALSE;
    #ifdef IMC
        int imcsocket = -1;
    #endif

    /* Memory debugging if needed. */
    #if defined(MALLOC_DEBUG)
        malloc_debug (2);
    #endif

    /* Catch CTRL-C so we can do some manditory clean-up. */
    init_signal_handlers ();

    /* Init time.  */
    gettimeofday (&now_time, NULL);
    current_time = (time_t) now_time.tv_sec;
    strcpy (str_boot_time, ctime (&current_time));

    /* Macintosh console initialization. */
    #if defined(macintosh)
        console_options.nrows = 31;
        cshow (stdout);
        csetmode (C_RAW, stdin);
        cecho2file ("log file", 1, stderr);
    #endif

    /* Reserve one channel for our use. */
    if ((fpReserve = fopen (NULL_FILE, "r")) == NULL) {
        perror (NULL_FILE);
        exit (1);
    }

    /* Get the port number. */
    port = 4000;
    if (argc > 1) {
        if (!is_number (argv[1])) {
            fprintf (stderr, "Usage: %s [port #]\n", argv[0]);
            exit (1);
        }
        else if ((port = atoi (argv[1])) <= 1024) {
            fprintf (stderr, "Port number must be above 1024.\n");
            exit (1);
        }

        /* Are we recovering from a copyover? */
        if (argv[2] && argv[2][0]) {
            fCopyOver = TRUE;
            control = atoi (argv[3]);
            #ifdef IMC
                imcsocket = atoi (argv[4]);
            #endif
        }
        else
            fCopyOver = FALSE;
    }

    /* Run the game. */
    #if defined(macintosh) || defined(MSDOS)
        qmconfig_read(); /* Here because it fits, no conflicts with Linux placement -- JR 05/06/01 */
        boot_db ();
        log_string ("Merc is ready to rock.");
        game_loop_mac_msdos ();
    #endif

    #if defined(unix)
        qmconfig_read(); /* Here so we can set the IP adress. -- JR 05/06/01 */
        if (!fCopyOver)
            control = init_socket (port);
        boot_db ();
        log_f ("ROM is ready to rock on port %d (%s).", port, mud_ipaddress);

        #ifdef IMC
            /* Initialize and connect to IMC2 */
            imc_startup (FALSE, imcsocket, fCopyOver);
        #endif

        if (fCopyOver)
            copyover_recover ();

        game_loop_unix (control);
        close (control);
        #ifdef IMC
            imc_shutdown (FALSE);
        #endif
    #endif

    /* That's all, folks. */
    log_string ("Normal termination of game.");
    exit (0);
    return 0;
}

#if defined(macintosh) || defined(MSDOS)
void game_loop_mac_msdos (void) {
    struct timeval last_time;
    struct timeval now_time;
    static DESCRIPTOR_DATA dcon;

    gettimeofday (&last_time, NULL);
    current_time = (time_t) last_time.tv_sec;

    /* New_descriptor analogue. */
    dcon.descriptor = 0;
    if (!mud_ansiprompt)
        dcon.connected = CON_GET_NAME;
    else
        dcon.connected = CON_ANSI;
    dcon.ansi = mud_ansicolor;
    dcon.host = str_dup ("localhost");
    dcon.outsize = 2000;
    dcon.outbuf = alloc_mem (dcon.outsize);
    dcon.next = descriptor_list;
    dcon.showstr_head = NULL;
    dcon.showstr_point = NULL;
    dcon.pEdit = NULL;            /* OLC */
    dcon.pString = NULL;        /* OLC */
    dcon.editor = 0;            /* OLC */
    descriptor_list = &dcon;

    /* First Contact! */
    if (!mud_ansiprompt) {
        extern char * help_greeting;
        if ( help_greeting[0] == '.' )
            send_to_desc ( help_greeting+1, &dcon );
        else
            send_to_desc ( help_greeting  , &dcon );
    }
    else
        write_to_buffer (&dcon, "Do you want ANSI? (Y/n) ", 0);

    /* Main loop */
    while (!merc_down) {
        DESCRIPTOR_DATA *d;

        /* Process input. */
        for (d = descriptor_list; d != NULL; d = d_next) {
            d_next = d->next;
            d->fcommand = FALSE;

            #if defined(MSDOS)
                if (kbhit ())
            #endif
            {
                if (d->character != NULL)
                    d->character->timer = 0;
                if (!read_from_descriptor (d)) {
                    if (d->character != NULL && d->connected == CON_PLAYING)
                        save_char_obj (d->character);
                    d->outtop = 0;
                    close_socket (d);
                    continue;
                }
            }

            if (d->character != NULL && d->character->wait > 0)
                continue;

            read_from_buffer (d);
            if (d->incomm[0] != '\0') {
                d->fcommand = TRUE;
                stop_idling (d->character);

                /* OLC */
                if (d->showstr_point) {
                    d->lines_written = 0;
                    show_page (d);
                }
                else if (d->pString)
                    string_add (d->character, d->incomm);
                else {
                    switch (d->connected) {
                        case CON_PLAYING:
                            if (!run_olc_editor (d))
                                substitute_alias (d, d->incomm);
                            break;
                        default:
                            nanny (d, d->incomm);
                            break;
                    }
                }

                d->incomm[0] = '\0';
            }
        }

        /* Autonomous game motion.  */
        update_handler ();

        /* Output. */
        for (d = descriptor_list; d != NULL; d = d_next) {
            d_next = d->next;
            if ((d->fcommand || d->outtop > 0)) {
                if (!process_output (d, TRUE)) {
                    if (d->character != NULL && d->connected == CON_PLAYING)
                        save_char_obj (d->character);
                    d->outtop = 0;
                    close_socket (d);
                }
            }
        }

        /* Synchronize to a clock.
         * Busy wait (blargh). */
        now_time = last_time;
        while (1) {
            int delta;

            #if defined(MSDOS)
                if (kbhit ())
            #endif
            {
                if (dcon.character != NULL)
                    dcon.character->timer = 0;
                if (!read_from_descriptor (&dcon)) {
                    if (dcon.character != NULL && d->connected == CON_PLAYING)
                        save_char_obj (d->character);
                    dcon.outtop = 0;
                    close_socket (&dcon);
                }
                #if defined(MSDOS)
                    break;
                #endif
            }

            gettimeofday (&now_time, NULL);
            delta = (now_time.tv_sec - last_time.tv_sec) * 1000 * 1000
                + (now_time.tv_usec - last_time.tv_usec);
            if (delta >= (1000000 / PULSE_PER_SECOND) * 100 / PULSE_SPEED)
                break;
        }
        last_time = now_time;
        current_time = (time_t) last_time.tv_sec;
    }
}
#endif

#if defined(unix)
void game_loop_unix (int control) {
    static struct timeval null_time;
    struct timeval last_time;

    signal (SIGPIPE, SIG_IGN);
    gettimeofday (&last_time, NULL);
    current_time = (time_t) last_time.tv_sec;

    /* Main loop */
    while (!merc_down) {
        fd_set in_set;
        fd_set out_set;
        fd_set exc_set;
        DESCRIPTOR_DATA *d;
        int maxdesc;

#if defined(MALLOC_DEBUG)
        if (malloc_verify () != 1)
            abort ();
#endif

        /* Poll all active descriptors.  */
        FD_ZERO (&in_set);
        FD_ZERO (&out_set);
        FD_ZERO (&exc_set);
        FD_SET (control, &in_set);
        maxdesc = control;
        for (d = descriptor_list; d; d = d->next) {
            maxdesc = UMAX (maxdesc, d->descriptor);
            FD_SET (d->descriptor, &in_set);
            FD_SET (d->descriptor, &out_set);
            FD_SET (d->descriptor, &exc_set);
        }

        if (select (maxdesc + 1, &in_set, &out_set, &exc_set, &null_time) < 0) {
            perror ("Game_loop: select: poll");
            exit (1);
        }

        /* New connection? */
        if (FD_ISSET (control, &in_set))
            init_descriptor (control);

        /* Kick out the freaky folks. */
        for (d = descriptor_list; d != NULL; d = d_next) {
            d_next = d->next;
            if (FD_ISSET (d->descriptor, &exc_set)) {
                FD_CLR (d->descriptor, &in_set);
                FD_CLR (d->descriptor, &out_set);
                if (d->character && d->connected == CON_PLAYING)
                    save_char_obj (d->character);
                d->outtop = 0;
                close_socket (d);
            }
        }

        /* Process input. */
        for (d = descriptor_list; d != NULL; d = d_next) {
            d_next = d->next;
            d->fcommand = FALSE;

            if (FD_ISSET (d->descriptor, &in_set)) {
                if (d->character != NULL)
                    d->character->timer = 0;
                if (!read_from_descriptor (d)) {
                    FD_CLR (d->descriptor, &out_set);
                    if (d->character != NULL && d->connected == CON_PLAYING)
                        save_char_obj (d->character);
                    d->outtop = 0;
                    close_socket (d);
                    continue;
                }
            }

            if (d->character != NULL && d->character->wait > 0)
                continue;

            read_from_buffer (d);
            if (d->incomm[0] != '\0') {
                d->fcommand = TRUE;
                stop_idling (d->character);

                /* OLC */
                if (d->showstr_point) {
                    d->lines_written = 0;
                    show_page (d);
                }
                else if (d->pString)
                    string_add (d->character, d->incomm);
                else {
                    switch (d->connected) {
                        case CON_PLAYING:
                            if (!run_olc_editor (d))
                                substitute_alias (d, d->incomm);
                            break;
                        default:
                            nanny (d, d->incomm);
                            break;
                    }
                }
                d->incomm[0] = '\0';
            }
        }

        #ifdef IMC
            imc_loop();
        #endif

        /* Autonomous game motion. */
        update_handler ();

        /* Output. */
        for (d = descriptor_list; d != NULL; d = d_next) {
            d_next = d->next;

            if ((d->fcommand || d->outtop > 0)
                && FD_ISSET (d->descriptor, &out_set))
            {
                if (!process_output (d, TRUE)) {
                    if (d->character != NULL && d->connected == CON_PLAYING)
                        save_char_obj (d->character);
                    d->outtop = 0;
                    close_socket (d);
                }
            }
        }

        /* Synchronize to a clock.
         * Sleep( last_time + 1/PULSE_PER_SECOND - now ).
         * Careful here of signed versus unsigned arithmetic. */
        {
            struct timeval now_time;
            long secDelta;
            long usecDelta;

            gettimeofday (&now_time, NULL);
            usecDelta = ((int) last_time.tv_usec) - ((int) now_time.tv_usec)
                + ((1000000 / PULSE_PER_SECOND) * 100 / PULSE_SPEED);
            secDelta = ((int) last_time.tv_sec) - ((int) now_time.tv_sec);
            while (usecDelta < 0) {
                usecDelta += 1000000;
                secDelta -= 1;
            }

            while (usecDelta >= 1000000) {
                usecDelta -= 1000000;
                secDelta += 1;
            }

            if (secDelta > 0 || (secDelta == 0 && usecDelta > 0)) {
                struct timeval stall_time;

                stall_time.tv_usec = usecDelta;
                stall_time.tv_sec = secDelta;
                if (select (0, NULL, NULL, NULL, &stall_time) < 0) {
                    perror ("Game_loop: select: stall");
                    exit (1);
                }
            }
        }

        gettimeofday (&last_time, NULL);
        current_time = (time_t) last_time.tv_sec;
    }
}
#endif
