package configs

import (
	"celer/buildsystems"
	"celer/buildtools"
	"celer/pkgs/cmd"
	"celer/pkgs/dirs"
	"celer/pkgs/encrypt"
	"celer/pkgs/expr"
	"celer/pkgs/fileio"
	"celer/pkgs/proxy"
	"errors"
	"fmt"
	"os"
	"path/filepath"
	"runtime"
	"strings"

	"github.com/BurntSushi/toml"
)

const defaultPortsRepo = "https://github.com/celer-pkg/ports.git"

var (
	Version = "v0.0.0" // It would be set by build script.
	Offline bool       // In offline mode, tools and repos would not be downloaded.
)

type Context interface {
	Version() string
	Platform() *Platform
	Project() *Project
	BuildType() string
	Toolchain() *Toolchain
	WindowsKit() *WindowsKit
	RootFS() *RootFS
	Jobs() int
	Offline() bool
	CacheDir() *CacheDir
	Verbose() bool
	Optimize(buildsystem, toolchain string) *buildsystems.Optimize
	GenerateToolchainFile() error
}

func NewCeler() *Celer {
	return &Celer{
		configData: configData{
			Global: global{
				Jobs:      runtime.NumCPU(),
				BuildType: "Release",
			},
		},
	}
}

type Celer struct {
	configData

	// Internal fields.
	platform Platform
	project  Project
}

type global struct {
	ConfRepo         string `toml:"conf_repo"`
	Platform         string `toml:"platform"`
	Project          string `toml:"project"`
	Jobs             int    `toml:"jobs"`
	BuildType        string `toml:"build_type"`
	Offline          bool   `toml:"offline"`
	Verbose          bool   `toml:"verbose"`
	GithubAssetProxy string `toml:"github_asset_proxy,omitempty"`
	GithubRepoProxy  string `toml:"github_repo_proxy,omitempty"`
}

type configData struct {
	Global   global    `toml:"global"`
	CacheDir *CacheDir `toml:"cache_dir"`
}

func (c *Celer) Init() error {
	c.platform.ctx = c

	configPath := filepath.Join(dirs.WorkspaceDir, "celer.toml")
	if !fileio.PathExists(configPath) {
		// Create conf dir if not exists.
		if err := os.MkdirAll(filepath.Dir(configPath), os.ModePerm); err != nil {
			return err
		}

		// Default global values.
		c.configData.Global = global{
			Jobs:      runtime.NumCPU(),
			BuildType: "release",
			Offline:   false,
		}

		// Create celer conf file with default values.
		bytes, err := toml.Marshal(c)
		if err != nil {
			return fmt.Errorf("marshal celer conf error: %w", err)
		}

		if err := os.WriteFile(configPath, bytes, os.ModePerm); err != nil {
			return err
		}

		// Auto detect native toolchain for different os.
		if err := c.platform.detectToolchain(); err != nil {
			return err
		}
	} else {
		// Read celer conf.
		bytes, err := os.ReadFile(configPath)
		if err != nil {
			return err
		}
		if err := toml.Unmarshal(bytes, c); err != nil {
			return err
		}

		// Validate cache dirs.
		if c.configData.CacheDir != nil {
			if err := c.configData.CacheDir.Validate(); err != nil {
				return err
			}
		}

		// Init platform with platform name.
		if c.configData.Global.Platform != "" {
			if err := c.platform.Init(c.configData.Global.Platform); err != nil {
				return err
			}
		} else {
			// Auto detect native toolchain for different os.
			if err := c.platform.detectToolchain(); err != nil {
				return err
			}
		}

		// Init project with project name.
		if c.configData.Global.Project != "" {
			if err := c.project.Init(c, c.configData.Global.Project); err != nil {
				return err
			}
		}
	}

	// Set default project name.
	if c.project.Name == "" {
		c.project.Name = "unnamed"
	}

	// Cache github proxies globally.
	proxy.CacheGithubProxies(c.configData.Global.GithubAssetProxy, c.configData.Global.GithubRepoProxy)

	// Git is required to clone/update repo.
	buildtools.Offline = c.Global.Offline
	if err := buildtools.CheckTools("git"); err != nil {
		return err
	}

	// Clone ports repo if empty.
	if err := c.clonePorts(); err != nil {
		return err
	}

	return nil
}

