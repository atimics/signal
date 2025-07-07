# Source File Configuration
# ============================================================================
# Defines all source files and object file mappings

# Directories
SRC_DIR = src
BUILD_DIR = build
DATA_DIR = data
ASSETS_DIR = assets
TOOLS_DIR = tools

# Main source files (matches original Makefile)
SOURCES = core.c systems.c \
          system/physics.c system/collision.c system/ai.c \
          system/camera.c system/lod.c system/performance.c system/memory.c \
          system/material.c \
          system/thrusters.c system/thruster_points_system.c system/control.c \
          system/scripted_flight.c \
          component/look_target.c component/thruster_points_component.c \
          thruster_points.c render_thrust_cones.c assets.c \
          asset_loader/asset_loader_index.c asset_loader/asset_loader_mesh.c \
          asset_loader/asset_loader_material.c \
          render_3d.c render_camera.c render_lighting.c render_mesh.c render_layers.c \
          render_pass_guard.c \
          microui/microui.c ui_microui.c ui_microui_adapter.c ui.c ui_api.c \
          ui_scene.c ui_components.c ui_adaptive_controls.c ui_menu_system.c \
          ui_navigation_menu_impl.c ui_navigation_menu_microui.c \
          data.c graphics_api.c gpu_resources.c graphics_health.c pipeline_manager.c \
          scene_state.c scene_script.c \
          scene_yaml_loader.c entity_yaml_loader.c \
          scripts/logo_scene.c scripts/navigation_menu_scene.c \
          scripts/flight_test_scene.c scripts/template_scene.c \
          game_input.c hal/input_hal_sokol.c hal/input_hal_mock.c \
          services/input_service.c services/input_action_maps.c input_state.c \
          config.c hidapi_mac.c main.c

# Object files
OBJECTS = $(SOURCES:%.c=$(BUILD_DIR)/%.o)

# Dependency files
DEPS = $(OBJECTS:.o=.d)

# Target executable
TARGET = $(BUILD_DIR)/signal

# WebAssembly source files (filtered for WASM compatibility)
WASM_SOURCES = $(filter-out hidapi_mac.c system/ode_physics.c system/gamepad.c \
                          input_processing.c scene_yaml_loader.c entity_yaml_loader.c \
                          scripts/ode_test_scene.c, $(SOURCES)) \
               wasm_stubs.c system/gamepad_wasm.c
