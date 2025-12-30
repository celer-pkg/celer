package buildsystems

import (
	"celer/context"
	"celer/pkgs/fileio"
	"path/filepath"
)

func NewPrebuilt(config *BuildConfig, optimize *context.Optimize) *prebuilt {
	return &prebuilt{
		BuildConfig: config,
		Optimize:    optimize,
		cmakeInside: NewCMake(config, optimize),
	}
}

type prebuilt struct {
	*BuildConfig
	*context.Optimize

	// This is used to generate CMake config file for prebuilt.
	cmakeInside *cmake
}

func (p *prebuilt) Name() string {
	return "prebuilt"
}

func (p prebuilt) CheckTools() []string {
	p.BuildTools = append(p.BuildTools, "git", "cmake")
	return p.BuildConfig.BuildTools
}

func (p *prebuilt) configured() bool {
	if fileio.PathExists(filepath.Join(p.PortConfig.RepoDir, "CMakeLists.txt")) {
		return p.cmakeInside.configured()
	}
	return false
}

func (p *prebuilt) preConfigure() error {
	if fileio.PathExists(filepath.Join(p.PortConfig.RepoDir, "CMakeLists.txt")) {
		return p.cmakeInside.preConfigure()
	}
	return nil
}

func (p *prebuilt) configureOptions() ([]string, error) {
	if fileio.PathExists(filepath.Join(p.PortConfig.RepoDir, "CMakeLists.txt")) {
		return p.cmakeInside.configureOptions()
	}
	return []string{}, nil
}

func (p *prebuilt) Configure(options []string) error {
	if fileio.PathExists(filepath.Join(p.PortConfig.RepoDir, "CMakeLists.txt")) {
		return p.cmakeInside.Configure(options)
	}
	return nil
}

func (p *prebuilt) buildOptions() ([]string, error) {
	if fileio.PathExists(filepath.Join(p.PortConfig.RepoDir, "CMakeLists.txt")) {
		return p.cmakeInside.buildOptions()
	}
	return []string{}, nil
}

func (p *prebuilt) Build(options []string) error {
	if fileio.PathExists(filepath.Join(p.PortConfig.RepoDir, "CMakeLists.txt")) {
		return p.cmakeInside.Build(options)
	}
	return nil
}

func (p *prebuilt) installOptions() ([]string, error) {
	if fileio.PathExists(filepath.Join(p.PortConfig.RepoDir, "CMakeLists.txt")) {
		return p.cmakeInside.installOptions()
	}
	return []string{}, nil
}

func (p *prebuilt) Install(options []string) error {
	if fileio.PathExists(filepath.Join(p.PortConfig.RepoDir, "CMakeLists.txt")) {
		return p.cmakeInside.Install(options)
	}
	return nil
}
