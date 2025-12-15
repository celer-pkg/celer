package cmds

import (
	"celer/configs"
	"celer/depcheck"
	"celer/pkgs/color"
	"celer/pkgs/expr"
	"celer/timemachine"
	"fmt"
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
		Run: func(cmd *cobra.Command, args []string) {
			if err := d.celer.Init(); err != nil {
				configs.PrintError(err, "failed to init celer.")
				return
			}

			// Display deployment header.
			fmt.Printf("%s %s %s %s\n",
				color.Color("Deploy with platform:", color.Blue, color.Bold),
				color.Color(d.celer.Global.Platform, color.BrightMagenta, color.Bold),
				color.Color("& project:", color.Blue, color.Bold),
				color.Color(d.celer.Global.Project, color.BrightMagenta, color.Bold),
			)
			titleLen := len(fmt.Sprintf("Deploy with platform: %s & project: %s",
				d.celer.Global.Platform,
				d.celer.Global.Project,
			))
			color.Println(color.Line, strings.Repeat("-", titleLen))

			if err := d.celer.Setup(); err != nil {
				configs.PrintError(err, "failed to setup celer.")
				return
			}

			// Check circular dependency and version conflict.
			if err := d.checkProject(); err != nil {
				configs.PrintError(err, "failed to check circular dependency and version conflict.")
				return
			}

			if err := d.celer.Deploy(d.force); err != nil {
				configs.PrintError(err, "failed to deploy celer.")
				return
			}

			projectName := expr.If(d.celer.Global.Project == "", "unnamed", d.celer.Global.Project)
			configs.PrintSuccess("The deployment is ready for project: %s", projectName)

			// Export snapshot if requested.
			if d.exportPath != "" {
				if err := timemachine.Export(d.celer, d.exportPath); err != nil {
					configs.PrintError(err, "failed to export workspace.")
					return
				}
			}
		},
		ValidArgsFunction: d.completion,
	}

	flags := command.Flags()
	flags.StringVar(&d.exportPath, "export", "", "Export workspace snapshot after successfully deployed.")
	flags.BoolVarP(&d.force, "force", "f", false, "Force deployment, ignoring any installed packages.")

	return command
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
	for _, flag := range []string{"--export", "--force"} {
		if strings.HasPrefix(flag, toComplete) {
			suggestions = append(suggestions, flag)
		}
	}
	return suggestions, cobra.ShellCompDirectiveNoFileComp
}
