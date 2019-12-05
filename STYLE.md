# Code Style Guide

## Horizontal Space

- As a ground rule, keep lines to 80 characters.
- If a line exceeds this rule by a few characters but cannot be broken cleanly
    without dramatically reducing readability, it can exceed the 80 character
    limit.
- If code is best represented as tabular data - large tables in `tables.c`,
    for example - the 80 character limit can be exceeded as much as necessary.
    (The spell table is 200+ characters long, and this is fine.)

## Braces

- Use two-line statements whenever possible, without braces:

```
    if (condition)
        statement;
```

- If an opening brace occurs after a statement that can be written on one line -
    which is the usual case - place the brace at the end of the line:

```
void function(void) {           // brace here
    if (two_line_condition) {   // brace here
        one();
        two();
    }
}    
```

- If an opening brace occurs after a statement that must be split into two or
    more lines, place the brace on its own line for better readability with
    regards to indenting.

```
void function_with_really_long_name(int arg1, int arg2, int arg3, int arg4,
    int arg5, int arg6)
{
    if (arg1 + arg2 + arg3 + arg4 + arg5 + arg6 >=
        yet_another_function_with_a_long_name ())
    {
        one();
        two();
    }
}    
```

- In the caess where braces is optional, if the preceeding statement must be
    split into two or more lines, the brace is now _required_ for readability:

```
if (arg1 + arg2 + arg3 + arg4 + arg5 + arg6 >=
    yet_another_function_with_a_long_name ())
{
    single_statement();
}
```

- If braces are required for what would otherwise be a one-line statement
    (a single-line function definition, for example), both braces can be placed
    on the line of the statement:

```
int math_add_numbers (int a, int b)
    { return a + b; }
```

## "Do" Sub-Routines and Filters

Several functions in the `do_*()` family of functions have shared code that can
be used between multiple commands. Whenever possible, implement void _action_
functions as "sub" functions such as:

```
do_sub_look_room()
do_sub_look_object()
(etc)
```

Several functions also have common code for predicates or early-exit
strategies. If there are accompanying messages to players, these should be
implemented as "filter" functions, and return TRUE if the condition for
filtering has been met (i.e, we _don't_ want to continue):

```
do_filter_can_put_item()
do_filter_can_attack_spell()
```

## Macros for Common Early-Exit Patterns

The codebase is riddled with common early-exit patterns that return from a
function with an accompanying message to a player or the log file. To reduce
code and better convey the role of these early-exits, all of the common
patterns have been reduced to one- or two-line macros that can be grouped
together. They should be used whenever possible, especially in the `do_*()`
family of functions.

These familes of macros exist:

Left side (return method):
`RETURN_IF*`: If true, return with a return value.
`BAIL_IF*  `: If true, return without no return value.
`EXIT_IF*  `: If true, kill the server with an error code.
`FILTER*   `: If true, return TRUE.

Right side (message or extra statement):
`(blank)   `: Send a message to a character via `send_to_char()`.
`_ACT      `: Send a message to a character via `act_new()`.
`_BUG      `: Write a message via `bug()`.
`_BUGF     `: Write a message via `bugf()`.
`_EXPR     `: Execute an expression.

Examples:
```
    BAIL_IF (char_is_npc (ch),
        "Nice try, NPC.\n\r", ch);
    BAIL_IF_ACT (char_is_npc (vict),
        "You can't, $E's an NPC.", ch, NULL, vict);
    BAIL_IF_EXPR (char_is_npc (vict),
        warn_npc (vict));
    RETURN_IF (hp == 0,
        "Sorry, you're dead.\n\r", FALSE);
    EXIT_IF_BUGF (x != 0,
        "error: 'x' should be 0, instead its %d.\n", x);
    FILTER (!CAN_WEAR_FLAG (obj, ITEM_WIELD),
        "You can't wield that.\n\r", ch);
```

## Naming
(WIP)

## Comments
(WIP)

## Organization
(WIP)
