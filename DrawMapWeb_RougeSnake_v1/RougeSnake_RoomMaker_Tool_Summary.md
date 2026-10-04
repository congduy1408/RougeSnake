# Rouge Snake Room Maker — Tool Summary

## Purpose

Rouge Snake Room Maker is a browser-based grid map editor adapted from the earlier Cypherrush drawing tool.

Its purpose is to create room-template JSON files for the Rouge Snake game. The same JSON file is used both:

1. by the editor when reopening and modifying a room, and
2. by the C++/raylib game when loading a room template.

There is no separate project-file/export-file workflow.

---

## Current Project Files

- `index.html` — editor UI.
- `styles.css` — editor styling.
- `app.js` — map state, drawing, layers, brushes, room markers, validation, save/open, undo/redo.
- `*.meta` — old Unity metadata; not required by the browser tool and can be removed if desired.

The tool is currently a simple standalone HTML/CSS/JavaScript application with no build step.

---

## Core Design Decisions

### 1. Grid-based rooms

Every room has a fixed grid width and height.

Map objects are painted onto grid coordinates `(x, y)`.

The initial map and every new map start with a one-cell-wide `wall` boundary on the base layer. These are ordinary editable cells, so entrance and exit markers can replace boundary walls.

### 2. Layers are editor/data layers, not Board layers

The editor supports multiple layers.

Layers exist so multiple things can occupy the same grid coordinate without destroying each other.

Example:

- lower layer: `gap`
- higher layer: `enemy`

At runtime this means there is still a GAP terrain cell at that position, while a flying enemy may also occupy that position.

The C++ `Board` itself is not intended to contain multiple layers.

### 3. Brush name defines meaning

There is no separate `cellType` property.

The generic brush/tile name is the identifier interpreted by the game.

Reserved/current names include:

- `ground`
- `wall`
- `gap`
- `entrance`
- `door`
- `enemy`
- `item`
- `decoration`

Future generic names may include things such as `food`, `npc`, `trap`, `chest`, etc.

Names should remain standardized and lowercase.

The game decides what each recognized name means.

For example:

- `ground`, `wall`, `gap`, `entrance`, `door` configure Board terrain.
- `enemy` is a generic enemy spawn marker.
- `item` is a generic item spawn marker.
- `decoration` is visual/non-board content.

A room does not normally specify a concrete enemy/item variant. The game chooses the actual content later from its own spawning/loot rules.

### 4. Terrain conflict rule

If terrain brushes occur at the same `(x, y)` on multiple layers, the higher layer index wins.

Example:

- Layer 0: `ground` at `(5,5)`
- Layer 1: `wall` at `(5,5)`
- Layer 2: `gap` at `(5,5)`

Final Board terrain at `(5,5)` is `gap`.

This overwrite rule applies to competing terrain values. Non-terrain content can coexist with terrain.

Example:

- Layer 0: `gap`
- Layer 1: `enemy`

Result: Board cell is GAP and an enemy spawn exists at the same position.

### 5. Sparse JSON storage

Layers should store only painted cells.

Do not serialize a complete 2D array containing large numbers of null values.

A painted entry is conceptually:

```json
{
  "name": "ground",
  "x": 5,
  "y": 3
}
```

### 6. One JSON for editing and gameplay

Clicking Save produces the JSON room file.

That file must preserve enough editor information to reopen it in Room Maker, while the C++ game simply ignores editor-only properties.

---

## Room Terrain Rules

Runtime Board terrain currently consists of:

### `ground`

Normal playable terrain.

- walkable
- non-lethal

### `wall`

Solid terrain.

- not walkable
- non-lethal

### `gap`

Lethal empty space.

- conceptually enterable by the snake
- lethal when the snake enters it

Flying enemies may later be allowed to occupy/traverse GAP cells.

### `entrance`

Represents the doorway through which the player entered the current room.

It is a closed entrance after entering the room.

- behaves physically like WALL
- not walkable
- uses a visually different sprite from an ordinary wall
- exactly one is required for a normal room

Player spawn does NOT need to be stored in the room file. The game derives snake spawn position and initial direction from the entrance position.

### `door`

An active exit.

- one grid cell wide
- walkable
- entering it causes a room transition

Exit direction is derived from its position on the map boundary.

Assuming `(0,0)` is top-left:

- `y == 0` → UP
- `x == width - 1` → RIGHT
- `y == height - 1` → DOWN
- `x == 0` → LEFT

The next room is selected using the opposite entrance direction.

Examples:

- exit RIGHT → choose a room whose entrance is LEFT
- exit LEFT → choose RIGHT entrance
- exit UP → choose BOTTOM entrance
- exit DOWN → choose TOP entrance

Room templates define their entrance and exits directly. There are no dynamic door slots.

---

## Normal Room Validation

Current normal-room rule:

- exactly 1 `entrance`
- 1 to 3 `door` exits

Entrance and doors:

- must be on the outer map boundary
- occupy exactly one grid cell
- should have a usable inward cell so the snake can enter/leave safely

The editor should run validation before saving.

If validation fails, Save should be prevented and the user should receive a useful error message.

Possible future validation:

- ensure every exit is reachable from the entrance through playable terrain
- ensure enough safe cells exist to spawn the entire starting snake

These do not need to be implemented unless requested.

---

## Room Maker UI / Feature Requirements

### File operations

Required:

- Add/New Map
- Open
- Save

New Map asks for:

- map name
- width
- height

Open reads a previously saved room JSON.

Save validates and then writes the JSON.

There is no separate Export command.

### Tool options

Required:

- background color
- grid color
- enable/disable grid

These are editor display settings and may be stored in the JSON so reopening restores the editing view.

### Viewport navigation

