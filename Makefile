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

obj/act_board.o: src/board.h src/merc.h src/basemud.h src/compat.h src/defs.h
obj/act_board.o: src/typedefs.h src/macros.h src/json.h src/structs.h
obj/act_board.o: src/types.h src/flags.h src/tables.h src/ext_flags.h
obj/act_board.o: src/comm.h src/recycle.h src/utils.h src/lookup.h
obj/act_board.o: src/interp.h src/act_info.h src/chars.h src/memory.h
obj/act_board.o: src/globals.h src/act_board.h
obj/act_comm.o: src/interp.h src/merc.h src/basemud.h src/compat.h src/defs.h
obj/act_comm.o: src/typedefs.h src/macros.h src/json.h src/structs.h
obj/act_comm.o: src/types.h src/flags.h src/tables.h src/ext_flags.h
obj/act_comm.o: src/recycle.h src/lookup.h src/comm.h src/utils.h
obj/act_comm.o: src/mob_prog.h src/db.h src/do_sub.h src/chars.h src/find.h
obj/act_comm.o: src/globals.h src/players.h src/memory.h src/act_comm.h
obj/act_conf.o: src/db.h src/merc.h src/basemud.h src/compat.h src/defs.h
obj/act_conf.o: src/typedefs.h src/macros.h src/json.h src/structs.h
obj/act_conf.o: src/types.h src/flags.h src/tables.h src/ext_flags.h
obj/act_conf.o: src/interp.h src/colour.h src/groups.h src/comm.h src/utils.h
obj/act_conf.o: src/lookup.h src/do_sub.h src/chars.h src/memory.h
obj/act_conf.o: src/act_conf.h
obj/act_fight.o: src/affects.h src/merc.h src/basemud.h src/compat.h
obj/act_fight.o: src/defs.h src/typedefs.h src/macros.h src/json.h
obj/act_fight.o: src/structs.h src/types.h src/flags.h src/tables.h
obj/act_fight.o: src/ext_flags.h src/utils.h src/comm.h src/lookup.h
obj/act_fight.o: src/interp.h src/groups.h src/mob_prog.h src/recycle.h
obj/act_fight.o: src/fight.h src/act_comm.h src/chars.h src/find.h
obj/act_fight.o: src/players.h src/act_fight.h
obj/act_group.o: src/interp.h src/merc.h src/basemud.h src/compat.h
obj/act_group.o: src/defs.h src/typedefs.h src/macros.h src/json.h
obj/act_group.o: src/structs.h src/types.h src/flags.h src/tables.h
obj/act_group.o: src/ext_flags.h src/groups.h src/utils.h src/comm.h src/db.h
obj/act_group.o: src/chars.h src/find.h src/globals.h src/act_group.h
obj/act_info.o: src/interp.h src/merc.h src/basemud.h src/compat.h src/defs.h
obj/act_info.o: src/typedefs.h src/macros.h src/json.h src/structs.h
obj/act_info.o: src/types.h src/flags.h src/tables.h src/ext_flags.h
obj/act_info.o: src/magic.h src/recycle.h src/lookup.h src/utils.h
obj/act_info.o: src/groups.h src/db.h src/fight.h src/update.h src/comm.h
obj/act_info.o: src/save.h src/do_sub.h src/act_comm.h src/act_obj.h
obj/act_info.o: src/chars.h src/rooms.h src/objs.h src/find.h
obj/act_info.o: src/spell_info.h src/globals.h src/memory.h src/items.h
obj/act_info.o: src/players.h src/extra_descrs.h src/act_info.h
obj/act_move.o: src/interp.h src/merc.h src/basemud.h src/compat.h src/defs.h
obj/act_move.o: src/typedefs.h src/macros.h src/json.h src/structs.h
obj/act_move.o: src/types.h src/flags.h src/tables.h src/ext_flags.h
obj/act_move.o: src/lookup.h src/utils.h src/comm.h src/mob_prog.h
obj/act_move.o: src/affects.h src/db.h src/fight.h src/groups.h
obj/act_move.o: src/act_info.h src/chars.h src/rooms.h src/objs.h src/find.h
obj/act_move.o: src/globals.h src/items.h src/players.h src/act_move.h
obj/act_obj.o: src/interp.h src/merc.h src/basemud.h src/compat.h src/defs.h
obj/act_obj.o: src/typedefs.h src/macros.h src/json.h src/structs.h
obj/act_obj.o: src/types.h src/flags.h src/tables.h src/ext_flags.h
obj/act_obj.o: src/affects.h src/utils.h src/comm.h src/db.h src/fight.h
obj/act_obj.o: src/groups.h src/mob_prog.h src/save.h src/magic.h
obj/act_obj.o: src/act_group.h src/act_comm.h src/act_move.h src/recycle.h
obj/act_obj.o: src/chars.h src/objs.h src/rooms.h src/find.h src/globals.h
obj/act_obj.o: src/lookup.h src/items.h src/players.h src/act_obj.h
obj/act_olc.o: src/recycle.h src/merc.h src/basemud.h src/compat.h src/defs.h
obj/act_olc.o: src/typedefs.h src/macros.h src/json.h src/structs.h
obj/act_olc.o: src/types.h src/flags.h src/tables.h src/ext_flags.h
obj/act_olc.o: src/lookup.h src/interp.h src/utils.h src/db.h src/comm.h
obj/act_olc.o: src/olc_save.h src/act_info.h src/chars.h src/globals.h
obj/act_olc.o: src/items.h src/mobiles.h src/rooms.h src/objs.h
obj/act_olc.o: src/mob_prog.h src/descs.h src/json_export.h src/resets.h
obj/act_olc.o: src/memory.h src/portals.h src/olc_aedit.h src/olc_hedit.h
obj/act_olc.o: src/olc_medit.h src/olc_mpedit.h src/olc_oedit.h
obj/act_olc.o: src/olc_redit.h src/olc.h src/act_olc.h
obj/act_player.o: src/db.h src/merc.h src/basemud.h src/compat.h src/defs.h
obj/act_player.o: src/typedefs.h src/macros.h src/json.h src/structs.h
obj/act_player.o: src/types.h src/flags.h src/tables.h src/ext_flags.h
obj/act_player.o: src/comm.h src/save.h src/utils.h src/fight.h src/interp.h
obj/act_player.o: src/recycle.h src/chars.h src/descs.h src/memory.h
obj/act_player.o: src/globals.h src/act_player.h
obj/act_shop.o: src/magic.h src/merc.h src/basemud.h src/compat.h src/defs.h
obj/act_shop.o: src/typedefs.h src/macros.h src/json.h src/structs.h
obj/act_shop.o: src/types.h src/flags.h src/tables.h src/ext_flags.h
obj/act_shop.o: src/lookup.h src/comm.h src/interp.h src/utils.h src/db.h
obj/act_shop.o: src/groups.h src/chars.h src/objs.h src/find.h src/act_comm.h
obj/act_shop.o: src/spell_cure.h src/materials.h src/globals.h src/memory.h
obj/act_shop.o: src/items.h src/mobiles.h src/players.h src/rooms.h
obj/act_shop.o: src/act_shop.h
obj/act_skills.o: src/db.h src/merc.h src/basemud.h src/compat.h src/defs.h
obj/act_skills.o: src/typedefs.h src/macros.h src/json.h src/structs.h
obj/act_skills.o: src/types.h src/flags.h src/tables.h src/ext_flags.h
obj/act_skills.o: src/utils.h src/comm.h src/interp.h src/magic.h src/fight.h
obj/act_skills.o: src/lookup.h src/recycle.h src/act_comm.h src/chars.h
obj/act_skills.o: src/find.h src/players.h src/memory.h src/act_skills.h
obj/affects.o: src/recycle.h src/merc.h src/basemud.h src/compat.h src/defs.h
obj/affects.o: src/typedefs.h src/macros.h src/json.h src/structs.h
obj/affects.o: src/types.h src/flags.h src/tables.h src/ext_flags.h
obj/affects.o: src/utils.h src/lookup.h src/chars.h src/affects.h
obj/areas.o: src/globals.h src/merc.h src/basemud.h src/compat.h src/defs.h
obj/areas.o: src/typedefs.h src/macros.h src/json.h src/structs.h src/types.h
obj/areas.o: src/flags.h src/tables.h src/ext_flags.h src/comm.h src/rooms.h
obj/areas.o: src/utils.h src/recycle.h src/resets.h src/areas.h
obj/ban.o: src/recycle.h src/merc.h src/basemud.h src/compat.h src/defs.h
obj/ban.o: src/typedefs.h src/macros.h src/json.h src/structs.h src/types.h
obj/ban.o: src/flags.h src/tables.h src/ext_flags.h src/utils.h src/fread.h
obj/ban.o: src/interp.h src/comm.h src/chars.h src/globals.h src/memory.h
obj/ban.o: src/fwrite.h src/ban.h
obj/board.o: src/interp.h src/merc.h src/basemud.h src/compat.h src/defs.h
obj/board.o: src/typedefs.h src/macros.h src/json.h src/structs.h src/types.h
obj/board.o: src/flags.h src/tables.h src/ext_flags.h src/fread.h src/utils.h
obj/board.o: src/comm.h src/recycle.h src/lookup.h src/chars.h src/descs.h
obj/board.o: src/memory.h src/globals.h src/board.h
obj/boot.o: src/signal.h src/merc.h src/basemud.h src/compat.h src/defs.h
obj/boot.o: src/typedefs.h src/macros.h src/json.h src/structs.h src/types.h
obj/boot.o: src/flags.h src/tables.h src/ext_flags.h src/globals.h
obj/boot.o: src/utils.h src/interp.h src/db.h src/descs.h src/recycle.h
obj/boot.o: src/memory.h src/save.h src/chars.h src/string.h src/olc.h
obj/boot.o: src/nanny.h src/update.h src/comm.h src/rooms.h src/quickmud.h
obj/boot.o: src/act_info.h src/boot.h
obj/chars.o: src/string.h src/merc.h src/basemud.h src/compat.h src/defs.h
obj/chars.o: src/typedefs.h src/macros.h src/json.h src/structs.h src/types.h
obj/chars.o: src/flags.h src/tables.h src/ext_flags.h src/utils.h src/comm.h
obj/chars.o: src/objs.h src/affects.h src/lookup.h src/magic.h src/db.h
obj/chars.o: src/groups.h src/fight.h src/interp.h src/recycle.h src/rooms.h
obj/chars.o: src/mob_prog.h src/wiz_l6.h src/materials.h src/globals.h
obj/chars.o: src/memory.h src/board.h src/update.h src/items.h src/players.h
obj/chars.o: src/mobiles.h src/save.h src/act_move.h src/act_info.h
obj/chars.o: src/act_group.h src/act_player.h src/chars.h
obj/colour.o: src/string.h src/merc.h src/basemud.h src/compat.h src/defs.h
obj/colour.o: src/typedefs.h src/macros.h src/json.h src/structs.h
obj/colour.o: src/types.h src/flags.h src/tables.h src/ext_flags.h
obj/colour.o: src/lookup.h src/chars.h src/colour.h
obj/comm.o: src/string.h src/merc.h src/basemud.h src/compat.h src/defs.h
obj/comm.o: src/typedefs.h src/macros.h src/json.h src/structs.h src/types.h
obj/comm.o: src/flags.h src/tables.h src/ext_flags.h src/colour.h
obj/comm.o: src/recycle.h src/utils.h src/fight.h src/interp.h src/db.h
obj/comm.o: src/olc.h src/save.h src/mob_prog.h src/lookup.h src/act_info.h
obj/comm.o: src/chars.h src/rooms.h src/objs.h src/descs.h src/globals.h
obj/comm.o: src/players.h src/comm.h
obj/db.o: src/string.h src/merc.h src/basemud.h src/compat.h src/defs.h
obj/db.o: src/typedefs.h src/macros.h src/json.h src/structs.h src/types.h
obj/db.o: src/flags.h src/tables.h src/ext_flags.h src/utils.h src/recycle.h
obj/db.o: src/affects.h src/lookup.h src/json_import.h src/music.h src/ban.h
obj/db.o: src/board.h src/portals.h src/rooms.h src/objs.h src/db_old.h
obj/db.o: src/globals.h src/memory.h src/items.h src/mobiles.h src/skills.h
obj/db.o: src/fread.h src/areas.h src/mob_prog.h src/resets.h
obj/db.o: src/extra_descrs.h src/help.h src/db.h
obj/db_old.o: src/string.h src/merc.h src/basemud.h src/compat.h src/defs.h
obj/db_old.o: src/typedefs.h src/macros.h src/json.h src/structs.h
obj/db_old.o: src/types.h src/flags.h src/tables.h src/ext_flags.h src/db.h
obj/db_old.o: src/recycle.h src/utils.h src/interp.h src/lookup.h src/olc.h
obj/db_old.o: src/affects.h src/globals.h src/memory.h src/items.h
obj/db_old.o: src/fread.h src/mobiles.h src/objs.h src/rooms.h
obj/db_old.o: src/extra_descrs.h src/db_old.h
obj/descs.o: src/string.h src/merc.h src/basemud.h src/compat.h src/defs.h
obj/descs.o: src/typedefs.h src/macros.h src/json.h src/structs.h src/types.h
obj/descs.o: src/flags.h src/tables.h src/ext_flags.h src/ban.h src/chars.h
obj/descs.o: src/colour.h src/comm.h src/db.h src/recycle.h src/utils.h
obj/descs.o: src/interp.h src/lookup.h src/globals.h src/memory.h src/descs.h
obj/do_sub.o: src/comm.h src/merc.h src/basemud.h src/compat.h src/defs.h
obj/do_sub.o: src/typedefs.h src/macros.h src/json.h src/structs.h
obj/do_sub.o: src/types.h src/flags.h src/tables.h src/ext_flags.h
obj/do_sub.o: src/chars.h src/do_sub.h
obj/effects.o: src/recycle.h src/merc.h src/basemud.h src/compat.h src/defs.h
obj/effects.o: src/typedefs.h src/macros.h src/json.h src/structs.h
obj/effects.o: src/types.h src/flags.h src/tables.h src/ext_flags.h
obj/effects.o: src/lookup.h src/db.h src/utils.h src/comm.h src/magic.h
obj/effects.o: src/update.h src/affects.h src/objs.h src/chars.h src/items.h
obj/effects.o: src/players.h src/effects.h
obj/ext_flags.o: src/string.h src/merc.h src/basemud.h src/compat.h
obj/ext_flags.o: src/defs.h src/typedefs.h src/macros.h src/json.h
obj/ext_flags.o: src/structs.h src/types.h src/flags.h src/tables.h
obj/ext_flags.o: src/ext_flags.h src/interp.h src/lookup.h src/utils.h
obj/extra_descrs.o: src/utils.h src/merc.h src/basemud.h src/compat.h
obj/extra_descrs.o: src/defs.h src/typedefs.h src/macros.h src/json.h
obj/extra_descrs.o: src/structs.h src/types.h src/flags.h src/tables.h
obj/extra_descrs.o: src/ext_flags.h src/extra_descrs.h
obj/fight.o: src/string.h src/merc.h src/basemud.h src/compat.h src/defs.h
obj/fight.o: src/typedefs.h src/macros.h src/json.h src/structs.h src/types.h
obj/fight.o: src/flags.h src/tables.h src/ext_flags.h src/interp.h
obj/fight.o: src/lookup.h src/update.h src/utils.h src/db.h src/effects.h
obj/fight.o: src/mob_prog.h src/comm.h src/save.h src/groups.h src/magic.h
obj/fight.o: src/affects.h src/act_fight.h src/act_obj.h src/act_comm.h
obj/fight.o: src/act_move.h src/chars.h src/objs.h src/find.h src/globals.h
obj/fight.o: src/memory.h src/items.h src/players.h src/mobiles.h src/fight.h
obj/find.o: src/interp.h src/merc.h src/basemud.h src/compat.h src/defs.h
obj/find.o: src/typedefs.h src/macros.h src/json.h src/structs.h src/types.h
obj/find.o: src/flags.h src/tables.h src/ext_flags.h src/utils.h src/chars.h
obj/find.o: src/db.h src/globals.h src/rooms.h src/find.h
obj/flags.o: src/string.h src/merc.h src/basemud.h src/compat.h src/defs.h
obj/flags.o: src/typedefs.h src/macros.h src/json.h src/structs.h src/types.h
obj/flags.o: src/flags.h src/tables.h src/ext_flags.h src/interp.h
obj/flags.o: src/lookup.h src/utils.h
obj/fread.o: src/string.h src/merc.h src/basemud.h src/compat.h src/defs.h
obj/fread.o: src/typedefs.h src/macros.h src/json.h src/structs.h src/types.h
obj/fread.o: src/flags.h src/tables.h src/ext_flags.h src/utils.h
obj/fread.o: src/memory.h src/globals.h src/fread.h
obj/fwrite.o: src/string.h src/merc.h src/basemud.h src/compat.h src/defs.h
obj/fwrite.o: src/typedefs.h src/macros.h src/json.h src/structs.h
obj/fwrite.o: src/types.h src/flags.h src/tables.h src/ext_flags.h src/save.h
obj/fwrite.o: src/fwrite.h
obj/globals.o: src/globals.h src/merc.h src/basemud.h src/compat.h src/defs.h
obj/globals.o: src/typedefs.h src/macros.h src/json.h src/structs.h
obj/globals.o: src/types.h src/flags.h src/tables.h src/ext_flags.h
obj/groups.o: src/utils.h src/merc.h src/basemud.h src/compat.h src/defs.h
obj/groups.o: src/typedefs.h src/macros.h src/json.h src/structs.h
obj/groups.o: src/types.h src/flags.h src/tables.h src/ext_flags.h
obj/groups.o: src/affects.h src/comm.h src/db.h src/chars.h src/globals.h
obj/groups.o: src/recycle.h src/groups.h
obj/help.o: src/help.h src/merc.h src/basemud.h src/compat.h src/defs.h
obj/help.o: src/typedefs.h src/macros.h src/json.h src/structs.h src/types.h
obj/help.o: src/flags.h src/tables.h src/ext_flags.h
obj/interp.o: src/string.h src/merc.h src/basemud.h src/compat.h src/defs.h
obj/interp.o: src/typedefs.h src/macros.h src/json.h src/structs.h
obj/interp.o: src/types.h src/flags.h src/tables.h src/ext_flags.h
obj/interp.o: src/utils.h src/comm.h src/db.h src/recycle.h src/chars.h
obj/interp.o: src/find.h src/descs.h src/globals.h src/memory.h src/lookup.h
obj/interp.o: src/act_board.h src/act_comm.h src/act_conf.h src/act_fight.h
obj/interp.o: src/act_group.h src/act_info.h src/act_move.h src/act_obj.h
obj/interp.o: src/act_player.h src/act_shop.h src/act_skills.h src/act_olc.h
obj/interp.o: src/wiz_im.h src/wiz_l1.h src/wiz_l2.h src/wiz_l3.h
obj/interp.o: src/wiz_l4.h src/wiz_l5.h src/wiz_l6.h src/wiz_l7.h
obj/interp.o: src/wiz_l8.h src/wiz_ml.h src/olc.h src/mob_cmds.h src/interp.h
obj/items.o: src/string.h src/merc.h src/basemud.h src/compat.h src/defs.h
obj/items.o: src/typedefs.h src/macros.h src/json.h src/structs.h src/types.h
obj/items.o: src/flags.h src/tables.h src/ext_flags.h src/utils.h src/comm.h
obj/items.o: src/interp.h src/objs.h src/lookup.h src/chars.h src/affects.h
obj/items.o: src/magic.h src/db.h src/recycle.h src/rooms.h src/mob_prog.h
obj/items.o: src/groups.h src/globals.h src/music.h src/players.h src/fread.h
obj/items.o: src/fwrite.h src/act_info.h src/act_move.h src/act_group.h
obj/items.o: src/items.h
obj/json.o: src/string.h src/merc.h src/basemud.h src/compat.h src/defs.h
obj/json.o: src/typedefs.h src/macros.h src/json.h src/structs.h src/types.h
obj/json.o: src/flags.h src/tables.h src/ext_flags.h src/utils.h src/memory.h
obj/json_export.o: src/areas.h src/merc.h src/basemud.h src/compat.h
obj/json_export.o: src/defs.h src/typedefs.h src/macros.h src/json.h
obj/json_export.o: src/structs.h src/types.h src/flags.h src/tables.h
obj/json_export.o: src/ext_flags.h src/recycle.h src/utils.h src/json_objw.h
obj/json_export.o: src/json_write.h src/lookup.h src/json_export.h
obj/json_import.o: src/string.h src/merc.h src/basemud.h src/compat.h
obj/json_import.o: src/defs.h src/typedefs.h src/macros.h src/json.h
obj/json_import.o: src/structs.h src/types.h src/flags.h src/tables.h
obj/json_import.o: src/ext_flags.h src/utils.h src/recycle.h src/db.h
obj/json_import.o: src/lookup.h src/portals.h src/globals.h src/memory.h
obj/json_import.o: src/json_objr.h src/help.h src/json_read.h src/rooms.h
obj/json_import.o: src/mobiles.h src/objs.h src/json_import.h
obj/json_objr.o: src/string.h src/merc.h src/basemud.h src/compat.h
obj/json_objr.o: src/defs.h src/typedefs.h src/macros.h src/json.h
obj/json_objr.o: src/structs.h src/types.h src/flags.h src/tables.h
obj/json_objr.o: src/ext_flags.h src/json_import.h src/recycle.h src/memory.h
obj/json_objr.o: src/lookup.h src/portals.h src/db.h src/globals.h
obj/json_objr.o: src/rooms.h src/resets.h src/extra_descrs.h src/affects.h
obj/json_objr.o: src/help.h src/json_objr.h
obj/json_objw.o: src/string.h src/merc.h src/basemud.h src/compat.h
obj/json_objw.o: src/defs.h src/typedefs.h src/macros.h src/json.h
obj/json_objw.o: src/structs.h src/types.h src/flags.h src/tables.h
obj/json_objw.o: src/ext_flags.h src/lookup.h src/utils.h src/json_objw.h
obj/json_read.o: src/string.h src/merc.h src/basemud.h src/compat.h
obj/json_read.o: src/defs.h src/typedefs.h src/macros.h src/json.h
obj/json_read.o: src/structs.h src/types.h src/flags.h src/tables.h
obj/json_read.o: src/ext_flags.h src/utils.h src/json_read.h
obj/json_tblr.o: src/string.h src/merc.h src/basemud.h src/compat.h
obj/json_tblr.o: src/defs.h src/typedefs.h src/macros.h src/json.h
obj/json_tblr.o: src/structs.h src/types.h src/flags.h src/tables.h
obj/json_tblr.o: src/ext_flags.h src/lookup.h src/colour.h src/memory.h
obj/json_tblr.o: src/json_import.h src/json_tblr.h
obj/json_tblw.o: src/string.h src/merc.h src/basemud.h src/compat.h
obj/json_tblw.o: src/defs.h src/typedefs.h src/macros.h src/json.h
obj/json_tblw.o: src/structs.h src/types.h src/flags.h src/tables.h
obj/json_tblw.o: src/ext_flags.h src/lookup.h src/colour.h src/json_tblw.h
obj/json_write.o: src/string.h src/merc.h src/basemud.h src/compat.h
obj/json_write.o: src/defs.h src/typedefs.h src/macros.h src/json.h
obj/json_write.o: src/structs.h src/types.h src/flags.h src/tables.h
obj/json_write.o: src/ext_flags.h src/utils.h src/json_write.h
obj/lookup.o: src/string.h src/merc.h src/basemud.h src/compat.h src/defs.h
obj/lookup.o: src/typedefs.h src/macros.h src/json.h src/structs.h
obj/lookup.o: src/types.h src/flags.h src/tables.h src/ext_flags.h
obj/lookup.o: src/utils.h src/interp.h src/db.h src/recycle.h src/globals.h
obj/lookup.o: src/lookup.h
obj/magic.o: src/string.h src/merc.h src/basemud.h src/compat.h src/defs.h
obj/magic.o: src/typedefs.h src/macros.h src/json.h src/structs.h src/types.h
obj/magic.o: src/flags.h src/tables.h src/ext_flags.h src/lookup.h
obj/magic.o: src/affects.h src/comm.h src/fight.h src/utils.h src/chars.h
obj/magic.o: src/magic.h
obj/materials.o: src/string.h src/merc.h src/basemud.h src/compat.h
obj/materials.o: src/defs.h src/typedefs.h src/macros.h src/json.h
obj/materials.o: src/structs.h src/types.h src/flags.h src/tables.h
obj/materials.o: src/ext_flags.h src/materials.h
obj/memory.o: src/string.h src/merc.h src/basemud.h src/compat.h src/defs.h
obj/memory.o: src/typedefs.h src/macros.h src/json.h src/structs.h
obj/memory.o: src/types.h src/flags.h src/tables.h src/ext_flags.h
obj/memory.o: src/utils.h src/globals.h src/recycle.h src/memory.h
obj/mob_cmds.o: src/lookup.h src/merc.h src/basemud.h src/compat.h src/defs.h
obj/mob_cmds.o: src/typedefs.h src/macros.h src/json.h src/structs.h
obj/mob_cmds.o: src/types.h src/flags.h src/tables.h src/ext_flags.h
obj/mob_cmds.o: src/interp.h src/groups.h src/utils.h src/comm.h src/fight.h
obj/mob_cmds.o: src/mob_prog.h src/db.h src/chars.h src/objs.h src/rooms.h
obj/mob_cmds.o: src/find.h src/act_info.h src/globals.h src/mobiles.h
obj/mob_cmds.o: src/mob_cmds.h
obj/mob_prog.o: src/string.h src/merc.h src/basemud.h src/compat.h src/defs.h
obj/mob_prog.o: src/typedefs.h src/macros.h src/json.h src/structs.h
obj/mob_prog.o: src/types.h src/flags.h src/tables.h src/ext_flags.h
obj/mob_prog.o: src/lookup.h src/utils.h src/groups.h src/interp.h
obj/mob_prog.o: src/mob_cmds.h src/db.h src/chars.h src/objs.h src/find.h
obj/mob_prog.o: src/globals.h src/mob_prog.h
obj/mobiles.o: src/globals.h src/merc.h src/basemud.h src/compat.h src/defs.h
obj/mobiles.o: src/typedefs.h src/macros.h src/json.h src/structs.h
obj/mobiles.o: src/types.h src/flags.h src/tables.h src/ext_flags.h
obj/mobiles.o: src/utils.h src/recycle.h src/memory.h src/chars.h
obj/mobiles.o: src/affects.h src/lookup.h src/fight.h src/interp.h
obj/mobiles.o: src/act_fight.h src/magic.h src/objs.h src/items.h
obj/mobiles.o: src/mob_prog.h src/comm.h src/db.h src/mobiles.h
obj/music.o: src/recycle.h src/merc.h src/basemud.h src/compat.h src/defs.h
obj/music.o: src/typedefs.h src/macros.h src/json.h src/structs.h src/types.h
obj/music.o: src/flags.h src/tables.h src/ext_flags.h src/utils.h src/comm.h
obj/music.o: src/interp.h src/globals.h src/lookup.h src/chars.h src/objs.h
obj/music.o: src/memory.h src/fread.h src/music.h
obj/nanny.o: src/string.h src/merc.h src/basemud.h src/compat.h src/defs.h
obj/nanny.o: src/typedefs.h src/macros.h src/json.h src/structs.h src/types.h
obj/nanny.o: src/flags.h src/tables.h src/ext_flags.h src/interp.h
obj/nanny.o: src/recycle.h src/lookup.h src/utils.h src/comm.h src/db.h
obj/nanny.o: src/save.h src/ban.h src/fight.h src/act_info.h src/act_skills.h
obj/nanny.o: src/act_board.h src/act_obj.h src/chars.h src/objs.h src/descs.h
obj/nanny.o: src/globals.h src/memory.h src/magic.h src/players.h src/rooms.h
obj/nanny.o: src/nanny.h
obj/objs.o: src/string.h src/merc.h src/basemud.h src/compat.h src/defs.h
obj/objs.o: src/typedefs.h src/macros.h src/json.h src/structs.h src/types.h
obj/objs.o: src/flags.h src/tables.h src/ext_flags.h src/affects.h
obj/objs.o: src/utils.h src/chars.h src/db.h src/recycle.h src/comm.h
obj/objs.o: src/lookup.h src/materials.h src/globals.h src/memory.h
obj/objs.o: src/items.h src/extra_descrs.h src/objs.h
obj/olc.o: src/string.h src/merc.h src/basemud.h src/compat.h src/defs.h
obj/olc.o: src/typedefs.h src/macros.h src/json.h src/structs.h src/types.h
obj/olc.o: src/flags.h src/tables.h src/ext_flags.h src/utils.h src/interp.h
obj/olc.o: src/comm.h src/db.h src/recycle.h src/lookup.h src/magic.h
obj/olc.o: src/chars.h src/memory.h src/act_olc.h src/olc_aedit.h
obj/olc.o: src/olc_hedit.h src/olc_medit.h src/olc_mpedit.h src/olc_oedit.h
obj/olc.o: src/olc_redit.h src/olc.h
obj/olc_aedit.o: src/string.h src/merc.h src/basemud.h src/compat.h
obj/olc_aedit.o: src/defs.h src/typedefs.h src/macros.h src/json.h
obj/olc_aedit.o: src/structs.h src/types.h src/flags.h src/tables.h
obj/olc_aedit.o: src/ext_flags.h src/comm.h src/lookup.h src/db.h
obj/olc_aedit.o: src/recycle.h src/interp.h src/globals.h src/olc.h
obj/olc_aedit.o: src/memory.h src/areas.h src/olc_aedit.h
obj/olc_hedit.o: src/string.h src/merc.h src/basemud.h src/compat.h
obj/olc_hedit.o: src/defs.h src/typedefs.h src/macros.h src/json.h
obj/olc_hedit.o: src/structs.h src/types.h src/flags.h src/tables.h
obj/olc_hedit.o: src/ext_flags.h src/comm.h src/lookup.h src/db.h
obj/olc_hedit.o: src/recycle.h src/utils.h src/interp.h src/globals.h
obj/olc_hedit.o: src/olc.h src/memory.h src/help.h src/olc_hedit.h
obj/olc_medit.o: src/string.h src/merc.h src/basemud.h src/compat.h
obj/olc_medit.o: src/defs.h src/typedefs.h src/macros.h src/json.h
obj/olc_medit.o: src/structs.h src/types.h src/flags.h src/tables.h
obj/olc_medit.o: src/ext_flags.h src/comm.h src/lookup.h src/db.h
obj/olc_medit.o: src/recycle.h src/utils.h src/interp.h src/mob_cmds.h
obj/olc_medit.o: src/chars.h src/globals.h src/olc.h src/memory.h
obj/olc_medit.o: src/mobiles.h src/mob_prog.h src/olc_medit.h
obj/olc_mpedit.o: src/string.h src/merc.h src/basemud.h src/compat.h
obj/olc_mpedit.o: src/defs.h src/typedefs.h src/macros.h src/json.h
obj/olc_mpedit.o: src/structs.h src/types.h src/flags.h src/tables.h
obj/olc_mpedit.o: src/ext_flags.h src/comm.h src/db.h src/recycle.h
obj/olc_mpedit.o: src/utils.h src/lookup.h src/chars.h src/globals.h
obj/olc_mpedit.o: src/olc.h src/memory.h src/mob_prog.h src/olc_mpedit.h
obj/olc_oedit.o: src/string.h src/merc.h src/basemud.h src/compat.h
obj/olc_oedit.o: src/defs.h src/typedefs.h src/macros.h src/json.h
obj/olc_oedit.o: src/structs.h src/types.h src/flags.h src/tables.h
obj/olc_oedit.o: src/ext_flags.h src/comm.h src/lookup.h src/db.h
obj/olc_oedit.o: src/recycle.h src/utils.h src/interp.h src/affects.h
obj/olc_oedit.o: src/act_info.h src/chars.h src/globals.h src/olc.h
obj/olc_oedit.o: src/memory.h src/items.h src/objs.h src/extra_descrs.h
obj/olc_oedit.o: src/olc_oedit.h
obj/olc_redit.o: src/string.h src/merc.h src/basemud.h src/compat.h
obj/olc_redit.o: src/defs.h src/typedefs.h src/macros.h src/json.h
obj/olc_redit.o: src/structs.h src/types.h src/flags.h src/tables.h
obj/olc_redit.o: src/ext_flags.h src/comm.h src/lookup.h src/db.h
obj/olc_redit.o: src/recycle.h src/utils.h src/interp.h src/act_info.h
obj/olc_redit.o: src/chars.h src/objs.h src/find.h src/globals.h src/olc.h
obj/olc_redit.o: src/olc_medit.h src/olc_oedit.h src/memory.h src/mobiles.h
obj/olc_redit.o: src/rooms.h src/extra_descrs.h src/resets.h src/portals.h
obj/olc_redit.o: src/olc_redit.h
obj/olc_save.o: src/string.h src/merc.h src/basemud.h src/compat.h src/defs.h
obj/olc_save.o: src/typedefs.h src/macros.h src/json.h src/structs.h
obj/olc_save.o: src/types.h src/flags.h src/tables.h src/ext_flags.h
obj/olc_save.o: src/lookup.h src/utils.h src/db.h src/mob_cmds.h src/comm.h
obj/olc_save.o: src/interp.h src/globals.h src/olc.h src/items.h src/fread.h
obj/olc_save.o: src/fwrite.h src/objs.h src/mobiles.h src/rooms.h
obj/olc_save.o: src/mob_prog.h src/json_export.h src/olc_save.h
obj/players.o: src/string.h src/merc.h src/basemud.h src/compat.h src/defs.h
obj/players.o: src/typedefs.h src/macros.h src/json.h src/structs.h
obj/players.o: src/types.h src/flags.h src/tables.h src/ext_flags.h
obj/players.o: src/chars.h src/lookup.h src/objs.h src/affects.h src/memory.h
obj/players.o: src/utils.h src/globals.h src/comm.h src/save.h src/magic.h
obj/players.o: src/groups.h src/fight.h src/rooms.h src/items.h src/players.h
obj/portals.o: src/string.h src/merc.h src/basemud.h src/compat.h src/defs.h
obj/portals.o: src/typedefs.h src/macros.h src/json.h src/structs.h
obj/portals.o: src/types.h src/flags.h src/tables.h src/ext_flags.h
obj/portals.o: src/lookup.h src/recycle.h src/utils.h src/memory.h
obj/portals.o: src/rooms.h src/portals.h
obj/quickmud.o: src/utils.h src/merc.h src/basemud.h src/compat.h src/defs.h
obj/quickmud.o: src/typedefs.h src/macros.h src/json.h src/structs.h
obj/quickmud.o: src/types.h src/flags.h src/tables.h src/ext_flags.h
obj/quickmud.o: src/fread.h src/quickmud.h
obj/recycle.o: src/string.h src/merc.h src/basemud.h src/compat.h src/defs.h
obj/recycle.o: src/typedefs.h src/macros.h src/json.h src/structs.h
obj/recycle.o: src/types.h src/flags.h src/tables.h src/ext_flags.h
obj/recycle.o: src/utils.h src/lookup.h src/db.h src/affects.h src/objs.h
obj/recycle.o: src/chars.h src/globals.h src/memory.h src/resets.h
obj/recycle.o: src/board.h src/extra_descrs.h src/mob_prog.h src/rooms.h
obj/recycle.o: src/mobiles.h src/help.h src/portals.h src/recycle.h
obj/resets.o: src/mobiles.h src/merc.h src/basemud.h src/compat.h src/defs.h
obj/resets.o: src/typedefs.h src/macros.h src/json.h src/structs.h
obj/resets.o: src/types.h src/flags.h src/tables.h src/ext_flags.h
obj/resets.o: src/utils.h src/rooms.h src/chars.h src/objs.h src/items.h
obj/resets.o: src/recycle.h src/resets.h
obj/rooms.o: src/db.h src/merc.h src/basemud.h src/compat.h src/defs.h
obj/rooms.o: src/typedefs.h src/macros.h src/json.h src/structs.h src/types.h
obj/rooms.o: src/flags.h src/tables.h src/ext_flags.h src/lookup.h
obj/rooms.o: src/utils.h src/objs.h src/interp.h src/chars.h src/globals.h
obj/rooms.o: src/mobiles.h src/items.h src/resets.h src/recycle.h src/rooms.h
obj/save.o: src/recycle.h src/merc.h src/basemud.h src/compat.h src/defs.h
obj/save.o: src/typedefs.h src/macros.h src/json.h src/structs.h src/types.h
obj/save.o: src/flags.h src/tables.h src/ext_flags.h src/lookup.h
obj/save.o: src/colour.h src/db.h src/utils.h src/board.h src/chars.h
obj/save.o: src/objs.h src/globals.h src/memory.h src/items.h src/mobiles.h
obj/save.o: src/players.h src/fread.h src/fwrite.h src/rooms.h src/affects.h
obj/save.o: src/extra_descrs.h src/save.h
obj/sha256.o: src/string.h src/merc.h src/basemud.h src/compat.h src/defs.h
obj/sha256.o: src/typedefs.h src/macros.h src/json.h src/structs.h
obj/sha256.o: src/types.h src/flags.h src/tables.h src/ext_flags.h
obj/sha256.o: src/sha256.h
obj/signal.o: src/signal.h src/merc.h src/basemud.h src/compat.h src/defs.h
obj/signal.o: src/typedefs.h src/macros.h src/json.h src/structs.h
obj/signal.o: src/types.h src/flags.h src/tables.h src/ext_flags.h
obj/signal.o: src/utils.h src/globals.h
obj/skills.o: src/string.h src/merc.h src/basemud.h src/compat.h src/defs.h
obj/skills.o: src/typedefs.h src/macros.h src/json.h src/structs.h
obj/skills.o: src/types.h src/flags.h src/tables.h src/ext_flags.h
obj/skills.o: src/lookup.h src/utils.h src/skills.h
obj/special.o: src/utils.h src/merc.h src/basemud.h src/compat.h src/defs.h
obj/special.o: src/typedefs.h src/macros.h src/json.h src/structs.h
obj/special.o: src/types.h src/flags.h src/tables.h src/ext_flags.h
obj/special.o: src/fight.h src/comm.h src/interp.h src/magic.h src/lookup.h
obj/special.o: src/db.h src/act_fight.h src/act_comm.h src/act_move.h
obj/special.o: src/chars.h src/objs.h src/spell_aff.h src/globals.h
obj/special.o: src/rooms.h src/special.h
obj/spell_aff.o: src/magic.h src/merc.h src/basemud.h src/compat.h src/defs.h
obj/spell_aff.o: src/typedefs.h src/macros.h src/json.h src/structs.h
obj/spell_aff.o: src/types.h src/flags.h src/tables.h src/ext_flags.h
obj/spell_aff.o: src/utils.h src/affects.h src/comm.h src/chars.h src/fight.h
obj/spell_aff.o: src/groups.h src/objs.h src/recycle.h src/lookup.h
obj/spell_aff.o: src/items.h src/extra_descrs.h src/spell_aff.h
obj/spell_create.o: src/magic.h src/merc.h src/basemud.h src/compat.h
obj/spell_create.o: src/defs.h src/typedefs.h src/macros.h src/json.h
obj/spell_create.o: src/structs.h src/types.h src/flags.h src/tables.h
obj/spell_create.o: src/ext_flags.h src/find.h src/comm.h src/db.h src/objs.h
obj/spell_create.o: src/utils.h src/chars.h src/globals.h src/memory.h
obj/spell_create.o: src/items.h src/spell_create.h
obj/spell_cure.o: src/magic.h src/merc.h src/basemud.h src/compat.h
obj/spell_cure.o: src/defs.h src/typedefs.h src/macros.h src/json.h
obj/spell_cure.o: src/structs.h src/types.h src/flags.h src/tables.h
obj/spell_cure.o: src/ext_flags.h src/comm.h src/utils.h src/lookup.h
obj/spell_cure.o: src/fight.h src/chars.h src/objs.h src/spell_cure.h
obj/spell_info.o: src/magic.h src/merc.h src/basemud.h src/compat.h
obj/spell_info.o: src/defs.h src/typedefs.h src/macros.h src/json.h
obj/spell_info.o: src/structs.h src/types.h src/flags.h src/tables.h
obj/spell_info.o: src/ext_flags.h src/comm.h src/utils.h src/lookup.h
obj/spell_info.o: src/chars.h src/recycle.h src/db.h src/interp.h
obj/spell_info.o: src/act_info.h src/affects.h src/objs.h src/globals.h
obj/spell_info.o: src/items.h src/memory.h src/spell_info.h
obj/spell_misc.o: src/magic.h src/merc.h src/basemud.h src/compat.h
obj/spell_misc.o: src/defs.h src/typedefs.h src/macros.h src/json.h
obj/spell_misc.o: src/structs.h src/types.h src/flags.h src/tables.h
obj/spell_misc.o: src/ext_flags.h src/comm.h src/utils.h src/lookup.h
obj/spell_misc.o: src/db.h src/affects.h src/objs.h src/interp.h src/chars.h
obj/spell_misc.o: src/globals.h src/items.h src/spell_misc.h
obj/spell_move.o: src/magic.h src/merc.h src/basemud.h src/compat.h
obj/spell_move.o: src/defs.h src/typedefs.h src/macros.h src/json.h
obj/spell_move.o: src/structs.h src/types.h src/flags.h src/tables.h
obj/spell_move.o: src/ext_flags.h src/find.h src/chars.h src/comm.h
obj/spell_move.o: src/interp.h src/act_info.h src/db.h src/fight.h src/objs.h
obj/spell_move.o: src/items.h src/players.h src/rooms.h src/spell_move.h
obj/spell_npc.o: src/magic.h src/merc.h src/basemud.h src/compat.h src/defs.h
obj/spell_npc.o: src/typedefs.h src/macros.h src/json.h src/structs.h
obj/spell_npc.o: src/types.h src/flags.h src/tables.h src/ext_flags.h
obj/spell_npc.o: src/utils.h src/lookup.h src/effects.h src/fight.h
obj/spell_npc.o: src/comm.h src/chars.h src/spell_npc.h
obj/spell_off.o: src/magic.h src/merc.h src/basemud.h src/compat.h src/defs.h
obj/spell_off.o: src/typedefs.h src/macros.h src/json.h src/structs.h
obj/spell_off.o: src/types.h src/flags.h src/tables.h src/ext_flags.h
obj/spell_off.o: src/utils.h src/fight.h src/comm.h src/affects.h
obj/spell_off.o: src/lookup.h src/db.h src/spell_aff.h src/chars.h src/objs.h
obj/spell_off.o: src/globals.h src/items.h src/players.h src/spell_off.h
obj/string.o: src/string.h src/merc.h src/basemud.h src/compat.h src/defs.h
obj/string.o: src/typedefs.h src/macros.h src/json.h src/structs.h
obj/string.o: src/types.h src/flags.h src/tables.h src/ext_flags.h src/comm.h
obj/string.o: src/utils.h src/interp.h src/olc.h src/db.h src/olc_mpedit.h
obj/string.o: src/globals.h src/memory.h
obj/tables.o: src/nanny.h src/merc.h src/basemud.h src/compat.h src/defs.h
obj/tables.o: src/typedefs.h src/macros.h src/json.h src/structs.h
obj/tables.o: src/types.h src/flags.h src/tables.h src/ext_flags.h
obj/tables.o: src/lookup.h src/recycle.h src/colour.h src/board.h
obj/tables.o: src/special.h src/effects.h src/json_tblr.h src/json_tblw.h
obj/tables.o: src/magic.h src/memory.h src/utils.h src/chars.h
obj/tables.o: src/spell_aff.h src/spell_create.h src/spell_cure.h
obj/tables.o: src/spell_info.h src/spell_misc.h src/spell_move.h
obj/tables.o: src/spell_npc.h src/spell_off.h src/act_skills.h
obj/types.o: src/merc.h src/basemud.h src/compat.h src/defs.h src/typedefs.h
obj/types.o: src/macros.h src/json.h src/structs.h src/types.h src/flags.h
obj/types.o: src/tables.h src/ext_flags.h src/lookup.h src/utils.h
obj/update.o: src/string.h src/merc.h src/basemud.h src/compat.h src/defs.h
obj/update.o: src/typedefs.h src/macros.h src/json.h src/structs.h
obj/update.o: src/types.h src/flags.h src/tables.h src/ext_flags.h
obj/update.o: src/chars.h src/utils.h src/players.h src/items.h src/comm.h
obj/update.o: src/music.h src/globals.h src/lookup.h src/fight.h
obj/update.o: src/mob_prog.h src/areas.h src/mobiles.h src/objs.h
obj/update.o: src/update.h
obj/utils.o: src/string.h src/merc.h src/basemud.h src/compat.h src/defs.h
obj/utils.o: src/typedefs.h src/macros.h src/json.h src/structs.h src/types.h
obj/utils.o: src/flags.h src/tables.h src/ext_flags.h src/comm.h src/db.h
obj/utils.o: src/interp.h src/chars.h src/globals.h src/utils.h
obj/wiz_im.o: src/string.h src/merc.h src/basemud.h src/compat.h src/defs.h
obj/wiz_im.o: src/typedefs.h src/macros.h src/json.h src/structs.h
obj/wiz_im.o: src/types.h src/flags.h src/tables.h src/ext_flags.h
obj/wiz_im.o: src/interp.h src/db.h src/utils.h src/comm.h src/lookup.h
obj/wiz_im.o: src/recycle.h src/do_sub.h src/mob_cmds.h src/act_info.h
obj/wiz_im.o: src/chars.h src/objs.h src/rooms.h src/find.h src/affects.h
obj/wiz_im.o: src/globals.h src/memory.h src/mob_prog.h src/wiz_im.h
obj/wiz_l1.o: src/string.h src/merc.h src/basemud.h src/compat.h src/defs.h
obj/wiz_l1.o: src/typedefs.h src/macros.h src/json.h src/structs.h
obj/wiz_l1.o: src/types.h src/flags.h src/tables.h src/ext_flags.h src/db.h
obj/wiz_l1.o: src/comm.h src/interp.h src/save.h src/fight.h src/ban.h
obj/wiz_l1.o: src/utils.h src/act_player.h src/wiz_l4.h src/chars.h
obj/wiz_l1.o: src/find.h src/descs.h src/globals.h src/wiz_l1.h
obj/wiz_l2.o: src/string.h src/merc.h src/basemud.h src/compat.h src/defs.h
obj/wiz_l2.o: src/typedefs.h src/macros.h src/json.h src/structs.h
obj/wiz_l2.o: src/types.h src/flags.h src/tables.h src/ext_flags.h src/db.h
obj/wiz_l2.o: src/comm.h src/interp.h src/ban.h src/utils.h src/recycle.h
obj/wiz_l2.o: src/lookup.h src/chars.h src/rooms.h src/find.h src/globals.h
obj/wiz_l2.o: src/wiz_l2.h
obj/wiz_l3.o: src/db.h src/merc.h src/basemud.h src/compat.h src/defs.h
obj/wiz_l3.o: src/typedefs.h src/macros.h src/json.h src/structs.h
obj/wiz_l3.o: src/types.h src/flags.h src/tables.h src/ext_flags.h src/comm.h
obj/wiz_l3.o: src/interp.h src/fight.h src/utils.h src/chars.h src/find.h
obj/wiz_l3.o: src/descs.h src/globals.h src/wiz_l3.h
obj/wiz_l4.o: src/string.h src/merc.h src/basemud.h src/compat.h src/defs.h
obj/wiz_l4.o: src/typedefs.h src/macros.h src/json.h src/structs.h
obj/wiz_l4.o: src/types.h src/flags.h src/tables.h src/ext_flags.h src/db.h
obj/wiz_l4.o: src/comm.h src/interp.h src/save.h src/fight.h src/utils.h
obj/wiz_l4.o: src/lookup.h src/affects.h src/chars.h src/objs.h src/find.h
obj/wiz_l4.o: src/recycle.h src/descs.h src/globals.h src/mobiles.h
obj/wiz_l4.o: src/wiz_l4.h
obj/wiz_l5.o: src/string.h src/merc.h src/basemud.h src/compat.h src/defs.h
obj/wiz_l5.o: src/typedefs.h src/macros.h src/json.h src/structs.h
obj/wiz_l5.o: src/types.h src/flags.h src/tables.h src/ext_flags.h src/db.h
obj/wiz_l5.o: src/comm.h src/interp.h src/save.h src/fight.h src/utils.h
obj/wiz_l5.o: src/lookup.h src/affects.h src/do_sub.h src/recycle.h
obj/wiz_l5.o: src/act_info.h src/chars.h src/objs.h src/rooms.h src/find.h
obj/wiz_l5.o: src/memory.h src/globals.h src/mobiles.h src/players.h
obj/wiz_l5.o: src/extra_descrs.h src/wiz_l5.h
obj/wiz_l6.o: src/db.h src/merc.h src/basemud.h src/compat.h src/defs.h
obj/wiz_l6.o: src/typedefs.h src/macros.h src/json.h src/structs.h
obj/wiz_l6.o: src/types.h src/flags.h src/tables.h src/ext_flags.h src/comm.h
obj/wiz_l6.o: src/interp.h src/chars.h src/rooms.h src/find.h src/globals.h
obj/wiz_l6.o: src/memory.h src/wiz_l6.h
obj/wiz_l7.o: src/db.h src/merc.h src/basemud.h src/compat.h src/defs.h
obj/wiz_l7.o: src/typedefs.h src/macros.h src/json.h src/structs.h
obj/wiz_l7.o: src/types.h src/flags.h src/tables.h src/ext_flags.h src/comm.h
obj/wiz_l7.o: src/interp.h src/utils.h src/chars.h src/rooms.h src/find.h
obj/wiz_l7.o: src/globals.h src/objs.h src/wiz_l7.h
obj/wiz_l8.o: src/string.h src/merc.h src/basemud.h src/compat.h src/defs.h
obj/wiz_l8.o: src/typedefs.h src/macros.h src/json.h src/structs.h
obj/wiz_l8.o: src/types.h src/flags.h src/tables.h src/ext_flags.h src/db.h
obj/wiz_l8.o: src/comm.h src/interp.h src/fight.h src/utils.h src/act_info.h
obj/wiz_l8.o: src/chars.h src/rooms.h src/find.h src/memory.h src/wiz_l8.h
obj/wiz_ml.o: src/string.h src/merc.h src/basemud.h src/compat.h src/defs.h
obj/wiz_ml.o: src/typedefs.h src/macros.h src/json.h src/structs.h
obj/wiz_ml.o: src/types.h src/flags.h src/tables.h src/ext_flags.h src/db.h
obj/wiz_ml.o: src/comm.h src/interp.h src/save.h src/fight.h src/utils.h
obj/wiz_ml.o: src/act_info.h src/chars.h src/rooms.h src/find.h src/descs.h
obj/wiz_ml.o: src/boot.h src/memory.h src/globals.h src/players.h
obj/wiz_ml.o: src/mobiles.h src/objs.h src/quickmud.h src/json_export.h
obj/wiz_ml.o: src/lookup.h src/wiz_ml.h
