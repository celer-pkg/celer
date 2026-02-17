package buildsystems

import (
	"celer/context"
	"celer/pkgs/fileio"
	"os"
	"path/filepath"
	"slices"
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
	// Start with build_tools from port.toml
	tools := slices.Clone(p.BuildConfig.BuildTools)

	// Add default tools
	tools = append(tools, "cmake")
	return tools
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
	// If CMakeLists.txt exists, use CMake to install.
	if fileio.PathExists(filepath.Join(p.PortConfig.RepoDir, "CMakeLists.txt")) {
		return p.cmakeInside.Install(options)
	} else {
		// For pure prebuilt without CMakeLists.txt, just copy files from repo dir to package dir.
		if err := os.MkdirAll(p.PortConfig.PackageDir, os.ModePerm); err != nil {
			return err
		}

		entities, err := os.ReadDir(p.PortConfig.RepoDir)
		if err != nil {
			return err
		}

		for _, entity := range entities {
			// .git should not be the installed files.
			if entity.Name() == ".git" {
				continue
			}

			srcPath := filepath.Join(p.PortConfig.RepoDir, entity.Name())
			destPath := filepath.Join(p.PortConfig.PackageDir, entity.Name())
			if entity.IsDir() {
				if err := fileio.CopyDir(srcPath, destPath); err != nil {
					return err
				}
			} else {
				if err := fileio.CopyFile(srcPath, destPath); err != nil {
					return err
				}
			}
		}
		return nil
	}
}
