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

#ifndef __ROM_STRUCTS_H
#define __ROM_STRUCTS_H

#include "merc.h"

/* Extended flags. */
/* NOTE: the type used should be determined by EXT_FLAGS_ELEMENT_SIZE,
 *       defined in 'defs.h'. If you want to use anything other than
 *       8-bit unsigned chars, please upgrade! */
struct ext_flags_type {
    unsigned char bits[EXT_FLAGS_ARRAY_LENGTH];
};

struct ext_init_flags_type {
    int *bits;
};

/* Objects that can be instantiated, freed, recycled, and catalogued. */
struct recycle_type {
    int type;
    char *name;
    size_t size;
    size_t obj_data_off;
    size_t obj_name_off;
    INIT_FUN *const init_fun;
    DISPOSE_FUN *const dispose_fun;

    /* internal - do not set in definition table! */
    int top, list_count, free_count;
    OBJ_RECYCLE_T *list_front, *list_back;
    OBJ_RECYCLE_T *free_front, *free_back;
};

/* Data stored in individual objects to make recycling work. */
struct obj_recycle_data {
    void *obj;
    OBJ_RECYCLE_T *prev, *next;
    bool valid;
};

/* Structures, ahoy!*/
struct ban_data {
    BAN_T *next;
    flag_t ban_flags;
    sh_int level;
    char *name;
    OBJ_RECYCLE_T rec_data;
};

struct buf_type {
    sh_int state;  /* error state of the buffer */
    sh_int size;   /* size in k */
    char *string; /* buffer's string */
    OBJ_RECYCLE_T rec_data;
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
    DESCRIPTOR_T *next;
    DESCRIPTOR_T *snoop_by;
    CHAR_T *character;
    CHAR_T *original;
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
    int lines_written;  /* for the pager */
    void *olc_edit;     /* OLC */
    char **string_edit; /* OLC */
    int editor;         /* OLC */
    OBJ_RECYCLE_T rec_data;
};

/* Attribute bonus structures. */
struct str_app_type {
    int stat;
    sh_int tohit;
    sh_int todam;
    sh_int carry;
    sh_int wield;
    OBJ_RECYCLE_T rec_data;
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
    HELP_T *next;
    HELP_T *next_area;
    sh_int level;
    char *keyword;
    char *text;
    OBJ_RECYCLE_T rec_data;
};

struct help_area_data {
    HELP_AREA_T *next;
    HELP_T *first;
    HELP_T *last;
    char *area_str;
    AREA_T *area;
    char *filename;
    char *name;
    bool changed;
    OBJ_RECYCLE_T rec_data;
};

struct shop_data {
    SHOP_T *next;
    sh_int keeper;                /* Vnum of shop keeper mob     */
    sh_int buy_type [MAX_TRADE];  /* Item types shop will buy    */
    sh_int profit_buy;            /* Cost multiplier for buying  */
    sh_int profit_sell;           /* Cost multiplier for selling */
    sh_int open_hour;             /* First opening hour          */
    sh_int close_hour;            /* First closing hour          */
    OBJ_RECYCLE_T rec_data;
};

struct class_type {
    char *name;              /* the full name of the class  */
    char who_name[4];        /* Three-letter name for 'who' */
    sh_int attr_prime;       /* Prime attribute             */
    sh_int weapon;           /* First weapon                */
    sh_int guild[MAX_GUILD]; /* Vnum of guild rooms         */
    sh_int skill_adept;      /* Maximum skill level         */
    sh_int thac0_00;         /* Thac0 for level  0          */
    sh_int thac0_32;         /* Thac0 for level 32          */
    sh_int hp_min;           /* Min hp gained on leveling   */
    sh_int hp_max;           /* Max hp gained on leveling   */
    bool gains_mana;         /* Class gains mana on level   */
    char *base_group;        /* base skills gained          */
    char *default_group;     /* default skills gained       */
    bool can_sneak_away;     /* Can sneak away when fleeing */
};

struct item_type {
    int type;
    char *name;
};

