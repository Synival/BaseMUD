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

#ifndef __ROM_STRUCTS_H
#define __ROM_STRUCTS_H

#include "merc.h"

typedef long int flag_t;

/* Structure types. */
typedef struct affect_data      AFFECT_DATA;
typedef struct area_data        AREA_DATA;
typedef struct ban_data         BAN_DATA;
typedef struct buf_type         BUFFER;
typedef struct char_data        CHAR_DATA;
typedef struct descriptor_data  DESCRIPTOR_DATA;
typedef struct exit_data        EXIT_DATA;
typedef struct extra_descr_data EXTRA_DESCR_DATA;
typedef struct help_data        HELP_DATA;
typedef struct help_area_data   HELP_AREA;
typedef struct kill_data        KILL_DATA;
typedef struct mem_data         MEM_DATA;
typedef struct mob_index_data   MOB_INDEX_DATA;
typedef struct note_data        NOTE_DATA;
typedef struct obj_data         OBJ_DATA;
typedef struct obj_map          OBJ_MAP;
typedef struct obj_map_value    OBJ_MAP_VALUE;
typedef struct obj_index_data   OBJ_INDEX_DATA;
typedef struct pc_data          PC_DATA;
typedef struct gen_data         GEN_DATA;
typedef struct reset_data       RESET_DATA;
typedef struct room_index_data  ROOM_INDEX_DATA;
typedef struct shop_data        SHOP_DATA;
typedef struct time_info_data   TIME_INFO_DATA;
typedef struct weather_data     WEATHER_DATA;
typedef struct mprog_list       MPROG_LIST;
typedef struct mprog_code       MPROG_CODE;
typedef struct nanny_handler    NANNY_HANDLER;
typedef struct furniture_bits   FURNITURE_BITS;
typedef struct colour_type      COLOUR_TYPE;
typedef struct colour_setting_type COLOUR_SETTING_TYPE;
typedef struct map_lookup_table MAP_LOOKUP_TABLE;
typedef struct liq_type         LIQ_TYPE;
typedef struct skill_type       SKILL_TYPE;
typedef struct weapon_type      WEAPON_TYPE;
typedef struct item_type        ITEM_TYPE;
typedef struct dam_type         DAM_TYPE;
typedef struct attack_type      ATTACK_TYPE;
typedef struct wiznet_type      WIZNET_TYPE;
typedef struct clan_type        CLAN_TYPE;
typedef struct position_type    POSITION_TYPE;
typedef struct sex_type         SEX_TYPE;
typedef struct size_type        SIZE_TYPE;
typedef struct sector_type      SECTOR_TYPE;
typedef struct race_type        RACE_TYPE;
typedef struct pc_race_type     PC_RACE_TYPE;
typedef struct class_type       CLASS_TYPE;
typedef struct skill_type       SKILL_TYPE;
typedef struct spec_type        SPEC_TYPE;
typedef struct group_type       GROUP_TYPE;
typedef struct flag_type        FLAG_TYPE;
typedef struct door_type        DOOR_TYPE;
typedef struct str_app_type     STR_APP_TYPE;
typedef struct int_app_type     INT_APP_TYPE;
typedef struct wis_app_type     WIS_APP_TYPE;
typedef struct dex_app_type     DEX_APP_TYPE;
typedef struct con_app_type     CON_APP_TYPE;
typedef struct social_type      SOCIAL_TYPE;
typedef struct board_data       BOARD_DATA;
typedef struct wear_type        WEAR_TYPE;
typedef struct recycle_type     RECYCLE_TYPE;
typedef struct obj_recycle_data OBJ_RECYCLE_DATA;
typedef struct material_type    MATERIAL_TYPE;
typedef struct flag_stat_type   FLAG_STAT_TYPE;
typedef struct table_type       TABLE_TYPE;
typedef struct portal_exit_type PORTAL_EXIT_TYPE;
typedef struct portal_type      PORTAL_TYPE;
typedef struct cmd_type         CMD_TYPE;
typedef struct affect_bit_type  AFFECT_BIT_TYPE;
typedef struct day_type         DAY_TYPE;
typedef struct month_type       MONTH_TYPE;

/* Function types. */
typedef void DO_FUN     (CHAR_DATA *ch, char *argument);
typedef bool SPEC_FUN   (CHAR_DATA *ch);
typedef void SPELL_FUN  (int sn, int level, CHAR_DATA *ch, void *vo,
                         int target);
