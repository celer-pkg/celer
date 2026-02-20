package configs

import (
	"celer/buildsystems"
	"celer/context"
	"celer/pkgs/dirs"
	"celer/pkgs/errors"
	"celer/pkgs/expr"
	"celer/pkgs/fileio"
	"fmt"
	"os"
	"path/filepath"
	"runtime"
	"strings"

	"github.com/BurntSushi/toml"
)

// preparedTmpDeps is used to store prepared deps.
var preparedTmpDeps []string

type InstallOptions struct {
	// Reinstall options.
	Force     bool
	Recursive bool

	// Cache options.
	StoreCache bool
	CacheToken string
}

type RemoveOptions struct {
	Purge      bool
	Recursive  bool
	BuildCache bool
}

type Package struct {
	Url             string `toml:"url"`
	Ref             string `toml:"ref"`
	Commit          string `toml:"commit,omitempty"`
	Depth           int    `toml:"depth,omitempty"`
	Archive         string `toml:"archive,omitempty"`
	SrcDir          string `toml:"src_dir,omitempty"`
	IgnoreSubmodule bool   `toml:"ignore_submodule,omitempty"`
	BuildTool       bool   `toml:"build_tool,omitempty"`
}

type Port struct {
	Package      Package                    `toml:"package"`
	BuildConfigs []buildsystems.BuildConfig `toml:"build_configs"`

	// Internal fields.
	Name          string                    `toml:"-"`
	Version       string                    `toml:"-"`
	Parent        string                    `toml:"-"`
	DevDep        bool                      `toml:"-"` // Whether the port is a dev_dependences.
	HostDep       bool                      `toml:"-"` // Whether the port is a dependencies of a dev_dependencies.
	MatchedConfig *buildsystems.BuildConfig `toml:"-"`
	PackageDir    string                    `toml:"-"`
	InstalledDir  string                    `toml:"-"`

	ctx           context.Context
	traceFile     string
	metaFile      string
	tmpDepsDir    string
	installReport *installReport
}

func (p Port) NameVersion() string {
	return p.Name + "@" + p.Version
}

func (p *Port) Init(ctx context.Context, nameVersion string) error {
	p.ctx = ctx

	// Validate name and version.
	nameVersion = strings.ReplaceAll(nameVersion, "`", "")
	parts := strings.Split(nameVersion, "@")
	if len(parts) != 2 {
		return fmt.Errorf("port name and version are invalid %s", nameVersion)
	}

	// Parse name and version.
	p.Name = parts[0]
	p.Version = parts[1]

	// Read name and version.
	portInProject := filepath.Join(dirs.ConfProjectsDir, ctx.Project().GetName(), parts[0], parts[1], "port.toml")
	portInPorts := dirs.GetPortPath(parts[0], parts[1])
	if !fileio.PathExists(portInProject) && !fileio.PathExists(portInPorts) {
		if p.Parent != "" {
			return fmt.Errorf("%w for %s in %s", errors.ErrPortNotFound, nameVersion, p.Parent)
		} else {
			return fmt.Errorf("%w for %s", errors.ErrPortNotFound, nameVersion)
		}
	}

	// Decode TOML.
	portPath := expr.If(!fileio.PathExists(portInPorts), portInProject, portInPorts)
	bytes, err := os.ReadFile(portPath)
	if err != nil {
		return fmt.Errorf("failed to read %s -> %w", portPath, err)
	}
	if err := toml.Unmarshal(bytes, p); err != nil {
		return fmt.Errorf("failed to unmarshal %s -> %w", portPath, err)
	}

	// Propagate build_tool flag from package to port.
	p.HostDep = p.HostDep || p.Package.BuildTool

	// Convert build type to lowercase for all build configs.
	for index := range p.BuildConfigs {
		p.BuildConfigs[index].BuildType = strings.ToLower(p.BuildConfigs[index].BuildType)
		p.BuildConfigs[index].BuildType_Windows = strings.ToLower(p.BuildConfigs[index].BuildType_Windows)
		p.BuildConfigs[index].BuildType_Linux = strings.ToLower(p.BuildConfigs[index].BuildType_Linux)
		p.BuildConfigs[index].BuildType_Darwin = strings.ToLower(p.BuildConfigs[index].BuildType_Darwin)
	}

	// Set matchedConfig as prebuilt config when no config found in toml.
	matchedConfig, err := p.findMatchedConfig(p.ctx.BuildType())
	if err != nil {
		return err
	}
	p.MatchedConfig = matchedConfig
	if p.MatchedConfig == nil {
		return fmt.Errorf("%w for %s", errors.ErrNoMatchedConfigFound, p.NameVersion())
	}
	if p.MatchedConfig.BuildSystem == "prebuilt" && p.MatchedConfig.Url != "" {
		p.Package.Url = p.MatchedConfig.Url
	}

	// Init build config.
	if err := p.initBuildConfig(nameVersion); err != nil {
		return fmt.Errorf("failed to init build config -> %w", err)
	}

	// Validate port.
	if err := p.validate(); err != nil {
		return fmt.Errorf("failed to validate %s -> %w", portPath, err)
	}

	return nil
}

