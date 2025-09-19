package configs

import (
	"celer/buildtools"
	"celer/pkgs/cmd"
	"celer/pkgs/dirs"
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
	DevMode bool       // In dev mode, detail message would be hide.
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
	JobNum() int
	Offline() bool
	CacheDir() *CacheDir
	SystemName() string
	SystemProcessor() string
}

func NewCeler() *Celer {
	return &Celer{
		configData: configData{
			Global: global{
				JobNum:    runtime.NumCPU(),
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
	JobNum           int    `toml:"job_num"`
	BuildType        string `toml:"build_type"`
	Offline          bool   `toml:"offline"`
	GithubAssetProxy string `toml:"github_asset_proxy,omitempty"`
	GithubRepoProxy  string `toml:"github_repo_proxy,omitempty"`
}

type configData struct {
	Global   global    `toml:"global"`
	CacheDir *CacheDir `toml:"cache_dir"`
}

func (c *Celer) Init() error {
	configPath := filepath.Join(dirs.WorkspaceDir, "celer.toml")
	if !fileio.PathExists(configPath) {
		// Create conf dir if not exists.
		if err := os.MkdirAll(filepath.Dir(configPath), os.ModePerm); err != nil {
			return err
		}

		// Default global values.
		c.configData.Global = global{
			JobNum:    runtime.NumCPU(),
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

		// Pass context to platform.
		c.platform.ctx = c
	} else {
		// Read celer conf.
		bytes, err := os.ReadFile(configPath)
		if err != nil {
			return err
		}
		if err := toml.Unmarshal(bytes, c); err != nil {
			return err
		}

		c.platform.ctx = c
		c.configData.Global.BuildType = strings.ToLower(c.configData.Global.BuildType)

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
			// No platform specified, setup will auto detect native toolchain.
			if err := c.platform.Setup(); err != nil {
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

func (c *Celer) SetJobNum(jobNum int) error {
	if jobNum < 0 {
		return ErrInvalidJobNum
	}

	if err := c.readOrCreate(); err != nil {
		return err
	}

	c.configData.Global.JobNum = jobNum
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

func (c *Celer) SetCacheDir(dir, token string) error {
	if err := c.readOrCreate(); err != nil {
		return err
	}

	if c.configData.CacheDir != nil {
		dir = expr.If(dir != "", dir, c.configData.CacheDir.Dir)
		token = expr.If(token != "", token, c.configData.CacheDir.Token)

		if !fileio.PathExists(dir) {
			return ErrCacheDirNotExist
		}
	}

	c.configData.CacheDir = &CacheDir{
		Dir:   dir,
		Token: token,
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

func (c Celer) GenerateToolchainFile() error {
	var toolchain strings.Builder

	// Setup celer during configuration.
	toolchain.WriteString(fmt.Sprintf(`# This was generated by celer. (Do not change it manually!)

# Setup celer during configuration.
if(NOT DEFINED CELER_DEPLOYED)
	get_filename_component(WORKSPACE_DIR "${CMAKE_CURRENT_LIST_FILE}" PATH)
	find_program(CELER celer PATHS ${WORKSPACE_DIR} NO_DEFAULT_PATH)
	if(CELER)
		execute_process(
			COMMAND ${CELER} deploy --dev-mode --build-type=%s
			WORKING_DIRECTORY ${WORKSPACE_DIR}
			RESULT_VARIABLE celer_result
		)
		if(NOT "${celer_result}" STREQUAL "0")
            message(FATAL_ERROR "celer deploy failed.")
        endif()
        set(CELER_DEPLOYED TRUE CACHE INTERNAL "celer already deploy")
	endif()
endif()`, c.BuildType()) + "\n")

	// Sysroot related.
	if c.RootFS() != nil {
		if err := c.RootFS().generate(&toolchain); err != nil {
			return err
		}
	}

	// Toolchain related.
	if c.Toolchain() != nil {
		// Append toolchain platform infos.
		toolchain.WriteString("\n# Set toolchain platform infos.\n")
		toolchain.WriteString(fmt.Sprintf("set(CMAKE_SYSTEM_NAME \"%s\")\n", c.SystemName()))
		toolchain.WriteString(fmt.Sprintf("set(CMAKE_SYSTEM_PROCESSOR \"%s\")\n", c.SystemProcessor()))

		if err := c.platform.Toolchain.generate(&toolchain, c.platform.HostName()); err != nil {
			return err
		}
	}

	// Pkg-config related.
	toolchain.WriteString("\n# Set pkg-config search paths.\n")
	pkgConfigPaths := []string{
		fmt.Sprintf("${WORKSPACE_DIR}/installed/%s-dev/bin", c.platform.HostName()),
	}
	if c.RootFS() != nil {
		toolchain.WriteString(`set(ENV{PKG_CONFIG_SYSROOT_DIR} "${CMAKE_SYSROOT}")` + "\n")

		for _, configPath := range c.RootFS().PkgConfigPath {
			fullConfigPath := filepath.Join(c.RootFS().fullpath, configPath)
			relConfigPath := strings.TrimPrefix(fullConfigPath, dirs.WorkspaceDir+string(os.PathSeparator))
			pkgConfigPaths = append(pkgConfigPaths,
				"${WORKSPACE_DIR}/"+relConfigPath,
			)
		}
	}

	toolchain.WriteString("set(PKG_CONFIG_PATH" + "\n")
	for _, configPath := range pkgConfigPaths {
		toolchain.WriteString(fmt.Sprintf(`	"%s"`, configPath) + "\n")
	}
	toolchain.WriteString(")\n")
	toolchain.WriteString(fmt.Sprintf(`list(JOIN PKG_CONFIG_PATH "%s" PKG_CONFIG_PATH_STR)`, string(os.PathListSeparator)) + "\n")
	toolchain.WriteString(`set(ENV{PKG_CONFIG_PATH} "${PKG_CONFIG_PATH_STR}")` + "\n")

	toolchain.WriteString("\n# Set cmake library search paths.\n")
	platformProject := c.Global.Platform + "@" + c.Global.Project + "@" + strings.ToLower(c.Global.BuildType)
	installedDir := "${WORKSPACE_DIR}/installed/" + platformProject
	toolchain.WriteString(fmt.Sprintf(`list(APPEND CMAKE_FIND_ROOT_PATH "%s")`, installedDir) + "\n")
	toolchain.WriteString(fmt.Sprintf(`list(APPEND CMAKE_PREFIX_PATH "%s")`, installedDir) + "\n")

	// Define global cmake vars, env vars, micro vars and compile options.
	for index, item := range c.project.Vars {
		if index == 0 {
			toolchain.WriteString("\n# Define global cmake vars.\n")
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
			toolchain.WriteString("\n# Define global envs.\n")
		}
		toolchain.WriteString(fmt.Sprintf(`set(ENV{%s} "%s")`, parts[0], parts[1]) + "\n")
	}

	for index, item := range c.project.Micros {
		if index == 0 {
			toolchain.WriteString("\n# Define global micros.\n")
		}
		toolchain.WriteString(fmt.Sprintf("add_compile_definitions(%s)\n", item))
	}

	if len(c.project.Flags) > 0 || c.project.OptFlags != nil {
		toolchain.WriteString("\n# Define flags.\n")
		toolchain.WriteString("add_compile_options(\n")
		if c.project.OptFlags != nil {
			if c.project.OptFlags.Release != "" {
				toolchain.WriteString(fmt.Sprintf("\t\"$<$<CONFIG:Release>:%s>\"\n", c.project.OptFlags.Release))
			}
			if c.project.OptFlags.Debug != "" {
				toolchain.WriteString(fmt.Sprintf("\t\"$<$<CONFIG:Debug>:%s>\"\n", c.project.OptFlags.Debug))
			}
			if c.project.OptFlags.RelWithDebInfo != "" {
				toolchain.WriteString(fmt.Sprintf("\t\"$<$<CONFIG:RelWithDebInfo>:%s>\"\n", c.project.OptFlags.RelWithDebInfo))
			}
			if c.project.OptFlags.MinSizeRel != "" {
				toolchain.WriteString(fmt.Sprintf("\t\"$<$<CONFIG:MinSizeRel>:%s>\"\n", c.project.OptFlags.MinSizeRel))
			}
		}
		for _, item := range c.project.Flags {
			toolchain.WriteString(fmt.Sprintf("\t%q\n", item))
		}
		toolchain.WriteString(")\n")
	}

	// Write toolchain file.
	toolchainPath := filepath.Join(dirs.WorkspaceDir, "toolchain_file.cmake")
	if err := os.WriteFile(toolchainPath, []byte(toolchain.String()), os.ModePerm); err != nil {
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

		if c.Global.JobNum == 0 {
			c.Global.JobNum = runtime.NumCPU()
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

		if c.Global.JobNum == 0 {
			c.Global.JobNum = runtime.NumCPU()
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

func (c Celer) SystemName() string {
	if c.Toolchain() == nil {
		return runtime.GOOS
	}

	return c.Toolchain().SystemName
}

func (c Celer) SystemProcessor() string {
	if c.Toolchain() == nil {
		return runtime.GOARCH
	}
	return c.Toolchain().SystemProcessor
}

func (c Celer) JobNum() int {
	return c.Global.JobNum
}

func (c Celer) Offline() bool {
	return c.Global.Offline
}

func (c Celer) CacheDir() *CacheDir {
	return c.configData.CacheDir
}