typedef void EFFECT_FUN (void *vo, int level, int dam, int target);
typedef void NANNY_FUN  (DESCRIPTOR_DATA * d, char *argument);
typedef JSON_T *TABLE_JSON_FUN (const void *obj);

typedef void RECYCLE_INIT_FUN (void *obj);
typedef void RECYCLE_DISPOSE_FUN (void *obj);

/* Objects that can be instantiated, freed, recycled, and catalogued. */
struct recycle_type {
    int type;
    char *name;
    size_t size;
    size_t obj_data_off;
    size_t obj_name_off;
    RECYCLE_INIT_FUN *const init_fun;
    RECYCLE_DISPOSE_FUN *const dispose_fun;

    /* internal - do not set in definition table! */
    int top, list_count, free_count;
    OBJ_RECYCLE_DATA *list_front, *list_back;
    OBJ_RECYCLE_DATA *free_front, *free_back;
};

/* Data stored in individual objects to make recycling work. */
struct obj_recycle_data {
    void *obj;
    OBJ_RECYCLE_DATA *prev, *next;
    bool valid;
};

/* Structures, ahoy!*/
struct ban_data {
    BAN_DATA *next;
    flag_t ban_flags;
    sh_int level;
    char *name;
    OBJ_RECYCLE_DATA rec_data;
};

struct buf_type {
    sh_int state;  /* error state of the buffer */
    sh_int size;   /* size in k */
    char *string; /* buffer's string */
    OBJ_RECYCLE_DATA rec_data;
};

struct time_info_data {
    int hour;
    int day;
    int month;
    int year;
};

struct weather_data {
    int mmhg;
    int change;
    int sky;
    int sunlight;
};

/* Descriptor (channel) structure. */
struct descriptor_data {
    DESCRIPTOR_DATA *next;
    DESCRIPTOR_DATA *snoop_by;
    CHAR_DATA *character;
    CHAR_DATA *original;
    bool ansi;
    char *host;
    sh_int descriptor;
    sh_int connected;
    bool fcommand;
    char inbuf[4 * MAX_INPUT_LENGTH];
    char incomm[MAX_INPUT_LENGTH];
    char inlast[MAX_INPUT_LENGTH];
    int repeat;
    char *outbuf;
    int outsize;
    int outtop;
    char *showstr_head;
    char *showstr_point;
    int lines_written; /* for the pager */
    void *pEdit;    /* OLC */
    char **pString; /* OLC */
    int editor;     /* OLC */
    OBJ_RECYCLE_DATA rec_data;
};

/* Attribute bonus structures. */
struct str_app_type {
    int stat;
    sh_int tohit;
    sh_int todam;
    sh_int carry;
    sh_int wield;
    OBJ_RECYCLE_DATA rec_data;
};

struct int_app_type {
    int stat;
    sh_int learn;
};

struct wis_app_type {
    int stat;
    sh_int practice;
};

struct dex_app_type {
    int stat;
    sh_int defensive;
};

struct con_app_type {
    int stat;
    sh_int hitp;
    sh_int shock;
};

/* Help table types. */
struct help_data {
    HELP_DATA *next;
    HELP_DATA *next_area;
    sh_int level;
    char *keyword;
    char *text;
    OBJ_RECYCLE_DATA rec_data;
};

struct help_area_data {
    HELP_AREA *next;
    HELP_DATA *first;
    HELP_DATA *last;
    AREA_DATA *area;
    char *filename;
    char *name;
    bool changed;
    OBJ_RECYCLE_DATA rec_data;
};

struct shop_data {
    SHOP_DATA *next;
    sh_int keeper;                /* Vnum of shop keeper mob     */
    sh_int buy_type [MAX_TRADE];  /* Item types shop will buy    */
    sh_int profit_buy;            /* Cost multiplier for buying  */
    sh_int profit_sell;           /* Cost multiplier for selling */
    sh_int open_hour;             /* First opening hour          */
    sh_int close_hour;            /* First closing hour          */
    OBJ_RECYCLE_DATA rec_data;
};

