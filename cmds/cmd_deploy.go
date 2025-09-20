package cmds

import (
	"celer/buildtools"
	"celer/configs"
	"celer/depcheck"
	"celer/pkgs/expr"

	"github.com/spf13/cobra"
)

type deployCmd struct {
	celer *configs.Celer
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

			if err := d.celer.GenerateToolchainFile(true); err != nil {
				configs.PrintError(err, "failed to generate toolchain file.")
				return
			}

			// Set offline mode.
			buildtools.Offline = d.celer.Global.Offline
			configs.Offline = d.celer.Global.Offline

			// Check circular dependency and version conflict.
			if err := d.checkProject(); err != nil {
				configs.PrintError(err, "check circular dependency and version conflict failed.")
				return
			}

			if err := d.celer.Deploy(); err != nil {
				configs.PrintError(err, "failed to deploy celer.")
				return
			}

			projectName := expr.If(d.celer.Global.Project == "", "unnamed", d.celer.Global.Project)
			configs.PrintSuccess("The deployment is ready for project: %s.", projectName)
		},
	}

	return command
}

func (d deployCmd) checkProject() error {
	depcheck := depcheck.NewDepCheck()

	var ports []configs.Port
	for _, nameVersion := range d.celer.Project().Ports {
		var port configs.Port
		if err := port.Init(d.celer, nameVersion, d.celer.Global.BuildType); err != nil {
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