func (p Port) Installed() (bool, error) {
	// Packages like autoconf, m4, automake, libtool cannot be build in windows.
	if !p.IsHostSupported() {
		return true, nil
	}

	// No trace file means not installed.
	if !fileio.PathExists(p.traceFile) {
		return false, nil
	}

	// For buildsystem other than nobuild, check if meta file outdated.
	if p.MatchedConfig.BuildSystem != "nobuild" {
		// Check if meta file exists.
		if !fileio.PathExists(p.metaFile) {
			return false, nil
		}

		// Check if build desc matches.
		metaBytes, err := os.ReadFile(p.metaFile)
		if err != nil {
			return false, err
		}
		newMeta, err := p.buildMeta(p.Package.Commit)
		if err != nil {
			// Repo not exist is not error.
			if errors.Is(err, errors.ErrRepoNotExit) {
				return false, nil
			}
			return false, err
		}

		// Remove installed package if build config changed.
		localMeta := string(metaBytes)
		if localMeta != newMeta {
			return false, nil
		}
	}

	return true, nil
}

func (p Port) Write(portPath string) error {
	p.Package.Url = ""
	p.Name = ""
	p.Package.Ref = ""
	p.Package.SrcDir = ""
	p.BuildConfigs = []buildsystems.BuildConfig{}
	p.BuildConfigs = append(p.BuildConfigs, buildsystems.BuildConfig{
		SystemName:      "",
		SystemProcessor: "",
		BuildSystem:     "",
		BuildTools:      []string{},
		LibraryType:     "",
		CStandard:       "",
		CXXStandard:     "",
		Envs:            []string{},
		Patches:         []string{},
		AutogenOptions:  []string{},
		Options:         []string{},
		Dependencies:    []string{},
		DevDependencies: []string{},
		PreConfigure:    []string{},
		PostConfigure:   []string{},
		PreBuild:        []string{},
		FixBuild:        []string{},
		PostBuild:       []string{},
		PreInstall:      []string{},
		PostInstall:     []string{},
	})
	bytes, err := toml.Marshal(p)
	if err != nil {
		return err
	}

	// Check if tool exists.
	if fileio.PathExists(portPath) {
		return fmt.Errorf("%s is already exists", portPath)
	}

	// Make sure the parent directory exists.
	parentDir := filepath.Dir(portPath)
	if err := os.MkdirAll(parentDir, os.ModePerm); err != nil {
		return err
	}
	return os.WriteFile(portPath, bytes, os.ModePerm)
}