struct class_type {
    int type;
    char * name;             /* the full name of the class  */
    char who_name    [4];    /* Three-letter name for 'who' */
    sh_int attr_prime;       /* Prime attribute             */
    sh_int weapon;           /* First weapon                */
    sh_int guild[MAX_GUILD]; /* Vnum of guild rooms         */
    sh_int skill_adept;      /* Maximum skill level         */
    sh_int thac0_00;         /* Thac0 for level  0          */
    sh_int thac0_32;         /* Thac0 for level 32          */
    sh_int hp_min;           /* Min hp gained on leveling   */
    sh_int hp_max;           /* Max hp gained on leveling   */
    bool fMana;              /* Class gains mana on level   */
    char * base_group;       /* base skills gained          */
    char * default_group;    /* default skills gained       */
    OBJ_RECYCLE_DATA rec_data;
};

struct item_type {
    int type;
    char * name;
};

struct weapon_type {
    sh_int type;
    char *name;
    sh_int newbie_vnum;
    sh_int *gsn;
};

struct wiznet_type {
    flag_t bit;
    char *name;
    int level;
};

struct dam_type {
    int type;
    char *name;
    flag_t res;
    EFFECT_FUN *effect;
};

struct attack_type {
    char *name; /* name         */
    char *noun; /* message      */
    int damage; /* damage class */
};

struct race_type {
    char * name;  /* call name of the race          */
    bool pc_race; /* can be chosen by pcs           */
    flag_t mob;   /* act bits for the race          */
    flag_t aff;   /* aff bits for the race          */
    flag_t off;   /* off bits for the race          */
    flag_t imm;   /* imm bits for the race          */
    flag_t res;   /* res bits for the race          */
    flag_t vuln;  /* vuln bits for the race         */
    flag_t form;  /* default form flag for the race */
    flag_t parts; /* default parts for the race     */
};

struct pc_race_type {             /* additional data for pc races    */
    char * name;                  /* MUST be in race_type            */
    char who_name[8];
    sh_int points;                /* cost in points of the race      */
    sh_int class_mult[CLASS_MAX]; /* exp multiplier for class, * 100 */
    char * skills[5];             /* bonus skills for the race       */
    sh_int stats[STAT_MAX];       /* starting stats                  */
    sh_int max_stats[STAT_MAX];   /* maximum stats                   */
    sh_int size;                  /* aff bits for the race           */
};

struct spec_type {
    char * name;         /* special function name */
    SPEC_FUN * function; /* the function          */
};

/* Data structure for notes. */
struct note_data {
    NOTE_DATA *next;
    sh_int type;
    char * sender;
    char * date;
    char * to_list;
    char * subject;
    char * text;
    time_t date_stamp;
    time_t expire;
    OBJ_RECYCLE_DATA rec_data;
};

/* An affect.  */
struct affect_data {
    AFFECT_DATA *next;
    sh_int bit_type;
    sh_int type;
    sh_int level;
    sh_int duration;
    sh_int apply;
    sh_int modifier;
    flag_t bits;
    OBJ_RECYCLE_DATA rec_data;
};

/* A kill structure (indexed by level). */
struct kill_data {
    sh_int number;
    sh_int killed;
};

struct flag_type {
    char *name;
    flag_t bit;
    bool settable;
};

struct sector_type {
    int type;
    const char *name;
    int move_loss;
    char colour_char;
};

struct clan_type {
    char *name;
    char *who_name;
    sh_int hall;
    bool independent; /* true for loners */
};

struct position_type {
    int pos;
    char *long_name;
    char *name;
};

struct sex_type {
    int sex;
    char *name;
};

struct size_type {
    int size;
    char *name;
};

struct door_type {
    int dir;
    const char *name;
    const char *from;
    const char *to_phrase;
    int reverse;
};

/* Prototype for a mob.
 * This is the in-memory version of #MOBILES. */
struct mob_index_data {
    MOB_INDEX_DATA *next;
    SPEC_FUN *spec_fun;
    SHOP_DATA *pShop;
    MPROG_LIST *mprogs;
    AREA_DATA *area; /* OLC */
    sh_int vnum, anum;
    sh_int group;
    bool new_format;
    sh_int count;
    sh_int killed;
    char * name;
    char * short_descr;
    char * long_descr;
    char * description;
    flag_t mob_orig, mob;
    flag_t affected_by_orig, affected_by;
    sh_int alignment;
    sh_int level;
    sh_int hitroll;
    sh_int hit[3];
    sh_int mana[3];
    sh_int damage[3];
    sh_int ac[4];
    char *dam_type_str;
    sh_int dam_type;
    flag_t off_flags_orig, off_flags;
    flag_t imm_flags_orig, imm_flags;
    flag_t res_flags_orig, res_flags;
    flag_t vuln_flags_orig, vuln_flags;
    char *start_pos_str;
    char *default_pos_str;
    char *sex_str;
    char *race_str;
    char *size_str;
    char *material_str;
    sh_int start_pos;
    sh_int default_pos;
    sh_int sex;
    sh_int race;
    long wealth;
    flag_t form_orig, form;
    flag_t parts_orig, parts;
    sh_int size;
    sh_int material;
    flag_t mprog_flags;
    OBJ_RECYCLE_DATA rec_data;
};

