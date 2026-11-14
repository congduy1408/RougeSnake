# Rouge Snake — Game Design and Implementation

Updated: 2026-11-14 (Asia/Ho_Chi_Minh). This is the current squad-game reference. Implemented behavior, prototype tuning, future plans, and historical verification are distinguished below. Source code remains authoritative for implementation details.

We are continuing development of Rouge Snake.

Read AGENTS.md and GAME_DESIGN.md, then inspect the current source code to understand the implementation.

We will follow a design-first workflow: discuss a feature, agree on its structure, I implement it, and you review my changes.

Do not modify any source files unless I explicitly request it.

The implemented prototype includes squad combat, detachment/rescue, random recruitment, healing consumables, limited leader-owned inventory, fourteen filler items, seven spells, three auras, menus, and a separate native remote-play companion. Food, score, and combo are inactive. Enemy variety, full cipher progression, boss gates, and victory content are future work.

Development follows the design-first workflow above; implementation status and agreed next steps are recorded below.

## CypherRush setting and story reference

Source: the user-supplied [CypherRush design document](/Users/dcd/Downloads/cypherrush_gdd_current.md), especially High Concept, Guardian / Boss Structure, Prophecy and Clues, HUD / UI, item catalogs, and Crypt Enemy Details. This local Downloads reference may be unavailable on another device; the agreed Rouge Snake rules are recorded here. CypherRush is a Unity/WebGL single-character design; Rouge Snake adapts selected concepts to C++/raylib continuous grid squad play, not its jumping/physics or full architecture.

### Setting

- CypherRush is a roguelike dungeon crawler built around combat, exploration, and deciphering a prophecy.
- Its first environment is an underground Crypt: bones, rotting corpses, grave soil, cursed tombs, and ancient sealed doors. Its inhabitants primarily belong to bone and undead enemy families.
- The setting's supernatural themes include holy power, darkness, the void, blood, and grave/bone magic. Forbidden necromancy is represented by Grave Bloom, a grave-rite acquired from a defeated Necromancer that temporarily summons weak skeleton servants.

### Established story premise and progression

- The player explores the dungeon, survives enemies and traps, gathers items, and decodes a prophecy. Completing the cipher opens the final boss seal; defeating the final boss completes the run.
- Prophecy clues can be discovered through clue-carrying ghosts, puzzle rooms, statues/tablets, and cipher-locked Guardian Doors. Item letters supply the required cipher, creating a choice between combat strength and carrying the items needed to complete the seal.
- Guardian encounters are optional challenges with better rewards and cipher clues. The final boss is the required confrontation, gated by the completed cipher. The CypherRush prototype names a Test Gravekeeper boss; its final story antagonist and the protagonist's personal motive remain undefined.
- Named Crypt encounters include the Old Gravekeeper Skeleton, a giant skeleton wielding a pickaxe, shovel, and lamp, and the Ancient Cult of Undead, whose leaders summon more undead until the leaders are defeated. These are Guardian concepts, not identified as the final boss.
- Other established inhabitants include Grave Zombies, Bone Hounds, Skeleton Archers, Skull Crabs, Shield Skeletons, Tombstone Skeletons, Timid Ghosts, Hostile Ghosts, and Necromancers.

### Connection to Rouge Snake and special soldiers

- The agreed future direction includes special soldiers associated with CypherRush story characters, alongside ordinary melee and ranged soldier types. Character names, biographies, recruitment events, and personal abilities have not yet been supplied in the available story notes.
- Current source separates combat type (`SoldierType`), combat settings and attack style (`SoldierStats`), and optional story identity (`Soldier::character_id`). This allows a future named character to use a combat archetype while retaining its own identity through movement, detachment, and rescue.
- The relationship between the squad's journey and CypherRush's prophecy, chronology, locations, and factions remains to be designed. Sharing characters does not yet establish a sequel, prequel, or a specific role for the leader.
- Leader inventory/cipher matching and the spell/aura roster below are adapted and implemented. Prophecy clues, Guardian progression, named enemy variants, boss gating, and story encounters remain reference material or future design. The existing squad movement, automatic follower combat, rescue, and room-transition rules remain in force.

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
- Voluntary single-follower detachment is implemented in the Squad menu. NPC encounter tracking, boss entrance enforcement, boss-room NPC spawning, and true-boss unlocking are not implemented. General detachment does not itself establish or validate the story route.
- Preserve the existing rule that unrescued detached soldiers are cleared when changing rooms. Boss-room story NPCs must be created from story encounter records, not by carrying the old room's detached collection into the new room.
- Still to decide: the required NPC names; what counts as meeting one; whether encounter progress is per-run or persistent; and whether dropped pre-boss followers are permanently lost or stored through a separate story rule.
- Followers can be detached individually through the menu; a dedicated drop-all/boss-entry flow is not implemented. Still to decide: when/where NPC pickups appear, their vulnerability before recruitment, and the normal route when true-ending requirements are missing.
- Still to decide: whether temporary detachment followed by rescue before boss defeat is allowed, or whether any casualty/detachment permanently fails the condition. The full NPC squad must remain at victory; precise failure tracking and retry rules need confirmation.
- The true final boss's identity, transition into its encounter, combat design, and final story resolution remain open.