func (c Celer) CreatePlatform(platformName string) error {
	// Clean platform name.
	platformName = strings.TrimSpace(platformName)
	platformName = strings.TrimSuffix(platformName, ".toml")

	if platformName == "" {
		return fmt.Errorf("platform name is empty")
	}

	// Create platform file.
	platformPath := filepath.Join(dirs.ConfPlatformsDir, platformName+".toml")
	var platform Platform
	if err := platform.Write(platformPath); err != nil {
		return err
	}

	return nil
}

func (c *Celer) SetBuildType(buildtype string) error {
	buildtype = strings.ToLower(buildtype)

	if buildtype != "release" && buildtype != "debug" && buildtype != "relwithdebinfo" && buildtype != "minsizerel" {
		return ErrInvalidBuildType
	}

	if err := c.readOrCreate(); err != nil {
		return err
	}

	c.configData.Global.BuildType = buildtype
	if err := c.save(); err != nil {
		return err
	}

	return nil
}

func (c *Celer) SetJobs(jobs int) error {
	if jobs < 0 {
		return ErrInvalidJobs
	}

	if err := c.readOrCreate(); err != nil {
		return err
	}

	c.configData.Global.Jobs = jobs
	if err := c.save(); err != nil {
		return err
	}

	return nil
}

func (c *Celer) SetPlatform(platformName string) error {
	if err := c.readOrCreate(); err != nil {
		return err
	}

	// Init and setup platform.
	if err := c.platform.Init(platformName); err != nil {
		return err
	}
	c.Global.Platform = platformName
	c.platform.Name = platformName

	if err := c.save(); err != nil {
		return err
	}

	return nil
}

func (c Celer) CreateProject(projectName string) error {
	// Clean platform name.
	projectName = strings.TrimSpace(projectName)
	projectName = strings.TrimSuffix(projectName, ".toml")

	if projectName == "" {
		return fmt.Errorf("project name is empty")
	}

	// Create project file.
	projectPath := filepath.Join(dirs.ConfProjectsDir, projectName+".toml")
	var project Project
	if err := project.Write(projectPath); err != nil {
		return err
	}

	return nil
}

func (c *Celer) SetProject(projectName string) error {
	if err := c.readOrCreate(); err != nil {
		return err
	}

	// Read project file and setup it.
	if err := c.project.Init(c, projectName); err != nil {
		return err
	}
	c.Global.Project = projectName

	if err := c.save(); err != nil {
		return err
	}

	return nil
}

func (c *Celer) SetOffline(offline bool) error {
	if err := c.readOrCreate(); err != nil {
		return err
	}

	c.Global.Offline = offline
	if err := c.save(); err != nil {
		return err
	}

	return nil
}

func (c *Celer) SetVerbose(vebose bool) error {
	if err := c.readOrCreate(); err != nil {
		return err
	}

	c.Global.Verbose = vebose
	if err := c.save(); err != nil {
		return err
	}

	return nil
}

func (c *Celer) SetCacheDir(dir, token string) error {
	if err := c.readOrCreate(); err != nil {
		return err
	}

	if c.configData.CacheDir != nil {
		dir = expr.If(dir != "", dir, c.configData.CacheDir.Dir)
		if !fileio.PathExists(dir) {
			return ErrCacheDirNotExist
		}
	}

	if token != "" {
		tokenFile := filepath.Join(dir, "token")
		if fileio.PathExists(tokenFile) {
			return ErrCacheTokenExist
		}

		// Token of cache dir should be encrypted.
		bytes, err := encrypt.EncodePassword(token)
		if err != nil {
			return fmt.Errorf("encode cache token: %w", err)
		}
		if err := os.WriteFile(tokenFile, bytes, os.ModePerm); err != nil {
			return fmt.Errorf("write cache token: %w", err)
		}
	}

	c.configData.CacheDir = &CacheDir{
		Dir: dir,
	}
	if err := c.save(); err != nil {
		return err
	}

	return nil
}

