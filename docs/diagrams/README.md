# Rouge Snake diagrams

## Proposed enemy attack lifecycle

- `attack-lifecycle.dot`: editable Graphviz source.
- `attack-lifecycle.svg`: scalable rendered diagram.
- `attack-lifecycle.png`: rendered image for sharing and preview.

This diagram records the 2026-10-27 design discussion. It is a proposal, not a claim that the attack phases are implemented. The lifecycle proposal expands the earlier simple wind-up prototype into Ready, WindUp, Execute, and Recovery; exact timing, cooldown policy, and targeting policies still need agreement. No gameplay source or `GAME_DESIGN.md` was modified when creating it.

Edit labels, nodes, and arrows in the `.dot` file. With Graphviz installed, regenerate from this directory:

```powershell
dot -Tsvg attack-lifecycle.dot -o attack-lifecycle.svg
dot -Tpng -Gdpi=144 attack-lifecycle.dot -o attack-lifecycle.png
```

The initial files were rendered using Graphviz through `@viz-js/viz`, with PNG conversion through Sharp. These are diagram-authoring tools only; the game has no new dependency.