struct weapon_type {
    sh_int type;
    char *name;
    char *skill;
    sh_int newbie_vnum;
    int skill_index; /* dynamically set */
};

struct wiznet_type {
    flag_t bit;
    char *name;
    int level;
};

struct effect_type {
    int type;
    char *name;
    EFFECT_FUN *effect_fun;
};

struct dam_type {
    int type;
    char *name;
    flag_t res;
    int effect;
    flag_t dam_flags;
};

struct attack_type {
    char *name;   /* name          */
    char *noun;   /* message       */
    int dam_type; /* type of DAM_T */
};

struct race_type {
    char *name;   /* call name of the race          */
    EXT_INIT_FLAGS_T ext_mob; /* act bits for the race          */
    flag_t aff;   /* aff bits for the race          */
    flag_t off;   /* off bits for the race          */
    flag_t imm;   /* imm bits for the race          */
    flag_t res;   /* res bits for the race          */
    flag_t vuln;  /* vuln bits for the race         */
    flag_t form;  /* default form flag for the race */
    flag_t parts; /* default parts for the race     */
};

struct pc_race_type {             /* additional data for pc races    */
    char *name;                   /* MUST be in race_type            */
    char who_name[8];
    sh_int creation_points;       /* cost in points of the race      */
    sh_int class_mult[CLASS_MAX]; /* exp multiplier for class, * 100 */
    char *skills[PC_RACE_SKILL_MAX]; /* bonus skills for the race    */
    sh_int stats[STAT_MAX];       /* starting stats                  */
    sh_int max_stats[STAT_MAX];   /* maximum stats                   */
    sh_int size;                  /* aff bits for the race           */
    sh_int bonus_max;             /* bonus to maximum stats          */
};

struct spec_type {
    char *name;         /* special function name */
    SPEC_FUN *function; /* the function          */
};

/* Data structure for notes. */
struct note_data {
    NOTE_T *next;
    sh_int type;
    char *sender;
    char *date;
    char *to_list;
    char *subject;
    char *text;
    time_t date_stamp;
    time_t expire;
    OBJ_RECYCLE_T rec_data;
};

/* An affect.  */
struct affect_data {
    AFFECT_T *next;
    sh_int bit_type;
    sh_int type;
    sh_int level;
    sh_int duration;
    sh_int apply;
    sh_int modifier;
    flag_t bits;
    OBJ_RECYCLE_T rec_data;
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

struct ext_flag_def_type {
    char *name;
    int bit;
    bool settable;
};

struct type_type {
    char *name;
    type_t type;
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

struct condition_type {
    int hp_percent;
    char *message;
};

struct position_type {
    int pos;
    char *long_name;
    char *name;
    char *room_msg;
    char *room_msg_furniture;
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
    const char *from_phrase;
    const char *to_phrase;
    int reverse;
    const char *short_name;
};

struct dice_type {
    sh_int number;
    sh_int size;
    sh_int bonus;
};

/* Prototype for a mob.
 * This is the in-memory version of #MOBILES. */
struct mob_index_data {
    MOB_INDEX_T *next;
    SPEC_FUN *spec_fun;
    SHOP_T *shop;
    MPROG_LIST_T *mprogs;
    char *area_str;
    AREA_T *area; /* OLC */
    sh_int vnum, anum;
    sh_int group;
    bool new_format;
    sh_int count;
    sh_int killed;
    char *name;
    char *short_descr;
    char *long_descr;
    char *description;
    sh_int alignment;
    sh_int level;
    sh_int hitroll;
    DICE_T hit;
    DICE_T mana;
    DICE_T damage;
    sh_int ac[4];
    sh_int attack_type;
    sh_int start_pos;
    sh_int default_pos;
    sh_int sex;
    sh_int race;
    long wealth;
    sh_int size;
    sh_int material;
    flag_t mprog_flags;
    EXT_FLAGS_T ext_mob_plus, ext_mob_final, ext_mob_minus;
    flag_t affected_by_plus, affected_by_final, affected_by_minus;
    flag_t off_flags_plus,   off_flags_final,   off_flags_minus;
    flag_t imm_flags_plus,   imm_flags_final,   imm_flags_minus;
    flag_t res_flags_plus,   res_flags_final,   res_flags_minus;
    flag_t vuln_flags_plus,  vuln_flags_final,  vuln_flags_minus;
    flag_t form_plus,        form_final,        form_minus;
    flag_t parts_plus,       parts_final,       parts_minus;
    OBJ_RECYCLE_T rec_data;
};

/* memory for mobs */
struct mem_data {
    int id;
    int reaction;
    time_t when;
    OBJ_RECYCLE_T rec_data;
};

/* One character (PC or NPC). */
struct char_data {
    CHAR_T *next;
    CHAR_T *next_in_room;
    CHAR_T *master;
    CHAR_T *leader;
    CHAR_T *fighting;
    CHAR_T *reply;
    CHAR_T *pet;
    CHAR_T *mprog_target;
    MEM_T *memory;
    SPEC_FUN *spec_fun;
    MOB_INDEX_T *index_data;
    DESCRIPTOR_T *desc;
    AFFECT_T *affected;
    OBJ_T *carrying;
    OBJ_T *on;
    ROOM_INDEX_T *in_room;
    ROOM_INDEX_T *was_in_room;
    AREA_T *zone;
    PC_T *pcdata;
    GEN_T *gen_data;
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
    EXT_FLAGS_T ext_mob;
    EXT_FLAGS_T ext_plr;
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
    DICE_T damage;
    sh_int attack_type;
    sh_int start_pos;
    sh_int default_pos;
    sh_int mprog_delay;

