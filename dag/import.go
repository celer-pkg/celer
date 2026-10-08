package dag

import (
	"encoding/json"
	"fmt"
	"os"

	"github.com/celer-pkg/celer/configs"
	"github.com/celer-pkg/celer/context"
	"github.com/celer-pkg/celer/depcheck"
	"github.com/celer-pkg/celer/pkgs/errors"
	"github.com/celer-pkg/celer/pkgs/expr"
	"github.com/celer-pkg/celer/pkgs/logger"
)

// Import reads an exported dag json and makes the workspace adopt its identity.
func Import(celer *configs.Celer, dagPath string) (*DagInfo, error) {
	dag, err := parseDag(dagPath, celer.Version())
	if err != nil {
		return nil, err
	}

	// Make sure that all scheduled jobs use the same revision of the conf repo.
	if err := dag.PinConf(); err != nil {
		return nil, err
	}

	// Adopt the identity of the schedule, one setter per property.
	if err := celer.SetPlatform(dag.Platform); err != nil {
		return nil, fmt.Errorf("failed to adopt the platform of %s -> %w", dagPath, err)
	}
	if err := celer.SetProject(dag.Project); err != nil {
		return nil, fmt.Errorf("failed to adopt the project of %s -> %w", dagPath, err)
	}
	if err := celer.SetBuildType(dag.BuildType); err != nil {
		return nil, fmt.Errorf("failed to adopt the build type of %s -> %w", dagPath, err)
	}
	logger.PrintInfo("build identity taken from %s: platform %s, project %s, build type %s",
		dagPath, dag.Platform, dag.Project, dag.BuildType)

	// Pin the sources of the schedule.
	schedule := DagInfo{Dag: dag}
	pins := dag.PinSources()
	configs.PinSources(pins)

	// Cache dag's data fields.
	schedule.DagHashes = dag.DagHashs()
	schedule.DagCached = dag.DagCached()

	logger.PrintInfo("pinned %d source revisions from %s", len(pins), dagPath)
	return &schedule, nil
}

type DagInfo struct {
	Dag       *Dag
	DagHashes map[string]string // nameVersion -> hash
	DagCached map[string]bool   // nameVersion -> bool
}

// Expectation is what an exported schedule requires of one node.
type Expectation struct {
	DagPath      string // Dag file path of the schedule, empty for a plain `celer install <port>`.
	BuildHash    string // It's the hash the schedule recorded, empty for a node it only reports as cached.
	Banner       bool   // Prints the per-port install header; one node per job wants it.
	SkipDepCheck bool   // Skips the circular and version conflict checks.
}

// Install installs one port and returns where it came from.
func Install(ctx context.Context, nameVersion string, devDep bool, options configs.InstallOptions, expect Expectation) (string, error) {
	if expect.Banner {
		platformName := expr.If(ctx.Platform().GetName() != "", ctx.Platform().GetName(), "native")

		// Display install header.
		logger.Println(logger.Title, "=======================================================================")
		logger.Printf(logger.Title, "🚀 start to install %s\n", nameVersion)
		logger.Printf(logger.Title, "📌 platform: %s\n", platformName)
		logger.Printf(logger.Title, "📌 product: %s\n", ctx.Project().GetName())
		logger.Println(logger.Title, "=======================================================================")
	}

	// Init the port.
	var port = configs.Port{
		DevDep: devDep,
	}
	if err := port.Init(ctx, nameVersion); err != nil {
		if errors.Is(err, errors.ErrPortNotFound) {
			err := fmt.Errorf("port %s is not yet available - consider adding it now ?", nameVersion)
			return "", logger.PrintError(err, "failed to install %s", nameVersion)
		}
		return "", logger.PrintError(err, "failed to init %s", nameVersion)
	}

	// Check circular dependence and version conflict. A schedule's graph was checked
	// where it was exported, so an apply run skips this for every one of its nodes.
	if !expect.SkipDepCheck {
		checker := depcheck.NewDepCheck()
		if err := checker.CheckCircular(ctx, port); err != nil {
			return "", logger.PrintError(err, "failed to check circular dependence.")
		}
		if err := checker.CheckConflict(ctx, port); err != nil {
			return "", logger.PrintError(err, "failed to check version conflict.")
		}
	}

	// With a schedule the expected build hash of this node is known up front: a
	// mismatch means the schedule no longer describes this workspace, so report it
	// instead of silently building a different revision.
	if expect.BuildHash != "" {
		buildhash, err := port.BuildHash()
		if err != nil {
			return "", logger.PrintError(err, "failed to calculate the build hash of %s", nameVersion)
		}
		if buildhash != expect.BuildHash {
			return "", logger.PrintError(
				fmt.Errorf("build hash mismatch: '%s' expects '%s' but this workspace computes '%s'", expect.DagPath, expect.BuildHash, buildhash),
				"failed to install '%s'", nameVersion)
		}
	}

	fromWhere, err := port.Install(options)
	if err != nil {
		return "", logger.PrintError(err, "failed to install %s", nameVersion)
	}

	// An explicitly requested path that could not serve this port is a failure:
	// the default chain is the only mode that may fall back to another path.
	if options.Prefer != "" && fromWhere == "" {
		return "", logger.PrintError(fmt.Errorf("%s is not available from %s", nameVersion, preferName(options.Prefer)),
			"failed to install %s", nameVersion)
	}

	if fromWhere != "" {
		if port.DevDep {
			logger.PrintSuccess("install %s from %s as dev successfully.", nameVersion, fromWhere)
		} else {
			logger.PrintSuccess("install %s from %s successfully.", nameVersion, fromWhere)
		}
	} else {
		if port.DevDep {
			logger.PrintSuccess("install %s as dev successfully.", nameVersion)
		} else {
			logger.PrintSuccess("install %s successfully.", nameVersion)
		}
	}

	return fromWhere, nil
}

// parseDag reads a DAG written by Export.
func parseDag(dagPath, celerVersion string) (*Dag, error) {
	bytes, err := os.ReadFile(dagPath)
	if err != nil {
		return nil, fmt.Errorf("failed to read %s -> %w", dagPath, err)
	}

	var dag Dag
	if err := json.Unmarshal(bytes, &dag); err != nil {
		return nil, fmt.Errorf("failed to parse %s -> %w", dagPath, err)
	}

	if dag.CelerVersion != celerVersion {
		return nil, fmt.Errorf("celer version doesn't matches, %s was exported by celer %s", dagPath, dag.CelerVersion)
	}
	return &dag, nil
}

// preferName names a strict install path the way --prefer takes it.
func preferName(prefer configs.InstallPrefer) string {
	switch prefer {
	case configs.PreferSource:
		return "source"
	case configs.PreferPackage:
		return "package"
	case configs.PreferPkgCache:
		return "pkgcache"
	case configs.PreferDevCache:
		return "devcache"
	default:
		return ""
	}
}