## Agreed squad concept (supersedes Snake casualty rules)

- The player controls a leader, represented by the current head. Each following node represents an individual soldier in the squad.
- Design intent: the last position is not a special soldier type/role; followers should retain ordinary type appearance. Current source still has a dedicated SNAKE_TAIL rendering branch, so that visual cleanup remains deferred. The leader remains distinct.
- Reuse the existing continuous grid movement and following-path foundation. Soldier types, HP, protection, and combat state belong to individual soldiers and must not shift between soldiers when movement information advances.
- Attached soldiers prepare attacks automatically when a living enemy is detected, even if it is beyond attack reach. Detection controls when preparation starts; attack reach controls the strike destination. Soldier roles will support different attack styles, including melee and ranged attacks.
- Soldier combat data includes detection range, attack reach, damage, phase durations, and detached-combat capability. The implemented pattern strikes a single cell at the configured reach; additional weapon patterns and role definitions remain future work.
- Attack facing is a normalized eight-way grid direction separate from following directions. Preparation locks this direction; release computes the strike cell from the soldier's current position plus direction multiplied by attack reach. Soldiers continue following the column during combat.
- Leader HP reaching zero ends the run. The leader does not perform automatic follower melee; the player manually activates leader-owned spell items with Space. Followers and eligible detached soldiers retain automatic combat.

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
- Random recruitment replaces food, score and combo. Healing Shards are implemented as separate consumables. Recruitment never heals. Recruitment balance and naming/asset refinement remain future work. Preserve the room-transition system.
- Rescue placement is implemented. Ordinary last-soldier rendering and additional soldier combat styles remain pending. No new manager or class hierarchy is required.

## Existing movement and HP baseline

- Each Snake node starts with 3 HP. Movement preserves each node's HP.
- Newly grown nodes start at full HP; entering another room preserves existing node HP and length. Restarting rebuilds the starting Snake with full HP.
- When a follower reaches zero HP or below, remove that casualty and transfer surviving rear soldiers to Game-owned detached storage.
- Starting length is a spawn setting, not a minimum surviving length. A squad with only a healthy leader can continue moving, using carried spells, and recruiting new followers.
- Head HP reaching zero or below ends the run. Keep the head node in the body; do not remove it through tail cutting.
- Snake reports its death through `IsDead()`. Game switches to `GAMEOVER` and stops gameplay processing when death is detected.
- Damage is caused by explicit attacks. Contact or movement overlap alone damages neither the Snake nor enemies. Terrain hazards remain lethal independently of node HP or attack protection.

## Agreed enemy behavior and attack architecture

- Enemy owns its AI, movement, movement and attack timers, and attack selection. Movement remains terrain-checked inside Enemy; Game does not execute returned movement destinations.
- Game calls each living enemy's `Update()` every fixed update with elapsed time, `const Board&`, `const Snake&`, and `const std::vector<Soldier>&` for detached soldiers. An update is an opportunity to observe and decide, not a requirement to attack.
- Observation and behavior selection initially belong in one AI function. Read actual squad positions/movement; helpers or prediction may be introduced for specific enemy designs. Leader item activation exists; a general dodge action does not.
- Complex enemies may prepare, release, and recover from attacks across multiple updates. Their targeting and preparation rules belong to the enemy's behavior.
- The update result describes an attack released during that update: whether an attack occurred, affected grid cells, and damage. A waiting or moving enemy can return no attack. A movement flag or destination is unnecessary for Game under the agreed no-contact-damage rule.
- Attack cells describe the released attack in board coordinates. The current enemy locks its target cells when preparation begins; Game resolves those cells without asking the enemy to choose a target again at release.
- The current simple enemy scans attached soldiers from head to tail, then detached soldiers in storage order, and targets the first living soldier inside a centered 3x3 square (`observation_radius = 1`), excluding its own cell. Diagonal cells are valid. An overlapping node is skipped so another neighboring node can still be selected; contact itself causes no damage.
- The current attack deals 1 damage to the selected cell on entry to Attack. Timings are 0.5 seconds Prepare, 0.08 seconds Attack, and 1.0 second End (recovery). There is no additional cooldown timer.
- Ready enemies observe and begin preparation immediately when a target is available; otherwise they move randomly with terrain checks. Enemies do not move or retarget during Prepare, Attack, or End.
- Prepare displays the locked cells in yellow at 50% opacity; Attack displays the released cells in red at 50% opacity. Drawing reads the attack phase, not a separate flash countdown, and never repeatedly applies damage while the zone is visible.

