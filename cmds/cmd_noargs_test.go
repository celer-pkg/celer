package cmds

import (
	"celer/configs"
	"testing"

	"github.com/spf13/cobra"
)

func TestNoArgsCommands_Validation(t *testing.T) {
	tests := []struct {
		name  string
		build func() *cobra.Command
	}{
		{
			name: "autoremove",
			build: func() *cobra.Command {
				return (&autoremoveCmd{}).Command(configs.NewCeler())
			},
		},
		{
			name: "configure",
			build: func() *cobra.Command {
				return (&configureCmd{}).Command(configs.NewCeler())
			},
		},
		{
			name: "deploy",
			build: func() *cobra.Command {
				return (&deployCmd{}).Command(configs.NewCeler())
			},
		},
		{
			name: "init",
			build: func() *cobra.Command {
				return (&initCmd{}).Command(configs.NewCeler())
			},
		},
		{
			name: "integrate",
			build: func() *cobra.Command {
				return (&integrateCmd{}).Command(configs.NewCeler())
			},
		},
		{
			name: "version",
			build: func() *cobra.Command {
				return (&versionCmd{}).Command(configs.NewCeler())
			},
		},
	}

	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			cmd := tt.build()
			if cmd.Args == nil {
				t.Fatalf("%s command should define Args validator", tt.name)
			}

			if err := cmd.Args(cmd, []string{}); err != nil {
				t.Fatalf("%s should accept empty positional args: %v", tt.name, err)
			}

			if err := cmd.Args(cmd, []string{"unexpected"}); err == nil {
				t.Fatalf("%s should reject unexpected positional args", tt.name)
			}
		})
	}
}
