# SIGNAL Test Suite Makefile
# ============================================================================
# Advanced test discovery, execution, and coverage analysis system
# Supports our new folder-based test organization structure

# Test directories following our mirror architecture
TEST_CORE_DIR = tests/core
TEST_SYSTEMS_DIR = tests/systems  
TEST_RENDERING_DIR = tests/rendering
TEST_ASSETS_DIR = tests/assets
TEST_UI_DIR = tests/ui
TEST_SCENES_DIR = tests/scenes
TEST_INTEGRATION_DIR = tests/integration
TEST_PERFORMANCE_DIR = tests/performance
TEST_REGRESSION_DIR = tests/regression
TEST_UNIT_DIR = tests/unit
TEST_BACKLOG_DIR = tests/backlog
TEST_SUPPORT_DIR = tests/support
TEST_VENDOR_DIR = tests/vendor

# Test discovery and runner executables
TEST_DISCOVERY = $(BUILD_DIR)/test_discovery
TEST_RUNNER = $(BUILD_DIR)/test_runner_unified
TEST_COVERAGE = $(BUILD_DIR)/test_coverage_analyzer

# Build directories for organized test outputs
TEST_BUILD_DIR = $(BUILD_DIR)/tests
TEST_REPORTS_DIR = $(BUILD_DIR)/test_reports
TEST_ARTIFACTS_DIR = $(BUILD_DIR)/test_artifacts

# Common test flags
TEST_CFLAGS = -Wall -Wextra -std=c99 -O2 -g -Isrc -Itests -Itests/vendor -Itests/support -Itests/stubs \
              -DUNITY_TESTING -DTEST_MODE -DSOKOL_DUMMY_BACKEND \
              -Wno-error=unused-function -Wno-error=unused-variable

# Unity test framework
UNITY_SRC = $(TEST_VENDOR_DIR)/unity.c

# Test support infrastructure
TEST_SUPPORT_SRC = $(TEST_SUPPORT_DIR)/test_discovery.c \
                   $(TEST_SUPPORT_DIR)/test_utilities.c

# ============================================================================
# AUTOMATED TEST DISCOVERY
# ============================================================================

.PHONY: test-discover
test-discover: $(TEST_DISCOVERY)
	@echo "🔍 Discovering tests in all directories..."
	./$(TEST_DISCOVERY) --scan-all --generate-report $(TEST_REPORTS_DIR)/test_discovery.md

# Build test discovery tool
$(TEST_DISCOVERY): $(TEST_SUPPORT_SRC) $(UNITY_SRC) | $(BUILD_DIR)
	@echo "🔨 Building test discovery system..."
	$(CC) $(TEST_CFLAGS) -DTEST_DISCOVERY_MAIN \
		-o $@ $(TEST_SUPPORT_SRC) $(UNITY_SRC) -lm

# ============================================================================
# ORGANIZED TEST SUITES (Mirror Architecture)
# ============================================================================

# Core ECS tests
.PHONY: test-core
test-core: build-test-core run-test-core