    /* temporary data for per-second stat regen */
    sh_int gain_hit_remainder;
    sh_int gain_mana_remainder;
    sh_int gain_move_remainder;
    OBJ_RECYCLE_T rec_data;
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
    const FLAG_T *flags;
};

/* Data which only PC's have. */
struct pc_data {
    BUFFER_T *buffer;
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
    sh_int skill_known[SKILL_MAX];
    sh_int group_known[SKILL_GROUP_MAX];
    sh_int creation_points;
    bool confirm_delete;
    char *alias[MAX_ALIAS];
    char *alias_sub[MAX_ALIAS];
    BOARD_T *board;              /* The current board        */
    time_t last_note[BOARD_MAX]; /* last note for the boards */
    NOTE_T *in_progress;
    int security;                /* OLC - Builder security */
    flag_t colour[COLOUR_SETTING_MAX];

#ifdef IMC
    IMC_CHARDATA *imcchardata;
#endif
    OBJ_RECYCLE_T rec_data;
};

/* Data for generating characters -- only used during generation */
struct gen_data {
    bool skill_chosen[SKILL_MAX];
    bool group_chosen[SKILL_GROUP_MAX];
    OBJ_RECYCLE_T rec_data;
};

struct liq_type {
    char *name;
    char *color;
    sh_int cond[COND_MAX];
    sh_int serving_size;
};

/* Extra description data for a room or object. */
struct extra_descr_data {
    EXTRA_DESCR_T *next;
    char *keyword;          /* Keyword in look/examine */
    char *description;      /* What to see             */
    OBJ_RECYCLE_T rec_data;
};

/* Object values for all item types. */
struct obj_values_weapon {
    flag_t weapon_type;
    flag_t dice_num;
    flag_t dice_size;
    flag_t attack_type;
    flag_t flags;
};

struct obj_values_container {
    flag_t capacity;
    flag_t flags;
    flag_t key;
    flag_t max_weight;
    flag_t weight_mult;
};

struct obj_values_drink_con {
    flag_t capacity;
    flag_t filled;
    flag_t liquid;
    flag_t poisoned;
    flag_t _value5;
};

struct obj_values_fountain {
    flag_t capacity;
    flag_t filled;
    flag_t liquid;
    flag_t poisoned;
    flag_t _value5;
};

struct obj_values_wand {
    flag_t level;
    flag_t recharge;
    flag_t charges;
    flag_t skill;
    flag_t _value5;
};

struct obj_values_staff {
    flag_t level;
    flag_t recharge;
    flag_t charges;
    flag_t skill;
    flag_t _value5;
};

struct obj_values_food {
    flag_t hunger;
    flag_t fullness;
    flag_t _value3;
    flag_t poisoned;
    flag_t _value5;
};

struct obj_values_money {
    flag_t silver;
    flag_t gold;
    flag_t _value_3;
    flag_t _value_4;
    flag_t _value_5;
};

struct obj_values_armor {
    flag_t vs_pierce;
    flag_t vs_bash;
    flag_t vs_slash;
    flag_t vs_magic;
    flag_t _value_5;
};

struct obj_values_potion {
    flag_t level;
    flag_t skill[POTION_SKILL_MAX];
};

struct obj_values_pill {
    flag_t level;
    flag_t skill[PILL_SKILL_MAX];
};

struct obj_values_scroll {
    flag_t level;
    flag_t skill[SCROLL_SKILL_MAX];
};

struct obj_values_map {
    flag_t persist;
    flag_t _value_2;
    flag_t _value_3;
    flag_t _value_4;
    flag_t _value_5;
};

struct obj_values_furniture {
    flag_t max_people;
    flag_t max_weight;
    flag_t flags;
    flag_t heal_rate;
    flag_t mana_rate;
};

struct obj_values_light {
    flag_t _value_1;
    flag_t _value_2;
    flag_t duration;
    flag_t _value_4;
    flag_t _value_5;
};

struct obj_values_portal {
    flag_t charges;
    flag_t exit_flags;
    flag_t gate_flags;
    flag_t to_vnum;
    flag_t key;
};

struct obj_values_jukebox {
    flag_t line;
    flag_t song;
    flag_t queue[JUKEBOX_QUEUE_MAX];
};

union obj_value_type {
    flag_t value[OBJ_VALUE_MAX];
    struct obj_values_weapon    weapon;
    struct obj_values_container container;
    struct obj_values_drink_con drink_con;
    struct obj_values_fountain  fountain;
    struct obj_values_wand      wand;
    struct obj_values_staff     staff;
    struct obj_values_food      food;
    struct obj_values_money     money;
    struct obj_values_armor     armor;
    struct obj_values_potion    potion;
    struct obj_values_pill      pill;
    struct obj_values_scroll    scroll;
    struct obj_values_map       map;
    struct obj_values_furniture furniture;
    struct obj_values_light     light;
    struct obj_values_portal    portal;
    struct obj_values_jukebox   jukebox;
};

/* Reset values for all reset types. */
struct reset_values_mob {
    sh_int _value1;
    sh_int mob_vnum;
    sh_int global_limit;
    sh_int room_vnum;
    sh_int room_limit;
};

struct reset_values_obj {
    sh_int room_limit;
    sh_int obj_vnum;
    sh_int global_limit;
    sh_int room_vnum;
    sh_int _value5;
};

struct reset_values_give {
    sh_int _value1;
    sh_int obj_vnum;
    sh_int global_limit;
    sh_int _value4;
    sh_int _value5;
};

struct reset_values_equip {
    sh_int _value1;
    sh_int obj_vnum;
    sh_int global_limit;
    sh_int wear_loc;
    sh_int _value5;
};

struct reset_values_put {
    sh_int _value1;
    sh_int obj_vnum;
    sh_int global_limit;
    sh_int into_vnum;
    sh_int put_count;
};

struct reset_values_door {
    sh_int _value1;
    sh_int room_vnum;
    sh_int dir;
    sh_int locks;
    sh_int _value5;
};

struct reset_values_randomize {
    sh_int _value1;
    sh_int room_vnum;
    sh_int dir_count;
    sh_int _value4;
    sh_int _value5;
};

union reset_value_type {
    sh_int value[RESET_VALUE_MAX];
    struct reset_values_mob       mob;
    struct reset_values_obj       obj;
    struct reset_values_give      give;
    struct reset_values_equip     equip;
    struct reset_values_put       put;
    struct reset_values_randomize randomize;
    struct reset_values_door      door;
};

/* Prototype for an object. */
struct obj_index_data {
    OBJ_INDEX_T *next;
    EXTRA_DESCR_T *extra_descr;
    AFFECT_T *affected;
    char *area_str;
    AREA_T *area; /* OLC */
    bool new_format;
    char *name;
    char *short_descr;
    char *description;
    sh_int vnum, anum;
    sh_int reset_num;
    sh_int material;
    sh_int item_type;
    flag_t extra_flags;
    flag_t wear_flags;
    sh_int level;
    sh_int condition;
    sh_int count;
    sh_int weight;
    int cost;
    OBJ_RECYCLE_T rec_data;
    OBJ_VALUE_T v;
};

/* Object stat <-> value[] mapping. */
struct obj_map_value {
    int index;
    flag_t default_value;
    char *name;
    int type, sub_type;
};

struct obj_map {
    int item_type;
    const struct obj_map_value values[OBJ_VALUE_MAX];
};

/* One object. */
struct obj_data {
    OBJ_T *next;
    OBJ_T *next_content;
    OBJ_T *contains;
    OBJ_T *in_obj;
    OBJ_T *on;
    CHAR_T *carried_by;
    EXTRA_DESCR_T *extra_descr;
    AFFECT_T *affected;
    OBJ_INDEX_T *index_data;
    ROOM_INDEX_T *in_room;
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
    OBJ_RECYCLE_T rec_data;
    OBJ_VALUE_T v;
};

/* Exit data. */
struct exit_data {
    ROOM_INDEX_T *to_room;
    sh_int vnum, area_vnum, room_anum;
    flag_t exit_flags;
    sh_int key;
    char *keyword;
    char *description;
    flag_t rs_flags;  /* OLC */
    int orig_door;    /* OLC */
    PORTAL_EXIT_T *portal;
    OBJ_RECYCLE_T rec_data;
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
    RESET_T *next;
    AREA_T *area;
    char command;
    int room_vnum;
    RESET_VALUE_T v;
    OBJ_RECYCLE_T rec_data;
};

/* Area definition.  */
struct area_data {
    AREA_T *next;
    HELP_AREA_T *helps;
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
    OBJ_RECYCLE_T rec_data;
};

/* Room type. */
struct room_index_data {
    ROOM_INDEX_T *next;
    CHAR_T *people;
    OBJ_T *contents;
    EXTRA_DESCR_T *extra_descr;
    char *area_str;
    AREA_T *area;
    EXIT_T *exit[DIR_MAX];
    RESET_T *reset_first; /* OLC */
    RESET_T *reset_last;  /* OLC */
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
    PORTAL_EXIT_T *portal;
    OBJ_RECYCLE_T rec_data;
};

struct skill_class_type {
    sh_int level;  /* Level needed by class       */
    sh_int effort; /* How hard it is to learn     */
};

/* Skills include spells as a particular case. */
struct skill_type {
    char *name;                    /* Name of skill               */
    SKILL_CLASS_T classes[CLASS_MAX]; /* Restrictions based on class */
    SPELL_FUN *spell_fun;          /* Spell pointer (for spells)  */
    sh_int target;                 /* Legal targets               */
    sh_int minimum_position;       /* Position for caster / user  */
    sh_int slot;                   /* Slot for #OBJECT loading    */
    sh_int min_mana;               /* Minimum mana used           */
    sh_int beats;                  /* Waiting time after use      */
    char *noun_damage;             /* Damage message              */
    char *msg_off;                 /* Wear off message            */
    char *msg_obj;                 /* Wear off message for obects */
    int map_index;                 /* Dynamically set             */
    int weapon_index;              /* Dynamically set             */
};

struct skill_group_class_type {
    sh_int cost; /* How hard it is to learn */
};

struct skill_group_type {
    char *name;
    SKILL_GROUP_CLASS_T classes[CLASS_MAX];
    char *spells[MAX_IN_GROUP];
};

struct skill_map_type {
    int map_index;
    char *name;
    int skill_index; /* dynamically set */
};

struct mprog_list {
    MPROG_LIST_T *next;
    int trig_type;
    char *trig_phrase;
    AREA_T *area;
    sh_int vnum, anum;
    char *code;
    OBJ_RECYCLE_T rec_data;
};

struct mprog_code {
    MPROG_CODE_T *next;
    AREA_T *area;
    sh_int vnum, anum;
    char *code;
    OBJ_RECYCLE_T rec_data;
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
    char *name;
    char *char_no_arg;
    char *others_no_arg;
    char *char_found;
    char *others_found;
    char *vict_found;
    char *char_not_found;
    char *char_auto;
    char *others_auto;
    int min_pos;
    OBJ_RECYCLE_T rec_data;
};

/* Data about a board */
struct board_data {
    char *name;      /* Max 8 chars */
    char *long_name; /* Explanatory text, should be no more than 40 ? chars */
    int read_level;  /* minimum level to see board */
    int write_level; /* minimum level to post notes */
    char *names;     /* Default recipient */
    int force_type;  /* Default action (DEF_XXX) */
    int purge_days;  /* Default expiration */

