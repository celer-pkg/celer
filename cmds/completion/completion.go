package completion

type Completion interface {
	Register() error
	Unregister() error

	installBinary() error
	uninstallBinary() error

	installCompletion() error
	uninstallCompletion() error

	registerRunCommand() error
	unregisterRunCommand() error
}
