We are continuing development of Rouge Snake.

Read AGENTS.md and GAME_DESIGN.md, then inspect the current source code to understand the implementation.

We will follow a design-first workflow: discuss a feature, agree on its structure, I implement it, and you review my changes.

Do not modify any source files unless I explicitly request it.

The baseline includes enemy attacks, squad detachment/rescue, typed soldiers, melee attacks, and Archer projectiles. The recruitment milestone below replaces food, score, and combo with limited random recruit spawning.

Development follows the design-first workflow above; implementation status and agreed next steps are recorded below.

## CypherRush setting and story reference

Source: [CypherRush design document](../CypherRush/CyperRush/cypherrush_gdd_current.md), especially High Concept, Guardian / Boss Structure, Prophecy and Clues, Crypt Biome, and Crypt Enemy Details. This section records the established background for future CypherRush-related content; it does not make CypherRush's gameplay systems implemented or required in Rouge Snake.

### Setting

- CypherRush is a roguelike dungeon crawler built around combat, exploration, and deciphering a prophecy.
- Its first environment is an underground Crypt: bones, rotting corpses, grave soil, cursed tombs, and ancient sealed doors. Its inhabitants primarily belong to bone and undead enemy families.
- The setting's supernatural themes include holy power, darkness, the void, blood, and grave/bone magic. Forbidden necromancy is represented by Grave Bloom, a grave-rite acquired from a defeated Necromancer that temporarily summons weak skeleton servants.

### Established story premise and progression

- The player explores the dungeon, survives enemies and traps, gathers items, and decodes a prophecy. Completing the cipher opens the final boss seal; defeating the final boss completes the run.
- Prophecy clues can be discovered through clue-carrying ghosts, puzzle rooms, statues/tablets, and cipher-locked Guardian Doors. Item letters supply the required cipher, creating a choice between combat strength and carrying the items needed to complete the seal.
- Guardian encounters are optional challenges with better rewards and cipher clues. The final boss is the required confrontation, gated by the completed cipher. The source document does not yet name that final boss or explain the protagonist's personal motive.
- Named Crypt encounters include the Old Gravekeeper Skeleton, a giant skeleton wielding a pickaxe, shovel, and lamp, and the Ancient Cult of Undead, whose leaders summon more undead until the leaders are defeated. These are Guardian concepts, not identified as the final boss.
- Other established inhabitants include Grave Zombies, Bone Hounds, Skeleton Archers, Skull Crabs, Shield Skeletons, Tombstone Skeletons, Timid Ghosts, Hostile Ghosts, and Necromancers.

### Connection to Rouge Snake and special soldiers

- The agreed future direction includes special soldiers associated with CypherRush story characters, alongside ordinary melee and ranged soldier types. Character names, biographies, recruitment events, and personal abilities have not yet been supplied in the available story notes.
- Current source separates combat type (`SoldierType`), combat settings and attack style (`SoldierStats`), and optional story identity (`Soldier::character_id`). This allows a future named character to use a combat archetype while retaining its own identity through movement, detachment, and rescue.
- The relationship between the squad's journey and CypherRush's prophecy, chronology, locations, and factions remains to be designed. Sharing characters does not yet establish a sequel, prequel, or a specific role for the leader.
- Cipher inventory, Guardian progression, spells, and the named enemy roster above are CypherRush reference material. Their adaptation to Rouge Snake requires separate gameplay design; the current grid-following squad, automatic attacks, rescue rules, and room transitions remain the implemented baseline.

### Story details still to define

- Named protagonist/leader and recruitable CypherRush characters.
- Their motives, relationships, and reason for entering the dungeon or joining the squad.
- The origin and meaning of the prophecy, ancient seals, and final antagonist.
- The squad's narrative objective, major story events, and ending scenes. The agreed true-ending unlock route is recorded below; its detailed story remains to be written.

## True-ending route (agreed design; not implemented)

This is the Rouge Snake squad route. It adds an encounter after the normal final boss; the earlier CypherRush reference describes that project's original run structure.

