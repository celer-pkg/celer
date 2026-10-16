package dag

import (
	"fmt"
	"path/filepath"

	"github.com/celer-pkg/celer/configs"
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
	CelerVersion string            `json:"celer_version"`
	ConfUrl      string            `json:"conf_url"`
	ConfRef      string            `json:"conf_ref"`
	PortsUrl     string            `json:"ports_url"`
	PortsRef     string            `json:"ports_ref"`
	Platform     string            `json:"platform"`
	Project      string            `json:"project"`
	BuildType    string            `json:"build_type"`
	PkgCache     *configs.PkgCache `json:"pkgcache"`

	// TargetNodes are the packages an agent has to build.
	TargetNodes []node `json:"target_nodes,omitempty"`

	// LocalNodes are the dev/host dependencies each agent builds for itself.
	LocalNodes []localNode `json:"local_nodes,omitempty"`

	// Cached are the nodes whose artifact is already in pkgcache.
	Cached []string `json:"cached,omitempty"`
}

func (d Dag) ApplyNodes() []string {
	nodes := make([]string, 0, len(d.TargetNodes)+len(d.Cached))
	for _, node := range d.TargetNodes {
		nodes = append(nodes, node.NameVersion)
	}
	return append(nodes, d.Cached...)
}

func (d Dag) PinRepo(repoName, repoDir, repoRef string) error {
	if repoRef == "" {
		return fmt.Errorf("'%s' ref is empty", repoName)
	}
	if !fileio.PathExists(filepath.Join(repoDir, ".git")) {
		return fmt.Errorf("no '%s' repo in the workspace, please init celer first", repoName)
	}

	modified, err := git.IsModified(repoDir)
	if err != nil {
		return fmt.Errorf("failed to inspect the '%s' repo -> %w", repoName, err)
	}
	if modified {
		return fmt.Errorf("the '%s' repo has local modifications and the schedule calls for '%s' -> '%s': commit or discard them first",
			repoName, repoName, repoRef)
	}

	current, err := git.GetCommitHash(repoDir)
	if err != nil {
		return fmt.Errorf("failed to read the '%s' revision -> %w", repoName, err)
	}
	if current == d.ConfRef {
		logger.PrintInfo("'%s' repo is at the schedule's revision '%s'", repoName, current)
		return nil
	}

	logger.PrintInfo("git reset --hard the '%s' repo from '%s' to the schedule's revision '%s'", repoName, current, d.ConfRef)

	// HardReset resets, and fetches from origin when the revision is not local yet.
	if err := git.HardReset(repoDir, repoRef); err != nil {
		return fmt.Errorf("failed to git reset --hard the '%s' repo to '%s' -> %w", repoName, repoRef, err)
	}
	return nil
}

// PinSources returns the source revision every port of the schedule is pinned to: the
// scheduled nodes, plus the dev/host dependencies each agent builds locally.
func (d Dag) PinSources() map[string]string {
	pins := make(map[string]string, len(d.TargetNodes)+len(d.LocalNodes))
	for _, node := range d.TargetNodes {
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
	hashes := make(map[string]string, len(d.TargetNodes))
	for _, node := range d.TargetNodes {
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