func (c Celer) CreatePort(nameVersion string) error {
	parts := strings.Split(nameVersion, "@")
	if len(parts) != 2 {
		return fmt.Errorf("invalid port name version")
	}

	parentDir := filepath.Join(dirs.PortsDir, parts[0])
	if err := os.MkdirAll(parentDir, os.ModePerm); err != nil {
		return err
	}

	var port Port
	portPath := filepath.Join(parentDir, parts[1], "port.toml")
	if err := port.Write(portPath); err != nil {
		return err
	}
	return nil
}

func (c *Celer) SetConfRepo(url, branch string) error {
	// No repo url specifeid, maybe want to update repo only.
	if strings.TrimSpace(url) == "" {
		return c.updateConfRepo("", branch)
	} else {
		if err := c.updateConfRepo(url, branch); err != nil {
			return err
		}
	}

	if err := c.readOrCreate(); err != nil {
		return err
	}

	c.Global.ConfRepo = url
	if err := c.save(); err != nil {
		return err
	}

	return nil
}

func (c *Celer) Deploy() error {
	if err := c.platform.Setup(); err != nil {
		return err
	}
	if err := c.project.Deploy(); err != nil {
		return err
	}

	return nil
}

func (c *Celer) readOrCreate() error {
	celerPath := filepath.Join(dirs.WorkspaceDir, "celer.toml")
	if !fileio.PathExists(celerPath) {
		// Create conf directory.
		if err := os.MkdirAll(filepath.Dir(celerPath), os.ModePerm); err != nil {
			return err
		}

		if c.Global.Jobs == 0 {
			c.Global.Jobs = runtime.NumCPU()
		}

		if c.Global.BuildType == "" {
			c.Global.BuildType = "release"
		}

		bytes, err := toml.Marshal(c)
		if err != nil {
			return err
		}
		if err := os.WriteFile(celerPath, bytes, os.ModePerm); err != nil {
			return err
		}
	} else {
		// Read celer.toml.
		bytes, err := os.ReadFile(celerPath)
		if err != nil {
			return err
		}
		if err := toml.Unmarshal(bytes, c); err != nil {
			return err
		}

		if c.Global.Jobs == 0 {
			c.Global.Jobs = runtime.NumCPU()
		}

		if c.Global.BuildType == "" {
			c.Global.BuildType = "release"
		}
	}

	return nil
}

func (c Celer) save() error {
	bytes, err := toml.Marshal(c)
	if err != nil {
		return err
	}

	celerPath := filepath.Join(dirs.WorkspaceDir, "celer.toml")
	if err := os.WriteFile(celerPath, bytes, os.ModePerm); err != nil {
		return err
	}

	return nil
}

func (c Celer) updateConfRepo(repo, branch string) error {
	// Extracted clone function for reusability.
	cloneFunc := func(workDir string) error {
		if err := os.RemoveAll(workDir); err != nil {
			return err
		}

		var commands []string
		commands = append(commands, fmt.Sprintf("git clone %s %s", repo, workDir))
		commandLine := strings.Join(commands, " && ")
		executor := cmd.NewExecutor("[clone]", commandLine)
		executor.SetWorkDir(dirs.WorkspaceDir)
		return executor.Execute()
	}

	// Extracted update function for reusability.
	updateFunc := func(workDir string) error {
		var commands []string
		commands = append(commands, "git reset --hard && git clean -xfd")
		commands = append(commands, "git fetch")
		if branch != "" {
			commands = append(commands, fmt.Sprintf("git checkout %s", branch))
		}
		commands = append(commands, "git pull")

		// Execute clone command.
		commandLine := strings.Join(commands, " && ")
		executor := cmd.NewExecutor("[update conf repo]", commandLine)
		executor.SetWorkDir(workDir)
		output, err := executor.ExecuteOutput()
		if err != nil {
			return err
		}

		fmt.Println(output)
		return nil
	}

	// Clone or checkout repo.
	confDir := filepath.Join(dirs.WorkspaceDir, "conf")
	if fileio.PathExists(confDir) {
		if fileio.PathExists(filepath.Join(confDir, ".git")) {
			return updateFunc(confDir)
		} else if repo != "" {
			return cloneFunc(confDir)
		} else {
			return fmt.Errorf("conf repo url is empty")
		}
	} else {
		return cloneFunc(confDir)
	}
}

