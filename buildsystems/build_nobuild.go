package buildsystems

func NewNoBuild(config *BuildConfig) *nobuild {
	return &nobuild{
		BuildConfig: config,
	}
}

type nobuild struct {
	*BuildConfig
}

func (n nobuild) CheckTools() []string {
	return nil
}

func (n nobuild) Name() string {
	return "nobuild"
}

func (n nobuild) configured() bool {
	return false
}

func (n nobuild) Configure(options []string) error {
	return nil
}

func (n nobuild) Build(options []string) error {
	return nil
}

func (n nobuild) Install(options []string) error {
	return nil
}