## Current update order

- Read gameplay input (including leader item use) before fixed updates. Menus return before simulation.
- During a fixed update: check leader death, advance world-spell/item timers, move squad, advance protection/animation, resolve terrain hazards, exits, and leader rescue/shard/item pickups.
- Successful room transition returns before old-room combat processing.
- Prepare an enemy-only collision board with temporary barriers; apply Grave Shade and update enemies, then collect attached/detached soldier actions. Hollow Night modifies damage at release using the attacking member's own HP.
- Resolve enemy actions first, stopping immediately on leader death; advance existing projectiles through every crossed cell; apply newly released follower actions; update Grave Bloom attacks; clean up dead enemies; attempt recruitment.
- Leader direct spells are triggered by input before this loop. Spell effects use their explicitly documented targeting/timing rather than inheriting follower wind-up phases.

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
- No new combat manager or class hierarchy is used. Additional follower attack patterns and attack-speed modifiers remain future work; persistent leader spell effects now exist separately as WorldSpell records.

## Historical combat milestones and verification

These entries preserve earlier reviews and tests; they are not a claim that every scenario was rerun after the latest changes. Current verification and next steps are recorded near the end of this document.

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
- Archers refresh live target positions within their six-cell detection square on each combat update, in enemy-vector order. Exclude dead, out-of-range, and own-cell enemies. Select the first eligible enemy, not the nearest and not a persistent detection-arrival queue. During preparation, update aim toward that first enemy's current position from the Archer's current position. If the first target leaves/dies, use the next; if none remain, cancel preparation into recovery. At release, arrows lock their normalized eight-way direction and do not home. There is no perpendicular-to-movement restriction.
- `SoldierAction` carries melee cells or projectile requests. Game collects either, resolves enemy damage first, advances existing arrows, then applies soldier releases. New arrows begin moving next fixed update. Each crossed cell is checked; the first living enemy is hit, walls/empty terrain and blocked diagonal corners stop arrows, gaps can be crossed, and range expiry removes arrows.
- Projectiles clear on successful room transition or restart; failed transitions retain them. They draw independently of soldier HP tint and protection blinking.
- Development keys K/L/O directly add Basic/Veteran/Archer followers and obey squad capacity. They are testing shortcuts, not the normal recruitment mechanic. Fresh growth creates the requested type and does not copy a previous soldier's story identity or abilities.

## Random recruitment replacing food and score

