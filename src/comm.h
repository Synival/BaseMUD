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
 *    ROM 2.4 is copyright 1993-1998 Russ Taylor                           *
 *    ROM has been brought to you by the ROM consortium                    *
 *        Russ Taylor (rtaylor@hypercube.org)                              *
 *        Gabrielle Taylor (gtaylor@hypercube.org)                         *
 *        Brian Moore (zump@rom.org)                                       *
 *    By using this code, you have agreed to follow the terms of the       *
 *    ROM license, in the file Rom24/doc/rom.license                       *
 ***************************************************************************/

#ifndef __ROM_COMM_H
#define __ROM_COMM_H

#include "merc.h"

/* Signal handling.
 * Apollo has a problem with __attribute(atomic) in signal.h,
 *   I dance around it. */
#if defined(apollo)
    #define __attribute(x)
#endif

#if defined(apollo)
    #undef __attribute
#endif

extern const char echo_off_str[];
extern const char echo_on_str[];
extern const char go_ahead_str[];

/* OS-dependent declarations. */
#if defined(_AIX)
    #include <sys/select.h>
    int accept args ((int s, struct sockaddr * addr, int *addrlen));
    int bind args ((int s, struct sockaddr * name, int namelen));
    void bzero args ((char *b, int length));
    int getpeername args ((int s, struct sockaddr * name, int *namelen));
    int getsockname args ((int s, struct sockaddr * name, int *namelen));
    int gettimeofday args ((struct timeval * tp, struct timezone * tzp));
    int listen args ((int s, int backlog));
    int setsockopt args ((int s, int level, int optname, void *optval,
                          int optlen));
    int socket args ((int domain, int type, int protocol));
#endif

#if defined(apollo)
    #include <unistd.h>
    void bzero args ((char *b, int length));
#endif

#if defined(__hpux)
    int accept args ((int s, void *addr, int *addrlen));
    int bind args ((int s, const void *addr, int addrlen));
    void bzero args ((char *b, int length));
    int getpeername args ((int s, void *addr, int *addrlen));
    int getsockname args ((int s, void *name, int *addrlen));
    int gettimeofday args ((struct timeval * tp, struct timezone * tzp));
    int listen args ((int s, int backlog));
    int setsockopt args ((int s, int level, int optname,
                          const void *optval, int optlen));
    int socket args ((int domain, int type, int protocol));
#endif

#if defined(interactive)
    #include <net/errno.h>
    #include <sys/fnctl.h>
#endif

#if defined(linux)
/* Linux shouldn't need these. If you have a problem compiling, try
   uncommenting these functions. */
/*
    int accept args( ( int s, struct sockaddr *addr, int *addrlen ) );
    int bind args( ( int s, struct sockaddr *name, int namelen ) );
    int getpeername args( ( int s, struct sockaddr *name, int *namelen ) );
    int getsockname args( ( int s, struct sockaddr *name, int *namelen ) );
    int listen args( ( int s, int backlog ) );
*/

    int close args ((int fd));
    int gettimeofday args ((struct timeval * tp, struct timezone * tzp));
    /* int read args( ( int fd, char *buf, int nbyte ) ); */
    int select args ((int width, fd_set * readfds, fd_set * writefds,
                      fd_set * exceptfds, struct timeval * timeout));
    int socket args ((int domain, int type, int protocol));
    /* int write args( ( int fd, char *buf, int nbyte ) ); *//* read,write in unistd.h */
#endif

#if defined(macintosh)
    #include <console.h>
    #include <fcntl.h>
    #include <unix.h>
    struct timeval {
        time_t tv_sec;
        time_t tv_usec;
    };
    #if !defined(isascii)
        #define isascii(c) ( (c) < 0200 )
    #endif
    static long theKeys[4];

    int gettimeofday args ((struct timeval * tp, void *tzp));
#endif

#if defined(MIPS_OS)
    extern int errno;
#endif

#if defined(MSDOS)
    int gettimeofday args ((struct timeval * tp, void *tzp));
    int kbhit args ((void));
#endif

#if defined(NeXT)
    int close args ((int fd));
    int fcntl args ((int fd, int cmd, int arg));
    #if !defined(htons)
        u_short htons args ((u_short hostshort));
    #endif
    #if !defined(ntohl)
        u_long ntohl args ((u_long hostlong));
    #endif
    int read args ((int fd, char *buf, int nbyte));
    int select args ((int width, fd_set * readfds, fd_set * writefds,
                      fd_set * exceptfds, struct timeval * timeout));
    int write args ((int fd, char *buf, int nbyte));
