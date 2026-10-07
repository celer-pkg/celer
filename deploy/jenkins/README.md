# Distributed build on Jenkins

`celer deploy --export-dag=dag.json` exports the build schedule of the current
project; this directory holds the Jenkins side that turns that schedule into one job
per node, wired by the schedule's own dependency edges, and then collects the artifacts
back with `celer deploy --apply-dag=dag.json`.

| File | Jenkins job | Purpose |
|------|-------------|---------|
| `celer-master.groovy` | `celer-master` | Master: exports the schedule, starts each scheduled node once its own dependencies are done, then collects the artifacts. |
| `celer-agent.groovy` | `celer-agent` | Parameterized node job: installs one node of the schedule on its platform's pool. |

## The schedule file

| Field | Use |
|-------|-----|
| `platform` / `project` / `build_type` | Adopted by `celer install --dag` and `celer deploy --apply-dag`: written to the workspace's `celer.toml` before celer is initialized, so the machine builds the schedule's workspace instead of its own. |
| `conf_ref` | The exact conf repo revision (a commit) the schedule was exported from. Both ends need the conf repo, which is why both jobs run `celer init` first: the export records the revision, and `install --dag` / `--apply-dag` reset the workspace's conf to it before reading any port config, so every agent builds on the same configs as the master. A run without the conf repo is refused. |
| `scheduled_nodes[]` | One job per entry, parameter `CELER_PORT`. |
| `scheduled_nodes[].dependencies` | Job edges, but only those that are themselves scheduled: a dependency listed in `cached` is already in pkgcache and needs no edge. |
| `local_nodes[]` | Never a job: every machine builds them locally, the pins in the file keep them reproducible. |
| `cached[]` | Not scheduled: the jobs that need them restore them from pkgcache, and `--apply-dag` collects them too. |
| *(no `scheduled_nodes`)* | Nothing to build: the distribute stage has nothing to start, but the deploy stage still collects `cached[]`. |

## The master job

- One branch per scheduled node, each waiting only for its own dependencies: a node
  starts the moment they are done, so a slow branch never holds back a node that does
  not depend on it. No two agents ever build the same node either: a node runs only
  after its whole dependency closure put its artifacts in pkgcache.
- The parallel step runs with `failFast`, so the first node that fails or times out
  stops every other node: branches still waiting for a dependency are terminated
  (they never start their job), and each branch already inside its `build` step aborts
  the `celer-agent` build it started. That abort needs the account running
  `celer-master` to hold Job/Cancel on `celer-agent`; without it Jenkins logs a message
  and leaves the node running.
- A cycle would leave the nodes of the cycle waiting forever. `celer deploy
  --export-dag` rejects cycles before exporting, and the master bounds the whole build
  with a 4-hour `timeout`, so a schedule it cannot make progress on fails instead of
  hanging. The timeout goes through the same failFast path and stops the running nodes
  too.
- Every node job is handed the master build number and copies exactly that build's
  `dag.json`, so a newer schedule cannot leak into a running build.
- The `deploy with dag` stage runs `celer deploy --apply-dag=dag.json`: it collects
  `scheduled_nodes[]` **plus** `cached[]` on the master, each one from pkgcache and
  nothing else. A node that never reached pkgcache therefore fails the stage instead of
  being rebuilt here. Add `--snapshot=<dir>` to write the reproducibility bundle as
  well (the matched conf, the ports with their checksums fixed, `celer.toml`, the
  toolchain file and the celer binary - not the collected artifacts), and `--strip` to
  strip the collected tree. `local_nodes[]` are built locally on the master, as on every
  other machine: their cache key embeds the workspace path, so nothing about them is
  shared.

## The node job

| Parameter | Use |
|-----------|-----|
| `CELER_PORT` | `name@version` to install, one entry of `scheduled_nodes`. |
| `CELER_PLATFORM` / `CELER_PROJECT` | The schedule's identity. celer adopts them from `dag.json` itself; they are passed along to describe the build in the job. |
| `MASTER_BUILD` | The `celer-master` build that exported `dag.json`: the node copies exactly that build's file, never a newer one. |

Every node job runs exactly one command:

```shell
celer install --dag=dag.json <name_version>
```

Agents carry the label `celer-agent`, which is what the node job asks for; the
schedule's `platform` decides what celer builds there, not which machine runs the job.
To split pools per platform instead, put `params.CELER_PLATFORM` back into the node
job's `agent { label ... }` and label the agents `celer-agent-<platform>`.

## Plugins

- Pipeline (built in).
- Pipeline Utility Steps (`readJSON` in the master job).
- Copy Artifact (`copyArtifacts` in the node job).
- Pipeline: Build Step recent enough that stopping a `build` step aborts the build it
  started (current releases do; the cancel needs Job/Cancel on the downstream job, per
  [SECURITY-3870](https://www.jenkins.io/security/advisory/2026-09-02/)).
- Pipeline: Basic Steps 2.20 or newer: the master waits with `waitUntil(quiet: true)`,
  and the `quiet` option only exists from that release
  ([JENKINS-59776](https://issues.jenkins.io/browse/JENKINS-59776)); without it every
  waiting node would log each check.

Both scripts are plain Declarative Pipeline: create `celer-master` with the content of
`celer-master.groovy` and `celer-agent` with the content of `celer-agent.groovy`. Let
`celer-master` make the first call: it passes `CELER_PORT`, `CELER_PLATFORM`,
`CELER_PROJECT` and `MASTER_BUILD` as build parameters, and `params` reads those
whether or not the node job has registered its own `parameters` block yet. So a
hand-triggered `celer-agent` needs those values typed in, or the parameters defined on
the job first.
