package cmds

import (
	"celer/configs"
	"fmt"

	"github.com/spf13/cobra"
)

type aboutCmd struct{}

func (a aboutCmd) Command() *cobra.Command {
	return &cobra.Command{
		Use:   "about",
		Short: "About celer.",
		Run: func(cmd *cobra.Command, args []string) {
			// Init celer.
			celer := configs.NewCeler()
			if err := celer.Init(); err != nil {
				configs.PrintError(err, "failed to init celer.")
				return
			}

			fmt.Print(celer.About())
		},
	}
}
