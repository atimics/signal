# Branch Protection Configuration

This document provides the GitHub repository settings required to enforce the CGame Release Standards.

## Required Branch Protection Rules

### `main` Branch Protection

**Path**: `Settings > Branches > Add rule`

```yaml
Branch name pattern: main

Protection rules:
✅ Require a pull request before merging
  ✅ Require approvals: 2
  ✅ Dismiss stale PR approvals when new commits are pushed
  ✅ Require review from code owners (if CODEOWNERS file exists)
  ✅ Restrict pushes that create new files (optional)

✅ Require status checks to pass before merging
  ✅ Require branches to be up to date before merging
  Required status checks:
    - build-macos
    - build-linux  
    - quality-gate-check

✅ Require conversation resolution before merging
✅ Require signed commits (recommended)
✅ Require linear history
✅ Require deployments to succeed before merging
  Required environments:
    - production (for manual verification)

✅ Restrict pushes that create new files
✅ Restrict pushes
  Restrictions:
    - Only allow specific people/teams to push
    - Maintainers only

✅ Allow force pushes: Never
✅ Allow deletions: Never
```

### `develop` Branch Protection

**Path**: `Settings > Branches > Add rule`

```yaml
Branch name pattern: develop

Protection rules:
✅ Require a pull request before merging
  ✅ Require approvals: 1
  ✅ Dismiss stale PR approvals when new commits are pushed

✅ Require status checks to pass before merging
  ✅ Require branches to be up to date before merging
  Required status checks:
    - build-macos
    - build-linux

✅ Require conversation resolution before merging
✅ Require linear history (optional)

✅ Allow force pushes: Admins only
✅ Allow deletions: Never
```

## Environment Configuration

### Production Environment

**Path**: `Settings > Environments > New environment`

```yaml
Environment name: production

Protection rules:
✅ Required reviewers: 
  - [Maintainer GitHub usernames]
  
✅ Wait timer: 0 minutes
✅ Deployment protection rules:
  - Require manual approval for deployments
  
Environment secrets: (none required)
```

## Repository Secrets

**Path**: `Settings > Secrets and variables > Actions`

```yaml
Required secrets:
- GITHUB_TOKEN (automatically provided)

Optional secrets:
- DISCORD_WEBHOOK_URL (for release notifications)
- SLACK_WEBHOOK_URL (for release notifications)
```

## Rulesets (Advanced)

**Path**: `Settings > Rules > Rulesets > New ruleset`

### Semantic Commit Enforcement

```yaml
Ruleset name: Semantic Commits
Target: All branches

Rules:
✅ Restrict commits
  ✅ Require conventional commits
  Pattern: ^(feat|fix|docs|style|refactor|perf|test|chore)(\(.+\))?: .{1,50}
  
✅ Restrict file path changes
  Restricted paths:
    - package.json (require specific users)
    - src/version.h (require specific users)
    - .github/workflows/* (require admin)
```

### Release Standards Enforcement

```yaml
Ruleset name: Release Standards
Target: main branch

Rules:
✅ Require pull request before merge
✅ Require status checks
✅ Block creation of matching refs
✅ Restrict pushes to matching refs
  Allowed actors: Admins only
```

## Automated Setup Script

You can use the GitHub CLI to apply these settings:

```bash
#!/bin/bash
# Branch protection setup script

REPO="atimics/cgame"

# Main branch protection
gh api repos/$REPO/branches/main/protection \
  --method PUT \
  --field required_status_checks='{"strict":true,"contexts":["build-macos","build-linux","quality-gate-check"]}' \
  --field enforce_admins=true \
  --field required_pull_request_reviews='{"required_approving_review_count":2,"dismiss_stale_reviews":true}' \
  --field restrictions=null

# Develop branch protection  
gh api repos/$REPO/branches/develop/protection \
  --method PUT \
  --field required_status_checks='{"strict":true,"contexts":["build-macos","build-linux"]}' \
  --field enforce_admins=true \
  --field required_pull_request_reviews='{"required_approving_review_count":1,"dismiss_stale_reviews":true}' \
  --field restrictions=null

echo "✅ Branch protection rules applied"
echo "⚠️  Manual setup required for environments and advanced rulesets"
```

## Verification

After applying these settings, verify protection by:

1. Attempting to push directly to `main` (should fail)
2. Creating a PR without required status checks (should block merge)
3. Testing the manual approval flow for production environment

## Troubleshooting

### Common Issues

1. **Status checks not appearing**: Ensure workflow names match exactly
2. **Environment not triggering**: Check environment name spelling
3. **Ruleset conflicts**: Rulesets override branch protection rules

### Recovery

If you accidentally lock yourself out:

1. Go to `Settings > Branches`
2. Click `Delete` on the problematic rule
3. Reapply with correct settings

---

**Note**: These settings require repository admin privileges to configure.
