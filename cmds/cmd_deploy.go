package cmds

import (
	"fmt"
	"os"
	"path/filepath"
	"strings"
	"time"

	"github.com/celer-pkg/celer/configs"
	"github.com/celer-pkg/celer/dag"
	"github.com/celer-pkg/celer/depcheck"
	"github.com/celer-pkg/celer/pkgs/dirs"
	"github.com/celer-pkg/celer/pkgs/expr"
	"github.com/celer-pkg/celer/pkgs/logger"
	"github.com/celer-pkg/celer/pkgs/refs"
	"github.com/celer-pkg/celer/snapshot"

	"github.com/spf13/cobra"
)

type deployCmd struct {
	celer        *configs.Celer
	force        bool
	snapshotPath string
	strip        bool
	exportDag    string
	applyDag     string

	dagInfo *dag.DagInfo
}

func (d *deployCmd) Command(celer *configs.Celer) *cobra.Command {
	d.celer = celer
	command := &cobra.Command{
		Use:   "deploy",
		Short: "Deploy with selected platform and project.",
		Long: `Deploy builds and installs all packages defined in the current project.

After successful deployment, you can optionally export a snapshot
for reproducible builds using the --snapshot flag, and you can also
strip installed binaries and libraies with --strip.

A build schedule exported by --export-dag can be applied here with --apply-dag: the
workspace then takes the schedule's identity, its pinned conf and source revisions and
its build hashes, and installs exactly the nodes the schedule names, from pkgcache.

Examples:
  celer deploy --force                     # Force deploy and ignore installed
  celer deploy --snapshot=${filepath}      # Initialize with conf repo
  celer deploy --strip                     # Strip installed binaries and libraries
  celer deploy --export-dag=dag.json       # Export the build DAG instead of deploying
  celer deploy --apply-dag=dag.json        # Collect the nodes of that DAG from pkgcache`,
		Args: d.validateArgs,
		RunE: func(cmd *cobra.Command, args []string) error {
			// A schedule is read before celer is initialized.
			if d.applyDag != "" {
				dagInfo, err := dag.Import(d.celer, d.applyDag)
				if err != nil {
					return logger.PrintError(err, "failed to read the build schedule %s.", d.applyDag)
				}
				d.dagInfo = dagInfo
			}

			if err := d.celer.Init(); err != nil {
				return logger.PrintError(err, "failed to init celer.")
			}

			platformName := expr.If(d.celer.Platform().GetName() != "", d.celer.Platform().GetName(), "native")
			projectName := d.celer.Project().GetName()

			// Display deployment header.
			logger.Println(logger.Title, "=======================================================================")
			logger.Printf(logger.Title, "🚀 start to %s:\n", d.action())
			logger.Printf(logger.Title, "📌 platform: %s\n", platformName)
			logger.Printf(logger.Title, "📌 project: %s\n", projectName)
			logger.Println(logger.Title, "=======================================================================")

			if d.dagInfo != nil {
				if err := d.collectArtifacts(); err != nil {
					return logger.PrintError(err, "failed to apply the build schedule %s.", d.applyDag)
				}
				if d.strip {
					if err := d.celer.Strip(); err != nil {
						return logger.PrintError(err, "failed to strip the collected tree.")
					}
				}
			} else {
				// Check circular dependency and version conflict.
				if err := d.checkProject(); err != nil {
					return logger.PrintError(err, "failed to check circular dependency and version conflict.")
				}

				// Resolve all dependency refs before any clone/download begins.
				if err := d.resolveAllRefs(); err != nil {
					return logger.PrintError(err, "failed to resolve refs.")
				}

				// Export the build DAG of the project instead of deploying it: the
				// orchestrator of a distributed build schedules one job per node
				// from this file.
				if d.exportDag != "" {
					return dag.Export(d.celer, d.exportDag)
				}

				if err := d.celer.Deploy(d.force, d.strip); err != nil {
					return logger.PrintError(err, "failed to deploy celer.")
				}
			}

			// Export snapshot if requested.
			if d.snapshotPath != "" {
				if err := snapshot.Export(d.celer, d.snapshotPath); err != nil {
					return fmt.Errorf("failed to export snapshot -> %w", err)
				}
			}

			logger.PrintSuccess("%s has been successfully %s.", projectName, expr.If(d.dagInfo != nil, "applied", "deployed"))
			return nil
		},
		ValidArgsFunction: d.completion,
	}

	flags := command.Flags()
	flags.StringVar(&d.snapshotPath, "snapshot", "", "Export workspace snapshot after successfully deployed.")
	flags.BoolVarP(&d.force, "force", "", false, "Force deployment, ignoring any installed packages.")
	flags.BoolVarP(&d.strip, "strip", "", false, "Build runtime stripped tree under workspace/stripped (same as celer strip).")
	flags.StringVar(&d.exportDag, "export-dag", "", "Export the build DAG of the project as JSON to <file>.")
	flags.StringVar(&d.applyDag, "apply-dag", "", "Install the nodes of a build DAG exported by --export-dag.")

	// Silence cobra's error and usage output to avoid duplicate messages.
	command.SilenceErrors = true
	command.SilenceUsage = true
	return command
}