func (p *Port) findMatchedConfig(buildType string) (*buildsystems.BuildConfig, error) {
	matchedIndexes := make([]int, 0, len(p.BuildConfigs))

	for index, config := range p.BuildConfigs {
		if p.matchBuildConfig(config) {
			matchedIndexes = append(matchedIndexes, index)
		}
	}

	if len(matchedIndexes) == 0 {
		return nil, nil
	}
	if len(matchedIndexes) > 1 {
		return nil, fmt.Errorf(
			"port %s has %d build_configs matching current platform (%s/%s), please keep only one",
			p.NameVersion(),
			len(matchedIndexes),
			p.currentSystemName(),
			p.currentSystemProcessor(),
		)
	}

	index := matchedIndexes[0]
	config := p.BuildConfigs[index]

	// If build type is not specified in port.toml, then set it to the build type defined in celer.toml.
	if p.BuildConfigs[index].BuildType == "" {
		p.BuildConfigs[index].BuildType = buildType
	}

	// If LibraryType is empty, set it to `shared`.
	if strings.TrimSpace(config.LibraryType) == "" {
		p.BuildConfigs[index].LibraryType = "shared"
	}

	// Placeholder variables.
	p.BuildConfigs[index].ExpressVars.Init(p.ctx.Vairables(), p.BuildConfigs[index])
	return &p.BuildConfigs[index], nil
}

func (p Port) PackageFiles(packageDir, platformName, projectName string) ([]string, error) {
	if !fileio.PathExists(packageDir) {
		return nil, nil
	}

	var files []string
	if err := filepath.WalkDir(packageDir, func(path string, d os.DirEntry, err error) error {
		if err != nil {
			return err
		}

		if d.IsDir() {
			return nil
		}

		relativePath, err := filepath.Rel(packageDir, path)
		if err != nil {
			return err
		}

		if p.DevDep || p.HostDep {
			files = append(files, filepath.Join(p.ctx.Platform().GetHostName()+"-dev", relativePath))
		} else {
			platformProject := fmt.Sprintf("%s@%s@%s", platformName, projectName, p.ctx.BuildType())
			files = append(files, filepath.Join(platformProject, relativePath))
		}
		return nil
	}); err != nil {
		return nil, err
	}

	return files, nil
}

func (p Port) IsHostSupported() bool {
	// Only build_tool ports (like m4, automake, libtool, autoconf) are restricted to Linux/Darwin.
	if !p.Package.BuildTool {
		return true
	}

	return runtime.GOOS == "linux" || runtime.GOOS == "darwin"
}

func (p Port) validate() error {
	if p.Package.Url == "" {
		return fmt.Errorf("url of %s is empty", p.Name)
	}

	if p.Name == "" {
		return fmt.Errorf("name of %s is empty", p.Name)
	}

	if p.Package.Ref == "" {
		return fmt.Errorf("version of %s is empty", p.Name)
	}

	for _, config := range p.BuildConfigs {
		if err := config.Validate(); err != nil {
			return err
		}
	}

	return nil
}

func (p Port) matchBuildConfig(config buildsystems.BuildConfig) bool {
	systemName := strings.ToLower(strings.TrimSpace(config.SystemName))
	systemProcessor := strings.ToLower(strings.TrimSpace(config.SystemProcessor))

	currentName := strings.ToLower(p.currentSystemName())
	currentProcessor := strings.ToLower(p.currentSystemProcessor())

	// No system constraints means this config is global (all platforms).
	if systemName == "" && systemProcessor == "" {
		return true
	}
	if systemName != "" && systemName != currentName {
		return false
	}
	if systemProcessor != "" && systemProcessor != currentProcessor {
		return false
	}
	return true
}

func (p Port) currentSystemName() string {
	toolchain := p.ctx.Platform().GetToolchain()
	if toolchain != nil && strings.TrimSpace(toolchain.GetSystemName()) != "" {
		return toolchain.GetSystemName()
	}
	return runtime.GOOS
}

func (p Port) currentSystemProcessor() string {
	toolchain := p.ctx.Platform().GetToolchain()
	if toolchain != nil && strings.TrimSpace(toolchain.GetSystemProcessor()) != "" {
		return toolchain.GetSystemProcessor()
	}
	return runtime.GOARCH
}
