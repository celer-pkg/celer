package cmds

import (
	"fmt"
	"os"
	"path/filepath"
	"strings"
	"time"

	"github.com/celer-pkg/celer/configs"
	"github.com/celer-pkg/celer/depcheck"
	"github.com/celer-pkg/celer/pkgs/color"
	"github.com/celer-pkg/celer/pkgs/dirs"
	"github.com/celer-pkg/celer/pkgs/expr"
	"github.com/celer-pkg/celer/pkgs/refs"
	"github.com/celer-pkg/celer/snapshot"

	"github.com/spf13/cobra"
)

type deployCmd struct {
	celer        *configs.Celer
	force        bool
	snapshotPath string
	strip        bool
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

Examples:
  celer deploy --force                  # Force deploy and ignore installed
  celer deploy --snapshot=${filepath}   # Initialize with conf repo
  celer deploy --strip                  # Strip installed binaries and libraries`,
		Args: d.validateArgs,
		RunE: func(cmd *cobra.Command, args []string) error {
			if err := d.celer.Init(); err != nil {
				return color.PrintError(err, "failed to init celer.")
			}

			platformName := expr.If(d.celer.Platform().GetName() != "", d.celer.Platform().GetName(), "native")
			projectName := d.celer.Project().GetName()

			// Display deployment header.
			color.Println(color.Title, "=======================================================================")
			color.Printf(color.Title, "🚀 start to deploy:\n")
			color.Printf(color.Title, "📌 platform: %s\n", platformName)
			color.Printf(color.Title, "📌 project: %s\n", projectName)
			color.Println(color.Title, "=======================================================================")

			// Check circular dependency and version conflict.
			if err := d.checkProject(); err != nil {
				return color.PrintError(err, "failed to check circular dependency and version conflict.")
			}

			// Resolve all dependency refs before any clone/download begins.
			if err := d.resolveAllRefs(); err != nil {
				return color.PrintError(err, "failed to resolve refs.")
			}

			if err := d.celer.Deploy(d.force, d.strip); err != nil {
				return color.PrintError(err, "failed to deploy celer.")
			}

			color.PrintSuccess("%s has been successfully deployed.", projectName)

			// Export snapshot if requested.
			if d.snapshotPath != "" {
				if err := snapshot.Export(d.celer, d.snapshotPath); err != nil {
					return fmt.Errorf("failed to export snapshot -> %w", err)
				}
			}

			return nil
		},
		ValidArgsFunction: d.completion,
	}

	flags := command.Flags()
	flags.StringVar(&d.snapshotPath, "snapshot", "", "Export workspace snapshot after successfully deployed.")
	flags.BoolVarP(&d.force, "force", "", false, "Force deployment, ignoring any installed packages.")
	flags.BoolVarP(&d.strip, "strip", "", false, "Strip installed binaries and libraries.")

	// Silence cobra's error and usage output to avoid duplicate messages.
	command.SilenceErrors = true
	command.SilenceUsage = true
	return command
}

func (d *deployCmd) validateArgs(cmd *cobra.Command, args []string) error {
	if err := cobra.NoArgs(cmd, args); err != nil {
		return err
	}

	if !cmd.Flags().Changed("snapshot") {
		return nil
	}

	snapshotPath, err := cmd.Flags().GetString("snapshot")
	if err != nil {
		return err
	}

	snapshotPath = strings.TrimSpace(snapshotPath)
	if snapshotPath == "" {
		return fmt.Errorf("--snapshot requires a non-empty path")
	}

	d.snapshotPath = filepath.Clean(snapshotPath)
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
	filePath := filepath.Join(dirs.InstalledDir, "celer", "deployments", timestamp+".md")
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
	color.Printf(color.Success, "Snapshot saved to: %s\n", filePath)

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
	for _, flag := range []string{"--snapshot", "--force", "--strip"} {
		if strings.HasPrefix(flag, toComplete) {
			suggestions = append(suggestions, flag)
		}
	}
	return suggestions, cobra.ShellCompDirectiveNoFileComp
}
