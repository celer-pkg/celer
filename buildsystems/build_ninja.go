package buildsystems

import (
	"celer/buildtools"
	"celer/pkgs/fileio"
	"path/filepath"
)

func NewNinja(config *BuildConfig, optimize *Optimize) *ninja {
	return &ninja{
		cmake: *NewCMake(config, optimize, "ninja"),
	}
}

type ninja struct {
	cmake
}

func (n ninja) Name() string {
	return "ninja"
}

func (n ninja) CheckTools() error {
	n.BuildConfig.BuildTools = append(n.BuildConfig.BuildTools, "git", "cmake", "ninja")
	return buildtools.CheckTools(n.BuildConfig.BuildTools...)
}

func (n ninja) Clean() error {
	return n.cmake.Clean()
}

func (n ninja) configureOptions() ([]string, error) {
	return n.cmake.configureOptions()
}

func (n ninja) configured() bool {
	cmakeCache := filepath.Join(n.PortConfig.BuildDir, "CMakeCache.txt")
	buildFile := filepath.Join(n.PortConfig.BuildDir, "build.ninja")
	ruluesFile := filepath.Join(n.PortConfig.BuildDir, "rules.ninja")
	return fileio.PathExists(cmakeCache) && fileio.PathExists(buildFile) && fileio.PathExists(ruluesFile)
}

func (n ninja) Configure(options []string) error {
	return n.cmake.Configure(options)
}

func (n ninja) buildOptions() ([]string, error) {
	return n.cmake.buildOptions()
}

func (n ninja) Build(options []string) error {
	return n.cmake.Build(options)
}

func (n ninja) installOptions() ([]string, error) {
	return n.cmake.installOptions()
}

func (n ninja) Install(options []string) error {
	return n.cmake.Install(options)
}