- Implemented milestone: active food/score/combo paths are removed, follower capacity is enforced, and Game schedules random recruit attempts. The HUD shows follower count and capacity. Legacy Food source/assets remain unused and may be removed in a later cleanup.
- Replace the active food pickup, immediate growth, score, combo, and multiplier systems with occasional waiting soldier spawns. No map spawn markers are required for random recruits. Healing uses separate shards; recruiting never heals the squad.
- Current prototype test cap: maximum 20 attached followers plus the leader (raised from the original eight to test menu scrolling, not final balance); one attempt every 5 gameplay seconds; base success chance 50%; no new spawn when 2 or more living soldiers already wait on the map. Initial type weights: Basic 60, Veteran 20, Archer 20. These are balancing defaults, not fixed story rules.
- The waiting limit counts both new recruits and detached survivors. It only blocks additional random spawns; detachment never deletes survivors to enforce this limit. A full squad also blocks random spawning.
- Consume an attempt even when chance fails, limits block spawning, no eligible cell exists, or all type weights are disabled. At most one spawn can occur per update; do not accumulate a burst of missed attempts. Pickup and death never reset the timer or force replacement. Timer freezes outside gameplay, resets on restart, and carries across successful room changes.
- Select an unoccupied safe ground cell, excluding leader/followers, living enemies, living waiting soldiers, active healing shard, ground items, temporary barriers, and Grave Bloom. Do not spawn on gaps, walls, empty terrain, entrance, or exit. Select among eligible cells without an unbounded random retry loop.
- Spawn a fresh typed `Soldier` at its normal maximum HP with `has_joined_squad = false`. New recruits cannot attack, including Veterans, but enemies can observe, attack, and kill them. They have no expiry timer. Remaining recruits and detached soldiers clear on successful room changes as already agreed.
- Only leader contact recruits one living soldier per movement step. Require an available follower slot; if full, leave the soldier waiting, with no healing, replacement, consumption, or disappearance. Attach at the last follower's cell and preserve the recruit's HP, protection, type, and identity.
- Set `has_joined_squad = true` on successful recruitment, initial squad creation, or development growth. Preserve it through movement and detachment. Combat eligibility is `attached || (has_joined_squad && stats.can_attack_detached)`; previous Veterans can fight after detachment, newly spawned Veterans cannot.
- Snake owns capacity checks for both attachment and direct growth; Game owns spawn timing, population checks, placement, and type selection. Reuse `detached_soldiers` for all waiting soldiers; no new manager is required.
- Centralize chance calculation as clamped `base_chance * terrain_multiplier * skill_multiplier`. Ground initially supplies multiplier 1. Terrain-specific bonuses and timed/cast leader skills are future work; the multiplier input is an extension point and cannot bypass population limits.
- True-ending NPC spawning remains a separate future story mechanic. Its required roster and squad-capacity policy must be reconciled when that encounter is implemented.
- Verification: all source/test files passed C++14 syntax checks. A temporary executable using production code passed 71 regression scenarios, including 18 recruitment/capacity scenarios plus the previous soldier/projectile cases with recruitment history made explicit. Checks include passive new Veterans, enemy damage/death, no expiry, full-squad contact, subsequent rescue preserving HP/protection, detached Veteran combat, timer/chance/population limits, safe placement, type selection, bonus clamping, pause/transition/restart timing, and zero-capacity initialization. Visual playtesting and balance tuning remain pending.

## Leader inventory and cipher rules (implemented foundation)

- The leader owns and uses combat items. Effects may affect the leader, squad, enemies, or environment; followers continue their independent automatic attacks. No regular weapon item effect is implemented yet.
- Inventory capacity is **cipher length + 2**. The current test cipher is **STAR**, giving six slots; this is prototype data, not a fixed permanent inventory size.
- Each carried item contributes one cipher letter. Cipher matching is order-independent and consumes each matching item at most once: MOON requires two separate O-bearing items. Letters are derived from carried inventory, not stored in an unlimited separate collection.
- Cipher progress updates immediately when items are collected/replaced. Completing the displayed cipher does not yet unlock a boss or victory; clues, changing extraction rules, and actual seal checks remain future work.
- A/D selects previous/next item with wraparound. Space activates the selected usable item only if ready and the leader is alive in gameplay. Paused menus do not activate spells. Filler items and auras cannot be activated with Space.
- Cooldown remaining belongs to each item instance. Switching selection does not reset or bypass it; two separately carried copies have independent cooldowns. Carried and dropped cooldowns advance only during gameplay, pause in menus, and carried instances retain cooldowns across rooms.
- Leader contact collects at most one ground item per movement step. With free capacity, append the pickup and keep the existing selection; the first pickup selects slot zero automatically. Followers do not collect items.
- At full capacity, collecting replaces/equips into the selected slot. A replaced spell/filler is dropped at the pickup cell, retaining instance state, and is ineligible until the leader leaves that cell and returns. This prevents immediate swap-back. A replaced aura is destroyed instead, with no ground pickup or remnant.
- Uncollected/dropped items clear on successful room transition; carried inventory persists. Failed transitions that return before room replacement retain pickups. Restart clears inventory and selection.
- Healing Shards use separate shared consumable storage and do not occupy limited item slots or contribute cipher letters.
- `Item` stores stable ID, displayed name, category (Filler/Weapon/Spell/Aura), cipher letter, description, use effect, cooldown duration and remaining cooldown. Catalog entries create independent instances; inventory changes never mutate the catalog. `GroundItem` adds position and pickup reentry protection.
- Descriptions contain flavor/effect text without repeating the cipher letter. The letter has its own UI field. `TestSignal` remains a diagnostic-only effect (activation message and cooldown) and is not in normal filler/spell/aura catalogs.
- Normal reward-room/enemy item spawning and automatic item pools are not implemented. J/U/P place developer pickups on safe ground for testing; they are not final acquisition rules.

