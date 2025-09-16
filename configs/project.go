package configs

import (
	"celer/pkgs/dirs"
	"celer/pkgs/expr"
	"celer/pkgs/fileio"
	"fmt"
	"os"
	"path/filepath"
	"strings"

	"github.com/BurntSushi/toml"
)

type Project struct {
	BuildType      string   `toml:"build_type"`
	Ports          []string `toml:"ports"`
	Vars           []string `toml:"vars"`
	Envs           []string `toml:"envs"`
	Micros         []string `toml:"micros"`
	CompileOptions []string `toml:"compile_options"`
	OptLevel       optLevel `toml:"opt_level"`

	// Internal fields.
	Name string `toml:"-"`
	ctx  Context
}

type optLevel struct {
	Debug          string `toml:"debug"`
	Release        string `toml:"release"`
	RelWithDebInfo string `toml:"relwithdebinfo"`
	MinSizeRel     string `toml:"minsizerel"`
}

func (p *Project) Init(ctx Context, projectName string) error {
	p.ctx = ctx

	// Check if project name is empty.
	projectName = strings.TrimSpace(projectName)
	if projectName == "" {
		return fmt.Errorf("project name is empty")
	}

	// Check if project file exists.
	projectPath := filepath.Join(dirs.ConfProjectsDir, projectName+".toml")
	if !fileio.PathExists(projectPath) {
		return fmt.Errorf("project %s does not exists", projectName)
	}

	// Read conf/projects/<project_name>.toml.
	bytes, err := os.ReadFile(projectPath)
	if err != nil {
		return err
	}
	if err := toml.Unmarshal(bytes, p); err != nil {
		return fmt.Errorf("read error: %w", err)
	}

	// Default build_type.
	if p.BuildType == "" {
		p.BuildType = "Release"
	}

	// Assign default opt level.
	p.OptLevel.Debug = expr.If(p.OptLevel.Debug != "", p.OptLevel.Debug, "-g")
	p.OptLevel.Release = expr.If(p.OptLevel.Release != "", p.OptLevel.Release, "-O3")
	p.OptLevel.RelWithDebInfo = expr.If(p.OptLevel.RelWithDebInfo != "", p.OptLevel.RelWithDebInfo, "-O2 -g")
	p.OptLevel.MinSizeRel = expr.If(p.OptLevel.MinSizeRel != "", p.OptLevel.MinSizeRel, "-Os")

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
	if len(p.Micros) == 0 {
		p.Micros = []string{}
	}

	// Default opt level values.
	p.OptLevel = optLevel{
		Debug:          "-g",
		Release:        "-O3",
		RelWithDebInfo: "-O2 -g",
		MinSizeRel:     "-Os",
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

func (p Project) Deploy() error {
	for _, nameVersion := range p.Ports {
		var port Port
		if err := port.Init(p.ctx, nameVersion, p.BuildType); err != nil {
			return fmt.Errorf("%s: %w", nameVersion, err)
		}
		if _, err := port.Install(); err != nil {
			return fmt.Errorf("%s: %w", nameVersion, err)
		}
	}
	return nil
}
