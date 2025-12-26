package buildsystems

import (
	"celer/context"
	"celer/pkgs/cmd"
	"celer/pkgs/expr"
	"celer/pkgs/fileio"
	"fmt"
	"os"
	"path/filepath"
	"strings"
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

// TODO: maybe not needed for prebuilt?
func (p *prebuilt) Clone(url, ref, archive string, depth int) error {
	// Clone repo only when source dir not exists.
	if !fileio.PathExists(p.PortConfig.RepoDir) {
		if strings.HasSuffix(url, ".git") {
			// Clone repo.
			command := fmt.Sprintf("git clone --branch %s %s %s --recursive", ref, url, p.PortConfig.PackageDir)
			title := fmt.Sprintf("[clone %s]", p.PortConfig.nameVersionDesc())
			if err := cmd.NewExecutor(title, command).Execute(); err != nil {
				return err
			}
		} else {
			// Check and repair resource.
			archive = expr.If(archive == "", filepath.Base(url), archive)
			repair := fileio.NewRepair(url, archive, ".", p.PortConfig.PackageDir)
			if err := repair.CheckAndRepair(p.Ctx); err != nil {
				return err
			}

			// Move extracted files to source dir.
			entities, err := os.ReadDir(p.PortConfig.PackageDir)
			if err != nil || len(entities) == 0 {
				return fmt.Errorf("failed to find extracted files under tmp dir")
			}

			// When the extracted files are in the first level of the archive and
			// the first level directory is not "include", "lib" or "bin", then move it to source dir.
			if len(entities) == 1 {
				enetityName := entities[0].Name()
				if enetityName != "include" && enetityName != "lib" && enetityName != "bin" {
					srcDir := filepath.Join(p.PortConfig.PackageDir, entities[0].Name())
					if err := fileio.RenameDir(srcDir, p.PortConfig.PackageDir); err != nil {
						return err
					}
				}
			}
		}
	}

	return nil
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
