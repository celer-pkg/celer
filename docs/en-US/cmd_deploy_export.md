# 📦 Deploy with Export

## Overview

The `deploy --export` command combines deployment and workspace snapshot export in one step. After successfully building and installing all project dependencies, it exports a reproducible workspace snapshot.

## Usage

```bash
celer deploy --export=<export-directory>
```

## What Gets Exported?

1. **ports/**: All port configuration files used by the project
2. **conf/**: Configuration directory (platforms, projects)
3. **celer.toml**: Workspace configuration
4. **toolchain_file.cmake**: CMake toolchain file
5. **snapshot.json**: Dependency snapshot with actual git commits
6. **celer**: The celer execuable file currently using

## Key Features

### Actual Git Commits
Unlike standalone `celer export`, the deploy command exports **actual git commit hashes** from the cloned repositories in `buildtrees/`, not just the refs from port.toml.

For each git-based dependency, the snapshot contains:
- The exact commit hash from `git rev-parse HEAD`
- Captured after successful build
- Guarantees reproducibility

### Export Only After Success
The export happens only if deployment succeeds. This ensures:
- All dependencies are built successfully
- Source code is properly cloned
- Commit hashes are from verified builds

## Example

```bash
# Deploy project and export snapshot
celer deploy --export=snapshots/2025-12-14

# Check the snapshot
cat snapshots/2025-12-14/snapshot.json
```

### Sample Snapshot Output

```json
{
  "exported_at": "2025-12-14T16:51:10.290199679+08:00",
  "celer_version": "0.1.0",
  "platform": "aarch64-linux-ubuntu-22.04-gcc-11.5.0",
  "project": "project_test_01",
  "dependencies": [
    {
      "name": "opencv",
      "version": "4.11.0",
      "commit": "0e5254ebf54d2aed6e7eaf6660bf3b797cf50a02",
      "url": "https://github.com/opencv/opencv.git"
    }
  ]
}
```

## Workflow

1. **Build**: Deploy builds all dependencies from scratch
2. **Clone**: Git repositories cloned to `buildtrees/{name}@{version}/src/`
3. **Compile**: Each dependency compiled and installed
4. **Snapshot**: If successful, export workspace to specified directory
5. **Commit Capture**: Read actual commit from each git repository

## Use Cases

### CI/CD Reproducibility

```bash
# Build and snapshot in CI
celer deploy --export=build-artifacts/snapshot

# Archive and share
tar -czf build-snapshot.tar.gz build-artifacts/snapshot
```

### Version Locking
```bash
# Lock current working versions
celer deploy --export=snapshots/working-$(date +%Y%m%d)

# Restore later if needed
cd snapshots/working-20251214 && ./celer deploy
```

## Notes

- Export directory is created if it doesn't exist
- Existing export directory will be overwritten
- Archive downloads (.zip, .tar.gz) use configured ref as commit
- Only git repositories have actual commit hashes