/* memory for mobs */
struct mem_data {
    int id;
    int reaction;
    time_t when;
    OBJ_RECYCLE_DATA rec_data;
};

/* One character (PC or NPC). */
struct char_data {
    CHAR_DATA *next;
    CHAR_DATA *next_in_room;
    CHAR_DATA *master;
    CHAR_DATA *leader;
    CHAR_DATA *fighting;
    CHAR_DATA *reply;
    CHAR_DATA *pet;
    CHAR_DATA *mprog_target;
    MEM_DATA *memory;
    SPEC_FUN *spec_fun;
    MOB_INDEX_DATA *pIndexData;
    DESCRIPTOR_DATA *desc;
    AFFECT_DATA *affected;
    OBJ_DATA *carrying;
    OBJ_DATA *on;
    ROOM_INDEX_DATA *in_room;
    ROOM_INDEX_DATA *was_in_room;
    AREA_DATA *zone;
    PC_DATA *pcdata;
    GEN_DATA *gen_data;
    char *name;
    long id;
    sh_int version;
    char *short_descr;
    char *long_descr;
    char *description;
    char *prompt;
    char *prefix;
    sh_int group;
    sh_int clan;
    sh_int sex;
    sh_int class;
    sh_int race;
    sh_int level;
    sh_int trust;
    int played;
    int lines; /* for the pager */
    time_t logon;
    sh_int timer;
    sh_int wait;
    sh_int daze;
    sh_int hit;
    sh_int max_hit;
    sh_int mana;
    sh_int max_mana;
    sh_int move;
    sh_int max_move;
    long gold;
    long silver;
    int exp;
    flag_t mob;
    flag_t plr;
    flag_t comm;   /* RT added to pad the vector */
    flag_t wiznet; /* wiz stuff */
    flag_t imm_flags;
    flag_t res_flags;
    flag_t vuln_flags;
    sh_int invis_level;
    sh_int incog_level;
    flag_t affected_by;
    sh_int position;
    sh_int practice;
    sh_int train;
    sh_int carry_weight;
    sh_int carry_number;
    sh_int saving_throw;
    sh_int alignment;
    sh_int hitroll;
    sh_int damroll;
    sh_int armor[4];
    sh_int wimpy;

    /* stats */
    sh_int perm_stat[STAT_MAX];
    sh_int mod_stat[STAT_MAX];

    /* parts stuff */
    flag_t form;
    flag_t parts;
    sh_int size;
    sh_int material;

    /* mobile stuff */
    flag_t off_flags;
    sh_int damage[3];
    sh_int dam_type;
    sh_int start_pos;
    sh_int default_pos;
    sh_int mprog_delay;

    /* temporary data for per-second stat regen */
    sh_int gain_hit_remainder;
    sh_int gain_mana_remainder;
    sh_int gain_move_remainder;
    OBJ_RECYCLE_DATA rec_data;
};

/* Colour settings */
struct colour_setting_type {
    int index;
    char *name;
    char act_char;
    flag_t default_colour;
};

/* Colour definition */
struct colour_type {
    flag_t mask;
    flag_t code;
    char *name;
};

/* Lookup information */
struct map_lookup_table {
    int index;
    char *name;
    const FLAG_TYPE *flags;
};

/* Data which only PC's have. */
struct pc_data {
    BUFFER *buffer;
    char *pwd;
    char *bamfin;
    char *bamfout;
    char *title;
    sh_int perm_hit;
    sh_int perm_mana;
    sh_int perm_move;
    sh_int true_sex;
    int last_level;
    sh_int condition[4];
    sh_int learned[SKILL_MAX];
    bool group_known[GROUP_MAX];
    sh_int points;
    bool confirm_delete;
    char *alias[MAX_ALIAS];
    char *alias_sub[MAX_ALIAS];
    BOARD_DATA *board;           /* The current board        */
    time_t last_note[BOARD_MAX]; /* last note for the boards */
    NOTE_DATA *in_progress;
    int security;                /* OLC - Builder security */
    flag_t colour[COLOUR_MAX];

#ifdef IMC
    IMC_CHARDATA *imcchardata;
#endif
    OBJ_RECYCLE_DATA rec_data;
};

