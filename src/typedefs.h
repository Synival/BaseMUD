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

#ifndef __ROM_TYPEDEFS_H
#define __ROM_TYPEDEFS_H

#include "merc.h"

/* Simple types. */
typedef long int flag_t;
typedef int type_t;

/* Structure types. */
typedef struct affect_data      AFFECT_T;
typedef struct area_data        AREA_T;
typedef struct ban_data         BAN_T;
typedef struct buf_type         BUFFER_T;
typedef struct char_data        CHAR_T;
typedef struct descriptor_data  DESCRIPTOR_T;
typedef struct exit_data        EXIT_T;
typedef struct extra_descr_data EXTRA_DESCR_T;
typedef struct help_data        HELP_T;
typedef struct help_area_data   HELP_AREA_T;
typedef struct kill_data        KILL_T;
typedef struct mem_data         MEM_T;
typedef struct mob_index_data   MOB_INDEX_T;
typedef struct note_data        NOTE_T;
typedef struct obj_data         OBJ_T;
typedef struct obj_map          OBJ_MAP_T;
typedef struct obj_map_value    OBJ_MAP_VALUE_T;
typedef struct obj_index_data   OBJ_INDEX_T;
typedef struct pc_data          PC_T;
typedef struct gen_data         GEN_T;
typedef struct reset_data       RESET_T;
typedef struct room_index_data  ROOM_INDEX_T;
typedef struct shop_data        SHOP_T;
typedef struct time_info_data   TIME_INFO_T;
typedef struct weather_data     WEATHER_T;
typedef struct mprog_list       MPROG_LIST_T;
typedef struct mprog_code       MPROG_CODE_T;
typedef struct nanny_handler    NANNY_HANDLER_T;
typedef struct furniture_bits   FURNITURE_BITS_T;
typedef struct colour_type      COLOUR_T;
typedef struct colour_setting_type COLOUR_SETTING_T;
typedef struct map_lookup_table MAP_LOOKUP_TABLE_T;
typedef struct liq_type         LIQ_T;
typedef struct skill_type       SKILL_T;
typedef struct skill_class_type SKILL_CLASS_T;
typedef struct skill_group_type SKILL_GROUP_T;
typedef struct skill_map_type   SKILL_MAP_T;
typedef struct skill_group_class_type SKILL_GROUP_CLASS_T;
typedef struct weapon_type      WEAPON_T;
typedef struct item_type        ITEM_T;
typedef struct effect_type      EFFECT_T;
typedef struct dam_type         DAM_T;
typedef struct attack_type      ATTACK_T;
typedef struct wiznet_type      WIZNET_T;
typedef struct clan_type        CLAN_T;
typedef struct hp_cond_type     HP_COND_T;
typedef struct position_type    POSITION_T;
typedef struct sex_type         SEX_T;
typedef struct size_type        SIZE_T;
typedef struct sector_type      SECTOR_T;
typedef struct race_type        RACE_T;
typedef struct pc_race_type     PC_RACE_T;
typedef struct class_type       CLASS_T;
typedef struct skill_type       SKILL_T;
typedef struct spec_type        SPEC_T;
typedef struct flag_type        FLAG_T;
typedef struct ext_flag_def_type EXT_FLAG_DEF_T;
typedef struct type_type        TYPE_T;
typedef struct door_type        DOOR_T;
typedef struct str_app_type     STR_APP_T;
typedef struct int_app_type     INT_APP_T;
typedef struct wis_app_type     WIS_APP_T;
typedef struct dex_app_type     DEX_APP_T;
typedef struct con_app_type     CON_APP_T;
typedef struct social_type      SOCIAL_T;
typedef struct board_data       BOARD_T;
typedef struct wear_loc_type    WEAR_LOC_T;
typedef struct recycle_type     RECYCLE_T;
typedef struct obj_recycle_data OBJ_RECYCLE_T;
typedef struct material_type    MATERIAL_T;
typedef struct flag_stat_type   FLAG_STAT_T;
typedef struct table_type       TABLE_T;
typedef struct portal_exit_type PORTAL_EXIT_T;
typedef struct portal_type      PORTAL_T;
typedef struct cmd_type         CMD_T;
typedef struct affect_bit_type  AFFECT_BIT_T;
typedef struct day_type         DAY_T;
typedef struct month_type       MONTH_T;
typedef struct sky_type         SKY_T;
typedef struct sun_type         SUN_T;
typedef struct mob_cmd_type     MOB_CMD_T;
typedef struct anum_type        ANUM_T;
typedef struct dice_type        DICE_T;
typedef struct olc_cmd_type     OLC_CMD_T;
typedef struct editor_cmd_type  EDITOR_CMD_T;
typedef struct pose_type        POSE_T;
typedef struct song_type        SONG_T;
typedef struct ext_flags_type   EXT_FLAGS_T;
typedef struct ext_init_flags_type EXT_INIT_FLAGS_T;
typedef struct cond_type        COND_T;

