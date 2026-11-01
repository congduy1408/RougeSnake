We are continuing development of Rouge Snake.

Read AGENTS.md and GAME_DESIGN.md, then inspect the current source code to understand the implementation.

We will follow a design-first workflow: discuss a feature, agree on its structure, I implement it, and you review my changes.

Do not modify any source files unless I explicitly request it.

The implemented baseline is Phase 4C: Enemy Attacks. The next design direction is a leader-controlled soldier squad built on the existing grid-following movement and node HP systems.

Development follows the design-first workflow above; implementation status and agreed next steps are recorded below.

## Agreed squad concept (supersedes Snake casualty rules)

- The player controls a leader, represented by the current head. Each following node represents an individual soldier in the squad.
- The last position in the line is not a special soldier type or role. All followers use their normal soldier/type sprite, including the last one; no dedicated tail sprite or tail-only behavior is required. The leader remains distinct.
- Reuse the existing continuous grid movement and following-path foundation. Soldier types, HP, protection, and combat state belong to individual soldiers and must not shift between soldiers when movement information advances.
- Attached soldiers attack automatically when a living enemy is detected and within attack range. Soldier roles will support different attack styles, including melee and ranged attacks.
- Soldier combat data includes detection range, attack speed/timing, damage, and attack pattern. Detection range and attack reach have distinct meanings; exact units, values, patterns, and target-selection rules still need design before implementation.
- A soldier's attack-facing direction must be represented separately from the direction data used to follow the column. The detailed aiming and release-origin policy for moving soldiers remains to be agreed.
- The existing leader/head death condition remains the baseline: leader HP reaching zero ends the run. Whether the leader also attacks automatically is still undecided.

## Casualties and detached soldiers

- When an attached soldier dies, remove that soldier. All surviving soldiers behind it leave the active squad and remain on the map at their current positions; do not delete those survivors.
- Detached soldiers are stationary, individually rescuable units. They remain vulnerable to explicit enemy attacks and can be killed while waiting for rescue.
- Preserve each survivor's identity, type, remaining HP, and applicable runtime state when detaching. Death and detachment do not heal survivors.
- Each soldier type has a capability flag, tentatively named `can_attack_detached`. Lower-level types cannot attack while detached; later collectible types may permit it.
- The capability flag and current attachment status are separate concepts. Attached soldiers use automatic combat; detached soldiers use it only when their type permits it and their normal attack requirements are met.
- Detached soldiers do not automatically form a moving replacement squad. The former rear portion of the line becomes individual soldiers waiting for rescue.

## Individual rescue

- Only leader/head contact with a detached soldier's cell triggers rescue. Contact from another attached soldier does not rescue anyone.
- Rescue appends only the touched soldier to the end of the active squad. Other detached soldiers remain on the map until individually rescued or killed.
- The rescued soldier initially occupies the current last squad member's cell. If only the leader remains, use the leader's cell. Copy that last member's following-position and direction information, then let the rescued soldier separate naturally on subsequent movement steps, as in the current growth placement.
- Rescue preserves the rescued soldier's type, remaining HP, and remaining damage-protection time. It is not a full-health growth operation: do not copy the last member's HP, protection, or combat state. Transfer the rescued soldier into the active squad without duplicating it.
- Placement at the end changes line order and following information, not soldier identity. The rescued soldier is rendered as its ordinary type and gains no special visual role from being last.
- The policy for preserving or canceling an in-progress soldier attack during rescue remains to be designed; remaining damage protection is preserved as agreed above.
- If several detached soldiers occupy one cell, rescue the first living soldier in storage order, at most once per leader movement step. Other soldiers wait for another leader contact.

## Squad implementation boundaries and open decisions

