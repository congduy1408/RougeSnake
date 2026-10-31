We are continuing development of Rouge Snake.

Read AGENTS.md and GAME_DESIGN.md, then inspect the current source code to understand the implementation.

We will follow a design-first workflow: discuss a feature, agree on its structure, I implement it, and you review my changes.

Do not modify any source files unless I explicitly request it.

Our current milestone is Phase 4C: Enemy Attacks. We have completed basic enemy spawning, random movement, terrain checking, HP, damage, and death.

Development follows the design-first workflow above; implementation status and agreed next steps are recorded below.

## Agreed Snake HP and survival rules (2026-10-24)

- Each Snake node starts with 3 HP. Movement preserves each node's HP.
- Newly grown nodes start at full HP; entering another room preserves existing node HP and length. Restarting rebuilds the starting Snake with full HP.
- When a body node reaches zero HP or below, remove that node and every node behind it, regardless of their remaining HP.
- Starting length is a spawn setting, not a minimum surviving length. A Snake with only a healthy head can continue moving, eating, and growing.
- Head HP reaching zero or below ends the run. Keep the head node in the body; do not remove it through tail cutting.
- Snake reports its death through `IsDead()`. Game switches to `GAMEOVER` and stops gameplay processing when death is detected.
- Damage is caused by explicit attacks. Contact or movement overlap alone damages neither the Snake nor enemies. Terrain hazards remain lethal independently of node HP or attack protection.

## Agreed enemy behavior and attack architecture

- Enemy owns its AI, movement, movement and attack timers, and attack selection. Movement remains terrain-checked inside Enemy; Game does not execute returned movement destinations.
- Game calls each living enemy's `Update()` every fixed update with elapsed time, `const Board&`, and `const Snake&`. An update is an opportunity to observe and decide, not a requirement to attack.
- Observation and behavior selection initially belong in one AI function. Read the Snake's actual positions and available movement information; separate helpers or prediction can be introduced when a specific enemy needs them. Do not assume player attack or dodge actions exist.
- Complex enemies may prepare, release, and recover from attacks across multiple updates. Their targeting and preparation rules belong to the enemy's behavior.
- The update result describes an attack released during that update: whether an attack occurred, affected grid cells, and damage. A waiting or moving enemy can return no attack. A movement flag or destination is unnecessary for Game under the agreed no-contact-damage rule.
- Attack cells describe the released attack in board coordinates. The current enemy locks its target cells when preparation begins; Game resolves those cells without asking the enemy to choose a target again at release.
- The current simple enemy scans Snake nodes from head to tail and targets the first node inside a centered 3x3 square (`observation_radius = 1`). This includes diagonal cells; same-cell overlap behavior is deferred.
- The current attack deals 1 damage to the selected cell on entry to Attack. Timings are 0.5 seconds Prepare, 0.08 seconds Attack, and 1.0 second End (recovery). There is no additional cooldown timer.
- Ready enemies observe and begin preparation immediately when a target is available; otherwise they move randomly with terrain checks. Enemies do not move or retarget during Prepare, Attack, or End.
- Prepare displays the locked cells in yellow at 50% opacity; Attack displays the released cells in red at 50% opacity. Drawing reads the attack phase, not a separate flash countdown, and never repeatedly applies damage while the zone is visible.

## Attack collection and sequential damage resolution

1. Preserve the current Snake movement, terrain-hazard, and room-transition flow. A room transition returns before enemy processing, so attacks from the old room are not carried into the new room.
2. Update all living enemies and collect their released attacks before applying enemy attack damage. Enemy decisions therefore do not observe cuts caused by earlier attacks in that same update.
3. Resolve collected attacks sequentially in enemy vector order. This is the chosen initial behavior; simultaneous damage aggregation is deferred.
4. For each attack, resolve each distinct affected cell once through `Snake::DamageNode(cell, damage)`. An empty cell misses. One area attack may affect several nodes at different cells.
5. For overlapping Snake nodes, a cell targets the first matching node from head to tail. If that node is protected, ignore the hit rather than redirecting it to another overlapping node.
6. Snake owns entry protection, per-node damage protection, HP reduction, and cutting. Game coordinates attack resolution without retaining references or indices to nodes that cutting may remove.
7. Later attacks resolve against the body remaining after earlier attacks. A node already removed by a cut cannot receive a later hit.
8. After each damage application, check Snake death. Head death immediately switches Game to `GAMEOVER` and stops further gameplay processing. Retain the existing dead-enemy cleanup for normal processing.

- Sequential resolution gives earlier enemies priority when several attacks target the same node. The first spawned enemy normally has index zero and resolves first; spawn order does not determine cooldown or attack frequency.
- Per-node damage protection prevents damage stacking on a surviving node, but does not eliminate order priority: if attacks have different damage values, the first accepted hit determines the damage taken.
- Game may use a small attack-resolution helper to keep its update readable. No new manager class is needed.

## Per-node protection after damage

