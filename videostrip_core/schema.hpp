// videostrip_core/schema.hpp
#pragma once
#include <string>

namespace videostrip
{

inline constexpr const char* kSchemaVersion = "1";

// YAML description for summary.yaml (human-friendly)
inline const char* SummarySchemaYaml() noexcept
{
    return R"YAML(
schema_version: "1"
summary_yaml:
  type: object
  required: [schema_version, app, input, output, processing, frames_count]
  properties:
    schema_version: { type: string, enum: ["1"] }
    app:
      type: object
      required: [name, version]
      properties:
        name: { type: string, const: "videostrip_cli" }
        version: { type: string }
        git: { type: object, properties: { commit: {type:string}, dirty: {type:boolean} } }
        build: { type: object, properties: { type:{type:string}, compiler:{type:string}, compiler_version:{type:string}, platform:{type:string} } }
        opencv: { type: object, properties: { version:{type:string}, vendored:{type:boolean} } }
    input:
      type: object
      properties:
        video: { type: string }
    output:
      type: object
      required: [base_dir]
      properties:
        base_dir: { type: string }
        images_dir: { type: string }
        frames_csv: { type: string }
        summary_yaml: { type: string }
        run_log: { type: string }
    processing:
      type: object
      properties:
        feature_type: { type: string, enum: [ORB, AKAZE, SURF] }
        image_format: { type: string, enum: [png, jpg] }
        overlap_threshold: { type: number }
        max_skipped_frames: { type: integer }
        apply_enhancement: { type: boolean}
        create_output_dirs: { type: boolean }
        feature_normalization: { type: string, enum: [none, grid] }
        grid_normalization:
          type: object
          properties:
            cell: { type: array, items: { type: integer }, minItems: 2, maxItems: 2 }
            max_per_cell: { type: integer }
            score: { type: string, enum: [response, size] }
        enable_logging: { type: boolean }
    enhance:
      type: object
      properties:
        enable: { type: boolean }
        sequence: { type: array, items: { type: object } }
    frames_count: { type: integer }
    started_at: { type: string }
    ended_at: { type: string }
    duration_s: { type: number }
)YAML";
}

// JSON description for frames.csv (machine-friendly)
inline const char* FramesCsvSchemaJson() noexcept
{
    return R"JSON(
{
  "schema_version": "1",
  "frames_csv": {
    "header": [
      "frame_idx",
      "timestamp_ms",
      "output_image",
      "feature_count",
      "quality_score",
      "georef"
    ],
    "columns": {
      "frame_idx":      {"type":"integer","description":"0-based index in decoded stream"},
      "timestamp_ms":   {"type":"integer","description":"pts in milliseconds"},
      "output_image":   {"type":"string","description":"relative path to saved frame"},
      "feature_count":  {"type":"integer","description":"number of detected features"},
      "quality_score":  {"type":"number","description":"unitless quality metric in [0,1]"},
      "georef":         {"type":"string","nullable":true,"description":"WKT/JSON or empty if not available"}
    }
  }
}
)JSON";
}

} // namespace videostrip