1. The player must meet all required story NPCs before entering the normal final boss room. The required NPC roster must be defined explicitly; ordinary recruited soldiers do not replace meeting those characters.
2. Before entering that room, the player must drop all follower nodes. Only the leader enters the boss room. Dropping followers for this entrance is a deliberate gameplay action, separate from lethal damage and casualty detachment.
3. The NPCs the player met appear inside the boss room as individual pickup soldiers. The encounter record must survive room changes so these characters can be spawned even though ordinary detached-soldier storage is cleared on transition.
4. Leader contact recruits each NPC into the squad, using the individual pickup/rescue interaction. Each contacted NPC joins at the end of the line; the others wait to be picked up.
5. Defeating the normal final boss with the full NPC squad surviving and remaining in the squad unlocks the true final boss. Meeting every required NPC, leader-only entry, and keeping the full recruited NPC squad are parts of the same unlock route.
6. Unlocking the true final boss is not itself recorded as completion of the true ending. The true final boss encounter and ending scenes still need design.

### Integration boundaries and remaining decisions

- Track met NPCs and the required boss-room roster by stable story identity, using `character_id` rather than vector index or squad length. Duplicate or ordinary soldiers must not substitute for a missing story NPC.
- Current source has no NPC encounter tracking, voluntary follower-drop action, final-boss entrance rule, NPC boss-room spawns, or true-final-boss unlock. These are future mechanics; this documentation update changes no gameplay code.
- Preserve the existing rule that unrescued detached soldiers are cleared when changing rooms. Boss-room story NPCs must be created from story encounter records, not by carrying the old room's detached collection into the new room.
- Still to decide: the required NPC names; what counts as meeting one; whether encounter progress is per-run or persistent; and whether dropped pre-boss followers are permanently lost or stored through a separate story rule.
- Still to decide: how the player drops followers; when and where NPC pickups appear; whether they are vulnerable before recruitment; and what the normal route does when true-ending requirements are missing.
- Still to decide: whether temporary detachment followed by rescue before boss defeat is allowed, or whether any casualty/detachment permanently fails the condition. The full NPC squad must remain at victory; precise failure tracking and retry rules need confirmation.
- The true final boss's identity, transition into its encounter, combat design, and final story resolution remain open.

## Agreed squad concept (supersedes Snake casualty rules)

- The player controls a leader, represented by the current head. Each following node represents an individual soldier in the squad.
- The last position in the line is not a special soldier type or role. All followers use their normal soldier/type sprite, including the last one; no dedicated tail sprite or tail-only behavior is required. The leader remains distinct.
- Reuse the existing continuous grid movement and following-path foundation. Soldier types, HP, protection, and combat state belong to individual soldiers and must not shift between soldiers when movement information advances.
- Attached soldiers prepare attacks automatically when a living enemy is detected, even if it is beyond attack reach. Detection controls when preparation starts; attack reach controls the strike destination. Soldier roles will support different attack styles, including melee and ranged attacks.
- Soldier combat data includes detection range, attack reach, damage, phase durations, and detached-combat capability. The implemented pattern strikes a single cell at the configured reach; additional weapon patterns and role definitions remain future work.
- Attack facing is a normalized eight-way grid direction separate from following directions. Preparation locks this direction; release computes the strike cell from the soldier's current position plus direction multiplied by attack reach. Soldiers continue following the column during combat.
- The existing leader/head death condition remains the baseline: leader HP reaching zero ends the run. The leader does not attack in the initial melee implementation; only followers and eligible detached soldiers do.

## Casualties and detached soldiers

- When an attached soldier dies, remove that soldier. All surviving soldiers behind it leave the active squad and remain on the map at their current positions; do not delete those survivors.
- Detached soldiers are stationary, individually rescuable units. They remain vulnerable to explicit enemy attacks and can be killed while waiting for rescue.
- Preserve each survivor's identity, type, remaining HP, and applicable runtime state when detaching. Death and detachment do not heal survivors.
- `can_attack_detached` is implemented per Soldier and defaults to false. Only previously recruited soldiers with this capability can attack while detached. Veteran enables it; Basic and Archer do not. All newly spawned recruits wait without attacking regardless of type.
- The capability flag and current attachment status are separate concepts. Attached soldiers use automatic combat; detached soldiers use it only when `has_joined_squad` is true, their type permits it, and normal attack requirements are met.
- Detached soldiers do not automatically form a moving replacement squad. The former rear portion of the line becomes individual soldiers waiting for rescue.
- Unrescued detached soldiers are lost when the squad successfully changes rooms. Clear their storage after the new room loads successfully; failed transitions retain them. Attached soldiers travel with the leader.

