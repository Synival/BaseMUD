# Work-in-progress JSON world docs

## Anums

For the time being, all loadables use something called "anums" (area numbers) instead of "vnums". These are numbers that are _relative to the loadable's area_ rather than _absolute to the entire world_.

**For example**:

The "Midgaard" area reserves vnums 3000 - 3399. "The Temple Of Mota" has a vnum of 3001. This is 1 higher than the minimum vnum (3000), so the anum is 1.

All references to other loadables (like room/mob/obj references in resets) are represented either as:

1. an integer for a loadable in the same area (ex: `to: 1`), or 
2. an object in the following form for a loadable in a different area: `{ "area": "shire", "anum": 3 }`

In a future update, I'd like to allow vnums as well. The reason for the "anums" are so areas can be imported and re-arranged without conflict.

## Portals

Exits between areas no longer need to reference room vnums directly. Instead, exits and rooms can be assigned a "portal" name, and one-way or two-way exits can be defined in `json/config/portals.json`. The portal format is pretty straight-forward:

```
{"portal": {
  "two-way": true,
  "from": "midgaard-up-1",
  "to": "air-down-1"
}},
```

Here is the "from" exit in `json/areas/midgaard/rooms.json`:

```
{
  "dir": "up",
  "to": null,
  "description": "More of the same.\n",
  "portal": "midgaard-up-1"
},
```

...and the "to" exit in `json/areas/air/rooms.json`:

```
{
  "dir": "down",
  "to": null,
  "description": "More of the same\n",
  "portal": "air-down-1"
} 
```
