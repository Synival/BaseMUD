/***************************************************************************
 *  File: olc_oedit.h                                                      *
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
 *                                                                         *
 *  This code was freely distributed with the The Isles 1.1 source code,   *
 *  and has been used here for OLC - OLC would not be what it is without   *
 *  all the previous coders who released their source code.                *
 *                                                                         *
 ***************************************************************************/

#ifndef __ROM_OLC_OEDIT_H
#define __ROM_OLC_OEDIT_H

#include "merc.h"

#define OEDIT(fun)        bool fun(CHAR_T *ch, char *argument)
#define EDIT_OBJ(ch, obj) (obj = (OBJ_INDEX_T *) ch->desc->olc_edit)

#define ALT_FLAGVALUE_SET(_blargh, _table, _arg) { \
        int blah = flag_value( _table, _arg );     \
        _blargh = (blah == NO_FLAG) ? 0 : blah;    \
    }

#define ALT_FLAGVALUE_TOGGLE(_blargh, _table, _arg) { \
        int blah = flag_value( _table, _arg );        \
        _blargh ^= (blah == NO_FLAG) ? 0 : blah;      \
    }

/* Sub-routines and filters. */
void oedit_show_obj_values (CHAR_T *ch, OBJ_INDEX_T *obj);
bool oedit_set_obj_values (CHAR_T *ch, OBJ_INDEX_T *obj,
    int value_num, char *argument);
bool oedit_set_value (CHAR_T *ch, OBJ_INDEX_T *obj, char *argument,
    int value);
bool oedit_values (CHAR_T *ch, char *argument, int value);

/* Commands (objects). */
OEDIT (oedit_show);
OEDIT (oedit_addaffect);
OEDIT (oedit_addapply);
OEDIT (oedit_delaffect);
OEDIT (oedit_name);
OEDIT (oedit_short);
OEDIT (oedit_long);
OEDIT (oedit_value0);
OEDIT (oedit_value1);
OEDIT (oedit_value2);
OEDIT (oedit_value3);
OEDIT (oedit_value4);
OEDIT (oedit_weight);
OEDIT (oedit_cost);
OEDIT (oedit_create);
OEDIT (oedit_ed);
OEDIT (oedit_extra);
OEDIT (oedit_wear);
OEDIT (oedit_type);
OEDIT (oedit_material);
OEDIT (oedit_level);
OEDIT (oedit_condition);

#endif
