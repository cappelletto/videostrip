# Clean build with tests enabled
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTS=ON

# Build all + explicitly build tests to trigger discovery
cmake --build build --config Release --target test_core_smoke test_config_yaml -j

# Run all tests
cd build
ctest --output-on-failure -C Release

# Or run a single test binary directly (handy for debug)
# ./bin/test_core_smoke
# ./bin/test_config_yaml