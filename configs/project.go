package configs

import (
	"fmt"
	"io/fs"
	"os"
	"path/filepath"
	"strings"

	"github.com/celer-pkg/celer/context"
	"github.com/celer-pkg/celer/pkgs/cmd"
	"github.com/celer-pkg/celer/pkgs/color"
	"github.com/celer-pkg/celer/pkgs/dirs"
	"github.com/celer-pkg/celer/pkgs/errors"
	"github.com/celer-pkg/celer/pkgs/fileio"

	"github.com/BurntSushi/toml"
)

type Project struct {
	TargetPlatform string   `toml:"target_platform,omitempty"`
	BuildType      string   `toml:"build_type"`
	IncludeDirs    []string `toml:"include_dirs,omitempty"`
	LibDirs        []string `toml:"lib_dirs,omitempty"`
	Ports          []string `toml:"ports"`
	Vars           []string `toml:"vars"`
	Envs           []string `toml:"envs"`
	Macros         []string `toml:"macros"`

	// Internal fields.
	Name string `toml:"-"`
	ctx  context.Context
}

func (p *Project) Init(ctx context.Context, projectName string) error {
	p.ctx = ctx

	// Check if project name is empty.
	projectName = strings.TrimSpace(projectName)
	if projectName == "" {
		return fmt.Errorf("project name is empty")
	}

	// Check if project file exists.
	projectPath := filepath.Join(dirs.ConfProjectsDir, projectName+".toml")
	if !fileio.PathExists(projectPath) {
		return fmt.Errorf("%w: %s", errors.ErrProjectNotExist, projectName)
	}

	// Read conf/projects/<project_name>.toml.
	bytes, err := os.ReadFile(projectPath)
	if err != nil {
		return err
	}
	if err := toml.Unmarshal(bytes, p); err != nil {
		return fmt.Errorf("failed to read %s -> %w", projectPath, err)
	}

	// Default build_type.
	if p.BuildType == "" {
		p.BuildType = "Release"
	}

	// Set values of internal fields.
	p.Name = projectName

	return nil
}

func (p Project) Write(platformPath string, override bool) error {
	if p.BuildType == "" {
		p.BuildType = "Release"
	}
	if len(p.Ports) == 0 {
		p.Ports = []string{}
	}
	if len(p.Vars) == 0 {
		p.Vars = []string{}
	}
	if len(p.Envs) == 0 {
		p.Envs = []string{}
	}
	if len(p.Macros) == 0 {
		p.Macros = []string{}
	}

	bytes, err := toml.Marshal(p)
	if err != nil {
		return err
	}

	// Check if conf/projects/<project_name>.toml exists.
	if fileio.PathExists(platformPath) && !override {
		return fmt.Errorf("%s is already exists", platformPath)
	}

	// Make sure the parent directory exists.
	parentDir := filepath.Dir(platformPath)
	if err := os.MkdirAll(parentDir, os.ModePerm); err != nil {
		return err
	}

	return os.WriteFile(platformPath, bytes, os.ModePerm)
}

func (p Project) GetName() string {
	return p.Name
}

func (p Project) GetTargetPlatform() string {
	return p.TargetPlatform
}

func (p Project) GetPorts() []string {
	return p.Ports
}

func (p Project) GetVars() []string {
	return p.Vars
}

func (p Project) deploy(force, strip bool) error {
	options := InstallOptions{
		Force:     force,
		Recursive: true,
	}

	// Collect a single deploy-wide report that includes all project ports.
	deployReport := newInstallReport(p.GetName())

	for _, nameVersion := range p.Ports {
		var port Port
		if err := port.Init(p.ctx, nameVersion); err != nil {
			return fmt.Errorf("failed to init %s -> %w", nameVersion, err)
		}

		port.installReport = deployReport
		if _, err := port.Install(options); err != nil {
			return fmt.Errorf("failed to install %s -> %w", nameVersion, err)
		}
	}

	// Strip ELF binaries and shared libraries to deduce the file size.
	if strip {
		if err := p.stripDeployed(); err != nil {
			return fmt.Errorf("failed to strip deployed binaries -> %w", err)
		}
	}

	return nil
}

// stripDeployed walks the per-platform installed tree and runs strip on every
// ELF file found. Static archives (.a) are intentionally skipped — stripping
// them removes symbols downstream linking against this deploy still needs.
func (p Project) stripDeployed() error {
	// Check if strip executable file has been configured.
	toolchain := p.ctx.Platform().GetToolchain()
	stripBin := toolchain.GetSTRIP()
	if stripBin == "" {
		return fmt.Errorf("strip executable file path is not configured in platform: %s.toml", p.ctx.Platform().GetName())
	}

	// Resolve target tree: installed/<platform>/<project>/<buildType>/.
	// Dev/host trees are not stripped — those binaries run on the build host
	// and people often want their symbols for debugging build issues.
	installedDir := p.ctx.InstalledDir()
	if !fileio.PathExists(installedDir) {
		return nil
	}

	color.Printf(color.Title, "\n[strip deployed binaries: %s]\n", installedDir)
	err := filepath.WalkDir(installedDir, func(path string, d fs.DirEntry, walkErr error) error {
		if walkErr != nil {
			return walkErr
		}
		if d.IsDir() {
			return nil
		}

		// Skip anything that is obviously not an ELF binary we want to strip.
		// Static archives are special: stripping breaks them for downstream linking.
		if strings.HasSuffix(path, ".a") {
			return nil
		}
		if !fileio.IsELFFile(path) {
			return nil
		}

		if _, err := cmd.NewExecutor("", stripBin, path).ExecuteOutput(); err != nil {
			return nil
		}
		color.PrintHint("✔ strip %s", path)
		return nil
	})
	if err != nil {
		return err
	}

	return nil
}
