package dag

import (
	"encoding/json"
	"fmt"
	"maps"
	"os"
	"path/filepath"
	"slices"
	"sort"

	"github.com/celer-pkg/celer/configs"
	"github.com/celer-pkg/celer/context"
	"github.com/celer-pkg/celer/pkgs/dirs"
	"github.com/celer-pkg/celer/pkgs/git"
	"github.com/celer-pkg/celer/pkgs/logger"
)

// Export writes the build schedule of the specified roots to outPath as JSON,
// then orchestrator can read it to create build tasks.
func Export(ctx context.Context, roots []string, ignoreCache bool, outPath string) error {
	dag, err := buildDag(ctx, roots, ignoreCache)
	if err != nil {
		return fmt.Errorf("failed to build dag -> %w", err)
	}
	return writeDag(dag, outPath)
}

// buildDag schedule the build of specified roots: it pins the whole closure, hashes every node of it.
func buildDag(ctx context.Context, roots []string, ignoreCache bool) (*Dag, error) {
	confUrl, confRef, err := ctx.ConfUrlRef()
	if err != nil {
		return nil, err
	}
	portsUrl, portsRef, err := ctx.PortsUrlRef()
	if err != nil {
		return nil, err
	}

	// Make sure no modification in conf and ports.
	for _, repo := range []struct{ name, dir, ref string }{
		{"conf", dirs.ConfDir, confRef},
		{"ports", dirs.PortsDir, portsRef},
	} {
		modified, err := git.IsModified(repo.dir)
		if err != nil {
			return nil, fmt.Errorf("failed to inspect the '%s' repo -> %w", repo.name, err)
		}
		if modified {
			logger.PrintWarning("the '%s' repo has uncommitted changes: the schedule pins '%s' but the hashes come from the working tree", repo.name, repo.ref)
		}
	}

	projectName := ctx.Project().GetName()
	if projectName == "" {
		return nil, fmt.Errorf("no project has been configured in celer.toml")
	}

	scheduledPins, localPins, err := pinClosure(ctx, roots)
	if err != nil {
		return nil, err
	}

	mergedNodes, err := hashClosure(ctx, roots, scheduledPins)
	if err != nil {
		return nil, err
	}

	targetNodes, cached, err := classifyNodes(ctx, mergedNodes, ignoreCache)
	if err != nil {
		return nil, err
	}

	dag := Dag{
		CelerVersion: ctx.Version(),
		ConfUrl:      confUrl,
		ConfRef:      confRef,
		PortsUrl:     portsUrl,
		PortsRef:     portsRef,
		Platform:     ctx.Platform().GetName(),
		Project:      projectName,
		BuildType:    ctx.BuildType(),
		TargetNodes:  targetNodes,
		Cached:       cached,
		LocalNodes:   localPins,
	}

	// Set pkgcache solution if configured: minio and fs are mutually exclusive.
	if ctx.PkgCache() != nil {
		minio := ctx.PkgCache().GetMinio()
		fs := ctx.PkgCache().GetFS()
		if minio != nil {
			if err := minio.Validate(); err != nil {
				return nil, err
			}
			dag.PkgCache = &configs.PkgCache{
				Minio: minio,
			}
		} else if fs != nil {
			if err := fs.Validate(); err != nil {
				return nil, err
			}
			dag.PkgCache = &configs.PkgCache{
				FS: fs,
			}
		}
	}

	return &dag, nil
}

// writeDag writes the schedule to outPath. It goes to the requested file instead
// of stdout: celer's own progress output shares stdout with it, and CI reads the
// file directly.
func writeDag(report *Dag, outPath string) error {
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

	logger.PrintInfo("the schedule pins conf '%s' and ports '%s'", report.ConfRef, report.PortsRef)

	if len(report.TargetNodes) == 0 {
		logger.PrintSuccess("%s written: nothing to build for project %s%s.", dagPath, report.Project, cached)
		return nil
	}
	logger.PrintSuccess("%s written: %d nodes to build for project %s%s.", dagPath, len(report.TargetNodes), report.Project, cached)
	return nil
}

// pinClosure pins every port a build needs: the scheduled closure of the project
// ports, and the dev/host dependencies each agent builds for itself.
func pinClosure(ctx context.Context, roots []string) (map[string]string, []localNode, error) {
	schedules := map[string]string{} // id -> commit hash
	locals := map[string]bool{}      // id -> bool

	// pinAll walks the scheduled closure. An edge to a dev dependency moves its
	// whole subtree to the local set: those are built natively by every agent.
	var pinAll func(nameVersion string) error
	pinAll = func(nameVersion string) error {
		if _, ok := schedules[nameVersion]; ok {
			return nil
		}

		var port configs.Port
		if err := port.Init(ctx, nameVersion); err != nil {
			return fmt.Errorf("failed to initialize port %s -> %w", nameVersion, err)
		}
		if err := resolveSource(&port, schedules); err != nil {
			return err
		}

		for _, dependency := range port.MatchedConfig.Dependencies {
			if err := pinAll(dependency); err != nil {
				return err
			}
		}
		for _, dependency := range port.MatchedConfig.DevDependencies {
			if err := pinLocal(ctx, dependency, schedules, locals); err != nil {
				return err
			}
		}
		return nil
	}

	for _, root := range roots {
		if err := pinAll(root); err != nil {
			return nil, nil, err
		}
	}

	// Emit the localNodes dependencies in a stable order.
	localNodes := make([]localNode, 0, len(locals))
	for _, id := range slices.Sorted(maps.Keys(locals)) {
		localNodes = append(localNodes, localNode{NameVersion: id, Checksum: schedules[id]})
	}
	return schedules, localNodes, nil
}

