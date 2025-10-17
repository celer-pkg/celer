package completion

import "github.com/spf13/cobra"

type completion interface {
	Register(homeDir string) error
	Unregister(homeDir string) error

	installBinary(homeDir string) error
	uninstallBinary(homeDir string) error

	installCompletion(cmd *cobra.Command, homeDir string) error
	uninstallCompletion(homeDir string) error

	registerRunCommand() error
	unregisterRunCommand(homeDir string) error
}
