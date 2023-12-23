DUMPMACHINE = $(shell gcc -dumpmachine)
ifneq (, $(findstring linux, $(DUMPMACHINE)))
    OS   = LINUX
    LIBS = -lcrypt
    EXE  = ../bin/basemud
else ifneq (, $(findstring mingw, $(DUMPMACHINE)))
    OS   = MINGW
    LIBS =
    EXE  = ../bin/basemud.exe
else
    OS   = UNKNOWN_OS
    LIBS =
    EXE  = ../bin/basemud
endif

CC      = gcc
PROF    = -g -O
C_FLAGS = -Wall -Wno-trigraphs -Wno-format-truncation -Werror $(PROF)
L_FLAGS = $(PROF)
SRC_PATH=.
OBJ_PATH=../obj
SRC_FILES = $(sort $(wildcard $(SRC_PATH)/*.c))
OBJ_FILES = $(SRC_FILES:./%.c=$(OBJ_PATH)/%.o)

all: $(EXE)

$(EXE): $(OBJ_FILES)
	$(CC) $(L_FLAGS) -o $(EXE) $(OBJ_FILES) $(LIBS)

depend:
	makedepend -Y. $(SRC_PATH)/*.c 2>/dev/null
	cat Makefile | sed -r "s/^\.\/(.*)\.o:/..\/obj\/\1.o:/" > Makefile.2 && mv Makefile.2 Makefile

$(OBJ_PATH)/%.o: $(SRC_PATH)/%.c
	$(CC) -c $(C_FLAGS) -c $< -o $@

clean:
	rm -f $(OBJ_FILES)
# DO NOT DELETE

../obj/act_board.o: ./string.h merc.h basemud.h compat.h defs.h typedefs.h
../obj/act_board.o: macros.h json.h structs.h types.h flags.h tables.h ext_flags.h
../obj/act_board.o: board.h comm.h recycle.h utils.h lookup.h interp.h act_info.h
../obj/act_board.o: chars.h memory.h globals.h act_board.h
../obj/act_comm.o: ./string.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
../obj/act_comm.o: json.h structs.h types.h flags.h tables.h ext_flags.h interp.h
../obj/act_comm.o: recycle.h lookup.h comm.h utils.h mob_prog.h db.h do_sub.h
../obj/act_comm.o: chars.h find.h globals.h players.h memory.h act_comm.h
../obj/act_conf.o: ./string.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
../obj/act_conf.o: json.h structs.h types.h flags.h tables.h ext_flags.h db.h
../obj/act_conf.o: interp.h colour.h groups.h comm.h utils.h lookup.h do_sub.h
../obj/act_conf.o: chars.h memory.h act_conf.h
../obj/act_fight.o: affects.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
../obj/act_fight.o: json.h structs.h types.h flags.h tables.h ext_flags.h utils.h
../obj/act_fight.o: comm.h lookup.h interp.h groups.h mob_prog.h recycle.h fight.h
../obj/act_fight.o: act_comm.h chars.h find.h players.h act_fight.h
../obj/act_group.o: interp.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
../obj/act_group.o: json.h structs.h types.h flags.h tables.h ext_flags.h groups.h
../obj/act_group.o: utils.h comm.h db.h chars.h find.h globals.h act_group.h
../obj/act_info.o: ./string.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
../obj/act_info.o: json.h structs.h types.h flags.h tables.h ext_flags.h interp.h
../obj/act_info.o: magic.h recycle.h lookup.h utils.h groups.h db.h fight.h
../obj/act_info.o: update.h comm.h save.h do_sub.h act_comm.h act_obj.h chars.h
../obj/act_info.o: rooms.h objs.h find.h spell_info.h globals.h memory.h items.h
../obj/act_info.o: players.h extra_descrs.h act_info.h
../obj/act_move.o: ./string.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
../obj/act_move.o: json.h structs.h types.h flags.h tables.h ext_flags.h interp.h
../obj/act_move.o: lookup.h utils.h comm.h mob_prog.h affects.h db.h fight.h
../obj/act_move.o: groups.h act_info.h chars.h rooms.h objs.h find.h globals.h
../obj/act_move.o: items.h players.h act_move.h
../obj/act_obj.o: interp.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
../obj/act_obj.o: json.h structs.h types.h flags.h tables.h ext_flags.h affects.h
../obj/act_obj.o: utils.h comm.h db.h fight.h groups.h mob_prog.h save.h magic.h
../obj/act_obj.o: act_group.h act_comm.h act_move.h recycle.h chars.h objs.h
../obj/act_obj.o: rooms.h find.h globals.h lookup.h items.h players.h act_obj.h
../obj/act_olc.o: ./string.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
../obj/act_olc.o: json.h structs.h types.h flags.h tables.h ext_flags.h recycle.h
../obj/act_olc.o: lookup.h interp.h utils.h db.h comm.h olc_save.h act_info.h
../obj/act_olc.o: chars.h globals.h items.h mobiles.h rooms.h objs.h mob_prog.h
../obj/act_olc.o: descs.h json_export.h resets.h memory.h portals.h olc_aedit.h
../obj/act_olc.o: olc_hedit.h olc_medit.h olc_mpedit.h olc_oedit.h olc_redit.h
../obj/act_olc.o: olc.h act_olc.h
../obj/act_player.o: ./string.h merc.h basemud.h compat.h defs.h typedefs.h
../obj/act_player.o: macros.h json.h structs.h types.h flags.h tables.h
../obj/act_player.o: ext_flags.h db.h comm.h save.h utils.h fight.h interp.h
../obj/act_player.o: recycle.h chars.h descs.h memory.h globals.h act_player.h
../obj/act_shop.o: magic.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
../obj/act_shop.o: json.h structs.h types.h flags.h tables.h ext_flags.h lookup.h
../obj/act_shop.o: comm.h interp.h utils.h db.h groups.h chars.h objs.h find.h
../obj/act_shop.o: act_comm.h spell_cure.h materials.h globals.h memory.h items.h
../obj/act_shop.o: mobiles.h players.h rooms.h act_shop.h
../obj/act_skills.o: ./string.h merc.h basemud.h compat.h defs.h typedefs.h
../obj/act_skills.o: macros.h json.h structs.h types.h flags.h tables.h
../obj/act_skills.o: ext_flags.h db.h utils.h comm.h interp.h magic.h fight.h
../obj/act_skills.o: lookup.h recycle.h act_comm.h chars.h find.h players.h
../obj/act_skills.o: memory.h act_skills.h
../obj/affects.o: ./string.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
../obj/affects.o: json.h structs.h types.h flags.h tables.h ext_flags.h recycle.h
../obj/affects.o: utils.h lookup.h chars.h affects.h
../obj/areas.o: globals.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
../obj/areas.o: json.h structs.h types.h flags.h tables.h ext_flags.h comm.h
../obj/areas.o: rooms.h utils.h recycle.h resets.h areas.h
../obj/ban.o: ./string.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
../obj/ban.o: json.h structs.h types.h flags.h tables.h ext_flags.h recycle.h
../obj/ban.o: utils.h fread.h interp.h comm.h chars.h globals.h memory.h fwrite.h
../obj/ban.o: ban.h
../obj/board.o: ./string.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
../obj/board.o: json.h structs.h types.h flags.h tables.h ext_flags.h interp.h
../obj/board.o: fread.h utils.h comm.h recycle.h lookup.h chars.h descs.h memory.h
../obj/board.o: globals.h board.h
../obj/boot.o: ./string.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
../obj/boot.o: json.h structs.h types.h flags.h tables.h ext_flags.h signal.h
../obj/boot.o: globals.h utils.h interp.h db.h descs.h recycle.h memory.h save.h
../obj/boot.o: chars.h olc.h nanny.h update.h comm.h rooms.h quickmud.h act_info.h
../obj/boot.o: boot.h
../obj/chars.o: ./string.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
../obj/chars.o: json.h structs.h types.h flags.h tables.h ext_flags.h utils.h
../obj/chars.o: comm.h objs.h affects.h lookup.h magic.h db.h groups.h fight.h
../obj/chars.o: interp.h recycle.h rooms.h mob_prog.h wiz_l6.h materials.h
../obj/chars.o: globals.h memory.h board.h update.h items.h players.h mobiles.h
../obj/chars.o: save.h act_move.h act_info.h act_group.h act_player.h chars.h
../obj/colour.o: ./string.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
../obj/colour.o: json.h structs.h types.h flags.h tables.h ext_flags.h lookup.h
../obj/colour.o: chars.h colour.h
../obj/comm.o: ./string.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
../obj/comm.o: json.h structs.h types.h flags.h tables.h ext_flags.h colour.h
../obj/comm.o: recycle.h utils.h fight.h interp.h db.h olc.h save.h mob_prog.h
../obj/comm.o: lookup.h act_info.h chars.h rooms.h objs.h descs.h globals.h
../obj/comm.o: players.h comm.h
../obj/db.o: ./string.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
../obj/db.o: json.h structs.h types.h flags.h tables.h ext_flags.h utils.h
../obj/db.o: recycle.h affects.h lookup.h json_import.h music.h ban.h board.h
../obj/db.o: portals.h rooms.h objs.h db_old.h globals.h memory.h items.h
../obj/db.o: mobiles.h skills.h fread.h areas.h mob_prog.h resets.h extra_descrs.h
../obj/db.o: help.h db.h
../obj/db_old.o: ./string.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
../obj/db_old.o: json.h structs.h types.h flags.h tables.h ext_flags.h db.h
../obj/db_old.o: recycle.h utils.h interp.h lookup.h olc.h affects.h globals.h
../obj/db_old.o: memory.h items.h fread.h mobiles.h objs.h rooms.h extra_descrs.h
../obj/db_old.o: db_old.h
../obj/descs.o: ./string.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
../obj/descs.o: json.h structs.h types.h flags.h tables.h ext_flags.h ban.h
../obj/descs.o: chars.h colour.h comm.h db.h recycle.h utils.h interp.h lookup.h
../obj/descs.o: globals.h memory.h descs.h
../obj/do_sub.o: comm.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
../obj/do_sub.o: json.h structs.h types.h flags.h tables.h ext_flags.h chars.h
../obj/do_sub.o: do_sub.h
../obj/effects.o: recycle.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
../obj/effects.o: json.h structs.h types.h flags.h tables.h ext_flags.h lookup.h
../obj/effects.o: db.h utils.h comm.h magic.h update.h affects.h objs.h chars.h
../obj/effects.o: items.h players.h effects.h
../obj/ext_flags.o: ./string.h merc.h basemud.h compat.h defs.h typedefs.h
../obj/ext_flags.o: macros.h json.h structs.h types.h flags.h tables.h ext_flags.h
../obj/ext_flags.o: interp.h lookup.h utils.h
../obj/extra_descrs.o: utils.h merc.h basemud.h compat.h defs.h typedefs.h
../obj/extra_descrs.o: macros.h json.h structs.h types.h flags.h tables.h
../obj/extra_descrs.o: ext_flags.h extra_descrs.h
../obj/fight.o: ./string.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
../obj/fight.o: json.h structs.h types.h flags.h tables.h ext_flags.h interp.h
../obj/fight.o: lookup.h update.h utils.h db.h effects.h mob_prog.h comm.h save.h
../obj/fight.o: groups.h magic.h affects.h act_fight.h act_obj.h act_comm.h
../obj/fight.o: act_move.h chars.h objs.h find.h globals.h memory.h items.h
../obj/fight.o: players.h mobiles.h fight.h
../obj/find.o: interp.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
../obj/find.o: json.h structs.h types.h flags.h tables.h ext_flags.h utils.h
../obj/find.o: chars.h db.h globals.h rooms.h find.h
../obj/flags.o: ./string.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
../obj/flags.o: json.h structs.h types.h flags.h tables.h ext_flags.h interp.h
../obj/flags.o: lookup.h utils.h
../obj/fread.o: ./string.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
../obj/fread.o: json.h structs.h types.h flags.h tables.h ext_flags.h utils.h
../obj/fread.o: memory.h globals.h fread.h
../obj/fwrite.o: ./string.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
../obj/fwrite.o: json.h structs.h types.h flags.h tables.h ext_flags.h save.h
../obj/fwrite.o: fwrite.h
../obj/globals.o: globals.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
../obj/globals.o: json.h structs.h types.h flags.h tables.h ext_flags.h
../obj/groups.o: utils.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
../obj/groups.o: json.h structs.h types.h flags.h tables.h ext_flags.h affects.h
../obj/groups.o: comm.h db.h chars.h globals.h recycle.h groups.h
../obj/help.o: help.h merc.h basemud.h compat.h defs.h typedefs.h macros.h json.h
../obj/help.o: structs.h types.h flags.h tables.h ext_flags.h
../obj/interp.o: ./string.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
../obj/interp.o: json.h structs.h types.h flags.h tables.h ext_flags.h utils.h
../obj/interp.o: comm.h db.h recycle.h chars.h find.h descs.h globals.h memory.h
../obj/interp.o: lookup.h act_board.h act_comm.h act_conf.h act_fight.h
../obj/interp.o: act_group.h act_info.h act_move.h act_obj.h act_player.h
../obj/interp.o: act_shop.h act_skills.h act_olc.h wiz_im.h wiz_l1.h wiz_l2.h
../obj/interp.o: wiz_l3.h wiz_l4.h wiz_l5.h wiz_l6.h wiz_l7.h wiz_l8.h wiz_ml.h
../obj/interp.o: olc.h mob_cmds.h interp.h
../obj/items.o: ./string.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
../obj/items.o: json.h structs.h types.h flags.h tables.h ext_flags.h utils.h
../obj/items.o: comm.h interp.h objs.h lookup.h chars.h affects.h magic.h db.h
../obj/items.o: recycle.h rooms.h mob_prog.h groups.h globals.h music.h players.h
../obj/items.o: fread.h fwrite.h act_info.h act_move.h act_group.h items.h
../obj/json.o: ./string.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
../obj/json.o: json.h structs.h types.h flags.h tables.h ext_flags.h utils.h
../obj/json.o: memory.h
../obj/json_export.o: areas.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
../obj/json_export.o: json.h structs.h types.h flags.h tables.h ext_flags.h
../obj/json_export.o: recycle.h utils.h json_objw.h json_write.h lookup.h
../obj/json_export.o: json_export.h
../obj/json_import.o: ./string.h merc.h basemud.h compat.h defs.h typedefs.h
../obj/json_import.o: macros.h json.h structs.h types.h flags.h tables.h
../obj/json_import.o: ext_flags.h utils.h recycle.h db.h lookup.h portals.h
../obj/json_import.o: globals.h memory.h json_objr.h help.h json_read.h rooms.h
../obj/json_import.o: mobiles.h objs.h json_import.h
../obj/json_objr.o: ./string.h merc.h basemud.h compat.h defs.h typedefs.h
../obj/json_objr.o: macros.h json.h structs.h types.h flags.h tables.h ext_flags.h
../obj/json_objr.o: json_import.h recycle.h memory.h lookup.h portals.h db.h
../obj/json_objr.o: globals.h rooms.h resets.h extra_descrs.h affects.h help.h
../obj/json_objr.o: json_objr.h
../obj/json_objw.o: ./string.h merc.h basemud.h compat.h defs.h typedefs.h
../obj/json_objw.o: macros.h json.h structs.h types.h flags.h tables.h ext_flags.h
../obj/json_objw.o: lookup.h utils.h json_objw.h
../obj/json_read.o: ./string.h merc.h basemud.h compat.h defs.h typedefs.h
../obj/json_read.o: macros.h json.h structs.h types.h flags.h tables.h ext_flags.h
../obj/json_read.o: utils.h json_read.h
../obj/json_tblr.o: ./string.h merc.h basemud.h compat.h defs.h typedefs.h
../obj/json_tblr.o: macros.h json.h structs.h types.h flags.h tables.h ext_flags.h
../obj/json_tblr.o: lookup.h colour.h memory.h json_import.h json_tblr.h
../obj/json_tblw.o: ./string.h merc.h basemud.h compat.h defs.h typedefs.h
../obj/json_tblw.o: macros.h json.h structs.h types.h flags.h tables.h ext_flags.h
../obj/json_tblw.o: lookup.h colour.h json_tblw.h
../obj/json_write.o: ./string.h merc.h basemud.h compat.h defs.h typedefs.h
../obj/json_write.o: macros.h json.h structs.h types.h flags.h tables.h
../obj/json_write.o: ext_flags.h utils.h json_write.h
../obj/lookup.o: ./string.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
../obj/lookup.o: json.h structs.h types.h flags.h tables.h ext_flags.h utils.h
../obj/lookup.o: interp.h db.h recycle.h globals.h lookup.h
../obj/magic.o: ./string.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
../obj/magic.o: json.h structs.h types.h flags.h tables.h ext_flags.h lookup.h
../obj/magic.o: affects.h comm.h fight.h utils.h chars.h magic.h
../obj/materials.o: ./string.h merc.h basemud.h compat.h defs.h typedefs.h
../obj/materials.o: macros.h json.h structs.h types.h flags.h tables.h ext_flags.h
../obj/materials.o: materials.h
../obj/memory.o: ./string.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
../obj/memory.o: json.h structs.h types.h flags.h tables.h ext_flags.h utils.h
../obj/memory.o: globals.h recycle.h memory.h
../obj/mob_cmds.o: lookup.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
../obj/mob_cmds.o: json.h structs.h types.h flags.h tables.h ext_flags.h interp.h
../obj/mob_cmds.o: groups.h utils.h comm.h fight.h mob_prog.h db.h chars.h objs.h
../obj/mob_cmds.o: rooms.h find.h act_info.h globals.h mobiles.h mob_cmds.h
../obj/mob_prog.o: ./string.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
../obj/mob_prog.o: json.h structs.h types.h flags.h tables.h ext_flags.h lookup.h
../obj/mob_prog.o: utils.h groups.h interp.h mob_cmds.h db.h chars.h objs.h find.h
../obj/mob_prog.o: globals.h mob_prog.h
../obj/mobiles.o: globals.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
../obj/mobiles.o: json.h structs.h types.h flags.h tables.h ext_flags.h utils.h
../obj/mobiles.o: recycle.h memory.h chars.h affects.h lookup.h fight.h interp.h
../obj/mobiles.o: act_fight.h magic.h objs.h items.h mob_prog.h comm.h db.h
../obj/mobiles.o: mobiles.h
../obj/music.o: recycle.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
../obj/music.o: json.h structs.h types.h flags.h tables.h ext_flags.h utils.h
../obj/music.o: comm.h interp.h globals.h lookup.h chars.h objs.h memory.h fread.h
../obj/music.o: music.h
../obj/nanny.o: ./string.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
../obj/nanny.o: json.h structs.h types.h flags.h tables.h ext_flags.h interp.h
../obj/nanny.o: recycle.h lookup.h utils.h comm.h db.h save.h ban.h fight.h
../obj/nanny.o: act_info.h act_skills.h act_board.h act_obj.h chars.h objs.h
../obj/nanny.o: descs.h globals.h memory.h magic.h players.h rooms.h nanny.h
../obj/objs.o: ./string.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
../obj/objs.o: json.h structs.h types.h flags.h tables.h ext_flags.h affects.h
../obj/objs.o: utils.h chars.h db.h recycle.h comm.h lookup.h materials.h
../obj/objs.o: globals.h memory.h items.h extra_descrs.h objs.h
../obj/olc.o: ./string.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
../obj/olc.o: json.h structs.h types.h flags.h tables.h ext_flags.h utils.h
../obj/olc.o: interp.h comm.h db.h recycle.h lookup.h magic.h chars.h memory.h
../obj/olc.o: act_olc.h olc_aedit.h olc_hedit.h olc_medit.h olc_mpedit.h
../obj/olc.o: olc_oedit.h olc_redit.h olc.h
../obj/olc_aedit.o: ./string.h merc.h basemud.h compat.h defs.h typedefs.h
../obj/olc_aedit.o: macros.h json.h structs.h types.h flags.h tables.h ext_flags.h
../obj/olc_aedit.o: comm.h lookup.h db.h recycle.h interp.h globals.h olc.h
../obj/olc_aedit.o: memory.h areas.h olc_aedit.h
../obj/olc_hedit.o: ./string.h merc.h basemud.h compat.h defs.h typedefs.h
../obj/olc_hedit.o: macros.h json.h structs.h types.h flags.h tables.h ext_flags.h
../obj/olc_hedit.o: comm.h lookup.h db.h recycle.h utils.h interp.h globals.h
../obj/olc_hedit.o: olc.h memory.h help.h olc_hedit.h
../obj/olc_medit.o: ./string.h merc.h basemud.h compat.h defs.h typedefs.h
../obj/olc_medit.o: macros.h json.h structs.h types.h flags.h tables.h ext_flags.h
../obj/olc_medit.o: comm.h lookup.h db.h recycle.h utils.h interp.h mob_cmds.h
../obj/olc_medit.o: chars.h globals.h olc.h memory.h mobiles.h mob_prog.h
../obj/olc_medit.o: olc_medit.h
../obj/olc_mpedit.o: ./string.h merc.h basemud.h compat.h defs.h typedefs.h
../obj/olc_mpedit.o: macros.h json.h structs.h types.h flags.h tables.h
../obj/olc_mpedit.o: ext_flags.h comm.h db.h recycle.h utils.h lookup.h chars.h
../obj/olc_mpedit.o: globals.h olc.h memory.h mob_prog.h olc_mpedit.h
../obj/olc_oedit.o: ./string.h merc.h basemud.h compat.h defs.h typedefs.h
../obj/olc_oedit.o: macros.h json.h structs.h types.h flags.h tables.h ext_flags.h
../obj/olc_oedit.o: comm.h lookup.h db.h recycle.h utils.h interp.h affects.h
../obj/olc_oedit.o: act_info.h chars.h globals.h olc.h memory.h items.h objs.h
../obj/olc_oedit.o: extra_descrs.h olc_oedit.h
../obj/olc_redit.o: ./string.h merc.h basemud.h compat.h defs.h typedefs.h
../obj/olc_redit.o: macros.h json.h structs.h types.h flags.h tables.h ext_flags.h
../obj/olc_redit.o: comm.h lookup.h db.h recycle.h utils.h interp.h act_info.h
../obj/olc_redit.o: chars.h objs.h find.h globals.h olc.h olc_medit.h olc_oedit.h
../obj/olc_redit.o: memory.h mobiles.h rooms.h extra_descrs.h resets.h portals.h
../obj/olc_redit.o: olc_redit.h
../obj/olc_save.o: ./string.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
../obj/olc_save.o: json.h structs.h types.h flags.h tables.h ext_flags.h lookup.h
../obj/olc_save.o: utils.h db.h mob_cmds.h comm.h interp.h globals.h olc.h items.h
../obj/olc_save.o: fread.h fwrite.h objs.h mobiles.h rooms.h mob_prog.h
../obj/olc_save.o: json_export.h olc_save.h
../obj/players.o: ./string.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
../obj/players.o: json.h structs.h types.h flags.h tables.h ext_flags.h chars.h
../obj/players.o: lookup.h objs.h affects.h memory.h utils.h globals.h comm.h
../obj/players.o: save.h magic.h groups.h fight.h rooms.h items.h players.h
../obj/portals.o: ./string.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
../obj/portals.o: json.h structs.h types.h flags.h tables.h ext_flags.h lookup.h
../obj/portals.o: recycle.h utils.h memory.h rooms.h portals.h
../obj/quickmud.o: utils.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
../obj/quickmud.o: json.h structs.h types.h flags.h tables.h ext_flags.h fread.h
../obj/quickmud.o: quickmud.h
../obj/recycle.o: ./string.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
../obj/recycle.o: json.h structs.h types.h flags.h tables.h ext_flags.h utils.h
../obj/recycle.o: lookup.h db.h affects.h objs.h chars.h globals.h memory.h
../obj/recycle.o: resets.h board.h extra_descrs.h mob_prog.h rooms.h mobiles.h
../obj/recycle.o: help.h portals.h recycle.h
../obj/resets.o: mobiles.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
../obj/resets.o: json.h structs.h types.h flags.h tables.h ext_flags.h utils.h
../obj/resets.o: rooms.h chars.h objs.h items.h recycle.h resets.h
../obj/rooms.o: db.h merc.h basemud.h compat.h defs.h typedefs.h macros.h json.h
../obj/rooms.o: structs.h types.h flags.h tables.h ext_flags.h lookup.h utils.h
../obj/rooms.o: objs.h interp.h chars.h globals.h mobiles.h items.h resets.h
../obj/rooms.o: recycle.h rooms.h
../obj/save.o: recycle.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
../obj/save.o: json.h structs.h types.h flags.h tables.h ext_flags.h lookup.h
../obj/save.o: colour.h db.h utils.h board.h chars.h objs.h globals.h memory.h
../obj/save.o: items.h mobiles.h players.h fread.h fwrite.h rooms.h affects.h
../obj/save.o: extra_descrs.h save.h
../obj/sha256.o: ./string.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
../obj/sha256.o: json.h structs.h types.h flags.h tables.h ext_flags.h sha256.h
../obj/signal.o: signal.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
../obj/signal.o: json.h structs.h types.h flags.h tables.h ext_flags.h utils.h
../obj/signal.o: globals.h
../obj/skills.o: ./string.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
../obj/skills.o: json.h structs.h types.h flags.h tables.h ext_flags.h lookup.h
../obj/skills.o: utils.h skills.h
../obj/special.o: utils.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
../obj/special.o: json.h structs.h types.h flags.h tables.h ext_flags.h fight.h
../obj/special.o: comm.h interp.h magic.h lookup.h db.h act_fight.h act_comm.h
../obj/special.o: act_move.h chars.h objs.h spell_aff.h globals.h rooms.h
../obj/special.o: special.h
../obj/spell_aff.o: magic.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
../obj/spell_aff.o: json.h structs.h types.h flags.h tables.h ext_flags.h utils.h
../obj/spell_aff.o: affects.h comm.h chars.h fight.h groups.h objs.h recycle.h
../obj/spell_aff.o: lookup.h items.h extra_descrs.h spell_aff.h
../obj/spell_create.o: magic.h merc.h basemud.h compat.h defs.h typedefs.h
../obj/spell_create.o: macros.h json.h structs.h types.h flags.h tables.h
../obj/spell_create.o: ext_flags.h find.h comm.h db.h objs.h utils.h chars.h
../obj/spell_create.o: globals.h memory.h items.h spell_create.h
../obj/spell_cure.o: magic.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
../obj/spell_cure.o: json.h structs.h types.h flags.h tables.h ext_flags.h comm.h
../obj/spell_cure.o: utils.h lookup.h fight.h chars.h objs.h spell_cure.h
../obj/spell_info.o: magic.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
../obj/spell_info.o: json.h structs.h types.h flags.h tables.h ext_flags.h comm.h
../obj/spell_info.o: utils.h lookup.h chars.h recycle.h db.h interp.h act_info.h
../obj/spell_info.o: affects.h objs.h globals.h items.h memory.h spell_info.h
../obj/spell_misc.o: magic.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
../obj/spell_misc.o: json.h structs.h types.h flags.h tables.h ext_flags.h comm.h
../obj/spell_misc.o: utils.h lookup.h db.h affects.h objs.h interp.h chars.h
../obj/spell_misc.o: globals.h items.h spell_misc.h
../obj/spell_move.o: magic.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
../obj/spell_move.o: json.h structs.h types.h flags.h tables.h ext_flags.h find.h
../obj/spell_move.o: chars.h comm.h interp.h act_info.h db.h fight.h objs.h
../obj/spell_move.o: items.h players.h rooms.h spell_move.h
../obj/spell_npc.o: magic.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
../obj/spell_npc.o: json.h structs.h types.h flags.h tables.h ext_flags.h utils.h
../obj/spell_npc.o: lookup.h effects.h fight.h comm.h chars.h spell_npc.h
../obj/spell_off.o: magic.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
../obj/spell_off.o: json.h structs.h types.h flags.h tables.h ext_flags.h utils.h
../obj/spell_off.o: fight.h comm.h affects.h lookup.h db.h spell_aff.h chars.h
../obj/spell_off.o: objs.h globals.h items.h players.h spell_off.h
../obj/string.o: ./string.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
../obj/string.o: json.h structs.h types.h flags.h tables.h ext_flags.h comm.h
../obj/string.o: utils.h interp.h olc.h db.h olc_mpedit.h globals.h memory.h
../obj/tables.o: nanny.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
../obj/tables.o: json.h structs.h types.h flags.h tables.h ext_flags.h lookup.h
../obj/tables.o: recycle.h colour.h board.h special.h effects.h json_tblr.h
../obj/tables.o: json_tblw.h magic.h memory.h utils.h chars.h spell_aff.h
../obj/tables.o: spell_create.h spell_cure.h spell_info.h spell_misc.h
../obj/tables.o: spell_move.h spell_npc.h spell_off.h act_skills.h
../obj/types.o: merc.h basemud.h compat.h defs.h typedefs.h macros.h json.h
../obj/types.o: structs.h types.h flags.h tables.h ext_flags.h lookup.h utils.h
../obj/update.o: ./string.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
../obj/update.o: json.h structs.h types.h flags.h tables.h ext_flags.h chars.h
../obj/update.o: utils.h players.h items.h comm.h music.h globals.h lookup.h
../obj/update.o: fight.h mob_prog.h areas.h mobiles.h objs.h update.h
../obj/utils.o: ./string.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
../obj/utils.o: json.h structs.h types.h flags.h tables.h ext_flags.h comm.h db.h
../obj/utils.o: interp.h chars.h globals.h utils.h
../obj/wiz_im.o: ./string.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
../obj/wiz_im.o: json.h structs.h types.h flags.h tables.h ext_flags.h interp.h
../obj/wiz_im.o: db.h utils.h comm.h lookup.h recycle.h do_sub.h mob_cmds.h
../obj/wiz_im.o: act_info.h chars.h objs.h rooms.h find.h affects.h globals.h
../obj/wiz_im.o: memory.h mob_prog.h wiz_im.h
../obj/wiz_l1.o: ./string.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
../obj/wiz_l1.o: json.h structs.h types.h flags.h tables.h ext_flags.h db.h comm.h
../obj/wiz_l1.o: interp.h save.h fight.h ban.h utils.h act_player.h wiz_l4.h
../obj/wiz_l1.o: chars.h find.h descs.h globals.h wiz_l1.h
../obj/wiz_l2.o: ./string.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
../obj/wiz_l2.o: json.h structs.h types.h flags.h tables.h ext_flags.h db.h comm.h
../obj/wiz_l2.o: interp.h ban.h utils.h recycle.h lookup.h chars.h rooms.h find.h
../obj/wiz_l2.o: globals.h wiz_l2.h
../obj/wiz_l3.o: db.h merc.h basemud.h compat.h defs.h typedefs.h macros.h json.h
../obj/wiz_l3.o: structs.h types.h flags.h tables.h ext_flags.h comm.h interp.h
../obj/wiz_l3.o: fight.h utils.h chars.h find.h descs.h globals.h wiz_l3.h
../obj/wiz_l4.o: ./string.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
../obj/wiz_l4.o: json.h structs.h types.h flags.h tables.h ext_flags.h db.h comm.h
../obj/wiz_l4.o: interp.h save.h fight.h utils.h lookup.h affects.h chars.h objs.h
../obj/wiz_l4.o: find.h recycle.h descs.h globals.h mobiles.h wiz_l4.h
../obj/wiz_l5.o: ./string.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
../obj/wiz_l5.o: json.h structs.h types.h flags.h tables.h ext_flags.h db.h comm.h
../obj/wiz_l5.o: interp.h save.h fight.h utils.h lookup.h affects.h do_sub.h
../obj/wiz_l5.o: recycle.h act_info.h chars.h objs.h rooms.h find.h memory.h
../obj/wiz_l5.o: globals.h mobiles.h players.h extra_descrs.h wiz_l5.h
../obj/wiz_l6.o: db.h merc.h basemud.h compat.h defs.h typedefs.h macros.h json.h
../obj/wiz_l6.o: structs.h types.h flags.h tables.h ext_flags.h comm.h interp.h
../obj/wiz_l6.o: chars.h rooms.h find.h globals.h memory.h wiz_l6.h
../obj/wiz_l7.o: db.h merc.h basemud.h compat.h defs.h typedefs.h macros.h json.h
../obj/wiz_l7.o: structs.h types.h flags.h tables.h ext_flags.h comm.h interp.h
../obj/wiz_l7.o: utils.h chars.h rooms.h find.h globals.h objs.h wiz_l7.h
../obj/wiz_l8.o: ./string.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
../obj/wiz_l8.o: json.h structs.h types.h flags.h tables.h ext_flags.h db.h comm.h
../obj/wiz_l8.o: interp.h fight.h utils.h act_info.h chars.h rooms.h find.h
../obj/wiz_l8.o: memory.h wiz_l8.h
../obj/wiz_ml.o: ./string.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
../obj/wiz_ml.o: json.h structs.h types.h flags.h tables.h ext_flags.h db.h comm.h
../obj/wiz_ml.o: interp.h save.h fight.h utils.h act_info.h chars.h rooms.h find.h
../obj/wiz_ml.o: descs.h boot.h memory.h globals.h players.h mobiles.h objs.h
../obj/wiz_ml.o: quickmud.h json_export.h lookup.h wiz_ml.h
