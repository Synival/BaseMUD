CC      = gcc
PROF    = -g -O
NOCRYPT =
C_FLAGS =  -Wall $(PROF) $(NOCRYPT)
L_FLAGS =  $(PROF)
LIBS    = -lcrypt
EXE     = ../bin/rom

O_FILES = \
    act_board.o act_comm.o act_conf.o act_fight.o act_group.o act_info.o \
    act_move.o act_obj.o act_olc.o act_player.o act_shop.o act_skills.o \
    affects.o ban.o board.o chars.o colour.o comm.o db2.o db.o do_sub.o \
    effects.o fight.o find.o flags.o groups.o interp.o json.o json_obj.o \
    json_tbl.o lookup.o main.o magic.o materials.o mob_cmds.o mob_prog.o music.o nanny.o \
    objs.o olc.o olc_aedit.o olc_hedit.o olc_medit.o olc_mpedit.o olc_oedit.o \
    olc_redit.o olc_save.o portals.o recycle.o rooms.o save.o signal.o \
    skills.o special.o spell_aff.o spell_create.o spell_cure.o spell_info.o \
    spell_misc.o spell_move.o spell_npc.o spell_off.o string.o tables.o \
    types.o update.o utils.o wiz_im.o wiz_l1.o wiz_l2.o wiz_l3.o wiz_l4.o \
    wiz_l5.o wiz_l6.o wiz_l7.o wiz_l8.o wiz_ml.o

rom: $(O_FILES)
	$(CC) $(L_FLAGS) -o $(EXE) $(O_FILES) $(LIBS)

.c.o: merc.h
	$(CC) -c $(C_FLAGS) $<

clean:
	rm *.o

# DO NOT DELETE

