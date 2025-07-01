# CGame Release & Branch Management Standards

**Version**: 1.0  
**Date**: July 1, 2025  
**Author**: CGame Development Team

## 🌳 Branch Strategy

### **Main Branches**

#### `main` Branch
- **Purpose**: Production-ready code only
- **Protection**: ✅ Protected branch with required reviews
- **Triggers**: Automated releases via semantic versioning
- **Quality Gate**: All tests must pass + manual verification
- **Direct Commits**: ❌ Prohibited (PR only)

#### `develop` Branch  
- **Purpose**: Integration branch for feature development
- **Protection**: ✅ Protected with required CI checks
- **Triggers**: Automated testing and preview builds
- **Quality Gate**: All automated tests must pass
- **Direct Commits**: ✅ Allowed for maintainers

### **Supporting Branches**

#### Feature Branches: `feature/sprint-X-task-Y-description`
- **Purpose**: Individual feature development
- **Lifetime**: Created from `develop`, merged back to `develop`
- **Example**: `feature/sprint-19-memory-optimization`
- **CI**: Full test suite on every push

#### Hotfix Branches: `hotfix/vX.Y.Z-description`
- **Purpose**: Critical production fixes
- **Lifetime**: Created from `main`, merged to both `main` and `develop`
- **Example**: `hotfix/v1.2.1-memory-leak-fix`
- **Release**: Triggers immediate patch release

#### Release Branches: `release/vX.Y.0`
- **Purpose**: Release preparation and stabilization
- **Lifetime**: Created from `develop`, merged to `main` after testing
- **Example**: `release/v1.3.0`
- **Activities**: Bug fixes, documentation updates, version bumps

## 🚀 Release Process

### **Semantic Versioning (SemVer)**

CGame follows strict semantic versioning: `MAJOR.MINOR.PATCH`

- **MAJOR** (`X.0.0`): Breaking API changes, major architecture updates
- **MINOR** (`x.Y.0`): New features, backward-compatible changes
- **PATCH** (`x.y.Z`): Bug fixes, security updates, performance improvements

#### Version Increment Rules:
```bash
# PATCH: Bug fixes, optimizations, security patches
v1.2.3 → v1.2.4

# MINOR: New features, Sprint completions, API additions  
v1.2.4 → v1.3.0

# MAJOR: Breaking changes, API redesigns, architecture overhauls
v1.3.0 → v2.0.0
```

### **Release Types**

#### 🏷️ **Stable Release** (`vX.Y.0`)
- **Trigger**: Completed sprint milestones
- **Source**: `release/vX.Y.0` branch → `main`
- **Artifacts**: Full binaries, documentation, changelog
- **Distribution**: GitHub Releases, tagged commits

#### 🔧 **Patch Release** (`vX.Y.Z`)
- **Trigger**: Critical bug fixes, security updates
- **Source**: `hotfix/vX.Y.Z-*` branch → `main`  
- **Artifacts**: Updated binaries, patch notes
- **Timeline**: Within 24 hours of critical issues

#### 🧪 **Preview Release** (`vX.Y.0-alpha.N`, `vX.Y.0-beta.N`)
- **Trigger**: Feature branch merges to `develop`
- **Source**: `develop` branch
- **Artifacts**: Development binaries, preview documentation
- **Purpose**: Early testing, stakeholder feedback

## 🔒 Quality Gates

### **Pre-commit Requirements**
```bash
# Mandatory checks before ANY commit
✅ Code compiles without warnings (make clean && make)
✅ All existing tests pass (make test)
✅ Code follows style guidelines (.clang-format)
✅ No memory leaks (valgrind on Linux builds)
✅ Documentation updated for API changes
```

### **Pull Request Requirements**

#### To `develop` Branch:
```yaml
Required Checks:
  ✅ All CI builds pass (macOS, Linux, WebAssembly)
  ✅ Unity test suite passes (100% success rate)
  ✅ Code review from 1+ maintainer
  ✅ Branch up-to-date with target
  ✅ Conventional commit format
```

#### To `main` Branch:
```yaml
Required Checks:
  ✅ All CI builds pass (macOS, Linux, WebAssembly)  
  ✅ Unity test suite passes (100% success rate)
  ✅ Performance regression tests pass
  ✅ Manual user verification completed
  ✅ Code review from 2+ maintainers
  ✅ Release notes prepared
  ✅ Version number incremented
```

### **Manual Verification Process**

#### 🎮 **User Verification Checklist**
Before any release to `main`, complete the following manual tests:

##### Engine Startup & Core Systems:
- [ ] Engine starts without errors
- [ ] All systems initialize correctly
- [ ] Memory system reports expected values
- [ ] Performance monitoring active

##### Asset Pipeline:
- [ ] Assets load successfully
- [ ] Texture rendering works correctly  
- [ ] Mesh rendering displays properly
- [ ] Material assignments are correct

##### Scene Management:
- [ ] Scene transitions work smoothly
- [ ] Entity spawning functions correctly
- [ ] Camera system operates as expected
- [ ] UI responds to user input

##### Performance & Stability:
- [ ] Frame rate stable (60+ FPS target)
- [ ] Memory usage within expected bounds
- [ ] No crashes during 5-minute test session
- [ ] Hot-reload functionality works (if enabled)

##### Cross-Platform Compatibility:
- [ ] macOS build functions correctly
- [ ] Linux build functions correctly  
- [ ] WebAssembly build loads and runs (when implemented)

