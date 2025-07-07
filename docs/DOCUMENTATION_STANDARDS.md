# Documentation Standards

**Document ID**: DOC_STANDARDS_V1  
**Date**: July 6, 2025  
**Author**: Development Team

## 📋 File Naming Conventions

### Sprint Documentation
- **Format**: `SPRINT_XX_FEATURE_NAME.md`
- **Examples**: 
  - `SPRINT_26_SEMI_AUTONOMOUS_FLIGHT.md`
  - `SPRINT_25_CANYON_RACING_PROTOTYPE.md`
- **Location**: `docs/sprints/active/` (during sprint), `docs/sprints/completed/` (after)

### Research Documentation
- **Format**: `RES_TOPIC_NAME.md` (individual) or `TOPIC_RESEARCH.md` (consolidated)
- **Examples**:
  - `FLIGHT_CONTROL_RESEARCH.md` (consolidated)
  - `RES_A_DRIVE_PROPULSION.md` (specific research)
- **Location**: `docs/project/research/`

### Technical Documentation
- **Format**: `SYSTEM_NAME.md` or descriptive names
- **Examples**:
  - `ARCHITECTURE_OVERVIEW.md`
  - `COMPONENT_CATALOG.md`
  - `PERFORMANCE_GUIDE.md`
- **Location**: `docs/technical/`

### User Documentation
- **Format**: Descriptive names focused on user tasks
- **Examples**:
  - `FLIGHT_CONTROLS.md`
  - `GETTING_STARTED.md`
  - `KEYBOARD_SHORTCUTS.md`
- **Location**: `docs/manual/`

---

## 📄 Required Document Sections

### All Documents Must Include:

#### 1. Metadata Header
```markdown
# Document Title

**Document ID**: UNIQUE_IDENTIFIER  
**Date**: YYYY-MM-DD  
**Author**: Team/Individual Name  
**Status**: Draft | Active | Complete | Superseded  
**Last Updated**: YYYY-MM-DD  

[Optional version/revision info]
```

#### 2. Table of Contents (for long documents)
```markdown
## Table of Contents
1. [Overview](#overview)
2. [Technical Details](#technical-details)
3. [Implementation](#implementation)
4. [Testing](#testing)
5. [Future Considerations](#future-considerations)
```

### Sprint Documentation Requirements

#### Essential Sections:
1. **🎯 Vision & Objectives** - Clear goals and success metrics
2. **📋 Technical Architecture** - System design and integration points
3. **🗓️ Implementation Plan** - Timeline with specific deliverables
4. **🧪 Testing Strategy** - How success will be validated
5. **📊 Success Criteria** - Measurable outcomes
6. **🚧 Risk Assessment** - Potential issues and mitigation
7. **📝 Deliverables** - Concrete outputs

#### Template:
```markdown
# Sprint XX: Feature Name

**Duration**: X weeks (Y days)  
**Start Date**: YYYY-MM-DD  
**End Date**: YYYY-MM-DD  
**Priority**: High | Medium | Low

## 🎯 Vision & Objectives
[Clear description of what will be built and why]

## 📋 Technical Architecture
[System design, components, integration points]

## 🗓️ Implementation Plan
[Week-by-week breakdown with deliverables]

## 🧪 Testing Strategy
[How the feature will be validated]

## 📊 Success Criteria
[Measurable outcomes and acceptance criteria]

## 🚧 Risk Assessment
[Potential issues and mitigation strategies]

## 📝 Deliverables
[Concrete outputs: code, docs, tests]
```

### Technical Documentation Requirements

#### Essential Sections:
1. **Overview** - Purpose and scope
2. **Architecture** - High-level design
3. **API Reference** - Function/component interfaces
4. **Integration** - How it connects to other systems
5. **Performance** - Characteristics and optimization
6. **Examples** - Usage patterns and code samples

### Research Documentation Requirements

#### Essential Sections:
1. **Research Question** - What problem is being solved
2. **Literature Review** - Existing solutions and approaches
3. **Analysis** - Evaluation of options
4. **Recommendations** - Preferred approach with rationale
5. **Implementation Notes** - Practical considerations
6. **References** - Sources and further reading

---

## 🔗 Linking Conventions

### Internal Links (within repository)
```markdown
<!-- Source code files -->
[component.h](../../src/component/component.h)

<!-- Other documentation -->
[Sprint 26](../sprints/active/SPRINT_26_SEMI_AUTONOMOUS_FLIGHT.md)

<!-- Relative to current document -->
[Architecture](./ARCHITECTURE_OVERVIEW.md)
```