build-test-core: $(TEST_BUILD_DIR)/suite_core
$(TEST_BUILD_DIR)/suite_core: $(wildcard $(TEST_CORE_DIR)/*.c) $(UNITY_SRC) $(TEST_SUPPORT_SRC) | $(TEST_BUILD_DIR)
	@echo "🧪 Building core ECS tests..."
	$(CC) $(TEST_CFLAGS) -o $@ \
		$(wildcard $(TEST_CORE_DIR)/*.c) \
		$(UNITY_SRC) $(TEST_SUPPORT_SRC) \
		src/core.c tests/stubs/engine_test_stubs.c -lm

run-test-core: $(TEST_BUILD_DIR)/suite_core
	@echo "🧪 Running Core ECS Tests..."
	./$(TEST_BUILD_DIR)/suite_core

# Systems tests (organized by subfolder)
.PHONY: test-systems
test-systems: test-systems-physics test-systems-control test-systems-camera test-systems-performance

test-systems-physics: build-test-systems-physics run-test-systems-physics
build-test-systems-physics: $(TEST_BUILD_DIR)/suite_systems_physics
$(TEST_BUILD_DIR)/suite_systems_physics: $(wildcard $(TEST_SYSTEMS_DIR)/physics/*.c) $(wildcard $(TEST_SYSTEMS_DIR)/test_physics*.c) $(UNITY_SRC) $(TEST_SUPPORT_SRC) | $(TEST_BUILD_DIR)
	@echo "🚀 Building physics systems tests..."
	$(CC) $(TEST_CFLAGS) -o $@ \
		$(wildcard $(TEST_SYSTEMS_DIR)/physics/*.c) $(wildcard $(TEST_SYSTEMS_DIR)/test_physics*.c) \
		$(UNITY_SRC) $(TEST_SUPPORT_SRC) \
		src/core.c src/system/physics.c tests/stubs/engine_test_stubs.c -lm

run-test-systems-physics: $(TEST_BUILD_DIR)/suite_systems_physics
	@echo "🚀 Running Physics Systems Tests..."
	./$(TEST_BUILD_DIR)/suite_systems_physics

test-systems-control: build-test-systems-control run-test-systems-control  
build-test-systems-control: $(TEST_BUILD_DIR)/suite_systems_control
$(TEST_BUILD_DIR)/suite_systems_control: $(wildcard $(TEST_SYSTEMS_DIR)/control/*.c) $(wildcard $(TEST_SYSTEMS_DIR)/test_control*.c) $(wildcard $(TEST_SYSTEMS_DIR)/test_thrusters*.c) $(wildcard $(TEST_SYSTEMS_DIR)/test_input*.c) $(UNITY_SRC) $(TEST_SUPPORT_SRC) | $(TEST_BUILD_DIR)
	@echo "🎮 Building control systems tests..."
	$(CC) $(TEST_CFLAGS) -o $@ \
		$(wildcard $(TEST_SYSTEMS_DIR)/control/*.c) $(wildcard $(TEST_SYSTEMS_DIR)/test_control*.c) $(wildcard $(TEST_SYSTEMS_DIR)/test_thrusters*.c) $(wildcard $(TEST_SYSTEMS_DIR)/test_input*.c) \
		$(UNITY_SRC) $(TEST_SUPPORT_SRC) \
		src/core.c src/system/control.c src/system/thrusters.c src/system/input.c src/system/physics.c tests/stubs/engine_test_stubs.c -lm

run-test-systems-control: $(TEST_BUILD_DIR)/suite_systems_control
	@echo "🎮 Running Control Systems Tests..."
	./$(TEST_BUILD_DIR)/suite_systems_control

test-systems-camera: build-test-systems-camera run-test-systems-camera
build-test-systems-camera: $(TEST_BUILD_DIR)/suite_systems_camera
$(TEST_BUILD_DIR)/suite_systems_camera: $(wildcard $(TEST_SYSTEMS_DIR)/camera/*.c) $(wildcard $(TEST_SYSTEMS_DIR)/test_camera*.c) $(UNITY_SRC) $(TEST_SUPPORT_SRC) | $(TEST_BUILD_DIR)
	@echo "📷 Building camera systems tests..."
	$(CC) $(TEST_CFLAGS) -o $@ \
		$(wildcard $(TEST_SYSTEMS_DIR)/camera/*.c) $(wildcard $(TEST_SYSTEMS_DIR)/test_camera*.c) \
		$(UNITY_SRC) $(TEST_SUPPORT_SRC) \
		src/core.c src/system/camera.c tests/stubs/engine_test_stubs.c -lm

run-test-systems-camera: $(TEST_BUILD_DIR)/suite_systems_camera
	@echo "📷 Running Camera Systems Tests..."
	./$(TEST_BUILD_DIR)/suite_systems_camera

test-systems-performance: build-test-systems-performance run-test-systems-performance
build-test-systems-performance: $(TEST_BUILD_DIR)/suite_systems_performance
$(TEST_BUILD_DIR)/suite_systems_performance: $(wildcard $(TEST_SYSTEMS_DIR)/performance/*.c) $(wildcard $(TEST_SYSTEMS_DIR)/test_performance*.c) $(wildcard $(TEST_SYSTEMS_DIR)/test_lod*.c) $(wildcard $(TEST_SYSTEMS_DIR)/test_memory*.c) $(UNITY_SRC) $(TEST_SUPPORT_SRC) | $(TEST_BUILD_DIR)
	@echo "📊 Building performance systems tests..."
	$(CC) $(TEST_CFLAGS) -o $@ \
		$(wildcard $(TEST_SYSTEMS_DIR)/performance/*.c) $(wildcard $(TEST_SYSTEMS_DIR)/test_performance*.c) $(wildcard $(TEST_SYSTEMS_DIR)/test_lod*.c) $(wildcard $(TEST_SYSTEMS_DIR)/test_memory*.c) \
		$(UNITY_SRC) $(TEST_SUPPORT_SRC) \
		src/core.c src/system/performance.c src/system/lod.c src/system/memory.c tests/stubs/engine_test_stubs.c -lm

run-test-systems-performance: $(TEST_BUILD_DIR)/suite_systems_performance
	@echo "📊 Running Performance Systems Tests..."
	./$(TEST_BUILD_DIR)/suite_systems_performance

# Rendering tests
.PHONY: test-rendering
test-rendering: build-test-rendering run-test-rendering

build-test-rendering: $(TEST_BUILD_DIR)/suite_rendering
$(TEST_BUILD_DIR)/suite_rendering: $(wildcard $(TEST_RENDERING_DIR)/**/*.c) $(wildcard $(TEST_RENDERING_DIR)/*.c) $(UNITY_SRC) $(TEST_SUPPORT_SRC) | $(TEST_BUILD_DIR)
	@echo "🎨 Building rendering tests..."
	$(CC) $(TEST_CFLAGS) -o $@ \
		$(wildcard $(TEST_RENDERING_DIR)/**/*.c) $(wildcard $(TEST_RENDERING_DIR)/*.c) \
		$(UNITY_SRC) $(TEST_SUPPORT_SRC) \
		src/core.c src/render_3d.c src/render_camera.c src/render_mesh.c src/render_lighting.c src/gpu_resources.c src/graphics_api.c \
		tests/stubs/graphics_api_test_stub.c tests/stubs/engine_test_stubs.c -lm

run-test-rendering: $(TEST_BUILD_DIR)/suite_rendering
	@echo "🎨 Running Rendering Tests..."
	./$(TEST_BUILD_DIR)/suite_rendering

# UI tests
.PHONY: test-ui
test-ui: build-test-ui run-test-ui

build-test-ui: $(TEST_BUILD_DIR)/suite_ui
$(TEST_BUILD_DIR)/suite_ui: $(wildcard $(TEST_UI_DIR)/**/*.c) $(wildcard $(TEST_UI_DIR)/*.c) $(UNITY_SRC) $(TEST_SUPPORT_SRC) | $(TEST_BUILD_DIR)
	@echo "🖥️  Building UI tests..."
	$(CC) $(TEST_CFLAGS) -o $@ \
		$(wildcard $(TEST_UI_DIR)/**/*.c) $(wildcard $(TEST_UI_DIR)/*.c) \
		$(UNITY_SRC) $(TEST_SUPPORT_SRC) \
		src/core.c src/ui.c src/ui_api.c src/ui_scene.c src/ui_components.c \
		tests/stubs/graphics_api_test_stub.c tests/stubs/engine_test_stubs.c -lm

run-test-ui: $(TEST_BUILD_DIR)/suite_ui
	@echo "🖥️  Running UI Tests..."
	./$(TEST_BUILD_DIR)/suite_ui

# Integration tests
.PHONY: test-integration
test-integration: build-test-integration run-test-integration

build-test-integration: $(TEST_BUILD_DIR)/suite_integration
$(TEST_BUILD_DIR)/suite_integration: $(wildcard $(TEST_INTEGRATION_DIR)/*.c) $(UNITY_SRC) $(TEST_SUPPORT_SRC) | $(TEST_BUILD_DIR)
	@echo "🔗 Building integration tests..."
	$(CC) $(TEST_CFLAGS) -o $@ \
		$(wildcard $(TEST_INTEGRATION_DIR)/*.c) \
		$(UNITY_SRC) $(TEST_SUPPORT_SRC) \
		src/core.c src/systems.c src/system/physics.c src/system/control.c src/system/thrusters.c src/system/input.c src/system/camera.c \
		tests/stubs/graphics_api_test_stub.c tests/stubs/engine_test_stubs.c -lm

run-test-integration: $(TEST_BUILD_DIR)/suite_integration
	@echo "🔗 Running Integration Tests..."
	./$(TEST_BUILD_DIR)/suite_integration

# Performance benchmarks
.PHONY: test-performance-benchmarks
test-performance-benchmarks: build-test-performance-benchmarks run-test-performance-benchmarks

build-test-performance-benchmarks: $(TEST_BUILD_DIR)/suite_performance_benchmarks
$(TEST_BUILD_DIR)/suite_performance_benchmarks: $(wildcard $(TEST_PERFORMANCE_DIR)/**/*.c) $(wildcard $(TEST_PERFORMANCE_DIR)/*.c) $(UNITY_SRC) $(TEST_SUPPORT_SRC) | $(TEST_BUILD_DIR)
	@echo "⚡ Building performance benchmarks..."
	$(CC) $(TEST_CFLAGS) -O3 -DPERFORMANCE_BENCHMARKS -o $@ \
		$(wildcard $(TEST_PERFORMANCE_DIR)/**/*.c) $(wildcard $(TEST_PERFORMANCE_DIR)/*.c) \
		$(UNITY_SRC) $(TEST_SUPPORT_SRC) \
		src/core.c src/systems.c src/system/physics.c src/system/performance.c \
		tests/stubs/graphics_api_test_stub.c tests/stubs/engine_test_stubs.c -lm

run-test-performance-benchmarks: $(TEST_BUILD_DIR)/suite_performance_benchmarks
	@echo "⚡ Running Performance Benchmarks..."
	./$(TEST_BUILD_DIR)/suite_performance_benchmarks

# Regression tests
.PHONY: test-regression
test-regression: build-test-regression run-test-regression

build-test-regression: $(TEST_BUILD_DIR)/suite_regression
$(TEST_BUILD_DIR)/suite_regression: $(wildcard $(TEST_REGRESSION_DIR)/**/*.c) $(wildcard $(TEST_REGRESSION_DIR)/*.c) $(UNITY_SRC) $(TEST_SUPPORT_SRC) | $(TEST_BUILD_DIR)
	@echo "🛡️  Building regression tests..."
	$(CC) $(TEST_CFLAGS) -o $@ \
		$(wildcard $(TEST_REGRESSION_DIR)/**/*.c) $(wildcard $(TEST_REGRESSION_DIR)/*.c) \
		$(UNITY_SRC) $(TEST_SUPPORT_SRC) \
		src/core.c src/system/physics.c \
		tests/stubs/graphics_api_test_stub.c tests/stubs/engine_test_stubs.c -lm

run-test-regression: $(TEST_BUILD_DIR)/suite_regression
	@echo "🛡️  Running Regression Tests..."
	./$(TEST_BUILD_DIR)/suite_regression

# ============================================================================
# UNIFIED TEST EXECUTION
# ============================================================================

.PHONY: test-all
test-all: test-discover test-core test-systems test-rendering test-ui test-integration test-performance-benchmarks test-regression
	@echo ""
	@echo "✅ ALL TEST SUITES COMPLETED"
	@echo "=============================="
	@echo "Core ECS Tests: ✅"
	@echo "Systems Tests: ✅"
	@echo "  - Physics Systems: ✅"
	@echo "  - Control Systems: ✅" 
	@echo "  - Camera Systems: ✅"
	@echo "  - Performance Systems: ✅"
	@echo "Rendering Tests: ✅"
	@echo "UI Tests: ✅"
	@echo "Integration Tests: ✅"
	@echo "Performance Benchmarks: ✅"
	@echo "Regression Tests: ✅"
	@echo ""

# Quick smoke tests (critical path only)
.PHONY: test-smoke
test-smoke: test-core test-systems-physics test-integration
	@echo "💫 Smoke tests completed - critical path verified"

# ============================================================================
# COVERAGE ANALYSIS
# ============================================================================

.PHONY: test-coverage
test-coverage: $(TEST_COVERAGE) test-all
	@echo "📊 Analyzing test coverage..."
	./$(TEST_COVERAGE) --analyze-all --generate-report $(TEST_REPORTS_DIR)/coverage_report.html

# Build coverage analyzer
$(TEST_COVERAGE): $(TEST_SUPPORT_DIR)/test_coverage.c $(UNITY_SRC) | $(BUILD_DIR)
	@echo "🔨 Building coverage analyzer..."
	$(CC) $(TEST_CFLAGS) -DTEST_COVERAGE_MAIN \
		-o $@ $(TEST_SUPPORT_DIR)/test_coverage.c $(UNITY_SRC) -lm

# Generate coverage reports using cloc
.PHONY: coverage-cloc
coverage-cloc: | $(TEST_REPORTS_DIR)
	@echo "📈 Generating cloc coverage analysis..."
	cloc src/ tests/ --by-file --csv > $(TEST_REPORTS_DIR)/cloc_analysis.csv
	cloc src/ tests/ --by-file > $(TEST_REPORTS_DIR)/cloc_analysis.txt
	@echo "Coverage analysis saved to $(TEST_REPORTS_DIR)/cloc_analysis.*"

# ============================================================================
# TEST REPORT GENERATION
# ============================================================================

.PHONY: test-reports
test-reports: test-all coverage-cloc
	@echo "📋 Generating comprehensive test reports..."
	@mkdir -p $(TEST_REPORTS_DIR)
	@echo "# SIGNAL Test Suite Report" > $(TEST_REPORTS_DIR)/test_summary.md
	@echo "" >> $(TEST_REPORTS_DIR)/test_summary.md
	@echo "Generated: $(shell date)" >> $(TEST_REPORTS_DIR)/test_summary.md
	@echo "" >> $(TEST_REPORTS_DIR)/test_summary.md
	@echo "## Test Coverage Summary" >> $(TEST_REPORTS_DIR)/test_summary.md
	@echo "" >> $(TEST_REPORTS_DIR)/test_summary.md
	@tail -n 20 $(TEST_REPORTS_DIR)/cloc_analysis.txt >> $(TEST_REPORTS_DIR)/test_summary.md
	@echo "" >> $(TEST_REPORTS_DIR)/test_summary.md
	@echo "## Test Discovery Results" >> $(TEST_REPORTS_DIR)/test_summary.md
	@echo "" >> $(TEST_REPORTS_DIR)/test_summary.md
	@if [ -f $(TEST_REPORTS_DIR)/test_discovery.md ]; then cat $(TEST_REPORTS_DIR)/test_discovery.md >> $(TEST_REPORTS_DIR)/test_summary.md; fi
	@echo "📋 Test reports generated in $(TEST_REPORTS_DIR)/"

.PHONY: test-dashboard
test-dashboard: test-reports
	@echo "🖥️  Generating test dashboard..."
	@echo "<!DOCTYPE html><html><head><title>SIGNAL Test Dashboard</title></head><body>" > $(TEST_REPORTS_DIR)/dashboard.html
	@echo "<h1>SIGNAL Test Suite Dashboard</h1>" >> $(TEST_REPORTS_DIR)/dashboard.html
	@echo "<p>Generated: $(shell date)</p>" >> $(TEST_REPORTS_DIR)/dashboard.html
	@echo "<h2>Quick Links</h2><ul>" >> $(TEST_REPORTS_DIR)/dashboard.html
	@echo "<li><a href='test_summary.md'>Test Summary</a></li>" >> $(TEST_REPORTS_DIR)/dashboard.html
	@echo "<li><a href='cloc_analysis.txt'>Coverage Analysis</a></li>" >> $(TEST_REPORTS_DIR)/dashboard.html
	@echo "<li><a href='test_discovery.md'>Test Discovery</a></li>" >> $(TEST_REPORTS_DIR)/dashboard.html
	@echo "</ul></body></html>" >> $(TEST_REPORTS_DIR)/dashboard.html
	@echo "🖥️  Dashboard available at $(TEST_REPORTS_DIR)/dashboard.html"

# ============================================================================
# MAINTENANCE AND UTILITIES
# ============================================================================

.PHONY: test-clean
test-clean:
	@echo "🧹 Cleaning test artifacts..."
	rm -rf $(TEST_BUILD_DIR)
	rm -rf $(TEST_REPORTS_DIR)
	rm -rf $(TEST_ARTIFACTS_DIR)

.PHONY: test-directories
test-directories: $(TEST_BUILD_DIR) $(TEST_REPORTS_DIR) $(TEST_ARTIFACTS_DIR)

$(TEST_BUILD_DIR):
	@mkdir -p $(TEST_BUILD_DIR)

$(TEST_REPORTS_DIR):
	@mkdir -p $(TEST_REPORTS_DIR)

$(TEST_ARTIFACTS_DIR): 
	@mkdir -p $(TEST_ARTIFACTS_DIR)

# ============================================================================
# HELP AND DOCUMENTATION
# ============================================================================

.PHONY: test-help
test-help:
	@echo "🧪 SIGNAL Test Suite - Advanced Organization System"
	@echo "=================================================="
	@echo ""
	@echo "📁 FOLDER-BASED TEST ORGANIZATION:"
	@echo "  tests/core/          - Core ECS & engine tests"
	@echo "  tests/systems/       - Game systems tests (organized by subfolder)"
	@echo "  tests/rendering/     - Rendering pipeline tests"
	@echo "  tests/ui/            - User interface tests"
	@echo "  tests/integration/   - Cross-system integration tests"
	@echo "  tests/performance/   - Performance benchmarks"
	@echo "  tests/regression/    - Regression tests"
	@echo ""
	@echo "🎯 TEST EXECUTION TARGETS:"
	@echo "  test-all             - Run all test suites (comprehensive)"
	@echo "  test-smoke           - Run critical path smoke tests"
	@echo "  test-core            - Run core ECS tests"
	@echo "  test-systems         - Run all systems tests"
	@echo "  test-systems-physics - Run physics systems tests"
	@echo "  test-systems-control - Run control systems tests"
	@echo "  test-systems-camera  - Run camera systems tests" 
	@echo "  test-systems-performance - Run performance systems tests"
	@echo "  test-rendering       - Run rendering tests"
	@echo "  test-ui              - Run UI tests"
	@echo "  test-integration     - Run integration tests"
	@echo "  test-performance-benchmarks - Run performance benchmarks"
	@echo "  test-regression      - Run regression tests"
	@echo ""
	@echo "🔍 DISCOVERY & ANALYSIS:"
	@echo "  test-discover        - Discover and catalog all tests"
	@echo "  test-coverage        - Analyze test coverage"
	@echo "  coverage-cloc        - Generate cloc-based coverage analysis"
	@echo ""
	@echo "📊 REPORTING:"
	@echo "  test-reports         - Generate comprehensive test reports"
	@echo "  test-dashboard       - Generate HTML test dashboard"
	@echo ""
	@echo "🛠️  MAINTENANCE:"
	@echo "  test-clean           - Clean test build artifacts"
	@echo "  test-help            - Show this help message"
	@echo ""

# Make test-help the default when someone runs 'make test' without arguments
.PHONY: test
test: test-help
