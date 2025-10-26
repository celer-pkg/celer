package configs

import (
	"celer/buildsystems"
	"celer/context"
	"celer/pkgs/color"
	"celer/pkgs/dirs"
	"celer/pkgs/expr"
	"celer/pkgs/fileio"
	"fmt"
	"os"
	"path/filepath"
	"runtime"
	"slices"
	"strings"

	"github.com/BurntSushi/toml"
)

// preparedTmpDeps is used to store prepared deps.
var preparedTmpDeps []string

type InstallOptions struct {
	// Reinstall options.
	Force   bool
	Recurse bool

	// Cache options.
	StoreCache bool
	CacheToken string
}

type RemoveOptions struct {
	Purge      bool
	Recurse    bool
	BuildCache bool
}

type Package struct {
	Url             string   `toml:"url"`
	Ref             string   `toml:"ref"`
	Commit          string   `toml:"commit,omitempty"`
	Archive         string   `toml:"archive,omitempty"`
	SrcDir          string   `toml:"src_dir,omitempty"`
	SupportedHosts  []string `toml:"supported_hosts,omitempty"`
	IgnoreSubmodule bool     `toml:"ignore_submodule,omitempty"`
}

type Port struct {
	Package      Package                    `toml:"package"`
	BuildConfigs []buildsystems.BuildConfig `toml:"build_configs"`

	// Internal fields.
	Name          string                    `toml:"-"`
	Version       string                    `toml:"-"`
	Parent        string                    `toml:"-"`
	DevDep        bool                      `toml:"-"`
	Native        bool                      `toml:"-"`
	MatchedConfig *buildsystems.BuildConfig `toml:"-"`
	PackageDir    string                    `toml:"-"`
	InstalledDir  string                    `toml:"-"`

	ctx        context.Context
	traceFile  string
	metaFile   string
	tmpDepsDir string
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
	portInPorts := filepath.Join(dirs.PortsDir, parts[0], parts[1], "port.toml")
	if !fileio.PathExists(portInProject) && !fileio.PathExists(portInPorts) {
		if p.Parent != "" {
			return fmt.Errorf("%s defined in %s is not exists", nameVersion, p.Parent)
		} else {
			return fmt.Errorf("port %s does not exists", nameVersion)
		}
	}

	// Decode TOML.
	portPath := expr.If(!fileio.PathExists(portInPorts), portInProject, portInPorts)
	bytes, err := os.ReadFile(portPath)
	if err != nil {
		return fmt.Errorf("failed to read %s.\n %w", portPath, err)
	}
	if err := toml.Unmarshal(bytes, p); err != nil {
		return fmt.Errorf("failed to unmarshal %s.\n %w", portPath, err)
	}

	// Set matchedConfig as prebuilt config when no config found in toml.
	p.MatchedConfig = p.findMatchedConfig(p.ctx.BuildType())
	if p.MatchedConfig == nil {
		return fmt.Errorf("no matched config found for %s", p.NameVersion())
	} else if p.MatchedConfig.BuildSystem == "prebuilt" &&
		p.MatchedConfig.Url != "" {
		p.Package.Url = p.MatchedConfig.Url
	}

	// Init build config.
	if err := p.initBuildConfig(nameVersion); err != nil {
		return fmt.Errorf("failed to init build config.\n %w", err)
	}

	// Validate port.
	if err := p.validate(); err != nil {
		return fmt.Errorf("failed to validate %s.\n %w", portPath, err)
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

	// No meta file means not installed.
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
			return false, err
		}

		// Remove installed package if build config changed.
		localMeta := string(metaBytes)
		if localMeta != newMeta {
			color.Printf(color.Yellow, "\n================ The outdated package of %s will be removed in next install. ================", p.NameVersion())
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
		Pattern:         "",
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

func (p *Port) findMatchedConfig(buildType string) *buildsystems.BuildConfig {
	for index, config := range p.BuildConfigs {
		if p.checkPatternMatch(config.Pattern) {
			p.BuildConfigs[index].BuildType = buildType

			// If LibraryType is empty, set it to `shared`.
			if strings.TrimSpace(config.LibraryType) == "" {
				p.BuildConfigs[index].LibraryType = "shared"
			}

			return &p.BuildConfigs[index]
		} else {
			if config.BuildSystem == "prebuilt" || config.BuildSystem == "nobuild" {
				return &config
			}
		}
	}

	return nil
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

		if p.DevDep {
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
	if len(p.Package.SupportedHosts) == 0 {
		return true
	}

	return slices.Contains(p.Package.SupportedHosts, runtime.GOOS)
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
		if !p.checkPatternMatch(config.Pattern) {
			continue
		}

		if err := config.Validate(); err != nil {
			return err
		}
	}

	return nil
}

func (p Port) checkPatternMatch(pattern string) bool {
	pattern = strings.TrimSpace(pattern)
	if pattern == "" || pattern == "*" {
		return true
	}

	// For dev mode, we change platformName to x86_64-windows-dev, x86_64-macos-dev, x86_64-linux-dev,
	// then we can match the most like pattern.
	platformName := p.ctx.Platform().GetName()
	if p.DevDep {
		platformName = p.ctx.Platform().GetHostName() + "-dev"
	} else if platformName == "" { // If empty, set as host system name.
		platformName = "*" + runtime.GOOS + "*"
	}

	if pattern[0] == '*' && pattern[len(pattern)-1] == '*' {
		return strings.Contains(platformName, pattern[1:len(pattern)-1])
	}

	if pattern[0] == '*' {
		return strings.HasSuffix(platformName, pattern[1:])
	}

	if pattern[len(pattern)-1] == '*' {
		return strings.HasPrefix(platformName, pattern[:len(pattern)-1])
	}

	return platformName == pattern
}

func (p Port) toolchain() *buildsystems.Toolchain {
	toolchain := p.ctx.Platform().GetToolchain()
	if toolchain == nil {
		return nil
	}

	var bsToolchain buildsystems.Toolchain
	nativeGcc := toolchain.GetName() == "gcc" && toolchain.GetPath() == "/usr/bin"
	visualStudioToolchain := strings.Contains(toolchain.GetFullPath(), "Microsoft Visual Studio")
	bsToolchain.Native = p.Native || visualStudioToolchain || nativeGcc
	bsToolchain.Name = toolchain.GetName()
	bsToolchain.Version = toolchain.GetVersion()
	bsToolchain.Host = toolchain.GetHost()
	bsToolchain.SystemName = toolchain.GetSystemName()
	bsToolchain.SystemProcessor = toolchain.GetSystemProcessor()
	bsToolchain.CrosstoolPrefix = toolchain.GetCrosstoolPrefix()
	bsToolchain.CStandard = toolchain.GetCStandard()
	bsToolchain.CXXStandard = toolchain.GetCXXStandard()

	rootfs := p.ctx.RootFS()
	if rootfs != nil {
		bsToolchain.RootFS = rootfs.GetFullPath()
		bsToolchain.PkgConfigPath = rootfs.GetPkgConfigPath()
		bsToolchain.IncludeDirs = rootfs.GetIncludeDirs()
		bsToolchain.LibDirs = rootfs.GetLibDirs()
	}

	bsToolchain.Fullpath = toolchain.GetFullPath()
	bsToolchain.CC = toolchain.GetCC()
	bsToolchain.CXX = toolchain.GetCXX()
	bsToolchain.LD = toolchain.GetLD()
	bsToolchain.AS = toolchain.GetAS()
	bsToolchain.FC = toolchain.GetFC()
	bsToolchain.AR = toolchain.GetAR()
	bsToolchain.RANLIB = toolchain.GetRANLIB()
	bsToolchain.NM = toolchain.GetNM()
	bsToolchain.OBJCOPY = toolchain.GetOBJCOPY()
	bsToolchain.OBJDUMP = toolchain.GetOBJDUMP()
	bsToolchain.STRIP = toolchain.GetSTRIP()
	bsToolchain.READELF = toolchain.GetREADELF()

	// For windows MSVC.
	bsToolchain.MSVC.VCVars = toolchain.GetMSVC().VCVars
	bsToolchain.MSVC.MT = toolchain.GetMSVC().MT
	bsToolchain.MSVC.RC = toolchain.GetMSVC().RC

	return &bsToolchain
}
