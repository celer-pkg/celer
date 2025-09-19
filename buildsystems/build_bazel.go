package buildsystems

import (
	"celer/buildtools"
)

func NewBazel(config *BuildConfig, optimize Optimize) *bazel {
	return &bazel{
		BuildConfig: config,
		Optimize:    optimize,
	}
}

type bazel struct {
	*BuildConfig
	Optimize
}

func (b bazel) Name() string {
	return "bazel"
}

func (b bazel) CheckTools() error {
	b.BuildConfig.BuildTools = append(b.BuildConfig.BuildTools, "git", "cmake")
	return buildtools.CheckTools(b.BuildConfig.BuildTools...)
}

func (b bazel) Clean() error {
	return nil
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