### Filler catalog (all fourteen implemented)

All fillers are passive, occupy a slot and contribute a letter. They have no usable action/cooldown. Cryptbound Book's future clue feature is explicitly labeled unavailable.

| Item | Letter | Current behavior |
| --- | --- | --- |
| Ashen Talisman | A | Passive cipher filler |
| Bone Pebble | B | Passive cipher filler |
| Cryptbound Book | C | Passive; clue reveal deferred |
| Dustvein Amulet | D | Passive cipher filler |
| Elder Charm | E | Passive cipher filler |
| Forgotten Ring | F | Passive cipher filler |
| Moonlit Talisman | M | Passive cipher filler |
| Nightgrave Pebble | N | Passive cipher filler |
| Ossuary Book | O | Passive cipher filler |
| Pale Amulet | P | Passive cipher filler |
| Quiet Charm | Q | Passive cipher filler |
| Rusted Ring | R | Passive cipher filler |
| Silent Talisman | S | Passive cipher filler |
| Tomb Pebble | T | Passive cipher filler |

CypherRush weapons Dash Dagger (D), Counter Shield (C), Bouncing Bow (B), Stone Spear (S), and Heavy Hammer (H) remain reference concepts requiring squad-specific design. The proposed Stone Spear tuning was not implemented. Saint's Halo is classified as a spell in the current roster, regardless of its original aura category.

## Healing Shards (implemented)

- Start a run with zero collected shards. On each room visit, attempt to place exactly one shard on uniformly selected safe, unoccupied ground after squad/enemy placement. If no eligible cell exists, skip the spawn rather than retrying indefinitely.
- Placement excludes attached soldiers, living enemies, living waiting soldiers, ground items, barriers/Bloom, entrances, exits, walls, gaps and empty terrain. The same bounded safe-cell scan is reused for recruit/debug item placement.
- The shard is a mint diamond with a dark cross. Leader contact collects it once into shared supply; followers cannot collect it. Pickup does not immediately heal. There is no timed respawn within that room visit.
- One shared shard restores 1 HP to the selected living leader/follower, capped at that member's maximum. Dawn Veil changes that to up to 2 HP; duplicate auras do not increase it further. Full-health, dead, invalid, or no-supply targets consume nothing. Healing does not change identity, combat phase, shield ownership or protection.
- Collected supply persists across rooms. Uncollected room shards disappear on successful transition, with a new spawn attempt in the next room. Restart clears supply. Re-entering a template creates another per-visit opportunity; unique-room tracking/farming prevention is not implemented.
- Consumables are separate from equipment/cipher slots. The HUD shows HEAL xN; Items has a Consumables section; the Squad menu performs healing on a selected member.

## Main menu, Game Over and gameplay HUD

- Main menu/Game Over use a dark grid-backed squad theme with a small leader/follower motif. Start/Retry accepts Enter or a mouse click. Main menu explains arrow movement, leader-contact recruitment, automatic follower attacks, and lethal terrain.
- Restart enters gameplay only when InitGameObject succeeds. Failure returns to Game Over, logs failure, displays a visible error, and permits another retry rather than allowing play with partial initialization.
- Game window is **640 x 560**: an **80-pixel top HUD** above the unchanged **640 x 480 playfield** (40 x 30 cells at 16 pixels). Camera2D offsets only room drawing; movement/collision stay in room-grid coordinates.
- HUD shows attached follower count/capacity and Basic/Veteran/Archer totals, excluding leader/waiting soldiers. It shows live cipher matches, carried item letters/selection, selected item cooldown or passive state, and separate HEAL supply.
- Do not duplicate squad HP in the gameplay HUD; sprite tints/blinking indicate HP/protection. HP is shown in the management menu where the player chooses healing targets. Attached shield charges have a visible ring.
- Item HUD shows up to six carried entries around the current selection and the current short test cipher. Large/dynamic cipher presentation needs future UI work; inventory itself uses cipher length + 2.

## Paused expedition menu and voluntary detachment