## Individual rescue

- Only leader/head contact with a detached soldier's cell triggers rescue. Contact from another attached soldier does not rescue anyone.
- Rescue appends only the touched soldier to the end of the active squad. Other detached soldiers remain on the map until individually rescued or killed.
- The rescued soldier initially occupies the current last squad member's cell. If only the leader remains, use the leader's cell. Copy that last member's following-position and direction information, then let the rescued soldier separate naturally on subsequent movement steps, as in the current growth placement.
- Rescue preserves the rescued soldier's type, remaining HP, and remaining damage-protection time. It is not a full-health growth operation: do not copy the last member's HP, protection, or combat state. Transfer the rescued soldier into the active squad without duplicating it.
- Placement at the end changes line order and following information, not soldier identity. The rescued soldier is rendered as its ordinary type and gains no special visual role from being last.
- Detachment, rescue, and room entry cancel Prepare or Attack and enter full recovery. Existing recovery retains its remaining time; HP and damage protection are preserved. Ready soldiers remain Ready. Fresh growth starts with fresh combat state.
- If several detached soldiers occupy one cell, rescue the first living soldier in storage order, at most once per leader movement step. Other soldiers wait for another leader contact.

## Squad implementation boundaries and open decisions

- Shared `Soldier` records now hold position, following directions, HP, and remaining protection. `Snake::body` stores attached soldiers; Game owns `detached_soldiers`. Basic, Veteran, and Archer types are implemented, including melee combat and traveling arrows. Additional roles and patterns remain future work.
- `DamageNode()` returns surviving rear soldiers when a follower dies. `DetachSoldier()` removes the casualty and rear portion from the active vector while preserving the returned survivors' records. Game and the test helper store those survivors.
- Detached soldiers are drawn at their preserved positions using the body sprite. `HandleSoldierRescue()` runs after a leader movement step, before enemy processing; `AttachSoldier()` preserves the recruit's HP/protection and copies only the last member's following information.
- Current attached drawing still has a dedicated `SNAKE_TAIL` branch. Replacing it with ordinary follower rendering remains deferred.
- Game coordinates damage against attached and detached soldiers. Enemies receive both collections as read-only observation inputs; they return attack cells without modifying either collection.
- `Soldier::UpdateCombat()` owns allied targeting and phase state, returning `SoldierAction` damage cells or projectile requests. Game resolves damage through `TakeDamage()` and owns projectile movement/collision.
- Initial melee ordering, targeting, moving attack origins, and leader participation are settled below. Additional weapon behavior and future overlap-policy refinements remain open.
- Random timed recruitment replaces food, score, and combo as described below. Healing, recruitment balance, and naming/asset changes remain future design work. Preserve the room-transition system.
- Rescue placement is implemented. Ordinary last-soldier rendering and additional soldier combat styles remain pending. No new manager or class hierarchy is required.

## Existing movement and HP baseline

- Each Snake node starts with 3 HP. Movement preserves each node's HP.
- Newly grown nodes start at full HP; entering another room preserves existing node HP and length. Restarting rebuilds the starting Snake with full HP.
- When a follower reaches zero HP or below, remove that casualty and transfer surviving rear soldiers to Game-owned detached storage.
- Starting length is a spawn setting, not a minimum surviving length. A Snake with only a healthy head can continue moving, fighting through surviving followers, and recruiting new soldiers.
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
- Enemy collection and sequential resolution are retained; soldier actions are collected before damage and resolved afterward as described below. Projectile release creates an arrow and later movement/collision applies damage; firing does not immediately damage its destination cell.

