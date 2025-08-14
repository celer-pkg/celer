package cmds

import (
	"celer/buildtools"
	"celer/configs"
	"celer/depcheck"
	"celer/pkgs/expr"
	"strings"

	"github.com/spf13/cobra"
)

type deployCmd struct {
	celer     *configs.Celer
	devMode   bool
	buildType string
}

func (d deployCmd) Command() *cobra.Command {
	command := &cobra.Command{
		Use:   "deploy",
		Short: "Deploy with selected platform and project.",
		Run: func(cmd *cobra.Command, args []string) {
			// Init celer.
			d.celer = configs.NewCeler()
			if err := d.celer.Init(); err != nil {
				configs.PrintError(err, "failed to init celer.")
				return
			}

			// Override dev mode if specified.
			buildtools.DevMode = d.devMode
			configs.DevMode = d.devMode

			// Override build_type if specified.
			if d.buildType != "" {
				d.celer.Gloabl.BuildType = d.buildType
			}

			// Check circular dependency and version conflict.
			if err := d.checkProject(); err != nil {
				configs.PrintError(err, "check circular dependency and version conflict failed.")
				return
			}

			if err := d.celer.Deploy(); err != nil {
				configs.PrintError(err, "failed to deploy celer.")
				return
			}

			// In dev mode, skip generate toolchain file.
			if !d.devMode {
				if err := d.celer.GenerateToolchainFile(); err != nil {
					configs.PrintError(err, "failed to generate toolchain file.")
					return
				}
			}

			if !d.devMode {
				projectName := expr.If(d.celer.Gloabl.Project == "", "unnamed", d.celer.Gloabl.Project)
				configs.PrintSuccess("celer is ready for project: %s.", projectName)
			}
		},
		ValidArgsFunction: d.completion,
	}

	// Register flags.
	command.Flags().BoolVarP(&d.devMode, "dev-mode", "d", false, "deploy in dev mode.")
	command.Flags().StringVarP(&d.buildType, "build-type", "b", "release", "deploy with build type.")

	return command
}

func (d deployCmd) checkProject() error {
	depcheck := depcheck.NewDepCheck()

	var ports []configs.Port
	for _, nameVersion := range d.celer.Project().Ports {
		var port configs.Port
		if err := port.Init(d.celer, nameVersion, d.celer.Gloabl.BuildType); err != nil {
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

func (d deployCmd) completion(cmd *cobra.Command, args []string, toComplete string) ([]string, cobra.ShellCompDirective) {
	var suggestions []string
	for _, flag := range []string{"--dev", "-d", "--build-type", "-b"} {
		if strings.HasPrefix(flag, toComplete) {
			suggestions = append(suggestions, flag)
		}
	}
	return suggestions, cobra.ShellCompDirectiveNoFileComp
}
