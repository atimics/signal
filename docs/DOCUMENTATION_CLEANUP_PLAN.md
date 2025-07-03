# Documentation Organization Cleanup - July 2, 2025

## 🗂️ Current Issues Identified

The CGame project documentation has grown organically and now suffers from:

1. **Multiple Competing Systems:**
   - `/docs/` (main docs)
   - `/docs/SIGNAL/` (classified project docs)
   - `/docs/sprints/` (sprint tracking)
   - `/docs/backlog/` (task management)

2. **Inconsistent Naming:**
   - Mixed case conventions
   - Date formats vary
   - Status indicators unclear

3. **Redundant Information:**
   - Sprint status in multiple locations
   - Overlapping architecture documents
   - Duplicate implementation guides

---

## 🎯 Proposed Organization Structure

### Primary Documentation (`/docs/`)
```
docs/
├── README.md                    # Main project documentation
├── ARCHITECTURE.md             # High-level system design
├── DEVELOPMENT_GUIDE.md        # Developer onboarding
├── API_REFERENCE.md            # Code documentation
└── CHANGELOG.md                # Release history
```

### Active Development (`/docs/active/`)
```
docs/active/
├── CURRENT_SPRINT.md           # Current sprint status
├── BLOCKERS.md                 # Active issues tracking  
├── DAILY_STANDUP.md            # Daily progress notes
└── QUICK_WINS.md               # Small improvements list
```

### Sprint Management (`/docs/sprints/`)
```
docs/sprints/
├── README.md                   # Sprint process overview
├── SPRINT_22_CURRENT.md        # Active sprint details
├── completed/                  # Archived sprint data
│   ├── sprint_21/
│   └── sprint_20/
└── templates/                  # Sprint templates
    └── SPRINT_TEMPLATE.md
```

### Research & Planning (`/docs/research/`)
```
docs/research/
├── ODE_PHYSICS_INTEGRATION.md  # Technical research
├── NEURAL_INPUT_ANALYSIS.md    # Algorithm research
├── PERFORMANCE_BENCHMARKS.md   # Performance studies
└── ARCHITECTURE_DECISIONS.md   # Design decisions log
```

### Archive (`/docs/archive/`)
```
docs/archive/
├── legacy_backup/              # Old system backups
├── deprecated/                 # Outdated documents
└── completed_features/         # Finished feature docs
```

---

## 🔧 Cleanup Actions Required

### Immediate (Today)
1. **Consolidate Sprint Status:**
   - Merge `/docs/sprints/CURRENT_SPRINT_STATUS.md` with `/docs/SIGNAL/active_sprint/`
   - Create single source of truth

2. **Archive Obsolete Docs:**
   - Move old analysis files to `/archive/`
   - Clean up root directory clutter

3. **Standardize Naming:**
   - Convert to consistent `SNAKE_CASE.md` pattern
   - Add clear status indicators

### Short-term (This Week)
1. **Restructure SIGNAL Directory:**
   - Keep sensitive/classified content separate
   - Link to main docs where appropriate
   - Clear classification levels

2. **Create Master Index:**
   - Document location guide
   - Quick reference for developers
   - Maintenance responsibilities

### Long-term (Next Sprint)
1. **Automated Documentation:**
   - API docs generation from code
   - Sprint report automation
   - Link validation

2. **Documentation Standards:**
   - Style guide for technical writing
   - Template system for common docs
   - Review process for changes

---

## 📝 Implementation Plan

### Phase 1: Emergency Cleanup (2 hours)
- [ ] Move obsolete files to `/archive/`
- [ ] Consolidate sprint documentation
- [ ] Create navigation README files

### Phase 2: Restructure (1 day)
- [ ] Implement new directory structure
- [ ] Update all cross-references
- [ ] Test all documentation links

### Phase 3: Standards (3 days)
- [ ] Create documentation style guide
- [ ] Implement automated checks
- [ ] Train team on new structure

---

## 🎯 Success Metrics

### Immediate
- [ ] Single sprint status source established
- [ ] <5 minutes to find any document
- [ ] No broken internal links

### Long-term
- [ ] 100% of new docs follow standards
- [ ] Automated freshness checking
- [ ] Developer satisfaction >90%

---

## 🚨 Quick Fixes Needed

### Files to Archive Immediately
- `CRITICAL_FINDINGS_SUMMARY.md` (obsolete)
- `CMAKE_TRANSITION_PLAN.md` (superseded)
- `SCENE_OVERHAUL_COMPLETED.md` (archived)
- Multiple `*_ANALYSIS.md` files (research archive)

### Files to Consolidate
- Sprint status scattered across 3 locations
- Architecture docs in multiple formats
- Implementation guides with overlap

### Files to Standardize
- Inconsistent date formats
- Mixed status indicators
- Unclear ownership

---

**Next Action:** Begin Phase 1 cleanup immediately to support Sprint 22 progress tracking.

**Owner:** Development Team  
**Timeline:** Phase 1 complete by end of day  
**Review:** July 3, 2025  