// pinLocal pins a port every agent builds for itself, and everything it needs:
// those artifacts are never shared, so the whole subtree stays local.
func pinLocal(ctx context.Context, nameVersion string, pins map[string]string, locals map[string]bool) error {
	if locals[nameVersion] {
		return nil
	}
	locals[nameVersion] = true

	var port = configs.Port{DevDep: true}
	if err := port.Init(ctx, nameVersion); err != nil {
		return fmt.Errorf("failed to initialize dev dependency %s -> %w", nameVersion, err)
	}
	if err := resolveSource(&port, pins); err != nil {
		return err
	}

	for _, dep := range port.MatchedConfig.Dependencies {
		if err := pinLocal(ctx, dep, pins, locals); err != nil {
			return err
		}
	}
	for _, dep := range port.MatchedConfig.DevDependencies {
		if err := pinLocal(ctx, dep, pins, locals); err != nil {
			return err
		}
	}
	return nil
}

// hashClosure computes the build hash of every scheduled node, now that every
// revision in the closure is pinned, and returns them in a stable order.
func hashClosure(ctx context.Context, roots []string, schedulePins map[string]string) ([]node, error) {
	var (
		nodes = map[string]*node{}
		hash  func(nameVersion string) error
	)
	hash = func(nameVersion string) error {
		if _, ok := nodes[nameVersion]; ok {
			return nil
		}

		var port configs.Port
		if err := port.Init(ctx, nameVersion); err != nil {
			return fmt.Errorf("failed to initialize port %s -> %w", nameVersion, err)
		}
		buildhash, err := port.BuildHash()
		if err != nil {
			return fmt.Errorf("failed to calculate the pkgcache buildhash of %s -> %w", nameVersion, err)
		}

		node := &node{
			NameVersion:  nameVersion,
			BuildHash:    buildhash,
			Checksum:     schedulePins[nameVersion],
			Dependencies: []string{},
		}
		nodes[nameVersion] = node

		for _, dep := range port.MatchedConfig.Dependencies {
			node.Dependencies = append(node.Dependencies, dep)
			if err := hash(dep); err != nil {
				return err
			}
		}
		return nil
	}

	for _, root := range roots {
		if err := hash(root); err != nil {
			return nil, err
		}
	}

	// Emit the nodes in a stable order.
	result := make([]node, 0, len(nodes))
	for _, id := range slices.Sorted(maps.Keys(nodes)) {
		item := *nodes[id]
		sort.Strings(item.Dependencies)
		result = append(result, item)
	}
	return result, nil
}

// classifyNodes splict notes into two parts, one is scheduled to build,
// anther one is already cached in pkgcache.
func classifyNodes(ctx context.Context, nodes []node, ignoreCache bool) ([]node, []string, error) {
	pkgCache := ctx.PkgCache()
	if pkgCache == nil {
		return nodes, nil, nil
	}

	// Return all as scheduled nodes when artifact cache is not configured or cache is ignored.
	artifactCache := pkgCache.GetArtifactCache()
	if artifactCache == nil || ignoreCache {
		return nodes, nil, nil
	}

	// Collect nodes whose artifact is not stored in pkgcache.
	targets := make([]node, 0, len(nodes))
	var cached []string
	for _, item := range nodes {
		exist, err := artifactCache.Exists(item.NameVersion, item.BuildHash)
		if err != nil {
			return nil, nil, fmt.Errorf("failed to check the pkgcache for '%s' -> %w", item.NameVersion, err)
		}
		if exist {
			cached = append(cached, item.NameVersion)
			continue
		}
	}
	return targets, cached, nil
}

// resolveSource resolves the source revision of a port against the remote and pins
// it, so that every later resolution reuses the value instead of cloning the
// repo to read its HEAD.
func resolveSource(port *configs.Port, pins map[string]string) error {
	checksum, err := port.ResolveSource()
	if err != nil {
		return fmt.Errorf("failed to resolve the source checksum of %s -> %w", port.NameVersion(), err)
	}

	configs.PinSource(port.NameVersion(), checksum)
	pins[port.NameVersion()] = checksum
	return nil
}
