package cmds

import (
	"celer/configs"
	"celer/depcheck"
	"celer/pkgs/color"
	"celer/pkgs/dirs"
	"celer/timemachine"
	"fmt"
	"path/filepath"
	"strings"

	"github.com/spf13/cobra"
)

type deployCmd struct {
	celer      *configs.Celer
	force      bool
	exportPath string
}

func (d *deployCmd) Command(celer *configs.Celer) *cobra.Command {
	d.celer = celer
	command := &cobra.Command{
		Use:   "deploy",
		Short: "Deploy with selected platform and project.",
		Long: `Deploy builds and installs all packages defined in the project.

After successful deployment, you can optionally export a snapshot
for reproducible builds using the --export flag.`,
		Args: d.validateArgs,
		RunE: func(cmd *cobra.Command, args []string) error {
			if err := d.celer.Init(); err != nil {
				return configs.PrintError(err, "failed to init celer.")
			}

			// Display deployment header.
			color.Println(color.Title, "=======================================================================")
			color.Println(color.Title, "🚀 start to deploy with below configurations: ")
			color.Printf(color.Title, "🛠️  platform: %s\n", d.celer.Global.Platform)
			color.Printf(color.Title, "🛠️  project : %s\n", d.celer.Global.Project)
			color.Println(color.Title, "=======================================================================")

			// Check circular dependency and version conflict.
			if err := d.checkProject(); err != nil {
				return configs.PrintError(err, "failed to check circular dependency and version conflict.")
			}

			if err := d.celer.Deploy(d.force); err != nil {
				return configs.PrintError(err, "failed to deploy celer.")
			}

			configs.PrintSuccess("%s has been successfully deployed.", d.celer.Global.Project)

			// Export snapshot if requested.
			if d.exportPath != "" {
				if err := timemachine.Export(d.celer, d.exportPath); err != nil {
					return configs.PrintError(err, "failed to export workspace.")
				}
			}

			return nil
		},
		ValidArgsFunction: d.completion,
	}

	flags := command.Flags()
	flags.StringVar(&d.exportPath, "export", "", "Export workspace snapshot after successfully deployed.")
	flags.BoolVarP(&d.force, "force", "f", false, "Force deployment, ignoring any installed packages.")

	// Silence cobra's error and usage output to avoid duplicate messages.
	command.SilenceErrors = true
	command.SilenceUsage = true
	return command
}

func (d *deployCmd) validateArgs(cmd *cobra.Command, args []string) error {
	if err := cobra.NoArgs(cmd, args); err != nil {
		return err
	}

	if !cmd.Flags().Changed("export") {
		return nil
	}

	exportPath, err := cmd.Flags().GetString("export")
	if err != nil {
		return err
	}

	exportPath = strings.TrimSpace(exportPath)
	if exportPath == "" {
		return fmt.Errorf("--export requires a non-empty path")
	}

	if filepath.IsAbs(exportPath) {
		return fmt.Errorf("--export must be a relative path inside workspace")
	}

	cleanedPath := filepath.Clean(exportPath)
	if cleanedPath == "." || cleanedPath == ".." || strings.HasPrefix(cleanedPath, ".."+string(filepath.Separator)) {
		return fmt.Errorf("--export path must stay inside workspace")
	}

	cleanedPath = filepath.ToSlash(cleanedPath)
	root := strings.SplitN(cleanedPath, "/", 2)[0]
	protectedRoots := map[string]bool{
		"conf":       true,
		"ports":      true,
		"buildtrees": true,
		"packages":   true,
		"installed":  true,
		"tmp":        true,
		".git":       true,
	}
	if protectedRoots[root] {
		return fmt.Errorf("--export path cannot target protected workspace directory: %s", root)
	}

	d.exportPath = filepath.Join(dirs.WorkspaceDir, filepath.FromSlash(cleanedPath))
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

func (d *deployCmd) completion(cmd *cobra.Command, args []string, toComplete string) ([]string, cobra.ShellCompDirective) {
	var suggestions []string
	for _, flag := range []string{"--export", "--force", "-f"} {
		if strings.HasPrefix(flag, toComplete) {
			suggestions = append(suggestions, flag)
		}
	}
	return suggestions, cobra.ShellCompDirectiveNoFileComp
}
