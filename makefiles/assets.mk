# Asset Pipeline Configuration
# ============================================================================
# Asset compilation and management system

# Asset directories
BUILD_ASSETS_DIR = $(BUILD_DIR)/assets

# Python environment
PYTHON = ./.venv/bin/python3
ASSET_COMPILER = $(TOOLS_DIR)/asset_compiler.py
ASSET_PIPELINE = $(TOOLS_DIR)/build_pipeline.py
CLEAN_PIPELINE = $(TOOLS_DIR)/clean_asset_pipeline.py

# ============================================================================
# ASSET COMPILATION TARGETS
# ============================================================================

# Main asset compilation target
.PHONY: assets
assets: $(BUILD_ASSETS_DIR)

$(BUILD_ASSETS_DIR): | $(BUILD_DIR)
	@echo "🔨 Compiling assets to binary format..."
	$(PYTHON) $(ASSET_PIPELINE)
	@echo "✅ Asset compilation complete."

# Force asset recompilation
.PHONY: assets-force
assets-force:
	@echo "🔨 Force recompiling assets..."
	$(PYTHON) $(ASSET_PIPELINE) --force
	@echo "✅ Asset compilation complete."

# Generate procedural source assets
.PHONY: generate-assets
generate-assets:
	@echo "🌱 Generating all mesh assets with UV layouts..."
	$(PYTHON) $(CLEAN_PIPELINE) --all
	@echo "✅ Source asset generation complete."

# Generate specific asset
.PHONY: generate-asset
generate-asset:
	@echo "🌱 Generating mesh asset: $(MESH)..."
	$(PYTHON) $(CLEAN_PIPELINE) --mesh $(MESH)
	@echo "✅ Asset $(MESH) generated."

# Full asset regeneration and compilation
.PHONY: regenerate-assets
regenerate-assets: clean-source-assets generate-assets assets

# Clean source assets (SVGs, PNGs, OBJs)
.PHONY: clean-source-assets
clean-source-assets:
	@echo "🧹 Cleaning source assets..."
	find $(ASSETS_DIR)/meshes/props -name "*.obj" -delete 2>/dev/null || true
	find $(ASSETS_DIR)/meshes/props -name "*.svg" -delete 2>/dev/null || true
	find $(ASSETS_DIR)/meshes/props -name "*.png" -delete 2>/dev/null || true
	find $(ASSETS_DIR)/meshes/props -name "*.mtl" -delete 2>/dev/null || true
	find $(ASSETS_DIR)/meshes/props -name "metadata.json" -delete 2>/dev/null || true
	@echo "✅ Source assets cleaned."

# Clean compiled assets only
.PHONY: clean-assets
clean-assets:
	@echo "🧹 Cleaning compiled assets..."
	rm -rf $(BUILD_ASSETS_DIR)
	@echo "✅ Compiled assets cleaned."

# ============================================================================
# WEBASSEMBLY ASSETS
# ============================================================================

# Compile assets for WASM (simplified)
.PHONY: assets-wasm
assets-wasm: $(BUILD_ASSETS_DIR)
	@echo "🔨 Preparing assets for WASM build..."
	@echo "📋 Using existing compiled assets from build/assets/"

# ============================================================================
# ASSET UTILITIES
# ============================================================================

# Launch mesh viewer
.PHONY: view-meshes
view-meshes:
	@echo "🎨 Launching mesh viewer..."
	$(PYTHON) $(TOOLS_DIR)/launch_mesh_viewer.py

# Validate thruster definitions
.PHONY: validate-thrusters
validate-thrusters: $(BUILD_DIR)/validate_thrusters
	@echo "🔍 Validating thruster attachment points..."
	@./$(BUILD_DIR)/validate_thrusters data/thrusters/ assets/meshes/
	@echo "✅ Thruster validation complete"

# Build thruster validation tool
$(BUILD_DIR)/validate_thrusters: tools/validate_thrusters.c | $(BUILD_DIR)
	$(CC) -Wall -Wextra -O2 -o $@ $< -lm

# ============================================================================
# ASSET DEPENDENCY TRACKING
# ============================================================================

# Asset dependency file
ASSET_DEPS = $(BUILD_DIR)/asset_deps.mk

# Generate asset dependencies (future enhancement)
$(ASSET_DEPS): $(ASSET_PIPELINE) | $(BUILD_DIR)
	@echo "# Auto-generated asset dependencies" > $@
	@echo "# Generated on: $(shell date)" >> $@
	@# TODO: Implement asset dependency scanning
	@touch $@

# Include asset dependencies if available
-include $(ASSET_DEPS)