- Tab or Esc during gameplay opens PAUSE_MENU. Movement, recruitment, combat phases, protection, projectiles, enemy slows, spell lifetimes and item cooldowns pause. The game remains drawn behind the overlay.
- Q/W switches tabs left/right, wrapping through Squad, Items, Enemies and Boss Travel.
- Squad tab currently uses a vertical selectable list. Leader is selectable for healing/info but never detachable. Up/Down selects members; Enter opens actions; Up/Down chooses Detach/Heal; Enter heals or requests detach confirmation, then Enter confirms. Esc cancels confirmation, backs out of actions, then resumes. Tab resumes directly.
- Eight member rows, including leader, are visible at once; the list scrolls to keep the selected member visible. The 20-follower test cap was user-tested beyond window capacity. A horizontal soldier-chain presentation was discussed but is not implemented; keep the current list unless the user approves a redesign.
- Voluntary Detach removes only the selected follower, alive at the same map cell with HP, type, identity, protection, recruitment history, and shield preserved. It cancels unfinished combat and places the soldier in waiting storage for normal rescue. Followers behind that member remain attached. This differs from death, which removes the casualty and detaches surviving rear soldiers.
- Items tab scrolls six entries, shows selected item name, separate letter field, wrapped description and passive/cooldown state. Up/Down inspects, Enter equips; item use happens after resuming. Healing Shards are listed separately under Consumables.
- Enemies and Boss Travel are clearly labeled placeholders. They do not yet track encounters, discovered doors, lock status, teleport destinations, or boss progression.
- Native Esc-to-exit is disabled so it can operate menus. Esc quits only from Main Menu/Game Over; the window close button still exits.

### Controls

| Context | Input | Action |
| --- | --- | --- |
| Main menu / Game Over | Enter or mouse | Start / retry |
| Main menu / Game Over | Esc | Quit |
| Gameplay | Arrow keys | Steer the continuously moving leader |
| Gameplay | A / D | Select previous / next item |
| Gameplay | Space | Use equipped active item when ready |
| Gameplay | Tab / Esc | Open paused menu |
| Paused menu | Q / W | Previous / next tab |
| Squad / Items list | Up / Down | Select member / inspect item |
| Squad | Enter, Up / Down, Enter | Open/select/execute action; detach requires confirmation |
| Items | Enter | Equip inspected item |
| Paused menu | Esc / Tab | Back or resume / resume directly |
| Development | I | Damage attached member index 2 |
| Development | J | Cycle fourteen filler pickups |
| Development | U | Cycle Dawn Veil, Grave Shade, Hollow Night pickups |
| Development | P | Cycle spells in the order in the table below |
| Development | K / L / O | Add Basic / Veteran / Archer follower, respecting capacity |

## Spell and aura roster (prototype implementation)

The following adapts CypherRush to Rouge Snake. Air Bounce, Void Echo, Rift Mark, Blood Oath, Bone Mantle, and Shadow Blink are removed. Saint's Halo is a spell, not an aura.

| Spell | Letter | Implemented behavior | Cooldown |
| --- | --- | --- | --- |
| Fireball | F | Forward projectile, 1 damage, 20 cells/s, 8-cell range, first enemy hit | 2 s |
| Saint's Halo | S | Give each living attached soldier, including leader, one attack-blocking charge; never stack charges | 180 s |
| Ice Spike | I | Forward projectile, 1 damage, 20 cells/s, 8-cell range; first hit slows movement 50% for 2 s, without slowing attacks | 3 s |
| Lightning Chain | L | Detect within 6 cells, strike nearest enemy, then up to 2 nearest unhit enemies within 3 cells of previous target; 1 damage each | 3 s |
| Water Wall | W | Three-cell-wide transverse barrier centered 5 cells ahead, lasts 5 s; enemies/projectiles blocked, squad passes through | 8 s |
| Earth Pillar | E | Strike nearest enemy within 6 cells for 1 damage; pillar at its cell blocks enemies/projectiles for 3 s, squad passes through | 5 s |
| Grave Bloom | G | One stationary bloom on random safe ground within 3 cells of any attached member; lasts 20 s, detects nearest enemy within 4 cells and deals 1 damage each second | 15 s |

Range checks use grid (Chebyshev) radius, including diagonals: Lightning initially detects in a 13 x 13 square centered on the leader (radius 6), then each jump searches a 7 x 7 square around the last enemy (radius 3). An offset of three horizontal and three vertical cells is within jump range. Squared Euclidean distance determines nearest-target ordering and enemy-vector order breaks ties. Lightning is an instantaneous beam sequence with a brief visible detection square and connecting lines; it hits each enemy once. It does not launch a homing projectile. Grave Bloom directly attacks from the bloom, not by spawning skeleton units; casting a new valid bloom replaces the old one. These are initial prototype choices for visual review and tuning.

