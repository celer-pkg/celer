package buildsystems

import (
	"celer/buildtools"
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
	}
}

type prebuilt struct {
	*BuildConfig
	*context.Optimize
}

func (p prebuilt) Name() string {
	return "prebuilt"
}

func (p prebuilt) CheckTools() error {
	p.BuildTools = append(p.BuildTools, "git", "cmake")
	return buildtools.CheckTools(p.Ctx, p.BuildTools...)
}

func (p prebuilt) Clean() error {
	// No repo to clean.
	return nil
}

func (p prebuilt) Clone(url, ref, archive string) error {
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
			if len(entities) == 1 {
				srcDir := filepath.Join(p.PortConfig.PackageDir, entities[0].Name())
				if err := fileio.RenameDir(srcDir, p.PortConfig.PackageDir); err != nil {
					return err
				}
			}
		}
	}

	return nil
}

func (b prebuilt) configured() bool {
	return false // No need to configure.
}

func (p prebuilt) Configure(options []string) error {
	return nil // No need to configure.
}

func (p prebuilt) Build(options []string) error {
	return nil // No need to build.
}

func (p prebuilt) Install(options []string) error {
	return nil // No need to install.
}
