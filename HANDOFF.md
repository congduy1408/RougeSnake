# Rouge Snake development handoff

Updated: 2026-11-10. This document transfers project context between computers and Codex chats. It does not synchronize chat history or files automatically.

## Start here

1. Read `AGENTS.md`, this file, and `GAME_DESIGN.md` in this directory.
2. Inspect the actual source and `git status` before making recommendations or changes.
3. Discuss design before implementation. The user normally prefers to implement C++ personally; direct requests to implement authorize the specified change.
4. Present UI layouts for review before implementing them.
5. Work on one agreed milestone at a time. Do not treat proposals as approved mechanics.

The Git repository root is currently the parent of this directory. The active project is `Untitled/`, not the old files in the repository's last commit.

## Current prototype

- C++14 and raylib. JSON room templates and a room catalog supply the map.
- Continuous grid movement with a leader and individual typed followers. Self-overlap is allowed. Walls/gaps are lethal; ordinary enemy contact causes no combat damage.
- Leader death ends the run. Followers have individual HP and protection. A killed follower is removed; surviving followers behind them detach and wait on the map.
- Leader contact rescues one waiting soldier per movement step if capacity allows. HP, type, identity, protection, and recruitment history persist.
- Basic and Veteran soldiers use automatic melee. Previously recruited Veterans can fight detached; fresh waiting recruits cannot. Archer cannot fight detached.
- Random recruitment replaces active food, score, and combo. Defaults: attempts every 5 gameplay seconds, base chance 50%, no spawn at 2 or more waiting soldiers, Basic/Veteran/Archer weights 60/20/20. Recruitment does not heal. Healing uses separately collected shards.
- Current capacity is **20 followers plus the leader**, increased from 8 to test menu scrolling. This is a testing setting, not a final balance decision.
- Successful room transitions preserve the attached squad and clear waiting soldiers/projectiles. Recruitment timing carries across rooms.

## Recent changes not fully reflected in older documents

### Archer targeting

`src/soldier.cpp` refreshes a list of live, in-range target positions in enemy-vector order. The Archer selects the first, not the nearest. During preparation, aim follows the selected enemy's current position relative to the Archer's current position. Dead/out-of-range enemies drop out; the next eligible enemy becomes first. Empty targeting cancels preparation. Own-cell targets are excluded.

Arrows use eight grid directions, fixed after launch. This replaces the old perpendicular-to-movement rule. It is not arbitrary-angle aiming, a persistent enemy identity queue, or homing.

### Main menu and Game Over

Dark squad-themed prototype, Start/Retry buttons, Enter or mouse support. Failed restart stays at Game Over, logs failure, and displays an error. Successful initialization enters gameplay.

### Gameplay HUD

Window: 640 × 560. An 80-pixel top panel sits above the original 640 × 480 room. A Camera2D drawing offset preserves logical grid coordinates.

The HUD shows follower count/capacity and Basic/Veteran/Archer totals, excluding the leader and waiting soldiers. No HUD HP: sprite appearance shows health. Cipher/item HUD now reflects leader inventory. A separate HEAL counter displays collected consumable shards.

### Paused expedition menu

- Tabs: Squad, Items, Enemies, Boss Travel. Items shows carried leader items and consumable shards; Enemies and Boss Travel remain placeholders. Combat effects are not designed yet.
- Gameplay, attacks, arrows, protection, and recruitment timers stop while `PAUSE_MENU` is active.
- Squad list includes leader and followers, with selected member HP/details. Leader information can expand when leader types are designed.
- Eight rows are visible at a time; selection scrolls through a larger squad. User tested capacity 20 and reported scrolling works.
- Only followers can Detach. Leader selection is allowed for future healing/info, never detachment.
- Detach removes only the selected follower, preserves their position/HP/type/identity/protection, cancels unfinished combat, and places them in waiting storage. Followers behind them remain attached. This is distinct from casualty detachment.
- Detach uses confirmation. Heal consumes one shard to restore 1 HP to the selected living leader/follower, without exceeding max HP.
- User originally imagined a horizontal chain of selectable soldiers, but agreed to test the vertical list first. Do not replace the layout without discussion.

## Controls

