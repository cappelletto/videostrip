[![C/C++ CI](https://github.com/cappelletto/videostrip/actions/workflows/c-cpp.yml/badge.svg)](https://github.com/cappelletto/videostrip/actions/workflows/c-cpp.yml)

---

# videostrip

**videostrip** is a sandbox project for **video frame extraction and keyframe selection** to support **2D/3D reconstruction of underwater video transects**.
It is designed to produce **SfM-ready frame sets** for tools like **COLMAP, Meshroom, Metashape**, and our **custom video transect pipeline**.

---

## **Project Status**

> **Alpha / Sandbox**: Core functionality is not yet complete.
> The repository currently provides:
>
> * A **CLI prototype** for argument parsing and basic info logging.
> * A **GUI stub** using **OpenGL + ImGui** for future interactive workflows.
> * Initial CMake setup for building core libaries, and CLI and GUI targets.

**Short-term priority** is to **refactor and modularize** the project to prepare for production-ready frame extraction aligned with the **pipeline user stories**.

---

## **Key Features (Planned)**

* ✅ **CLI and GUI support** for video processing.
* ⏳ **Frame extraction with guaranteed overlap** (for SfM pipelines).
* ⏳ **Feature-based frame selection** (e.g., ORB/SIFT/AKAZE keypoints).
* ⏳ **Batch processing with YAML configuration** for reproducible workflows.
* ⏳ **Metadata output (YAML/CSV)** for downstream SfM and archival.

---

## **User Stories Alignment**

* **PI / Project Lead**

  * Wants to track processed videos, metadata, and reproducibility.
  * Will use the future **batch + metadata outputs**.

* **Student / Data Collector**

  * Needs a simple CLI to extract usable frames for 3D reconstruction.
  * Will use the **CLI with progress logging**.

* **Data Manager**

  * Requires deterministic outputs and archival.
  * Will rely on **standardized folder structures + YAML logs**.

* **Analyst / Scientist**

  * Needs SfM-ready frames with **good feature coverage and overlap**.
  * Will use the **feature-aware frame selection** (future milestone).

---

## **Build Instructions**

### **Dependencies**

* **C++17** compiler
* **CMake ≥ 3.18**
* **OpenCV ≥ 4.5**
* **OpenGL + GLFW3** (for GUI)

### **Build (CLI + GUI)**

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
build/videostrip            # CLI
build/libvideostrip_core.a  # Core library
build/libvideostrip_utils.a # Utilities library
```

---

## **Usage (Current CLI)**

Check OpenCV and build info:

```bash
./videostrip --dump
```

[TODO] ⏳ Process a single video:

```bash
./videostrip --input=mytransect.mp4 --output=frames
```

> **Note:** Frame extraction logic is **not yet implemented**.
> Current CLI validates input and prints metadata.

---

## **Short-Term Roadmap**

The first development sprint focuses on **refactor and modularization**:

1. **Refactor repository into modular structure**

   * Separate core library, CLI, and GUI.
2. **Introduce `videostrip-core` library target**

   * Both executables link against shared core logic.
3. **Modernize CMake setup**

   * Proper target dependencies, no global flags.
4. **Add initial test scaffold**

   * Prepare for automated feature extraction and regression tests.

Future milestones:

* Frame extraction with overlap guarantee (to port existing homography based implementation)
* Feature-based keyframe selection
* Pipeline-ready metadata (YAML + CSV)
* Batch + GUI interactive workflows

---

## **License**

See [LICENSE](LICENSE) for details.

