package configs

import (
	"fmt"
	"os"
	"path/filepath"
	"runtime"
	"strings"

	"github.com/celer-pkg/celer/buildsystems"
	"github.com/celer-pkg/celer/context"
	"github.com/celer-pkg/celer/pkgs/dirs"
	"github.com/celer-pkg/celer/pkgs/errors"
	"github.com/celer-pkg/celer/pkgs/fileio"

	"github.com/BurntSushi/toml"
)

var (
	// preparedTmpDeps tracks deps already prepared for tmp, to avoid redundant Init().
	preparedTmpDeps = map[string]bool{}

	// visitedPorts tracks ports visited during dependency-tree traversal so
	// each port is processed at most once even when it appears under many parents.
	visitedPorts = map[string]bool{}

	// clonedPorts tracks which ports have already been cloned during a single
	// cloneAllRepos invocation, to avoid redundant Init() + Clone() calls.
	clonedPorts = map[string]bool{}
)

type InstallOptions struct {
	Force     bool
	Recursive bool
}

type RemoveOptions struct {
	Purge      bool
	Recursive  bool
	BuildCache bool
}

type Package struct {
	Url             string `toml:"url"`
	Ref             string `toml:"ref"`
	Checksum        string `toml:"checksum,omitempty"`
	Depth           int    `toml:"depth,omitempty,omitzero"`
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

	ctx                        context.Context
	portFile                   string
	traceFile                  string
	metaFile                   string
	tmpDepsDir                 string
	installReport              *installReport
	exprVars                   context.ExprVars
	sourceModified             bool
	pkgCacheStoreSkippedReason string
}

func (p Port) NameVersion() string {
	return p.Name + "@" + p.Version
}

// visitedKey is the key used in visitedPorts to dedupe per-command processing.
func (p Port) visitedKey() string {
	if p.DevDep || p.HostDep {
		return p.NameVersion() + " [dev]"
	}
	return p.NameVersion()
}

func (p *Port) Init(ctx context.Context, nameVersion string) error {
	p.ctx = ctx
	p.exprVars = ctx.ExprVars().Clone()

	// Validate name and version.
	nameVersion = strings.ReplaceAll(nameVersion, "`", "")
	parts := strings.Split(nameVersion, "@")
	if len(parts) != 2 {
		return fmt.Errorf("port name and version are invalid %s", nameVersion)
	}

	// Parse name and version.
	p.Name = parts[0]
	p.Version = parts[1]

	// Choose the right port file.
	portFile, err := p.resolveProjectPort(ctx.Project().GetName(), parts[0], parts[1])
	if err != nil {
		if errors.Is(err, errors.ErrPortNotFound) {
			if p.Parent != "" {
				return fmt.Errorf("%w for %s in %s", errors.ErrPortNotFound, nameVersion, p.Parent)
			}
			return fmt.Errorf("%w for %s", errors.ErrPortNotFound, nameVersion)
		}
		return err
	}
	p.portFile = portFile

	// Decode TOML.
	bytes, err := os.ReadFile(p.portFile)
	if err != nil {
		return fmt.Errorf("failed to read %s -> %w", p.portFile, err)
	}
	if err := toml.Unmarshal(bytes, p); err != nil {
		return fmt.Errorf("failed to unmarshal %s -> %w", p.portFile, err)
	}

	// Build_tool ports are always built by native toolchain.
	if p.Package.BuildTool {
		p.DevDep = true
	}

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
		// For build_tool packages not supported on current platform, create a nobuild config.
		if p.Package.BuildTool && !p.IsHostSupported() {
			nobuildConfig := buildsystems.BuildConfig{
				BuildSystem: "nobuild",
			}
			p.BuildConfigs = append(p.BuildConfigs, nobuildConfig)
			p.MatchedConfig = &p.BuildConfigs[len(p.BuildConfigs)-1]
		} else {
			return fmt.Errorf("%w for %s", errors.ErrNoMatchedConfigFound, p.NameVersion())
		}
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
		return fmt.Errorf("failed to validate %s -> %w", p.portFile, err)
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

	// nobuild is already regard as installed.
	if p.MatchedConfig.BuildSystem == "nobuild" {
		return true, nil
	}

	// Check if meta file exists.
	if !fileio.PathExists(p.metaFile) {
		return false, nil
	}

	// Check if meta data matches.
	metaBytes, err := os.ReadFile(p.metaFile)
	if err != nil {
		return false, err
	}
	newMeta, err := p.buildMeta()
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
		options := RemoveOptions{
			Purge:      true,
			Recursive:  false,
			BuildCache: false,
		}
		if err := p.Remove(options); err != nil {
			return false, fmt.Errorf("failed to remove installed package -> %w", err)
		}
		return false, nil
	}

	// Verify that all dependencies are also installed.
	// If a dependency was removed the parent's trace/meta still exist but the artifact is gone.
	depsInstalled, err := p.checkDepsInstalled()
	if err != nil {
		return false, fmt.Errorf("failed to check depts installed for %s -> %w", p.NameVersion(), err)
	}

	return depsInstalled, nil
}

