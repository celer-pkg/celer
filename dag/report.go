package dag

import (
	"encoding/json"
	"fmt"
	"os"
	"path/filepath"

	"github.com/celer-pkg/celer/context"
	"github.com/celer-pkg/celer/pkgs/logger"
)

// node is one scheduled node of a distributed build: a package an agent either
// builds or restores from pkgcache.
type node struct {
	NameVersion  string   `json:"name_version"`
	BuildHash    string   `json:"build_hash"`
	Checksum     string   `json:"checksum"`
	Dependencies []string `json:"dependencies"`
}

// localNode is a dev/host dependency of the graph. It is not scheduled (every
// agent builds it locally, its cache key embeds the workspace path), but its
// source revision is pinned like the scheduled nodes so that no agent has to
// resolve - or clone - it again.
type localNode struct {
	NameVersion string `json:"name_version"`
	Checksum    string `json:"checksum"`
}

// Report is the machine readable build DAG consumed by an external
// orchestrator (see deploy/jenkins/README.md).
type Report struct {
	CelerVersion string `json:"celer_version"`
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

// PinSources returns the source revision every port of the plan is pinned to: the
// scheduled nodes, plus the dev/host dependencies each agent builds locally.
func (r Report) PinSources() map[string]string {
	pins := make(map[string]string, len(r.ScheduledNodes)+len(r.LocalNodes))
	for _, node := range r.ScheduledNodes {
		if node.Checksum != "" {
			pins[node.NameVersion] = node.Checksum
		}
	}
	for _, dep := range r.LocalNodes {
		if dep.Checksum != "" {
			pins[dep.NameVersion] = dep.Checksum
		}
	}
	return pins
}

// DagHashs returns the build hash expected for every scheduled node.
func (r Report) DagHashs() map[string]string {
	hashes := make(map[string]string, len(r.ScheduledNodes))
	for _, node := range r.ScheduledNodes {
		hashes[node.NameVersion] = node.BuildHash
	}
	return hashes
}

// DagCached returns the nodes the plan reports as already present in pkgcache.
func (r Report) DagCached() map[string]bool {
	cached := make(map[string]bool, len(r.Cached))
	for _, id := range r.Cached {
		cached[id] = true
	}
	return cached
}

// Export writes the build plan of the current project to outPath as JSON, for an
// external build orchestrator.
func Export(ctx context.Context, outPath string) error {
	report, err := buildReport(ctx)
	if err != nil {
		return err
	}
	return writeReport(report, outPath)
}

// Parse reads a DAG written by Export.
func Parse(path, celerVersion string) (*Report, error) {
	bytes, err := os.ReadFile(path)
	if err != nil {
		return nil, fmt.Errorf("failed to read %s -> %w", path, err)
	}

	var report Report
	if err := json.Unmarshal(bytes, &report); err != nil {
		return nil, fmt.Errorf("failed to parse %s -> %w", path, err)
	}

	if report.CelerVersion != celerVersion {
		return nil, fmt.Errorf("%s was exported by celer %s", path, report.CelerVersion)
	}
	return &report, nil
}

// buildReport plans the build of the current project: it pins the whole closure,
// hashes every node of it, and drops the nodes pkgcache can supply.
func buildReport(ctx context.Context) (*Report, error) {
	roots := ctx.Project().GetPorts()
	if len(roots) == 0 {
		return nil, fmt.Errorf("project '%s' has no ports", ctx.Project().GetName())
	}

	pins, locals, err := pinClosure(ctx, roots)
	if err != nil {
		return nil, err
	}

	nodes, err := hashClosure(ctx, roots, pins)
	if err != nil {
		return nil, err
	}

	scheduled, cached, err := classifyNodes(ctx, nodes)
	if err != nil {
		return nil, err
	}

	return &Report{
		CelerVersion:   ctx.Version(),
		Platform:       ctx.Platform().GetName(),
		Project:        ctx.Project().GetName(),
		BuildType:      ctx.BuildType(),
		ScheduledNodes: scheduled,
		Cached:         cached,
		LocalNodes:     locals,
	}, nil
}

// writeReport writes the plan to outPath. It goes to the requested file instead
// of stdout: celer's own progress output shares stdout with it, and CI reads the
// file directly.
func writeReport(report *Report, outPath string) error {
	bytes, err := json.MarshalIndent(report, "", "  ")
	if err != nil {
		return fmt.Errorf("failed to encode the build DAG -> %w", err)
	}

	outPath = filepath.Clean(outPath)
	if err := os.MkdirAll(filepath.Dir(outPath), os.ModePerm); err != nil {
		return fmt.Errorf("failed to create the output dir of %s -> %w", outPath, err)
	}
	if err := os.WriteFile(outPath, append(bytes, '\n'), os.ModePerm); err != nil {
		return fmt.Errorf("failed to write the build DAG to %s -> %w", outPath, err)
	}

	dagPath, err := filepath.Abs(outPath)
	if err != nil {
		dagPath = outPath
	}

	cached := ""
	if len(report.Cached) > 0 {
		cached = fmt.Sprintf(", %d restored from pkgcache", len(report.Cached))
	}
	if len(report.ScheduledNodes) == 0 {
		logger.PrintSuccess("%s written: nothing to build for project %s%s.", dagPath, report.Project, cached)
		return nil
	}
	logger.PrintSuccess("%s written: %d nodes to build for project %s%s.", dagPath, len(report.ScheduledNodes), report.Project, cached)
	return nil
}
