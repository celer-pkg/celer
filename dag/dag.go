package dag

import (
	"fmt"
	"maps"
	"slices"
	"sort"

	"github.com/celer-pkg/celer/configs"
	"github.com/celer-pkg/celer/context"
)

// pinClosure pins every port a build needs: the scheduled closure of the project
// ports, and the dev/host dependencies each agent builds for itself.
func pinClosure(ctx context.Context, roots []string) (map[string]string, []localNode, error) {
	pins := map[string]string{} // id -> commit hash
	locals := map[string]bool{} // id -> bool

	// pinAll walks the scheduled closure. An edge to a dev dependency moves its
	// whole subtree to the local set: those are built natively by every agent.
	var pinAll func(nameVersion string) error
	pinAll = func(nameVersion string) error {
		if _, ok := pins[nameVersion]; ok {
			return nil
		}

		var port configs.Port
		if err := port.Init(ctx, nameVersion); err != nil {
			return fmt.Errorf("failed to initialize port %s -> %w", nameVersion, err)
		}
		if err := resolveSource(&port, pins); err != nil {
			return err
		}

		for _, dependency := range port.MatchedConfig.Dependencies {
			if err := pinAll(dependency); err != nil {
				return err
			}
		}
		for _, dependency := range port.MatchedConfig.DevDependencies {
			if err := pinLocal(ctx, dependency, pins, locals); err != nil {
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
		localNodes = append(localNodes, localNode{NameVersion: id, Checksum: pins[id]})
	}
	return pins, localNodes, nil
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
func hashClosure(ctx context.Context, roots []string, pins map[string]string) ([]node, error) {
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

		item := &node{NameVersion: nameVersion, BuildHash: buildhash, Checksum: pins[nameVersion], Dependencies: []string{}}
		nodes[nameVersion] = item

		for _, dep := range port.MatchedConfig.Dependencies {
			item.Dependencies = append(item.Dependencies, dep)
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
func classifyNodes(ctx context.Context, nodes []node) ([]node, []string, error) {
	pkgCache := ctx.PkgCache()
	if pkgCache == nil {
		return nodes, nil, nil
	}

	artifactCache := pkgCache.GetArtifactCache()
	if artifactCache == nil {
		return nodes, nil, nil
	}

	scheduled := make([]node, 0, len(nodes))
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
		scheduled = append(scheduled, item)
	}
	return scheduled, cached, nil
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
