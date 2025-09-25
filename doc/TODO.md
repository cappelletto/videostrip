# 📦 New repo: `videostrip-meshroom`

### Issue: **Bootstrap repository & import history**

**Description:**
Create `videostrip-meshroom` as a standalone repo and import the Meshroom plugin code **with history** from `videostrip`. Use `git filter-repo` to extract `meshroom/**` and keep authorship. Create default branch, protections, CODEOWNERS.
**Acceptance Criteria:**

* Repo created under org.
* History-preserving import done (`meshroom/` subtree only).
* CODEOWNERS + branch protection enabled.

---

### Issue: **Repo layout & packaging skeleton**

**Description:**
Adopt minimal Python packaging so the plugin can be zipped or installed as a package. Layout:

```
videostrip-meshroom/
  meshroom/
    videostrip_nodes/(__init__.py, VideostripExtractor.py)
    videostrip_minimal.mg
    config.json   # Meshroom env config (array-of-objects format)
  pyproject.toml  # project metadata, no runtime deps
  README.md, LICENSE
```

**Acceptance Criteria:**

* `pyproject.toml` present (build-system + metadata).
* Plugin structure matches Meshroom requirements.
* `config.json` uses **array of objects** format (no dict).

---

### Issue: **Node entrypoint hardening (binary discovery & env)**

**Description:**
Harden `VideostripExtractor.py`:

* Resolve `videostrip_cli` via: (1) absolute param override, (2) `$PATH`, (3) well-known locations.
* Clear, actionable error if binary missing (prints how to set MESHROOM_PLUGINS_PATH / install path).
* Optional: allow `extraArgs` passthrough.
  **Acceptance Criteria:**
* Node runs if `videostrip_cli` is on PATH or via override.
* Helpful error on missing binary with remediation hints.
* Unit-free smoke test script that imports node and assembles command string.

---

### Issue: **Lock node I/O contract to CLI 0.9.x**

**Description:**
Freeze compatibility with `videostrip_cli >=0.9.0`. Validate node parameters → CLI args mapping (`--input`, `--output`, `--feature`, `--format`, `--conf`, `--skip`, `--enhance`, `--grid-normalization`, etc). Document in repo.
**Acceptance Criteria:**

* `docs/api_contract.md` enumerates mapping and examples.
* Version gate in node: rejects older CLI versions with readable error.

---

### Issue: **Minimal CI: import & command construction smoke tests**

**Description:**
GitHub Actions job (Linux) that:

* Installs only Python (no Meshroom).
* Imports `meshroom.videostrip_nodes` module.
* Mocks `desc.CommandLineNode` enough to instantiate and **render** command.
* (Optional) If `videostrip_cli` present in runner, run a no-op `--help` call.
  **Acceptance Criteria:**
* CI green without installing Meshroom/aliceVision.
* Fails fast if import or command rendering breaks.

---

### Issue: **Release packaging: plugin zip**

**Description:**
Create a workflow to publish a `videostrip-meshroom-<ver>.zip` artifact with:

```
meshroom/** (nodes + .mg + config.json)
LICENSE, README.md
```

**Acceptance Criteria:**

* Tag push → Release with attached ZIP.
* Release notes include required `videostrip_cli` version.

---

### Issue: **User guide (focused)**

**Description:**
Short `README.md`:

* Installation via `MESHROOM_PLUGINS_PATH`.
* Node shows up under “videostrip”.
* Example CLI path override & troubleshooting.
  **Acceptance Criteria:**
* Verified by manual test (doc instructions work on clean machine).

---

### Issue: **Template mg refresh & sample datasets link**

**Description:**
Ensure `videostrip_minimal.mg` loads and links correctly. Provide URLs to tiny public sample videos for manual test (no bundling large assets).
**Acceptance Criteria:**

* `.mg` loads; node visible.
* Readme references samples.

---

# 🔧 Existing repo: `videostrip`

### Issue: **Remove embedded Meshroom plugin & point to new repo**

**Description:**
Delete `meshroom/` dir from `videostrip` and update docs to link to `videostrip-meshroom`.
**Acceptance Criteria:**

* `meshroom/` removed.
* README has a “Meshroom plugin” section linking to new repo and install steps.

---

### Issue: **CLI API contract & backward-compat policy (0.9.x)**

**Description:**
Write a stable spec for `videostrip_cli` options that the plugin depends on. Define policy:

