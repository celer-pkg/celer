package configs

import (
	"fmt"
	"os"
	"path/filepath"
	"runtime"
	"strings"

	"github.com/celer-pkg/celer/buildtools"
	"github.com/celer-pkg/celer/context"
	"github.com/celer-pkg/celer/envs"
	"github.com/celer-pkg/celer/pkgs/color"
	"github.com/celer-pkg/celer/pkgs/dirs"
	"github.com/celer-pkg/celer/pkgs/errors"
	"github.com/celer-pkg/celer/pkgs/fileio"
	"github.com/celer-pkg/celer/pkgs/git"

	"github.com/BurntSushi/toml"
)

const (
	defaultPortsRepoUrl   = "https://github.com/celer-pkg/ports.git"
	defaultPortRepoBranch = ""
)

// InitOption is the option for InitWithOptions.
type InitOption struct {
	SkipPlatform bool
	SkipProject  bool
}

var Version = "v0.0.0" // It would be set by build script.

func NewCeler() *Celer {
	// Make sure build env is clean.
	envs.CleanEnv()

	// Clear metadata caches from previous invocations.
	ResetMetaCache()

	return &Celer{
		configData: configData{
			Main: main{
				Jobs:      runtime.NumCPU() - 1,
				BuildType: "Release",
			},
		},
	}
}

type Celer struct {
	configData

	// Internal fields.
	platform       Platform
	project        Project
	exprVars       context.ExprVars
	devCacheConfig *DevCacheConfig
}

type main struct {
	ConfRepo  string `toml:"conf_repo"`
	Platform  string `toml:"platform"`
	Project   string `toml:"project"`
	BuildType string `toml:"build_type"`
	Downloads string `toml:"downloads,omitempty"`
	Jobs      int    `toml:"jobs"`
	Verbose   bool   `toml:"verbose"`
	Offline   bool   `toml:"offline"`
}

type Proxy struct {
	Host string `toml:"host"`
	Port int    `toml:"port"`
}

type features struct {
	IgnoreCheckCMakeAbsPath bool `toml:"ignore_check_cmake_abs_path"`
}

func (i features) ShouldIgnoreCheckCMakeAbsPath() bool {
	return i.IgnoreCheckCMakeAbsPath
}

type Python struct {
	Version        string   `toml:"version,omitempty"`
	IndexUrl       string   `toml:"index_url,omitempty"`
	ExtraIndexUrls []string `toml:"extra_index_urls,omitempty"`
	TrustedHosts   []string `toml:"trusted_hosts,omitempty"`
}

func (p *Python) GetVersion() string {
	return p.Version
}

func (p *Python) GetIndexUrl() string {
	return p.IndexUrl
}

func (p *Python) GetExtraIndexUrls() []string {
	return p.ExtraIndexUrls
}

func (p *Python) GetTrustedHosts() []string {
	return p.TrustedHosts
}

type configData struct {
	Main           main            `toml:"main"`
	Proxy          *Proxy          `toml:"proxy,omitempty"`
	PkgCacheConfig *PkgCacheConfig `toml:"pkgcache,omitempty"`
	CCache         *CCache         `toml:"ccache,omitempty"`
	Python         *Python         `toml:"python,omitempty"`
	Features       *features       `toml:"features,omitempty"`
}

// Init initializes celer with default options.
func (c *Celer) Init() error {
	return c.InitWithOptions(InitOption{})
}

// InitWithOptions initializes celer with options.
func (c *Celer) InitWithOptions(opts InitOption) error {
	return c.InitWithPlatform(c.Main.Platform, opts)
}

