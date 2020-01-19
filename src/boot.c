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
 ***************************************************************************/

#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <errno.h>

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
#include "chars.h"
#include "descs.h"

#include "recycle.h"
#include "act_info.h"
#include "globals.h"
#include "memory.h"

#include "boot.h"

int main (int argc, char **argv) {
    struct timeval now_time;
    bool copyover = FALSE;
    int free_count;
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
    strcpy (str_boot_time, ctime_fixed (&current_time));

    /* Macintosh console initialization. */
    #if defined(macintosh)
        console_options.nrows = 31;
        cshow (stdout);
        csetmode (C_RAW, stdin);
        cecho2file ("log file", 1, stderr);
    #endif

    /* Reserve one channel for our use. */
    if ((reserve_file = fopen (NULL_FILE, "r")) == NULL) {
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
            copyover = TRUE;
            control = atoi (argv[3]);
            #ifdef IMC
                imcsocket = atoi (argv[4]);
            #endif
        }
        else
            copyover = FALSE;
    }

    /* Run the game. */
    qmconfig_read(); /* Here because it fits, no conflicts with Linux placement -- JR 05/06/01 */
                     /* Here so we can set the IP adress. -- JR 05/06/01 */

    #if defined(macintosh) || defined(MSDOS)
        boot_db ();
        log_string ("Merc is ready to rock.");
        game_loop_mac_msdos ();
    #endif

    #if defined(unix)
        if (!copyover)
            control = init_socket (port);
        boot_db ();
        log_f ("ROM is ready to rock on port %d (%s).", port, mud_ipaddress);

        #ifdef IMC
            /* Initialize and connect to IMC2 */
            imc_startup (FALSE, imcsocket, copyover);
        #endif

        if (copyover)
            copyover_recover ();

        game_loop_unix (control);
        close (control);
        #ifdef IMC
            imc_shutdown (FALSE);
        #endif
    #endif

    /* Free allocated memory so we can track what was lost due to
     * memory leaks. */
    log_string ("Freeing all objects.");
    free_count = recycle_free_all ();
    log_f ("   %d object(s) freed.", free_count);

    /* Free allocated memory so we can track what was lost due to
     * memory leaks. */
    log_string ("Freeing all allocated memory.");
    free_count = string_space_dispose ();
    log_f ("   %d bytes of string space freed.", free_count);
    free_count = mem_pages_dispose ();
    log_f ("   %d bytes of paged memory freed.", free_count);

    /* Close our reserved file. */
    fclose (reserve_file);

    /* That's all, folks. */
    log_string ("Normal termination of game.");
    exit (0);
    return 0;
}