func (c Celer) portsRepoUrl() string {
	portsRepo := os.Getenv("CELER_PORTS_REPO")
	if portsRepo != "" {
		return portsRepo
	}

	return defaultPortsRepo
}

func (c Celer) clonePorts() error {
	var cloneRequired bool

	portsDir := filepath.Join(dirs.WorkspaceDir, "ports")
	if !fileio.PathExists(portsDir) {
		cloneRequired = true
	} else {
		entities, err := os.ReadDir(portsDir)
		if err != nil {
			return err
		}

		if len(entities) == 0 {
			cloneRequired = true
		}
	}

	if cloneRequired {
		// Remove ports dir before clone it.
		if err := os.RemoveAll(portsDir); err != nil {
			return err
		}

		// Clone ports repo.
		if c.Global.Offline {
			PrintWarning(errors.New("offline is on"), "skip clone ports repo")
			return nil
		}

		command := fmt.Sprintf("git clone %s %s", c.portsRepoUrl(), portsDir)
		executor := cmd.NewExecutor("[clone ports]", command)
		if err := executor.Execute(); err != nil {
			return fmt.Errorf("`https://github.com/celer-pkg/ports.git` is not available, but your can change the default ports repo in celer.toml: %w", err)
		}
	}

	return nil
}

// ======================= celer context implementation ====================== //

func (c *Celer) Version() string {
	return Version
}

func (c *Celer) Platform() *Platform {
	return &c.platform
}

func (c Celer) Project() *Project {
	return &c.project
}

func (c Celer) BuildType() string {
	return c.configData.Global.BuildType
}

func (c *Celer) Toolchain() *Toolchain {
	return c.platform.Toolchain
}

func (c Celer) WindowsKit() *WindowsKit {
	return c.platform.WindowsKit
}

func (c Celer) RootFS() *RootFS {
	return c.platform.RootFS
}

func (c Celer) Jobs() int {
	return c.Global.Jobs
}

func (c Celer) Offline() bool {
	return c.Global.Offline
}

func (c Celer) CacheDir() *CacheDir {
	return c.configData.CacheDir
}

func (c Celer) Verbose() bool {
	return c.configData.Global.Verbose
}

func (c Celer) Optimize(buildsystem, toolchain string) *buildsystems.Optimize {
	if c.project.Optimize != nil {
		return c.project.Optimize
	}

	if runtime.GOOS == "windows" {
		if toolchain == "msvc" {
			switch buildsystem {
			case "cmake":
				return c.project.OptimizeWindows
			case "makefiles":
				return c.project.OptimizeLinux
			}
		}
	} else {
		return c.project.OptimizeLinux
	}

	return c.project.OptimizeLinux
}

