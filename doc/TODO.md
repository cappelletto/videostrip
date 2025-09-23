Short answer: all three are viable. Given your goals (“stable node, minimal churn”), I recommend **keeping it in-tree now at repo top-level `meshroom/`** and only considering a split later if adoption warrants. If you want a middle ground, use `integrations/meshroom/` but still ship it from this repo. Below are the trade-offs.

# Option A — Top-level `meshroom/` (in this repo)

**Pros**

* **Zero friction packaging:** add one `install(DIRECTORY ... )` like your current CPack flow (same pattern you already use for docs/assets).
* **Version lock:** node and CLI evolve together; avoids matrix of incompatible plugin vs CLI versions.
* **Discoverability:** users see “Meshroom support” immediately in the repo/README.
* **Simpler CI:** one pipeline builds the CLI and ensures the plugin files ship.

**Cons**

* Repo scope grows slightly (Python module, optional venv).
* Any plugin change requires releasing the whole project (though you said node will be stable).

**When to choose**

* Now. You want #48 done fast, with minimal risk and packaging already wired.

# Option B — `integrations/meshroom/` (still in this repo)

**Pros**

* **Clean separation of concerns:** integrations are grouped (future: COLMAP adapters, Metashape scripts, etc.).
* **Docs clarity:** “Everything third-party lives under `integrations/`”.

**Cons**

* Slightly longer paths and install rules, but functionally identical to Option A.
* Users must be told that the **plugin root** is `integrations/meshroom` (they point `MESHROOM_PLUGINS_PATH` to its parent folder).

**When to choose**

* If you foresee multiple integrations soon and want a tidy tree from day one.

# Option C — Separate repo (e.g., `videostrip-meshroom-plugin`)

**Pros**

* **Independent release cadence:** update plugin without touching core.
* **Lighter dependencies per repo:** plugin repo can be Python-only; core stays C++.
* **Easier external contributions:** some Meshroom users may prefer forking a small plugin repo.

**Cons**

* **Version compatibility management:** you must pin supported videostrip versions in the plugin (README + runtime checks).
* **Two issue trackers / PR flows:** overhead for triage and cross-repo coordination.
* **Packaging/user friction:** either (1) ship the plugin separately (users must set `MESHROOM_PLUGINS_PATH`), or (2) still bundle it in videostrip release artifacts, which defeats the point of separation.

**When to choose**

* If plugin evolves on a different tempo, or you expect Meshroom-specific features/bugs needing quick releases, or you want to invite a distinct contributor base.

# Recommendation (for v0.9.x)

* **Go with Option A (top-level `meshroom/`)** to land #48 quickly and ship via CPack alongside the CLI and headers. It matches your current packaging/documentation model and keeps the node version-locked to the CLI while we stabilize.
* If/when you add more third-party integrations, **move to Option B** (`integrations/meshroom/`) in a trivial refactor (update install path + docs).
* Only split to Option C once usage justifies independent releases and you’re ready to maintain a compatibility matrix.

# Concrete next steps

1. Create **`meshroom/`** (or `integrations/meshroom/` if you prefer now).
2. Add `videostrip_nodes/` with `VideostripExtractor.py` + `__init__.py` (Python CommandLine node).
3. Add a CMake `install(DIRECTORY …)` to package the plugin with your releases (mirroring your existing install rules).
4. In the README, add a small “Meshroom plugin” blurb pointing users to set `MESHROOM_PLUGINS_PATH` to the **plugin root** (your package path), consistent with your current doc style.