- Scroll the mouse wheel over the map viewport to zoom in/out, anchored at the cursor.
- Hold Space and drag with the left mouse button to pan the map viewport.
- Panning must not paint cells or add painting history.

### Undo / Redo

The editor supports undo and redo.

Painting operations should participate in history.

Ideally layer operations and other map-changing actions should also participate in history where practical.

### Layer menu

Keep the existing Cypherrush-style layer controls:

- add layer
- remove layer
- rename layer
- move layer up
- move layer down

Layer order matters because higher layers have priority for conflicting terrain.

Each layer has a checkbox to show/hide it in the editor. Visibility is stored as an editor-only `visible` property on the layer and defaults to true when opening older files. Hidden layers retain all cells and still participate in gameplay terrain resolution, validation, and saving. Show a hidden active layer before painting on it.

Layers themselves do NOT have gameplay types such as TERRAIN/OBJECT/DECORATION.

### Drawing tools

Required tools:

- Brush
- Eraser
- Rectangle

Brush paints the selected tile.

Eraser removes content from the active layer only.

Rectangle paints a rectangular region with the selected brush. While dragging, show a translucent brush-color preview with a contrasting dashed outline and the selected width/height in cells. Commit the cells when the mouse is released; clear the preview if drawing is cancelled.

### Brush/tile manager

Keep the existing tile-manager concept.

Required operations:

- add brush
- edit brush
- remove brush

Brush properties:

- `name`
- `color`
- `ratio`

`name` is also the game identifier.

`color` is primarily editor visualization.

`ratio` is retained from the previous tool for future spawning/randomization uses. The C++ room loader may ignore it initially.

### Room Maker marker controls

Provide dedicated controls:

- Place Exit Door
- Place Entrance Door
- Clear Entrance
- Clear Exit

Although `entrance` and `door` ultimately appear as named map entries, they should remain dedicated room-marker tools rather than ordinary user-created brushes.

The editor should use the reserved names:

- `entrance`
- `door`

Benefits of dedicated marker tools:

- enforce boundary placement
- enforce one entrance
- enforce maximum exit count
- simplify clearing/replacing markers
- prevent accidental brush-name mistakes

---

## Target JSON Direction

The exact schema can still be improved, but the intended structure is approximately:

```json
{
  "version": 2,
  "name": "room_01",

  "size": {
    "width": 20,
    "height": 15
  },

  "settings": {
    "backgroundColor": "#000000",
    "gridColor": "#444444",
    "showGrid": true
  },

  "tiles": [
    {
      "name": "ground",
      "color": "#808080",
      "ratio": 100
    },
    {
      "name": "wall",
      "color": "#303030",
      "ratio": 100
    }
  ],

  "layers": [
    {
      "index": 0,
      "name": "Base",
      "cells": [
        { "name": "ground", "x": 1, "y": 1 },
        { "name": "wall", "x": 2, "y": 1 }
      ]
    },
    {
      "index": 1,
      "name": "Objects",
      "cells": [
        { "name": "enemy", "x": 5, "y": 4 },
        { "name": "item", "x": 8, "y": 6 }
      ]
    }
  ]
}
```

Do not assume every example property above must remain exactly as written. Preserve compatibility with the design goals when improving the schema.

---

## C++ Runtime Interpretation

The C++ implementation is intentionally separate from this tool.

The intended future loading flow is:

```text
Room JSON
    ↓
Room loader reads every layer
    ↓
Read every painted entry
    ↓
Interpret entry by name
    ↓
ground/wall/gap/entrance/door
    → Board

enemy
    → enemy/content spawning

item
    → item/content spawning

decoration
    → decoration/rendering
```

The runtime Board remains simple:

```text
Board
└── BoardCell[x][y]
    └── CellType
```

`BoardCell` contains only the runtime terrain type.

Board behavior is provided through Board functions such as:

- `GetCellType(position)`
- `IsWalkable(position)`
- `IsLethal(position)`
- `IsDoor(position)`

The game, not BoardCell, decides what happens after querying those properties.

---

## Player Spawn

Do not store a player spawn marker.

The game finds the `entrance`, determines which boundary it occupies, and places the snake immediately inward from it.

Initial direction points into the room.

Examples:

- LEFT entrance → snake faces RIGHT
- RIGHT entrance → snake faces LEFT
- TOP entrance → snake faces DOWN
- BOTTOM entrance → snake faces UP

Future validation may check that enough inward ground exists to fit the starting snake length.

---

## Features Removed From the Old Cypherrush Tool

The Rouge Snake version does not need the old Cypherrush-specific data/features:

- explicit player spawn
- door size / multi-cell doors
- stored entrance direction
- stored exit direction
- next-room position/anchor
- room center
- star-room door map
- next-room door map
- dynamic door slots
- separate export format

Direction is derived from boundary coordinates instead.

---

## Important Design Principle for Future Changes

Keep the editor generic.

The editor should mainly know:

- grid
- layers
- brushes
- names
- positions
- room entrance/exit constraints

Avoid putting enemy AI, item selection, Board gameplay logic, loot tables, or other C++ game behavior into the editor.

The JSON says **where generic content can exist**. Rouge Snake decides **what actual gameplay object is created there**.

---

## Suggested Codex Improvement Priorities

When improving the tool, prioritize:

1. Keep save/open round-trip reliable. Saving a map, reopening it, and saving again should preserve the room.
2. Keep layer ordering and sparse cell data correct.
3. Make entrance/exit placement and validation robust.
4. Ensure undo/redo is predictable.
5. Make resizing/new-map operations safe.
6. Keep brush management simple and prevent accidental reserved-name conflicts where appropriate.
7. Improve UI/UX without changing the room-data semantics above.
8. Keep the application standalone and easy to run locally.

Before making major schema changes, preserve the agreed runtime assumptions described in this document.