func (d *deployCmd) action() string {
	switch {
	case d.dagInfo != nil:
		return "apply dag"
	case d.exportDag != "":
		return "export dag"
	default:
		return "deploy"
	}
}

// collectArtifacts installs every node the schedule names, plus the ones pkgcache
// already had, and reports where each one came from. Applying the schedule (its
// identity and pins) happened in dag.Import; this is the part that fills the workspace.
func (d *deployCmd) collectArtifacts() error {
	// Set prefer as 'pkgcache' to make sure all artifacts can be retrieved from pkgcache.
	options := configs.InstallOptions{Prefer: configs.PreferPkgCache}

	nodes := d.dagInfo.Dag.ApplyNodes()
	fromPkgCache := 0
	var local []string

	for _, nameVersion := range nodes {
		fromWhere, err := dag.Install(d.celer, nameVersion, false, options, dag.Expectation{
			DagPath:      d.applyDag,
			BuildHash:    d.dagInfo.DagHashes[nameVersion],
			SkipDepCheck: true,
		})
		if err != nil {
			return err
		}

		if fromWhere == "pkgcache" {
			fromPkgCache++
			continue
		}

		local = append(local, fmt.Sprintf("%s (%s)", nameVersion, expr.If(fromWhere != "", fromWhere, "not installed")))
	}

	if len(local) > 0 {
		logger.PrintWarning("%d of %d nodes did not come from pkgcache: %s", len(local), len(nodes), strings.Join(local, ", "))
	}
	if devHost := len(d.dagInfo.Dag.LocalNodes); devHost > 0 {
		logger.PrintInfo("%d dev/host nodes are built locally by every machine and are never part of the shared pkgcache.", devHost)
	}

	logger.PrintSuccess("applied %s: %d nodes, %d from pkgcache, %d from the local build.",
		d.applyDag, len(nodes), fromPkgCache, len(local))
	return nil
}

func (d *deployCmd) validateArgs(cmd *cobra.Command, args []string) error {
	if err := cobra.NoArgs(cmd, args); err != nil {
		return err
	}

	if cmd.Flags().Changed("snapshot") {
		snapshotPath, err := cmd.Flags().GetString("snapshot")
		if err != nil {
			return err
		}

		snapshotPath = strings.TrimSpace(snapshotPath)
		if snapshotPath == "" {
			return fmt.Errorf("--snapshot requires a non-empty path")
		}

		d.snapshotPath = filepath.Clean(snapshotPath)
	}

	if cmd.Flags().Changed("export-dag") {
		dagPath, err := cmd.Flags().GetString("export-dag")
		if err != nil {
			return err
		}

		dagPath = strings.TrimSpace(dagPath)
		if dagPath == "" {
			return fmt.Errorf("--export-dag requires a non-empty path")
		}

		d.exportDag = filepath.Clean(dagPath)
	}

	if cmd.Flags().Changed("apply-dag") {
		dagPath, err := cmd.Flags().GetString("apply-dag")
		if err != nil {
			return err
		}

		dagPath = strings.TrimSpace(dagPath)
		if dagPath == "" {
			return fmt.Errorf("--apply-dag requires a non-empty path")
		}

		d.applyDag = filepath.Clean(dagPath)
	}

	if d.exportDag != "" && d.applyDag != "" {
		return fmt.Errorf("--export-dag and --apply-dag are mutually exclusive")
	}

	if d.applyDag != "" && d.force {
		return fmt.Errorf("--apply-dag installs the schedule as it is, so it cannot be combined with --force")
	}

	return nil
}