- Shared `Soldier` records now hold position, following directions, HP, and remaining protection. `Snake::body` stores attached soldiers; Game owns `detached_soldiers`. Soldier roles and allied automatic combat are still future work.
- `DamageNode()` returns surviving rear soldiers when a follower dies. `DetachSoldier()` removes the casualty and rear portion from the active vector while preserving the returned survivors' records. Game and the test helper store those survivors.
- Detached soldiers are drawn at their preserved positions using the body sprite. `HandleSoldierRescue()` runs after a leader movement step, before food and enemy processing; `AttachSoldier()` preserves the recruit's HP/protection and copies only the last member's following information.
- Current attached drawing still has a dedicated `SNAKE_TAIL` branch. Replacing it with ordinary follower rendering remains deferred.
- Game coordinates damage against attached and detached soldiers. Enemies receive both collections as read-only observation inputs; they return attack cells without modifying either collection.
- Allied automatic attacks will require enemy targeting and effects resolved by Game. The current EnemyAction/cell-damage approach and timed phase lifecycle are reusable foundations; projectile spawning and travel remain future work.
- Open decisions include allied/enemy attack ordering, soldier targeting priorities, moving attack origins, leader combat, future overlap-policy refinements, and the fate of unrescued soldiers when leaving a room.
- Recruitment sources, how the current food/score mechanics map to the squad concept, and naming/asset changes remain undecided. Preserve the room-transition system while designing the new behavior.
- Rescue placement is implemented. Ordinary last-soldier rendering and soldier combat remain pending. No new manager or class hierarchy is required.

## Existing movement and HP baseline

- Each Snake node starts with 3 HP. Movement preserves each node's HP.
- Newly grown nodes start at full HP; entering another room preserves existing node HP and length. Restarting rebuilds the starting Snake with full HP.
- When a follower reaches zero HP or below, remove that casualty and transfer surviving rear soldiers to Game-owned detached storage.
- Starting length is a spawn setting, not a minimum surviving length. A Snake with only a healthy head can continue moving, eating, and growing.
- Head HP reaching zero or below ends the run. Keep the head node in the body; do not remove it through tail cutting.
- Snake reports its death through `IsDead()`. Game switches to `GAMEOVER` and stops gameplay processing when death is detected.
- Damage is caused by explicit attacks. Contact or movement overlap alone damages neither the Snake nor enemies. Terrain hazards remain lethal independently of node HP or attack protection.

## Agreed enemy behavior and attack architecture

- Enemy owns its AI, movement, movement and attack timers, and attack selection. Movement remains terrain-checked inside Enemy; Game does not execute returned movement destinations.
- Game calls each living enemy's `Update()` every fixed update with elapsed time, `const Board&`, `const Snake&`, and `const std::vector<Soldier>&` for detached soldiers. An update is an opportunity to observe and decide, not a requirement to attack.
- Observation and behavior selection initially belong in one AI function. Read the Snake's actual positions and available movement information; separate helpers or prediction can be introduced when a specific enemy needs them. Do not assume player attack or dodge actions exist.
- Complex enemies may prepare, release, and recover from attacks across multiple updates. Their targeting and preparation rules belong to the enemy's behavior.
- The update result describes an attack released during that update: whether an attack occurred, affected grid cells, and damage. A waiting or moving enemy can return no attack. A movement flag or destination is unnecessary for Game under the agreed no-contact-damage rule.
- Attack cells describe the released attack in board coordinates. The current enemy locks its target cells when preparation begins; Game resolves those cells without asking the enemy to choose a target again at release.
- The current simple enemy scans attached soldiers from head to tail, then detached soldiers in storage order, and targets the first living soldier inside a centered 3x3 square (`observation_radius = 1`), excluding its own cell. Diagonal cells are valid. An overlapping node is skipped so another neighboring node can still be selected; contact itself causes no damage.
- The current attack deals 1 damage to the selected cell on entry to Attack. Timings are 0.5 seconds Prepare, 0.08 seconds Attack, and 1.0 second End (recovery). There is no additional cooldown timer.
- Ready enemies observe and begin preparation immediately when a target is available; otherwise they move randomly with terrain checks. Enemies do not move or retarget during Prepare, Attack, or End.
- Prepare displays the locked cells in yellow at 50% opacity; Attack displays the released cells in red at 50% opacity. Drawing reads the attack phase, not a separate flash countdown, and never repeatedly applies damage while the zone is visible.

## Attack collection and sequential damage resolution

