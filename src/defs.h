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

#ifndef __ROM_DEFS_H
#define __ROM_DEFS_H

/* Disable this is you want some changed features. */
/* Do a search for "#ifndef VANILLA" to find changed items. */

/* #define VANILLA */

/* Accommodate old non-Ansi compilers. */
#if defined(TRADITIONAL)
    #define const
    #define args(list)              ()
    #define DECLARE_DO_FUN(fun)     void fun()
    #define DECLARE_SPEC_FUN(fun)   bool fun()
    #define DECLARE_SPELL_FUN(fun)  void fun()
#else
    #define args(list)              list
    #define DECLARE_DO_FUN(fun)     DO_FUN    fun
    #define DECLARE_SPEC_FUN(fun)   SPEC_FUN  fun
    #define DECLARE_SPELL_FUN(fun)  SPELL_FUN fun
#endif

/* Function definitions in *.c */
#define DEFINE_DO_FUN(fun)      void fun (CHAR_DATA *ch, char *argument)
#define DEFINE_SPELL_FUN(fun)   void fun (int sn, int level, CHAR_DATA *ch, \
                                          void *vo, int target)

/* Short scalar types.
 * Diavolo reports AIX compiler has bugs with short types. */
#if !defined(FALSE)
    #define FALSE 0
#endif

#if !defined(TRUE)
    #define TRUE  1
#endif

#if defined(_AIX)
    #if !defined(const)
        #define const
    #endif
#endif

#if defined(macintosh)
    #define PLAYER_DIR  "player/"           /* Player files          */
    #define NULL_FILE   "proto.are"         /* To reserve one stream */
    #define TEMP_FILE   "romtmp"
#endif

#if defined(MSDOS)
    #define PLAYER_DIR  "player/"           /* Player files          */
    #define NULL_FILE   "nul"               /* To reserve one stream */
    #define TEMP_FILE   "romtmp"
#endif

#if defined(unix)
    #define PLAYER_DIR  "player/"           /* Player files          */
    #define GOD_DIR     "gods/"             /* list of gods          */
    #define NULL_FILE   "/dev/null"         /* To reserve one stream */
    #define TEMP_FILE   "player/romtmp"
#endif

#define AREA_DIR        "area/"
#define AREA_LIST       AREA_DIR "area.lst"     /* List of areas       */
#define BUG_FILE        AREA_DIR "bugs.txt"     /* For 'bug' and bug() */
#define TYPO_FILE       AREA_DIR "typos.txt"    /* For 'typo'          */
#define SHUTDOWN_FILE   AREA_DIR "shutdown.txt" /* For 'shutdown'      */
#define BAN_FILE        AREA_DIR "ban.txt"
#define MUSIC_FILE      AREA_DIR "music.txt"
#define OHELPS_FILE     AREA_DIR "orphaned_helps.txt" /* Unmet 'help' requests */
#define QMCONFIG_FILE   AREA_DIR "qmconfig.rc"

/* ea */
#define MSL MAX_STRING_LENGTH
#define MIL MAX_INPUT_LENGTH

/* String and memory management parameters. */
#define MAX_KEY_HASH          1024
#define MAX_STRING_LENGTH     4608
#define MAX_INPUT_LENGTH       256
#define PAGELEN                 22

/* I am lazy :) */
#define MSL MAX_STRING_LENGTH
#define MIL MAX_INPUT_LENGTH

/* Game parameters.
 * Increase the max'es if you add more of something.
 * Adjust the pulse numbers to suit yourself. */
#define MAX_IN_GROUP        15
#define MAX_ALIAS           5
#define MAX_LEVEL           60
#define LEVEL_HERO          (MAX_LEVEL - 9)
#define LEVEL_IMMORTAL      (MAX_LEVEL - 8)
#define L_IMM LEVEL_IMMORTAL

/* Added this for "orphaned help" code. Check do_help() -- JR */
#define MAX_CMD_LEN      50

#define PULSE_PER_SECOND 4
#define PULSE_SPEED      100 /* percentage; 100 = normal */

#define PULSE_VIOLENCE   (  3 * PULSE_PER_SECOND)
#define PULSE_MOBILE     (  4 * PULSE_PER_SECOND)
#define PULSE_MUSIC      (  6 * PULSE_PER_SECOND)
#define PULSE_TICK       ( 60 * PULSE_PER_SECOND)
#define PULSE_AREA       (120 * PULSE_PER_SECOND)