| Context | Controls |
| --- | --- |
| Main menu / Game Over | Enter or click Start/Retry; Esc quits |
| Gameplay | Arrow keys steer; A/D cycles items; Space uses equipped item; Tab or Esc opens paused menu |
| Paused menu | Q/W previous/next tab, wrapping at ends |
| Squad list | Up/Down selects member; Enter opens actions |
| Follower actions | Up/Down chooses Detach/Heal; Enter heals or selects Detach, then Enter confirms |
| Back / resume | Esc cancels confirmation, backs out of actions, then resumes; Tab resumes directly |
| Development shortcuts | K/L/O adds Basic/Veteran/Archer while playing; I damages node index 2; J spawns a filler item pickup |

The window close button still exits. Automatic raylib Esc-to-exit is disabled so Esc can operate the pause menu.

## Healing Shards (implemented)

- One shard spawns per room visit on randomly selected unoccupied ground, after squad/enemy placement. No eligible cell means no spawn. No timed respawn within a visit.
- Exclude attached squad, living enemies, living waiting soldiers, entrances, exits and hazards. Recruitment also excludes an active shard's cell.
- Only leader contact collects it. Pickup adds one shared shard, not HP. The ground marker is a mint diamond with a dark cross.
- Start each run with zero collected shards. Collected supply persists through room transitions and resets on restart. A successful transition replaces the uncollected room shard; early failed transitions retain it.
- Menu: select member, Enter for actions, Up/Down selects Detach or Heal, Enter applies. Leader defaults to Heal and cannot detach. Heal is visually disabled at full HP or zero supply. Dead/invalid targets are rejected without consuming supply.
- Items tab has a distinct Consumables section. Healing is separate from equipment/item slots. The HUD shows HEAL xN.
- Release and Debug macOS builds passed. Temporary headless checks covered placement, exhausted cells, leader-only one-time collection, healing limits and resource use, menu healing, transition persistence, restart reset, and recruit exclusion. Visual pickup/healing playtest remains pending.
- Next: playtest shard visibility, pickup, and menu operation. Future balance and actual item/enemy/boss systems still need design. No rare-drop probability or three-charge starting supply was implemented.

## Leader inventory and item controls (implemented after healing)

- The leader owns and uses combat items; future effects may affect squad/enemies/environment. Real effect designs remain pending.
- Capacity is cipher length + 2. Current test cipher is STAR, giving six slots. Cipher matching reads item letters, ignores order, and consumes each matching item once for duplicate letters. No boss gate or clue-reveal mechanics yet.
- A/D cycles selection with wrapping. Space activates a ready item. Individual cooldowns continue while another item is selected, including dropped items while in the room; all pause with the menu.
- Empty inventory at run start. Normal item reward sources are not implemented. J cycles through the 14 real filler items on safe ground. Fillers are passive: Space does not activate them or start a cooldown. Diagnostic TestSignal remains only as a framework test effect, not in the filler pool.
- Leader contact collects at most one ground item per movement step. With free slots, append it and preserve the existing selection (first item equips automatically). At capacity, swap into the equipped slot and leave the old item at that cell with its cooldown intact.
- Dropped items require leaving their cell before pickup eligibility returns. Ground items clear on successful room transition; carried inventory/cooldowns persist. Restart clears inventory and selection.
- HUD shows inventory letters, selected item and remaining cooldown, cipher matches, and separate HEAL supply. In Items menu, Up/Down inspects and Enter equips; resume before using Space.
- Debug shortcuts: I damages follower index 2; J spawns a filler pickup; K/L/O adds Basic/Veteran/Archer. A is no longer a debug damage key.
- Release/Debug builds and temporary headless inventory/integration checks passed. Tests covered duplicate matching, wrapping, cooldowns, pause, equipping, swaps, drop lock, transition persistence, safe spawning and restart. User reported inventory operation works. Filler-specific visual playtest remains pending.
- General Item now has stable id, name, category, letter, description, effect and per-instance cooldown. The 14 GDD fillers are defined in GetFillerItems() in src/item.cpp. Descriptions are flavor-only and do not repeat cipher letters; letters have a separate UI field. Cryptbound Book is passive with clue reveal explicitly deferred.
- Persistent catalog tests: tests/filler_items_test.cpp. Mac Release/Debug builds and temporary game-level pickup/swap/transition regressions passed after the filler change.
- Next: playtest filler UI, then design a small set of actual leader-usable combat items before enemy redesign.

## Build and run

### Separate native remote-play companion

