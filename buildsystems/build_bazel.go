package buildsystems

import (
	"celer/context"
)

func NewBazel(config *BuildConfig, optimize *context.Optimize) *bazel {
	return &bazel{
		BuildConfig: config,
		Optimize:    optimize,
	}
}

type bazel struct {
	*BuildConfig
	*context.Optimize
}

func (b bazel) Name() string {
	return "bazel"
}

func (b bazel) CheckTools() []string {
	b.BuildConfig.BuildTools = append(b.BuildConfig.BuildTools, "git", "cmake")
	return b.BuildConfig.BuildTools
}

func (b bazel) configured() bool {
	return false
}

func (b bazel) Configure(options []string) error {
	return nil
}

func (b bazel) Build(options []string) error {
	return nil
}

func (b bazel) Install(options []string) error {
	return nil
}