// resolveProjectPort returns the port.toml path for (name@version), searching
// in priority order:
//
//  1. <ConfProjectsDir>/<project>/<name>/<version>/port.toml         (project top-level)
//  2. <ConfProjectsDir>/<project>/ports/<name>/<version>/port.toml   (project vendor)
//  3. <PortsDir>/<first-char>/<name>/<version>/port.toml             (global)
//
// Returns the found path, or:
//   - ErrAmbiguousProjectPort if both (1) and (2) exist
//   - ErrPortNotFound if none of the three exists
func (p Port) resolveProjectPort(project, name, version string) (string, error) {
	topLevelPort := filepath.Join(dirs.ConfProjectsDir, project, name, version, "port.toml")
	vendorPort := filepath.Join(dirs.ConfProjectsDir, project, "ports", name, version, "port.toml")

	hasTopLevelPort := fileio.PathExists(topLevelPort)
	hasVendorPort := fileio.PathExists(vendorPort)

	switch {
	case hasTopLevelPort && hasVendorPort:
		return "", fmt.Errorf("%w in booth '%s' and '%s' for '%s' — remove one to disambiguate",
			errors.ErrAmbiguousPortFound, project+"/", project+"/"+"ports/", name+"@"+version)

	case hasTopLevelPort:
		return topLevelPort, nil

	case hasVendorPort:
		return vendorPort, nil
	}

	// Fall back to the global ports/ collection.
	publicPort := dirs.GetPortPath(name, version)
	if fileio.PathExists(publicPort) {
		return publicPort, nil
	}

	return "", errors.ErrPortNotFound
}

func (p Port) checkDepsInstalled() (bool, error) {
	for _, nameVersion := range p.MatchedConfig.Dependencies {
		var port = Port{
			DevDep:  p.DevDep,
			HostDep: p.HostDep,
		}
		if err := port.Init(p.ctx, nameVersion); err != nil {
			return false, err
		}
		if installed, _ := port.Installed(); !installed {
			return false, nil
		}
	}
	for _, nameVersion := range p.MatchedConfig.DevDependencies {
		if (p.DevDep || p.HostDep) && p.NameVersion() == nameVersion {
			continue
		}

		var port = Port{
			DevDep:  true,
			HostDep: p.HostDep,
		}
		if err := port.Init(p.ctx, nameVersion); err != nil {
			return false, err
		}
		if installed, _ := port.Installed(); !installed {
			return false, nil
		}
	}

	return true, nil
}