#endif

#if defined(sequent)
    int accept args ((int s, struct sockaddr * addr, int *addrlen));
    int bind args ((int s, struct sockaddr * name, int namelen));
    int close args ((int fd));
    int fcntl args ((int fd, int cmd, int arg));
    int getpeername args ((int s, struct sockaddr * name, int *namelen));
    int getsockname args ((int s, struct sockaddr * name, int *namelen));
    int gettimeofday args ((struct timeval * tp, struct timezone * tzp));
    #if !defined(htons)
        u_short htons args ((u_short hostshort));
    #endif
    int listen args ((int s, int backlog));
    #if !defined(ntohl)
        u_long ntohl args ((u_long hostlong));
    #endif
    int read args ((int fd, char *buf, int nbyte));
    int select args ((int width, fd_set * readfds, fd_set * writefds,
                      fd_set * exceptfds, struct timeval * timeout));
    int setsockopt args ((int s, int level, int optname, caddr_t optval,
                          int optlen));
    int socket args ((int domain, int type, int protocol));
    int write args ((int fd, char *buf, int nbyte));
#endif

/* This includes Solaris Sys V as well */
#if defined(sun)
    int accept args ((int s, struct sockaddr * addr, int *addrlen));
    int bind args ((int s, struct sockaddr * name, int namelen));
    void bzero args ((char *b, int length));
    int close args ((int fd));
    int getpeername args ((int s, struct sockaddr * name, int *namelen));
    int getsockname args ((int s, struct sockaddr * name, int *namelen));
    int listen args ((int s, int backlog));
    int read args ((int fd, char *buf, int nbyte));
    int select args ((int width, fd_set * readfds, fd_set * writefds,
                      fd_set * exceptfds, struct timeval * timeout));

    #if !defined(__SVR4)
        int gettimeofday args ((struct timeval * tp, struct timezone * tzp));

        #if defined(SYSV)
            int setsockopt args ((int s, int level, int optname,
                                  const char *optval, int optlen));
        #else
            int setsockopt args ((int s, int level, int optname, void *optval,
                                  int optlen));
        #endif
    #endif
    int socket args ((int domain, int type, int protocol));
    int write args ((int fd, char *buf, int nbyte));
#endif

#if defined(ultrix)
    int accept args ((int s, struct sockaddr * addr, int *addrlen));
    int bind args ((int s, struct sockaddr * name, int namelen));
    void bzero args ((char *b, int length));
    int close args ((int fd));
    int getpeername args ((int s, struct sockaddr * name, int *namelen));
    int getsockname args ((int s, struct sockaddr * name, int *namelen));
    int gettimeofday args ((struct timeval * tp, struct timezone * tzp));
    int listen args ((int s, int backlog));
    int read args ((int fd, char *buf, int nbyte));
    int select args ((int width, fd_set * readfds, fd_set * writefds,
                      fd_set * exceptfds, struct timeval * timeout));
    int setsockopt args ((int s, int level, int optname, void *optval,
                          int optlen));
    int socket args ((int domain, int type, int protocol));
    int write args ((int fd, char *buf, int nbyte));
#endif

/* OS-dependent local functions. */
#if defined(macintosh) || defined(MSDOS)
    void game_loop_mac_msdos args ((void));
    bool read_from_descriptor args ((DESCRIPTOR_DATA * d));
    bool write_to_descriptor args ((int desc, char *txt, int length));
#endif

#if defined(unix)
    void game_loop_unix args ((int control));
    int init_socket args ((int port));
    void init_descriptor args ((int control));
    bool read_from_descriptor args ((DESCRIPTOR_DATA * d));
    bool write_to_descriptor args ((int desc, char *txt, int length));
#endif

/* Global variables. */
extern DESCRIPTOR_DATA *descriptor_list; /* All open descriptors     */
extern DESCRIPTOR_DATA *d_next;          /* Next descriptor in loop  */
extern FILE *fpReserve;                  /* Reserved file handle     */
extern bool god;                         /* All new chars are gods!  */
extern bool merc_down;                   /* Shutdown         */
extern bool wizlock;                     /* Game is wizlocked        */
extern bool newlock;                     /* Game is newlocked        */
extern char str_boot_time[MAX_INPUT_LENGTH];
extern time_t current_time;              /* time of this pulse */
extern bool MOBtrigger;                  /* act() switch */