/* Amount of hit/mana/move we're dividing by when running health_update() */
#ifdef VANILLA
    #define PULSE_DIVISOR    1 
#else
    #define PULSE_DIVISOR    30
#endif
#define PULSE_HEALTH     (PULSE_TICK / PULSE_DIVISOR)

#define IMPLEMENTOR (MAX_LEVEL - 0)
#define CREATOR     (MAX_LEVEL - 1)
#define SUPREME     (MAX_LEVEL - 2)
#define DEITY       (MAX_LEVEL - 3)
#define GOD         (MAX_LEVEL - 4)
#define IMMORTAL    (MAX_LEVEL - 5)
#define DEMI        (MAX_LEVEL - 6)
#define ANGEL       (MAX_LEVEL - 7)
#define AVATAR      (MAX_LEVEL - 8)
#define HERO        (LEVEL_HERO)

/* Shorthand for command types */
#define ML    (MAX_LEVEL - 0)  /* implementor */
#define L1    (MAX_LEVEL - 1)  /* creator */
#define L2    (MAX_LEVEL - 2)  /* supreme being */
#define L3    (MAX_LEVEL - 3)  /* deity */
#define L4    (MAX_LEVEL - 4)  /* god */
#define L5    (MAX_LEVEL - 5)  /* immortal */
#define L6    (MAX_LEVEL - 6)  /* demigod */
#define L7    (MAX_LEVEL - 7)  /* angel */
#define L8    (MAX_LEVEL - 8)  /* avatar */
#define IM    (LEVEL_IMMORTAL) /* avatar */
#define HE    (LEVEL_HERO)     /* hero */

/* Time and weather stuff. */
#define SUN_DARK       0
#define SUN_RISE       1
#define SUN_LIGHT      2
#define SUN_SET        3

#define SKY_CLOUDLESS  0
#define SKY_CLOUDY     1
#define SKY_RAINING    2
#define SKY_LIGHTNING  3

/* damage classes */
#define DAM_NONE            0
#define DAM_BASH            1
#define DAM_PIERCE          2
#define DAM_SLASH           3
#define DAM_FIRE            4
#define DAM_COLD            5
#define DAM_LIGHTNING       6
#define DAM_ACID            7
#define DAM_POISON          8
#define DAM_NEGATIVE        9
#define DAM_HOLY           10
#define DAM_ENERGY         11
#define DAM_MENTAL         12
#define DAM_DISEASE        13
#define DAM_DROWNING       14
#define DAM_LIGHT          15
#define DAM_OTHER          16
#define DAM_HARM           17
#define DAM_CHARM          18
#define DAM_SOUND          19
#define DAM_MAX            20

/* TO types for act. */
#define TO_CHAR    (A) /* 'ch' only */
#define TO_VICT    (B) /* 'vch' only */
#define TO_OTHERS  (C) /* everyone else */
#define TO_NOTCHAR (TO_VICT | TO_OTHERS)
#define TO_ALL     (TO_CHAR | TO_VICT | TO_OTHERS)

/* Shop types. */
#define MAX_TRADE 5

/* Per-class stuff. */
#define MAX_GUILD 2

#define STAT_STR  0
#define STAT_INT  1
#define STAT_WIS  2
#define STAT_DEX  3
#define STAT_CON  4
#define STAT_MAX  5

/* where definitions */
#define TO_AFFECTS   0
#define TO_OBJECT    1
#define TO_IMMUNE    2
#define TO_RESIST    3
#define TO_VULN      4
#define TO_WEAPON    5

/* Well known mob virtual numbers.
 * Defined in #MOBILES. */
#define MOB_VNUM_FIDO       3090
#define MOB_VNUM_CITYGUARD  3060
#define MOB_VNUM_VAMPIRE    3404

#define MOB_VNUM_PATROLMAN  2106
#define GROUP_VNUM_TROLLS   2100
#define GROUP_VNUM_OGRES    2101

/* return values for check_imm */
#define IS_NORMAL          0
#define IS_IMMUNE          1
#define IS_RESISTANT       2
#define IS_VULNERABLE      3

/* Liquids. */
#define LIQ_WATER   0
/* ... lots of stuff ... */
#define LIQ_MAX     36

/* dice */
#define DICE_NUMBER  0
#define DICE_TYPE    1
#define DICE_BONUS   2

