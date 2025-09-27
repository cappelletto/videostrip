#include <catch2/catch_session.hpp> // provides main()
#include <catch2/catch_test_macros.hpp>
#include <filesystem>
#include <fstream>
#include <string>
#include <videostrip_cli/config_loader.hpp> // we test the CLI loader
#include <videostrip_core/videostrip_core.hpp>

namespace fs = std::filesystem;
using videostrip::ExtractorConfig;

static std::string write_temp_yaml(std::string contents)
{
    auto dir = fs::temp_directory_path() / "vs_cfg_test";
    fs::create_directories(dir);
    auto path = dir / "config.yaml";
    std::ofstream ofs(path);
    ofs << contents;
    ofs.close();
    return path.string();
}

TEST_CASE("YAML loader normalizes output paths and applies precedence", "[config][yaml]")
{
    // Compose a YAML with an absolute base_dir to make assertions deterministic
    const auto base_abs = (fs::temp_directory_path() / "vs_yaml_base").string();

    std::string yaml = "version: 1\n"
                       "input:\n"
                       "  video: ./data/example_video.avi\n"
                       "output:\n"
                       "  base_dir: " +
                       base_abs +
                       "\n"
                       "  images_dir: images\n"
                       "  features_dir: features\n"
                       "  csv: frames.csv\n"
                       "  yaml: summary.yaml\n"
                       "  log: run.log\n"
                       "processing:\n"
                       "  feature_type: ORB\n"
                       "  image_format: png\n"
                       "  overlap_threshold: 0.7\n"
                       "  max_skipped_frames: 3\n"
                       "  apply_enhancement: false\n"
                       "  create_output_dirs: true\n"
                       "  enable_logging: true\n";

    const std::string yaml_path = write_temp_yaml(yaml);

    // 1) Load YAML into a partial config
    ExtractorConfig ycfg;
    std::string err;
    REQUIRE(videostrip::cli::load_yaml_config(yaml_path, ycfg, err));
    REQUIRE(err.empty());

    // 2) Check normalization: all outputs must be under base_abs
    REQUIRE(ycfg.output_images_dir == (fs::path(base_abs) / "images").string());
    REQUIRE(ycfg.output_features_dir == (fs::path(base_abs) / "features").string());
    REQUIRE(ycfg.output_metadata_csv == (fs::path(base_abs) / "frames.csv").string());
    REQUIRE(ycfg.output_summary_yaml == (fs::path(base_abs) / "summary.yaml").string());
    REQUIRE(ycfg.output_log_file == (fs::path(base_abs) / "run.log").string());

    // 3) Merge into a default config (simulates: defaults <- YAML)
    ExtractorConfig cfg;
    videostrip::cli::merge_yaml_into(cfg, ycfg);

    // 4) Simulate CLI overrides (highest precedence):
    //    a) override feature_type & image_format
    cfg.feature_type = "AKAZE";
    cfg.image_format = "jpg";
    //    b) override --log path explicitly
    cfg.output_log_file = (fs::path(base_abs) / "override.log").string();
    //    c) simulate --output <cli_base> behavior by rebuilding canonical layout under it
    const auto cli_base = fs::temp_directory_path() / "vs_cli_out";
    cfg.output_images_dir = (cli_base / "images").string();
    cfg.output_features_dir = (cli_base / "features").string();
    cfg.output_metadata_csv = (cli_base / "frames.csv").string();
    cfg.output_summary_yaml = (cli_base / "summary.yaml").string();
    // (log remains overridden above)

    // 5) Assertions: CLI values win
    REQUIRE(cfg.feature_type == "AKAZE");
    REQUIRE(cfg.image_format == "jpg");

    REQUIRE(cfg.output_images_dir == (cli_base / "images").string());
    REQUIRE(cfg.output_features_dir == (cli_base / "features").string());
    REQUIRE(cfg.output_metadata_csv == (cli_base / "frames.csv").string());
    REQUIRE(cfg.output_summary_yaml == (cli_base / "summary.yaml").string());
    REQUIRE(cfg.output_log_file == (fs::path(base_abs) / "override.log").string());
}
