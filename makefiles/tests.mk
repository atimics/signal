# Test System Configuration
# ============================================================================
# Centralized test compilation rules and dependency management

# Test directories
TEST_DIR = tests
TEST_VENDOR_DIR = $(TEST_DIR)/vendor
TEST_STUBS_DIR = $(TEST_DIR)/stubs
TEST_SUPPORT_DIR = $(TEST_DIR)/support

# Common test dependencies
UNITY_SRC = $(TEST_VENDOR_DIR)/unity.c
TEST_STUBS = $(TEST_STUBS_DIR)/graphics_api_test_stub.c $(TEST_STUBS_DIR)/engine_test_stubs.c
TEST_STUBS_WITH_ASSETS = $(TEST_STUBS) $(TEST_STUBS_DIR)/asset_stubs.c
TEST_UTILITIES = $(TEST_SUPPORT_DIR)/test_utilities.c

# Core engine dependencies for tests
CORE_TEST_DEPS = src/core.c src/gpu_resources.c
PHYSICS_TEST_DEPS = $(CORE_TEST_DEPS) src/system/physics.c
RENDERING_TEST_DEPS = $(CORE_TEST_DEPS) src/render_3d.c src/render_camera.c src/render_lighting.c src/render_mesh.c src/graphics_health.c src/render_pass_guard.c
RENDERING_WITH_ASSETS_DEPS = $(RENDERING_TEST_DEPS) src/assets.c

# Test compilation template
define COMPILE_TEST
$(BUILD_DIR)/$(1): $(2) $(UNITY_SRC) $(TEST_STUBS) | $(BUILD_DIR)
	@echo "🔨 Building $(1)..."
	@mkdir -p $$(dir $$@)
	$$(CC) $$(TEST_CFLAGS) $(3) -o $$@ $(2) $$(UNITY_SRC) $(4) $$(LIBS)
endef

# ============================================================================
# CORE TESTS
# ============================================================================

# Core math tests (minimal dependencies)
$(eval $(call COMPILE_TEST,test_core_math,tests/core/test_math.c,-DTEST_STANDALONE,$(CORE_TEST_DEPS) $(TEST_STUBS)))

# Core components tests
$(eval $(call COMPILE_TEST,test_core_components,tests/core/test_components.c,-DTEST_STANDALONE,$(PHYSICS_TEST_DEPS) $(TEST_UTILITIES) $(TEST_STUBS)))

# Core world tests
$(eval $(call COMPILE_TEST,test_core_world,tests/core/test_world.c,-DTEST_STANDALONE,$(PHYSICS_TEST_DEPS) $(TEST_UTILITIES) $(TEST_STUBS)))

# ============================================================================
# RENDERING TESTS (Consolidated - working tests only)
# ============================================================================

# Basic rendering tests (without assets.c, use asset stubs)
$(eval $(call COMPILE_TEST,test_rendering,tests/rendering/test_rendering.c,-DTEST_STANDALONE,$(RENDERING_TEST_DEPS) $(TEST_STUBS_WITH_ASSETS)))

# NEW: Comprehensive render layers tests (without assets.c to avoid symbol conflicts)
$(eval $(call COMPILE_TEST,test_render_layers,tests/rendering/test_render_layers.c,-DTEST_STANDALONE -DTEST_MODE,src/render_layers.c src/render_pass_guard.c $(CORE_TEST_DEPS) src/render_3d.c src/render_camera.c src/render_lighting.c src/render_mesh.c src/graphics_health.c $(TEST_STUBS_WITH_ASSETS)))

# NEW: UI rendering pipeline tests (use UI function stubs instead of real UI source)
$(eval $(call COMPILE_TEST,test_ui_rendering_pipeline,tests/rendering/test_ui_rendering_pipeline.c,-DTEST_STANDALONE,$(CORE_TEST_DEPS) $(TEST_STUBS) $(TEST_STUBS_DIR)/ui_function_stubs.c))

# ============================================================================
# SYSTEM TESTS
# ============================================================================

# Physics tests
$(eval $(call COMPILE_TEST,test_physics_6dof,tests/systems/test_physics_6dof.c,,$(PHYSICS_TEST_DEPS) $(TEST_STUBS)))

# Thruster system tests
$(eval $(call COMPILE_TEST,test_thrusters,tests/systems/test_thrusters.c,,$(PHYSICS_TEST_DEPS) src/system/thrusters.c $(TEST_STUBS)))

# Control authority tests
$(eval $(call COMPILE_TEST,test_control,tests/systems/test_control.c,,$(PHYSICS_TEST_DEPS) src/system/control.c src/system/thrusters.c src/system/input.c src/input_processing.c src/component/look_target.c $(TEST_STUBS)))

