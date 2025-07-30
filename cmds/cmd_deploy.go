package cmds

import (
	"celer/buildtools"
	"celer/configs"
	"celer/depcheck"
	"celer/pkgs/expr"
	"strings"

	"github.com/spf13/cobra"
)

type deployCmd struct{}

func (d deployCmd) Command() *cobra.Command {
	command := &cobra.Command{
		Use:   "deploy",
		Short: "Deploy with selected platform and project.",
		Run: func(cmd *cobra.Command, args []string) {
			devMode, _ := cmd.Flags().GetBool("dev-mode")
			buildType, _ := cmd.Flags().GetString("build-type")

			// Override dev mode if specified.
			buildtools.DevMode = devMode
			configs.DevMode = devMode

			// Init celer.
			celer := configs.NewCeler()
			if err := celer.Init(); err != nil {
				configs.PrintError(err, "failed to init celer.")
				return
			}

			// Override build_type if specified.
			if buildType != "" {
				celer.Settings.BuildType = buildType
			}

			// Check circular dependency and version conflict.
			if err := d.checkProject(celer); err != nil {
				configs.PrintError(err, "check circular dependency and version conflict failed.")
				return
			}

			if err := celer.Deploy(); err != nil {
				configs.PrintError(err, "failed to deploy celer.")
				return
			}

			// Generate toolchain file in not in dev mode.
			if !devMode {
				if err := celer.GenerateToolchainFile(); err != nil {
					configs.PrintError(err, "failed to generate toolchain file.")
					return
				}
			}

			if !devMode {
				projectName := expr.If(celer.Settings.Project == "", "unnamed", celer.Settings.Project)
				configs.PrintSuccess("celer is ready for project: %s.", projectName)
			}
		},
		ValidArgsFunction: func(cmd *cobra.Command, args []string, toComplete string) ([]string, cobra.ShellCompDirective) {
			// Support flags completion.
			var suggestions []string
			for _, flag := range []string{"--dev", "--build-type"} {
				if strings.HasPrefix(flag, toComplete) {
					suggestions = append(suggestions, flag)
				}
			}
			return suggestions, cobra.ShellCompDirectiveNoFileComp
		},
	}

	// Register flags.
	command.Flags().Bool("dev", false, "deploy in dev mode.")
	command.Flags().String("build-type", "", "deploy with specified build type.")

	return command
}

func (d deployCmd) checkProject(ctx configs.Context) error {
	depcheck := depcheck.NewDepCheck()

	var ports []configs.Port
	for _, nameVersion := range ctx.Project().Ports {
		var port configs.Port
		if err := port.Init(ctx, nameVersion, ctx.BuildType()); err != nil {
			return err
		}

		// Check if every port have circular dependency.
		if err := depcheck.CheckCircular(ctx, port); err != nil {
			return err
		}

		ports = append(ports, port)
	}

	// Check if ports have conflict versions.
	if err := depcheck.CheckConflict(ctx, ports...); err != nil {
		return err
	}

	return nil
}