1. Preserve the current Snake movement, terrain-hazard, and room-transition flow. A room transition returns before enemy processing, so attacks from the old room are not carried into the new room.
2. Update all living enemies and collect their released attacks before applying enemy attack damage. Enemy decisions therefore do not observe cuts caused by earlier attacks in that same update.
3. Resolve collected attacks sequentially in enemy vector order. This is the chosen initial behavior; simultaneous damage aggregation is deferred.
4. For each attack cell, check attached occupancy first. If occupied, use `Snake::DamageNode(cell, damage)` and store any returned survivors; otherwise use `Game::DamageDetachedSoldier()`. Empty cells miss. Each cell selects one victim; the current enemy emits one cell. Future area-pattern producers must supply distinct cells.
5. For overlapping Snake nodes, a cell targets the first matching node from head to tail. If that node is protected, ignore the hit rather than redirecting it to another overlapping node.
6. Snake owns attached entry protection, per-node protection, HP reduction, and detachment. Game owns detached HP/protection handling and removes only the killed detached soldier. Detached soldiers have no squad room-entry immunity. No node references or indices survive a casualty operation.
7. Later attacks resolve against the current attached and detached collections. A newly detached survivor can receive a later attack. Within one cell resolution, do not hit both the attached victim and a detached soldier, including one just detached by that hit. If no attached soldier occupies a cell, select the first living detached soldier there; protection blocks the hit without redirecting to another overlapping soldier.
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
- Per-node protection and blinking are implemented. The current duration is 1.0 second. Movement preserves each node's timer and Game decreases timers each gameplay fixed update. Room entry currently preserves timers. Fresh growth resets the new node's ghost timer to zero; rescue preserves the rescued soldier's remaining protection rather than applying the fresh-growth reset. Game also reduces detached protection once per gameplay fixed update before rescue and attacks; timers freeze outside STAGE. Detached survivors gain the same 1.0-second protection after damage and blink while protected.

## Attack lifecycle and update structure

- Attack phases are `Ready -> Prepare -> Attack -> End -> Ready`. Preparation, attack display, and recovery run sequentially using one `phase_elapsed` timer, reset by `ChangeAttackPhase()` at every transition.
- `Enemy::Update()` rejects dead enemies, then dispatches to `UpdateBehavior()` while Ready or `UpdateAttack()` during an active attack sequence.
- `UpdateBehavior()` observes attached and detached soldiers, locks target cells and begins Prepare, or performs terrain-checked random movement. Its separate movement timer advances only while Ready, preserving movement readiness while an attack sequence is active.
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
- Own-cell enemy targeting is excluded in source, as agreed. Visual attack-lifecycle verification and runtime overlap checks remain to be recorded.
- Squad detachment, Game-owned storage, detached drawing, and individual leader-contact rescue are implemented. The user verified that the detached rear portion draws correctly.
- Enemy observation and damage now include detached soldiers, preserving locked warning cells, sequential resolution, and one-victim-per-cell priority. Detached death removes only that soldier, with no second detachment operation or game over.
- Detached-damage verification: C++14 syntax checks passed for all source and test files. A temporary executable linked against production code passed nine scenarios covering detached observation and wind-up, attached targeting priority, own-cell/dead-target exclusion, simultaneous-hit protection and expiry, isolated detached death, overlap priority, casualty transfer followed by later damage, nonpositive damage/menu timers, and immediate leader death. Protection blinking and rescue during a warning still need visual playtesting. Existing signedness and hidden-base-Update warnings remain.
- Next milestone: playtest detached targeting, protection blinking, death, and rescue during warning; then design soldier roles and allied automatic attacks incrementally.

## Deferred fixes and future improvements

- Build: Makefile does not track header dependencies, so changing values in headers may leave stale object files. A clean or forced rebuild is currently needed; add automatic header dependency tracking later.
- Growth protection reset is now implemented in source; manual verification of growing from a protected tail remains to be recorded.
- Enemy own-cell exclusion applies to both attached and detached targets. Initial overlap priority is deterministic; broader targeting policies may be revisited later.
- Room transition policy for unrescued soldiers remains undecided. Current source clears detached storage on restart but retains it across room transitions, so old-room coordinates can appear in the new room. Resolve this before treating detached units as room-persistent content.
- Cleanup: remove unused enemy `ghost_phase` and obsolete contact hooks if no longer needed. Enemy's parameterized `Update()` is not an override of `GameObject::Update()`; revisit the base interface if polymorphic updates become necessary.
- Future features: simultaneous damage aggregation, prediction helpers, more complex attack states, projectiles, and larger attack patterns. No new managers or external dependencies are planned.
