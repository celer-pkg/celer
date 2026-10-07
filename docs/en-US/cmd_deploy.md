# Deploy Command

The `deploy` command builds and installs all ports defined by the current project.

## Command Syntax

```shell
celer deploy [flags]
```

## Important Behavior

- Before deployment, Celer checks circular dependencies and version conflicts across project ports.
- Deployment uses the current workspace context (`platform`, `project`, `build_type`).
- `--force` passes force mode to project deployment (reinstall behavior).
- `--snapshot=<path>` triggers snapshot export only after deployment succeeds.
- `--snapshot` accepts both relative and absolute paths.
- `--snapshot` must be a non-empty path.
- `--export-dag=<file>` exports the build DAG of the current project as JSON and
  stops there: nothing is cloned, built or installed (see below).
- `--apply-dag=<file>` installs the nodes of such a schedule instead of deploying the
  project's ports: the workspace adopts the schedule's identity, its pinned source
  revisions and its build hashes (see below).
- Both `--export-dag` and `--apply-dag` need the workspace's conf repo: the export
  records the revision its port configs come from, and the apply resets the repo to that
  revision. A workspace without one refuses to run; initialize it with
  `celer init --url=<conf repo>` first.
- All port refs are resolved to concrete commits in one pass before any cloning begins, ensuring consistent code versions (see below). `--apply-dag` skips that pass: the schedule already carries the revisions.

## Command Options

| Option     | Short | Type    | Default Value | Description                                       |
|------------|-------|---------|---------------|---------------------------------------------------|
| --force    | -     | boolean | false         | Force deployment, ignoring already installed libs |
| --snapshot | -     | string  | empty string  | Export workspace snapshot after successful deploy |
| --strip    | -     | boolean | false         | After deploy, run the same runtime strip as [`celer strip`](./cmd_strip.md) |
| --export-dag | -   | string  | empty string  | Export the build DAG of the current project as JSON to `<file>` instead of deploying |
| --apply-dag | -    | string  | empty string  | Collect the nodes of a schedule exported by `--export-dag` from pkgcache instead of deploying the project's ports; a node that is missing fails the run |

## Common Examples

```shell
# Normal deployment
celer deploy

# Force deployment
celer deploy --force

# Deploy and export snapshot
celer deploy --snapshot=snapshots/2026-02-21

# Deploy and build runtime stripped tree (same as celer strip)
celer deploy --strip

# Force deploy, export snapshot, and strip
celer deploy --force --snapshot=snapshots/rebuild --strip

# Export the build DAG of the current project, without deploying
celer deploy --export-dag=dag.json

# Collect the nodes of that schedule, from pkgcache only
celer deploy --apply-dag=dag.json

# ... and also write the reproducibility bundle (config and pins, not the artifacts)
celer deploy --apply-dag=dag.json --snapshot=snapshots/2026-02-21
```

## Notes

- `--strip` runs the same logic as [`celer strip`](./cmd_strip.md) after a successful deploy, writing `workspace/stripped/...` (does not modify `installed/`).
- Make sure platform and project are configured before running deploy.
- Export is skipped if deployment fails.
- When deployment succeeds, you can use `toolchain_file.cmake` in CMake with `-DCMAKE_TOOLCHAIN_FILE=...`.

## Pre-Resolution of Refs

Before cloning, `deploy` resolves all ports' refs (branch/tag names) to commit hashes in a single pass, then clones uniformly. Results are saved as `snapshot.md` under `<workspace>/installed/celer/deployments/`.

This avoids the risk of remote pushes causing inconsistent commits for the same branch when resolving one-by-one during cloning, ensuring the entire deployment is based on a consistent code snapshot.

- **Fresh clone**: `git clone --branch <ref>` + `git reset --hard <commit>` — branch name preserved.
- **Existing repo**: `git reset --hard <commit>` directly.

## JSON Output (Build DAG)

`--export-dag=<file>` writes the target-platform build schedule, keyed by pkgcache
build hash, for an external build orchestrator. The root of the graph is the
project of the workspace, so the entry nodes are the ports its `celer.toml`
declares, and the file is written instead of running the deployment:

| Field | Description |
|-------|-------------|
| `celer_version` / `platform` / `project` / `build_type` | Workspace identity; `platform` also selects the build agent |
| `conf_ref` | The exact conf repo revision the schedule was exported from a commit or a branch. Applying a schedule, or building a node from one, moves the workspace's conf to it first, so every machine reads the same port configs |
| `scheduled_nodes[]` | The nodes an agent has to build (the key is omitted when there is nothing to build) |
| `scheduled_nodes[].name_version` | `name@version` |
| `scheduled_nodes[].build_hash` | pkgcache key: `artifacts-<celer_version>/.../<build_hash>.tar.gz` |
| `scheduled_nodes[].checksum` | Source revision resolved at export time (git commit or archive sha-256) |
| `scheduled_nodes[].dependencies` | Dependencies that must be installed before this node: built earlier, or restored from pkgcache |
| `cached` | `name@version` of the nodes whose artifact is already in pkgcache: not scheduled, restored by the jobs that need them |
| `local_nodes[]` | Dev/host dependencies each agent builds for itself: never scheduled, only pinned |
| `local_nodes[].name_version` / `local_nodes[].checksum` | The package and the source revision it is pinned to |

The export checks every scheduled node against pkgcache and splits the closure
into `scheduled_nodes` and `cached`; when `scheduled_nodes` is empty the key is
omitted and there is nothing to build at all. The check is two lookups per node
(its metadata and its archive), so a read-only pkgcache is enough, and an
unreachable cache fails the export instead of guessing. Without a pkgcache, or in
offline mode, every node is scheduled.

Dev/host dependencies are **not** scheduled and never take part in the cache
check: their cache key embeds the workspace path, so it is not portable between
agents. Every agent builds them locally once and reuses its `devcache`.

Hand the file to `celer install --dag=<file> <node>` to build a scheduled node
from the schedule: the agent adopts the `platform`, `project` and `build_type` of the
schedule instead of its own `celer.toml`, which is what makes it read the right
pkgcache prefix, then pins every source revision to the exported `checksum` and
fails when the node's build hash does not match the DAG. Nodes listed in `cached`
are not scheduled, so installing one of them that way is refused.

## Applying a Schedule (collecting the artifacts)

On the machine that should end up with the finished build tree — normally the master,
after all node jobs are done:

```shell
celer deploy --apply-dag=dag.json
```

It reads the schedule that `celer deploy --export-dag=dag.json` wrote, takes the
workspace identity (`platform`, `project`, `build_type`) and the pinned source revisions
from it, and installs every node the schedule names. The result is a normal celer install
tree under `installed/`, ready to snapshot, package or check.

A few things to know before you run it:

- **It installs from pkgcache only.** If a node is not there, the command stops with
  `<node> is not available from pkgcache`. It never builds a missing node itself: that
  error means the distributed build is incomplete (a node job failed or uploaded
  nothing), so fix the build and run again instead of building the node here.
- **It moves the workspace's `conf` repo to the revision the schedule records**, so the
  port configs read here are the ones the schedule was exported with. Commit or discard
  local conf changes first: the command refuses to throw them away.
- **Dev/host dependencies are built on this machine**, as on every other one (the
  schedule lists them as `local_nodes[]` and they are never shared), so this step may
  compile a few tools.
- `--snapshot=<dir>` additionally writes the reproducibility bundle — configs and pins,
  not the artifacts — and `--strip` strips the collected tree. The artifacts themselves
  are only in `installed/`.

The schedule has to come from the same celer version you are running; if it does not, the
command says so and stops.
