package cmds

import (
	"celer/configs"
	"celer/depcheck"
	"celer/pkgs/expr"
	"os"

	"github.com/spf13/cobra"
)

type deployCmd struct {
	celer *configs.Celer
}

func (d deployCmd) Command(celer *configs.Celer) *cobra.Command {
	d.celer = celer
	command := &cobra.Command{
		Use:   "deploy",
		Short: "Deploy with selected platform and project.",
		Run: func(cmd *cobra.Command, args []string) {
			if d.celer.HandleInitError() {
				os.Exit(1)
			}

			if err := d.celer.Platform().Setup(); err != nil {
				configs.PrintError(err, "setup platform error.")
				os.Exit(1)
			}

			// Check circular dependency and version conflict.
			if err := d.checkProject(); err != nil {
				configs.PrintError(err, "check circular dependency and version conflict failed.")
				os.Exit(1)
			}

			if err := d.celer.Deploy(); err != nil {
				configs.PrintError(err, "failed to deploy celer.")
				os.Exit(1)
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
	for _, nameVersion := range d.celer.Project().GetPorts() {
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
