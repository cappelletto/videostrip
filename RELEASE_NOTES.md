Release Notes Draft

  # videostrip v0.9.2 — 2025-09-25

  ## Highlights
  - Completed the Meshroom integration split: all node templates and scripts now live in the new [videostrip-meshroom](https://github.com/cappelletto/videostrip-meshroom) repository, keeping this tree focused on the core extractor and CLI.
  - Added `scripts/todo2issues.py`, a GitHub CLI helper that turns structured sections in `doc/TODO.md` into labelled issues (supports dry-run, milestone assignment, and cross-repo targeting).

  ## Breaking Changes
  - Meshroom node assets (`meshroom/` and `doc/TODO.md` entries tied to them) have been removed from this repository. Clone `videostrip-meshroom` alongside `videostrip` if you rely on Meshroom pipelines.

  ## Enhancements
  - Promoted the bundled `args.hxx` header to `third_party/args.hxx` and rewired include paths in the CLI/core targets, simplifying packaging and making the dependency boundary explicit.
  - Pruned the unused `videostrip_utils` module to reduce build surface area.
  - CLI `--enhance` flag now applies the default `{grayworld, clahe(YCrCb)}` enhancement pipeline even when no YAML config is provided, matching the legacy `processing.apply_enhancement: true` behavior.

  ## Documentation
  - README now opens with the Meshroom migration notice, refreshed build/usage guidance, and a direct link to the contributing guide.
  - AGENTS.md updated to reference the new Meshroom repository and keep contributor roles accurate.

  ## Maintenance
  - TODO backlog cleaned while moving Meshroom work items into issue automation.
  - Version bumped to 0.9.2 in CMake to anchor release artifacts.

  ## Upgrade Notes
  - Consumers who automated Meshroom flows should fetch `videostrip-meshroom` and update any paths that previously referenced `meshroom/` inside this repo.
  - To try the TODO automation, install GitHub CLI, then run:
    ```bash
    python scripts/todo2issues.py --todo doc/TODO.md --owner <github-org> --dry-run
    ```
