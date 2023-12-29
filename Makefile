SRC_PATH=src
OBJ_PATH=obj
BIN_PATH=bin

DUMPMACHINE = $(shell gcc -dumpmachine)
ifneq (, $(findstring linux, $(DUMPMACHINE)))
    OS   = LINUX
    LIBS = -lcrypt
    EXE  = $(BIN_PATH)/basemud
else ifneq (, $(findstring mingw, $(DUMPMACHINE)))
    OS   = MINGW
    LIBS =
    EXE  = $(BIN_PATH)/basemud.exe
else
    OS   = UNKNOWN_OS
    LIBS =
    EXE  = $(BIN_PATH)/basemud
endif

CC      = gcc
PROF    = -g -O
C_FLAGS = -Wall -Wno-trigraphs -Wno-format-truncation -Werror $(PROF)
L_FLAGS = $(PROF)
SRC_FILES = $(sort $(wildcard $(SRC_PATH)/*.c))
OBJ_FILES = $(SRC_FILES:${SRC_PATH}/%.c=$(OBJ_PATH)/%.o)

all: $(EXE)

$(EXE): $(OBJ_FILES)
	$(CC) $(L_FLAGS) -o $(EXE) $(OBJ_FILES) $(LIBS)

depend:
	makedepend -Y. $(SRC_PATH)/*.c 2>/dev/null
	cat Makefile | sed -r "s/^$(SRC_PATH)\/(.*)\.o:/$(OBJ_PATH)\/\1.o:/" > Makefile.2 && mv Makefile.2 Makefile

$(OBJ_PATH)/%.o: $(SRC_PATH)/%.c
	$(CC) -c $(C_FLAGS) -c $< -o $@

clean:
	rm -f $(OBJ_FILES)
# DO NOT DELETE

obj/act_board.o: src/act_board.h src/merc.h src/basemud.h src/compat.h
obj/act_board.o: src/defs.h src/macros.h src/typedefs.h src/structs.h
obj/act_board.o: src/flags.h src/ext_flags.h src/types.h src/act_info.h
obj/act_board.o: src/board.h src/chars.h src/comm.h src/globals.h
obj/act_board.o: src/interp.h src/memory.h src/recycle.h src/tables.h
obj/act_board.o: src/utils.h
obj/act_comm.o: src/act_comm.h src/merc.h src/basemud.h src/compat.h
obj/act_comm.o: src/defs.h src/macros.h src/typedefs.h src/structs.h
obj/act_comm.o: src/flags.h src/ext_flags.h src/types.h src/chars.h
obj/act_comm.o: src/comm.h src/do_sub.h src/find.h src/globals.h src/interp.h
obj/act_comm.o: src/lookup.h src/memory.h src/mob_prog.h src/players.h
obj/act_comm.o: src/recycle.h src/tables.h src/utils.h
obj/act_conf.o: src/act_conf.h src/merc.h src/basemud.h src/compat.h
obj/act_conf.o: src/defs.h src/macros.h src/typedefs.h src/structs.h
obj/act_conf.o: src/flags.h src/ext_flags.h src/types.h src/chars.h
obj/act_conf.o: src/colour.h src/comm.h src/db.h src/do_sub.h src/groups.h
obj/act_conf.o: src/interp.h src/lookup.h src/memory.h src/tables.h
obj/act_conf.o: src/utils.h
obj/act_fight.o: src/act_fight.h src/merc.h src/basemud.h src/compat.h
obj/act_fight.o: src/defs.h src/macros.h src/typedefs.h src/structs.h
obj/act_fight.o: src/flags.h src/ext_flags.h src/types.h src/act_comm.h
obj/act_fight.o: src/affects.h src/chars.h src/comm.h src/fight.h src/find.h
obj/act_fight.o: src/groups.h src/interp.h src/lookup.h src/mob_prog.h
obj/act_fight.o: src/players.h src/tables.h src/utils.h
obj/act_group.o: src/act_group.h src/merc.h src/basemud.h src/compat.h
obj/act_group.o: src/defs.h src/macros.h src/typedefs.h src/structs.h
obj/act_group.o: src/flags.h src/ext_flags.h src/types.h src/chars.h
obj/act_group.o: src/comm.h src/find.h src/globals.h src/groups.h
obj/act_group.o: src/interp.h src/tables.h src/utils.h
obj/act_info.o: src/act_info.h src/merc.h src/basemud.h src/compat.h
obj/act_info.o: src/defs.h src/macros.h src/typedefs.h src/structs.h
obj/act_info.o: src/flags.h src/ext_flags.h src/types.h src/act_comm.h
obj/act_info.o: src/chars.h src/comm.h src/extra_descrs.h src/find.h
obj/act_info.o: src/globals.h src/interp.h src/items.h src/lookup.h
obj/act_info.o: src/memory.h src/objs.h src/players.h src/recycle.h
obj/act_info.o: src/rooms.h src/spell_info.h src/tables.h src/update.h
obj/act_info.o: src/utils.h
obj/act_move.o: src/act_move.h src/merc.h src/basemud.h src/compat.h
obj/act_move.o: src/defs.h src/macros.h src/typedefs.h src/structs.h
obj/act_move.o: src/flags.h src/ext_flags.h src/types.h src/act_info.h
obj/act_move.o: src/affects.h src/chars.h src/comm.h src/fight.h src/find.h
obj/act_move.o: src/globals.h src/interp.h src/items.h src/lookup.h
obj/act_move.o: src/objs.h src/players.h src/rooms.h src/tables.h src/utils.h
obj/act_obj.o: src/act_obj.h src/merc.h src/basemud.h src/compat.h src/defs.h
obj/act_obj.o: src/macros.h src/typedefs.h src/structs.h src/flags.h
obj/act_obj.o: src/ext_flags.h src/types.h src/act_comm.h src/act_group.h
obj/act_obj.o: src/act_move.h src/affects.h src/chars.h src/comm.h
obj/act_obj.o: src/fight.h src/find.h src/globals.h src/groups.h src/interp.h
obj/act_obj.o: src/items.h src/lookup.h src/mob_prog.h src/objs.h
obj/act_obj.o: src/players.h src/rooms.h src/save.h src/tables.h src/utils.h
obj/act_olc.o: src/act_olc.h src/merc.h src/basemud.h src/compat.h src/defs.h
obj/act_olc.o: src/macros.h src/typedefs.h src/structs.h src/flags.h
obj/act_olc.o: src/ext_flags.h src/types.h src/act_info.h src/chars.h
obj/act_olc.o: src/comm.h src/descs.h src/globals.h src/interp.h src/items.h
obj/act_olc.o: src/json_export.h src/lookup.h src/memory.h src/mob_prog.h
obj/act_olc.o: src/mobiles.h src/objs.h src/olc.h src/olc_aedit.h
obj/act_olc.o: src/olc_hedit.h src/olc_medit.h src/olc_mpedit.h
obj/act_olc.o: src/olc_oedit.h src/olc_redit.h src/olc_save.h src/portals.h
obj/act_olc.o: src/recycle.h src/resets.h src/rooms.h src/tables.h
obj/act_olc.o: src/utils.h
obj/act_player.o: src/act_player.h src/merc.h src/basemud.h src/compat.h
obj/act_player.o: src/defs.h src/macros.h src/typedefs.h src/structs.h
obj/act_player.o: src/flags.h src/ext_flags.h src/types.h src/chars.h
obj/act_player.o: src/comm.h src/descs.h src/fight.h src/globals.h
obj/act_player.o: src/interp.h src/memory.h src/recycle.h src/save.h
obj/act_player.o: src/utils.h
obj/act_shop.o: src/act_shop.h src/merc.h src/basemud.h src/compat.h
obj/act_shop.o: src/defs.h src/macros.h src/typedefs.h src/structs.h
obj/act_shop.o: src/flags.h src/ext_flags.h src/types.h src/act_comm.h
obj/act_shop.o: src/chars.h src/comm.h src/find.h src/globals.h src/groups.h
obj/act_shop.o: src/interp.h src/items.h src/lookup.h src/magic.h
obj/act_shop.o: src/materials.h src/memory.h src/mobiles.h src/objs.h
obj/act_shop.o: src/players.h src/rooms.h src/tables.h src/utils.h
obj/act_skills.o: src/act_skills.h src/merc.h src/basemud.h src/compat.h
obj/act_skills.o: src/defs.h src/macros.h src/typedefs.h src/structs.h
obj/act_skills.o: src/flags.h src/ext_flags.h src/types.h src/chars.h
obj/act_skills.o: src/comm.h src/db.h src/fight.h src/find.h src/interp.h
obj/act_skills.o: src/lookup.h src/magic.h src/memory.h src/players.h
obj/act_skills.o: src/recycle.h src/tables.h src/utils.h
obj/affects.o: src/affects.h src/merc.h src/basemud.h src/compat.h src/defs.h
obj/affects.o: src/macros.h src/typedefs.h src/structs.h src/flags.h
obj/affects.o: src/ext_flags.h src/types.h src/chars.h src/lookup.h
obj/affects.o: src/recycle.h src/utils.h
obj/areas.o: src/areas.h src/merc.h src/basemud.h src/compat.h src/defs.h
obj/areas.o: src/macros.h src/typedefs.h src/structs.h src/flags.h
obj/areas.o: src/ext_flags.h src/types.h src/comm.h src/globals.h
obj/areas.o: src/recycle.h src/resets.h src/rooms.h src/utils.h
obj/ban.o: src/ban.h src/merc.h src/basemud.h src/compat.h src/defs.h
obj/ban.o: src/macros.h src/typedefs.h src/structs.h src/flags.h
obj/ban.o: src/ext_flags.h src/types.h src/chars.h src/comm.h src/fread.h
obj/ban.o: src/fwrite.h src/globals.h src/interp.h src/memory.h src/recycle.h
obj/ban.o: src/utils.h
obj/board.o: src/board.h src/merc.h src/basemud.h src/compat.h src/defs.h
obj/board.o: src/macros.h src/typedefs.h src/structs.h src/flags.h
obj/board.o: src/ext_flags.h src/types.h src/chars.h src/comm.h src/descs.h
obj/board.o: src/fread.h src/globals.h src/interp.h src/lookup.h src/memory.h
obj/board.o: src/recycle.h src/tables.h src/utils.h
obj/boot.o: src/boot.h src/merc.h src/basemud.h src/compat.h src/defs.h
obj/boot.o: src/macros.h src/typedefs.h src/structs.h src/flags.h
obj/boot.o: src/ext_flags.h src/types.h src/act_info.h src/chars.h src/comm.h
obj/boot.o: src/db.h src/descs.h src/globals.h src/interp.h src/memory.h
obj/boot.o: src/nanny.h src/olc.h src/quickmud.h src/recycle.h src/rooms.h
obj/boot.o: src/save.h src/signal.h src/string.h src/tables.h src/update.h
obj/boot.o: src/utils.h
obj/chars.o: src/chars.h src/merc.h src/basemud.h src/compat.h src/defs.h
obj/chars.o: src/macros.h src/typedefs.h src/structs.h src/flags.h
obj/chars.o: src/ext_flags.h src/types.h src/affects.h src/board.h src/comm.h
obj/chars.o: src/fight.h src/globals.h src/groups.h src/interp.h src/items.h
obj/chars.o: src/lookup.h src/magic.h src/materials.h src/mob_prog.h
obj/chars.o: src/mobiles.h src/objs.h src/players.h src/recycle.h src/rooms.h
obj/chars.o: src/save.h src/tables.h src/utils.h src/wiz_l6.h src/string.h
obj/chars.o: src/act_info.h src/act_move.h src/act_player.h
obj/colour.o: src/colour.h src/merc.h src/basemud.h src/compat.h src/defs.h
obj/colour.o: src/macros.h src/typedefs.h src/structs.h src/flags.h
obj/colour.o: src/ext_flags.h src/types.h src/chars.h src/tables.h
obj/colour.o: src/string.h
obj/comm.o: src/comm.h src/merc.h src/basemud.h src/compat.h src/defs.h
obj/comm.o: src/macros.h src/typedefs.h src/structs.h src/flags.h
obj/comm.o: src/ext_flags.h src/types.h src/chars.h src/colour.h src/descs.h
obj/comm.o: src/globals.h src/mob_prog.h src/objs.h src/olc.h src/players.h
obj/comm.o: src/rooms.h src/utils.h src/string.h
obj/db.o: src/db.h src/merc.h src/basemud.h src/compat.h src/defs.h
obj/db.o: src/macros.h src/typedefs.h src/structs.h src/flags.h
obj/db.o: src/ext_flags.h src/types.h src/affects.h src/areas.h src/ban.h
obj/db.o: src/board.h src/db_old.h src/extra_descrs.h src/fread.h
obj/db.o: src/globals.h src/help.h src/items.h src/json_import.h src/lookup.h
obj/db.o: src/memory.h src/mob_prog.h src/mobiles.h src/music.h src/objs.h
obj/db.o: src/portals.h src/recycle.h src/resets.h src/rooms.h src/skills.h
obj/db.o: src/tables.h src/utils.h src/string.h
obj/db_old.o: src/db_old.h src/merc.h src/basemud.h src/compat.h src/defs.h
obj/db_old.o: src/macros.h src/typedefs.h src/structs.h src/flags.h
obj/db_old.o: src/ext_flags.h src/types.h src/affects.h src/db.h
obj/db_old.o: src/extra_descrs.h src/fread.h src/globals.h src/interp.h
obj/db_old.o: src/items.h src/lookup.h src/mobiles.h src/objs.h src/recycle.h
obj/db_old.o: src/rooms.h src/tables.h src/utils.h
obj/descs.o: src/descs.h src/merc.h src/basemud.h src/compat.h src/defs.h
obj/descs.o: src/macros.h src/typedefs.h src/structs.h src/flags.h
obj/descs.o: src/ext_flags.h src/types.h src/ban.h src/chars.h src/colour.h
obj/descs.o: src/comm.h src/globals.h src/interp.h src/memory.h src/recycle.h
obj/descs.o: src/utils.h src/string.h
obj/do_sub.o: src/do_sub.h src/merc.h src/basemud.h src/compat.h src/defs.h
obj/do_sub.o: src/macros.h src/typedefs.h src/structs.h src/flags.h
obj/do_sub.o: src/ext_flags.h src/types.h src/chars.h src/comm.h
obj/effects.o: src/effects.h src/merc.h src/basemud.h src/compat.h src/defs.h
obj/effects.o: src/macros.h src/typedefs.h src/structs.h src/flags.h
obj/effects.o: src/ext_flags.h src/types.h src/affects.h src/chars.h
obj/effects.o: src/comm.h src/items.h src/lookup.h src/magic.h src/objs.h
obj/effects.o: src/players.h src/tables.h src/utils.h
obj/ext_flags.o: src/ext_flags.h src/merc.h src/basemud.h src/compat.h
obj/ext_flags.o: src/defs.h src/macros.h src/typedefs.h src/structs.h
obj/ext_flags.o: src/flags.h src/types.h src/interp.h src/lookup.h
obj/ext_flags.o: src/utils.h src/string.h
obj/extra_descrs.o: src/extra_descrs.h src/merc.h src/basemud.h src/compat.h
obj/extra_descrs.o: src/defs.h src/macros.h src/typedefs.h src/structs.h
obj/extra_descrs.o: src/flags.h src/ext_flags.h src/types.h src/utils.h
obj/fight.o: src/fight.h src/merc.h src/basemud.h src/compat.h src/defs.h
obj/fight.o: src/macros.h src/typedefs.h src/structs.h src/flags.h
obj/fight.o: src/ext_flags.h src/types.h src/act_comm.h src/act_fight.h
obj/fight.o: src/act_move.h src/act_obj.h src/affects.h src/chars.h
obj/fight.o: src/comm.h src/effects.h src/find.h src/globals.h src/groups.h
obj/fight.o: src/interp.h src/items.h src/magic.h src/memory.h src/mob_prog.h
obj/fight.o: src/mobiles.h src/objs.h src/players.h src/save.h src/tables.h
obj/fight.o: src/utils.h src/string.h
obj/find.o: src/find.h src/merc.h src/basemud.h src/compat.h src/defs.h
obj/find.o: src/macros.h src/typedefs.h src/structs.h src/flags.h
obj/find.o: src/ext_flags.h src/types.h src/chars.h src/globals.h
obj/find.o: src/interp.h src/rooms.h src/utils.h
obj/flags.o: src/interp.h src/merc.h src/basemud.h src/compat.h src/defs.h
obj/flags.o: src/macros.h src/typedefs.h src/structs.h src/flags.h
obj/flags.o: src/ext_flags.h src/types.h src/lookup.h src/utils.h
obj/flags.o: src/string.h
obj/fread.o: src/fread.h src/merc.h src/basemud.h src/compat.h src/defs.h
obj/fread.o: src/macros.h src/typedefs.h src/structs.h src/flags.h
obj/fread.o: src/ext_flags.h src/types.h src/globals.h src/memory.h
obj/fread.o: src/utils.h src/string.h
obj/fwrite.o: src/fwrite.h src/merc.h src/basemud.h src/compat.h src/defs.h
obj/fwrite.o: src/macros.h src/typedefs.h src/structs.h src/flags.h
obj/fwrite.o: src/ext_flags.h src/types.h src/save.h
obj/globals.o: src/globals.h src/merc.h src/basemud.h src/compat.h src/defs.h
obj/globals.o: src/macros.h src/typedefs.h src/structs.h src/flags.h
obj/globals.o: src/ext_flags.h src/types.h
obj/groups.o: src/groups.h src/merc.h src/basemud.h src/compat.h src/defs.h
obj/groups.o: src/macros.h src/typedefs.h src/structs.h src/flags.h
obj/groups.o: src/ext_flags.h src/types.h src/affects.h src/chars.h
obj/groups.o: src/comm.h src/globals.h src/recycle.h src/tables.h src/utils.h
obj/help.o: src/help.h src/merc.h src/basemud.h src/compat.h src/defs.h
obj/help.o: src/macros.h src/typedefs.h src/structs.h src/flags.h
obj/help.o: src/ext_flags.h src/types.h
obj/interp.o: src/act_board.h src/merc.h src/basemud.h src/compat.h
obj/interp.o: src/defs.h src/macros.h src/typedefs.h src/structs.h
obj/interp.o: src/flags.h src/ext_flags.h src/types.h src/act_comm.h
obj/interp.o: src/chars.h src/comm.h src/descs.h src/find.h src/globals.h
obj/interp.o: src/lookup.h src/memory.h src/utils.h src/string.h
obj/interp.o: src/act_conf.h src/act_fight.h src/act_group.h src/act_info.h
obj/interp.o: src/act_move.h src/act_obj.h src/act_player.h src/act_shop.h
obj/interp.o: src/act_skills.h src/act_olc.h src/wiz_im.h src/wiz_l1.h
obj/interp.o: src/wiz_l2.h src/wiz_l3.h src/wiz_l4.h src/wiz_l5.h
obj/interp.o: src/wiz_l6.h src/wiz_l7.h src/wiz_l8.h src/wiz_ml.h
obj/interp.o: src/mob_cmds.h src/interp.h
obj/items.o: src/act_info.h src/merc.h src/basemud.h src/compat.h src/defs.h
obj/items.o: src/macros.h src/typedefs.h src/structs.h src/flags.h
obj/items.o: src/ext_flags.h src/types.h src/act_move.h src/affects.h
obj/items.o: src/chars.h src/comm.h src/fread.h src/fwrite.h src/globals.h
obj/items.o: src/groups.h src/interp.h src/lookup.h src/magic.h
obj/items.o: src/mob_prog.h src/music.h src/objs.h src/players.h
obj/items.o: src/recycle.h src/rooms.h src/tables.h src/utils.h src/string.h
obj/items.o: src/act_group.h src/items.h
obj/json.o: src/json.h src/merc.h src/basemud.h src/compat.h src/defs.h
obj/json.o: src/macros.h src/typedefs.h src/structs.h src/flags.h
obj/json.o: src/ext_flags.h src/types.h src/utils.h src/string.h
obj/json_export.o: src/json_export.h src/merc.h src/basemud.h src/compat.h
obj/json_export.o: src/defs.h src/macros.h src/typedefs.h src/structs.h
obj/json_export.o: src/flags.h src/ext_flags.h src/types.h src/json.h
obj/json_export.o: src/json_objw.h src/json_write.h src/lookup.h
obj/json_export.o: src/recycle.h src/utils.h
obj/json_import.o: src/json_import.h src/merc.h src/basemud.h src/compat.h
obj/json_import.o: src/defs.h src/macros.h src/typedefs.h src/structs.h
obj/json_import.o: src/flags.h src/ext_flags.h src/types.h src/db.h
obj/json_import.o: src/globals.h src/help.h src/json.h src/json_objr.h
obj/json_import.o: src/json_read.h src/lookup.h src/memory.h src/mobiles.h
obj/json_import.o: src/objs.h src/recycle.h src/rooms.h src/utils.h
obj/json_import.o: src/string.h
obj/json_objr.o: src/json_objr.h src/merc.h src/basemud.h src/compat.h
obj/json_objr.o: src/defs.h src/macros.h src/typedefs.h src/structs.h
obj/json_objr.o: src/flags.h src/ext_flags.h src/types.h src/affects.h
obj/json_objr.o: src/db.h src/extra_descrs.h src/globals.h src/help.h
obj/json_objr.o: src/json.h src/json_import.h src/lookup.h src/memory.h
obj/json_objr.o: src/portals.h src/recycle.h src/resets.h src/rooms.h
obj/json_objr.o: src/string.h
obj/json_objw.o: src/json_objw.h src/merc.h src/basemud.h src/compat.h
obj/json_objw.o: src/defs.h src/macros.h src/typedefs.h src/structs.h
obj/json_objw.o: src/flags.h src/ext_flags.h src/types.h src/json.h
obj/json_objw.o: src/lookup.h src/utils.h src/string.h
obj/json_read.o: src/json_read.h src/merc.h src/basemud.h src/compat.h
obj/json_read.o: src/defs.h src/macros.h src/typedefs.h src/structs.h
obj/json_read.o: src/flags.h src/ext_flags.h src/types.h src/json.h
obj/json_read.o: src/utils.h src/string.h
obj/json_tblr.o: src/json_tblr.h src/merc.h src/basemud.h src/compat.h
obj/json_tblr.o: src/defs.h src/macros.h src/typedefs.h src/structs.h
obj/json_tblr.o: src/flags.h src/ext_flags.h src/types.h src/json.h
obj/json_tblr.o: src/json_import.h src/lookup.h src/memory.h src/tables.h
obj/json_tblr.o: src/string.h
obj/json_tblw.o: src/json_tblw.h src/merc.h src/basemud.h src/compat.h
obj/json_tblw.o: src/defs.h src/macros.h src/typedefs.h src/structs.h
obj/json_tblw.o: src/flags.h src/ext_flags.h src/types.h src/colour.h
obj/json_tblw.o: src/json.h src/lookup.h src/string.h
obj/json_write.o: src/json_write.h src/merc.h src/basemud.h src/compat.h
obj/json_write.o: src/defs.h src/macros.h src/typedefs.h src/structs.h
obj/json_write.o: src/flags.h src/ext_flags.h src/types.h src/json.h
obj/json_write.o: src/utils.h src/string.h
obj/lookup.o: src/lookup.h src/merc.h src/basemud.h src/compat.h src/defs.h
obj/lookup.o: src/macros.h src/typedefs.h src/structs.h src/flags.h
obj/lookup.o: src/ext_flags.h src/types.h src/globals.h src/recycle.h
obj/lookup.o: src/tables.h src/utils.h src/string.h
obj/magic.o: src/magic.h src/merc.h src/basemud.h src/compat.h src/defs.h
obj/magic.o: src/macros.h src/typedefs.h src/structs.h src/flags.h
obj/magic.o: src/ext_flags.h src/types.h src/affects.h src/chars.h src/comm.h
obj/magic.o: src/fight.h src/lookup.h src/tables.h src/utils.h src/string.h
obj/materials.o: src/materials.h src/merc.h src/basemud.h src/compat.h
obj/materials.o: src/defs.h src/macros.h src/typedefs.h src/structs.h
obj/materials.o: src/flags.h src/ext_flags.h src/types.h src/string.h
obj/memory.o: src/memory.h src/merc.h src/basemud.h src/compat.h src/defs.h
obj/memory.o: src/macros.h src/typedefs.h src/structs.h src/flags.h
obj/memory.o: src/ext_flags.h src/types.h src/globals.h src/recycle.h
obj/memory.o: src/tables.h src/utils.h src/string.h
obj/mob_cmds.o: src/mob_cmds.h src/merc.h src/basemud.h src/compat.h
obj/mob_cmds.o: src/defs.h src/macros.h src/typedefs.h src/structs.h
obj/mob_cmds.o: src/flags.h src/ext_flags.h src/types.h src/act_info.h
obj/mob_cmds.o: src/chars.h src/comm.h src/fight.h src/find.h src/globals.h
obj/mob_cmds.o: src/groups.h src/interp.h src/lookup.h src/mob_prog.h
obj/mob_cmds.o: src/mobiles.h src/objs.h src/rooms.h src/tables.h src/utils.h
obj/mob_prog.o: src/mob_prog.h src/merc.h src/basemud.h src/compat.h
obj/mob_prog.o: src/defs.h src/macros.h src/typedefs.h src/structs.h
obj/mob_prog.o: src/flags.h src/ext_flags.h src/types.h src/chars.h
obj/mob_prog.o: src/find.h src/globals.h src/groups.h src/interp.h
obj/mob_prog.o: src/lookup.h src/mob_cmds.h src/utils.h src/string.h
obj/mobiles.o: src/mobiles.h src/merc.h src/basemud.h src/compat.h src/defs.h
obj/mobiles.o: src/macros.h src/typedefs.h src/structs.h src/flags.h
obj/mobiles.o: src/ext_flags.h src/types.h src/act_fight.h src/affects.h
obj/mobiles.o: src/chars.h src/comm.h src/db.h src/fight.h src/globals.h
obj/mobiles.o: src/interp.h src/items.h src/lookup.h src/magic.h src/memory.h
obj/mobiles.o: src/mob_prog.h src/objs.h src/recycle.h src/tables.h
obj/mobiles.o: src/utils.h
obj/music.o: src/music.h src/merc.h src/basemud.h src/compat.h src/defs.h
obj/music.o: src/macros.h src/typedefs.h src/structs.h src/flags.h
obj/music.o: src/ext_flags.h src/types.h src/chars.h src/comm.h src/fread.h
obj/music.o: src/globals.h src/interp.h src/lookup.h src/memory.h src/objs.h
obj/music.o: src/recycle.h src/tables.h src/utils.h
obj/nanny.o: src/nanny.h src/merc.h src/basemud.h src/compat.h src/defs.h
obj/nanny.o: src/macros.h src/typedefs.h src/structs.h src/flags.h
obj/nanny.o: src/ext_flags.h src/types.h src/act_board.h src/act_info.h
obj/nanny.o: src/act_obj.h src/act_skills.h src/ban.h src/chars.h src/comm.h
obj/nanny.o: src/descs.h src/globals.h src/interp.h src/lookup.h src/magic.h
obj/nanny.o: src/memory.h src/objs.h src/players.h src/recycle.h src/rooms.h
obj/nanny.o: src/save.h src/tables.h src/utils.h src/string.h
obj/objs.o: src/objs.h src/merc.h src/basemud.h src/compat.h src/defs.h
obj/objs.o: src/macros.h src/typedefs.h src/structs.h src/flags.h
obj/objs.o: src/ext_flags.h src/types.h src/affects.h src/chars.h src/comm.h
obj/objs.o: src/db.h src/extra_descrs.h src/globals.h src/items.h
obj/objs.o: src/lookup.h src/materials.h src/memory.h src/recycle.h
obj/objs.o: src/tables.h src/utils.h src/string.h
obj/olc.o: src/olc.h src/merc.h src/basemud.h src/compat.h src/defs.h
obj/olc.o: src/macros.h src/typedefs.h src/structs.h src/flags.h
obj/olc.o: src/ext_flags.h src/types.h src/act_olc.h src/chars.h src/comm.h
obj/olc.o: src/db.h src/interp.h src/lookup.h src/magic.h src/memory.h
obj/olc.o: src/olc_aedit.h src/olc_hedit.h src/olc_medit.h src/olc_mpedit.h
obj/olc.o: src/olc_oedit.h src/olc_redit.h src/recycle.h src/tables.h
obj/olc.o: src/utils.h src/string.h
obj/olc_aedit.o: src/olc_aedit.h src/merc.h src/basemud.h src/compat.h
obj/olc_aedit.o: src/defs.h src/macros.h src/typedefs.h src/structs.h
obj/olc_aedit.o: src/flags.h src/ext_flags.h src/types.h src/areas.h
obj/olc_aedit.o: src/comm.h src/globals.h src/interp.h src/lookup.h
obj/olc_aedit.o: src/memory.h src/olc.h src/recycle.h src/string.h
obj/olc_hedit.o: src/olc_hedit.h src/merc.h src/basemud.h src/compat.h
obj/olc_hedit.o: src/defs.h src/macros.h src/typedefs.h src/structs.h
obj/olc_hedit.o: src/flags.h src/ext_flags.h src/types.h src/comm.h
obj/olc_hedit.o: src/globals.h src/help.h src/interp.h src/lookup.h
obj/olc_hedit.o: src/memory.h src/olc.h src/recycle.h src/string.h
obj/olc_hedit.o: src/utils.h
obj/olc_medit.o: src/olc_medit.h src/merc.h src/basemud.h src/compat.h
obj/olc_medit.o: src/defs.h src/macros.h src/typedefs.h src/structs.h
obj/olc_medit.o: src/flags.h src/ext_flags.h src/types.h src/chars.h
obj/olc_medit.o: src/comm.h src/db.h src/globals.h src/interp.h src/lookup.h
obj/olc_medit.o: src/memory.h src/mob_cmds.h src/mob_prog.h src/mobiles.h
obj/olc_medit.o: src/olc.h src/recycle.h src/string.h src/tables.h
obj/olc_medit.o: src/utils.h
obj/olc_mpedit.o: src/olc_mpedit.h src/merc.h src/basemud.h src/compat.h
obj/olc_mpedit.o: src/defs.h src/macros.h src/typedefs.h src/structs.h
obj/olc_mpedit.o: src/flags.h src/ext_flags.h src/types.h src/chars.h
obj/olc_mpedit.o: src/comm.h src/globals.h src/lookup.h src/memory.h
obj/olc_mpedit.o: src/mob_prog.h src/olc.h src/recycle.h src/string.h
obj/olc_mpedit.o: src/utils.h
obj/olc_oedit.o: src/olc_oedit.h src/merc.h src/basemud.h src/compat.h
obj/olc_oedit.o: src/defs.h src/macros.h src/typedefs.h src/structs.h
obj/olc_oedit.o: src/flags.h src/ext_flags.h src/types.h src/act_info.h
obj/olc_oedit.o: src/affects.h src/chars.h src/comm.h src/extra_descrs.h
obj/olc_oedit.o: src/globals.h src/interp.h src/items.h src/lookup.h
obj/olc_oedit.o: src/memory.h src/objs.h src/olc.h src/recycle.h src/string.h
obj/olc_oedit.o: src/utils.h
obj/olc_redit.o: src/olc_redit.h src/merc.h src/basemud.h src/compat.h
obj/olc_redit.o: src/defs.h src/macros.h src/typedefs.h src/structs.h
obj/olc_redit.o: src/flags.h src/ext_flags.h src/types.h src/act_info.h
obj/olc_redit.o: src/chars.h src/comm.h src/db.h src/extra_descrs.h
obj/olc_redit.o: src/find.h src/interp.h src/lookup.h src/memory.h
obj/olc_redit.o: src/mobiles.h src/objs.h src/olc.h src/olc_medit.h
obj/olc_redit.o: src/olc_oedit.h src/portals.h src/recycle.h src/resets.h
obj/olc_redit.o: src/rooms.h src/string.h src/tables.h src/utils.h
obj/olc_save.o: src/olc_save.h src/merc.h src/basemud.h src/compat.h
obj/olc_save.o: src/defs.h src/macros.h src/typedefs.h src/structs.h
obj/olc_save.o: src/flags.h src/ext_flags.h src/types.h src/comm.h
obj/olc_save.o: src/fwrite.h src/globals.h src/items.h src/json_export.h
obj/olc_save.o: src/lookup.h src/mob_cmds.h src/mob_prog.h src/mobiles.h
obj/olc_save.o: src/objs.h src/rooms.h src/tables.h src/utils.h
obj/players.o: src/players.h src/merc.h src/basemud.h src/compat.h src/defs.h
obj/players.o: src/macros.h src/typedefs.h src/structs.h src/flags.h
obj/players.o: src/ext_flags.h src/types.h src/affects.h src/chars.h
obj/players.o: src/comm.h src/fight.h src/globals.h src/groups.h src/items.h
obj/players.o: src/lookup.h src/magic.h src/memory.h src/objs.h src/rooms.h
obj/players.o: src/save.h src/tables.h src/utils.h src/string.h
obj/portals.o: src/portals.h src/merc.h src/basemud.h src/compat.h src/defs.h
obj/portals.o: src/macros.h src/typedefs.h src/structs.h src/flags.h
obj/portals.o: src/ext_flags.h src/types.h src/lookup.h src/memory.h
obj/portals.o: src/recycle.h src/rooms.h src/utils.h src/string.h
obj/quickmud.o: src/quickmud.h src/merc.h src/basemud.h src/compat.h
obj/quickmud.o: src/defs.h src/macros.h src/typedefs.h src/structs.h
obj/quickmud.o: src/flags.h src/ext_flags.h src/types.h src/fread.h
obj/quickmud.o: src/utils.h
obj/recycle.o: src/recycle.h src/merc.h src/basemud.h src/compat.h src/defs.h
obj/recycle.o: src/macros.h src/typedefs.h src/structs.h src/flags.h
obj/recycle.o: src/ext_flags.h src/types.h src/affects.h src/board.h
obj/recycle.o: src/chars.h src/db.h src/extra_descrs.h src/globals.h
obj/recycle.o: src/help.h src/lookup.h src/memory.h src/mob_prog.h
obj/recycle.o: src/mobiles.h src/objs.h src/portals.h src/resets.h
obj/recycle.o: src/rooms.h src/tables.h src/utils.h src/string.h
obj/resets.o: src/resets.h src/merc.h src/basemud.h src/compat.h src/defs.h
obj/resets.o: src/macros.h src/typedefs.h src/structs.h src/flags.h
obj/resets.o: src/ext_flags.h src/types.h src/chars.h src/items.h
obj/resets.o: src/mobiles.h src/objs.h src/recycle.h src/rooms.h src/utils.h
obj/rooms.o: src/rooms.h src/merc.h src/basemud.h src/compat.h src/defs.h
obj/rooms.o: src/macros.h src/typedefs.h src/structs.h src/flags.h
obj/rooms.o: src/ext_flags.h src/types.h src/chars.h src/db.h src/globals.h
obj/rooms.o: src/interp.h src/lookup.h src/mobiles.h src/objs.h src/recycle.h
obj/rooms.o: src/resets.h src/tables.h src/utils.h
obj/save.o: src/save.h src/merc.h src/basemud.h src/compat.h src/defs.h
obj/save.o: src/macros.h src/typedefs.h src/structs.h src/flags.h
obj/save.o: src/ext_flags.h src/types.h src/affects.h src/chars.h
obj/save.o: src/colour.h src/db.h src/extra_descrs.h src/fread.h src/fwrite.h
obj/save.o: src/globals.h src/items.h src/lookup.h src/memory.h src/mobiles.h
obj/save.o: src/objs.h src/players.h src/recycle.h src/rooms.h src/tables.h
obj/save.o: src/utils.h
obj/sha256.o: src/string.h src/merc.h src/basemud.h src/compat.h src/defs.h
obj/sha256.o: src/macros.h src/typedefs.h src/structs.h src/flags.h
obj/sha256.o: src/ext_flags.h src/types.h src/sha256.h
obj/signal.o: src/signal.h src/merc.h src/basemud.h src/compat.h src/defs.h
obj/signal.o: src/macros.h src/typedefs.h src/structs.h src/flags.h
obj/signal.o: src/ext_flags.h src/types.h src/globals.h src/utils.h
obj/skills.o: src/skills.h src/merc.h src/basemud.h src/compat.h src/defs.h
obj/skills.o: src/macros.h src/typedefs.h src/structs.h src/flags.h
obj/skills.o: src/ext_flags.h src/types.h src/lookup.h src/tables.h
obj/skills.o: src/utils.h
obj/special.o: src/special.h src/merc.h src/basemud.h src/compat.h src/defs.h
obj/special.o: src/macros.h src/typedefs.h src/structs.h src/flags.h
obj/special.o: src/ext_flags.h src/types.h src/act_comm.h src/act_fight.h
obj/special.o: src/act_move.h src/chars.h src/comm.h src/fight.h
obj/special.o: src/globals.h src/interp.h src/lookup.h src/magic.h src/objs.h
obj/special.o: src/rooms.h src/spell_aff.h src/tables.h src/utils.h
obj/spell_aff.o: src/spell_aff.h src/merc.h src/basemud.h src/compat.h
obj/spell_aff.o: src/defs.h src/macros.h src/typedefs.h src/structs.h
obj/spell_aff.o: src/flags.h src/ext_flags.h src/types.h src/affects.h
obj/spell_aff.o: src/chars.h src/comm.h src/fight.h src/groups.h src/items.h
obj/spell_aff.o: src/lookup.h src/magic.h src/objs.h src/recycle.h
obj/spell_aff.o: src/tables.h src/utils.h
obj/spell_create.o: src/spell_create.h src/merc.h src/basemud.h src/compat.h
obj/spell_create.o: src/defs.h src/macros.h src/typedefs.h src/structs.h
obj/spell_create.o: src/flags.h src/ext_flags.h src/types.h src/chars.h
obj/spell_create.o: src/comm.h src/find.h src/globals.h src/items.h
obj/spell_create.o: src/memory.h src/objs.h src/utils.h
obj/spell_cure.o: src/spell_cure.h src/merc.h src/basemud.h src/compat.h
obj/spell_cure.o: src/defs.h src/macros.h src/typedefs.h src/structs.h
obj/spell_cure.o: src/flags.h src/ext_flags.h src/types.h src/chars.h
obj/spell_cure.o: src/comm.h src/fight.h src/lookup.h src/magic.h src/objs.h
obj/spell_cure.o: src/tables.h src/utils.h
obj/spell_info.o: src/spell_info.h src/merc.h src/basemud.h src/compat.h
obj/spell_info.o: src/defs.h src/macros.h src/typedefs.h src/structs.h
obj/spell_info.o: src/flags.h src/ext_flags.h src/types.h src/act_info.h
obj/spell_info.o: src/affects.h src/chars.h src/comm.h src/globals.h
obj/spell_info.o: src/interp.h src/items.h src/lookup.h src/memory.h
obj/spell_info.o: src/objs.h src/recycle.h src/tables.h src/utils.h
obj/spell_misc.o: src/spell_misc.h src/merc.h src/basemud.h src/compat.h
obj/spell_misc.o: src/defs.h src/macros.h src/typedefs.h src/structs.h
obj/spell_misc.o: src/flags.h src/ext_flags.h src/types.h src/affects.h
obj/spell_misc.o: src/chars.h src/comm.h src/globals.h src/interp.h
obj/spell_misc.o: src/items.h src/lookup.h src/magic.h src/objs.h src/utils.h
obj/spell_move.o: src/spell_move.h src/merc.h src/basemud.h src/compat.h
obj/spell_move.o: src/defs.h src/macros.h src/typedefs.h src/structs.h
obj/spell_move.o: src/flags.h src/ext_flags.h src/types.h src/act_info.h
obj/spell_move.o: src/chars.h src/comm.h src/fight.h src/find.h src/interp.h
obj/spell_move.o: src/items.h src/magic.h src/objs.h src/players.h
obj/spell_move.o: src/rooms.h
obj/spell_npc.o: src/spell_npc.h src/merc.h src/basemud.h src/compat.h
obj/spell_npc.o: src/defs.h src/macros.h src/typedefs.h src/structs.h
obj/spell_npc.o: src/flags.h src/ext_flags.h src/types.h src/chars.h
obj/spell_npc.o: src/comm.h src/effects.h src/fight.h src/lookup.h
obj/spell_npc.o: src/magic.h src/utils.h
obj/spell_off.o: src/spell_off.h src/merc.h src/basemud.h src/compat.h
obj/spell_off.o: src/defs.h src/macros.h src/typedefs.h src/structs.h
obj/spell_off.o: src/flags.h src/ext_flags.h src/types.h src/affects.h
obj/spell_off.o: src/chars.h src/comm.h src/fight.h src/globals.h src/items.h
obj/spell_off.o: src/lookup.h src/magic.h src/objs.h src/players.h
obj/spell_off.o: src/spell_aff.h src/tables.h src/utils.h
obj/string.o: src/string.h src/merc.h src/basemud.h src/compat.h src/defs.h
obj/string.o: src/macros.h src/typedefs.h src/structs.h src/flags.h
obj/string.o: src/ext_flags.h src/types.h src/comm.h src/globals.h
obj/string.o: src/interp.h src/memory.h src/olc.h src/olc_mpedit.h
obj/string.o: src/utils.h
obj/tables.o: src/tables.h src/merc.h src/basemud.h src/compat.h src/defs.h
obj/tables.o: src/macros.h src/typedefs.h src/structs.h src/flags.h
obj/tables.o: src/ext_flags.h src/types.h src/act_skills.h src/board.h
obj/tables.o: src/chars.h src/colour.h src/effects.h src/json_tblr.h
obj/tables.o: src/json_tblw.h src/magic.h src/memory.h src/nanny.h
obj/tables.o: src/recycle.h src/special.h src/spell_aff.h src/spell_create.h
obj/tables.o: src/spell_cure.h src/spell_info.h src/spell_misc.h
obj/tables.o: src/spell_move.h src/spell_npc.h src/spell_off.h src/utils.h
obj/types.o: src/types.h src/merc.h src/basemud.h src/compat.h src/defs.h
obj/types.o: src/macros.h src/typedefs.h src/structs.h src/flags.h
obj/types.o: src/ext_flags.h src/lookup.h src/utils.h
obj/update.o: src/update.h src/merc.h src/basemud.h src/compat.h src/defs.h
obj/update.o: src/macros.h src/typedefs.h src/structs.h src/flags.h
obj/update.o: src/ext_flags.h src/types.h src/areas.h src/chars.h src/comm.h
obj/update.o: src/fight.h src/globals.h src/items.h src/lookup.h
obj/update.o: src/mob_prog.h src/mobiles.h src/music.h src/objs.h
obj/update.o: src/players.h src/tables.h src/utils.h src/string.h
obj/utils.o: src/utils.h src/merc.h src/basemud.h src/compat.h src/defs.h
obj/utils.o: src/macros.h src/typedefs.h src/structs.h src/flags.h
obj/utils.o: src/ext_flags.h src/types.h src/chars.h src/comm.h src/globals.h
obj/utils.o: src/interp.h src/string.h
obj/wiz_im.o: src/wiz_im.h src/merc.h src/basemud.h src/compat.h src/defs.h
obj/wiz_im.o: src/macros.h src/typedefs.h src/structs.h src/flags.h
obj/wiz_im.o: src/ext_flags.h src/types.h src/act_info.h src/affects.h
obj/wiz_im.o: src/chars.h src/comm.h src/do_sub.h src/find.h src/globals.h
obj/wiz_im.o: src/interp.h src/lookup.h src/memory.h src/mob_cmds.h
obj/wiz_im.o: src/mob_prog.h src/objs.h src/recycle.h src/rooms.h
obj/wiz_im.o: src/tables.h src/utils.h src/string.h
obj/wiz_l1.o: src/wiz_l1.h src/merc.h src/basemud.h src/compat.h src/defs.h
obj/wiz_l1.o: src/macros.h src/typedefs.h src/structs.h src/flags.h
obj/wiz_l1.o: src/ext_flags.h src/types.h src/act_player.h src/ban.h
obj/wiz_l1.o: src/chars.h src/comm.h src/descs.h src/fight.h src/find.h
obj/wiz_l1.o: src/globals.h src/interp.h src/save.h src/utils.h src/wiz_l4.h
obj/wiz_l1.o: src/string.h
obj/wiz_l2.o: src/wiz_l2.h src/merc.h src/basemud.h src/compat.h src/defs.h
obj/wiz_l2.o: src/macros.h src/typedefs.h src/structs.h src/flags.h
obj/wiz_l2.o: src/ext_flags.h src/types.h src/ban.h src/chars.h src/comm.h
obj/wiz_l2.o: src/find.h src/globals.h src/interp.h src/lookup.h
obj/wiz_l2.o: src/recycle.h src/rooms.h src/tables.h src/utils.h src/string.h
obj/wiz_l3.o: src/wiz_l3.h src/merc.h src/basemud.h src/compat.h src/defs.h
obj/wiz_l3.o: src/macros.h src/typedefs.h src/structs.h src/flags.h
obj/wiz_l3.o: src/ext_flags.h src/types.h src/chars.h src/comm.h src/descs.h
obj/wiz_l3.o: src/find.h src/globals.h src/interp.h src/utils.h
obj/wiz_l4.o: src/wiz_l4.h src/merc.h src/basemud.h src/compat.h src/defs.h
obj/wiz_l4.o: src/macros.h src/typedefs.h src/structs.h src/flags.h
obj/wiz_l4.o: src/ext_flags.h src/types.h src/affects.h src/chars.h
obj/wiz_l4.o: src/comm.h src/descs.h src/fight.h src/find.h src/globals.h
obj/wiz_l4.o: src/interp.h src/lookup.h src/mobiles.h src/objs.h
obj/wiz_l4.o: src/recycle.h src/save.h src/tables.h src/utils.h src/string.h
obj/wiz_l5.o: src/wiz_l5.h src/merc.h src/basemud.h src/compat.h src/defs.h
obj/wiz_l5.o: src/macros.h src/typedefs.h src/structs.h src/flags.h
obj/wiz_l5.o: src/ext_flags.h src/types.h src/act_info.h src/chars.h
obj/wiz_l5.o: src/comm.h src/extra_descrs.h src/fight.h src/find.h
obj/wiz_l5.o: src/globals.h src/interp.h src/lookup.h src/memory.h
obj/wiz_l5.o: src/mobiles.h src/objs.h src/players.h src/recycle.h
obj/wiz_l5.o: src/rooms.h src/utils.h src/string.h
obj/wiz_l6.o: src/wiz_l6.h src/merc.h src/basemud.h src/compat.h src/defs.h
obj/wiz_l6.o: src/macros.h src/typedefs.h src/structs.h src/flags.h
obj/wiz_l6.o: src/ext_flags.h src/types.h src/chars.h src/comm.h src/find.h
obj/wiz_l6.o: src/globals.h src/interp.h src/memory.h src/rooms.h
obj/wiz_l7.o: src/wiz_l7.h src/merc.h src/basemud.h src/compat.h src/defs.h
obj/wiz_l7.o: src/macros.h src/typedefs.h src/structs.h src/flags.h
obj/wiz_l7.o: src/ext_flags.h src/types.h src/chars.h src/comm.h src/find.h
obj/wiz_l7.o: src/globals.h src/interp.h src/objs.h src/rooms.h src/utils.h
obj/wiz_l8.o: src/wiz_l8.h src/merc.h src/basemud.h src/compat.h src/defs.h
obj/wiz_l8.o: src/macros.h src/typedefs.h src/structs.h src/flags.h
obj/wiz_l8.o: src/ext_flags.h src/types.h src/act_info.h src/chars.h
obj/wiz_l8.o: src/comm.h src/fight.h src/find.h src/interp.h src/memory.h
obj/wiz_l8.o: src/rooms.h src/utils.h src/string.h
obj/wiz_ml.o: src/wiz_ml.h src/merc.h src/basemud.h src/compat.h src/defs.h
obj/wiz_ml.o: src/macros.h src/typedefs.h src/structs.h src/flags.h
obj/wiz_ml.o: src/ext_flags.h src/types.h src/act_info.h src/chars.h
obj/wiz_ml.o: src/comm.h src/db.h src/descs.h src/fight.h src/find.h
obj/wiz_ml.o: src/globals.h src/interp.h src/json.h src/json_export.h
obj/wiz_ml.o: src/lookup.h src/memory.h src/mobiles.h src/objs.h
obj/wiz_ml.o: src/players.h src/quickmud.h src/rooms.h src/save.h
obj/wiz_ml.o: src/tables.h src/utils.h src/string.h
