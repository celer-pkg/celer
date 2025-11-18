package buildsystems

import (
	"celer/context"
)

func NewNoBuild(config *BuildConfig, optimize *context.Optimize) *nobuild {
	return &nobuild{
		BuildConfig: config,
		Optimize:    optimize,
	}
}

type nobuild struct {
	*BuildConfig
	*context.Optimize
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
