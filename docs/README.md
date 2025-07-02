# Documentation Hub - SIGNAL Engine

**Last Updated**: July 2, 2025  
**Status**: 🚨 Critical Architectural Review in Progress

Welcome to the SIGNAL engine documentation. This directory contains all technical documentation, guides, and project management resources for the CGame engine and "The Ghost Signal" game.

## 🚨 **URGENT: Critical Findings**

**ARCHITECTURAL REVIEW REQUIRED** for Sprint 21 implementation:
- **[Critical Findings Summary](CRITICAL_FINDINGS_SUMMARY.md)** - Immediate action items and executive summary
- **[Architecture Analysis](ARCHITECTURE_ANALYSIS_FLIGHT_MECHANICS.md)** - Detailed technical analysis and refactoring recommendations

## 📋 **Current Sprint Status**

**Sprint 21**: Flight Mechanics - **UNDER ARCHITECTURAL REVIEW**
- **Active**: [Sprint 21 Design](sprints/active/SPRINT_21_DESIGN.md) ⚠️ Requires scope adjustment
- **Issue**: Over-scoping and entity-agnostic design missing
- **Recommendation**: Split into focused sub-sprints with capability composition architecture

## 🔗 **Quick Navigation**

### **Development Documentation**
- **[Developer Manual](development/DEVELOPER_MANUAL.md)** - Complete development guide and coding standards
- **[Asset Pipeline](development/ASSET_PIPELINE.md)** - Asset creation and compilation guide

### **Project Documentation**
- **[Game Vision](project/GAME_VISION.md)** - Core game design and vision document
- **[Player Manual](manual/PLAYER_MANUAL.md)** - Gameplay and controls guide for "The Ghost Signal"
- **[Project Status](PROJECT_STATUS_CURRENT.md)** - Current engine state assessment

### **Technical Analysis**
- **[Flight Mechanics Analysis](FLIGHT_MECHANICS_ANALYSIS.md)** - Current flight system technical review
- **[Scene Overhaul](SCENE_OVERHAUL_COMPLETED.md)** - FTL navigation system implementation

### **Sprint Management**
- **[Active Sprints](sprints/active/)** - Current sprint designs and plans
- **[Completed Sprints](sprints/completed/)** - Historical sprint documentation
- **[Sprint Backlog](project/sprints/backlog/)** - Future sprint planning

## 📁 **Documentation Structure**

```
docs/
├── sprints/
│   ├── active/                    # Current sprint designs
│   │   └── SPRINT_21_DESIGN.md    # Flight mechanics (under review)
│   └── completed/                 # Completed sprint documentation
│       ├── SPRINT_19_COMPLETE.md  # Performance optimization
│       └── SPRINT_20_COMPLETE.md  # Lighting & materials
├── project/
│   ├── GAME_VISION.md             # Core game design document
│   ├── lore/                      # Game world and narrative
│   ├── research/                  # Technical research documents
│   └── sprints/                   # Sprint planning and backlog
├── development/
│   ├── DEVELOPER_MANUAL.md        # Development guide
│   └── ASSET_PIPELINE.md          # Asset workflow
├── manual/
│   └── PLAYER_MANUAL.md           # User documentation
├── archive/                       # Historical planning documents
│   ├── SPRINT_19_REFACTOR_PHASE.md
│   ├── SPRINT_19_TDD_PLAN.md
│   └── SPRINT_20_LIGHTING_PLAN.md
├── ARCHITECTURE_ANALYSIS_FLIGHT_MECHANICS.md  # Critical architectural review
├── CRITICAL_FINDINGS_SUMMARY.md              # Executive summary of issues
├── FLIGHT_MECHANICS_ANALYSIS.md              # Technical flight system analysis
├── PROJECT_STATUS_CURRENT.md                 # Current project state
└── _posts/                                   # Blog posts and announcements
```

## 🎯 **For LLMs and AI Assistants**

This documentation is optimized for both human and LLM consumption:

### **Document Types and Purpose**
- **Analysis Documents**: Technical deep-dives with architectural recommendations
- **Sprint Documents**: Implementation plans with task breakdowns and success criteria
- **Status Documents**: Current state assessments and progress tracking
- **Vision Documents**: High-level design and strategic direction

### **Navigation Principles**
- **Cross-References**: Documents link to related content for full context
- **Status Indicators**: Clear flags for document currency and action requirements
- **Hierarchical Structure**: Logical organization from high-level to implementation details
- **Technical Depth**: Detailed specifications for implementation guidance

### **Critical Information Handling**
- **Urgent Issues**: Prominently flagged with 🚨 indicators
- **Dependencies**: Clear identification of document relationships
- **Architectural Concerns**: Detailed analysis with specific recommendations
- **Implementation Guidance**: Step-by-step technical specifications

## 📊 **Documentation Maintenance Standards**

**Archivist Role**: Documentation maintained for comprehensive accessibility and LLM/human collaboration:

### **Organization Principles**
- ✅ Completed documents moved to appropriate archive/completed folders
- ✅ Active documents clearly marked with status and dependencies  
- ✅ Critical issues flagged prominently for immediate attention
- ✅ Cross-references maintained for efficient navigation
- ✅ Technical specifications detailed for implementation guidance

### **Quality Standards**
- **Consistency**: Uniform formatting and structure across all documents
- **Completeness**: Comprehensive coverage of technical and design aspects
- **Currency**: Regular updates reflecting current project state
- **Accessibility**: Clear language and structure for both human and AI readers

### **Update Protocols**
- **Sprint Completion**: Move completed sprint docs to completed folder
- **Critical Issues**: Create immediate action summaries for urgent matters
- **Architectural Changes**: Document analysis and recommendations comprehensively
- **Status Changes**: Update project status and cross-references accordingly

---

**📋 Current Priority**: Review and resolve architectural concerns in Sprint 21 design before implementation begins. See critical findings documents for immediate action items.**