## 🤖 Automated Release Pipeline

### **GitHub Actions Workflows**

#### `release.yml` - Production Release
```yaml
Triggers:
  - Push to main branch
  - Manual workflow dispatch

Jobs:
  1. Build & Test (all platforms)
  2. Generate Release Notes
  3. Create GitHub Release
  4. Upload Release Artifacts
  5. Update Documentation
  6. Notify Stakeholders
```

#### `develop.yml` - Development Integration  
```yaml
Triggers:
  - Push to develop branch
  - Pull request to develop

Jobs:
  1. Build & Test (all platforms)
  2. Generate Preview Build
  3. Run Performance Benchmarks
  4. Update Development Docs
```

#### `security.yml` - Security Scanning
```yaml
Triggers:
  - Daily schedule
  - Pull request to main/develop

Jobs:
  1. Dependency vulnerability scan
  2. Static code analysis
  3. Secret detection
  4. License compliance check
```

### **Release Artifact Standards**

#### 📦 **Required Artifacts**
```
cgame-vX.Y.Z-release/
├── binaries/
│   ├── cgame-macos-arm64
│   ├── cgame-macos-x64  
│   ├── cgame-linux-x64
│   └── cgame-wasm/
│       ├── cgame.html
│       ├── cgame.js
│       ├── cgame.wasm
│       └── assets/
├── documentation/
│   ├── CHANGELOG.md
│   ├── API_REFERENCE.md
│   ├── INSTALLATION.md
│   └── MIGRATION_GUIDE.md (for breaking changes)
├── examples/
│   ├── basic-setup/
│   └── sample-game/
└── metadata/
    ├── RELEASE_NOTES.md
    ├── CHECKSUMS.sha256
    └── SIGNATURES.asc
```

## 📋 Commit & PR Standards

### **Conventional Commits**
All commits must follow the [Conventional Commits](https://www.conventionalcommits.org/) specification:

```bash
# Format
<type>[optional scope]: <description>

[optional body]

[optional footer(s)]

# Examples
feat(memory): implement intelligent asset unloading system
fix(rendering): resolve texture binding memory leak  
docs(api): update ECS component documentation
test(lod): add comprehensive LOD system test suite
chore(ci): update GitHub Actions to latest versions
```

#### Commit Types:
- `feat`: New features
- `fix`: Bug fixes  
- `docs`: Documentation changes
- `test`: Test additions/modifications
- `refactor`: Code refactoring
- `perf`: Performance improvements
- `chore`: Maintenance tasks
- `ci`: CI/CD changes

### **Pull Request Template**
```markdown
## 🎯 Summary
Brief description of changes and motivation.

## 🧪 Testing
- [ ] Unit tests added/updated
- [ ] Manual testing completed
- [ ] Performance impact assessed
- [ ] Cross-platform compatibility verified

## 📖 Documentation  
- [ ] API documentation updated
- [ ] CHANGELOG.md updated
- [ ] README.md updated (if needed)

## 🔗 Related Issues
Closes #123
Related to #456

## 📸 Screenshots/Videos
(If applicable)

## ⚠️ Breaking Changes
(If any - requires MAJOR version bump)
```

## 🚨 Emergency Procedures

### **Critical Bug Response**
1. **Detection**: Issue reported or identified
2. **Assessment**: Severity classification (Critical/High/Medium/Low)
3. **Response Time**: 
   - Critical: 2 hours
   - High: 24 hours
   - Medium: 1 week
   - Low: Next sprint
4. **Hotfix Process**: Create hotfix branch, implement fix, emergency release

### **Security Vulnerability Response**
1. **Private Disclosure**: Handle security issues privately
2. **Assessment**: CVSS scoring and impact analysis
3. **Patch Development**: Coordinated fix development
4. **Coordinated Disclosure**: Public disclosure with fix
5. **Post-Mortem**: Security review and process improvement

## 📊 Release Metrics & KPIs

### **Quality Metrics**
- **Test Coverage**: >95% for core systems
- **Build Success Rate**: >99% for main/develop branches
- **Time to Release**: <2 hours from main branch push
- **Issue Resolution Time**: <24 hours for critical bugs

### **Performance Benchmarks**
- **Engine Startup Time**: <500ms
- **Asset Loading Time**: <100ms per mesh
- **Memory Usage**: <256MB for standard scenes
- **Frame Rate**: >60 FPS consistently

## 🛠️ Tools & Automation

### **Required Tools**
- **Git Flow**: Branch management
- **GitHub CLI**: Release automation
- **Semantic Release**: Version management
- **Conventional Changelog**: Release notes generation
- **GitHub Actions**: CI/CD pipeline

### **Development Environment**
- **Pre-commit Hooks**: Automated code quality checks
- **IDE Integration**: VS Code with CGame extensions
- **Static Analysis**: Clang static analyzer
- **Memory Analysis**: Valgrind, AddressSanitizer

---

## 📚 References

- [Git Flow Workflow](https://nvie.com/posts/a-successful-git-branching-model/)
- [Conventional Commits](https://www.conventionalcommits.org/)
- [Semantic Versioning](https://semver.org/)
- [GitHub Flow](https://guides.github.com/introduction/flow/)
- [Keep a Changelog](https://keepachangelog.com/)

---

**Document Owner**: CGame Development Team  
**Last Updated**: July 1, 2025  
**Next Review**: September 1, 2025