/* Well known object virtual numbers.
 * Defined in #OBJECTS. */
#define OBJ_VNUM_SILVER_ONE         1
#define OBJ_VNUM_GOLD_ONE           2
#define OBJ_VNUM_GOLD_SOME          3
#define OBJ_VNUM_SILVER_SOME        4
#define OBJ_VNUM_COINS              5

#define OBJ_VNUM_CORPSE_NPC        10
#define OBJ_VNUM_CORPSE_PC         11
#define OBJ_VNUM_SEVERED_HEAD      12
#define OBJ_VNUM_TORN_HEART        13
#define OBJ_VNUM_SLICED_ARM        14
#define OBJ_VNUM_SLICED_LEG        15
#define OBJ_VNUM_GUTS              16
#define OBJ_VNUM_BRAINS            17

#define OBJ_VNUM_MUSHROOM          20
#define OBJ_VNUM_LIGHT_BALL        21
#define OBJ_VNUM_SPRING            22
#define OBJ_VNUM_DISC              23
#define OBJ_VNUM_PORTAL            25

#define OBJ_VNUM_ROSE              1001

#define OBJ_VNUM_PIT               3010

#define OBJ_VNUM_SCHOOL_MACE       3700
#define OBJ_VNUM_SCHOOL_DAGGER     3701
#define OBJ_VNUM_SCHOOL_SWORD      3702
#define OBJ_VNUM_SCHOOL_SPEAR      3717
#define OBJ_VNUM_SCHOOL_STAFF      3718
#define OBJ_VNUM_SCHOOL_AXE        3719
#define OBJ_VNUM_SCHOOL_FLAIL      3720
#define OBJ_VNUM_SCHOOL_WHIP       3721
#define OBJ_VNUM_SCHOOL_POLEARM    3722

#define OBJ_VNUM_SCHOOL_VEST       3703
#define OBJ_VNUM_SCHOOL_SHIELD     3704
#define OBJ_VNUM_SCHOOL_BANNER     3716
#define OBJ_VNUM_MAP               3162

#define OBJ_VNUM_WHISTLE           2116

/* magic numbers. */
#define OBJ_VALUE_MAX   5

/* Values for object mapping.
 * Used in #OBJECTS. */
#define MAP_INTEGER     0
#define MAP_LOOKUP      1
#define MAP_FLAGS       2
#define MAP_IGNORE      3
#define MAP_BOOLEAN     4

/* Sub-types of MAP_LOOKUP. */
#define MAP_LOOKUP_WEAPON_TYPE  0
#define MAP_LOOKUP_ATTACK_TYPE  1
#define MAP_LOOKUP_LIQUID       2
#define MAP_LOOKUP_SKILL        3

/* Sub-types of MAP_FLAGS. */
#define MAP_FLAGS_WEAPON        0
#define MAP_FLAGS_CONT          1
#define MAP_FLAGS_FURNITURE     3
#define MAP_FLAGS_EXIT          4
#define MAP_FLAGS_PORTAL        5

/* Well known room virtual numbers.
 * Defined in #ROOMS. */
#define ROOM_VNUM_LIMBO       2
#define ROOM_VNUM_CHAT     1200
#define ROOM_VNUM_TEMPLE   3001
#define ROOM_VNUM_ALTAR    3054
#define ROOM_VNUM_SCHOOL   3700
#define ROOM_VNUM_BALANCE  4500
#define ROOM_VNUM_CIRCLE   4400
#define ROOM_VNUM_DEMISE   4201
#define ROOM_VNUM_HONOR    4300

/* Directions.
 * Used in #ROOMS. */
#define DIR_NONE  -1
#define DIR_NORTH  0
#define DIR_EAST   1
#define DIR_SOUTH  2
#define DIR_WEST   3
#define DIR_UP     4
#define DIR_DOWN   5
#define DIR_MAX    6

/* Conditions. */
#define COND_DRUNK    0
#define COND_FULL     1
#define COND_THIRST   2
#define COND_HUNGER   3

/* Types of attacks.
 * Must be non-overlapping with spell/skill types,
 * but may be arbitrary beyond that. */
#define TYPE_UNDEFINED   -1
#define TYPE_HIT       1000