/* Data for generating characters -- only used during generation */
struct gen_data {
    bool skill_chosen[SKILL_MAX];
    bool group_chosen[GROUP_MAX];
    int points_chosen;
    OBJ_RECYCLE_DATA rec_data;
};

struct liq_type {
    char * name;
    char * color;
    sh_int affect[5];
};

/* Extra description data for a room or object. */
struct extra_descr_data {
    EXTRA_DESCR_DATA *next;
    char * keyword;          /* Keyword in look/examine */
    char * description;      /* What to see             */
    OBJ_RECYCLE_DATA rec_data;
};

/* Prototype for an object. */
struct obj_index_data {
    OBJ_INDEX_DATA *next;
    EXTRA_DESCR_DATA *extra_descr;
    AFFECT_DATA *affected;
    AREA_DATA *area; /* OLC */
    bool new_format;
    char *name;
    char *short_descr;
    char *description;
    sh_int vnum, anum;
    sh_int reset_num;
    char *material_str;
    char *item_type_str;
    sh_int material;
    sh_int item_type;
    flag_t extra_flags;
    flag_t wear_flags;
    sh_int level;
    sh_int condition;
    sh_int count;
    sh_int weight;
    int cost;
    int value[OBJ_VALUE_MAX];
    OBJ_RECYCLE_DATA rec_data;
};

/* Object stat <-> value[] mapping. */
struct obj_map_value {
    int index;
    char *name;
    int type, sub_type;
};

struct obj_map {
    int item_type;
    const struct obj_map_value values[OBJ_VALUE_MAX];
};

/* One object. */
struct obj_data {
    OBJ_DATA *next;
    OBJ_DATA *next_content;
    OBJ_DATA *contains;
    OBJ_DATA *in_obj;
    OBJ_DATA *on;
    CHAR_DATA *carried_by;
    EXTRA_DESCR_DATA *extra_descr;
    AFFECT_DATA *affected;
    OBJ_INDEX_DATA *pIndexData;
    ROOM_INDEX_DATA *in_room;
    bool enchanted;
    char *owner;
    char *name;
    char *short_descr;
    char *description;
    sh_int item_type;
    flag_t extra_flags;
    flag_t wear_flags;
    sh_int wear_loc;
    sh_int weight;
    int cost;
    sh_int level;
    sh_int condition;
    sh_int material;
    sh_int timer;
    int value[5];
    OBJ_RECYCLE_DATA rec_data;
};

/* Exit data. */
struct exit_data {
    ROOM_INDEX_DATA *to_room;
    sh_int vnum, area_vnum, room_anum;
    flag_t exit_flags;
    sh_int key;
    char *keyword;
    char *description;
    flag_t rs_flags;  /* OLC */
    int orig_door;    /* OLC */
    PORTAL_EXIT_TYPE *portal;
    OBJ_RECYCLE_DATA rec_data;
};

/* Reset commands:
 *   '*': comment
 *   'M': read a mobile
 *   'O': read an object
 *   'P': put object in object
 *   'G': give object to mobile
 *   'E': equip object to mobile
 *   'D': set state of door
 *   'R': randomize room exits
 *   'S': stop (end of list) */

/* Area-reset definition. */
struct reset_data {
    RESET_DATA *next;
    AREA_DATA *area;
    char command;
    sh_int value[5];
    OBJ_RECYCLE_DATA rec_data;
};

/* Area definition.  */
struct area_data {
    AREA_DATA *next;
    HELP_AREA *helps;
    char *name;
    char *filename;
    char *title;
    char *credits;
    sh_int age;
    sh_int nplayer;
    sh_int low_range;
    sh_int high_range;
    sh_int min_vnum;
    sh_int max_vnum;
    bool empty;
    char *builders;    /* OLC - Listing of */
    int vnum;          /* OLC - Area vnum  */
    flag_t area_flags; /* OLC              */
    int security;      /* OLC - Value 1-9  */
    int portal_count;
    OBJ_RECYCLE_DATA rec_data;
};