#if defined(macintosh) || defined(MSDOS)
void game_loop_mac_msdos (void) {
    struct timeval last_time;
    struct timeval now_time;
    static DESCRIPTOR_T dcon;

    gettimeofday (&last_time, NULL);
    current_time = (time_t) last_time.tv_sec;

    /* New_descriptor analogue. */
    dcon.descriptor = 0;
    if (!mud_ansiprompt)
        dcon.connected = CON_GET_NAME;
    else
        dcon.connected = CON_ANSI;

    dcon.ansi          = mud_ansicolor;
    dcon.host          = str_dup ("localhost");
    dcon.outsize       = 2000;
    dcon.outbuf        = mem_alloc (dcon.outsize);
    dcon.next          = descriptor_list;
    dcon.showstr_head  = NULL;
    dcon.showstr_point = NULL;
    dcon.olc_edit      = NULL; /* OLC */
    dcon.string_edit   = NULL; /* OLC */
    dcon.editor        = 0;    /* OLC */
    descriptor_list    = &dcon;

    /* First Contact! */
    if (!mud_ansiprompt) {
        extern char *help_greeting;
        if ( help_greeting[0] == '.' )
            send_to_desc ( help_greeting+1, &dcon );
        else
            send_to_desc ( help_greeting  , &dcon );
    }
    else
        write_to_buffer (&dcon, "Do you want ANSI? (Y/n) ", 0);

    /* Main loop */
    merc_down = FALSE;
    while (!merc_down) {
        DESCRIPTOR_T *d;

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
                char_stop_idling (d->character);

                /* OLC */
                if (d->showstr_point) {
                    d->lines_written = 0;
                    show_page (d);
                }
                else if (d->string_edit)
                    string_add (d->character, d->incomm);
                else {
                    switch (d->connected) {
                        case CON_PLAYING:
                            if (!run_olc_editor (d))
                                desc_substitute_alias (d, d->incomm);
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
    merc_down = FALSE;
    while (!merc_down) {
        fd_set in_set;
        fd_set out_set;
        fd_set exc_set;
        DESCRIPTOR_T *d;
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
                char_stop_idling (d->character);

                /* OLC */
                if (d->showstr_point) {
                    d->lines_written = 0;
                    show_page (d);
                }
                else if (d->string_edit)
                    string_add (d->character, d->incomm);
                else {
                    switch (d->connected) {
                        case CON_PLAYING:
                            if (!run_olc_editor (d))
                                desc_substitute_alias (d, d->incomm);
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
                    if (errno != EINTR) {
                        perror ("Game_loop: select: stall");
                        exit (1);
                    }
                }
            }
        }

        gettimeofday (&last_time, NULL);
        current_time = (time_t) last_time.tv_sec;
    }
}
#endif

/* Recover from a copyover - load players */
void copyover_recover (void) {
    DESCRIPTOR_T *d;
    FILE *fp;
    char name[100];
    char host[MSL];
    int desc;
    bool old;

    log_f ("Copyover recovery initiated");
    fp = fopen (COPYOVER_FILE, "r");

    /* there are some descriptors open which will hang forever then ? */
    if (!fp) {
        perror ("copyover_recover:fopen");
        log_f ("Copyover file not found. Exitting.\n\r");
        exit (1);
    }

    /* In case something crashes - doesn't prevent reading  */
    unlink (COPYOVER_FILE);
    while (1) {
        int errorcheck = fscanf (fp, "%d %s %s\n", &desc, name, host);
        if (errorcheck < 0)
            break;
        if (desc == -1)
            break;

        /* Write something, and check if it goes error-free */
        if (!write_to_descriptor (desc, "\n\rRestoring from copyover...\n\r", 0)) {
            close (desc); /* nope */
            continue;
        }

        d = descriptor_new ();
        d->descriptor = desc;

        d->host = str_dup (host);
        LIST_FRONT (d, next, descriptor_list);
        d->connected = CON_COPYOVER_RECOVER;    /* -15, so close_socket frees the char */

        /* Now, find the pfile */
        old = load_char_obj (d, name);

        /* Player file not found?! */
        if (!old) {
            write_to_descriptor (desc,
                "\n\rSomehow, your character was lost in the copyover. Sorry.\n\r", 0);
            close_socket (d);
            continue;
        }

        /* Player file found - ok! */
        write_to_descriptor (desc, "\n\rCopyover recovery complete.\n\r", 0);

        /* Just In Case */
        if (!d->character->in_room)
            d->character->in_room = get_room_index (ROOM_VNUM_TEMPLE);

        /* Insert in the char_list */
        LIST_FRONT (d->character, next, char_list);

        char_to_room (d->character, d->character->in_room);
        do_look (d->character, "auto");
        act ("$n materializes!", d->character, NULL, NULL, TO_NOTCHAR);
        d->connected = CON_PLAYING;

        if (d->character->pet != NULL) {
            char_to_room (d->character->pet, d->character->in_room);
            act ("$n materializes!.", d->character->pet, NULL, NULL,
                 TO_NOTCHAR);
        }
    }
    fclose (fp);
}

void qmconfig_read (void) {
    FILE *fp;
    bool match, reading;
    char *word;
    extern int mud_ansiprompt, mud_ansicolor, mud_telnetga;

    log_f ("Loading configuration settings from %s.", QMCONFIG_FILE);
    fp = fopen (QMCONFIG_FILE, "r");
    if (!fp) {
        log_f ("%s not found. Using compiled-in defaults.", QMCONFIG_FILE);
        return;
    }

    reading = TRUE;
    while (reading) {
        word = feof (fp) ? "END" : fread_word_static (fp);
        match = FALSE;

        switch (UPPER(word[0])) {
            case '#':
                /* This is a comment line! */
                match = TRUE;
                fread_to_eol (fp);
                break;

            case '*':
                match = TRUE;
                fread_to_eol (fp);
                break;

            case 'A':
                KEY ("Ansicolor", mud_ansicolor, fread_number(fp));
                KEY ("Ansiprompt", mud_ansiprompt, fread_number(fp));
                break;

            case 'E':
                if (!str_cmp (word, "END")) {
                    reading = FALSE;
                    match = TRUE;
                }
                break;

            case 'T':
                KEY ("Telnetga", mud_telnetga, fread_number(fp));
                break;
        }
        if (!match) {
            log_f ("qmconfig_read: no match for %s!", word);
            fread_to_eol(fp);
        }
    }

 // log_f ("Settings have been read from %s", QMCONFIG_FILE);
    fclose (fp);
}