/* Needs to be global because of do_copyover */
extern int port, control;

/* Put global mud config values here. Look at qmconfig command for clues.     */
/*   -- JR 09/23/2000                                                         */
/* Set values for all but IP address in ../area/qmconfig.rc file.             */
/*   -- JR 05/10/2001                                                         */
extern int mud_ansiprompt, mud_ansicolor, mud_telnetga;

/* Set this to the IP address you want to listen on (127.0.0.1 is good for    */
/* paranoid types who don't want the 'net at large peeking at their MUD)      */
extern char *mud_ipaddress;

/* Function prototypes. */
int main(int argc, char **argv);
int init_socket (int port);
void game_loop_mac_msdos (void);
void game_loop_unix (int control);
void init_descriptor (int control);
void close_socket (DESCRIPTOR_DATA * dclose);
bool read_from_descriptor (DESCRIPTOR_DATA * d);
void read_from_buffer (DESCRIPTOR_DATA * d);
bool process_output (DESCRIPTOR_DATA * d, bool fPrompt);
void bust_a_prompt (CHAR_DATA *ch);
void write_to_buffer (DESCRIPTOR_DATA * d, const char *txt, int length);
bool write_to_descriptor (int desc, char *txt, int length);
bool check_parse_name (char *name);
bool check_reconnect (DESCRIPTOR_DATA * d, char *name, bool fConn);
bool check_playing (DESCRIPTOR_DATA * d, char *name);
void stop_idling (CHAR_DATA * ch);
void send_to_char_bw (const char *txt, CHAR_DATA *ch);
void page_to_char_bw (const char *txt, CHAR_DATA *ch);
void send_to_char (const char *txt, CHAR_DATA * ch);
void send_to_desc (const char *txt, DESCRIPTOR_DATA * d);
void page_to_char (const char *txt, CHAR_DATA * ch);
void clear_page (DESCRIPTOR_DATA *d);
void append_to_page (DESCRIPTOR_DATA *d, const char *txt);
int show_page (DESCRIPTOR_DATA *d);
void fix_sex (CHAR_DATA * ch);
void act2 (const char *to_char, const char *to_room, CHAR_DATA *ch,
           const void *arg1, const void *arg2, flag_t flags, int min_pos);
void act3 (const char *to_char, const char *to_vict, const char *to_room,
           CHAR_DATA *ch, const void *arg1, const void *arg2, flag_t flags,
           int min_pos);
void act_new (const char *format, CHAR_DATA * ch, const void *arg1,
              const void *arg2, flag_t flags, int min_pos);
void printf_to_desc (DESCRIPTOR_DATA * d, char *fmt, ...);
void printf_to_char (CHAR_DATA * ch, char *fmt, ...);
void wiznet (char *string, CHAR_DATA * ch, OBJ_DATA * obj,
             flag_t flag, flag_t flag_skip, int min_level);
void qmconfig_read (void);
const char *get_align_name (int align);
const char *get_sex_name (int sex);
const char *get_ch_class_name (CHAR_DATA * ch);
const char *get_ac_type_name (int type);
const char *get_position_name (int position);
const char *get_character_position_str (CHAR_DATA * ch, int position,
    OBJ_DATA * on, int with_punct);
const char *get_ac_rating_phrase (int ac);
void copyover_recover ();
bool position_change_message (CHAR_DATA * ch, int from, int to,
    OBJ_DATA *obj);
bool position_change_message_to_standing (CHAR_DATA * ch, int from, OBJ_DATA *obj);
bool position_change_message_to_fighting (CHAR_DATA * ch, int from, OBJ_DATA *obj);
bool position_change_message_to_resting (CHAR_DATA * ch, int from, OBJ_DATA *obj);
bool position_change_message_to_sitting (CHAR_DATA * ch, int from, OBJ_DATA *obj);
bool position_change_message_to_sleeping (CHAR_DATA * ch, int from, OBJ_DATA *obj);
void substitute_alias (DESCRIPTOR_DATA * d, char *argument);
void echo_to_char (CHAR_DATA *to, CHAR_DATA *from, const char *type,
    const char *msg);

#endif