func (c Celer) GenerateToolchainFile() error {
	var toolchain strings.Builder
	toolchain.WriteString("# ========= WARNING: This toolchain file is generated by celer. ========= #\n")

	// Toolchain related.
	if err := c.platform.Toolchain.generate(&toolchain, c.platform.HostName()); err != nil {
		return err
	}

	// Rootfs related.
	rootfs := c.RootFS()
	if rootfs != nil {
		if err := rootfs.generate(&toolchain); err != nil {
			return err
		}
	}

	// Write pkg config.
	c.writePkgConfig(&toolchain)

	// Set CMAKE_FIND_ROOT_PATH.
	platformProject := c.Global.Platform + "@" + c.Global.Project + "@" + strings.ToLower(c.Global.BuildType)
	dependencyDir := "${WORKSPACE_DIR}/installed/" + platformProject
	var rootpaths = []string{dependencyDir}
	if c.RootFS() != nil {
		rootpaths = append(rootpaths, "${CMAKE_SYSROOT}")
	}
	toolchain.WriteString("\n# Library search paths.\n")
	toolchain.WriteString("if(DEFINED CMAKE_FIND_ROOT_PATH)\n")
	toolchain.WriteString("\tset(CMAKE_FIND_ROOT_PATH \"${CMAKE_FIND_ROOT_PATH}\")\n")
	toolchain.WriteString("else()\n")
	toolchain.WriteString(fmt.Sprintf("\tset(%s %q)\n", "CMAKE_FIND_ROOT_PATH", strings.Join(rootpaths, ";")))
	toolchain.WriteString("endif()\n")

	// Define global cmake vars, env vars, micro vars and compile flags.
	for index, item := range c.project.Vars {
		if index == 0 {
			toolchain.WriteString("\n# Global cmake vars.\n")
		}

		parts := strings.Split(item, "=")
		if len(parts) == 1 {
			toolchain.WriteString(fmt.Sprintf(`set(%s CACHE INTERNAL "defined by celer globally.")`, item) + "\n")
		} else if len(parts) == 2 {
			toolchain.WriteString(fmt.Sprintf(`set(%s "%s" CACHE INTERNAL "defined by celer globally.")`, parts[0], parts[1]) + "\n")
		} else {
			return fmt.Errorf("invalid cmake var: %s", item)
		}
	}

	for index, item := range c.project.Envs {
		parts := strings.Split(item, "=")
		if len(parts) != 2 {
			return fmt.Errorf("invalid env var: %s", item)
		}

		if index == 0 {
			toolchain.WriteString("\n# Global envs.\n")
		}
		toolchain.WriteString(fmt.Sprintf(`set(ENV{%s} "%s")`, parts[0], parts[1]) + "\n")
	}

	for index, item := range c.project.Micros {
		if index == 0 {
			toolchain.WriteString("\n# Global micros.\n")
		}
		toolchain.WriteString(fmt.Sprintf("add_compile_definitions(%s)\n", item))
	}

	optimize := c.Optimize("cmake", expr.If(runtime.GOOS == "windows", "msvc", "gcc"))
	if optimize != nil {
		toolchain.WriteString("\n# Compile flags.\n")
		toolchain.WriteString("add_compile_options(\n")
		if optimize.Release != "" {
			flags := strings.Join(strings.Fields(optimize.Release), ";")
			toolchain.WriteString(fmt.Sprintf("\t\"$<$<CONFIG:Release>:%s>\"\n", flags))
		}
		if optimize.Debug != "" {
			flags := strings.Join(strings.Fields(optimize.Debug), ";")
			toolchain.WriteString(fmt.Sprintf("\t\"$<$<CONFIG:Debug>:%s>\"\n", flags))
		}
		if optimize.RelWithDebInfo != "" {
			flags := strings.Join(strings.Fields(optimize.RelWithDebInfo), ";")
			toolchain.WriteString(fmt.Sprintf("\t\"$<$<CONFIG:RelWithDebInfo>:%s>\"\n", flags))
		}
		if optimize.MinSizeRel != "" {
			flags := strings.Join(strings.Fields(optimize.MinSizeRel), ";")
			toolchain.WriteString(fmt.Sprintf("\t\"$<$<CONFIG:MinSizeRel>:%s>\"\n", flags))
		}
		if len(c.project.Flags) > 0 {
			for _, item := range c.project.Flags {
				toolchain.WriteString(fmt.Sprintf("\t%q\n", item))
			}
		}
		toolchain.WriteString(")\n")
	}

	toolchain.WriteString("\n")
	if c.Toolchain().Name == "gcc" {
		toolchain.WriteString(fmt.Sprintf("set(%s %q)\n", "CMAKE_INSTALL_RPATH", `\$ORIGIN/../lib`))
	}
	toolchain.WriteString(fmt.Sprintf("set(%-30s%s)\n", "CMAKE_EXPORT_COMPILE_COMMANDS", "ON"))

	// Write toolchain file.
	toolchainPath := filepath.Join(dirs.WorkspaceDir, "toolchain_file.cmake")
	if err := os.WriteFile(toolchainPath, []byte(toolchain.String()), os.ModePerm); err != nil {
		return err
	}

	return nil
}