func (p Port) Write(portPath string) error {
	p.Package.Url = ""
	p.Name = ""
	p.Package.Ref = ""
	p.Package.Checksum = ""
	p.Package.SrcDir = ""
	p.BuildConfigs = []buildsystems.BuildConfig{}
	p.BuildConfigs = append(p.BuildConfigs, buildsystems.BuildConfig{
		SystemName:      "",
		SystemNames:     []string{},
		SystemProcessor: "",
		BuildSystem:     "",
		BuildTools:      []string{},
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

	// If build type is not specified in port.toml, then set it to the build type defined in celer.toml.
	if p.BuildConfigs[index].BuildType == "" {
		p.BuildConfigs[index].BuildType = buildType
	}

	// Placeholder variables.
	p.putExprVars(p.BuildConfigs[index])
	p.BuildConfigs[index].ExprVars = p.exprVars
	return &p.BuildConfigs[index], nil
}

func (p *Port) putExprVars(config buildsystems.BuildConfig) {
	p.exprVars = p.ctx.ExprVars().Clone()
	p.exprVars.Put("REPO_DIR", config.PortConfig.RepoDir)
	p.exprVars.Put("SRC_DIR", config.PortConfig.SrcDir)
	p.exprVars.Put("BUILD_DIR", config.PortConfig.BuildDir)
	p.exprVars.Put("PACKAGE_DIR", config.PortConfig.PackageDir)
	p.exprVars.Put("DEPS_DEV_DIR", filepath.Join(dirs.TmpDepsDir, config.PortConfig.HostName+"-dev"))

	if config.DevDep {
		p.exprVars.Put("DEPS_DIR", filepath.Join(dirs.TmpDepsDir, config.PortConfig.HostName+"-dev"))
	} else {
		p.exprVars.Put("DEPS_DIR", filepath.Join(dirs.TmpDepsDir, config.PortConfig.LibraryDir))
	}
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
			file := filepath.Join(p.ctx.Platform().GetHostName()+"-dev", relativePath)
			files = append(files, file)
		} else {
			libraryDir := filepath.Join(platformName, projectName, p.ctx.BuildType())
			file := filepath.Join(libraryDir, relativePath)
			files = append(files, file)
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
	// Merge SystemNames and SystemName into a single list.
	var targetSystemNames []string
	if len(config.SystemNames) > 0 {
		targetSystemNames = config.SystemNames
	} else if config.SystemName != "" {
		targetSystemNames = []string{config.SystemName}
	}

	// Trim whitespace from system names and processor.
	targetSystemProcessor := strings.ToLower(strings.TrimSpace(config.SystemProcessor))
	currentSystemName := strings.ToLower(p.currentSystemName())
	currentProcessor := strings.ToLower(p.currentSystemProcessor())

	// Compare system processor if specified.
	if targetSystemProcessor != "" && targetSystemProcessor != currentProcessor {
		return false
	}

	// No target system name specified indicates that
	// system name is not a factor for matching, so consider it a match.
	if len(targetSystemNames) == 0 {
		return true
	} else {
		// Compare system names if specified.
		for _, targetSystemName := range targetSystemNames {
			targetSystemName = strings.ToLower(strings.TrimSpace(targetSystemName))
			if targetSystemName != "" && targetSystemName == currentSystemName {
				return true
			}
		}
		return false
	}
}

func (p Port) currentSystemName() string {
	if p.DevDep || p.HostDep {
		// Host-side tools/dev dependencies must match the native machine,
		// not the target toolchain. Otherwise ports like ICU may select the
		// target config (for example aarch64) while building an x86_64 host tool.
		hostName := strings.TrimSpace(p.ctx.Platform().GetHostName())
		if _, systemName, ok := strings.Cut(hostName, "-"); ok && systemName != "" {
			return systemName
		}
		return runtime.GOOS
	}

	toolchain := p.ctx.Platform().GetToolchain()
	if toolchain != nil && strings.TrimSpace(toolchain.GetSystemName()) != "" {
		return toolchain.GetSystemName()
	}
	return runtime.GOOS
}

func (p Port) currentSystemProcessor() string {
	if p.DevDep || p.HostDep {
		// Host-side tools/dev dependencies must use the host architecture for
		// build_config matching so we pick the native x86_64 config instead of
		// the target architecture's config.
		hostName := strings.TrimSpace(p.ctx.Platform().GetHostName())
		if systemProcessor, _, ok := strings.Cut(hostName, "-"); ok && systemProcessor != "" {
			return systemProcessor
		}

		switch runtime.GOARCH {
		case "amd64":
			return "x86_64"
		case "arm64":
			return "aarch64"
		default:
			return runtime.GOARCH
		}
	}

	toolchain := p.ctx.Platform().GetToolchain()
	if toolchain != nil && strings.TrimSpace(toolchain.GetSystemProcessor()) != "" {
		return toolchain.GetSystemProcessor()
	}
	return runtime.GOARCH
}