## Basic soldier melee attacks

- Every attached follower can attack automatically. Game skips the leader (index zero). Living detached soldiers attack only when `has_joined_squad` and `can_attack_detached` are both true; disabled detached soldiers still advance recovery timers.
- Current source tuning: detection range 2, attack reach 1, damage 1, Prepare 0.1 seconds, Attack 0.5 seconds, End 0.5 seconds. Detection uses square-grid distance, including diagonals. Attack remains a one-shot damage event on entry to Attack; its 0.5-second display does not repeatedly apply damage.
- Ready soldiers choose the nearest living enemy within detection range without requiring it to be within attack reach. Compare squared Euclidean distances; enemy vector order breaks ties. Exclude enemies on the soldier's own cell. Starting an attack requires positive damage and positive attack reach. No movement or contact causes damage.
- `Ready -> Prepare -> Attack -> End -> Ready` uses one phase timer reset on transition. Preparation locks a normalized eight-way direction without retaining an enemy pointer or index. At release, the strike cell is `current position + attack_direction * attack_reach`, and one `SoldierAction` is emitted. Only that destination cell is hit; intervening cells are not included. The released strike cell stays fixed during the Attack display.
- Detection and reach are independent: with detection 2 and reach 1, an enemy two cells away triggers preparation, but the soldier strikes one cell away. Movement may bring the strike into contact with an enemy; the attack may also miss. Increasing reach moves the endpoint without expanding detection. Unequal target offsets map to an eight-way direction rather than exact aiming at every possible target cell.
- `Soldier` owns combat settings/state and behavior in `src/soldier.cpp`. Movement copies only following data. Growth resets new combat state; detachment/rescue/room entry cancel unfinished attacks while preserving recovery as described above.
- In each gameplay update: preserve movement/hazard/transition/rescue handling; update enemies and collect enemy actions; update attached followers then detached soldiers and collect soldier actions; resolve enemy actions first, then soldier actions; finally clean up dead enemies. Soldier observation sees enemy positions after enemy movement.
- A released soldier action survives its attacker's death or detachment during same-tick enemy resolution. Immediate leader death still ends the run and discards remaining actions. An unreleased Prepare cannot deal damage after cancellation.
- Each soldier strike cell hits the first living enemy at that cell in enemy vector order. Multiple soldiers can damage the same enemy in one update; enemies have no damage-protection timer. Dead enemies are skipped and erased after resolution.
- Prepare draws a sky-blue cell outline at the prospective strike position; Attack draws a translucent sky-blue cell with a white outline at the released position. Game draws attack indicators after enemies, independently of node protection blinking. Sprite HP tint remains normal at 3+ HP, blue at 2 HP, red at 1 HP or below.
- There is no new combat manager or class hierarchy. Additional melee/projectile patterns, attack speed modifiers, and persistent effects remain later milestones.

## Current implementation and next milestone

