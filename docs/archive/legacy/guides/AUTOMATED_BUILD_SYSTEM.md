# CGame Automated Build System

This directory contains an automated build monitoring system that ensures code quality by continuously monitoring file changes and preventing commits when builds fail.

## Components

### 1. File Watcher (`watch_build.sh`)
Monitors source files for changes and automatically builds the project.

**Usage:**
```bash
./watch_build.sh
```

**Features:**
- Watches `src/`, `assets/`, `data/`, and `Makefile` for changes
- Automatically runs `make clean && make` when files change
- Updates `.build_status` file with current build state
- Uses `fswatch` (macOS) or `entr` (cross-platform) for efficient monitoring
- Falls back to polling if neither tool is available

**Installation Requirements:**
```bash
# macOS (recommended)
brew install fswatch

# or cross-platform alternative
brew install entr
```

### 2. Pre-commit Hook (`.git/hooks/pre-commit`)
Prevents commits when the build is failing.

**Features:**
- Checks `.build_status` file before allowing commits
- Blocks commits if build status is FAILED or BUILDING
- Shows recent build errors for quick debugging
- Falls back to manual build check if no status file exists

### 3. Build Status Checker (`build_status.sh`)
Quick utility to check current build status.

**Usage:**
```bash
./build_status.sh
```

## Workflow

### Recommended Development Workflow

1. **Start the build watcher:**
   ```bash
   ./watch_build.sh
   ```

2. **Develop normally:**
   - Edit source files
   - The watcher automatically builds on changes
   - Check build status in terminal or with `./build_status.sh`

3. **Commit code:**
   - Pre-commit hook automatically checks build status
   - Commit is blocked if build is failing
   - Fix any build errors and try again

### Manual Commands

```bash
# Check current build status
./build_status.sh

# Manual build
make

# Build and run
make run

# Clean build
make clean

# View recent build errors
tail build.log
```

## Files

- `.build_status` - Current build status (not committed to git)
- `build.log` - Detailed build output from watcher (not committed to git)
- `.build_check` - Timestamp file for polling fallback (not committed to git)

## Build Status Format

The `.build_status` file contains pipe-separated values:
```
STATUS_CODE|TIMESTAMP|MESSAGE
```

**Status codes:**
- `SUCCESS` - Build completed successfully
- `FAILED` - Build failed (commit blocked)
- `BUILDING` - Build in progress (commit blocked)

## Troubleshooting

### "fswatch not found" or "entr not found"
Install the file monitoring tools:
```bash
brew install fswatch  # macOS recommended
brew install entr      # cross-platform alternative
```

### Pre-commit hook not working
Ensure the hook is executable:
```bash
chmod +x .git/hooks/pre-commit
```

### Build status not updating
1. Check if `watch_build.sh` is running
2. Verify file permissions: `chmod +x watch_build.sh`
3. Check for error messages in the watcher output

### False build failures
If you encounter persistent build failures:
1. Run `make clean && make` manually
2. Check `build.log` for detailed error information
3. Verify all dependencies are installed

## Integration with CI/CD

The build status system can be extended for CI/CD:
- `.build_status` file format is machine-readable
- `build.log` contains detailed build output
- Exit codes from scripts indicate success/failure

## Customization

### Watched Files
Edit `WATCH_DIRS` and `WATCH_EXTENSIONS` in `watch_build.sh`:
```bash
WATCH_DIRS="src/ assets/ data/ Makefile"
WATCH_EXTENSIONS="c,h,json,txt,glsl,metal,frag,vert"
```

### Build Commands
Modify the build commands in the `build_project()` function in `watch_build.sh`.

### Pre-commit Behavior
Adjust the pre-commit hook logic in `.git/hooks/pre-commit` to customize commit blocking behavior.
