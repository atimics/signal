# CGame Documentation Map

*Last Updated: July 4, 2025*

This document provides a complete overview of the CGame project documentation structure and key resources.

## 🗂️ Documentation Structure

```
cgame/
├── README.md                      # Main project README
├── CLAUDE.md                      # AI assistant guide
├── docs/
│   ├── README.md                  # Documentation hub
│   ├── PROJECT_STATUS.md          # Current project status
│   ├── DOCUMENTATION_MAP.md       # This file
│   │
│   ├── active/                    # Current development focus
│   │   └── CURRENT_SPRINT.md      # Points to active sprint
│   │
│   ├── sprints/                   # Sprint management
│   │   ├── README.md              # Sprint overview & history
│   │   ├── active/                # Current sprint work
│   │   │   ├── CURRENT_SPRINT_STATUS.md
│   │   │   ├── SPRINT_24_MICROUI_IMPROVEMENTS.md
│   │   │   ├── engineering_report.md
│   │   │   └── SPRINT_24_PLAN.md
│   │   ├── completed/             # Historical sprints
│   │   │   ├── sprint_23/         # Technical Excellence
│   │   │   ├── sprint_22/         # Advanced Input
│   │   │   └── sprint_21/         # Flight Mechanics
│   │   └── backlog/               # Future work
│   │       ├── SPRINT_25_CANYON_RACING.md
│   │       └── SIMPLIFIED_THRUSTER_SYSTEM.md
│   │
│   ├── development/               # Developer resources
│   │   ├── CONTRIBUTING.md        # Contribution guide
│   │   ├── ASSET_PIPELINE_VALIDATION.md
│   │   └── MEMORY_TESTING.md
│   │
│   ├── guides/                    # Technical guides
│   │   ├── ECS_ARCHITECTURE.md    # ECS patterns
│   │   ├── COMPONENT_REFERENCE.md  # All components
│   │   ├── SYSTEM_REFERENCE.md     # All systems
│   │   ├── YAML_SCENE_FORMAT.md    # Scene definitions
│   │   └── SCRIPTED_FLIGHT_GUIDE.md # Flight paths
│   │
│   ├── manual/                    # User documentation
│   │   └── PLAYER_MANUAL.md       # Game guide
│   │
│   ├── design/                    # Design documents
│   │   └── [Various design docs]
│   │
│   ├── api/                       # API documentation
│   │   └── html_generated/        # Generated docs
│   │
│   └── archive/                   # Historical documents
│       ├── research/              # Old research
│       ├── proposals/             # Past proposals
│       └── migrations/            # Completed work
```

## 📍 Key Entry Points

### For New Contributors
1. **Start Here**: [CLAUDE.md](../CLAUDE.md) - AI assistant instructions
2. **Project Status**: [PROJECT_STATUS.md](PROJECT_STATUS.md) - Current state
3. **Current Work**: [Sprint 24 Status](sprints/active/CURRENT_SPRINT_STATUS.md)
4. **How to Help**: [CONTRIBUTING.md](development/CONTRIBUTING.md)

### For Active Development
1. **Sprint Dashboard**: [sprints/README.md](sprints/README.md)
2. **Current Sprint**: [Sprint 24 - MicroUI](sprints/active/SPRINT_24_MICROUI_IMPROVEMENTS.md)
3. **ECS Guide**: [ECS_ARCHITECTURE.md](guides/ECS_ARCHITECTURE.md)
4. **Components**: [COMPONENT_REFERENCE.md](guides/COMPONENT_REFERENCE.md)

### For Planning
1. **Next Sprint**: [Sprint 25 - Canyon Racing](sprints/backlog/SPRINT_25_CANYON_RACING.md)
2. **Sprint History**: [sprints/completed/](sprints/completed/)
3. **Feature Backlog**: [sprints/backlog/](sprints/backlog/)

## 🎯 Current Focus (Sprint 24)

**Problem**: MicroUI generates 0 vertices, blocking all UI development
**Goal**: Fix rendering pipeline and establish UI foundation
**Duration**: July 4-18, 2025
**Status**: Day 1 - Diagnosis Phase

## 📊 Quick Stats

- **Test Coverage**: 98% (56/57 tests)
- **Active Sprint**: #24 - MicroUI Improvements
- **Completed Sprints**: 5 (Sprints 19-23)
- **Documentation Coverage**: 95%+
- **Performance**: 60+ FPS

## 🔗 External Resources

- **Repository**: [GitHub](https://github.com/ratimics/cgame)
- **Issues**: [GitHub Issues](https://github.com/ratimics/cgame/issues)
- **Build Status**: See badges in main README

## 📝 Documentation Standards

### File Organization
- **Active work** → `sprints/active/`
- **Completed work** → `sprints/completed/`
- **Future work** → `sprints/backlog/`
- **Reference docs** → `guides/`
- **Old/outdated** → `archive/`

### Naming Conventions
- Sprint docs: `SPRINT_XX_NAME.md`
- Status updates: `CURRENT_SPRINT_STATUS.md`
- Guides: `TOPIC_GUIDE.md` or `TOPIC_REFERENCE.md`

### Update Frequency
- **Sprint Status**: Daily during active development
- **Project Status**: After each sprint or major milestone
- **Guides**: When features change significantly
- **Archives**: When documents become outdated

## 🚀 Recent Changes

### July 4, 2025
- Created Sprint 24 for MicroUI improvements
- Archived Sprint 23 (98% test coverage achieved!)
- Updated all documentation references
- Created this documentation map

### July 3, 2025
- Sprint 23 completed in single day
- Added scripted flight system
- Consolidated documentation structure
- Fixed build system issues

---

*This map is maintained to help navigate the CGame documentation. Update when the structure changes significantly.*