/* Target types. */
#define TAR_IGNORE          0
#define TAR_CHAR_OFFENSIVE  1
#define TAR_CHAR_DEFENSIVE  2
#define TAR_CHAR_SELF       3
#define TAR_OBJ_INV         4
#define TAR_OBJ_CHAR_DEF    5
#define TAR_OBJ_CHAR_OFF    6

#define TARGET_CHAR         0
#define TARGET_OBJ          1
#define TARGET_ROOM         2
#define TARGET_NONE         3

/* Object defined in limbo.are
 * Used in save.c to load objects that don't exist. */
#define OBJ_VNUM_DUMMY    30

#define NO_FLAG    -1

/* Includes for board system */
/* This is version 2 of the board system, (c) 1995-96 erwin@pip.dknet.dk */
#define NOTE_DIR    "notes" /* set it to something you like */

#define DEF_NORMAL  0  /* No forced change, but default (any string)  */
#define DEF_INCLUDE 1 /* 'names' MUST be included (only ONE name!)    */
#define DEF_EXCLUDE 2 /* 'names' must NOT be included (one name only) */

#define DEFAULT_BOARD 0 /* default board is board #0 in the boards      */
                        /* It should be readable by everyone!           */

#define MAX_LINE_LENGTH 80 /* enforce a max length of 80 on text lines, reject longer lines */
                           /* This only applies in the Body of the note */

#define MAX_NOTE_TEXT (4 * MAX_STRING_LENGTH - 1000)

#define BOARD_NOTFOUND -1 /* Error code from board_lookup() and board_number */

/* Connected state for a channel. */
#define CON_GET_NAME         -14
#define CON_GET_OLD_PASSWORD -13
#define CON_CONFIRM_NEW_NAME -12
#define CON_GET_NEW_PASSWORD -11
#define CON_CONFIRM_PASSWORD -10
#define CON_ANSI              -9
#define CON_GET_TELNETGA      -8
#define CON_GET_NEW_RACE      -7
#define CON_GET_NEW_SEX       -6
#define CON_GET_NEW_CLASS     -5
#define CON_GET_ALIGNMENT     -4
#define CON_DEFAULT_CHOICE    -3
#define CON_GEN_GROUPS        -2
#define CON_PICK_WEAPON       -1
#define CON_PLAYING            0
#define CON_READ_IMOTD         1
#define CON_READ_MOTD          2
#define CON_BREAK_CONNECT      3
#define CON_COPYOVER_RECOVER   4
#define CON_NOTE_TO            5
#define CON_NOTE_SUBJECT       6
#define CON_NOTE_EXPIRE        7
#define CON_NOTE_TEXT          8
#define CON_NOTE_FINISH        9
#define NANNY_MAX              24

/* Character classes. */
#define CLASS_NONE     -1
#define CLASS_MAGE      0
#define CLASS_CLERIC    1
#define CLASS_THIEF     2
#define CLASS_WARRIOR   3
#define CLASS_MAX       4

/* Limits not defined elsewhere. */
#define CLAN_MAX            3
#define WIZNET_MAX          20
#define RACE_MAX            30
#define PC_RACE_MAX         6
#define SOCIAL_MAX          256
#define SKILL_MAX           150
#define GROUP_MAX           30
#define SPEC_MAX            22
#define ATTRIBUTE_MAX       26
#define ATTACK_MAX          40
#define BOARD_MAX           5

/* Colour setting codes */
#define COLOUR_TEXT            0
#define COLOUR_AUCTION         1
#define COLOUR_AUCTION_TEXT    2
#define COLOUR_GOSSIP          3
#define COLOUR_GOSSIP_TEXT     4
#define COLOUR_MUSIC           5
#define COLOUR_MUSIC_TEXT      6
#define COLOUR_QUESTION        7
#define COLOUR_QUESTION_TEXT   8
#define COLOUR_ANSWER          9
#define COLOUR_ANSWER_TEXT    10
#define COLOUR_QUOTE          11
#define COLOUR_QUOTE_TEXT     12
#define COLOUR_IMMTALK_TEXT   13
#define COLOUR_IMMTALK_TYPE   14
#define COLOUR_INFO           15
#define COLOUR_SAY            16
#define COLOUR_SAY_TEXT       17
#define COLOUR_TELL           18
#define COLOUR_TELL_TEXT      19
#define COLOUR_REPLY          20
#define COLOUR_REPLY_TEXT     21
#define COLOUR_GTELL_TEXT     22
#define COLOUR_GTELL_TYPE     23
#define COLOUR_WIZNET         24
#define COLOUR_ROOM_TITLE     25
#define COLOUR_ROOM_TEXT      26
#define COLOUR_ROOM_EXITS     27
#define COLOUR_ROOM_THINGS    28
#define COLOUR_PROMPT         29
#define COLOUR_FIGHT_DEATH    30
#define COLOUR_FIGHT_YHIT     31
#define COLOUR_FIGHT_OHIT     32
#define COLOUR_FIGHT_THIT     33
#define COLOUR_FIGHT_SKILL    34
#define COLOUR_MAX            35