// InitWithPlatform initializes celer with platform.
func (c *Celer) InitWithPlatform(platform string, opts InitOption) error {
	c.platform.ctx = c

	configPath := filepath.Join(dirs.WorkspaceDir, "celer.toml")
	if !fileio.PathExists(configPath) {
		// Create conf dir if not exists.
		if err := os.MkdirAll(filepath.Dir(configPath), os.ModePerm); err != nil {
			return err
		}

		// Use all CPU cores for CI, otherwise use all cores except 1 to avoid blocking UI.
		var jobs int
		if _, ok := os.LookupEnv("GITHUB_ACTIONS"); ok {
			jobs = runtime.NumCPU()
			fmt.Printf("-- GITHUB_ACTIONS: jobs: %d\n", jobs)
		} else {
			jobs = runtime.NumCPU() - 1
		}

		// Default global values.
		c.Main = main{
			BuildType: "release",
			Downloads: "",
			Jobs:      jobs,
			Offline:   false,
			Verbose:   false,
		}

		// Create celer conf file with default values.
		bytes, err := toml.Marshal(c)
		if err != nil {
			return fmt.Errorf("failed to marshal conf -> %w", err)
		}

		// Set platform and init platform if specified.
		if platform != "" {
			c.Main.Platform = platform
			if err := c.platform.Init(c.Main.Platform); err != nil {
				// Skip platform init if platform not exist.
				if errors.Is(err, errors.ErrPlatformNotExist) && opts.SkipPlatform {
					c.Main.Platform = ""
					return nil
				}
				return err
			}
		}

		if err := os.WriteFile(configPath, bytes, os.ModePerm); err != nil {
			return err
		}
	} else {
		// Read celer conf.
		bytes, err := os.ReadFile(configPath)
		if err != nil {
			return fmt.Errorf("failed to read conf -> %w", err)
		}
		if err := toml.Unmarshal(bytes, c); err != nil {
			return fmt.Errorf("failed to unmarshal conf -> %w", err)
		}

		// Normalize path separators to forward slashes.
		c.Main.Downloads = filepath.ToSlash(c.Main.Downloads)
		if c.configData.CCache != nil {
			c.configData.CCache.Dir = filepath.ToSlash(c.configData.CCache.Dir)
		}
		if c.configData.PkgCacheConfig != nil {
			c.configData.PkgCacheConfig.Dir = filepath.ToSlash(c.configData.PkgCacheConfig.Dir)
		}

		// Use lower case build type in celer as default.
		c.Main.BuildType = strings.ToLower(c.Main.BuildType)

		// Set platform and init platform if specified.
		if platform != "" {
			c.Main.Platform = platform
		}
		if c.Main.Platform != "" {
			if err := c.platform.Init(c.Main.Platform); err != nil {
				// Skip platform init if platform not exist.
				if errors.Is(err, errors.ErrPlatformNotExist) && opts.SkipPlatform {
					c.Main.Platform = ""
					return nil
				}
				return err
			}
		}

		// Init project with project name.
		if c.Main.Project != "" {
			if err := c.project.Init(c, c.Main.Project); err != nil {
				// Skip project init if project not exist.
				if errors.Is(err, errors.ErrProjectNotExist) && opts.SkipProject {
					c.Main.Project = ""
					return nil
				}
				return err
			}
		}

		// Validate package cache.
		if c.configData.PkgCacheConfig != nil {
			c.configData.PkgCacheConfig.ctx = c
			if err := c.configData.PkgCacheConfig.Refresh(); err != nil {
				return err
			}
		}

		// Setup ccache.
		if c.configData.CCache != nil {
			if err := c.configData.CCache.Setup(); err != nil {
				return err
			}
		}

		// Assign default project and python version after saving celer.toml.
		if c.Main.Project == "" {
			c.Main.Project = "default"
			c.project.Name = "default"
		}
	}

	// Celer support detect local toolchain, if platform name is not specified, use default toolchain:
	// Windows: default is msvc,
	// Linux: default is gcc.
	if c.Main.Platform == "" {
		var toolchain = Toolchain{ctx: c}
		if err := toolchain.Detect(c.Main.Platform); err != nil {
			return fmt.Errorf("detect celer.toolchain -> %w", err)
		}
		c.platform.Toolchain = &toolchain
		c.platform.Toolchain.SystemName = runtime.GOOS

		switch runtime.GOARCH {
		case "amd64":
			c.platform.Toolchain.SystemProcessor = "x86_64"
		case "arm64":
			c.platform.Toolchain.SystemProcessor = "aarch64"
		case "386":
			c.platform.Toolchain.SystemProcessor = "i686"
		default:
			return fmt.Errorf("unsupported architecture: %s", runtime.GOARCH)
		}
	}

	// Detected windows kit.
	if runtime.GOOS == "windows" {
		var windowsKit WindowsKit
		if err := windowsKit.Detect(&c.platform.Toolchain.MSVC); err != nil {
			return fmt.Errorf("failed to detect celer.windows_kit.\n -> %w", err)
		}
		c.platform.WindowsKit = &windowsKit
	}

	// No platform name, detect default platform.
	if c.Main.Platform == "" {
		switch runtime.GOOS {
		case "windows":
			if c.platform.Toolchain.Name == "msvc" || c.platform.Toolchain.Name == "clang" || c.platform.Toolchain.Name == "clang-cl" {
				c.platform.Name = "x86_64-windows"
			} else {
				return fmt.Errorf("unsupported toolchian %s", c.platform.Toolchain.Name)
			}

		case "linux":
			c.platform.Name = "x86_64-linux"

		default:
			return fmt.Errorf("unsupported operating system: %s", runtime.GOOS)
		}
	}

	if c.platform.Toolchain == nil {
		panic("Toolchain should not be empty, it may specified in platform or automatically detected.")
	}

	// Set global proxy.
	if c.configData.Proxy != nil {
		os.Setenv("all_proxy", fmt.Sprintf("http://%s:%d", c.configData.Proxy.Host, c.configData.Proxy.Port))
	} else {
		os.Unsetenv("all_proxy")
	}

	if c.Main.Offline {
		color.Printf(color.Warning, "\n================ WARNING: You're in offline mode currently! ================\n")
	}

	// Must init at the end of InitWithPlatform, because it depends on the celer fields.
	c.exprVars.Init(c)

	// Load project-level variables to exprVars.
	c.loadProjectVars()

	// Used to cache local dev artifacts.
	c.devCacheConfig = NewDevCacheConfig(c)

	// Clone ports repo if empty.
	if err := c.clonePorts(); err != nil {
		return err
	}

	return nil
}

