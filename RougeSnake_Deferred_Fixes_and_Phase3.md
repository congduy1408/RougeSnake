# Rouge Snake — Deferred Fixes & Phase 3 Plan

Updated: 2026-10-16

## Scope and decisions

These items were identified during review of `RougeSnake(20261013-173917).zip` and subsequent design discussion. They are deferred so development can proceed to room transitions. This document is a backlog, **not** a claim that the issues have been fixed or that the game was executed successfully.

- Room Maker is responsible for ensuring exactly one entrance, valid exits, and safe inward spawn cells for authored maps. Runtime validation can remain minimal for now; reconsider when adding procedural maps or untrusted map files.
- Entrance/exit are structural markers with priority over ordinary terrain, and can be collected during JSON layer processing. Higher-index layers override ordinary terrain. Non-terrain object markers are stored separately.
- A new room entry positions the entire existing snake body at the spawn cell; body segments unfold during movement. Player control is immediate. Temporary enemy-attack invincibility lasts until at least three nodes have unfolded (normally two successful steps), independent of total snake length. Wall/gap hazards remain lethal.

## Deferred review findings

| Priority | Area | Issue / follow-up | Suggested future resolution |
|---|---|---|---|
| Medium | Food placement | Bounded random attempts may fail even when valid cells exist; no valid cell may exist at all | Keep bounded attempts. Later add deterministic scan fallback, return `bool`, and make food inactive on true exhaustion. Never retry indefinitely or recursively. |
| Medium | Room loading | Room/Board can be partially modified if a later load step fails | Prepare and validate next room before committing it, particularly during transitions. |
| Medium | Room validation | Missing/duplicate entrance, missing/duplicate exits, corner or out-of-bounds doors may not be fully rejected | Validate in Room Maker; add lightweight runtime checks if needed for robustness or procedural generation. |
| Medium | Spawn validation | Inward spawn cell may be blocked or lethal | Validate in Room Maker; optionally check `Board::IsWalkable` and `!Board::IsLethal` at runtime. |
| Medium | Game restart | Restart path may enter STAGE despite failed `InitGameObject()` | Propagate and handle failure instead of continuing. |
| Medium | Room markers | Ensure `object_markers` and exit lists are cleared on every `Room::Load()` | Clear collections at load start; avoid stale or duplicate markers across transitions. |
| Medium | Layer rules | Structural door priority must be consistent regardless of layer order; duplicate/conflicting structural markers require defined handling | Keep explicit structural-marker policy and add tests for both layer orders. |
| Low | String handling | `TryGetCellType()` copies a string argument | Use `const std::string&` if appropriate. |
| Future | Enemy damage | `IsInvincible()` exists but is not used until enemy damage is implemented | Check it centrally in `Snake::TakeDamage()`, not in terrain collision. |
| Future | Unfolding | `IsUnfolding()` and `IsInvincible()` should remain conceptually distinct | Full unfolding depends on actual body length; short entry protection depends on first two successful moves. |

## Phase 3 — Room transitions

### Goal

When the snake head enters an exit cell, load a compatible next room and reposition the snake at its entrance while preserving run state.

### Proposed incremental milestones

1. **Exit detection:** In Game's movement/collision coordination, detect `Board::IsDoor(headPosition)` and identify which exit was reached. Avoid triggering repeatedly in the same transition.
2. **Next-room selection:** Initially use a small fixed sequence or test mapping of JSON room templates. Match the next room's entrance side to the opposite side of the exit: RIGHT exit → LEFT entrance, TOP exit → BOTTOM entrance, etc. Do not introduce random generation yet.
3. **Safe room replacement:** Load the next `RoomTemplate`, build the new Room/Board, and only replace the current playable state after successful loading. Define failure behavior (e.g. keep current room and report error).
4. **Snake entry:** Add `Snake::EnterRoom(const Room&)` distinct from `Reset()`. Reposition all existing segments at the new inward spawn cell, set movement inward, reset unfolding progress, preserve node HP, length, and upgrades.
5. **Room contents:** Reset or respawn room-local food and, later, enemies/items using the new room's markers. Preserve run-global player state.
6. **Tests:** Transition through left/right/top/bottom exits; verify opposite entrance and direction; verify length/HP persistence; verify no repeated transition; verify failed map load leaves a playable room.

### Responsibilities

- **RoomTemplate:** Parse JSON only.
- **Room:** Own loaded-room metadata (entrance, exits, markers) and apply terrain to Board.
- **Board:** Terrain queries and drawing; does not select the next room.
- **Snake:** Preserve snake state while repositioning on room entry.
- **Game:** Detect door contact, choose next template, coordinate transition and room-local respawn.

### Design questions for implementation

- Should door transitions happen as soon as the head moves onto the door cell? (Suggested: yes.)
- What should happen if no compatible next-room template is available? (Suggested for prototype: log error and retain current room.)
- Should food be respawned on every room entry? (Suggested: yes, as room-local state.)
