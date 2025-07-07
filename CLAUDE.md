# SIGNAL Development Guide

## 📍 Navigation & Discovery

### Finding Current Context
- **Active Work**: Check `docs/sprints/active/` for current sprint documentation
- **Recent Changes**: Use `git log --oneline -10` to see latest commits
- **Project Status**: Look for `CURRENT_SPRINT_STATUS.md` in active sprints folder
- **Build Health**: Run `make test` to see test coverage and failures

### Key Documentation Locations
```
docs/
├── sprints/           # Sprint planning and progress
│   ├── active/       # What's being worked on now
│   ├── completed/    # Historical context and decisions
│   └── backlog/      # Future work and ideas
├── technical/        # Architecture and design docs
├── manual/           # User-facing documentation
└── api/             # Code documentation
```

### Understanding the Codebase
- **Architecture**: Look for `ARCHITECTURE.md` or similar in `docs/technical/`
- **Component List**: Check `src/component/` directory for available components
- **System Logic**: Browse `src/system/` for game logic implementations
- **Recent Issues**: Search codebase for `TODO`, `FIXME`, `HACK` comments

## 🔍 Common Investigation Patterns

### "What's the current focus?"
1. Check `docs/sprints/active/`
2. Look at recent commits: `git log --oneline --since="1 week ago"`
3. Search for recent TODOs: `grep -r "TODO" src/ | grep -i "sprint\|current"`

### "How does X system work?"
1. Find the system file: `find src/ -name "*system_name*"`
2. Look for tests: `find tests/ -name "*test_system_name*"`
3. Check for documentation: `find docs/ -name "*system_name*"`
4. Trace usage: `grep -r "system_name" src/`

### "What's broken/needs work?"
1. Run tests: `make test`
2. Check known issues: `grep -r "FIXME\|BUG\|HACK" src/`
3. Review sprint documentation for blockers
4. Look at test output for failing tests

### "How do I test my changes?"
1. Unit tests: `make test`
2. Integration: `./build/signal --test-flight` (or check for test flags)
3. Performance: Look for benchmark tools in `tools/` or `scripts/`
4. Manual testing: Check `docs/manual/` for testing procedures

## 🛠️ Development Patterns

### Build Commands
```bash
# Common patterns (verify current usage)
make clean && make        # Full rebuild
make test                # Run test suite
make debug              # Debug build (if available)
./build/signal --help    # See runtime options
```

### Code Organization
- **ECS Pattern**: Entities (IDs) → Components (Data) → Systems (Logic)
- **Services**: Shared functionality in `src/services/`
- **HAL Layer**: Hardware abstraction in `src/hal/`
- **Scenes**: Game states in `src/scripts/` or `src/scenes/`

### Finding Examples
- **Component Creation**: Look at existing components for patterns
- **System Implementation**: Check simpler systems first
- **Test Writing**: Mirror existing test structure
- **Integration Points**: Trace through `main.c` or scene files

## 📊 Project Intelligence

### Performance Monitoring
- Look for profiling output in logs
- Check for performance targets in documentation
- Search for benchmarking code or scripts

### Configuration
- Runtime config: Often `*.txt`, `*.yaml`, or `*.json` in root or `config/`
- Build config: `CMakeLists.txt`, `Makefile`, or build scripts
- Game settings: Check for config service or settings system

### Asset Pipeline
- Asset locations: `assets/` directory structure
- Index files: Look for `index.json` or similar manifests
- Loading code: Search for asset loaders in `src/`

## 🔄 Workflow Helpers

### Before Starting Work
```bash
# Get oriented
git status                          # Check working state
git pull                           # Get latest changes
make test                          # Verify build health
find docs/sprints/active -name "*.md" | head -5  # See active work
```

### While Working
```bash
# Stay informed
grep -r "TODO.*your_feature" src/  # Find related TODOs
git diff                           # Review changes
make test specific_test            # Test incrementally
tail -f *.log                      # Watch runtime logs
```

### Investigation Tools
```bash
# Code archaeology
git blame src/path/to/file.c      # Who wrote this?
git log -p src/path/to/file.c     # How did it evolve?
grep -r "function_name" src/       # Where is it used?
find src/ -newer reference_file    # What changed recently?
```

## 💡 Tips for Exploration

### Reading the Code
1. **Start with tests** - They show intended usage
2. **Follow the data** - Trace component flow through systems
3. **Check initialization** - See how things are set up
4. **Look for patterns** - Similar files often work similarly

### Understanding Decisions
- **Git history**: Commit messages explain "why"
- **Sprint docs**: Capture planning and pivots
- **Code comments**: Often explain tricky parts
- **Test names**: Describe expected behavior

### Getting Help
- **Error messages**: Usually point to the problem
- **Debug output**: Enable verbose logging if available
- **Test failures**: Read the assertion messages
- **Documentation**: Check multiple places - code, docs, tests

## 🚀 Quick Reference

### Common Patterns to Search For
- Setup/Init: `_create`, `_init`, `_setup`
- Cleanup: `_destroy`, `_cleanup`, `_free`
- Updates: `_update`, `_tick`, `_process`
- Queries: `_get`, `_find`, `_query`
- Events: `_on`, `_handle`, `_event`

### Debugging Helpers
- Print debugging: Look for `DEBUG`, `LOG`, `printf` patterns
- Assertions: `assert`, `TEST_ASSERT`, validation code
- Error handling: `_error`, `failed`, `invalid`
- State dumps: `_debug`, `_dump`, `_print`

---

Remember: This guide helps you find information. The source of truth is always:
1. The code itself
2. The tests that verify it
3. The documentation that explains it
4. The git history that tracks it