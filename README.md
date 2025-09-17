[![C/C++ CI](https://github.com/cappelletto/videostrip/actions/workflows/c-cpp.yml/badge.svg)](https://github.com/cappelletto/videostrip/actions/workflows/c-cpp.yml)
[![License](https://img.shields.io/badge/License-Apache_2.0-blue.svg)](https://www.apache.org/licenses/LICENSE-2.0)
[![License](https://img.shields.io/badge/License-LGPL_3.0-blue.svg)](https://www.gnu.org/licenses/lgpl-3.0.html)

---

# videostrip

**videostrip** is a modular project for **video frame extraction and keyframe selection** to support **2D/3D reconstruction of underwater video transects**.
It produces **SfM-ready frame sets** and metadata for tools like **COLMAP, Meshroom, Metashape**, and our (future) **custom video transect pipeline**.

---

## **Project Status**

> **MVP+ / Early Stable**: The repository now provides a reproducible, schema-locked output contract (Schema v1).
>
> * A **CLI application** for processing videos into frames + metadata.
> * A **core library** exposing `VideoFrameExtractor` and related modules.
> * A **metadata writer** for reproducible CSV + YAML outputs.
> * CI/CD with unit tests for schema regression, running on Linux and Windows.
> * A modular image enhacement pipeline (pre-export stage)
> * Optional packaging system (DEB/TAR)
> * Precompiled binaries for Linux and Windows as part of the release payload (CD)
> * Linux compatible documentation (manpages)

**Short-term focus**: Consolidate usability, error resilience, and schema compliance before expanding to advanced feature modes (grid, enhancement, optical flow).

---

## **Key Features (v0.8.2)**

* ✅ **CLI support** for video processing with YAML or CLI configs.
* ✅ **Frame extraction with stride or overlap-based selection**.
* ✅ **Feature-based detection (ORB, AKAZE, SURF)**.
* ✅ **Schema v1 metadata outputs**:
  * `frames.csv` — per-frame metadata.
  * `summary.yaml` — run summary + configuration snapshot.
* ✅ **Deterministic file structure**: images/, features/, frames.csv, summary.yaml, run.log.
* ✅ **On-export frame enhancement**.
* ✅ **Grid-based feature density normalization**.

---

## **User Stories Alignment**

* **PI / Project Lead**
  * Tracks processed assets and reproducibility with `summary.yaml`.
* **Student / Data Collector**
  * Runs a simple CLI: `--input video.mp4 --output ./out` → usable frames + metadata.
* **Data Manager**
  * Archives deterministic outputs: locked schema v1.
* **Analyst / Scientist**
  * Consumes SfM-ready frames with known overlap/quality scores.

---

## **Build Instructions**

### **Dependencies**

* **C++17** compiler
* **CMake ≥ 3.18**
* **OpenCV ≥ 4.5**
* **yaml-cpp** (for YAML config parsing)
* **Catch2** (for unit testing)

### **Build (CLI)**

```bash
# Clone repository
git clone https://github.com/cappelletto/videostrip.git
cd videostrip

# Configure and build
cmake -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_MANPAGE=OFF -DBUILD_TESTS=OFF
cmake --build build -j4
```

The build produces:

```
build/bin/videostrip_cli     # CLI binary
build/lib/libvideostrip_core.a  # Core library
```

---

## **Usage (CLI)**

Basic run (defaults to ./output/*):

```bash
./videostrip_cli --input reef_transect.mp4
```

Outputs:
```
./output/images/*.png
./output/features/*.feat.txt
./output/frames.csv
./output/summary.yaml
./output/run.log
```

With YAML config:
```bash
./videostrip_cli --config configs/sample_min.yaml
```

CLI overrides YAML:
```bash
./videostrip_cli --config configs/sample_min.yaml --feature AKAZE --output ./out
```

### **Minimal YAML Config Example**

```yaml
version: 1

input:
  video: ./data/example_video.avi

output:
  base_dir: ./output

processing:
  feature_type: ORB
  image_format: png
  overlap_threshold: 0.8
  max_skipped_frames: 4
  apply_enhancement: false
  enable_logging: true
  feature_normalization: grid     # none|grid
  grid_normalization:
    cell: [32, 32]
    max_per_cell: 50
    score: response               # response|size
```

---

## **Enhancement Pipeline (NEW)**
TODO: extract this into a separate `docs/enhancement.md`.

Starting with **v0.7.x**, videostrip supports an optional **image enhancement stage** that preprocesses frames before feature extraction. This improves contrast and uniformity in underwater imagery.

### Supported enhancement steps

* **Contrast & offset**
  Linear transform per pixel: `alpha * I + beta`.
  *Params*: `alpha` (gain), `beta` (offset).

* **Gray-world white balance**
  Scales RGB channels so that their mean matches the global mean.
  *Params*: none.

* **Gamma correction**
  Applies a gamma LUT (`I_out = I_in^(1/gamma)`).
  *Params*: `value` (gamma > 0).

* **CLAHE (Contrast Limited Adaptive Histogram Equalization)**
  Adaptive histogram equalization on luminance channel.
  *Params*:

  * `clip_limit` (float, default 2.0)
  * `tile_grid` (two-element array, default `[8,8]`)
  * `space` (one of `YCrCb`, `HSV`, `Lab`, `BGR`)
    Channel is chosen implicitly: Y (YCrCb), V (HSV), L (Lab), all channels (BGR).

### Minimal YAML configuration

Add an `enhance` block at the top level of the config file:

```yaml
enhance:
  enable: true
  sequence:
    - type: contrast
      alpha: 1.10
      beta: -5
    - type: grayworld
    - type: gamma
      value: 1.05
    - type: clahe
      clip_limit: 2.0
      tile_grid: [8, 8]
      space: YCrCb
```

If no `enhance:` block is given but the legacy flag

```yaml
processing:
  apply_enhancement: true
```

is set, a default sequence `{grayworld, clahe(YCrCb)}` will be applied.

### CLI override

You can also pass a shorthand string (for quick tests):

```bash
./videostrip_cli --enhance.sequence "contrast(alpha=1.1,beta=-5); grayworld; gamma(1.05); clahe(clip=2.0,grid=8x8,space=YCrCb)"
```

---

## **Schema v1 Contract**

* **frames.csv** — columns: `frame_idx,timestamp_ms,output_image,feature_count,quality_score,georef`
* **summary.yaml** — includes `schema_version`, run info, config snapshot, list of exported images.
* **images/** — exported frames.
* **features/** — per-frame feature files.
* **run.log** — warnings, errors, info.

Schema is locked at `v1`. Future changes will bump schema_version.

---

## **Roadmap**

Near-term milestones:
1. In-depth performance profiling
2. Performance release by adding multithreading and GPU support.

Long-term:
* Optical flow / ECC overlap modes.
* Cross-platform GUI for both pipeline configuration and dispatching.
* Integration with Meshroom (node-base core library)

---

## **License**

**Dual License Notice**

videostrip is dual-licensed under:

  * Apache License, Version 2.0
    (see LICENSE.Apache or https://www.apache.org/licenses/LICENSE-2.0)

  * GNU Lesser General Public License, Version 3.0 or later
    (see LICENSE.LGPL or https://www.gnu.org/licenses/lgpl-3.0.html)

You may choose to use *videostrip* as a whole under either license. Note that some dependencies (e.g., OpenCV with non-free modules) have their own licensing terms.

See LICENSE.Apache and LICENSE.LGPL files in this repository for the complete license texts.

