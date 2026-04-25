package configs

import (
	"fmt"
	"os"
	"path/filepath"
	"strings"

	"github.com/celer-pkg/celer/context"
	"github.com/celer-pkg/celer/pkgs/dirs"
	"github.com/celer-pkg/celer/pkgs/fileio"

	"github.com/BurntSushi/toml"
)

type Project struct {
	TargetPlatform string   `toml:"target_platform,omitempty"`
	BuildType      string   `toml:"build_type"`
	PythonVersion  string   `toml:"python_version,omitempty"`
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
		return fmt.Errorf("project does not exist: %s", projectName)
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

func (p Project) Write(platformPath string) error {
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
	if fileio.PathExists(platformPath) {
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

func (p Project) GetPythonVersion() string {
	return p.PythonVersion
}

func (p Project) deploy(force bool) error {
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
	return nil
}
