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
- Attack cells describe the released attack in board coordinates. Targeting uses the enemy's position at release; Game resolves those cells without asking the enemy to choose a target again.
- Start with a simple aimed melee attack against an adjacent Snake node. Exact cooldown, damage, and target-selection details remain to be agreed during implementation. Larger zones, projectiles, and more complex behavior are future milestones.

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
- Per-node protection is agreed but not implemented. A duration of 0.3 seconds was proposed as a starting point; the final duration and timer behavior on growth and room entry remain to be confirmed.

## Current implementation and next milestone

- Node HP, HP-preserving movement, fresh-node HP on growth, body cutting, and `Snake::IsDead()` are implemented. Body cutting has been manually tested by the user.
- Game checks Snake death before movement. `DamageNode()` rejects nonpositive damage and damage to a dead Snake.
- Entry protection has been corrected in source: the counter advances while unfolding or protection remains active, and damage no longer waits for full unfolding. This has been source-reviewed; runtime verification for lengths one, two, three, and a longer Snake remains to be recorded.
- Enemy attacks, read-only Snake input to enemy AI, the attack result, collection/resolution passes, and per-node damage protection remain pending in Phase 4C.
- Implementation differs from the agreed no-contact-damage rule: Game still calls `Enemy::OnSnakeEnter()`, which damages the enemy on head overlap after movement. Remove that interaction when implementing the agreed attack flow; keep enemy HP support for future explicit player attacks.
- Next milestone: implement and verify per-node damage protection, then connect the first aimed melee enemy attack and immediate death checks during attack resolution.
- Deferred improvements: simultaneous damage aggregation, prediction helpers, complex attack states, projectiles, and larger attack patterns. No new managers or external dependencies are planned.
