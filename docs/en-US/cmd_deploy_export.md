# Deploy Export (`deploy --export`)

`deploy --export` runs normal deployment, then exports a reproducible workspace snapshot.

## Command Syntax

```shell
celer deploy --export=<export_dir>
```

## Important Behavior

- Export starts only after deployment succeeds.
- Existing export directory is removed and recreated.
- Snapshot contains fixed dependency checksums for reproducibility.

## Exported Content

- `ports/`: used ports with matched build config and fixed checksum
- `conf/`: workspace conf directory (`.git` excluded)
- `celer.toml`
- `toolchain_file.cmake`
- `snapshot.json`
- current `celer` executable

## Checksum Rules

- Git URL (`*.git`): read the actual local commit hash from cloned source as the checksum.
- Private repo with fixed `package.checksum`: use that fixed checksum.
- Archive URL (`.zip/.tar...`): use `sha-256:<checksum>` as the checksum.

## Common Examples

```shell
# Deploy and export snapshot
celer deploy --export=snapshots/2026-02-21

# Deploy with force and export
celer deploy --force --export=snapshots/rebuild
```

## Notes

- Export requires `toolchain_file.cmake` to exist (normally produced by successful deploy).
- If deployment fails, export is not executed.

## Sample Snapshot Output

```json
{
  "exported_at": "2025-12-14T16:51:10.290199679+08:00",
  "github.com/celer-pkg/celer_version": "0.1.0",
  "platform": "aarch64-linux-ubuntu-22.04-gcc-11.5.0",
  "project": "project_test_01",
  "dependencies": [
    {
      "name": "opencv",
      "version": "4.11.0",
      "checksum": "0e5254ebf54d2aed6e7eaf6660bf3b797cf50a02",
      "url": "https://github.com/opencv/opencv.git"
    }
  ]
}
```