* Patch/minor: backward compatible.
* Breaking: add deprecation period & compatibility shim.
  **Acceptance Criteria:**
* `docs/cli_api_contract.md` added.
* Labeled as **stable** for `0.9.x`.
* Changelog template updated to call out any CLI changes.

---

### Issue: **CLI `--version` and `--print-schema` improvements**

**Description:**
Ensure `videostrip_cli --version` prints SemVer + git sha. Add `--print-schema` (JSON/YAML) of outputs (`frames.csv`, `summary.yaml` fields) for tooling.
**Acceptance Criteria:**

* Version string machine-readable.
* `--print-schema` outputs spec used by Meshroom docs (optional but helpful).

---

### Issue: **Release note automation that pings plugin repo**

**Description:**
Upon tagging a release, open a PR or issue in `videostrip-meshroom` to update compatibility matrix if needed.
**Acceptance Criteria:**

* GitHub Action: `repository_dispatch` or PAT-based PR to bump README badge (e.g. “Requires videostrip_cli ≥ X.Y.Z”).

---

### Issue: **Deb packaging: keep vendored OpenCV private**

**Description:**
Ensure `.deb` installs libs under project prefix (e.g. `/opt/videostrip/lib`) and sets RPATH on `videostrip_cli` to prefer bundled libs; do **not** collide with system `/usr/lib`. Provide `postinst` to create symlinks in `/usr/local/bin` only for the binary (not libs).
**Acceptance Criteria:**

* `ldd videostrip_cli` shows bundled OpenCV under `/opt/videostrip/lib` (or package `lib/`).
* No system `libopencv*.so` picked up.
* Package lint clean.

---

### Issue: **Document Meshroom usage from videostrip README**

**Description:**
Short cross-repo guide: how to install plugin ZIP and run videostrip from Meshroom; link out for full docs.
**Acceptance Criteria:**

* One-page doc present and tested.

---

### Issue: **CI: drop Meshroom plugin steps**

**Description:**
Remove any CI steps referencing `meshroom/` and keep only core + packaging + perf smoke.
**Acceptance Criteria:**

* CI green with new matrix.
* No references to removed plugin paths.

---

# 🔁 Cross-repo coordination

### Issue: **Compatibility badge & matrix**

**Description:**
Add a small badge/table to both READMEs:

* `videostrip-meshroom vA.B` → requires `videostrip_cli >= X.Y.Z`
* `videostrip vX.Y.Z` → compatible plugin ≥ `A.B`
  **Acceptance Criteria:**
* Badge displayed.
* Table updated during releases.

---

### Issue: **Repository split script & docs**

**Description:**
Provide one-time scripts and notes:

* `git filter-repo --path meshroom/ --to-subdirectory-filter meshroom`
* Remove plugin from old repo; add link.
  **Acceptance Criteria:**
* Scripts in `tools/repo-split/`.
* Step-by-step doc followed once during migration.

---

### Issue: **Security & licensing alignment**

**Description:**
Ensure the new repo carries dual-license notice same as core, and that any node code respects upstream Meshroom licensing conventions.
**Acceptance Criteria:**

* LICENSE headers present.
* SPDX identifiers added to Python files.

---

# 🧩 Optional (nice-to-have)

### Issue: **Preflight binary check tool**

**Description:**
Add `python -m videostrip_meshroom.probe` that prints whether `videostrip_cli` is found and shows its version.
**Acceptance Criteria:**

* Script returns non-zero exit if not found or version too low.

---

### Issue: **Automated plugin self-test on Meshroom**

**Description:**
Nightly job (non-blocking) that spins a container with Meshroom, installs plugin, and runs the node on a 10-frame sample. Artifact: log + outputs.
**Acceptance Criteria:**

* Nightly results visible; non-required.

---

## Cutover sequence (operational)

1. Create `videostrip-meshroom` (Issue 1) and import plugin code.
2. Finish packaging skeleton + entrypoint hardening + CI smoke (Issues 2–5).
3. Tag an initial `videostrip-meshroom v0.9.0` release (Issue 6).
4. In `videostrip`, remove plugin dir and add links + API contract docs (Issues 7–9).
5. Update compatibility badges & set up cross-repo release automation (Issues 10–12).
6. (Optional) Add preflight check, nightly self-test (Optional issues).

This plan keeps the **core HPC/C++** repo lean while the **Meshroom node** remains a lightweight Python repo, versioned independently but with a clear compatibility contract.
