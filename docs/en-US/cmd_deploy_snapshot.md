# Deploy snapshot (`deploy --snapshot`)

`deploy --snapshot` runs normal deployment, then exports a reproducible workspace snapshot.

## Command Syntax

```shell
celer deploy --snapshot=<snapshot_dir>
```

## Important Behavior

- Export snapshot starts only after deployment succeeds.
- Existing snapshot directory is removed and recreated.
- Snapshot contains fixed dependency checksums for reproducibility.

## Exported Content

- `ports/`: used ports with matched build config and fixed checksum
- `conf/`: workspace conf directory (`.git` excluded)
- `celer.toml`
- `toolchain_file.cmake`
- `snapshot.md`
- current `celer` executable

## Checksum Rules

- Git URL (`*.git`): read the actual local commit hash from cloned source as the checksum.
- Private repo with fixed `package.checksum`: use that fixed checksum.
- Archive URL (`.zip/.tar...`): use `sha256` as the checksum.

## Common Examples

```shell
# Deploy and export snapshot
celer deploy --snapshot=snapshots/2026-02-21

# Deploy with force and export
celer deploy --force --snapshot=snapshots/rebuild
```

## Notes

- Export snapshot requires `toolchain_file.cmake` to exist (normally produced by successful deploy).
- If deployment fails, exporting snapshot is not executed.

## Sample Snapshot Output

```md
# Build snapshot

## Build environment

- deployed at: 2026-05-31T11:20:42.175732704+08:00
- celer version: v0.0.0
- platform: aarch64-linux-ubuntu-22.04-gcc-11.5.0
- project: project_test_01

## Resolved commits

| Name@Version | Type | URL | Ref | Resolved |
|---|---|---|---|---|
| opencv@4.11.0 | git | https://github.com/opencv/opencv.git | 4.11.0 | 1d3b34ddd080bbf3e3d3cec58e11038fca21dcfe |
| ffmpeg@5.1.6 | archive | https://ffmpeg.org/releases/ffmpeg-5.1.6.tar.xz | 5.1.6 | - |
```