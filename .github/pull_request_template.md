---
name: Pull Request Template
about: Standard template for all pull requests
title: ''
labels: ''
assignees: ''
---

## 🎯 Summary
<!-- Brief description of changes and motivation -->

## 📋 Type of Change
<!-- Select the type that applies -->
- [ ] 🐛 Bug fix (patch - non-breaking change that fixes an issue)
- [ ] ✨ New feature (minor - new functionality that is backward compatible)
- [ ] 💥 Breaking change (major - changes that break backward compatibility)
- [ ] 📖 Documentation update
- [ ] 🔧 Maintenance/refactor (no functional changes)
- [ ] 🧪 Test improvements
- [ ] ⚡ Performance optimization

## 🧪 Testing Checklist
<!-- All items must be checked before merge -->
- [ ] Unit tests added/updated and passing
- [ ] Manual testing completed
- [ ] Performance impact assessed (if applicable)
- [ ] Cross-platform compatibility verified (macOS/Linux)
- [ ] Memory usage tested (if applicable)
- [ ] No compiler warnings introduced

## 📖 Documentation
- [ ] API documentation updated (if applicable)
- [ ] CHANGELOG.md updated
- [ ] README.md updated (if needed)
- [ ] Code comments added for complex logic

## 🔗 Related Issues
<!-- Link any related issues -->
Closes #
Related to #

## 🚀 Release Impact
<!-- For main branch PRs -->
- [ ] Version bump required (patch/minor/major)
- [ ] Release notes prepared
- [ ] Breaking changes documented (if applicable)

## 📊 Performance Metrics
<!-- Complete if changes affect performance -->
- **Before**: (FPS, memory usage, etc.)
- **After**: (FPS, memory usage, etc.)
- **Impact**: (positive/negative/neutral)

## 🔍 Quality Gate Requirements

### For PRs to `develop`:
- [ ] All CI builds pass (macOS, Linux)
- [ ] Unity test suite passes (100% success rate)
- [ ] Code review from 1+ maintainer
- [ ] Branch up-to-date with target
- [ ] Conventional commit format used

### For PRs to `main` (Additional Requirements):
- [ ] All CI builds pass (macOS, Linux)
- [ ] Unity test suite passes (100% success rate) 
- [ ] Performance regression tests pass
- [ ] Manual user verification completed (see checklist below)
- [ ] Code review from 2+ maintainers
- [ ] Release notes prepared
- [ ] Version number incremented in package.json

## 🎮 Manual Verification Checklist
<!-- Required for main branch PRs -->
<details>
<summary>Click to expand manual verification requirements</summary>

### Engine Startup & Core Systems:
- [ ] Engine starts without errors
- [ ] All systems initialize correctly  
- [ ] Memory system reports expected values
- [ ] Performance monitoring active

### Asset Pipeline:
- [ ] Assets load successfully
- [ ] Texture rendering works correctly
- [ ] Mesh rendering displays properly
- [ ] Material assignments are correct

### Scene Management:
- [ ] Scene transitions work smoothly
- [ ] Entity spawning functions correctly
- [ ] Camera system operates as expected
- [ ] UI responds to user input

### Performance & Stability:
- [ ] Frame rate stable (60+ FPS target)
- [ ] Memory usage within expected bounds
- [ ] No crashes during 5-minute test session
- [ ] Hot-reload functionality works (if enabled)

### Cross-Platform Compatibility:
- [ ] macOS build functions correctly
- [ ] Linux build functions correctly
- [ ] WebAssembly build loads and runs (when implemented)

</details>

## 🔒 Security Considerations
<!-- Complete if changes affect security -->
- [ ] No sensitive data exposed
- [ ] Input validation implemented
- [ ] Memory safety verified
- [ ] No new attack vectors introduced

## 📸 Screenshots/Videos
<!-- Add visual documentation if applicable -->

## ⚠️ Breaking Changes
<!-- Detail any breaking changes -->
- None
<!-- OR describe breaking changes and migration path -->

## 📝 Additional Notes
<!-- Any additional context, concerns, or implementation details -->

---

## Reviewer Checklist
<!-- For reviewers - ensure all applicable items are verified -->
- [ ] Code follows project style guidelines
- [ ] Tests are comprehensive and meaningful
- [ ] Documentation is clear and complete
- [ ] Performance impact is acceptable
- [ ] Security implications considered
- [ ] Breaking changes properly documented
- [ ] Version bump is appropriate
