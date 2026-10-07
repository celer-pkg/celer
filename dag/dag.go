package dag

import (
	"fmt"
	"path/filepath"

	"github.com/celer-pkg/celer/pkgs/dirs"
	"github.com/celer-pkg/celer/pkgs/fileio"
	"github.com/celer-pkg/celer/pkgs/git"
	"github.com/celer-pkg/celer/pkgs/logger"
)

// node is one scheduled node of a distributed build.
type node struct {
	NameVersion  string   `json:"name_version"`
	BuildHash    string   `json:"build_hash"`
	Checksum     string   `json:"checksum"`
	Dependencies []string `json:"dependencies"`
}

// localNode is a dev/host dependency of the graph. It is not scheduled (every
// agent builds it locally), but its source revision is pinned like the scheduled
// nodes so that no agent has to resolve - or clone - it again.
type localNode struct {
	NameVersion string `json:"name_version"`
	Checksum    string `json:"checksum"`
}

// Dag is the machine readable build DAG consumed by an external
// orchestrator (see deploy/jenkins/README.md).
type Dag struct {
	CelerVersion string `json:"celer_version"`
	ConfRef      string `json:"conf_ref"`
	Platform     string `json:"platform"`
	Project      string `json:"project"`
	BuildType    string `json:"build_type"`

	// ScheduledNodes are the packages an agent has to build.
	ScheduledNodes []node `json:"scheduled_nodes,omitempty"`

	// LocalNodes are the dev/host dependencies each agent builds for itself.
	LocalNodes []localNode `json:"local_nodes,omitempty"`

	// Cached are the nodes whose artifact is already in pkgcache.
	Cached []string `json:"cached,omitempty"`
}

func (d Dag) ApplyNodes() []string {
	nodes := make([]string, 0, len(d.ScheduledNodes)+len(d.Cached))
	for _, node := range d.ScheduledNodes {
		nodes = append(nodes, node.NameVersion)
	}
	return append(nodes, d.Cached...)
}

func (d Dag) PinConf() error {
	if d.ConfRef == "" {
		return fmt.Errorf("conf ref is empty")
	}
	if err := requireConf(); err != nil {
		return err
	}
	modified, err := git.IsModified(dirs.ConfDir)
	if err != nil {
		return fmt.Errorf("failed to inspect the conf repo -> %w", err)
	}
	if modified {
		return fmt.Errorf("the conf repo has local modifications and the schedule calls for conf %s: commit or discard them first", d.ConfRef)
	}

	current, err := git.GetCommitHash(dirs.ConfDir)
	if err != nil {
		return fmt.Errorf("failed to read the conf revision -> %w", err)
	}
	if current == d.ConfRef {
		logger.PrintInfo("conf repo is at the schedule's revision %s", current)
		return nil
	}

	logger.PrintInfo("git reset --hard the conf repo from '%s' to the schedule's revision '%s'", current, d.ConfRef)

	// HardReset resets, and fetches from origin when the revision is not local yet.
	if err := git.HardReset(dirs.ConfDir, d.ConfRef); err != nil {
		return fmt.Errorf("failed to git reset --hard the conf repo to '%s' -> %w", d.ConfRef, err)
	}
	return nil
}

// PinSources returns the source revision every port of the schedule is pinned to: the
// scheduled nodes, plus the dev/host dependencies each agent builds locally.
func (d Dag) PinSources() map[string]string {
	pins := make(map[string]string, len(d.ScheduledNodes)+len(d.LocalNodes))
	for _, node := range d.ScheduledNodes {
		if node.Checksum != "" {
			pins[node.NameVersion] = node.Checksum
		}
	}
	for _, dep := range d.LocalNodes {
		if dep.Checksum != "" {
			pins[dep.NameVersion] = dep.Checksum
		}
	}
	return pins
}

// DagHashs returns the build hash expected for every scheduled node.
func (d Dag) DagHashs() map[string]string {
	hashes := make(map[string]string, len(d.ScheduledNodes))
	for _, node := range d.ScheduledNodes {
		hashes[node.NameVersion] = node.BuildHash
	}
	return hashes
}

// DagCached returns the nodes the schedule reports as already present in pkgcache.
func (d Dag) DagCached() map[string]bool {
	cached := make(map[string]bool, len(d.Cached))
	for _, nameVersion := range d.Cached {
		cached[nameVersion] = true
	}
	return cached
}

// requireConf refuses a workspace that has no conf repo.
func requireConf() error {
	if !fileio.PathExists(filepath.Join(dirs.ConfDir, ".git")) {
		return fmt.Errorf("no conf repo in the workspace: initialize it with 'celer init --url=<conf repo>' first")
	}
	return nil
}

// confRevision reads the revision of the workspace's conf repo, or refuses the workspace when it has none.
func confRevision() (string, error) {
	if err := requireConf(); err != nil {
		return "", err
	}

	commit, err := git.GetCommitHash(dirs.ConfDir)
	if err != nil {
		return "", fmt.Errorf("failed to read the conf repo revision -> %w", err)
	}
	return commit, nil
}
