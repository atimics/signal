# SIGNAL Do**[📊 Sprint Status Dashboard](SPRINT_STATUS.md)** - Current sprint overview and tracking  
**[🔧 Documentation Improvements](DOCUMENTATION_IMPROVEMENTS_SUMMARY.md)** - Recent improvements and standardsumentation

**Document ID**: DOC_README_MAIN  
**Date**: July 6, 2025  
**Author**: Development Team

Welcome to the SIGNAL project documentation. This directory contains all project documentation organized for easy navigation and maintenance.

## 📚 Documentation Structure

**[� Sprint Status Dashboard](SPRINT_STATUS.md)** - Current sprint overview and tracking

### 🎯 Active Development
- **[`sprints/active/`](sprints/active/)** - Current sprint work
  - [Sprint Status Dashboard](SPRINT_STATUS.md) - Sprint 26 Semi-Autonomous Flight
  - [Sprint 26 Plan](sprints/active/SPRINT_26_SEMI_AUTONOMOUS_FLIGHT.md) - Flight assist system

### 🔧 Technical Documentation
- **[`technical/`](technical/)** - Architecture and system documentation
  - [Architecture Overview](technical/ARCHITECTURE_OVERVIEW.md) - System architecture guide
  - [Component Catalog](technical/COMPONENT_CATALOG.md) - All available components
- **[`project/`](project/)** - Project-level documentation
  - [Research](project/research/FLIGHT_CONTROL_RESEARCH.md) - Consolidated research
  - [Game Vision](project/GAME_VISION.md) - Project vision and goals
### 🔧 Development Guides
- **[`guides/`](guides/)** - Technical implementation guides
  - [ECS Architecture](guides/ECS_ARCHITECTURE.md) - Entity Component System patterns
  - [Component Reference](guides/COMPONENT_REFERENCE.md) - All available components
  - [System Reference](guides/SYSTEM_REFERENCE.md) - All game systems
  - [YAML Scene Format](guides/YAML_SCENE_FORMAT.md) - Scene definition guide
  - [Scripted Flight System](guides/SCRIPTED_FLIGHT_GUIDE.md) - Autonomous flight paths

### 📖 User Documentation
- **[`manual/`](manual/)** - User-facing documentation
  - [Player Manual](manual/PLAYER_MANUAL.md) - Complete game guide
  - Controls reference and troubleshooting

### 🏃 Sprint Management
- **[`sprints/`](sprints/)** - Sprint planning and tracking
  - [Sprint Overview](sprints/README.md) - Sprint history and management
  - [`completed/`](sprints/completed/) - Historical sprint documentation
  - [`backlog/`](sprints/backlog/) - Future work organization
    - [`immediate/`](sprints/backlog/immediate/) - Next 2-3 sprints
    - [`features/`](sprints/backlog/features/) - Feature proposals
    - [`technical_debt/`](sprints/backlog/technical_debt/) - Technical debt tracking

### 🎨 Design Documents
- **[`design/`](design/)** - Design and vision documents
  - Game vision, technical architecture, and active proposals

### 🔧 API Documentation
- **[`api/`](api/)** - Code documentation
  - [`html_generated/`](api/html_generated/) - Auto-generated HTML documentation
  - Component and system documentation

### 📦 Archive
- **[`archive/`](archive/)** - Historical documents
  - [`research/`](archive/research/) - Old research papers
  - [`proposals/`](archive/proposals/) - Superseded proposals
  - [`migrations/`](archive/migrations/) - Completed migrations
  - [`SIGNAL/`](archive/SIGNAL/) - Alternative game concept (historical)

## 🔍 Quick Navigation

### For New Developers
1. Start with [Current Sprint Status](sprints/active/CURRENT_SPRINT_STATUS.md)
2. Read [Contributing Guide](development/CONTRIBUTING.md)
3. Check [Sprint Overview](sprints/README.md) for context

### For Development
- **Current Work**: [Sprint 26 Semi-Autonomous Flight](sprints/active/SPRINT_26_SEMI_AUTONOMOUS_FLIGHT.md) 🚀
- **Sprint Status**: [Dashboard](SPRINT_STATUS.md) ✅
- **Previous Sprint**: [Sprint 25 Complete](sprints/completed/) 🎮
- **Technical Docs**: [`technical/`](technical/)
- **API Reference**: [`api/`](api/)

### For Planning
- **Sprint Backlog**: [`sprints/backlog/`](sprints/backlog/)
- **Design Documents**: [`design/`](design/)
- **Feature Proposals**: [`sprints/backlog/features/`](sprints/backlog/features/)

## 📋 Documentation Standards

All documentation follows the [Documentation Standards](DOCUMENTATION_STANDARDS.md) which include:

### Organization Principles
1. **Single Source of Truth**: Each document type has one canonical location
2. **Clear Hierarchy**: Easy to find current vs historical information
3. **Consistent Structure**: Predictable organization patterns
4. **Active vs Archive**: Clear separation of current and historical content

### File Naming and Structure
- Use descriptive, consistent names per [standards](DOCUMENTATION_STANDARDS.md)
- Include metadata headers for all documents
- Use relative links and maintain cross-references
- Follow validation requirements (use `docs/scripts/validate_docs.sh`)

## 🔄 Maintenance

### Regular Tasks
- Update [Sprint Status Dashboard](SPRINT_STATUS.md)
- Archive completed sprints
- Run documentation validation: `docs/scripts/validate_docs.sh`
- Update cross-references and maintain standards compliance

### Sprint Boundaries
- Move completed sprints to [`completed/`](sprints/completed/)
- Update active sprint documentation
- Review and organize backlog items

## 🚀 Recent Updates

### July 6, 2025 - Sprint 26 Started
- ✅ Documentation standards implemented
- ✅ Sprint 26 Semi-Autonomous Flight planning complete
- ✅ Documentation structure improved and consolidated
- ✅ Validation scripts and compliance tools added

### July 4, 2025 - Documentation Improvements
- ✅ New technical documentation structure
- ✅ Consolidated research documentation
- ✅ Architecture overview and component catalog
- ✅ Documentation standards and validation

## 📞 Support

For questions about documentation:
1. Check the [Sprint Status Dashboard](SPRINT_STATUS.md)
2. Review [Documentation Standards](DOCUMENTATION_STANDARDS.md)
3. Use validation tools: `docs/scripts/validate_docs.sh`

---

*Last Updated: July 6, 2025 - Sprint 26 Semi-Autonomous Flight Active*