# Distributed build on Jenkins

`celer deploy --dag=dag.json` exports the build plan of the current project; this
directory holds the Jenkins side that turns that plan into one job per node, wired
by the plan's own dependency edges.

| File | Jenkins job | Purpose |
|------|-------------|---------|
| `celer-plan.groovy` | `celer-plan` | Coordinator: exports the plan and starts each scheduled node once its own dependencies are done. |
| `celer-agent.groovy` | `celer-agent` | Parameterized node job: installs one node of the plan on the pool of its platform. |

## The plan file

| Field | Use |
|-------|-----|
| `platform` / `project` / `build_type` | Adopted by `celer install --dag`: written to the agent's `celer.toml` before celer is initialized, so the agent builds the plan's workspace. `platform` also selects the node job's agent pool: a node runs on the agents labelled `celer-agent-<platform>`. |
| `scheduled_nodes[]` | One job per entry, parameter `CELER_NODE`. |
| `scheduled_nodes[].dependencies` | Job edges, but only those that are themselves scheduled: a dependency listed in `cached` is already in pkgcache and needs no edge. |
| `local_nodes[]` | Never a job: every agent builds them locally, the pins in the file keep them reproducible. |
| `cached[]` | Not scheduled; the jobs that need them restore them from pkgcache. |
| *(no `scheduled_nodes`)* | Nothing to build: skip the whole build stage. |

## The coordinator

- One branch per scheduled node, each waiting only for its own dependencies: a node
  starts the moment they are done, so a slow branch never holds back a node that does
  not depend on it. No two agents ever build the same node either: a node runs only
  after its whole dependency closure put its artifacts in pkgcache.
- The parallel step runs with `failFast`, so the first node that fails or times out
  stops every other node: branches still waiting for a dependency are terminated
  (they never start their job), and each branch already inside its `build` step aborts
  the `celer-agent` build it started. That abort needs the account running `celer-plan`
  to hold Job/Cancel on `celer-agent`; without it Jenkins logs a message and leaves the
  node running.
- A cycle would leave the nodes of the cycle waiting forever. `celer deploy --dag`
  rejects cycles before exporting, and the coordinator bounds the whole build with a
  4-hour `timeout`, so a plan it cannot make progress on fails instead of hanging. The
  timeout goes through the same failFast path and stops the running nodes too.
- Every node job is handed the plan build number and copies exactly that build's
  `dag.json`, so a newer plan cannot leak into a running build.

## The node job

| Parameter | Use |
|-----------|-----|
| `CELER_NODE` | `name@version` to install, one entry of `scheduled_nodes`. |
| `PLATFORM` | Platform of the plan; the job runs on `celer-agent-<platform>`. |
| `PLAN_BUILD` | The `celer-plan` build that exported `dag.json`. Empty (a manual run) falls back to the latest successful plan. |

Every node job runs exactly one command:

```shell
celer install --dag=dag.json <name_version>
```

Agents must carry the label `celer-agent-<platform>`, with the platform spelled
exactly as in the plan (`celer-agent-x86_64-linux-ubuntu-22.04-gcc-11.5.0`). A single
homogeneous pool is labelled after the one platform it builds. The coordinator always
passes `PLATFORM`; a hand-triggered run has to fill it in, since an empty one matches
no agent and would queue forever.

## Plugins

- Pipeline (built in).
- Pipeline Utility Steps (`readJSON` in the coordinator).
- Copy Artifact (`copyArtifacts` in the node job).
- Pipeline: Build Step recent enough that stopping a `build` step aborts the build it
  started (current releases do; the cancel needs Job/Cancel on the downstream job, per
  [SECURITY-3870](https://www.jenkins.io/security/advisory/2026-09-02/)).
- Pipeline: Basic Steps 2.20 or newer: the coordinator waits with `waitUntil(quiet:
  true)`, and the `quiet` option only exists from that release
  ([JENKINS-59776](https://issues.jenkins.io/browse/JENKINS-59776)); without it every
  waiting node would log each check.

Both scripts are plain Declarative Pipeline: create `celer-plan` with the content
of `celer-plan.groovy` and `celer-agent` with the content of `celer-agent.groovy`.
Let `celer-plan` make the first call: it passes `CELER_NODE`, `PLATFORM` and
`PLAN_BUILD` as build parameters, and `params` reads those whether or not the node job
has registered its own `parameters` block yet. So a hand-triggered `celer-agent` needs
those values typed in, or the parameters defined on the job first.