/* This file holds the copyover data */
#define COPYOVER_FILE "copyover.data"

/* This is the executable file */
#define EXE_FILE      "bin/rom"

/* Recycleable types. */
#define RECYCLE_BAN_DATA            0
#define RECYCLE_AREA_DATA           1
#define RECYCLE_EXTRA_DESCR_DATA    2
#define RECYCLE_EXIT_DATA           3
#define RECYCLE_ROOM_INDEX_DATA     4
#define RECYCLE_OBJ_INDEX_DATA      5
#define RECYCLE_SHOP_DATA           6
#define RECYCLE_MOB_INDEX_DATA      7
#define RECYCLE_RESET_DATA          8
#define RECYCLE_HELP_DATA           9
#define RECYCLE_MPROG_CODE         10
#define RECYCLE_DESCRIPTOR_DATA    11
#define RECYCLE_GEN_DATA           12
#define RECYCLE_AFFECT_DATA        13
#define RECYCLE_OBJ_DATA           14
#define RECYCLE_CHAR_DATA          15
#define RECYCLE_PC_DATA            16
#define RECYCLE_MEM_DATA           17
#define RECYCLE_BUFFER             18
#define RECYCLE_MPROG_LIST         19
#define RECYCLE_HELP_AREA          20
#define RECYCLE_NOTE_DATA          21
#define RECYCLE_SOCIAL_TYPE        22
#define RECYCLE_PORTAL_EXIT_TYPE   23
#define RECYCLE_PORTAL_TYPE        24
#define RECYCLE_MAX                25

/* Memory management.
 * Increase MAX_STRING if you have too.
 * Tune the others only if you understand what you're doing. */
#define MAX_STRING      1413120
#define MAX_PERM_BLOCK  131072
#define MAX_MEM_LIST    11
#define MAX_PERM_BLOCKS 1024

/* Material types - currently unused, but neat. */
#define MATERIAL_GENERIC     0
#define MATERIAL_ADAMANTITE  1
#define MATERIAL_ADAMITE     2
#define MATERIAL_ALUMINUM    3
#define MATERIAL_BRASS       4
#define MATERIAL_BRONZE      5
#define MATERIAL_CHINA       6
#define MATERIAL_CLAY        7
#define MATERIAL_CLOTH       8
#define MATERIAL_COPPER      9
#define MATERIAL_CRYSTAL    10
#define MATERIAL_DIAMOND    11
#define MATERIAL_ENERGY     12
#define MATERIAL_FLESH      13
#define MATERIAL_FOOD       14
#define MATERIAL_FUR        15
#define MATERIAL_GEM        16
#define MATERIAL_GLASS      17
#define MATERIAL_GOLD       18
#define MATERIAL_ICE        19
#define MATERIAL_IRON       20
#define MATERIAL_IVORY      21
#define MATERIAL_LEAD       22
#define MATERIAL_LEATHER    23
#define MATERIAL_MEAT       24
#define MATERIAL_MITHRIL    25
#define MATERIAL_OBSIDIAN   26
#define MATERIAL_PAPER      27
#define MATERIAL_PARCHMENT  28
#define MATERIAL_PEARL      29
#define MATERIAL_PLATINUM   30
#define MATERIAL_RUBBER     31
#define MATERIAL_SHADOW     32
#define MATERIAL_SILVER     33
#define MATERIAL_STEEL      34
#define MATERIAL_TIN        35
#define MATERIAL_VELLUM     36
#define MATERIAL_WATER      37
#define MATERIAL_WOOD       38
#define MATERIAL_MAX        39

/* Types of tables used for our master reference table. */
#define TABLE_FLAG_TYPE 0x01
#define TABLE_BITS      0x02

/* Types of portals. */
#define PORTAL_TO_ROOM  0
#define PORTAL_TO_EXIT  1

#endif