- Basic Phase 4C enemy attacks are implemented: target detection, locked warning cells, delayed attack release, timed Attack and End phases, returned attack cells, collection and sequential resolution through `ProcessEnemyAttack()`, and immediate head-death handling.
- Node HP, HP-preserving movement, fresh-node HP on growth, body cutting, `Snake::IsDead()`, per-node damage protection, and blinking are implemented.
- Game checks Snake death before movement and after each attack-cell resolution. `DamageNode()` rejects nonpositive damage and damage to a dead Snake.
- Entry protection has been corrected in source: the counter advances while unfolding or protection remains active, and damage no longer waits for full unfolding. This has been source-reviewed; runtime verification for lengths one, two, three, and a longer Snake remains to be recorded.
- Contact damage is disabled in both directions: Game no longer invokes the old overlap damage interaction, and Enemy's `OnSnakeEnter()` is currently a no-op. Soldier strike cells now use Enemy HP through `TakeDamage()`.
- User verification: body cutting and yellow targeting indicator work; a clean rebuild picked up the 1.0-second enemy cooldown; multiple-enemy hits respect node protection; lethal head damage ends the run immediately.
- Shared-timer refactor verification: C++14 compile checks passed. A temporary executable using the production code passed checks for preparation delay, locked targets, one-shot damage release, recovery, retargeting on a later attack, stationary attack phases, and the dead-enemy guard. This does not substitute for visual playtesting.
- Random movement is restored in the current source. Visual playtesting remains for the full yellow-warning/red-strike sequence, movement resumption, and room transitions during preparation.
- Own-cell enemy targeting is excluded in source, as agreed. Visual attack-lifecycle verification and runtime overlap checks remain to be recorded.
- Squad detachment, Game-owned storage, detached drawing, and individual leader-contact rescue are implemented. The user verified that the detached rear portion draws correctly.
- Enemy observation and damage now include detached soldiers, preserving locked warning cells, sequential resolution, and one-victim-per-cell priority. Detached death removes only that soldier, with no second detachment operation or game over.
- Detached-damage verification: C++14 syntax checks passed for all source and test files. A temporary executable linked against production code passed nine scenarios covering detached observation and wind-up, attached targeting priority, own-cell/dead-target exclusion, simultaneous-hit protection and expiry, isolated detached death, overlap priority, casualty transfer followed by later damage, nonpositive damage/menu timers, and immediate leader death. Protection blinking and rescue during a warning still need visual playtesting. Existing signedness and hidden-base-Update warnings remain.
- Basic follower melee attacks and the detached-combat capability flag are implemented. The user verified that independent detection and reach better match the intended continuous-movement attack timing. Remaining visual checks include cancellation during rescue/detachment and the full combat lifecycle. Next milestone: verify the new random recruitment flow and tune capacity, spawn timing, and type weights.
- Soldier melee verification: C++14 syntax checks passed for all source/test files. A temporary executable using production code passed 15 scenarios covering target priority, diagonals, movement-relative release, one-shot damage/recovery, range gates, detached capability, cancellation, Game resolution and cleanup, released attacks surviving follower death, immediate leader death, growth/rescue/detachment state, following movement, room-entry cancellation, and missed strikes. Visual verification remains pending; existing compiler warnings remain.
- Detection/reach revision verification: the temporary regression executable passed 20 scenarios after adding checks for detection beyond reach, reach-independent endpoints, normalized directions, range/invalid-reach guards, and short attacks missing distant enemies. These checks preceded the user's latest phase-duration tuning; they are not a new verification of those tuned timings.

## Soldier types and Archer implementation

- `SoldierType` selects Basic, Veteran, or Archer defaults through `GetSoldierStats()`. `SoldierStats` contains configurable capabilities; HP, protection, recruitment history, phase, and timers belong to each individual soldier. Optional `character_id` carries story identity separately.
- Basic and Veteran use melee attacks; Veteran can fight after detachment once recruited. Archer has detection range 6, Prepare 0.2 seconds, Attack 0.08 seconds, recovery 0.7 seconds, arrow speed 10 grid steps/second, and arrow range 8 steps. Archer cannot attack while detached.
- Archers aim perpendicular to each soldier's local movement: left/right movement allows up/down fire, up/down movement allows left/right fire. Ignore targets directly ahead/behind on that movement axis. Lock the selected direction at preparation and release from the current position; arrows do not home.
- `SoldierAction` carries melee cells or projectile requests. Game collects either, resolves enemy damage first, advances existing arrows, then applies soldier releases. New arrows begin moving next fixed update. Each crossed cell is checked; the first living enemy is hit, walls/empty terrain and blocked diagonal corners stop arrows, gaps can be crossed, and range expiry removes arrows.
- Projectiles clear on successful room transition or restart; failed transitions retain them. They draw independently of soldier HP tint and protection blinking.
- Development keys B/V/R directly add Basic/Veteran/Archer followers and obey squad capacity. They are testing shortcuts, not the normal recruitment mechanic. Fresh growth creates the requested type and does not copy a previous soldier's story identity or abilities.

## Random recruitment replacing food and score

