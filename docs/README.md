# CGame Engine Documentation

**Last Updated**: June 30, 2025

Welcome to the CGame engine documentation. This directory contains comprehensive documentation for the engine's architecture, development process, and current status.

## 📋 Current Status

- **Current Sprint**: [Sprint 19: Performance Optimization & Polish](./sprints/active/sprint_19_performance_optimization.md)
- **Project Status**: [CURRENT_STATUS.md](./CURRENT_STATUS.md)
- **Asset Pipeline**: [ASSET_PIPELINE_VALIDATION.md](../ASSET_PIPELINE_VALIDATION.md) ✅ COMPLETE

## 🚀 Quick Start

1. **For Developers**: See [CURRENT_STATUS.md](./CURRENT_STATUS.md) for the latest engine status
2. **For Artists**: See [Asset Pipeline Guide](./guides/) for content creation workflow
3. **For Contributors**: Check [active sprints](./sprints/active/) for current development focus

## 📚 Documentation Structure

### Core Documentation
- **[CURRENT_STATUS.md](./CURRENT_STATUS.md)** - Real-time project status and current focus
- **[STRATEGIC_PLAN.md](./STRATEGIC_PLAN.md)** - Long-term project roadmap and vision
- **[ASSET_PIPELINE.md](./ASSET_PIPELINE.md)** - Complete asset pipeline documentation

### Development
- **[sprints/](./sprints/)** - Sprint planning and execution tracking
  - [active/](./sprints/active/) - Currently running sprints
  - [completed/](./sprints/completed/) - Completed sprint documentation
  - [backlog/](./sprints/backlog/) - Planned future sprints
- **[guides/](./guides/)** - Technical guides and tutorials
- **[research/](./research/)** - Technical research and analysis

### Archive
- **[archive/](./archive/)** - Historical documentation and deprecated guides

## 🎯 Current Capabilities

### ✅ Complete Systems
- **Asset Pipeline**: UV-mapped mesh generation with SVG texture editing
- **Engine Core**: ECS architecture with scene management
- **Rendering**: 7 working meshes with proper UV textures
- **Development Tools**: Web-based mesh viewer and asset validation

### 🔄 Active Development
- **Performance Optimization**: LOD systems and memory optimization
- **Developer Experience**: Hot-reload and profiling tools
- **Production Polish**: Release configuration and documentation

### 📋 Planned Features
- **Gameplay Systems**: Physics, AI, player controls
- **Audio System**: 3D spatial audio and music
- **Advanced Rendering**: Shadows, lighting, post-processing

## 📊 Key Metrics

- **Assets**: 7 meshes, all rendering with textures ✅
- **Performance**: 60+ FPS stable in all test scenes ✅
- **Memory**: Under 256MB usage target ✅
- **Loading**: ~10x faster than OBJ parsing ✅
- **File Size**: 40-50% reduction with binary format ✅

## 🔗 Quick Links

- **Build & Run**: See [README.md](../README.md) in project root
- **Asset Creation**: [Asset Pipeline Validation](../ASSET_PIPELINE_VALIDATION.md)
- **Mesh Viewer**: `make view-meshes` for web-based 3D preview
- **Performance Testing**: `make test-performance`

For detailed technical information, see the [guides/](./guides/) directory.