/* JSON typedefs. */
typedef struct json_t JSON_T;
typedef struct json_read_t JSON_READ_T;
typedef double json_num;
typedef long int json_int;

/* Union types. */
typedef union obj_value_type   OBJ_VALUE_T;
typedef union reset_value_type RESET_VALUE_T;

/* Function types. */
typedef void DO_FUN     (CHAR_T *ch, char *argument);
typedef bool SPEC_FUN   (CHAR_T *ch);
typedef void SPELL_FUN  (int sn, int level, CHAR_T *ch, void *vo, int target,
                         const char *target_name);
typedef void EFFECT_FUN (void *vo, int level, int dam, int target);
typedef void NANNY_FUN  (DESCRIPTOR_T *d, char *argument);
typedef void *JSON_READ_FUN (const JSON_T *json, const char *obj_name);
typedef JSON_T *JSON_WRITE_FUN (const void *obj, const char *obj_name);
typedef bool OLC_FUN    (CHAR_T *ch, char *argument);
typedef int LOOKUP_FUN  (const char *name);
typedef void INIT_FUN (void *obj);
typedef void DISPOSE_FUN (void *obj);
typedef bool COND_FUN (const CHAR_T *ch);

/* Accommodate old non-Ansi compilers. */
#if defined(TRADITIONAL)
    #define const
    #define args(list)              ()
    #define DECLARE_DO_FUN(fun)     void fun()
    #define DECLARE_SPEC_FUN(fun)   bool fun()
    #define DECLARE_SPELL_FUN(fun)  void fun()
    #define DECLARE_EFFECT_FUN(fun) void fun()
    #define DECLARE_NANNY_FUN(fun)  void fun()
    #define DECLARE_OLC_FUN(fun)    bool fun()
    #define DECLARE_JSON_READ_FUN(fun) void *fun()
    #define DECLARE_JSON_WRITE_FUN(fun) JSON_T *fun()
    #define DECLARE_LOOKUP_FUN(fun) int fun()
    #define DECLARE_INIT_FUN(fun)   void fun()
    #define DECLARE_DISPOSE_FUN(fun) void fun()
    #define DECLARE_COND_FUN(fun)   bool fun()
#else
    #define args(list)              list
    #define DECLARE_DO_FUN(fun)     DO_FUN     fun
    #define DECLARE_SPEC_FUN(fun)   SPEC_FUN   fun
    #define DECLARE_SPELL_FUN(fun)  SPELL_FUN  fun
    #define DECLARE_EFFECT_FUN(fun) EFFECT_FUN fun
    #define DECLARE_NANNY_FUN(fun)  NANNY_FUN  fun
    #define DECLARE_OLC_FUN(fun)    OLC_FUN    fun
    #define DECLARE_JSON_READ_FUN(fun)  JSON_READ_FUN  fun
    #define DECLARE_JSON_WRITE_FUN(fun) JSON_WRITE_FUN fun
    #define DECLARE_LOOKUP_FUN(fun) LOOKUP_FUN fun
    #define DECLARE_INIT_FUN(fun)   INIT_FUN fun
    #define DECLARE_DISPOSE_FUN(fun) DISPOSE_FUN fun
    #define DECLARE_COND_FUN(fun)   COND_FUN fun
#endif

#endif