act_board.o: /usr/include/string.h /usr/include/features.h
act_board.o: /usr/include/stdc-predef.h /usr/include/sys/cdefs.h
act_board.o: /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h
act_board.o: /usr/include/xlocale.h /usr/include/stdlib.h
act_board.o: /usr/include/bits/waitflags.h /usr/include/bits/waitstatus.h
act_board.o: /usr/include/endian.h /usr/include/bits/endian.h
act_board.o: /usr/include/bits/byteswap.h /usr/include/bits/types.h
act_board.o: /usr/include/bits/typesizes.h /usr/include/bits/byteswap-16.h
act_board.o: /usr/include/sys/types.h /usr/include/time.h
act_board.o: /usr/include/sys/select.h /usr/include/bits/select.h
act_board.o: /usr/include/bits/sigset.h /usr/include/bits/time.h
act_board.o: /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h
act_board.o: /usr/include/alloca.h /usr/include/bits/stdlib-float.h board.h
act_board.o: merc.h /usr/include/stdio.h /usr/include/libio.h
act_board.o: /usr/include/_G_config.h /usr/include/wchar.h
act_board.o: /usr/include/bits/wchar.h /usr/include/bits/stdio_lim.h
act_board.o: /usr/include/bits/sys_errlist.h compat.h defs.h
act_board.o: /usr/include/sys/time.h macros.h json.h structs.h types.h
act_board.o: flags.h tables.h comm.h recycle.h utils.h lookup.h interp.h
act_board.o: act_info.h chars.h act_board.h
act_comm.o: /usr/include/string.h /usr/include/features.h
act_comm.o: /usr/include/stdc-predef.h /usr/include/sys/cdefs.h
act_comm.o: /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h
act_comm.o: /usr/include/xlocale.h /usr/include/stdlib.h
act_comm.o: /usr/include/bits/waitflags.h /usr/include/bits/waitstatus.h
act_comm.o: /usr/include/endian.h /usr/include/bits/endian.h
act_comm.o: /usr/include/bits/byteswap.h /usr/include/bits/types.h
act_comm.o: /usr/include/bits/typesizes.h /usr/include/bits/byteswap-16.h
act_comm.o: /usr/include/sys/types.h /usr/include/time.h
act_comm.o: /usr/include/sys/select.h /usr/include/bits/select.h
act_comm.o: /usr/include/bits/sigset.h /usr/include/bits/time.h
act_comm.o: /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h
act_comm.o: /usr/include/alloca.h /usr/include/bits/stdlib-float.h
act_comm.o: /usr/include/ctype.h interp.h merc.h /usr/include/stdio.h
act_comm.o: /usr/include/libio.h /usr/include/_G_config.h
act_comm.o: /usr/include/wchar.h /usr/include/bits/wchar.h
act_comm.o: /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h
act_comm.o: compat.h defs.h /usr/include/sys/time.h macros.h json.h structs.h
act_comm.o: types.h flags.h tables.h recycle.h lookup.h comm.h utils.h
act_comm.o: mob_prog.h db.h do_sub.h chars.h find.h act_comm.h
act_conf.o: /usr/include/string.h /usr/include/features.h
act_conf.o: /usr/include/stdc-predef.h /usr/include/sys/cdefs.h
act_conf.o: /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h
act_conf.o: /usr/include/xlocale.h /usr/include/stdlib.h
act_conf.o: /usr/include/bits/waitflags.h /usr/include/bits/waitstatus.h
act_conf.o: /usr/include/endian.h /usr/include/bits/endian.h
act_conf.o: /usr/include/bits/byteswap.h /usr/include/bits/types.h
act_conf.o: /usr/include/bits/typesizes.h /usr/include/bits/byteswap-16.h
act_conf.o: /usr/include/sys/types.h /usr/include/time.h
act_conf.o: /usr/include/sys/select.h /usr/include/bits/select.h
act_conf.o: /usr/include/bits/sigset.h /usr/include/bits/time.h
act_conf.o: /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h
act_conf.o: /usr/include/alloca.h /usr/include/bits/stdlib-float.h db.h
act_conf.o: merc.h /usr/include/stdio.h /usr/include/libio.h
act_conf.o: /usr/include/_G_config.h /usr/include/wchar.h
act_conf.o: /usr/include/bits/wchar.h /usr/include/bits/stdio_lim.h
act_conf.o: /usr/include/bits/sys_errlist.h compat.h defs.h
act_conf.o: /usr/include/sys/time.h macros.h json.h structs.h types.h flags.h
act_conf.o: tables.h interp.h colour.h groups.h comm.h utils.h lookup.h
act_conf.o: do_sub.h act_conf.h
act_fight.o: /usr/include/stdlib.h /usr/include/features.h
act_fight.o: /usr/include/stdc-predef.h /usr/include/sys/cdefs.h
act_fight.o: /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h
act_fight.o: /usr/include/bits/waitflags.h /usr/include/bits/waitstatus.h
act_fight.o: /usr/include/endian.h /usr/include/bits/endian.h
act_fight.o: /usr/include/bits/byteswap.h /usr/include/bits/types.h
act_fight.o: /usr/include/bits/typesizes.h /usr/include/bits/byteswap-16.h
act_fight.o: /usr/include/sys/types.h /usr/include/time.h
act_fight.o: /usr/include/sys/select.h /usr/include/bits/select.h
act_fight.o: /usr/include/bits/sigset.h /usr/include/bits/time.h
act_fight.o: /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h
act_fight.o: /usr/include/alloca.h /usr/include/bits/stdlib-float.h skills.h
act_fight.o: merc.h /usr/include/stdio.h /usr/include/libio.h
act_fight.o: /usr/include/_G_config.h /usr/include/wchar.h
act_fight.o: /usr/include/bits/wchar.h /usr/include/xlocale.h
act_fight.o: /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h
act_fight.o: compat.h defs.h /usr/include/sys/time.h macros.h json.h
act_fight.o: structs.h types.h flags.h tables.h affects.h utils.h comm.h
act_fight.o: lookup.h interp.h groups.h mob_prog.h recycle.h fight.h
act_fight.o: act_comm.h chars.h find.h act_fight.h
act_group.o: /usr/include/stdlib.h /usr/include/features.h
act_group.o: /usr/include/stdc-predef.h /usr/include/sys/cdefs.h
act_group.o: /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h
act_group.o: /usr/include/bits/waitflags.h /usr/include/bits/waitstatus.h
act_group.o: /usr/include/endian.h /usr/include/bits/endian.h
act_group.o: /usr/include/bits/byteswap.h /usr/include/bits/types.h
act_group.o: /usr/include/bits/typesizes.h /usr/include/bits/byteswap-16.h
act_group.o: /usr/include/sys/types.h /usr/include/time.h
act_group.o: /usr/include/sys/select.h /usr/include/bits/select.h
act_group.o: /usr/include/bits/sigset.h /usr/include/bits/time.h
act_group.o: /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h
act_group.o: /usr/include/alloca.h /usr/include/bits/stdlib-float.h interp.h
act_group.o: merc.h /usr/include/stdio.h /usr/include/libio.h
act_group.o: /usr/include/_G_config.h /usr/include/wchar.h
act_group.o: /usr/include/bits/wchar.h /usr/include/xlocale.h
act_group.o: /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h
act_group.o: compat.h defs.h /usr/include/sys/time.h macros.h json.h
act_group.o: structs.h types.h flags.h tables.h groups.h utils.h comm.h db.h
act_group.o: chars.h find.h act_group.h
act_info.o: /usr/include/string.h /usr/include/features.h
act_info.o: /usr/include/stdc-predef.h /usr/include/sys/cdefs.h
act_info.o: /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h
act_info.o: /usr/include/xlocale.h /usr/include/stdlib.h
act_info.o: /usr/include/bits/waitflags.h /usr/include/bits/waitstatus.h
act_info.o: /usr/include/endian.h /usr/include/bits/endian.h
act_info.o: /usr/include/bits/byteswap.h /usr/include/bits/types.h
act_info.o: /usr/include/bits/typesizes.h /usr/include/bits/byteswap-16.h
act_info.o: /usr/include/sys/types.h /usr/include/time.h
act_info.o: /usr/include/sys/select.h /usr/include/bits/select.h
act_info.o: /usr/include/bits/sigset.h /usr/include/bits/time.h
act_info.o: /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h
act_info.o: /usr/include/alloca.h /usr/include/bits/stdlib-float.h
act_info.o: /usr/include/ctype.h interp.h merc.h /usr/include/stdio.h
act_info.o: /usr/include/libio.h /usr/include/_G_config.h
act_info.o: /usr/include/wchar.h /usr/include/bits/wchar.h
act_info.o: /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h
act_info.o: compat.h defs.h /usr/include/sys/time.h macros.h json.h structs.h
act_info.o: types.h flags.h tables.h magic.h recycle.h lookup.h utils.h
act_info.o: comm.h skills.h groups.h db.h db2.h fight.h update.h save.h
act_info.o: do_sub.h act_comm.h act_obj.h chars.h rooms.h objs.h find.h
act_info.o: spell_info.h act_info.h
act_move.o: /usr/include/string.h /usr/include/features.h
act_move.o: /usr/include/stdc-predef.h /usr/include/sys/cdefs.h
act_move.o: /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h
act_move.o: /usr/include/xlocale.h interp.h merc.h /usr/include/stdio.h
act_move.o: /usr/include/bits/types.h /usr/include/bits/typesizes.h
act_move.o: /usr/include/libio.h /usr/include/_G_config.h
act_move.o: /usr/include/wchar.h /usr/include/bits/wchar.h
act_move.o: /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h
act_move.o: compat.h defs.h /usr/include/sys/types.h /usr/include/time.h
act_move.o: /usr/include/endian.h /usr/include/bits/endian.h
act_move.o: /usr/include/bits/byteswap.h /usr/include/bits/byteswap-16.h
act_move.o: /usr/include/sys/select.h /usr/include/bits/select.h
act_move.o: /usr/include/bits/sigset.h /usr/include/bits/time.h
act_move.o: /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h
act_move.o: /usr/include/sys/time.h macros.h json.h structs.h types.h flags.h
act_move.o: tables.h lookup.h utils.h comm.h mob_prog.h affects.h skills.h
act_move.o: db.h fight.h groups.h act_info.h chars.h rooms.h objs.h find.h
act_move.o: act_move.h
act_obj.o: /usr/include/stdlib.h /usr/include/features.h
act_obj.o: /usr/include/stdc-predef.h /usr/include/sys/cdefs.h
act_obj.o: /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h
act_obj.o: /usr/include/bits/waitflags.h /usr/include/bits/waitstatus.h
act_obj.o: /usr/include/endian.h /usr/include/bits/endian.h
act_obj.o: /usr/include/bits/byteswap.h /usr/include/bits/types.h
act_obj.o: /usr/include/bits/typesizes.h /usr/include/bits/byteswap-16.h
act_obj.o: /usr/include/sys/types.h /usr/include/time.h
act_obj.o: /usr/include/sys/select.h /usr/include/bits/select.h
act_obj.o: /usr/include/bits/sigset.h /usr/include/bits/time.h
act_obj.o: /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h
act_obj.o: /usr/include/alloca.h /usr/include/bits/stdlib-float.h interp.h
act_obj.o: merc.h /usr/include/stdio.h /usr/include/libio.h
act_obj.o: /usr/include/_G_config.h /usr/include/wchar.h
act_obj.o: /usr/include/bits/wchar.h /usr/include/xlocale.h
act_obj.o: /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h
act_obj.o: compat.h defs.h /usr/include/sys/time.h macros.h json.h structs.h
act_obj.o: types.h flags.h tables.h affects.h utils.h comm.h db.h skills.h
act_obj.o: fight.h groups.h mob_prog.h save.h magic.h update.h act_group.h
act_obj.o: act_comm.h act_move.h recycle.h music.h chars.h objs.h rooms.h
act_obj.o: find.h act_obj.h
act_olc.o: /usr/include/string.h /usr/include/features.h
act_olc.o: /usr/include/stdc-predef.h /usr/include/sys/cdefs.h
act_olc.o: /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h
act_olc.o: /usr/include/xlocale.h /usr/include/stdlib.h
act_olc.o: /usr/include/bits/waitflags.h /usr/include/bits/waitstatus.h
act_olc.o: /usr/include/endian.h /usr/include/bits/endian.h
act_olc.o: /usr/include/bits/byteswap.h /usr/include/bits/types.h
act_olc.o: /usr/include/bits/typesizes.h /usr/include/bits/byteswap-16.h
act_olc.o: /usr/include/sys/types.h /usr/include/time.h
act_olc.o: /usr/include/sys/select.h /usr/include/bits/select.h
act_olc.o: /usr/include/bits/sigset.h /usr/include/bits/time.h
act_olc.o: /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h
act_olc.o: /usr/include/alloca.h /usr/include/bits/stdlib-float.h recycle.h
act_olc.o: merc.h /usr/include/stdio.h /usr/include/libio.h
act_olc.o: /usr/include/_G_config.h /usr/include/wchar.h
act_olc.o: /usr/include/bits/wchar.h /usr/include/bits/stdio_lim.h
act_olc.o: /usr/include/bits/sys_errlist.h compat.h defs.h
act_olc.o: /usr/include/sys/time.h macros.h json.h structs.h types.h flags.h
act_olc.o: tables.h lookup.h interp.h utils.h db.h comm.h olc_save.h
act_olc.o: act_info.h chars.h olc_aedit.h olc_hedit.h olc_medit.h
act_olc.o: olc_mpedit.h olc_oedit.h olc_redit.h olc.h act_olc.h
act_player.o: /usr/include/string.h /usr/include/features.h
act_player.o: /usr/include/stdc-predef.h /usr/include/sys/cdefs.h
act_player.o: /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h
act_player.o: /usr/include/xlocale.h /usr/include/ctype.h
act_player.o: /usr/include/bits/types.h /usr/include/bits/typesizes.h
act_player.o: /usr/include/endian.h /usr/include/bits/endian.h
act_player.o: /usr/include/bits/byteswap.h /usr/include/bits/byteswap-16.h
act_player.o: db.h merc.h /usr/include/stdio.h /usr/include/libio.h
act_player.o: /usr/include/_G_config.h /usr/include/wchar.h
act_player.o: /usr/include/bits/wchar.h /usr/include/bits/stdio_lim.h
act_player.o: /usr/include/bits/sys_errlist.h compat.h defs.h
act_player.o: /usr/include/sys/types.h /usr/include/time.h
act_player.o: /usr/include/sys/select.h /usr/include/bits/select.h
act_player.o: /usr/include/bits/sigset.h /usr/include/bits/time.h
act_player.o: /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h
act_player.o: /usr/include/sys/time.h macros.h json.h structs.h types.h
act_player.o: flags.h tables.h comm.h save.h utils.h fight.h interp.h
act_player.o: recycle.h chars.h act_player.h
act_shop.o: magic.h merc.h /usr/include/stdio.h /usr/include/features.h
act_shop.o: /usr/include/stdc-predef.h /usr/include/sys/cdefs.h
act_shop.o: /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h
act_shop.o: /usr/include/bits/types.h /usr/include/bits/typesizes.h
act_shop.o: /usr/include/libio.h /usr/include/_G_config.h
act_shop.o: /usr/include/wchar.h /usr/include/bits/wchar.h
act_shop.o: /usr/include/xlocale.h /usr/include/bits/stdio_lim.h
act_shop.o: /usr/include/bits/sys_errlist.h compat.h defs.h
act_shop.o: /usr/include/sys/types.h /usr/include/time.h
act_shop.o: /usr/include/endian.h /usr/include/bits/endian.h
act_shop.o: /usr/include/bits/byteswap.h /usr/include/bits/byteswap-16.h
act_shop.o: /usr/include/sys/select.h /usr/include/bits/select.h
act_shop.o: /usr/include/bits/sigset.h /usr/include/bits/time.h
act_shop.o: /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h
act_shop.o: /usr/include/sys/time.h macros.h json.h structs.h types.h flags.h
act_shop.o: tables.h lookup.h comm.h interp.h utils.h db.h skills.h groups.h
act_shop.o: chars.h objs.h find.h act_comm.h spell_cure.h act_shop.h
act_skills.o: /usr/include/string.h /usr/include/features.h
act_skills.o: /usr/include/stdc-predef.h /usr/include/sys/cdefs.h
act_skills.o: /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h
act_skills.o: /usr/include/xlocale.h /usr/include/stdlib.h
act_skills.o: /usr/include/bits/waitflags.h /usr/include/bits/waitstatus.h
act_skills.o: /usr/include/endian.h /usr/include/bits/endian.h
act_skills.o: /usr/include/bits/byteswap.h /usr/include/bits/types.h
act_skills.o: /usr/include/bits/typesizes.h /usr/include/bits/byteswap-16.h
act_skills.o: /usr/include/sys/types.h /usr/include/time.h
act_skills.o: /usr/include/sys/select.h /usr/include/bits/select.h
act_skills.o: /usr/include/bits/sigset.h /usr/include/bits/time.h
act_skills.o: /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h
act_skills.o: /usr/include/alloca.h /usr/include/bits/stdlib-float.h db.h
act_skills.o: merc.h /usr/include/stdio.h /usr/include/libio.h
act_skills.o: /usr/include/_G_config.h /usr/include/wchar.h
act_skills.o: /usr/include/bits/wchar.h /usr/include/bits/stdio_lim.h
act_skills.o: /usr/include/bits/sys_errlist.h compat.h defs.h
act_skills.o: /usr/include/sys/time.h macros.h json.h structs.h types.h
act_skills.o: flags.h tables.h utils.h comm.h interp.h magic.h fight.h
act_skills.o: lookup.h skills.h recycle.h act_comm.h chars.h find.h
act_skills.o: act_skills.h
affects.o: /usr/include/string.h /usr/include/features.h
affects.o: /usr/include/stdc-predef.h /usr/include/sys/cdefs.h
affects.o: /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h
affects.o: /usr/include/xlocale.h recycle.h merc.h /usr/include/stdio.h
affects.o: /usr/include/bits/types.h /usr/include/bits/typesizes.h
affects.o: /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h
affects.o: /usr/include/bits/wchar.h /usr/include/bits/stdio_lim.h
affects.o: /usr/include/bits/sys_errlist.h compat.h defs.h
affects.o: /usr/include/sys/types.h /usr/include/time.h /usr/include/endian.h
affects.o: /usr/include/bits/endian.h /usr/include/bits/byteswap.h
affects.o: /usr/include/bits/byteswap-16.h /usr/include/sys/select.h
affects.o: /usr/include/bits/select.h /usr/include/bits/sigset.h
affects.o: /usr/include/bits/time.h /usr/include/sys/sysmacros.h
affects.o: /usr/include/bits/pthreadtypes.h /usr/include/sys/time.h macros.h
affects.o: json.h structs.h types.h flags.h tables.h utils.h comm.h lookup.h
affects.o: chars.h affects.h
ban.o: /usr/include/string.h /usr/include/features.h
ban.o: /usr/include/stdc-predef.h /usr/include/sys/cdefs.h
ban.o: /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h
ban.o: /usr/include/xlocale.h recycle.h merc.h /usr/include/stdio.h
ban.o: /usr/include/bits/types.h /usr/include/bits/typesizes.h
ban.o: /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h
ban.o: /usr/include/bits/wchar.h /usr/include/bits/stdio_lim.h
ban.o: /usr/include/bits/sys_errlist.h compat.h defs.h
ban.o: /usr/include/sys/types.h /usr/include/time.h /usr/include/endian.h
ban.o: /usr/include/bits/endian.h /usr/include/bits/byteswap.h
ban.o: /usr/include/bits/byteswap-16.h /usr/include/sys/select.h
ban.o: /usr/include/bits/select.h /usr/include/bits/sigset.h
ban.o: /usr/include/bits/time.h /usr/include/sys/sysmacros.h
ban.o: /usr/include/bits/pthreadtypes.h /usr/include/sys/time.h macros.h
ban.o: json.h structs.h types.h flags.h tables.h utils.h db.h save.h interp.h
ban.o: comm.h chars.h ban.h
board.o: /usr/include/string.h /usr/include/features.h
board.o: /usr/include/stdc-predef.h /usr/include/sys/cdefs.h
board.o: /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h
board.o: /usr/include/xlocale.h /usr/include/stdlib.h
board.o: /usr/include/bits/waitflags.h /usr/include/bits/waitstatus.h
board.o: /usr/include/endian.h /usr/include/bits/endian.h
board.o: /usr/include/bits/byteswap.h /usr/include/bits/types.h
board.o: /usr/include/bits/typesizes.h /usr/include/bits/byteswap-16.h
board.o: /usr/include/sys/types.h /usr/include/time.h
board.o: /usr/include/sys/select.h /usr/include/bits/select.h
board.o: /usr/include/bits/sigset.h /usr/include/bits/time.h
board.o: /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h
board.o: /usr/include/alloca.h /usr/include/bits/stdlib-float.h
board.o: /usr/include/ctype.h interp.h merc.h /usr/include/stdio.h
board.o: /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h
board.o: /usr/include/bits/wchar.h /usr/include/bits/stdio_lim.h
board.o: /usr/include/bits/sys_errlist.h compat.h defs.h
board.o: /usr/include/sys/time.h macros.h json.h structs.h types.h flags.h
board.o: tables.h db.h utils.h comm.h recycle.h lookup.h chars.h board.h
chars.o: /usr/include/string.h /usr/include/features.h
chars.o: /usr/include/stdc-predef.h /usr/include/sys/cdefs.h
chars.o: /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h
chars.o: /usr/include/xlocale.h utils.h merc.h /usr/include/stdio.h
chars.o: /usr/include/bits/types.h /usr/include/bits/typesizes.h
chars.o: /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h
chars.o: /usr/include/bits/wchar.h /usr/include/bits/stdio_lim.h
chars.o: /usr/include/bits/sys_errlist.h compat.h defs.h
chars.o: /usr/include/sys/types.h /usr/include/time.h /usr/include/endian.h
chars.o: /usr/include/bits/endian.h /usr/include/bits/byteswap.h
chars.o: /usr/include/bits/byteswap-16.h /usr/include/sys/select.h
chars.o: /usr/include/bits/select.h /usr/include/bits/sigset.h
chars.o: /usr/include/bits/time.h /usr/include/sys/sysmacros.h
chars.o: /usr/include/bits/pthreadtypes.h /usr/include/sys/time.h macros.h
chars.o: json.h structs.h types.h flags.h tables.h skills.h comm.h objs.h
chars.o: affects.h lookup.h magic.h db.h groups.h fight.h interp.h recycle.h
chars.o: rooms.h mob_prog.h act_move.h act_info.h act_group.h wiz_l6.h
chars.o: chars.h
colour.o: /usr/include/string.h /usr/include/features.h
colour.o: /usr/include/stdc-predef.h /usr/include/sys/cdefs.h
colour.o: /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h
colour.o: /usr/include/xlocale.h lookup.h merc.h /usr/include/stdio.h
colour.o: /usr/include/bits/types.h /usr/include/bits/typesizes.h
colour.o: /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h
colour.o: /usr/include/bits/wchar.h /usr/include/bits/stdio_lim.h
colour.o: /usr/include/bits/sys_errlist.h compat.h defs.h
colour.o: /usr/include/sys/types.h /usr/include/time.h /usr/include/endian.h
colour.o: /usr/include/bits/endian.h /usr/include/bits/byteswap.h
colour.o: /usr/include/bits/byteswap-16.h /usr/include/sys/select.h
colour.o: /usr/include/bits/select.h /usr/include/bits/sigset.h
colour.o: /usr/include/bits/time.h /usr/include/sys/sysmacros.h
colour.o: /usr/include/bits/pthreadtypes.h /usr/include/sys/time.h macros.h
colour.o: json.h structs.h types.h flags.h tables.h utils.h colour.h
comm.o: /usr/include/ctype.h /usr/include/features.h
comm.o: /usr/include/stdc-predef.h /usr/include/sys/cdefs.h
comm.o: /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h
comm.o: /usr/include/bits/types.h /usr/include/bits/typesizes.h
comm.o: /usr/include/endian.h /usr/include/bits/endian.h
comm.o: /usr/include/bits/byteswap.h /usr/include/bits/byteswap-16.h
comm.o: /usr/include/xlocale.h /usr/include/errno.h /usr/include/bits/errno.h
comm.o: /usr/include/linux/errno.h /usr/include/asm/errno.h
comm.o: /usr/include/asm-generic/errno.h
comm.o: /usr/include/asm-generic/errno-base.h /usr/include/string.h
comm.o: /usr/include/stdlib.h /usr/include/bits/waitflags.h
comm.o: /usr/include/bits/waitstatus.h /usr/include/sys/types.h
comm.o: /usr/include/time.h /usr/include/sys/select.h
comm.o: /usr/include/bits/select.h /usr/include/bits/sigset.h
comm.o: /usr/include/bits/time.h /usr/include/sys/sysmacros.h
comm.o: /usr/include/bits/pthreadtypes.h /usr/include/alloca.h
comm.o: /usr/include/bits/stdlib-float.h /usr/include/unistd.h
comm.o: /usr/include/bits/posix_opt.h /usr/include/bits/confname.h
comm.o: /usr/include/getopt.h colour.h merc.h /usr/include/stdio.h
comm.o: /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h
comm.o: /usr/include/bits/wchar.h /usr/include/bits/stdio_lim.h
comm.o: /usr/include/bits/sys_errlist.h compat.h defs.h
comm.o: /usr/include/sys/time.h macros.h json.h structs.h types.h flags.h
comm.o: tables.h recycle.h utils.h ban.h fight.h update.h interp.h db.h olc.h
comm.o: nanny.h save.h mob_prog.h lookup.h act_info.h chars.h rooms.h objs.h
comm.o: find.h comm.h
db2.o: /usr/include/string.h /usr/include/features.h
db2.o: /usr/include/stdc-predef.h /usr/include/sys/cdefs.h
db2.o: /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h
db2.o: /usr/include/xlocale.h /usr/include/stdlib.h
db2.o: /usr/include/bits/waitflags.h /usr/include/bits/waitstatus.h
db2.o: /usr/include/endian.h /usr/include/bits/endian.h
db2.o: /usr/include/bits/byteswap.h /usr/include/bits/types.h
db2.o: /usr/include/bits/typesizes.h /usr/include/bits/byteswap-16.h
db2.o: /usr/include/sys/types.h /usr/include/time.h /usr/include/sys/select.h
db2.o: /usr/include/bits/select.h /usr/include/bits/sigset.h
db2.o: /usr/include/bits/time.h /usr/include/sys/sysmacros.h
db2.o: /usr/include/bits/pthreadtypes.h /usr/include/alloca.h
db2.o: /usr/include/bits/stdlib-float.h db.h merc.h /usr/include/stdio.h
db2.o: /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h
db2.o: /usr/include/bits/wchar.h /usr/include/bits/stdio_lim.h
db2.o: /usr/include/bits/sys_errlist.h compat.h defs.h
db2.o: /usr/include/sys/time.h macros.h json.h structs.h types.h flags.h
db2.o: tables.h lookup.h utils.h olc.h recycle.h affects.h db2.h
db.o: /usr/include/stdlib.h /usr/include/features.h
db.o: /usr/include/stdc-predef.h /usr/include/sys/cdefs.h
db.o: /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h
db.o: /usr/include/bits/waitflags.h /usr/include/bits/waitstatus.h
db.o: /usr/include/endian.h /usr/include/bits/endian.h
db.o: /usr/include/bits/byteswap.h /usr/include/bits/types.h
db.o: /usr/include/bits/typesizes.h /usr/include/bits/byteswap-16.h
db.o: /usr/include/sys/types.h /usr/include/time.h /usr/include/sys/select.h
db.o: /usr/include/bits/select.h /usr/include/bits/sigset.h
db.o: /usr/include/bits/time.h /usr/include/sys/sysmacros.h
db.o: /usr/include/bits/pthreadtypes.h /usr/include/alloca.h
db.o: /usr/include/bits/stdlib-float.h /usr/include/string.h
db.o: /usr/include/xlocale.h /usr/include/ctype.h utils.h merc.h
db.o: /usr/include/stdio.h /usr/include/libio.h /usr/include/_G_config.h
db.o: /usr/include/wchar.h /usr/include/bits/wchar.h
db.o: /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h compat.h
db.o: defs.h /usr/include/sys/time.h macros.h json.h structs.h types.h
db.o: flags.h tables.h recycle.h affects.h lookup.h json_obj.h music.h ban.h
db.o: board.h comm.h db2.h interp.h magic.h olc.h portals.h chars.h rooms.h
db.o: objs.h db.h
do_sub.o: comm.h merc.h /usr/include/stdio.h /usr/include/features.h
do_sub.o: /usr/include/stdc-predef.h /usr/include/sys/cdefs.h
do_sub.o: /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h
do_sub.o: /usr/include/bits/types.h /usr/include/bits/typesizes.h
do_sub.o: /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h
do_sub.o: /usr/include/bits/wchar.h /usr/include/xlocale.h
do_sub.o: /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h
do_sub.o: compat.h defs.h /usr/include/sys/types.h /usr/include/time.h
do_sub.o: /usr/include/endian.h /usr/include/bits/endian.h
do_sub.o: /usr/include/bits/byteswap.h /usr/include/bits/byteswap-16.h
do_sub.o: /usr/include/sys/select.h /usr/include/bits/select.h
do_sub.o: /usr/include/bits/sigset.h /usr/include/bits/time.h
do_sub.o: /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h
do_sub.o: /usr/include/sys/time.h macros.h json.h structs.h types.h flags.h
do_sub.o: tables.h do_sub.h
effects.o: recycle.h merc.h /usr/include/stdio.h /usr/include/features.h
effects.o: /usr/include/stdc-predef.h /usr/include/sys/cdefs.h
effects.o: /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h
effects.o: /usr/include/bits/types.h /usr/include/bits/typesizes.h
effects.o: /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h
effects.o: /usr/include/bits/wchar.h /usr/include/xlocale.h
effects.o: /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h
effects.o: compat.h defs.h /usr/include/sys/types.h /usr/include/time.h
effects.o: /usr/include/endian.h /usr/include/bits/endian.h
effects.o: /usr/include/bits/byteswap.h /usr/include/bits/byteswap-16.h
effects.o: /usr/include/sys/select.h /usr/include/bits/select.h
effects.o: /usr/include/bits/sigset.h /usr/include/bits/time.h
effects.o: /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h
effects.o: /usr/include/sys/time.h macros.h json.h structs.h types.h flags.h
effects.o: tables.h lookup.h db.h utils.h comm.h skills.h magic.h update.h
effects.o: affects.h objs.h effects.h
fight.o: /usr/include/string.h /usr/include/features.h
fight.o: /usr/include/stdc-predef.h /usr/include/sys/cdefs.h
fight.o: /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h
fight.o: /usr/include/xlocale.h interp.h merc.h /usr/include/stdio.h
fight.o: /usr/include/bits/types.h /usr/include/bits/typesizes.h
fight.o: /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h
fight.o: /usr/include/bits/wchar.h /usr/include/bits/stdio_lim.h
fight.o: /usr/include/bits/sys_errlist.h compat.h defs.h
fight.o: /usr/include/sys/types.h /usr/include/time.h /usr/include/endian.h
fight.o: /usr/include/bits/endian.h /usr/include/bits/byteswap.h
fight.o: /usr/include/bits/byteswap-16.h /usr/include/sys/select.h
fight.o: /usr/include/bits/select.h /usr/include/bits/sigset.h
fight.o: /usr/include/bits/time.h /usr/include/sys/sysmacros.h
fight.o: /usr/include/bits/pthreadtypes.h /usr/include/sys/time.h macros.h
fight.o: json.h structs.h types.h flags.h tables.h lookup.h update.h utils.h
fight.o: db.h effects.h mob_prog.h skills.h comm.h save.h groups.h magic.h
fight.o: affects.h act_fight.h act_obj.h act_comm.h act_move.h chars.h objs.h
fight.o: find.h fight.h
find.o: interp.h merc.h /usr/include/stdio.h /usr/include/features.h
find.o: /usr/include/stdc-predef.h /usr/include/sys/cdefs.h
find.o: /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h
find.o: /usr/include/bits/types.h /usr/include/bits/typesizes.h
find.o: /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h
find.o: /usr/include/bits/wchar.h /usr/include/xlocale.h
find.o: /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h
find.o: compat.h defs.h /usr/include/sys/types.h /usr/include/time.h
find.o: /usr/include/endian.h /usr/include/bits/endian.h
find.o: /usr/include/bits/byteswap.h /usr/include/bits/byteswap-16.h
find.o: /usr/include/sys/select.h /usr/include/bits/select.h
find.o: /usr/include/bits/sigset.h /usr/include/bits/time.h
find.o: /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h
find.o: /usr/include/sys/time.h macros.h json.h structs.h types.h flags.h
find.o: tables.h utils.h chars.h db.h act_comm.h find.h
flags.o: merc.h /usr/include/stdio.h /usr/include/features.h
flags.o: /usr/include/stdc-predef.h /usr/include/sys/cdefs.h
flags.o: /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h
flags.o: /usr/include/bits/types.h /usr/include/bits/typesizes.h
flags.o: /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h
flags.o: /usr/include/bits/wchar.h /usr/include/xlocale.h
flags.o: /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h
flags.o: compat.h defs.h /usr/include/sys/types.h /usr/include/time.h
flags.o: /usr/include/endian.h /usr/include/bits/endian.h
flags.o: /usr/include/bits/byteswap.h /usr/include/bits/byteswap-16.h
flags.o: /usr/include/sys/select.h /usr/include/bits/select.h
flags.o: /usr/include/bits/sigset.h /usr/include/bits/time.h
flags.o: /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h
flags.o: /usr/include/sys/time.h macros.h json.h structs.h types.h flags.h
flags.o: tables.h
groups.o: utils.h merc.h /usr/include/stdio.h /usr/include/features.h
groups.o: /usr/include/stdc-predef.h /usr/include/sys/cdefs.h
groups.o: /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h
groups.o: /usr/include/bits/types.h /usr/include/bits/typesizes.h
groups.o: /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h
groups.o: /usr/include/bits/wchar.h /usr/include/xlocale.h
groups.o: /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h
groups.o: compat.h defs.h /usr/include/sys/types.h /usr/include/time.h
groups.o: /usr/include/endian.h /usr/include/bits/endian.h
groups.o: /usr/include/bits/byteswap.h /usr/include/bits/byteswap-16.h
groups.o: /usr/include/sys/select.h /usr/include/bits/select.h
groups.o: /usr/include/bits/sigset.h /usr/include/bits/time.h
groups.o: /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h
groups.o: /usr/include/sys/time.h macros.h json.h structs.h types.h flags.h
groups.o: tables.h affects.h comm.h skills.h db.h chars.h groups.h
interp.o: /usr/include/string.h /usr/include/features.h
interp.o: /usr/include/stdc-predef.h /usr/include/sys/cdefs.h
interp.o: /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h
interp.o: /usr/include/xlocale.h /usr/include/ctype.h
interp.o: /usr/include/bits/types.h /usr/include/bits/typesizes.h
interp.o: /usr/include/endian.h /usr/include/bits/endian.h
interp.o: /usr/include/bits/byteswap.h /usr/include/bits/byteswap-16.h
interp.o: /usr/include/stdlib.h /usr/include/bits/waitflags.h
interp.o: /usr/include/bits/waitstatus.h /usr/include/sys/types.h
interp.o: /usr/include/time.h /usr/include/sys/select.h
interp.o: /usr/include/bits/select.h /usr/include/bits/sigset.h
interp.o: /usr/include/bits/time.h /usr/include/sys/sysmacros.h
interp.o: /usr/include/bits/pthreadtypes.h /usr/include/alloca.h
interp.o: /usr/include/bits/stdlib-float.h utils.h merc.h
interp.o: /usr/include/stdio.h /usr/include/libio.h /usr/include/_G_config.h
interp.o: /usr/include/wchar.h /usr/include/bits/wchar.h
interp.o: /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h
interp.o: compat.h defs.h /usr/include/sys/time.h macros.h json.h structs.h
interp.o: types.h flags.h tables.h comm.h db.h db2.h recycle.h chars.h find.h
interp.o: act_board.h act_comm.h act_conf.h act_fight.h act_group.h
interp.o: act_info.h act_move.h act_obj.h act_player.h act_shop.h
interp.o: act_skills.h act_olc.h wiz_im.h wiz_l1.h wiz_l2.h wiz_l3.h wiz_l4.h
interp.o: wiz_l5.h wiz_l6.h wiz_l7.h wiz_l8.h wiz_ml.h olc.h mob_cmds.h
interp.o: interp.h
json.o: /usr/include/sys/stat.h /usr/include/features.h
json.o: /usr/include/stdc-predef.h /usr/include/sys/cdefs.h
json.o: /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h
json.o: /usr/include/bits/types.h /usr/include/bits/typesizes.h
json.o: /usr/include/time.h /usr/include/bits/stat.h /usr/include/sys/types.h
json.o: /usr/include/endian.h /usr/include/bits/endian.h
json.o: /usr/include/bits/byteswap.h /usr/include/bits/byteswap-16.h
json.o: /usr/include/sys/select.h /usr/include/bits/select.h
json.o: /usr/include/bits/sigset.h /usr/include/bits/time.h
json.o: /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h
json.o: /usr/include/stdlib.h /usr/include/bits/waitflags.h
json.o: /usr/include/bits/waitstatus.h /usr/include/alloca.h
json.o: /usr/include/bits/stdlib-float.h /usr/include/string.h
json.o: /usr/include/xlocale.h utils.h merc.h /usr/include/stdio.h
json.o: /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h
json.o: /usr/include/bits/wchar.h /usr/include/bits/stdio_lim.h
json.o: /usr/include/bits/sys_errlist.h compat.h defs.h
json.o: /usr/include/sys/time.h macros.h json.h structs.h types.h flags.h
json.o: tables.h
json_obj.o: /usr/include/string.h /usr/include/features.h
json_obj.o: /usr/include/stdc-predef.h /usr/include/sys/cdefs.h
json_obj.o: /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h
json_obj.o: /usr/include/xlocale.h lookup.h merc.h /usr/include/stdio.h
json_obj.o: /usr/include/bits/types.h /usr/include/bits/typesizes.h
json_obj.o: /usr/include/libio.h /usr/include/_G_config.h
json_obj.o: /usr/include/wchar.h /usr/include/bits/wchar.h
json_obj.o: /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h
json_obj.o: compat.h defs.h /usr/include/sys/types.h /usr/include/time.h
json_obj.o: /usr/include/endian.h /usr/include/bits/endian.h
json_obj.o: /usr/include/bits/byteswap.h /usr/include/bits/byteswap-16.h
json_obj.o: /usr/include/sys/select.h /usr/include/bits/select.h
json_obj.o: /usr/include/bits/sigset.h /usr/include/bits/time.h
json_obj.o: /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h
json_obj.o: /usr/include/sys/time.h macros.h json.h structs.h types.h flags.h
json_obj.o: tables.h utils.h json_obj.h
json_tbl.o: json_obj.h merc.h /usr/include/stdio.h /usr/include/features.h
json_tbl.o: /usr/include/stdc-predef.h /usr/include/sys/cdefs.h
json_tbl.o: /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h
json_tbl.o: /usr/include/bits/types.h /usr/include/bits/typesizes.h
json_tbl.o: /usr/include/libio.h /usr/include/_G_config.h
json_tbl.o: /usr/include/wchar.h /usr/include/bits/wchar.h
json_tbl.o: /usr/include/xlocale.h /usr/include/bits/stdio_lim.h
json_tbl.o: /usr/include/bits/sys_errlist.h compat.h defs.h
json_tbl.o: /usr/include/sys/types.h /usr/include/time.h
json_tbl.o: /usr/include/endian.h /usr/include/bits/endian.h
json_tbl.o: /usr/include/bits/byteswap.h /usr/include/bits/byteswap-16.h
json_tbl.o: /usr/include/sys/select.h /usr/include/bits/select.h
json_tbl.o: /usr/include/bits/sigset.h /usr/include/bits/time.h
json_tbl.o: /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h
json_tbl.o: /usr/include/sys/time.h macros.h json.h structs.h types.h flags.h
json_tbl.o: tables.h json_tbl.h
lookup.o: /usr/include/string.h /usr/include/features.h
lookup.o: /usr/include/stdc-predef.h /usr/include/sys/cdefs.h
lookup.o: /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h
lookup.o: /usr/include/xlocale.h utils.h merc.h /usr/include/stdio.h
lookup.o: /usr/include/bits/types.h /usr/include/bits/typesizes.h
lookup.o: /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h
lookup.o: /usr/include/bits/wchar.h /usr/include/bits/stdio_lim.h
lookup.o: /usr/include/bits/sys_errlist.h compat.h defs.h
lookup.o: /usr/include/sys/types.h /usr/include/time.h /usr/include/endian.h
lookup.o: /usr/include/bits/endian.h /usr/include/bits/byteswap.h
lookup.o: /usr/include/bits/byteswap-16.h /usr/include/sys/select.h
lookup.o: /usr/include/bits/select.h /usr/include/bits/sigset.h
lookup.o: /usr/include/bits/time.h /usr/include/sys/sysmacros.h
lookup.o: /usr/include/bits/pthreadtypes.h /usr/include/sys/time.h macros.h
lookup.o: json.h structs.h types.h flags.h tables.h interp.h db.h db2.h
lookup.o: recycle.h lookup.h
magic.o: /usr/include/stdlib.h /usr/include/features.h
magic.o: /usr/include/stdc-predef.h /usr/include/sys/cdefs.h
magic.o: /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h
magic.o: /usr/include/bits/waitflags.h /usr/include/bits/waitstatus.h
magic.o: /usr/include/endian.h /usr/include/bits/endian.h
magic.o: /usr/include/bits/byteswap.h /usr/include/bits/types.h
magic.o: /usr/include/bits/typesizes.h /usr/include/bits/byteswap-16.h
magic.o: /usr/include/sys/types.h /usr/include/time.h
magic.o: /usr/include/sys/select.h /usr/include/bits/select.h
magic.o: /usr/include/bits/sigset.h /usr/include/bits/time.h
magic.o: /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h
magic.o: /usr/include/alloca.h /usr/include/bits/stdlib-float.h
magic.o: /usr/include/string.h /usr/include/xlocale.h lookup.h merc.h
magic.o: /usr/include/stdio.h /usr/include/libio.h /usr/include/_G_config.h
magic.o: /usr/include/wchar.h /usr/include/bits/wchar.h
magic.o: /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h
magic.o: compat.h defs.h /usr/include/sys/time.h macros.h json.h structs.h
magic.o: types.h flags.h tables.h affects.h comm.h skills.h fight.h utils.h
magic.o: magic.h
main.o: /usr/include/stdlib.h /usr/include/features.h
main.o: /usr/include/stdc-predef.h /usr/include/sys/cdefs.h
main.o: /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h
main.o: /usr/include/bits/waitflags.h /usr/include/bits/waitstatus.h
main.o: /usr/include/endian.h /usr/include/bits/endian.h
main.o: /usr/include/bits/byteswap.h /usr/include/bits/types.h
main.o: /usr/include/bits/typesizes.h /usr/include/bits/byteswap-16.h
main.o: /usr/include/sys/types.h /usr/include/time.h
main.o: /usr/include/sys/select.h /usr/include/bits/select.h
main.o: /usr/include/bits/sigset.h /usr/include/bits/time.h
main.o: /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h
main.o: /usr/include/alloca.h /usr/include/bits/stdlib-float.h
main.o: /usr/include/string.h /usr/include/xlocale.h interp.h merc.h
main.o: /usr/include/stdio.h /usr/include/libio.h /usr/include/_G_config.h
main.o: /usr/include/wchar.h /usr/include/bits/wchar.h
main.o: /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h
main.o: compat.h defs.h /usr/include/sys/time.h macros.h json.h structs.h
main.o: types.h flags.h tables.h comm.h db.h utils.h signal.h
mob_cmds.o: /usr/include/stdlib.h /usr/include/features.h
mob_cmds.o: /usr/include/stdc-predef.h /usr/include/sys/cdefs.h
mob_cmds.o: /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h
mob_cmds.o: /usr/include/bits/waitflags.h /usr/include/bits/waitstatus.h
mob_cmds.o: /usr/include/endian.h /usr/include/bits/endian.h
mob_cmds.o: /usr/include/bits/byteswap.h /usr/include/bits/types.h
mob_cmds.o: /usr/include/bits/typesizes.h /usr/include/bits/byteswap-16.h
mob_cmds.o: /usr/include/sys/types.h /usr/include/time.h
mob_cmds.o: /usr/include/sys/select.h /usr/include/bits/select.h
mob_cmds.o: /usr/include/bits/sigset.h /usr/include/bits/time.h
mob_cmds.o: /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h
mob_cmds.o: /usr/include/alloca.h /usr/include/bits/stdlib-float.h lookup.h
mob_cmds.o: merc.h /usr/include/stdio.h /usr/include/libio.h
mob_cmds.o: /usr/include/_G_config.h /usr/include/wchar.h
mob_cmds.o: /usr/include/bits/wchar.h /usr/include/xlocale.h
mob_cmds.o: /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h
mob_cmds.o: compat.h defs.h /usr/include/sys/time.h macros.h json.h structs.h
mob_cmds.o: types.h flags.h tables.h interp.h groups.h utils.h comm.h fight.h
mob_cmds.o: mob_prog.h db.h chars.h objs.h rooms.h find.h mob_cmds.h
mob_prog.o: /usr/include/string.h /usr/include/features.h
mob_prog.o: /usr/include/stdc-predef.h /usr/include/sys/cdefs.h
mob_prog.o: /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h
mob_prog.o: /usr/include/xlocale.h /usr/include/stdlib.h
mob_prog.o: /usr/include/bits/waitflags.h /usr/include/bits/waitstatus.h
mob_prog.o: /usr/include/endian.h /usr/include/bits/endian.h
mob_prog.o: /usr/include/bits/byteswap.h /usr/include/bits/types.h
mob_prog.o: /usr/include/bits/typesizes.h /usr/include/bits/byteswap-16.h
mob_prog.o: /usr/include/sys/types.h /usr/include/time.h
mob_prog.o: /usr/include/sys/select.h /usr/include/bits/select.h
mob_prog.o: /usr/include/bits/sigset.h /usr/include/bits/time.h
mob_prog.o: /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h
mob_prog.o: /usr/include/alloca.h /usr/include/bits/stdlib-float.h
mob_prog.o: /usr/include/ctype.h lookup.h merc.h /usr/include/stdio.h
mob_prog.o: /usr/include/libio.h /usr/include/_G_config.h
mob_prog.o: /usr/include/wchar.h /usr/include/bits/wchar.h
mob_prog.o: /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h
mob_prog.o: compat.h defs.h /usr/include/sys/time.h macros.h json.h structs.h
mob_prog.o: types.h flags.h tables.h utils.h groups.h interp.h mob_cmds.h
mob_prog.o: db.h chars.h objs.h find.h mob_prog.h
music.o: recycle.h merc.h /usr/include/stdio.h /usr/include/features.h
music.o: /usr/include/stdc-predef.h /usr/include/sys/cdefs.h
music.o: /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h
music.o: /usr/include/bits/types.h /usr/include/bits/typesizes.h
music.o: /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h
music.o: /usr/include/bits/wchar.h /usr/include/xlocale.h
music.o: /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h
music.o: compat.h defs.h /usr/include/sys/types.h /usr/include/time.h
music.o: /usr/include/endian.h /usr/include/bits/endian.h
music.o: /usr/include/bits/byteswap.h /usr/include/bits/byteswap-16.h
music.o: /usr/include/sys/select.h /usr/include/bits/select.h
music.o: /usr/include/bits/sigset.h /usr/include/bits/time.h
music.o: /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h
music.o: /usr/include/sys/time.h macros.h json.h structs.h types.h flags.h
music.o: tables.h utils.h comm.h db.h interp.h music.h
nanny.o: /usr/include/ctype.h /usr/include/features.h
nanny.o: /usr/include/stdc-predef.h /usr/include/sys/cdefs.h
nanny.o: /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h
nanny.o: /usr/include/bits/types.h /usr/include/bits/typesizes.h
nanny.o: /usr/include/endian.h /usr/include/bits/endian.h
nanny.o: /usr/include/bits/byteswap.h /usr/include/bits/byteswap-16.h
nanny.o: /usr/include/xlocale.h /usr/include/string.h interp.h merc.h
nanny.o: /usr/include/stdio.h /usr/include/libio.h /usr/include/_G_config.h
nanny.o: /usr/include/wchar.h /usr/include/bits/wchar.h
nanny.o: /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h
nanny.o: compat.h defs.h /usr/include/sys/types.h /usr/include/time.h
nanny.o: /usr/include/sys/select.h /usr/include/bits/select.h
nanny.o: /usr/include/bits/sigset.h /usr/include/bits/time.h
nanny.o: /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h
nanny.o: /usr/include/sys/time.h macros.h json.h structs.h types.h flags.h
nanny.o: tables.h recycle.h lookup.h skills.h utils.h comm.h db.h save.h
nanny.o: ban.h fight.h act_info.h act_skills.h act_board.h act_obj.h chars.h
nanny.o: objs.h nanny.h
objs.o: /usr/include/string.h /usr/include/features.h
objs.o: /usr/include/stdc-predef.h /usr/include/sys/cdefs.h
objs.o: /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h
objs.o: /usr/include/xlocale.h utils.h merc.h /usr/include/stdio.h
objs.o: /usr/include/bits/types.h /usr/include/bits/typesizes.h
objs.o: /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h
objs.o: /usr/include/bits/wchar.h /usr/include/bits/stdio_lim.h
objs.o: /usr/include/bits/sys_errlist.h compat.h defs.h
objs.o: /usr/include/sys/types.h /usr/include/time.h /usr/include/endian.h
objs.o: /usr/include/bits/endian.h /usr/include/bits/byteswap.h
objs.o: /usr/include/bits/byteswap-16.h /usr/include/sys/select.h
objs.o: /usr/include/bits/select.h /usr/include/bits/sigset.h
objs.o: /usr/include/bits/time.h /usr/include/sys/sysmacros.h
objs.o: /usr/include/bits/pthreadtypes.h /usr/include/sys/time.h macros.h
objs.o: json.h structs.h types.h flags.h tables.h chars.h db.h recycle.h
objs.o: comm.h lookup.h objs.h
olc_aedit.o: /usr/include/string.h /usr/include/features.h
olc_aedit.o: /usr/include/stdc-predef.h /usr/include/sys/cdefs.h
olc_aedit.o: /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h
olc_aedit.o: /usr/include/xlocale.h /usr/include/stdlib.h
olc_aedit.o: /usr/include/bits/waitflags.h /usr/include/bits/waitstatus.h
olc_aedit.o: /usr/include/endian.h /usr/include/bits/endian.h
olc_aedit.o: /usr/include/bits/byteswap.h /usr/include/bits/types.h
olc_aedit.o: /usr/include/bits/typesizes.h /usr/include/bits/byteswap-16.h
olc_aedit.o: /usr/include/sys/types.h /usr/include/time.h
olc_aedit.o: /usr/include/sys/select.h /usr/include/bits/select.h
olc_aedit.o: /usr/include/bits/sigset.h /usr/include/bits/time.h
olc_aedit.o: /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h
olc_aedit.o: /usr/include/alloca.h /usr/include/bits/stdlib-float.h
olc_aedit.o: /usr/include/ctype.h comm.h merc.h /usr/include/stdio.h
olc_aedit.o: /usr/include/libio.h /usr/include/_G_config.h
olc_aedit.o: /usr/include/wchar.h /usr/include/bits/wchar.h
olc_aedit.o: /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h
olc_aedit.o: compat.h defs.h /usr/include/sys/time.h macros.h json.h
olc_aedit.o: structs.h types.h flags.h tables.h lookup.h db.h recycle.h
olc_aedit.o: utils.h interp.h olc.h olc_aedit.h
olc.o: /usr/include/string.h /usr/include/features.h
olc.o: /usr/include/stdc-predef.h /usr/include/sys/cdefs.h
olc.o: /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h
olc.o: /usr/include/xlocale.h utils.h merc.h /usr/include/stdio.h
olc.o: /usr/include/bits/types.h /usr/include/bits/typesizes.h
olc.o: /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h
olc.o: /usr/include/bits/wchar.h /usr/include/bits/stdio_lim.h
olc.o: /usr/include/bits/sys_errlist.h compat.h defs.h
olc.o: /usr/include/sys/types.h /usr/include/time.h /usr/include/endian.h
olc.o: /usr/include/bits/endian.h /usr/include/bits/byteswap.h
olc.o: /usr/include/bits/byteswap-16.h /usr/include/sys/select.h
olc.o: /usr/include/bits/select.h /usr/include/bits/sigset.h
olc.o: /usr/include/bits/time.h /usr/include/sys/sysmacros.h
olc.o: /usr/include/bits/pthreadtypes.h /usr/include/sys/time.h macros.h
olc.o: json.h structs.h types.h flags.h tables.h interp.h comm.h db.h
olc.o: recycle.h lookup.h magic.h act_olc.h olc_aedit.h olc_hedit.h
olc.o: olc_medit.h olc_mpedit.h olc_oedit.h olc_redit.h olc.h
olc_hedit.o: /usr/include/string.h /usr/include/features.h
olc_hedit.o: /usr/include/stdc-predef.h /usr/include/sys/cdefs.h
olc_hedit.o: /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h
olc_hedit.o: /usr/include/xlocale.h /usr/include/stdlib.h
olc_hedit.o: /usr/include/bits/waitflags.h /usr/include/bits/waitstatus.h
olc_hedit.o: /usr/include/endian.h /usr/include/bits/endian.h
olc_hedit.o: /usr/include/bits/byteswap.h /usr/include/bits/types.h
olc_hedit.o: /usr/include/bits/typesizes.h /usr/include/bits/byteswap-16.h
olc_hedit.o: /usr/include/sys/types.h /usr/include/time.h
olc_hedit.o: /usr/include/sys/select.h /usr/include/bits/select.h
olc_hedit.o: /usr/include/bits/sigset.h /usr/include/bits/time.h
olc_hedit.o: /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h
olc_hedit.o: /usr/include/alloca.h /usr/include/bits/stdlib-float.h comm.h
olc_hedit.o: merc.h /usr/include/stdio.h /usr/include/libio.h
olc_hedit.o: /usr/include/_G_config.h /usr/include/wchar.h
olc_hedit.o: /usr/include/bits/wchar.h /usr/include/bits/stdio_lim.h
olc_hedit.o: /usr/include/bits/sys_errlist.h compat.h defs.h
olc_hedit.o: /usr/include/sys/time.h macros.h json.h structs.h types.h
olc_hedit.o: flags.h tables.h lookup.h db.h recycle.h utils.h interp.h olc.h
olc_hedit.o: olc_hedit.h
olc_medit.o: /usr/include/string.h /usr/include/features.h
olc_medit.o: /usr/include/stdc-predef.h /usr/include/sys/cdefs.h
olc_medit.o: /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h
olc_medit.o: /usr/include/xlocale.h /usr/include/stdlib.h
olc_medit.o: /usr/include/bits/waitflags.h /usr/include/bits/waitstatus.h
olc_medit.o: /usr/include/endian.h /usr/include/bits/endian.h
olc_medit.o: /usr/include/bits/byteswap.h /usr/include/bits/types.h
olc_medit.o: /usr/include/bits/typesizes.h /usr/include/bits/byteswap-16.h
olc_medit.o: /usr/include/sys/types.h /usr/include/time.h
olc_medit.o: /usr/include/sys/select.h /usr/include/bits/select.h
olc_medit.o: /usr/include/bits/sigset.h /usr/include/bits/time.h
olc_medit.o: /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h
olc_medit.o: /usr/include/alloca.h /usr/include/bits/stdlib-float.h
olc_medit.o: /usr/include/ctype.h comm.h merc.h /usr/include/stdio.h
olc_medit.o: /usr/include/libio.h /usr/include/_G_config.h
olc_medit.o: /usr/include/wchar.h /usr/include/bits/wchar.h
olc_medit.o: /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h
olc_medit.o: compat.h defs.h /usr/include/sys/time.h macros.h json.h
olc_medit.o: structs.h types.h flags.h tables.h lookup.h db.h recycle.h
olc_medit.o: utils.h interp.h olc.h mob_cmds.h olc_medit.h
olc_mpedit.o: /usr/include/string.h /usr/include/features.h
olc_mpedit.o: /usr/include/stdc-predef.h /usr/include/sys/cdefs.h
olc_mpedit.o: /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h
olc_mpedit.o: /usr/include/xlocale.h /usr/include/stdlib.h
olc_mpedit.o: /usr/include/bits/waitflags.h /usr/include/bits/waitstatus.h
olc_mpedit.o: /usr/include/endian.h /usr/include/bits/endian.h
olc_mpedit.o: /usr/include/bits/byteswap.h /usr/include/bits/types.h
olc_mpedit.o: /usr/include/bits/typesizes.h /usr/include/bits/byteswap-16.h
olc_mpedit.o: /usr/include/sys/types.h /usr/include/time.h
olc_mpedit.o: /usr/include/sys/select.h /usr/include/bits/select.h
olc_mpedit.o: /usr/include/bits/sigset.h /usr/include/bits/time.h
olc_mpedit.o: /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h
olc_mpedit.o: /usr/include/alloca.h /usr/include/bits/stdlib-float.h comm.h
olc_mpedit.o: merc.h /usr/include/stdio.h /usr/include/libio.h
olc_mpedit.o: /usr/include/_G_config.h /usr/include/wchar.h
olc_mpedit.o: /usr/include/bits/wchar.h /usr/include/bits/stdio_lim.h
olc_mpedit.o: /usr/include/bits/sys_errlist.h compat.h defs.h
olc_mpedit.o: /usr/include/sys/time.h macros.h json.h structs.h types.h
olc_mpedit.o: flags.h tables.h db.h recycle.h utils.h lookup.h olc.h
olc_mpedit.o: olc_mpedit.h
olc_oedit.o: /usr/include/string.h /usr/include/features.h
olc_oedit.o: /usr/include/stdc-predef.h /usr/include/sys/cdefs.h
olc_oedit.o: /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h
olc_oedit.o: /usr/include/xlocale.h /usr/include/stdlib.h
olc_oedit.o: /usr/include/bits/waitflags.h /usr/include/bits/waitstatus.h
olc_oedit.o: /usr/include/endian.h /usr/include/bits/endian.h
olc_oedit.o: /usr/include/bits/byteswap.h /usr/include/bits/types.h
olc_oedit.o: /usr/include/bits/typesizes.h /usr/include/bits/byteswap-16.h
olc_oedit.o: /usr/include/sys/types.h /usr/include/time.h
olc_oedit.o: /usr/include/sys/select.h /usr/include/bits/select.h
olc_oedit.o: /usr/include/bits/sigset.h /usr/include/bits/time.h
olc_oedit.o: /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h
olc_oedit.o: /usr/include/alloca.h /usr/include/bits/stdlib-float.h comm.h
olc_oedit.o: merc.h /usr/include/stdio.h /usr/include/libio.h
olc_oedit.o: /usr/include/_G_config.h /usr/include/wchar.h
olc_oedit.o: /usr/include/bits/wchar.h /usr/include/bits/stdio_lim.h
olc_oedit.o: /usr/include/bits/sys_errlist.h compat.h defs.h
olc_oedit.o: /usr/include/sys/time.h macros.h json.h structs.h types.h
olc_oedit.o: flags.h tables.h lookup.h db.h recycle.h utils.h interp.h
olc_oedit.o: affects.h act_info.h olc.h olc_oedit.h
olc_redit.o: /usr/include/string.h /usr/include/features.h
olc_redit.o: /usr/include/stdc-predef.h /usr/include/sys/cdefs.h
olc_redit.o: /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h
olc_redit.o: /usr/include/xlocale.h /usr/include/stdlib.h
olc_redit.o: /usr/include/bits/waitflags.h /usr/include/bits/waitstatus.h
olc_redit.o: /usr/include/endian.h /usr/include/bits/endian.h
olc_redit.o: /usr/include/bits/byteswap.h /usr/include/bits/types.h
olc_redit.o: /usr/include/bits/typesizes.h /usr/include/bits/byteswap-16.h
olc_redit.o: /usr/include/sys/types.h /usr/include/time.h
olc_redit.o: /usr/include/sys/select.h /usr/include/bits/select.h
olc_redit.o: /usr/include/bits/sigset.h /usr/include/bits/time.h
olc_redit.o: /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h
olc_redit.o: /usr/include/alloca.h /usr/include/bits/stdlib-float.h comm.h
olc_redit.o: merc.h /usr/include/stdio.h /usr/include/libio.h
olc_redit.o: /usr/include/_G_config.h /usr/include/wchar.h
olc_redit.o: /usr/include/bits/wchar.h /usr/include/bits/stdio_lim.h
olc_redit.o: /usr/include/bits/sys_errlist.h compat.h defs.h
olc_redit.o: /usr/include/sys/time.h macros.h json.h structs.h types.h
olc_redit.o: flags.h tables.h lookup.h db.h recycle.h utils.h interp.h
olc_redit.o: act_info.h olc.h olc_medit.h olc_oedit.h chars.h objs.h find.h
olc_redit.o: olc_redit.h
olc_save.o: /usr/include/string.h /usr/include/features.h
olc_save.o: /usr/include/stdc-predef.h /usr/include/sys/cdefs.h
olc_save.o: /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h
olc_save.o: /usr/include/xlocale.h /usr/include/stdlib.h
olc_save.o: /usr/include/bits/waitflags.h /usr/include/bits/waitstatus.h
olc_save.o: /usr/include/endian.h /usr/include/bits/endian.h
olc_save.o: /usr/include/bits/byteswap.h /usr/include/bits/types.h
olc_save.o: /usr/include/bits/typesizes.h /usr/include/bits/byteswap-16.h
olc_save.o: /usr/include/sys/types.h /usr/include/time.h
olc_save.o: /usr/include/sys/select.h /usr/include/bits/select.h
olc_save.o: /usr/include/bits/sigset.h /usr/include/bits/time.h
olc_save.o: /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h
olc_save.o: /usr/include/alloca.h /usr/include/bits/stdlib-float.h merc.h
olc_save.o: /usr/include/stdio.h /usr/include/libio.h
olc_save.o: /usr/include/_G_config.h /usr/include/wchar.h
olc_save.o: /usr/include/bits/wchar.h /usr/include/bits/stdio_lim.h
olc_save.o: /usr/include/bits/sys_errlist.h compat.h defs.h
olc_save.o: /usr/include/sys/time.h macros.h json.h structs.h types.h flags.h
olc_save.o: tables.h lookup.h utils.h db.h mob_cmds.h comm.h interp.h olc.h
olc_save.o: olc_save.h
portals.o: lookup.h merc.h /usr/include/stdio.h /usr/include/features.h
portals.o: /usr/include/stdc-predef.h /usr/include/sys/cdefs.h
portals.o: /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h
portals.o: /usr/include/bits/types.h /usr/include/bits/typesizes.h
portals.o: /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h
portals.o: /usr/include/bits/wchar.h /usr/include/xlocale.h
portals.o: /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h
portals.o: compat.h defs.h /usr/include/sys/types.h /usr/include/time.h
portals.o: /usr/include/endian.h /usr/include/bits/endian.h
portals.o: /usr/include/bits/byteswap.h /usr/include/bits/byteswap-16.h
portals.o: /usr/include/sys/select.h /usr/include/bits/select.h
portals.o: /usr/include/bits/sigset.h /usr/include/bits/time.h
portals.o: /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h
portals.o: /usr/include/sys/time.h macros.h json.h structs.h types.h flags.h
portals.o: tables.h recycle.h utils.h portals.h
recycle.o: /usr/include/string.h /usr/include/features.h
recycle.o: /usr/include/stdc-predef.h /usr/include/sys/cdefs.h
recycle.o: /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h
recycle.o: /usr/include/xlocale.h /usr/include/stdlib.h
recycle.o: /usr/include/bits/waitflags.h /usr/include/bits/waitstatus.h
recycle.o: /usr/include/endian.h /usr/include/bits/endian.h
recycle.o: /usr/include/bits/byteswap.h /usr/include/bits/types.h
recycle.o: /usr/include/bits/typesizes.h /usr/include/bits/byteswap-16.h
recycle.o: /usr/include/sys/types.h /usr/include/time.h
recycle.o: /usr/include/sys/select.h /usr/include/bits/select.h
recycle.o: /usr/include/bits/sigset.h /usr/include/bits/time.h
recycle.o: /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h
recycle.o: /usr/include/alloca.h /usr/include/bits/stdlib-float.h utils.h
recycle.o: merc.h /usr/include/stdio.h /usr/include/libio.h
recycle.o: /usr/include/_G_config.h /usr/include/wchar.h
recycle.o: /usr/include/bits/wchar.h /usr/include/bits/stdio_lim.h
recycle.o: /usr/include/bits/sys_errlist.h compat.h defs.h
recycle.o: /usr/include/sys/time.h macros.h json.h structs.h types.h flags.h
recycle.o: tables.h lookup.h db.h comm.h affects.h objs.h recycle.h
rooms.o: db.h merc.h /usr/include/stdio.h /usr/include/features.h
rooms.o: /usr/include/stdc-predef.h /usr/include/sys/cdefs.h
rooms.o: /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h
rooms.o: /usr/include/bits/types.h /usr/include/bits/typesizes.h
rooms.o: /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h
rooms.o: /usr/include/bits/wchar.h /usr/include/xlocale.h
rooms.o: /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h
rooms.o: compat.h defs.h /usr/include/sys/types.h /usr/include/time.h
rooms.o: /usr/include/endian.h /usr/include/bits/endian.h
rooms.o: /usr/include/bits/byteswap.h /usr/include/bits/byteswap-16.h
rooms.o: /usr/include/sys/select.h /usr/include/bits/select.h
rooms.o: /usr/include/bits/sigset.h /usr/include/bits/time.h
rooms.o: /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h
rooms.o: /usr/include/sys/time.h macros.h json.h structs.h types.h flags.h
rooms.o: tables.h lookup.h utils.h objs.h interp.h rooms.h
save.o: recycle.h merc.h /usr/include/stdio.h /usr/include/features.h
save.o: /usr/include/stdc-predef.h /usr/include/sys/cdefs.h
save.o: /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h
save.o: /usr/include/bits/types.h /usr/include/bits/typesizes.h
save.o: /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h
save.o: /usr/include/bits/wchar.h /usr/include/xlocale.h
save.o: /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h
save.o: compat.h defs.h /usr/include/sys/types.h /usr/include/time.h
save.o: /usr/include/endian.h /usr/include/bits/endian.h
save.o: /usr/include/bits/byteswap.h /usr/include/bits/byteswap-16.h
save.o: /usr/include/sys/select.h /usr/include/bits/select.h
save.o: /usr/include/bits/sigset.h /usr/include/bits/time.h
save.o: /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h
save.o: /usr/include/sys/time.h macros.h json.h structs.h types.h flags.h
save.o: tables.h lookup.h colour.h db.h utils.h skills.h board.h comm.h
save.o: chars.h objs.h save.h
sha256.o: /usr/include/stdio.h /usr/include/features.h
sha256.o: /usr/include/stdc-predef.h /usr/include/sys/cdefs.h
sha256.o: /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h
sha256.o: /usr/include/bits/types.h /usr/include/bits/typesizes.h
sha256.o: /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h
sha256.o: /usr/include/bits/wchar.h /usr/include/xlocale.h
sha256.o: /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h
sha256.o: /usr/include/string.h /usr/include/sys/param.h
sha256.o: /usr/include/sys/types.h /usr/include/time.h /usr/include/endian.h
sha256.o: /usr/include/bits/endian.h /usr/include/bits/byteswap.h
sha256.o: /usr/include/bits/byteswap-16.h /usr/include/sys/select.h
sha256.o: /usr/include/bits/select.h /usr/include/bits/sigset.h
sha256.o: /usr/include/bits/time.h /usr/include/sys/sysmacros.h
sha256.o: /usr/include/bits/pthreadtypes.h /usr/include/limits.h
sha256.o: /usr/include/bits/posix1_lim.h /usr/include/bits/local_lim.h
sha256.o: /usr/include/linux/limits.h /usr/include/bits/posix2_lim.h signal.h
sha256.o: merc.h compat.h defs.h /usr/include/sys/time.h macros.h json.h
sha256.o: structs.h types.h flags.h tables.h /usr/include/bits/param.h
sha256.o: /usr/include/linux/param.h /usr/include/asm/param.h
sha256.o: /usr/include/asm-generic/param.h sha256.h
signal.o: signal.h merc.h /usr/include/stdio.h /usr/include/features.h
signal.o: /usr/include/stdc-predef.h /usr/include/sys/cdefs.h
signal.o: /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h
signal.o: /usr/include/bits/types.h /usr/include/bits/typesizes.h
signal.o: /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h
signal.o: /usr/include/bits/wchar.h /usr/include/xlocale.h
signal.o: /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h
signal.o: compat.h defs.h /usr/include/sys/types.h /usr/include/time.h
signal.o: /usr/include/endian.h /usr/include/bits/endian.h
signal.o: /usr/include/bits/byteswap.h /usr/include/bits/byteswap-16.h
signal.o: /usr/include/sys/select.h /usr/include/bits/select.h
signal.o: /usr/include/bits/sigset.h /usr/include/bits/time.h
signal.o: /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h
signal.o: /usr/include/sys/time.h macros.h json.h structs.h types.h flags.h
signal.o: tables.h /usr/include/stdlib.h /usr/include/bits/waitflags.h
signal.o: /usr/include/bits/waitstatus.h /usr/include/alloca.h
signal.o: /usr/include/bits/stdlib-float.h
skills.o: /usr/include/string.h /usr/include/features.h
skills.o: /usr/include/stdc-predef.h /usr/include/sys/cdefs.h
skills.o: /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h
skills.o: /usr/include/xlocale.h /usr/include/stdlib.h
skills.o: /usr/include/bits/waitflags.h /usr/include/bits/waitstatus.h
skills.o: /usr/include/endian.h /usr/include/bits/endian.h
skills.o: /usr/include/bits/byteswap.h /usr/include/bits/types.h
skills.o: /usr/include/bits/typesizes.h /usr/include/bits/byteswap-16.h
skills.o: /usr/include/sys/types.h /usr/include/time.h
skills.o: /usr/include/sys/select.h /usr/include/bits/select.h
skills.o: /usr/include/bits/sigset.h /usr/include/bits/time.h
skills.o: /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h
skills.o: /usr/include/alloca.h /usr/include/bits/stdlib-float.h interp.h
skills.o: merc.h /usr/include/stdio.h /usr/include/libio.h
skills.o: /usr/include/_G_config.h /usr/include/wchar.h
skills.o: /usr/include/bits/wchar.h /usr/include/bits/stdio_lim.h
skills.o: /usr/include/bits/sys_errlist.h compat.h defs.h
skills.o: /usr/include/sys/time.h macros.h json.h structs.h types.h flags.h
skills.o: tables.h magic.h recycle.h lookup.h affects.h comm.h utils.h
skills.o: fight.h act_info.h act_skills.h chars.h skills.h
special.o: utils.h merc.h /usr/include/stdio.h /usr/include/features.h
special.o: /usr/include/stdc-predef.h /usr/include/sys/cdefs.h
special.o: /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h
special.o: /usr/include/bits/types.h /usr/include/bits/typesizes.h
special.o: /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h
special.o: /usr/include/bits/wchar.h /usr/include/xlocale.h
special.o: /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h
special.o: compat.h defs.h /usr/include/sys/types.h /usr/include/time.h
special.o: /usr/include/endian.h /usr/include/bits/endian.h
special.o: /usr/include/bits/byteswap.h /usr/include/bits/byteswap-16.h
special.o: /usr/include/sys/select.h /usr/include/bits/select.h
special.o: /usr/include/bits/sigset.h /usr/include/bits/time.h
special.o: /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h
special.o: /usr/include/sys/time.h macros.h json.h structs.h types.h flags.h
special.o: tables.h fight.h comm.h interp.h magic.h lookup.h skills.h db.h
special.o: act_fight.h act_comm.h act_move.h chars.h objs.h spell_aff.h
special.o: special.h
spell_aff.o: magic.h merc.h /usr/include/stdio.h /usr/include/features.h
spell_aff.o: /usr/include/stdc-predef.h /usr/include/sys/cdefs.h
spell_aff.o: /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h
spell_aff.o: /usr/include/bits/types.h /usr/include/bits/typesizes.h
spell_aff.o: /usr/include/libio.h /usr/include/_G_config.h
spell_aff.o: /usr/include/wchar.h /usr/include/bits/wchar.h
spell_aff.o: /usr/include/xlocale.h /usr/include/bits/stdio_lim.h
spell_aff.o: /usr/include/bits/sys_errlist.h compat.h defs.h
spell_aff.o: /usr/include/sys/types.h /usr/include/time.h
spell_aff.o: /usr/include/endian.h /usr/include/bits/endian.h
spell_aff.o: /usr/include/bits/byteswap.h /usr/include/bits/byteswap-16.h
spell_aff.o: /usr/include/sys/select.h /usr/include/bits/select.h
spell_aff.o: /usr/include/bits/sigset.h /usr/include/bits/time.h
spell_aff.o: /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h
spell_aff.o: /usr/include/sys/time.h macros.h json.h structs.h types.h
spell_aff.o: flags.h tables.h utils.h affects.h comm.h chars.h fight.h
spell_aff.o: groups.h skills.h objs.h recycle.h lookup.h spell_aff.h
spell_create.o: magic.h merc.h /usr/include/stdio.h /usr/include/features.h
spell_create.o: /usr/include/stdc-predef.h /usr/include/sys/cdefs.h
spell_create.o: /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h
spell_create.o: /usr/include/bits/types.h /usr/include/bits/typesizes.h
spell_create.o: /usr/include/libio.h /usr/include/_G_config.h
spell_create.o: /usr/include/wchar.h /usr/include/bits/wchar.h
spell_create.o: /usr/include/xlocale.h /usr/include/bits/stdio_lim.h
spell_create.o: /usr/include/bits/sys_errlist.h compat.h defs.h
spell_create.o: /usr/include/sys/types.h /usr/include/time.h
spell_create.o: /usr/include/endian.h /usr/include/bits/endian.h
spell_create.o: /usr/include/bits/byteswap.h /usr/include/bits/byteswap-16.h
spell_create.o: /usr/include/sys/select.h /usr/include/bits/select.h
spell_create.o: /usr/include/bits/sigset.h /usr/include/bits/time.h
spell_create.o: /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h
spell_create.o: /usr/include/sys/time.h macros.h json.h structs.h types.h
spell_create.o: flags.h tables.h find.h comm.h db.h objs.h utils.h chars.h
spell_create.o: spell_create.h
spell_cure.o: magic.h merc.h /usr/include/stdio.h /usr/include/features.h
spell_cure.o: /usr/include/stdc-predef.h /usr/include/sys/cdefs.h
spell_cure.o: /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h
spell_cure.o: /usr/include/bits/types.h /usr/include/bits/typesizes.h
spell_cure.o: /usr/include/libio.h /usr/include/_G_config.h
spell_cure.o: /usr/include/wchar.h /usr/include/bits/wchar.h
spell_cure.o: /usr/include/xlocale.h /usr/include/bits/stdio_lim.h
spell_cure.o: /usr/include/bits/sys_errlist.h compat.h defs.h
spell_cure.o: /usr/include/sys/types.h /usr/include/time.h
spell_cure.o: /usr/include/endian.h /usr/include/bits/endian.h
spell_cure.o: /usr/include/bits/byteswap.h /usr/include/bits/byteswap-16.h
spell_cure.o: /usr/include/sys/select.h /usr/include/bits/select.h
spell_cure.o: /usr/include/bits/sigset.h /usr/include/bits/time.h
spell_cure.o: /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h
spell_cure.o: /usr/include/sys/time.h macros.h json.h structs.h types.h
spell_cure.o: flags.h tables.h skills.h comm.h utils.h lookup.h fight.h
spell_cure.o: spell_cure.h
spell_info.o: /usr/include/stdlib.h /usr/include/features.h
spell_info.o: /usr/include/stdc-predef.h /usr/include/sys/cdefs.h
spell_info.o: /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h
spell_info.o: /usr/include/bits/waitflags.h /usr/include/bits/waitstatus.h
spell_info.o: /usr/include/endian.h /usr/include/bits/endian.h
spell_info.o: /usr/include/bits/byteswap.h /usr/include/bits/types.h
spell_info.o: /usr/include/bits/typesizes.h /usr/include/bits/byteswap-16.h
spell_info.o: /usr/include/sys/types.h /usr/include/time.h
spell_info.o: /usr/include/sys/select.h /usr/include/bits/select.h
spell_info.o: /usr/include/bits/sigset.h /usr/include/bits/time.h
spell_info.o: /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h
spell_info.o: /usr/include/alloca.h /usr/include/bits/stdlib-float.h magic.h
spell_info.o: merc.h /usr/include/stdio.h /usr/include/libio.h
spell_info.o: /usr/include/_G_config.h /usr/include/wchar.h
spell_info.o: /usr/include/bits/wchar.h /usr/include/xlocale.h
spell_info.o: /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h
spell_info.o: compat.h defs.h /usr/include/sys/time.h macros.h json.h
spell_info.o: structs.h types.h flags.h tables.h comm.h utils.h lookup.h
spell_info.o: chars.h recycle.h db.h interp.h act_info.h spell_info.h
spell_misc.o: magic.h merc.h /usr/include/stdio.h /usr/include/features.h
spell_misc.o: /usr/include/stdc-predef.h /usr/include/sys/cdefs.h
spell_misc.o: /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h
spell_misc.o: /usr/include/bits/types.h /usr/include/bits/typesizes.h
spell_misc.o: /usr/include/libio.h /usr/include/_G_config.h
spell_misc.o: /usr/include/wchar.h /usr/include/bits/wchar.h
spell_misc.o: /usr/include/xlocale.h /usr/include/bits/stdio_lim.h
spell_misc.o: /usr/include/bits/sys_errlist.h compat.h defs.h
spell_misc.o: /usr/include/sys/types.h /usr/include/time.h
spell_misc.o: /usr/include/endian.h /usr/include/bits/endian.h
spell_misc.o: /usr/include/bits/byteswap.h /usr/include/bits/byteswap-16.h
spell_misc.o: /usr/include/sys/select.h /usr/include/bits/select.h
spell_misc.o: /usr/include/bits/sigset.h /usr/include/bits/time.h
spell_misc.o: /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h
spell_misc.o: /usr/include/sys/time.h macros.h json.h structs.h types.h
spell_misc.o: flags.h tables.h comm.h utils.h lookup.h db.h affects.h objs.h
spell_misc.o: interp.h spell_misc.h
spell_move.o: magic.h merc.h /usr/include/stdio.h /usr/include/features.h
spell_move.o: /usr/include/stdc-predef.h /usr/include/sys/cdefs.h
spell_move.o: /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h
spell_move.o: /usr/include/bits/types.h /usr/include/bits/typesizes.h
spell_move.o: /usr/include/libio.h /usr/include/_G_config.h
spell_move.o: /usr/include/wchar.h /usr/include/bits/wchar.h
spell_move.o: /usr/include/xlocale.h /usr/include/bits/stdio_lim.h
spell_move.o: /usr/include/bits/sys_errlist.h compat.h defs.h
spell_move.o: /usr/include/sys/types.h /usr/include/time.h
spell_move.o: /usr/include/endian.h /usr/include/bits/endian.h
spell_move.o: /usr/include/bits/byteswap.h /usr/include/bits/byteswap-16.h
spell_move.o: /usr/include/sys/select.h /usr/include/bits/select.h
spell_move.o: /usr/include/bits/sigset.h /usr/include/bits/time.h
spell_move.o: /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h
spell_move.o: /usr/include/sys/time.h macros.h json.h structs.h types.h
spell_move.o: flags.h tables.h find.h chars.h comm.h interp.h act_info.h db.h
spell_move.o: fight.h objs.h spell_move.h
spell_npc.o: magic.h merc.h /usr/include/stdio.h /usr/include/features.h
spell_npc.o: /usr/include/stdc-predef.h /usr/include/sys/cdefs.h
spell_npc.o: /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h
spell_npc.o: /usr/include/bits/types.h /usr/include/bits/typesizes.h
spell_npc.o: /usr/include/libio.h /usr/include/_G_config.h
spell_npc.o: /usr/include/wchar.h /usr/include/bits/wchar.h
spell_npc.o: /usr/include/xlocale.h /usr/include/bits/stdio_lim.h
spell_npc.o: /usr/include/bits/sys_errlist.h compat.h defs.h
spell_npc.o: /usr/include/sys/types.h /usr/include/time.h
spell_npc.o: /usr/include/endian.h /usr/include/bits/endian.h
spell_npc.o: /usr/include/bits/byteswap.h /usr/include/bits/byteswap-16.h
spell_npc.o: /usr/include/sys/select.h /usr/include/bits/select.h
spell_npc.o: /usr/include/bits/sigset.h /usr/include/bits/time.h
spell_npc.o: /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h
spell_npc.o: /usr/include/sys/time.h macros.h json.h structs.h types.h
spell_npc.o: flags.h tables.h utils.h lookup.h effects.h fight.h comm.h
spell_npc.o: spell_npc.h
spell_off.o: magic.h merc.h /usr/include/stdio.h /usr/include/features.h
spell_off.o: /usr/include/stdc-predef.h /usr/include/sys/cdefs.h
spell_off.o: /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h
spell_off.o: /usr/include/bits/types.h /usr/include/bits/typesizes.h
spell_off.o: /usr/include/libio.h /usr/include/_G_config.h
spell_off.o: /usr/include/wchar.h /usr/include/bits/wchar.h
spell_off.o: /usr/include/xlocale.h /usr/include/bits/stdio_lim.h
spell_off.o: /usr/include/bits/sys_errlist.h compat.h defs.h
spell_off.o: /usr/include/sys/types.h /usr/include/time.h
spell_off.o: /usr/include/endian.h /usr/include/bits/endian.h
spell_off.o: /usr/include/bits/byteswap.h /usr/include/bits/byteswap-16.h
spell_off.o: /usr/include/sys/select.h /usr/include/bits/select.h
spell_off.o: /usr/include/bits/sigset.h /usr/include/bits/time.h
spell_off.o: /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h
spell_off.o: /usr/include/sys/time.h macros.h json.h structs.h types.h
spell_off.o: flags.h tables.h utils.h fight.h comm.h affects.h lookup.h db.h
spell_off.o: spell_aff.h skills.h chars.h objs.h spell_off.h
string.o: /usr/include/string.h /usr/include/features.h
string.o: /usr/include/stdc-predef.h /usr/include/sys/cdefs.h
string.o: /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h
string.o: /usr/include/xlocale.h /usr/include/stdlib.h
string.o: /usr/include/bits/waitflags.h /usr/include/bits/waitstatus.h
string.o: /usr/include/endian.h /usr/include/bits/endian.h
string.o: /usr/include/bits/byteswap.h /usr/include/bits/types.h
string.o: /usr/include/bits/typesizes.h /usr/include/bits/byteswap-16.h
string.o: /usr/include/sys/types.h /usr/include/time.h
string.o: /usr/include/sys/select.h /usr/include/bits/select.h
string.o: /usr/include/bits/sigset.h /usr/include/bits/time.h
string.o: /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h
string.o: /usr/include/alloca.h /usr/include/bits/stdlib-float.h comm.h
string.o: merc.h /usr/include/stdio.h /usr/include/libio.h
string.o: /usr/include/_G_config.h /usr/include/wchar.h
string.o: /usr/include/bits/wchar.h /usr/include/bits/stdio_lim.h
string.o: /usr/include/bits/sys_errlist.h compat.h defs.h
string.o: /usr/include/sys/time.h macros.h json.h structs.h types.h flags.h
string.o: tables.h utils.h interp.h olc.h db.h olc_mpedit.h
tables.o: skills.h merc.h /usr/include/stdio.h /usr/include/features.h
tables.o: /usr/include/stdc-predef.h /usr/include/sys/cdefs.h
tables.o: /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h
tables.o: /usr/include/bits/types.h /usr/include/bits/typesizes.h
tables.o: /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h
tables.o: /usr/include/bits/wchar.h /usr/include/xlocale.h
tables.o: /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h
tables.o: compat.h defs.h /usr/include/sys/types.h /usr/include/time.h
tables.o: /usr/include/endian.h /usr/include/bits/endian.h
tables.o: /usr/include/bits/byteswap.h /usr/include/bits/byteswap-16.h
tables.o: /usr/include/sys/select.h /usr/include/bits/select.h
tables.o: /usr/include/bits/sigset.h /usr/include/bits/time.h
tables.o: /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h
tables.o: /usr/include/sys/time.h macros.h json.h structs.h types.h flags.h
tables.o: tables.h nanny.h lookup.h recycle.h colour.h board.h special.h
tables.o: effects.h json_tbl.h magic.h spell_aff.h spell_create.h
tables.o: spell_cure.h spell_info.h spell_misc.h spell_move.h spell_npc.h
tables.o: spell_off.h
types.o: merc.h /usr/include/stdio.h /usr/include/features.h
types.o: /usr/include/stdc-predef.h /usr/include/sys/cdefs.h
types.o: /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h
types.o: /usr/include/bits/types.h /usr/include/bits/typesizes.h
types.o: /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h
types.o: /usr/include/bits/wchar.h /usr/include/xlocale.h
types.o: /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h
types.o: compat.h defs.h /usr/include/sys/types.h /usr/include/time.h
types.o: /usr/include/endian.h /usr/include/bits/endian.h
types.o: /usr/include/bits/byteswap.h /usr/include/bits/byteswap-16.h
types.o: /usr/include/sys/select.h /usr/include/bits/select.h
types.o: /usr/include/bits/sigset.h /usr/include/bits/time.h
types.o: /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h
types.o: /usr/include/sys/time.h macros.h json.h structs.h types.h flags.h
types.o: tables.h
update.o: /usr/include/string.h /usr/include/features.h
update.o: /usr/include/stdc-predef.h /usr/include/sys/cdefs.h
update.o: /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h
update.o: /usr/include/xlocale.h interp.h merc.h /usr/include/stdio.h
update.o: /usr/include/bits/types.h /usr/include/bits/typesizes.h
update.o: /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h
update.o: /usr/include/bits/wchar.h /usr/include/bits/stdio_lim.h
update.o: /usr/include/bits/sys_errlist.h compat.h defs.h
update.o: /usr/include/sys/types.h /usr/include/time.h /usr/include/endian.h
update.o: /usr/include/bits/endian.h /usr/include/bits/byteswap.h
update.o: /usr/include/bits/byteswap-16.h /usr/include/sys/select.h
update.o: /usr/include/bits/select.h /usr/include/bits/sigset.h
update.o: /usr/include/bits/time.h /usr/include/sys/sysmacros.h
update.o: /usr/include/bits/pthreadtypes.h /usr/include/sys/time.h macros.h
update.o: json.h structs.h types.h flags.h tables.h music.h comm.h affects.h
update.o: utils.h skills.h mob_prog.h fight.h save.h magic.h db.h
update.o: act_player.h chars.h objs.h update.h
utils.o: /usr/include/stdlib.h /usr/include/features.h
utils.o: /usr/include/stdc-predef.h /usr/include/sys/cdefs.h
utils.o: /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h
utils.o: /usr/include/bits/waitflags.h /usr/include/bits/waitstatus.h
utils.o: /usr/include/endian.h /usr/include/bits/endian.h
utils.o: /usr/include/bits/byteswap.h /usr/include/bits/types.h
utils.o: /usr/include/bits/typesizes.h /usr/include/bits/byteswap-16.h
utils.o: /usr/include/sys/types.h /usr/include/time.h
utils.o: /usr/include/sys/select.h /usr/include/bits/select.h
utils.o: /usr/include/bits/sigset.h /usr/include/bits/time.h
utils.o: /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h
utils.o: /usr/include/alloca.h /usr/include/bits/stdlib-float.h
utils.o: /usr/include/unistd.h /usr/include/bits/posix_opt.h
utils.o: /usr/include/bits/confname.h /usr/include/getopt.h
utils.o: /usr/include/string.h /usr/include/xlocale.h comm.h merc.h
utils.o: /usr/include/stdio.h /usr/include/libio.h /usr/include/_G_config.h
utils.o: /usr/include/wchar.h /usr/include/bits/wchar.h
utils.o: /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h
utils.o: compat.h defs.h /usr/include/sys/time.h macros.h json.h structs.h
utils.o: types.h flags.h tables.h db.h interp.h utils.h
wiz_im.o: /usr/include/string.h /usr/include/features.h
wiz_im.o: /usr/include/stdc-predef.h /usr/include/sys/cdefs.h
wiz_im.o: /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h
wiz_im.o: /usr/include/xlocale.h /usr/include/stdlib.h
wiz_im.o: /usr/include/bits/waitflags.h /usr/include/bits/waitstatus.h
wiz_im.o: /usr/include/endian.h /usr/include/bits/endian.h
wiz_im.o: /usr/include/bits/byteswap.h /usr/include/bits/types.h
wiz_im.o: /usr/include/bits/typesizes.h /usr/include/bits/byteswap-16.h
wiz_im.o: /usr/include/sys/types.h /usr/include/time.h
wiz_im.o: /usr/include/sys/select.h /usr/include/bits/select.h
wiz_im.o: /usr/include/bits/sigset.h /usr/include/bits/time.h
wiz_im.o: /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h
wiz_im.o: /usr/include/alloca.h /usr/include/bits/stdlib-float.h interp.h
wiz_im.o: merc.h /usr/include/stdio.h /usr/include/libio.h
wiz_im.o: /usr/include/_G_config.h /usr/include/wchar.h
wiz_im.o: /usr/include/bits/wchar.h /usr/include/bits/stdio_lim.h
wiz_im.o: /usr/include/bits/sys_errlist.h compat.h defs.h
wiz_im.o: /usr/include/sys/time.h macros.h json.h structs.h types.h flags.h
wiz_im.o: tables.h db.h utils.h comm.h lookup.h recycle.h do_sub.h mob_cmds.h
wiz_im.o: act_info.h chars.h objs.h rooms.h find.h wiz_im.h
wiz_l1.o: /usr/include/string.h /usr/include/features.h
wiz_l1.o: /usr/include/stdc-predef.h /usr/include/sys/cdefs.h
wiz_l1.o: /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h
wiz_l1.o: /usr/include/xlocale.h db.h merc.h /usr/include/stdio.h
wiz_l1.o: /usr/include/bits/types.h /usr/include/bits/typesizes.h
wiz_l1.o: /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h
wiz_l1.o: /usr/include/bits/wchar.h /usr/include/bits/stdio_lim.h
wiz_l1.o: /usr/include/bits/sys_errlist.h compat.h defs.h
wiz_l1.o: /usr/include/sys/types.h /usr/include/time.h /usr/include/endian.h
wiz_l1.o: /usr/include/bits/endian.h /usr/include/bits/byteswap.h
wiz_l1.o: /usr/include/bits/byteswap-16.h /usr/include/sys/select.h
wiz_l1.o: /usr/include/bits/select.h /usr/include/bits/sigset.h
wiz_l1.o: /usr/include/bits/time.h /usr/include/sys/sysmacros.h
wiz_l1.o: /usr/include/bits/pthreadtypes.h /usr/include/sys/time.h macros.h
wiz_l1.o: json.h structs.h types.h flags.h tables.h comm.h interp.h save.h
wiz_l1.o: fight.h ban.h utils.h act_player.h wiz_l4.h chars.h find.h wiz_l1.h
wiz_l2.o: /usr/include/string.h /usr/include/features.h
wiz_l2.o: /usr/include/stdc-predef.h /usr/include/sys/cdefs.h
wiz_l2.o: /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h
wiz_l2.o: /usr/include/xlocale.h /usr/include/stdlib.h
wiz_l2.o: /usr/include/bits/waitflags.h /usr/include/bits/waitstatus.h
wiz_l2.o: /usr/include/endian.h /usr/include/bits/endian.h
wiz_l2.o: /usr/include/bits/byteswap.h /usr/include/bits/types.h
wiz_l2.o: /usr/include/bits/typesizes.h /usr/include/bits/byteswap-16.h
wiz_l2.o: /usr/include/sys/types.h /usr/include/time.h
wiz_l2.o: /usr/include/sys/select.h /usr/include/bits/select.h
wiz_l2.o: /usr/include/bits/sigset.h /usr/include/bits/time.h
wiz_l2.o: /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h
wiz_l2.o: /usr/include/alloca.h /usr/include/bits/stdlib-float.h db.h merc.h
wiz_l2.o: /usr/include/stdio.h /usr/include/libio.h /usr/include/_G_config.h
wiz_l2.o: /usr/include/wchar.h /usr/include/bits/wchar.h
wiz_l2.o: /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h
wiz_l2.o: compat.h defs.h /usr/include/sys/time.h macros.h json.h structs.h
wiz_l2.o: types.h flags.h tables.h comm.h interp.h ban.h utils.h recycle.h
wiz_l2.o: lookup.h chars.h rooms.h find.h wiz_l2.h
wiz_l3.o: /usr/include/stdlib.h /usr/include/features.h
wiz_l3.o: /usr/include/stdc-predef.h /usr/include/sys/cdefs.h
wiz_l3.o: /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h
wiz_l3.o: /usr/include/bits/waitflags.h /usr/include/bits/waitstatus.h
wiz_l3.o: /usr/include/endian.h /usr/include/bits/endian.h
wiz_l3.o: /usr/include/bits/byteswap.h /usr/include/bits/types.h
wiz_l3.o: /usr/include/bits/typesizes.h /usr/include/bits/byteswap-16.h
wiz_l3.o: /usr/include/sys/types.h /usr/include/time.h
wiz_l3.o: /usr/include/sys/select.h /usr/include/bits/select.h
wiz_l3.o: /usr/include/bits/sigset.h /usr/include/bits/time.h
wiz_l3.o: /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h
wiz_l3.o: /usr/include/alloca.h /usr/include/bits/stdlib-float.h db.h merc.h
wiz_l3.o: /usr/include/stdio.h /usr/include/libio.h /usr/include/_G_config.h
wiz_l3.o: /usr/include/wchar.h /usr/include/bits/wchar.h
wiz_l3.o: /usr/include/xlocale.h /usr/include/bits/stdio_lim.h
wiz_l3.o: /usr/include/bits/sys_errlist.h compat.h defs.h
wiz_l3.o: /usr/include/sys/time.h macros.h json.h structs.h types.h flags.h
wiz_l3.o: tables.h comm.h interp.h fight.h utils.h chars.h find.h wiz_l3.h
wiz_l4.o: /usr/include/string.h /usr/include/features.h
wiz_l4.o: /usr/include/stdc-predef.h /usr/include/sys/cdefs.h
wiz_l4.o: /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h
wiz_l4.o: /usr/include/xlocale.h /usr/include/stdlib.h
wiz_l4.o: /usr/include/bits/waitflags.h /usr/include/bits/waitstatus.h
wiz_l4.o: /usr/include/endian.h /usr/include/bits/endian.h
wiz_l4.o: /usr/include/bits/byteswap.h /usr/include/bits/types.h
wiz_l4.o: /usr/include/bits/typesizes.h /usr/include/bits/byteswap-16.h
wiz_l4.o: /usr/include/sys/types.h /usr/include/time.h
wiz_l4.o: /usr/include/sys/select.h /usr/include/bits/select.h
wiz_l4.o: /usr/include/bits/sigset.h /usr/include/bits/time.h
wiz_l4.o: /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h
wiz_l4.o: /usr/include/alloca.h /usr/include/bits/stdlib-float.h db.h merc.h
wiz_l4.o: /usr/include/stdio.h /usr/include/libio.h /usr/include/_G_config.h
wiz_l4.o: /usr/include/wchar.h /usr/include/bits/wchar.h
wiz_l4.o: /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h
wiz_l4.o: compat.h defs.h /usr/include/sys/time.h macros.h json.h structs.h
wiz_l4.o: types.h flags.h tables.h comm.h interp.h save.h fight.h utils.h
wiz_l4.o: lookup.h skills.h affects.h chars.h objs.h find.h wiz_l4.h
wiz_l5.o: /usr/include/string.h /usr/include/features.h
wiz_l5.o: /usr/include/stdc-predef.h /usr/include/sys/cdefs.h
wiz_l5.o: /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h
wiz_l5.o: /usr/include/xlocale.h /usr/include/stdlib.h
wiz_l5.o: /usr/include/bits/waitflags.h /usr/include/bits/waitstatus.h
wiz_l5.o: /usr/include/endian.h /usr/include/bits/endian.h
wiz_l5.o: /usr/include/bits/byteswap.h /usr/include/bits/types.h
wiz_l5.o: /usr/include/bits/typesizes.h /usr/include/bits/byteswap-16.h
wiz_l5.o: /usr/include/sys/types.h /usr/include/time.h
wiz_l5.o: /usr/include/sys/select.h /usr/include/bits/select.h
wiz_l5.o: /usr/include/bits/sigset.h /usr/include/bits/time.h
wiz_l5.o: /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h
wiz_l5.o: /usr/include/alloca.h /usr/include/bits/stdlib-float.h db.h merc.h
wiz_l5.o: /usr/include/stdio.h /usr/include/libio.h /usr/include/_G_config.h
wiz_l5.o: /usr/include/wchar.h /usr/include/bits/wchar.h
wiz_l5.o: /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h
wiz_l5.o: compat.h defs.h /usr/include/sys/time.h macros.h json.h structs.h
wiz_l5.o: types.h flags.h tables.h comm.h interp.h save.h fight.h utils.h
wiz_l5.o: lookup.h skills.h affects.h do_sub.h recycle.h act_info.h chars.h
wiz_l5.o: objs.h rooms.h find.h wiz_l5.h
wiz_l6.o: db.h merc.h /usr/include/stdio.h /usr/include/features.h
wiz_l6.o: /usr/include/stdc-predef.h /usr/include/sys/cdefs.h
wiz_l6.o: /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h
wiz_l6.o: /usr/include/bits/types.h /usr/include/bits/typesizes.h
wiz_l6.o: /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h
wiz_l6.o: /usr/include/bits/wchar.h /usr/include/xlocale.h
wiz_l6.o: /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h
wiz_l6.o: compat.h defs.h /usr/include/sys/types.h /usr/include/time.h
wiz_l6.o: /usr/include/endian.h /usr/include/bits/endian.h
wiz_l6.o: /usr/include/bits/byteswap.h /usr/include/bits/byteswap-16.h
wiz_l6.o: /usr/include/sys/select.h /usr/include/bits/select.h
wiz_l6.o: /usr/include/bits/sigset.h /usr/include/bits/time.h
wiz_l6.o: /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h
wiz_l6.o: /usr/include/sys/time.h macros.h json.h structs.h types.h flags.h
wiz_l6.o: tables.h comm.h interp.h utils.h chars.h rooms.h find.h wiz_l6.h
wiz_l7.o: db.h merc.h /usr/include/stdio.h /usr/include/features.h
wiz_l7.o: /usr/include/stdc-predef.h /usr/include/sys/cdefs.h
wiz_l7.o: /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h
wiz_l7.o: /usr/include/bits/types.h /usr/include/bits/typesizes.h
wiz_l7.o: /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h
wiz_l7.o: /usr/include/bits/wchar.h /usr/include/xlocale.h
wiz_l7.o: /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h
wiz_l7.o: compat.h defs.h /usr/include/sys/types.h /usr/include/time.h
wiz_l7.o: /usr/include/endian.h /usr/include/bits/endian.h
wiz_l7.o: /usr/include/bits/byteswap.h /usr/include/bits/byteswap-16.h
wiz_l7.o: /usr/include/sys/select.h /usr/include/bits/select.h
wiz_l7.o: /usr/include/bits/sigset.h /usr/include/bits/time.h
wiz_l7.o: /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h
wiz_l7.o: /usr/include/sys/time.h macros.h json.h structs.h types.h flags.h
wiz_l7.o: tables.h comm.h interp.h utils.h chars.h rooms.h find.h wiz_l7.h
wiz_l8.o: /usr/include/string.h /usr/include/features.h
wiz_l8.o: /usr/include/stdc-predef.h /usr/include/sys/cdefs.h
wiz_l8.o: /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h
wiz_l8.o: /usr/include/xlocale.h db.h merc.h /usr/include/stdio.h
wiz_l8.o: /usr/include/bits/types.h /usr/include/bits/typesizes.h
wiz_l8.o: /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h
wiz_l8.o: /usr/include/bits/wchar.h /usr/include/bits/stdio_lim.h
wiz_l8.o: /usr/include/bits/sys_errlist.h compat.h defs.h
wiz_l8.o: /usr/include/sys/types.h /usr/include/time.h /usr/include/endian.h
wiz_l8.o: /usr/include/bits/endian.h /usr/include/bits/byteswap.h
wiz_l8.o: /usr/include/bits/byteswap-16.h /usr/include/sys/select.h
wiz_l8.o: /usr/include/bits/select.h /usr/include/bits/sigset.h
wiz_l8.o: /usr/include/bits/time.h /usr/include/sys/sysmacros.h
wiz_l8.o: /usr/include/bits/pthreadtypes.h /usr/include/sys/time.h macros.h
wiz_l8.o: json.h structs.h types.h flags.h tables.h comm.h interp.h fight.h
wiz_l8.o: utils.h act_info.h chars.h rooms.h wiz_l8.h
wiz_ml.o: /usr/include/string.h /usr/include/features.h
wiz_ml.o: /usr/include/stdc-predef.h /usr/include/sys/cdefs.h
wiz_ml.o: /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h
wiz_ml.o: /usr/include/xlocale.h /usr/include/stdlib.h
wiz_ml.o: /usr/include/bits/waitflags.h /usr/include/bits/waitstatus.h
wiz_ml.o: /usr/include/endian.h /usr/include/bits/endian.h
wiz_ml.o: /usr/include/bits/byteswap.h /usr/include/bits/types.h
wiz_ml.o: /usr/include/bits/typesizes.h /usr/include/bits/byteswap-16.h
wiz_ml.o: /usr/include/sys/types.h /usr/include/time.h
wiz_ml.o: /usr/include/sys/select.h /usr/include/bits/select.h
wiz_ml.o: /usr/include/bits/sigset.h /usr/include/bits/time.h
wiz_ml.o: /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h
wiz_ml.o: /usr/include/alloca.h /usr/include/bits/stdlib-float.h
wiz_ml.o: /usr/include/unistd.h /usr/include/bits/posix_opt.h
wiz_ml.o: /usr/include/bits/confname.h /usr/include/getopt.h db.h merc.h
wiz_ml.o: /usr/include/stdio.h /usr/include/libio.h /usr/include/_G_config.h
wiz_ml.o: /usr/include/wchar.h /usr/include/bits/wchar.h
wiz_ml.o: /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h
wiz_ml.o: compat.h defs.h /usr/include/sys/time.h macros.h json.h structs.h
wiz_ml.o: types.h flags.h tables.h comm.h interp.h save.h fight.h utils.h
wiz_ml.o: act_info.h chars.h rooms.h find.h wiz_ml.h