func (c *Celer) Deploy(force, strip bool) error {
	if err := c.project.deploy(force, strip); err != nil {
		return fmt.Errorf("failed to deploy -> %w", err)
	}

	return nil
}

func (c *Celer) CreatePlatform(platformName string) error {
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

func (c *Celer) CreateProject(projectName string) error {
	// Clean platform name.
	projectName = strings.TrimSpace(projectName)
	projectName = strings.TrimSuffix(projectName, ".toml")

	if projectName == "" {
		return fmt.Errorf("project name is empty")
	}

	// Create project file.
	projectPath := filepath.Join(dirs.ConfProjectsDir, projectName+".toml")
	var project Project
	if err := project.Write(projectPath, false); err != nil {
		return err
	}

	return nil
}

func (c *Celer) CreatePort(nameVersion string) error {
	parts := strings.Split(nameVersion, "@")
	if len(parts) != 2 {
		return fmt.Errorf("invalid port name version")
	}

	portDir := dirs.GetPortDir(parts[0], parts[1])
	if err := os.MkdirAll(portDir, os.ModePerm); err != nil {
		return err
	}

	var port Port
	portPath := filepath.Join(portDir, "port.toml")
	if err := port.Write(portPath); err != nil {
		return err
	}
	return nil
}

func (c *Celer) CloneConf(url, branch string, force bool) error {
	if err := buildtools.CheckTools(c, "git"); err != nil {
		return err
	}

	confDir := filepath.Join(dirs.WorkspaceDir, "conf")
	if fileio.PathExists(confDir) {
		modified, err := git.IsModified(confDir)
		if err != nil {
			return err
		}
		if modified && !force {
			return fmt.Errorf("conf repo has local modifications, update is skipped ... 🚩 you can try with --force/-f 🚩")
		}

		if err := git.UpdateRepo("conf repo", branch, confDir, force); err != nil {
			return fmt.Errorf("update conf repo -> %w", err)
		}
	} else {
		// Clone conf repo.
		if err := git.CloneRepo("[clone conf repo]", "conf repo", url, branch, 0, confDir); err != nil {
			return fmt.Errorf("clone conf repo -> %w", err)
		}

		if err := c.readOrCreate(); err != nil {
			return err
		}

		c.Main.ConfRepo = url
		if err := c.save(); err != nil {
			return err
		}
	}

	return nil
}

func (c *Celer) portsRepoUrl() string {
	portsRepo := os.Getenv("CELER_PORTS_REPO")
	if portsRepo != "" {
		return portsRepo
	}

	return defaultPortsRepoUrl
}

func (c *Celer) clonePorts() error {
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
		if c.Main.Offline {
			return fmt.Errorf("offline is on, cloning ports is aborted")
		}

		portsRepoUrl := c.portsRepoUrl()
		if err := fileio.CheckAccessible(portsRepoUrl); err != nil {
			return fmt.Errorf("%s is not accessible, cloning ports is aborted", portsRepoUrl)
		}

		// Make sure git available.
		if err := buildtools.CheckTools(c, "git"); err != nil {
			return err
		}

		if err := git.CloneRepo("[clone ports]", "ports repo", portsRepoUrl, defaultPortRepoBranch, 0, portsDir); err != nil {
			return err
		}
	}

	return nil
}

// loadProjectVars loads project-level variables into global ExprVars.
func (c *Celer) loadProjectVars() {
	for _, item := range c.project.Vars {
		parts := strings.Split(item, "=")
		if len(parts) != 2 {
			continue
		}

		key := strings.TrimSpace(parts[0])
		if key == "" {
			continue
		}

		value := strings.TrimSpace(parts[1])
		value = strings.Trim(value, `"`)
		value = c.exprVars.Expand(value)
		c.exprVars.Put(key, value)
	}
}
