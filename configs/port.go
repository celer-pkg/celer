package configs

import (
	"celer/buildsystems"
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

type Package struct {
	Url            string   `toml:"url"`
	Ref            string   `toml:"ref"`
	Commit         string   `toml:"commit,omitempty"`
	Archive        string   `toml:"archive,omitempty"`
	SrcDir         string   `toml:"src_dir,omitempty"`
	SupportedHosts []string `toml:"supported_hosts,omitempty"`
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

	ctx          Context
	buildType    string
	packageDir   string
	installedDir string
	infoFile     string
	hashFile     string
	tmpDepsDir   string
}

func (p Port) NameVersion() string {
	return p.Name + "@" + p.Version
}

func (p *Port) Init(ctx Context, nameVersion, buildType string) error {
	p.ctx = ctx
	p.buildType = strings.ToLower(buildType)

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
	portInProject := filepath.Join(dirs.ConfProjectsDir, ctx.Project().Name, parts[0], parts[1], "port.toml")
	portInPorts := filepath.Join(dirs.PortsDir, parts[0], parts[1], "port.toml")
	if !fileio.PathExists(portInProject) && !fileio.PathExists(portInPorts) {
		if p.Parent != "" {
			return fmt.Errorf("%s defined in %s is not exists", nameVersion, p.Parent)
		} else {
			return fmt.Errorf("port %s does not exists", nameVersion)
		}
	}

	// Decode TOML.
	bytes, err := os.ReadFile(expr.If(!fileio.PathExists(portInPorts), portInProject, portInPorts))
	if err != nil {
		return fmt.Errorf("read port error: %w", err)
	}
	if err := toml.Unmarshal(bytes, p); err != nil {
		return fmt.Errorf("unmarshal port error: %w", err)
	}

	// Init build config.
	if err := p.initBuildConfig(nameVersion); err != nil {
		return fmt.Errorf("init build config error: %w", err)
	}

	p.MatchedConfig = p.findMatchedConfig(p.buildType)

	// Set matchedConfig as prebuilt config when no config found in toml.
	if p.MatchedConfig == nil {
		return fmt.Errorf("no matched config found for %s", p.NameVersion())
	} else if p.MatchedConfig.BuildSystem == "prebuilt" &&
		p.MatchedConfig.Url != "" {
		p.Package.Url = p.MatchedConfig.Url
	}

	// Validate port.
	if err := p.validate(); err != nil {
		return fmt.Errorf("validate port error: %w", err)
	}

	return nil
}

func (p Port) Installed() (bool, error) {
	// Packages like autoconf, m4, automake, libtool cannot be build in windows.
	if !p.IsHostSupported() {
		return true, nil
	}

	// No info no hash means not installed.
	if !fileio.PathExists(p.infoFile) || !fileio.PathExists(p.hashFile) {
		return false, nil
	}

	// Check if build desc matches.
	descBytes, err := os.ReadFile(p.hashFile)
	if err != nil {
		return false, err
	}
	newMeta, err := p.buildMeta(p.Package.Commit)
	if err != nil {
		return false, err
	}

	// Remove installed package if build config changed.
	localMeta := string(descBytes)
	if localMeta != newMeta {
		color.Printf(color.Green, "\n================ package will be removed, because build desc not match for %s: ================\n", p.NameVersion())
		color.Println(color.Green, ">>>>>>>>>>>>>>>>> Local meta: <<<<<<<<<<<<<<<<<")
		color.Println(color.Blue, newMeta)
		color.Println(color.Green, ">>>>>>>>>>>>>>>>> New meta <<<<<<<<<<<<<<<<<")
		color.Println(color.Blue, newMeta)

		if err := p.Remove(false, true, true); err != nil {
			return false, err
		}

		return false, nil
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
			if strings.TrimSpace(p.BuildConfigs[index].LibraryType) == "" {
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
			files = append(files, filepath.Join(p.ctx.Platform().HostName()+"-dev", relativePath))
		} else {
			platformProject := fmt.Sprintf("%s@%s@%s", platformName, projectName, p.buildType)
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
	platformName := p.ctx.Platform().Name
	if p.DevDep {
		platformName = p.ctx.Platform().HostName() + "-dev"
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

func (p Port) crossTools() *buildsystems.CrossTools {
	toolchain := p.ctx.Toolchain()
	if toolchain == nil {
		return nil
	}

	crossTools := buildsystems.CrossTools{
		SystemName:      p.ctx.SystemName(),
		SystemProcessor: p.ctx.SystemProcessor(),
	}

	crossTools.Native = p.Native || toolchain.Name == "msvc" ||
		(toolchain.Name == "gcc" && toolchain.Path == "/usr/bin")
	crossTools.Name = toolchain.Name
	crossTools.Version = toolchain.Version
	crossTools.Host = toolchain.Host
	crossTools.CrosstoolPrefix = toolchain.CrosstoolPrefix

	rootfs := p.ctx.RootFS()
	if rootfs != nil {
		crossTools.RootFS = rootfs.fullpath
		crossTools.PkgConfigPath = rootfs.PkgConfigPath
		crossTools.IncludeDirs = rootfs.IncludeDirs
		crossTools.LibDirs = rootfs.LibDirs
	}

	crossTools.Fullpath = toolchain.fullpath
	crossTools.CC = toolchain.CC
	crossTools.CXX = toolchain.CXX
	crossTools.LD = toolchain.LD
	crossTools.AS = toolchain.AS
	crossTools.FC = toolchain.FC
	crossTools.AR = toolchain.AR
	crossTools.RANLIB = toolchain.RANLIB

	crossTools.NM = toolchain.NM
	crossTools.OBJCOPY = toolchain.OBJCOPY
	crossTools.OBJDUMP = toolchain.OBJDUMP
	crossTools.STRIP = toolchain.STRIP
	crossTools.READELF = toolchain.READELF

	// For windows MSVC.
	crossTools.MSVC.MT = toolchain.msvc.MtPath
	crossTools.MSVC.RC = toolchain.msvc.RcPath
	crossTools.MSVC.IncludeDirs = toolchain.msvc.includeDirs
	crossTools.MSVC.LibDirs = toolchain.msvc.libDirs
	crossTools.MSVC.BinDirs = toolchain.msvc.binDirs

	return &crossTools
}
