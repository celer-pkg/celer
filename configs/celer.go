package configs

import (
	"celer/buildtools"
	"celer/pkgs/cmd"
	"celer/pkgs/dirs"
	"celer/pkgs/fileio"
	"celer/pkgs/proxy"
	"fmt"
	"os"
	"path/filepath"
	"runtime"
	"strings"

	"github.com/BurntSushi/toml"
)

const portsRepo = "https://github.com/celer-pkg/ports.git"

var (
	Version = "v0.0.0" // It would be set by build script.
	DevMode bool       // In dev mode, detail message would be hide.
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
	CacheDir() *CacheDir
	SystemName() string
	SystemProcessor() string
}

func NewCeler() *Celer {
	return &Celer{
		configData: configData{
			Gloabl: gloabl{
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

type gloabl struct {
	ConfRepo         string `toml:"conf_repo"`
	PortsRepo        string `toml:"ports_repo"`
	Platform         string `toml:"platform"`
	Project          string `toml:"project"`
	JobNum           int    `toml:"job_num"`
	BuildType        string `toml:"build_type"`
	GithubAssetProxy string `toml:"github_asset_proxy,omitempty"`
	GithubRepoProxy  string `toml:"github_repo_proxy,omitempty"`
}

type configData struct {
	Gloabl   gloabl    `toml:"gloabl"`
	CacheDir *CacheDir `toml:"cache_dir"`
}

func (c *Celer) Init() error {
	configPath := filepath.Join(dirs.WorkspaceDir, "celer.toml")
	if !fileio.PathExists(configPath) {
		// Create conf dir if not exists.
		if err := os.MkdirAll(filepath.Dir(configPath), os.ModePerm); err != nil {
			return err
		}

		c.configData.Gloabl.JobNum = runtime.NumCPU()
		c.configData.Gloabl.BuildType = "release"
		c.configData.Gloabl.PortsRepo = portsRepo

		// Create celer conf file with default values.
		bytes, err := toml.Marshal(c)
		if err != nil {
			return fmt.Errorf("cannot marshal celer conf: %w", err)
		}

		if err := os.WriteFile(configPath, bytes, os.ModePerm); err != nil {
			return err
		}
	} else {
		// Rewrite celer file with new platform.
		bytes, err := os.ReadFile(configPath)
		if err != nil {
			return err
		}
		if err := toml.Unmarshal(bytes, c); err != nil {
			return err
		}

		// Set default ports repo if not set.
		if c.Gloabl.PortsRepo == "" {
			c.configData.Gloabl.PortsRepo = portsRepo
		}

		// Lower case build type always.
		c.configData.Gloabl.BuildType = strings.ToLower(c.configData.Gloabl.BuildType)

		// Validate cache dirs.
		if c.configData.CacheDir != nil {
			if err := c.configData.CacheDir.Validate(); err != nil {
				return err
			}
		}

		// Init platform with platform name.
		if c.configData.Gloabl.Platform != "" {
			if err := c.platform.Init(c, c.configData.Gloabl.Platform); err != nil {
				return err
			}
		} else {
			// No platform specified, setup will auto detect native toolchain.
			if err := c.platform.Setup(); err != nil {
				return err
			}
		}

		// Init project with project name.
		if c.configData.Gloabl.Project != "" {
			if err := c.project.Init(c, c.configData.Gloabl.Project); err != nil {
				return err
			}
		}
	}

	// Set default project name.
	if c.project.Name == "" {
		c.project.Name = "unnamed"
	}

	// Cache github proxies globally.
	proxy.CacheGithubProxies(c.configData.Gloabl.GithubAssetProxy, c.configData.Gloabl.GithubRepoProxy)

	// Git is required to clone/update repo.
	if err := buildtools.CheckTools("git"); err != nil {
		return err
	}

	// Clone ports repo if empty.
	if err := c.clonePorts(); err != nil {
		return err
	}

	// Clone conf repo if specified.
	if c.configData.Gloabl.ConfRepo != "" {
		if err := c.SyncConf(c.configData.Gloabl.ConfRepo, ""); err != nil {
			return err
		}
	}

	return nil
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
		command := fmt.Sprintf("git clone %s %s", c.configData.Gloabl.PortsRepo, portsDir)
		executor := cmd.NewExecutor("[clone ports]", command)
		if err := executor.Execute(); err != nil {
			return fmt.Errorf("`https://github.com/celer-pkg/ports.git` is not available, but your can change the default ports repo in celer.toml: %w", err)
		}
	}

	return nil
}

func (c Celer) Deploy() error {
	if err := c.platform.Setup(); err != nil {
		return err
	}
	if err := c.project.Deploy(); err != nil {
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

func (c *Celer) ChangePlatform(platformName string) error {
	// Init celer with "celer.toml"
	celerPath := filepath.Join(dirs.WorkspaceDir, "celer.toml")

	if !fileio.PathExists(celerPath) { // Create celer.toml if not exist.
		// Create conf directory.
		if err := os.MkdirAll(filepath.Dir(celerPath), os.ModePerm); err != nil {
			return err
		}

		// Create celer conf file with default values.
		c.Gloabl.JobNum = runtime.NumCPU()
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
	}

	// Init and setup platform.
	if err := c.platform.Init(c, platformName); err != nil {
		return err
	}
	c.Gloabl.Platform = platformName
	c.platform.Name = platformName

	// Do change platform.
	bytes, err := toml.Marshal(c)
	if err != nil {
		return err
	}
	if err := os.WriteFile(celerPath, bytes, os.ModePerm); err != nil {
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

func (c *Celer) ChangeProject(projectName string) error {
	celerPath := filepath.Join(dirs.WorkspaceDir, "celer.toml")

	// Create celer.toml if not exist.
	if !fileio.PathExists(celerPath) {
		// Create conf directory.
		if err := os.MkdirAll(filepath.Dir(celerPath), os.ModePerm); err != nil {
			return err
		}

		// Create celer conf file with default values.
		c.Gloabl.JobNum = runtime.NumCPU()
		bytes, err := toml.Marshal(c)
		if err != nil {
			return fmt.Errorf("cannot marshal celer conf: %w", err)
		}
		if err := os.WriteFile(celerPath, bytes, os.ModePerm); err != nil {
			return err
		}

		return nil
	}

	// Read celer.toml to check if platform is selected.
	bytes, err := os.ReadFile(celerPath)
	if err != nil {
		return err
	}
	if err := toml.Unmarshal(bytes, c); err != nil {
		return fmt.Errorf("cannot unmarshal celer conf: %w", err)
	}

	// Read project file and setup it.
	if err := c.project.Init(c, projectName); err != nil {
		return err
	}
	c.Gloabl.Project = projectName

	// Do change project.
	bytes, err = toml.Marshal(c)
	if err != nil {
		return err
	}
	if err := os.WriteFile(celerPath, bytes, os.ModePerm); err != nil {
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

func (c *Celer) SyncConf(url, branch string) error {
	// No repo url specifeid, maybe want to update repo only.
	if strings.TrimSpace(url) == "" {
		return c.updateConfRepo("", branch)
	}

	// Create celer.toml if not exist.
	confPath := filepath.Join(dirs.WorkspaceDir, "celer.toml")
	if !fileio.PathExists(confPath) {
		// Create conf directory.
		if err := os.MkdirAll(filepath.Dir(confPath), os.ModePerm); err != nil {
			return err
		}

		// Create celer conf file with default values.
		c.Gloabl.JobNum = runtime.NumCPU()
		c.Gloabl.BuildType = "release"
		c.Gloabl.ConfRepo = url
		bytes, err := toml.Marshal(c)
		if err != nil {
			return err
		}
		if err := os.WriteFile(confPath, []byte(bytes), os.ModePerm); err != nil {
			return err
		}
	}

	// Update conf repo with repo url.
	bytes, err := os.ReadFile(confPath)
	if err != nil {
		return err
	}

	// Unmarshall with celer.toml
	if err := toml.Unmarshal(bytes, c); err != nil {
		return err
	}

	// Override celer.toml with repo url.
	if url != "" {
		c.Gloabl.ConfRepo = url
		bytes, err := toml.Marshal(c)
		if err != nil {
			return err
		}
		if err := os.WriteFile(confPath, []byte(bytes), os.ModePerm); err != nil {
			return err
		}
	}

	// Update repo.
	return c.updateConfRepo(c.Gloabl.ConfRepo, branch)
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
	platformProject := c.Gloabl.Platform + "@" + c.Gloabl.Project + "@" + strings.ToLower(c.Gloabl.BuildType)
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
	for index, item := range c.project.CompileOptions {
		if index == 0 {
			toolchain.WriteString("\n# Define global compile options.\n")
		}
		toolchain.WriteString(fmt.Sprintf("add_compile_options(%s)\n", item))
	}

	// Write toolchain file.
	toolchainPath := filepath.Join(dirs.WorkspaceDir, "toolchain_file.cmake")
	if err := os.WriteFile(toolchainPath, []byte(toolchain.String()), os.ModePerm); err != nil {
		return err
	}

	return nil
}

func (c Celer) updateConfRepo(repo, branch string) error {
	// Extracted clone function for reusability.
	cloneFunc := func(commands []string, workDir string) error {
		commands = append(commands, fmt.Sprintf("git clone %s %s", repo, workDir))

		// Execute clone command.
		commandLine := strings.Join(commands, " && ")
		executor := cmd.NewExecutor("[clone]", commandLine)
		executor.SetWorkDir(dirs.WorkspaceDir)
		return executor.Execute()
	}

	// Extracted update function for reusability.
	updateFunc := func(workDir string) error {
		var commands []string
		commands = append(commands, "git reset --hard && git clean -xfd")
		commands = append(commands, fmt.Sprintf("git -C %s fetch", workDir))
		if branch != "" {
			commands = append(commands, fmt.Sprintf("git -C %s checkout %s", workDir, branch))
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
			commands := []string{fmt.Sprintf("rm -rf %s", confDir)}
			return cloneFunc(commands, confDir)
		} else {
			return fmt.Errorf("conf repo url is empty")
		}
	} else {
		var commands []string
		return cloneFunc(commands, confDir)
	}
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
	return c.configData.Gloabl.BuildType
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
	return c.Gloabl.JobNum
}

func (c Celer) CacheDir() *CacheDir {
	return c.configData.CacheDir
}