func (d *deployCmd) checkProject() error {
	depcheck := depcheck.NewDepCheck()

	var ports []configs.Port
	for _, nameVersion := range d.celer.Project().GetPorts() {
		var port configs.Port
		if err := port.Init(d.celer, nameVersion); err != nil {
			return err
		}

		// Check if every port have circular dependency.
		if err := depcheck.CheckCircular(d.celer, port); err != nil {
			return err
		}

		ports = append(ports, port)
	}

	// Check if ports have conflict versions.
	if err := depcheck.CheckConflict(d.celer, ports...); err != nil {
		return err
	}

	return nil
}

func (d *deployCmd) resolveAllRefs() error {
	// Collect all ports (top-level + transitive dependencies) into []refs.PortInfo.
	collected := make(map[string]refs.PortInfo)
	var collect func(nameVersion string) error

	collect = func(nameVersion string) error {
		if _, exists := collected[nameVersion]; exists {
			return nil
		}

		var port configs.Port
		if err := port.Init(d.celer, nameVersion); err != nil {
			return err
		}
		collected[nameVersion] = refs.PortInfo{
			NameVersion: port.NameVersion(),
			Url:         port.Package.Url,
			Ref:         port.Package.Ref,
			Checksum:    port.Package.Checksum,
		}
		for _, dep := range port.MatchedConfig.Dependencies {
			if err := collect(dep); err != nil {
				return err
			}
		}
		for _, dep := range port.MatchedConfig.DevDependencies {
			if err := collect(dep); err != nil {
				return err
			}
		}
		return nil
	}

	for _, port := range d.celer.Project().GetPorts() {
		if err := collect(port); err != nil {
			return err
		}
	}

	var portInfos []refs.PortInfo
	for _, info := range collected {
		portInfos = append(portInfos, info)
	}

	projectName := d.celer.Project().GetName()
	resolvedRefs := refs.ResolvePorts(portInfos)

	// Store resolved commits for use during clone/checkout.
	commits := make(map[string]string, len(resolvedRefs))
	for _, r := range resolvedRefs {
		if r.ResolvedCommit != "" {
			commits[r.NameVersion] = r.ResolvedCommit
		}
	}
	refs.StoreResolvedCommits(commits)
	refs.PrintResolvedRefs(projectName, resolvedRefs)

	// Save to file in deployments.
	timestamp := time.Now().Format(fmt.Sprintf("%s_20060102_150405", projectName))
	filePath := filepath.Join(dirs.InstalledDir, "infos", "deployments", timestamp+".md")
	if err := os.MkdirAll(filepath.Dir(filePath), os.ModePerm); err != nil {
		return err
	}

	env := snapshot.BuildEnv{
		ExportedAt:   time.Now(),
		CelerVersion: d.celer.Version(),
		Platform:     d.celer.Platform().GetName(),
		Project:      projectName,
	}
	if err := snapshot.SaveSnapshotMarkdown(filePath, env, resolvedRefs); err != nil {
		return fmt.Errorf("failed to save snapshot -> %w", err)
	}
	logger.Printf(logger.Success, "Snapshot saved to: %s\n", filePath)

	// Abort deploy if any ref resolution failed.
	var failedPorts []string
	for _, r := range resolvedRefs {
		if r.Error != "" {
			failedPorts = append(failedPorts, r.NameVersion)
		}
	}
	if len(failedPorts) > 0 {
		return fmt.Errorf("ref resolution failed for: %s", strings.Join(failedPorts, ", "))
	}

	return nil
}

func (d *deployCmd) completion(cmd *cobra.Command, args []string, toComplete string) ([]string, cobra.ShellCompDirective) {
	var suggestions []string
	for _, flag := range []string{"--snapshot", "--force", "--strip", "--export-dag", "--apply-dag"} {
		if strings.HasPrefix(flag, toComplete) {
			suggestions = append(suggestions, flag)
		}
	}
	return suggestions, cobra.ShellCompDirectiveNoFileComp
}
