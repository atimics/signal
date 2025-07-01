# GitHub Repository Configuration Guide

This document provides step-by-step instructions for configuring GitHub repository settings to enforce the CGame release and branch management standards.

## 🔒 Branch Protection Rules

### Main Branch Protection

Navigate to **Settings** → **Branches** → **Add rule** for `main`:

```yaml
Branch name pattern: main
Settings:
  ✅ Restrict pushes that create files larger than 100MB
  ✅ Require a pull request before merging
    ✅ Require approvals: 2
    ✅ Dismiss stale PR approvals when new commits are pushed
    ✅ Require review from code owners
    ✅ Restrict reviews to users with explicit read or higher access
  ✅ Require status checks to pass before merging
    ✅ Require branches to be up to date before merging
    Required status checks:
      - Build (macOS)
      - Build (Linux)  
      - Unity Framework Tests
      - Cross-Platform Tests (ubuntu-latest)
      - Cross-Platform Tests (macos-latest)
  ✅ Require conversation resolution before merging
  ✅ Require signed commits
  ✅ Require linear history
  ✅ Require deployments to succeed before merging
    Required deployment environments:
      - production (manual approval required)
  ✅ Lock branch (prevent direct pushes)
  ✅ Do not allow bypassing the above settings
  ✅ Restrict pushes that create files larger than 100MB
```

### Develop Branch Protection

Navigate to **Settings** → **Branches** → **Add rule** for `develop`:

```yaml
Branch name pattern: develop
Settings:
  ✅ Require a pull request before merging
    ✅ Require approvals: 1
    ✅ Dismiss stale PR approvals when new commits are pushed
  ✅ Require status checks to pass before merging
    ✅ Require branches to be up to date before merging
    Required status checks:
      - Build & Test Preview (macos)
      - Build & Test Preview (linux)
  ✅ Require conversation resolution before merging
  ✅ Allow force pushes: ❌ Everyone
  ✅ Allow deletions: ❌
```

## 🌍 Environment Protection Rules

### Production Environment

Navigate to **Settings** → **Environments** → **New environment** → `production`:

```yaml
Environment name: production
Protection rules:
  ✅ Required reviewers: 
    - @maintainer-team
    - @lead-developer
  ✅ Wait timer: 0 minutes
  ✅ Prevent self-review: ✅
  Environment secrets: (none required)
  Environment variables: (none required)
```

## 🤖 GitHub Actions Settings

### Workflow Permissions

Navigate to **Settings** → **Actions** → **General**:

```yaml
Actions permissions:
  ○ Disable actions
  ○ Allow enterprise actions only
  ○ Allow marketplace actions by verified creators
  ● Allow all actions and reusable workflows

Workflow permissions:
  ○ Read repository contents and packages permissions
  ● Read and write permissions
  ✅ Allow GitHub Actions to create and approve pull requests

Fork pull request workflows:
  ✅ Run workflows from fork pull requests
  ○ Send write tokens to workflows from fork pull requests  
  ○ Send secrets to workflows from fork pull requests
```

### Artifact and Log Retention

```yaml
Artifact and log retention:
  - Default artifact retention: 90 days
  - Default workflow run retention: 400 days
```

## 📋 Repository Settings

### General Settings

Navigate to **Settings** → **General**:

```yaml
Repository name: cgame
Description: High-Performance C Game Engine with Entity-Component-System Architecture
Website: (optional)
Topics: c, game-engine, ecs, graphics, 3d, sokol, performance

Features:
  ✅ Wikis
  ✅ Issues  
  ✅ Sponsorships
  ✅ Preserve this repository
  ✅ Projects
  ✅ Discussions

Pull Requests:
  ✅ Allow merge commits
  ✅ Allow squash merging
    Default to: Default commit message
  ✅ Allow rebase merging  
  ✅ Always suggest updating pull request branches
  ✅ Allow auto-merge
  ✅ Automatically delete head branches

Pushes:
  ✅ Limit pushes that create files larger than: 100 MB
```

### Code Security and Analysis

Navigate to **Settings** → **Code security and analysis**:

```yaml
Private vulnerability reporting: ✅ Enable
Dependency graph: ✅ Enable
Dependabot alerts: ✅ Enable
Dependabot security updates: ✅ Enable
Dependabot version updates: ✅ Enable (configure in .github/dependabot.yml)
Code scanning: ✅ Enable (configure CodeQL)
Secret scanning: ✅ Enable
Secret scanning push protection: ✅ Enable
```

## 🏷️ Issue and PR Templates

### Issue Templates

Create `.github/ISSUE_TEMPLATE/`:

1. **Bug Report** (`bug_report.yml`)
2. **Feature Request** (`feature_request.yml`) 
3. **Performance Issue** (`performance.yml`)
4. **Documentation** (`documentation.yml`)

### Pull Request Template

Create `.github/pull_request_template.md`:

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
Closes #XXX
Related to #XXX

## ⚠️ Breaking Changes
(If any - requires MAJOR version bump)

---

**Checklist before requesting review:**
- [ ] Code follows project style guidelines
- [ ] Self-review completed
- [ ] Comments added for complex logic
- [ ] Tests cover the changes
- [ ] All CI checks pass
```

## 📊 Status Badges

Add to README.md:

```markdown
[![Build Status](https://github.com/atimics/cgame/workflows/Build%20and%20Test/badge.svg)](https://github.com/atimics/cgame/actions)
[![Test Suite](https://github.com/atimics/cgame/workflows/Test%20Suite/badge.svg)](https://github.com/atimics/cgame/actions)
[![Release](https://github.com/atimics/cgame/workflows/Release%20Pipeline/badge.svg)](https://github.com/atimics/cgame/releases)
```

## 🔐 Required Secrets

Navigate to **Settings** → **Secrets and variables** → **Actions**:

Currently no secrets are required. All workflows use the default `GITHUB_TOKEN`.

## 📝 Additional Configuration Files

Ensure these files exist in your repository:

- `.github/workflows/build.yml` ✅
- `.github/workflows/test.yml` ✅  
- `.github/workflows/release.yml` ✅
- `.github/workflows/preview.yml` ✅
- `.github/copilot-instructions.md` ✅
- `docs/RELEASE_STANDARDS.md` ✅
- `CONTRIBUTING.md` ✅
- `package.json` ✅

---

## 🚀 Quick Setup Checklist

1. [ ] Configure main branch protection with 2 required reviewers
2. [ ] Configure develop branch protection with 1 required reviewer  
3. [ ] Create production environment with manual approval
4. [ ] Enable required status checks for all CI jobs
5. [ ] Set up workflow permissions (read/write)
6. [ ] Configure security features (Dependabot, CodeQL)
7. [ ] Add issue and PR templates
8. [ ] Test a feature branch → develop → main flow
9. [ ] Verify release automation triggers correctly
10. [ ] Document any custom configuration in team wiki

**Estimated setup time:** 30-45 minutes

**Next steps:** Test the workflow with a small feature branch to ensure all protections work as expected.