/* Room type. */
struct room_index_data {
    ROOM_INDEX_DATA *next;
    CHAR_DATA *people;
    OBJ_DATA *contents;
    EXTRA_DESCR_DATA *extra_descr;
    AREA_DATA *area;
    EXIT_DATA *exit[6];
    RESET_DATA *reset_first; /* OLC */
    RESET_DATA *reset_last;  /* OLC */
    char *name;
    char *description;
    char *owner;
    sh_int vnum, anum;
    flag_t room_flags;
    sh_int light;
    sh_int sector_type;
    sh_int heal_rate;
    sh_int mana_rate;
    sh_int clan;
    char *clan_str;
    PORTAL_EXIT_TYPE *portal;
    OBJ_RECYCLE_DATA rec_data;
};

/* Skills include spells as a particular case. */
struct skill_type {
    char *name;                    /* Name of skill               */
    sh_int skill_level[CLASS_MAX]; /* Level needed by class       */
    sh_int rating[CLASS_MAX];      /* How hard it is to learn     */
    SPELL_FUN *spell_fun;          /* Spell pointer (for spells)  */
    sh_int target;                 /* Legal targets               */
    sh_int minimum_position;       /* Position for caster / user  */
    sh_int *pgsn;                  /* Pointer to associated gsn   */
    sh_int slot;                   /* Slot for #OBJECT loading    */
    sh_int min_mana;               /* Minimum mana used           */
    sh_int beats;                  /* Waiting time after use      */
    char *noun_damage;             /* Damage message              */
    char *msg_off;                 /* Wear off message            */
    char *msg_obj;                 /* Wear off message for obects */
};

struct  group_type {
    char * name;
    sh_int rating[CLASS_MAX];
    char * spells[MAX_IN_GROUP];
};

struct mprog_list {
    MPROG_LIST *next;
    int trig_type;
    char *trig_phrase;
    AREA_DATA *area;
    sh_int vnum, anum;
    char *code;
    OBJ_RECYCLE_DATA rec_data;
};

struct mprog_code {
    MPROG_CODE *next;
    AREA_DATA *area;
    sh_int vnum, anum;
    char * code;
    OBJ_RECYCLE_DATA rec_data;
};

struct nanny_handler {
    int state;
    char *name;
    NANNY_FUN *const action;
};

struct furniture_bits {
    int position;
    char *name;
    flag_t bit_at;
    flag_t bit_on;
    flag_t bit_in;
};

/* Structure for a social in the socials table. */
struct social_type {
    char name[20];
    char *char_no_arg;
    char *others_no_arg;
    char *char_found;
    char *others_found;
    char *vict_found;
    char *char_not_found;
    char *char_auto;
    char *others_auto;
    OBJ_RECYCLE_DATA rec_data;
};

/* Data about a board */
struct board_data {
    char *name;       /* Max 8 chars */
    char *long_name;  /* Explanatory text, should be no more than 40 ? chars */

    int read_level;   /* minimum level to see board */
    int write_level;  /* minimum level to post notes */

    char *names;      /* Default recipient */
    int force_type;   /* Default action (DEF_XXX) */

    int purge_days;  /* Default expiration */

    /* Non-constant data */
    BOARD_DATA *next;
    NOTE_DATA *note_first; /* pointer to board's first note */
    bool changed;          /* currently unused */

};

/* Things we can wear, wield, hold, etc. */
struct wear_type {
    int type;
    const char *name;
    const char *look_msg;
    flag_t wear_loc;
};

/* Material types - currently unused. */
struct material_type {
    int type;
    const char *name;
    char color;
};

struct flag_stat_type {
    const FLAG_TYPE *structure;
    bool stat;
};

struct table_type {
    const void *table;
    const char *name;
    flag_t flags;
    const char *description;

    size_t type_size;
    TABLE_JSON_FUN *json_write_func; /* TODO: write me */
    TABLE_JSON_FUN *json_read_func;
};

struct portal_exit_type {
    ROOM_INDEX_DATA *room;
    char *name;
    int dir;
    OBJ_RECYCLE_DATA rec_data;
};

struct portal_type {
    char *name_from, *name_to;
    PORTAL_EXIT_TYPE *from, *to;
    PORTAL_TYPE *opposite;
    OBJ_RECYCLE_DATA rec_data;
};

struct affect_bit_type {
    char *name;
    int type;
    const FLAG_TYPE *flags;
    char *help;
};

struct day_type {
    int type;
    const char *name;
};

struct month_type {
    int type;
    const char *name;
};

#endif
