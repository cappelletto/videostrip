[![C/C++ CI](https://github.com/cappelletto/videostrip/actions/workflows/c-cpp.yml/badge.svg)](https://github.com/cappelletto/videostrip/actions/workflows/c-cpp.yml)

---

# videostrip

**videostrip** is a modular project for **video frame extraction and keyframe selection** to support **2D/3D reconstruction of underwater video transects**.
It produces **SfM-ready frame sets** and metadata for tools like **COLMAP, Meshroom, Metashape**, and our **custom video transect pipeline**.

---

## **Project Status**

> **MVP+ / Early Stable**: The repository now provides a reproducible, schema-locked output contract (Schema v1).
>
> * A **CLI application** for processing videos into frames + metadata.
> * A **core library** exposing `VideoFrameExtractor` and related modules.
> * A **metadata writer** for reproducible CSV + YAML outputs.
> * CI/CD with unit tests for schema regression.

**Short-term focus**: Consolidate usability, error resilience, and schema compliance before expanding to advanced feature modes (grid, enhancement, optical flow).

---

## **Key Features (v0.4.0)**

* ✅ **CLI support** for video processing with YAML or CLI configs.
* ✅ **Frame extraction with stride or overlap-based selection**.
* ✅ **Feature-based detection (ORB, AKAZE, SURF)**.
* ✅ **Schema v1 metadata outputs**:
  * `frames.csv` — per-frame metadata.
  * `summary.yaml` — run summary + configuration snapshot.
* ✅ **Deterministic file structure**: images/, features/, frames.csv, summary.yaml, run.log.
* ⏳ **On-export frame enhancement** (future).
* ⏳ **Grid-based feature density normalization** (future).

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
cmake -B build -DCMAKE_BUILD_TYPE=Release
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
1. Add **frame enhancement filters** (CLAHE, WB).
2. Introduce **grid-based feature normalization**.
3. Expand **Windows CI/CD** coverage.

Long-term:
* Optical flow / ECC overlap modes.
* Batch manager & GUI front-end.

---

## **License**

See [LICENSE](LICENSE) for details.