`../tools/remote-play/` contains a Mac-only window capture/input helper and Python HTTP server with a responsive browser client. It streams the actual native executable; gameplay/build files were not modified for it. See its README for startup, Tailscale Serve, and permission steps.

Use `sh tools/remote-play/run.sh --launch-game` from the repository root (omit launch flag when already running). Default loopback port 8787, 20 fps JPEG frames, token-authenticated access, one controller, keyboard/touch buttons and disconnect key release. Tailscale Serve proxies this privately. Screen Recording and Accessibility permissions must be approved on the Mac. No audio or Windows hosting. A locked/sleeping Mac can block capture/control. Do not claim end-to-end streaming is verified solely from compilation or HTTP tests.

Always launch with this directory as the working directory: `map/` and `sprite/` paths are relative.

macOS prerequisites: Apple Command Line Tools, Homebrew raylib. These were already installed on the Mac used in this chat.

```sh
make -j4
./RougeSnake
make -j4 BUILD_MODE=DEBUG PROJECT_NAME=RougeSnakeDebug
```

Windows uses the existing raylib/w64devkit configuration. From this directory:

```powershell
& 'C:\raylib\w64devkit\bin\mingw32-make.exe'
.\RougeSnake.exe
```

Check the installed Windows toolchain paths first. Windows has not been rebuilt since these changes. The Makefile defaults to `C:/raylib/raylib-min`; IntelliSense has separate paths that may need alignment with the actual installation.

VS Code has platform-specific tasks/configurations in both the repository root and this directory. On Mac, choose `Launch RougeSnake (macOS)` for debug. Actual F5 debugger launch remains unverified.

Objects are isolated by OS and build mode under `obj/`. Automatic header dependencies are enabled. `.gitignore` excludes compiler outputs, libraries/debug symbols, and the packaged game ZIP. The macOS executable links Homebrew raylib dynamically and is not a standalone release package.

## Verification and remaining work

- Release/Debug macOS builds passed for the HUD and paused menu. Release passed after increasing capacity to 20.
- Temporary headless regression checks passed for restart failure/retry, Archer priority/range/death/cancellation, menu input, HUD counts/dimensions, paused updates, leader detach guard, confirmation, selected-only detach, preserved state, and selection bounds.
- Those checks were in `/tmp`, not committed reusable test files. Do not claim they are available or rerun on another computer.
- Earlier design notes record 71 regression scenarios for older recruitment/combat behavior; they were not rerun as a complete suite in this chat.
- User approved the displayed HUD and reported expanded-squad scrolling works. Broad combat/recruitment/transition playtesting remains useful. Do not infer complete visual verification from builds or headless tests.
- Existing virtual-method and unused-field warnings remain. Room-load atomicity and malformed-map handling need review. Actual combat item effects, full cipher/clue progression and gates, enemy types/encyclopedia, bosses, and teleport gating remain future work. Healing and inventory are implemented.

## Documentation drift

`GAME_DESIGN.md` is the long-form design reference, but still contains the old Archer rule, capacity 8, and an obsolete missing-header-dependencies backlog item. It lacks the recent UI and voluntary detach details. Discuss conflicts before changing behavior; update the design document only with user approval per AGENTS.md.

`RougeSnake_Implementation_Tracker.xlsx` was revised for squad gameplay earlier in this chat. Later menus, targeting, HUD, and capacity changes have not been reconciled into it. Do not use its older task statuses as proof that these newer changes are missing.

## Switching devices

Use Git to transfer source/assets/docs. Build outputs remain local. Signing into Codex does not by itself transfer local repository changes.

1. Check `git status` and review changes.
2. Commit the intended changes and push the working branch before switching.
3. Pull that branch on the other device before editing.
4. For simultaneous development, use different branches for separate tasks; merge deliberately.
5. Update this handoff after each milestone: decisions, verification, unresolved issues, next task.

**Current Git issue:** the working tree shows old root project files as deleted, the active `Untitled/` tree as untracked, modified root VS Code files, and untracked generated spreadsheet exports in `outputs/`. Review this migration and choose intended files before staging. Do not blindly stage everything, restore old files, or discard work. No commit or push was performed in this chat.

Suggested opening prompt on the other device:

> Continue Rouge Snake. Read Untitled/AGENTS.md, Untitled/HANDOFF.md, and Untitled/GAME_DESIGN.md. Inspect current source and Git status. Follow the design-first workflow. Healing Shards are implemented; review the handoff and discuss the next milestone before changing gameplay.