- Implemented milestone: active food/score/combo paths are removed, follower capacity is enforced, and Game schedules random recruit attempts. The HUD shows follower count and capacity. Legacy Food source/assets remain unused and may be removed in a later cleanup.
- Replace the active food pickup, immediate growth, score, combo, and multiplier systems with occasional waiting soldier spawns. No map spawn markers are required for random recruits. Healing remains deferred; recruiting never heals the squad.
- Initial tunable defaults: maximum 8 attached followers (leader excluded); one attempt every 5 gameplay seconds; base success chance 50%; no new spawn when 2 or more living soldiers already wait on the map. Initial type weights: Basic 60, Veteran 20, Archer 20. These are balancing defaults, not fixed story rules.
- The waiting limit counts both new recruits and detached survivors. It only blocks additional random spawns; detachment never deletes survivors to enforce this limit. A full squad also blocks random spawning.
- Consume an attempt even when chance fails, limits block spawning, no eligible cell exists, or all type weights are disabled. At most one spawn can occur per update; do not accumulate a burst of missed attempts. Pickup and death never reset the timer or force replacement. Timer freezes outside gameplay, resets on restart, and carries across successful room changes.
- Select an unoccupied safe ground cell, excluding leader/followers, living enemies, and living waiting soldiers. Do not spawn on gaps, walls, empty terrain, entrance, or exit. Select among eligible cells without an unbounded random retry loop.
- Spawn a fresh typed `Soldier` at its normal maximum HP with `has_joined_squad = false`. New recruits cannot attack, including Veterans, but enemies can observe, attack, and kill them. They have no expiry timer. Remaining recruits and detached soldiers clear on successful room changes as already agreed.
- Only leader contact recruits one living soldier per movement step. Require an available follower slot; if full, leave the soldier waiting, with no healing, replacement, consumption, or disappearance. Attach at the last follower's cell and preserve the recruit's HP, protection, type, and identity.
- Set `has_joined_squad = true` on successful recruitment, initial squad creation, or development growth. Preserve it through movement and detachment. Combat eligibility is `attached || (has_joined_squad && stats.can_attack_detached)`; previous Veterans can fight after detachment, newly spawned Veterans cannot.
- Snake owns capacity checks for both attachment and direct growth; Game owns spawn timing, population checks, placement, and type selection. Reuse `detached_soldiers` for all waiting soldiers; no new manager is required.
- Centralize chance calculation as clamped `base_chance * terrain_multiplier * skill_multiplier`. Ground initially supplies multiplier 1. Terrain-specific bonuses and timed/cast leader skills are future work; the multiplier input is an extension point and cannot bypass population limits.
- True-ending NPC spawning remains a separate future story mechanic. Its required roster and squad-capacity policy must be reconciled when that encounter is implemented.
- Verification: all source/test files passed C++14 syntax checks. A temporary executable using production code passed 71 regression scenarios, including 18 recruitment/capacity scenarios plus the previous soldier/projectile cases with recruitment history made explicit. Checks include passive new Veterans, enemy damage/death, no expiry, full-squad contact, subsequent rescue preserving HP/protection, detached Veteran combat, timer/chance/population limits, safe placement, type selection, bonus clamping, pause/transition/restart timing, and zero-capacity initialization. Visual playtesting and balance tuning remain pending.

## Deferred fixes and future improvements

- Build: Makefile does not track header dependencies, so changing values in headers may leave stale object files. A clean or forced rebuild is currently needed; add automatic header dependency tracking later.
- Growth protection reset is now implemented in source; manual verification of growing from a protected tail remains to be recorded.
- Enemy own-cell exclusion applies to both attached and detached targets. Initial overlap priority is deterministic; broader targeting policies may be revisited later.
- Detached storage is cleared on restart and after a successful room load in `Transition()`. Manual verification of successful and failed transitions with detached soldiers remains to be recorded.
- Cleanup: remove unused enemy `ghost_phase` and obsolete contact hooks if no longer needed. Enemy's parameterized `Update()` is not an override of `GameObject::Update()`; revisit the base interface if polymorphic updates become necessary.
- Future features: simultaneous damage aggregation, prediction helpers, more complex attack states, additional projectile behaviors, and larger attack patterns. No new managers or external dependencies are planned.