    /* Non-constant data */
    BOARD_T *next;
    NOTE_T *note_first; /* pointer to board's first note */
    bool changed;       /* currently unused */

};

/* Things we can wear, wield, hold, etc. */
struct wear_loc_type {
    int type;
    const char *name;
    const char *phrase;
    const char *look_msg;
    flag_t wear_flag;
    int ac_bonus;
    const char *msg_wear_self, *msg_wear_room;
};

/* Material types - currently unused. */
struct material_type {
    int type;
    const char *name;
    char color;
};

struct flag_stat_type {
    const FLAG_T *structure;
    bool stat;
};

struct table_type {
    const void *table;
    const char *name;
    int type;
    const char *description;

    size_t type_size, table_length;
    const char *obj_name;
    const char *json_path;
    JSON_WRITE_FUN *json_write_func;
    JSON_READ_FUN  *json_read_func;
    DISPOSE_FUN *dispose_fun;
};

struct portal_exit_type {
    ROOM_INDEX_T *room;
    char *name;
    int dir;
    OBJ_RECYCLE_T rec_data;
};

struct portal_type {
    char *name_from, *name_to;
    bool two_way, generated;
    PORTAL_EXIT_T *from, *to;
    OBJ_RECYCLE_T rec_data;
};

struct affect_bit_type {
    char *name;
    int type;
    const FLAG_T *flags;
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

struct sky_type {
    int type;
    const char *name;
    const char *description;
    int mmhg_min;
    int mmhg_max;
};

struct sun_type {
    int type;
    const char *name;
    bool is_dark;
    int hour_start;
    int hour_end;
    const char *message;
};

struct mob_cmd_type {
    const char *name;
    DO_FUN *do_fun;
};

struct anum_type {
    int type;
    sh_int *vnum_ref;
    char *area_str;
    int anum;
    ANUM_T *prev, *next;
};

/* Structure for a command in the command lookup table. */
struct cmd_type {
    char *const name;
    DO_FUN *do_fun;
    sh_int position;
    sh_int level;
    sh_int log;
    sh_int show;
};

/* Structure for an OLC editor command. */
struct olc_cmd_type {
    char *const name;
    OLC_FUN *olc_fun;
};

/* Structure for an OLC editor startup command. */
struct editor_cmd_type {
    char *const name;
    DO_FUN *do_fun;
};

/* All the posing stuff. */
struct pose_type {
    const char *class_name;
    const char *message[MAX_LEVEL * 2 + 2];
};

/* Music stuff. */
struct song_type {
    char *group;
    char *name;
    char *lyrics[MAX_SONG_LINES];
    int lines;
};

#endif
