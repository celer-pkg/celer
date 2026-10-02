# Distributed build on Jenkins

`celer deploy --dag=dag.json` exports the build plan of the current project; this
directory holds the Jenkins side that turns that plan into one job per node, wired
by the plan's own dependency edges.

| File | Purpose |
|------|---------|
| `celer-plan.groovy` | Coordinator: exports the plan and runs the scheduled nodes in dependency waves. |
| `celer-node.groovy` | Parameterized node job (`NODE`): installs one node of the plan. |

## The plan file

| Field | Use |
|-------|-----|
| `platform` / `project` / `build_type` | Adopted by `celer install --dag`: written to the agent's `celer.toml` before celer is initialized, so the agent builds the plan's workspace. |
| `scheduled_nodes[]` | One job per entry, parameter `name_version`. |
| `scheduled_nodes[].dependencies` | Job edges, but only those that are themselves scheduled: a dependency listed in `cached` is already in pkgcache and needs no edge. |
| `local_nodes[]` | Never a job: every agent builds them locally, the pins in the file keep them reproducible. |
| `cached[]` | Not scheduled; the jobs that need them restore them from pkgcache. |
| *(no `scheduled_nodes`)* | Nothing to build: skip the whole build stage. |

Every node job runs exactly one command:

```shell
celer install --dag=dag.json <name_version>
```

## Plugins

- Pipeline (built in).
- Pipeline Utility Steps (`readJSON` in the coordinator).
- Copy Artifact (`copyArtifacts` in the node job).

Both scripts are plain Declarative Pipeline: create `celer-plan` with the content
of `celer-plan.groovy`, `celer-node` with the content of `celer-node.groovy`, and
point their `agent { label ... }` at the labels you want.
