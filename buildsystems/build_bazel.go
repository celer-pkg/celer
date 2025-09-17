package buildsystems

import (
	"celer/buildtools"
)

func NewBazel(config *BuildConfig, optFlags *OptFlags) *bazel {
	return &bazel{
		BuildConfig: config,
		OptFlags:    optFlags,
	}
}

type bazel struct {
	*BuildConfig
	*OptFlags
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