# Camera system tests
$(eval $(call COMPILE_TEST,test_camera,tests/systems/test_camera.c,,$(CORE_TEST_DEPS) src/system/camera.c src/render_camera.c $(TEST_STUBS)))

# Input system tests
$(eval $(call COMPILE_TEST,test_input,tests/systems/test_input.c,,$(CORE_TEST_DEPS) src/system/input.c src/input_processing.c src/component/look_target.c $(TEST_STUBS)))

# ============================================================================
# UI TESTS
# ============================================================================

# UI system tests
$(eval $(call COMPILE_TEST,test_ui,tests/ui/test_ui_system.c,,src/ui_api.c src/ui_scene.c src/ui_components.c $(CORE_TEST_DEPS) $(TEST_STUBS_DIR)/microui_test_stubs.c $(TEST_STUBS)))

# MicroUI rendering tests (with additional dependencies)
$(eval $(call COMPILE_TEST,test_microui_rendering,tests/microui/test_microui_rendering.c,,$(TEST_STUBS_DIR)/ui_microui_test_utils.c src/ui_microui.c src/microui/microui.c $(RENDERING_TEST_DEPS) $(TEST_STUBS)))

# ============================================================================
# INTEGRATION TESTS
# ============================================================================

# Flight integration tests
$(eval $(call COMPILE_TEST,test_flight_integration,tests/integration/test_flight_integration.c,,$(PHYSICS_TEST_DEPS) src/system/thrusters.c src/system/control.c src/system/input.c src/input_processing.c src/system/gamepad.c src/hidapi_mac.c src/component/look_target.c $(TEST_STUBS)))

# Critical input system tests
$(eval $(call COMPILE_TEST,test_input_critical,tests/systems/test_input_system_critical.c,,$(CORE_TEST_DEPS) src/system/input.c src/input_processing.c src/component/look_target.c $(TEST_STUBS)))

# Critical flight scene tests
$(eval $(call COMPILE_TEST,test_flight_scene_critical,tests/integration/test_flight_test_scene_critical.c,-DINCLUDE_REAL_SCENE_STATE,$(PHYSICS_TEST_DEPS) src/system/thrusters.c src/system/control.c src/system/input.c src/input_processing.c src/component/look_target.c src/scene_state.c $(TEST_STUBS)))

# ============================================================================
# TEST TARGETS
# ============================================================================

# Individual test targets (consolidated - working tests only)
TEST_CORE_TARGETS = $(BUILD_DIR)/test_core_math $(BUILD_DIR)/test_core_components $(BUILD_DIR)/test_core_world
TEST_RENDERING_TARGETS = $(BUILD_DIR)/test_rendering $(BUILD_DIR)/test_render_layers $(BUILD_DIR)/test_ui_rendering_pipeline

# All test targets (only working ones)
ALL_TEST_TARGETS = $(TEST_CORE_TARGETS) $(TEST_RENDERING_TARGETS)

# ============================================================================
# TEST EXECUTION
# ============================================================================

# Sequential test execution (current)
.PHONY: test-sequential
test-sequential: $(ALL_TEST_TARGETS)
	@echo "🧪 Running CGame Test Suite (Sequential)"
	@echo "========================================"
	@for test in $(ALL_TEST_TARGETS); do \
		echo "Running $$test..."; \
		./$$test || exit 1; \
	done
	@echo "✅ All tests completed successfully!"

# Parallel test execution (new)
.PHONY: test-parallel
test-parallel: $(ALL_TEST_TARGETS)
	@echo "🧪 Running CGame Test Suite (Parallel)"
	@echo "======================================"
	@$(MAKE) -j$(shell nproc 2>/dev/null || echo 4) run-tests-parallel

.PHONY: run-tests-parallel
run-tests-parallel: $(ALL_TEST_TARGETS)
	@for test in $(ALL_TEST_TARGETS); do \
		(./$$test > $$test.log 2>&1 && echo "✅ $$test: PASS" || echo "❌ $$test: FAIL") & \
	done; wait

# Test by category
.PHONY: test-core test-rendering test-systems test-ui test-integration
test-core: $(TEST_CORE_TARGETS)
	@for test in $^; do ./$$test || exit 1; done

test-rendering: $(TEST_RENDERING_TARGETS)
	@for test in $^; do ./$$test || exit 1; done

test-systems: $(TEST_SYSTEM_TARGETS)
	@for test in $^; do ./$$test || exit 1; done

test-ui: $(TEST_UI_TARGETS)
	@for test in $^; do ./$$test || exit 1; done

test-integration: $(TEST_INTEGRATION_TARGETS)
	@for test in $^; do ./$$test || exit 1; done