### External Links
```markdown
<!-- External references with context -->
[Elite Dangerous Flight Assist](https://elite-dangerous.fandom.com/wiki/Flight_Assist) - Reference implementation

<!-- Academic papers -->
[PID Control Theory](https://doi.org/10.1109/control.2005.1630) - Theoretical foundation
```

### Code References
```markdown
<!-- Specific file and line numbers when relevant -->
[UnifiedFlightControl](../../src/component/unified_flight_control.h#L25-L40) - Component definition

<!-- Function or struct references -->
[physics_system_update()](../../src/system/physics.c#L156) - Main update function
```

---

## 📝 Writing Guidelines

### Tone and Style
- **Professional but Accessible**: Clear technical communication
- **Action-Oriented**: Focus on what to do, not just what exists
- **Specific**: Provide concrete examples and code snippets
- **Concise**: Respect reader's time, use bullet points and tables

### Code Examples
- **Complete**: Show full context, not just fragments
- **Commented**: Explain non-obvious parts
- **Tested**: Ensure code examples actually work
- **Consistent**: Follow project coding standards

### Visual Elements
- **Diagrams**: Use mermaid for flow charts and architecture diagrams
- **Tables**: For structured comparisons and reference data
- **Emojis**: Sparingly, for section headers and visual organization
- **Code Blocks**: Specify language for syntax highlighting

---

## 🔄 Document Lifecycle

### Creation Process
1. **Draft**: Author creates initial version with required sections
2. **Review**: Technical review by relevant team members
3. **Approval**: Sprint lead or technical lead approves
4. **Publication**: Move to appropriate directory structure

### Maintenance Process
1. **Regular Review**: Documents reviewed during sprint retrospectives
2. **Update Triggers**: Code changes, architectural decisions, user feedback
3. **Version Control**: Use git for change tracking
4. **Deprecation**: Clearly mark superseded documents

### Status Tracking
```markdown
**Status**: Draft | Review | Active | Complete | Superseded
**Last Reviewed**: YYYY-MM-DD
**Next Review**: YYYY-MM-DD
```

---

## 🤖 Automation & Validation

### Automated Checks
Create validation script: `docs/scripts/validate_docs.sh`

```bash
#!/bin/bash
# Document validation script

echo "Checking documentation standards..."

# Check for required metadata headers
find docs -name "*.md" | while read file; do
    if ! grep -q "Document ID" "$file"; then
        echo "❌ Missing Document ID: $file"
    fi
    if ! grep -q "Date:" "$file"; then
        echo "❌ Missing Date: $file"
    fi
done

# Check for broken internal links
find docs -name "*.md" -exec grep -l "\]\(\.\./" {} \; | while read file; do
    grep -o "](\.\.\/[^)]*)" "$file" | while read link; do
        target=$(echo "$link" | sed 's/](//;s/)//')
        full_path="$(dirname "$file")/$target"
        if [ ! -f "$full_path" ]; then
            echo "❌ Broken link in $file: $target"
        fi
    done
done

# Check for outdated sprint references
current_sprint=26
find docs -name "*.md" -exec grep -l "Sprint [0-9]" {} \; | while read file; do
    if ! grep -q "completed\|archive" "$file"; then
        old_sprints=$(grep -o "Sprint [0-9][0-9]*" "$file" | grep -v "Sprint $current_sprint" | sort -u)
        if [ -n "$old_sprints" ]; then
            echo "⚠️  Potential outdated sprint references in $file: $old_sprints"
        fi
    fi
done

echo "Documentation validation complete."
```

### Link Validation
```bash
# Check that referenced source files exist
grep -r "src/.*\.h" docs/ | while IFS=: read file link; do
    clean_link=$(echo "$link" | grep -o "src/[^)]*.h")
    if [ ! -f "$clean_link" ]; then
        echo "❌ Dead source link in $file: $clean_link"
    fi
done
```

---

## 📊 Quality Metrics

### Documentation Quality Checklist
- [ ] All required sections present
- [ ] Clear, actionable language
- [ ] Code examples tested and working
- [ ] Internal links verified
- [ ] Up-to-date with current implementation
- [ ] Reviewed by team members
- [ ] Proper formatting and structure

### Review Criteria
1. **Accuracy**: Information matches current codebase
2. **Completeness**: All required sections included
3. **Clarity**: Technical concepts explained clearly
4. **Usefulness**: Helps reader accomplish their goals
5. **Maintainability**: Easy to update as code evolves

---

*Following these standards ensures consistent, high-quality documentation that serves both current team members and future contributors.*

**See Also**:
- [Architecture Overview](technical/ARCHITECTURE_OVERVIEW.md)
- [Sprint Planning Template](sprints/SPRINT_TEMPLATE.md)
- [Code Style Guide](../CODE_STYLE.md)