Water Wall skips invalid/occupied cells and may be shorter than three cells near obstacles. Earth Pillar requires safe ground under its target with no squad/pickup overlap. Doors, entrances, gaps, and permanent walls are never overwritten. No-target/invalid-placement casts do not consume cooldown. World spell timers, enemy slows and carried/dropped item cooldowns pause in the expedition menu. World effects clear only on successful room transition/restart; carried cooldowns and soldier-owned shields persist across transitions.

Halo charges are stored per soldier. A valid attack consumes a charge instead of HP, then starts ordinary hit protection. Existing room-entry/hit protection prevents shield consumption while already protected. Detachment/rescue preserve remaining charge; new recruits do not inherit another soldier's shield. Recasting gives missing charges only; when all attached soldiers are charged, there is no effect/cooldown consumption. It does not protect against lethal terrain.

| Aura | Letter | Implemented behavior |
| --- | --- | --- |
| Dawn Veil | D | One shard heals up to 2 HP rather than 1, capped at the selected living member's maximum |
| Grave Shade | G | Attached soldiers reduce enemy detection by 1 cell, minimum radius 1; waiting soldiers are unaffected |
| Hollow Night | H | An attacker at exactly 1 HP gains +1 damage on soldier melee/arrows and leader direct damage spells |

Auras are passive while carried, regardless of selected item. Identical effects do not stack; copies occupy separate slots and contribute separate cipher letters. Auras have no lifetime or remnants. Replacing/dropping an aura destroys it permanently and removes its effect/cipher credit; another carried copy keeps the effect active. Fresh ground aura pickups remain collectible. Normal spell/filler drops can be recollected.

Grave Shade cannot shrink the current basic enemy's radius of 1 further. Grave Bloom's independent attacks deal a fixed 1 damage and have no HP-based Hollow Night bonus. Dropping an aura does not retroactively change damage already stored in a released projectile. Temporary barriers block friendly projectiles too; hostile projectiles do not exist yet.

Development testing: U cycles Dawn Veil, Grave Shade, Hollow Night pickups; P cycles the spell table in order; J cycles fillers. Initial tuning is in src/item.cpp and src/spell.cpp. Release/Debug builds and six standalone item/gameplay test executables passed. Visual verification and balance testing of this expanded batch remain pending.

## Development, builds and native remote play

- C++14/raylib, design-first workflow. Source is in src/, headers in include/, maps in map/, sprites in sprite/. The active project is currently Untitled/ under the repository root.
- macOS setup uses Apple Command Line Tools/clang++ and Homebrew raylib. Release: `make -j4`; Debug: `make -j4 BUILD_MODE=DEBUG PROJECT_NAME=RougeSnakeDebug`. From the parent repository root, add `-C Untitled`. Run with Untitled as working directory so relative map/sprite paths resolve.
- Makefile has native macOS linker flags and Homebrew include/library discovery. Objects are separated under obj/OS/BUILD_MODE so copied Windows objects cannot contaminate Mac builds. Automatic header dependency tracking uses -MMD/-MP. VS Code has platform-specific tasks, IntelliSense, and a macOS LLDB configuration; actual debugger launch still needs verification.
- Existing Windows raylib/w64devkit paths must match that machine's installation. The latest gameplay changes have not been built/tested on Windows in this session. Desktop builds are not proof of browser or cross-platform equivalence.
- .gitignore excludes object/dependency files, executables, libraries, debug symbols, build directories, packaged RougeSnake.zip and macOS metadata. Previously generated object files/executables were cleaned earlier; later builds regenerate ignored local outputs. Source/assets/docs remain available for commits. Ignore rules do not remove already tracked files from Git history.
- Use Git branches/commits to transfer code between devices and read HANDOFF.md when starting a new chat. Resolve the existing root-to-Untitled migration deliberately; do not blindly restore old root files or stage every generated export. Chat continuity is separate from file synchronization.

### Remote-play companion (separate from gameplay/build)

