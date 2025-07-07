# Test Backlog

This directory contains one-off tests created during development that haven't yet been integrated into the main test structure.

## Purpose

- **Temporary storage** for quick debugging tests
- **Development artifacts** from specific sprints/tasks
- **Experimental tests** that may become permanent
- **Proof-of-concept** validation code

## Guidelines

### When to Use Backlog
- Quick debugging tests during development
- Task-specific validation (e.g., `test_task_4_isolated.c`)
- One-time regression verification
- Experimental features testing

### Migration Process
Tests in the backlog should be regularly reviewed and either:
1. **Migrated** to appropriate test categories (unit/integration/performance/regression)
2. **Enhanced** to meet project testing standards
3. **Archived** if no longer relevant
4. **Deleted** if superseded by better tests

## Current Backlog Items

### Sprint 10.5 Task 4 Tests
- `test_task_4_isolated.c` - Isolated mesh GPU upload testing
- `test_task_4_mock.c` - Mock Sokol functions for testing
- `test_task_4_simple.c` - Simplified graphics pipeline test

**Status**: Should be migrated to `assets/test_mesh_gpu_upload.c`

### Legacy Tests
- Various debugging tests from previous sprints

## Review Schedule

- **Weekly**: Check for new backlog items
- **Monthly**: Migrate stable tests to main structure
- **Quarterly**: Clean up obsolete tests

## Migration Checklist

Before migrating a test from backlog:
- [ ] Follows project naming conventions
- [ ] Uses Unity framework properly
- [ ] Has proper setup/teardown
- [ ] Includes documentation
- [ ] Fits logically in target category
- [ ] Passes with current codebase
- [ ] Removes temporary/debug code