- Room-entry protection applies to the entire Snake and ends after two movement steps following reset or room entry, independently of total length and full unfolding.
- Separately, each surviving node gains a short period of attack invincibility after accepting damage. Further hits on that node during this period are ignored and do not refresh the timer. Other nodes remain independently vulnerable.
- Store remaining protection as elapsed-time-based state per node, rather than counting rendered frames. Decrease timers once per gameplay fixed update, independently of movement and before resolving new attacks.
- Like HP, the timer belongs to its node and must not be copied along the body with position or drawing information during movement.
- Apply cutting or head death immediately on lethal damage; protection does not rescue a destroyed node. Terrain hazards bypass both forms of attack protection.
- Per-node protection and blinking are implemented. The current duration is 1.0 second. Movement preserves each node's timer and Game decreases timers each gameplay fixed update. Room entry currently preserves timers; growth currently copies the tail's timer and still needs the fresh-node reset described in the deferred items below.

## Attack lifecycle and update structure

- Attack phases are `Ready -> Prepare -> Attack -> End -> Ready`. Preparation, attack display, and recovery run sequentially using one `phase_elapsed` timer, reset by `ChangeAttackPhase()` at every transition.
- `Enemy::Update()` rejects dead enemies, then dispatches to `UpdateBehavior()` while Ready or `UpdateAttack()` during an active attack sequence.
- `UpdateBehavior()` observes the Snake, locks target cells and begins Prepare, or performs terrain-checked random movement. Its separate movement timer advances only while Ready, preserving movement readiness while an attack sequence is active.
- `UpdateAttack()` advances the shared phase timer. Prepare returns no action for 0.5 seconds, then emits exactly one `EnemyAction` while entering Attack. Attack lasts 0.08 seconds without emitting further damage. End lasts 1.0 second, then returns to Ready; the next decision occurs on the next fixed update.
- Each transition resets elapsed phase time rather than carrying excess time into the next phase. Durations are therefore quantized to fixed-update steps and are tuning values.
- The target warning remains fixed even if the original node moves away. At release the attack may miss or hit another node occupying the cell. No Snake-node reference or index is retained.
- Target data is retained through Prepare and Attack and cleared when entering End. Enemy recreation on room transition or restart clears pending attacks naturally.
- Visual style remains simple and retro: enemy sprites, colored warning/strike areas, and optional weapon overlays or projectile sprites. Multi-frame attack animation assets are deferred. A future visual effect can use phase and phase time without controlling the gameplay update.
- Game's collection and sequential resolution flow is unchanged. Future projectile support will require a spawn effect and subsequent collision handling; firing a projectile must not immediately damage its destination cell.

## Current implementation and next milestone

- Basic Phase 4C enemy attacks are implemented: target detection, locked warning cells, delayed attack release, timed Attack and End phases, returned attack cells, collection and sequential resolution through `ProcessEnemyAttack()`, and immediate head-death handling.
- Node HP, HP-preserving movement, fresh-node HP on growth, body cutting, `Snake::IsDead()`, per-node damage protection, and blinking are implemented.
- Game checks Snake death before movement and after each attack-cell resolution. `DamageNode()` rejects nonpositive damage and damage to a dead Snake.
- Entry protection has been corrected in source: the counter advances while unfolding or protection remains active, and damage no longer waits for full unfolding. This has been source-reviewed; runtime verification for lengths one, two, three, and a longer Snake remains to be recorded.
- Contact damage is disabled in both directions: Game no longer invokes the old overlap damage interaction, and Enemy's `OnSnakeEnter()` is currently a no-op. Enemy HP support remains for future explicit player attacks.
- User verification: body cutting and yellow targeting indicator work; a clean rebuild picked up the 1.0-second enemy cooldown; multiple-enemy hits respect node protection; lethal head damage ends the run immediately.
- Shared-timer refactor verification: C++14 compile checks passed. A temporary executable using the production code passed checks for preparation delay, locked targets, one-shot damage release, recovery, retargeting on a later attack, stationary attack phases, and the dead-enemy guard. This does not substitute for visual playtesting.
- Random movement is restored in the current source. Visual playtesting remains for the full yellow-warning/red-strike sequence, movement resumption, and room transitions during preparation.
- Next milestone: finish visual verification of the attack lifecycle, then agree on and test same-cell Snake/enemy overlap targeting. More attack patterns can follow after that rule is settled.

## Deferred fixes and future improvements

- Build: Makefile does not track header dependencies, so changing values in headers may leave stale object files. A clean or forced rebuild is currently needed; add automatic header dependency tracking later.
- Growth: explicitly reset a new node's `ghost_time` to zero rather than inheriting the copied tail timer.
- Define same-cell Snake/enemy overlap targeting before adding special handling.
- Cleanup: remove unused enemy `ghost_phase` and obsolete contact hooks if no longer needed. Enemy's parameterized `Update()` is not an override of `GameObject::Update()`; revisit the base interface if polymorphic updates become necessary.
- Future features: simultaneous damage aggregation, prediction helpers, more complex attack states, projectiles, and larger attack patterns. No new managers or external dependencies are planned.