- [Remote-play setup](../tools/remote-play/README.md) documents the Mac-only companion in tools/remote-play/. It captures only the configured native game's window via ScreenCaptureKit, serves JPEG frames through a Python HTTP server, and forwards allowlisted keys to that game process. No VNC, WebAssembly game port, audio, or Windows capture host is used.
- Browser clients on phones/PCs have keyboard and touch buttons. It is the native Mac binary being tested; video compression/network latency can change perceived responsiveness and do not establish native Windows behavior.
- Server defaults: 127.0.0.1:8787, up to 20 captured frames/second, a fresh connection token and one controlling session. Held keys release on blur/disconnect or watchdog expiry. Disconnect does not pause or stop the native game: pause with Tab first.
- Tailscale Serve proxies this privately to the tailnet; Funnel/public exposure is not required. The Mac must be awake, signed in and unlocked for the current setup, with Screen Recording and Accessibility granted to the helper. Capture/control while locked/minimized or without permissions is not guaranteed.
- Start from repository root with `sh tools/remote-play/run.sh --launch-game` (omit launch flag if already open). Changes to the game require relaunching the native executable; changed remote key mapping/helper code requires restarting/rebuilding the companion.
- Native helper compilation, six server/control tests, and phone/desktop page layout review passed previously. Actual end-to-end capture/control over Tailscale remained permission/setup-dependent; no later user report established that it was fully verified. Do not infer this from the gameplay tests.

## Current verification, content roadmap and next milestone

### Verification recorded so far

- User confirmed the top HUD looks correct, inventory selection/pickup/swaps work, expanded squad scrolling works, and Fireball works after increasing its speed to 20 cells/second.
- Recent macOS Release and Debug builds passed. Six persistent standalone C++ tests are available in tests/: filler_items_test.cpp, aura_items_test.cpp, aura_gameplay_test.cpp, fireball_gameplay_test.cpp, spell_catalog_gameplay_test.cpp, and enemy_spell_effects_test.cpp. They compile against real item/game code and are excluded from the game's main build.
- Their most recent pass covered filler catalog/passivity, duplicate cipher matching, Dawn Veil/nonstacking/removal, Fireball movement/collision/cooldowns, Halo shield ownership/consumption, Lightning nearest-first jumps, temporary barriers, Bloom attacks, pause/transition/restart behavior, enemy slow timing, and Grave Shade detection limits.
- Historical temporary tests (including the earlier recorded 71 recruitment/combat scenarios) are separate evidence. They were not all rerun as a single current suite, may no longer exist in /tmp, and do not substitute for visual playtests.
- New expanded spell/aura effects still need user visual verification, feedback, and balance tuning. No fresh builds/tests were run merely for this documentation reconciliation.

### Future content, not implemented

- Enemy types from CypherRush remain concepts: Grave Zombie, Bone Hound, Skeleton Archer, Skull Crab, Shield Skeleton, Tombstone Skeleton, Timid Ghost, Hostile Ghost, Necromancer. Guardian concepts: Old Gravekeeper Skeleton and Ancient Cult of Undead. The source currently has one generic wandering melee enemy, not this full roster.
- Test Gravekeeper/simple final boss, victory/results screen, discovered-door registry, actual Boss Travel, Guardian/final seal checks, clues, enemy encyclopedia, named story NPC encounters, and true-ending unlocks are unimplemented.
- Merchants, keys/money, reward choices/weighted item generation, full weapon redesign, save/meta progression and web distribution remain future work. Cipher matching/HUD already exist, but the full item-to-seal-to-victory run does not.
- Recommended progression target after current combat testing: two distinct enemy types, controlled item acquisition, a small fixed-cipher run, a sealed door, simple boss, then victory. This is a suggested milestone sequence, not an implemented or fully approved content schedule.

Next: playtest/tune the expanded seven-spell/three-aura roster, especially chain range, barrier placement, shield persistence, and Bloom behavior. Discuss enemy redesign and regular item reward spawning after that review.

## Deferred fixes and future improvements

- Build dependency tracking is implemented with -MMD/-MP. Remaining build work: verify the latest changes natively on Windows and verify VS Code debugger launch on each platform.
- Growth protection reset is now implemented in source; manual verification of growing from a protected tail remains to be recorded.
- Enemy own-cell exclusion applies to both attached and detached targets. Initial overlap priority is deterministic; broader targeting policies may be revisited later.
- Detached storage is cleared on restart and after a successful room load in `Transition()`. Manual verification of successful and failed transitions with detached soldiers remains to be recorded.
- Cleanup: remove unused enemy `ghost_phase` and obsolete contact hooks if no longer needed. Enemy's parameterized `Update()` is not an override of `GameObject::Update()`; revisit the base interface if polymorphic updates become necessary.
- Future features: simultaneous damage aggregation, prediction helpers, more complex attack states, additional projectile behaviors, and larger attack patterns. No new managers or external dependencies are planned.