func (c Celer) writePkgConfig(toolchain *strings.Builder) {
	var (
		configPaths   []string
		configLibDirs []string
		sysrootDir    string
	)

	libraryFolder := fmt.Sprintf("%s@%s@%s", c.Platform().Name, c.Project().Name, strings.ToLower(c.BuildType()))
	installedDir := filepath.Join("${WORKSPACE_DIR}/installed", libraryFolder)

	switch runtime.GOOS {
	case "windows":
		configPaths = []string{
			filepath.ToSlash(filepath.Join(installedDir, "lib", "pkgconfig")),
			filepath.ToSlash(filepath.Join(installedDir, "share", "pkgconfig")),
		}
		sysrootDir = filepath.ToSlash(installedDir)

	case "linux":
		// Target directory.
		var targetDir string
		if c.RootFS() != nil {
			for _, configPath := range c.RootFS().PkgConfigPath {
				configLibDirs = append(configLibDirs, filepath.Join("${CMAKE_SYSROOT}", configPath))
			}

			// tmpdeps is a symlink in rootfs.
			sysrootDir = "${CMAKE_SYSROOT}"
			targetDir = filepath.Join(sysrootDir, "tmp", "deps", libraryFolder)
		} else {
			sysrootDir = "${WORKSPACE_DIR}/installed"
			targetDir = sysrootDir
		}

		// Append pkgconfig with tmp/deps directory.
		configPaths = []string{
			filepath.Join(targetDir, "lib", "pkgconfig"),
			filepath.Join(targetDir, "share", "pkgconfig"),
		}
	}

	toolchain.WriteString("\n# pkg-config search paths.\n")
	executablePath := fmt.Sprintf("${WORKSPACE_DIR}/installed/%s-dev/bin/pkgconf", c.platform.HostName())
	toolchain.WriteString(fmt.Sprintf("set(%-28s%q)\n", "PKG_CONFIG_EXECUTABLE", executablePath))

	// PKG_CONFIG_SYSROOT_DIR
	toolchain.WriteString(fmt.Sprintf("set(%s %q)\n", "ENV{PKG_CONFIG_SYSROOT_DIR}", sysrootDir))

	// PKG_CONFIG_LIBDIR
	if len(configLibDirs) > 0 {
		toolchain.WriteString("set(PKG_CONFIG_LIBDIR" + "\n")
		for _, path := range configLibDirs {
			toolchain.WriteString(fmt.Sprintf(`	"%s"`, path) + "\n")
		}
		toolchain.WriteString(")\n")
		toolchain.WriteString(fmt.Sprintf(`list(JOIN PKG_CONFIG_LIBDIR "%s" PKG_CONFIG_LIBDIR_STR)`, string(os.PathListSeparator)) + "\n")
		toolchain.WriteString(fmt.Sprintf("set(%s %q)\n\n", "ENV{PKG_CONFIG_LIBDIR}", "${PKG_CONFIG_LIBDIR_STR}"))
	}

	// PKG_CONFIG_PATH
	if len(configPaths) > 0 {
		toolchain.WriteString("set(PKG_CONFIG_PATH" + "\n")
		for _, path := range configPaths {
			toolchain.WriteString(fmt.Sprintf("\t%q", path) + "\n")
		}
		toolchain.WriteString(")\n")
		toolchain.WriteString(fmt.Sprintf(`list(JOIN PKG_CONFIG_PATH "%s" PKG_CONFIG_PATH_STR)`, string(os.PathListSeparator)) + "\n")
		toolchain.WriteString(fmt.Sprintf("set(%s %q)\n", "ENV{PKG_CONFIG_PATH}", "${PKG_CONFIG_PATH_STR}"))
	}
}
