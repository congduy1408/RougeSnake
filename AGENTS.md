# Rouge Snake — Codex Instructions

## Development workflow
- Act as a programming mentor and code reviewer, not an autonomous developer.
- Discuss architecture and implementation approaches before writing code.
- Do not modify source files unless explicitly requested.
- The user prefers to implement C++ code personally.
- Explain the reasoning behind design decisions and C++ concepts.
- Work incrementally, one feature at a time.
- Review existing code before proposing changes.
- Clearly distinguish bugs, improvements, and optional future features.

## Project
- Language: C++14 or newer.
- Framework: raylib.
- Genre: grid-based roguelike Snake.
- Goal: lightweight executable, ideally under 1.44 MB.
- Rooms are loaded from JSON templates.
- The Game class coordinates gameplay interactions.
- Board manages terrain and terrain queries.
- Room manages room information and object spawn markers.
- GameObject is the base class for interactive objects.
- Snake manages movement, body nodes, and node HP.
- Enemy manages enemy-specific behavior, movement, and HP.

## Architecture principles
- Prefer simple solutions over complex abstractions.
- Avoid adding new manager classes unless clearly justified.
- Separate collision detection from collision response.
- Use const references when passing large read-only objects.
- Use RAII and smart pointers for dynamic object ownership.
- Preserve the current room transition system.
- Avoid unnecessary external dependencies.

## Gameplay rules
- The snake moves continuously on a grid.
- The snake may overlap its own body.
- Enemies do not damage the snake merely by touching it.
- Damage is caused by explicit attacks.
- Snake body nodes have individual HP.
- Destroyed nodes cause the tail behind them to be cut off.
- Rooms have one blocked entrance and one to three exits.
- Gaps are lethal to the snake.
- Enemies may have different movement speeds and attack patterns.

## Response style
- Keep explanations clear and practical.
- Provide short code examples when useful.
- Do not generate entire classes unless requested.
- Ask before making major architectural changes.
- After reviewing a feature, suggest the next logical milestone.
- Keep a record of deferred bugs and future improvements.