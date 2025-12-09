package configs

import (
	"celer/buildtools"
	"celer/context"
	"celer/envs"
	"celer/pkgs/cmd"
	"celer/pkgs/color"
	"celer/pkgs/dirs"
	"celer/pkgs/encrypt"
	"celer/pkgs/expr"
	"celer/pkgs/fileio"
	"celer/pkgs/git"
	"fmt"
	"net/url"
	"os"
	"path/filepath"
	"runtime"
	"strings"

	"github.com/BurntSushi/toml"
)

const defaultPortsRepo = "https://github.com/celer-pkg/ports.git"

var Version = "v0.0.0" // It would be set by build script.

func NewCeler() *Celer {
	// Make sure build env is clean.
	envs.CleanEnv()

	return &Celer{
		configData: configData{
			Global: global{
				Jobs:      runtime.NumCPU() - 1,
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
	ConfRepo  string `toml:"conf_repo"`
	Platform  string `toml:"platform"`
	Project   string `toml:"project"`
	BuildType string `toml:"build_type"`
	Jobs      int    `toml:"jobs"`
	Verbose   bool   `toml:"verbose"`
	Offline   bool   `toml:"offline"`
}

type Proxy struct {
	Host string `toml:"host"`
	Port int    `toml:"port"`
}

type configData struct {
	Global      global       `toml:"global"`
	Proxy       *Proxy       `toml:"proxy,omitempty"`
	BinaryCache *BinaryCache `toml:"binary_cache,omitempty"`
	CCache      *CCache      `toml:"ccache,omitempty"`
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
			BuildType: "release",
			Jobs:      runtime.NumCPU() - 1,
			Offline:   false,
			Verbose:   false,
		}

		// Create celer conf file with default values.
		bytes, err := toml.Marshal(c)
		if err != nil {
			return fmt.Errorf("failed to marshal conf.\n %w", err)
		}

		if err := os.WriteFile(configPath, bytes, os.ModePerm); err != nil {
			return err
		}
	} else {
		// Read celer conf.
		bytes, err := os.ReadFile(configPath)
		if err != nil {
			return fmt.Errorf("failed to read conf.\n %w", err)
		}
		if err := toml.Unmarshal(bytes, c); err != nil {
			return fmt.Errorf("failed to unmarshal conf.\n %w", err)
		}

		// Use lower case build type in celer as default.
		c.Global.BuildType = strings.ToLower(c.Global.BuildType)

		// Init platform with platform name.
		if c.configData.Global.Platform != "" {
			if err := c.platform.Init(c.configData.Global.Platform); err != nil {
				return err
			}
		}

		// Init project with project name.
		if c.configData.Global.Project != "" {
			if err := c.project.Init(c, c.configData.Global.Project); err != nil {
				return err
			}
		}

		// Validate binary cache.
		if c.configData.BinaryCache != nil {
			c.configData.BinaryCache.ctx = c
			if err := c.configData.BinaryCache.Validate(); err != nil {
				return err
			}
		}

		// Validate ccache.
		if c.configData.CCache != nil {
			c.configData.CCache.ctx = c
			if err := c.configData.CCache.Validate(); err != nil {
				return err
			}
		}
	}

	// Celer support detect local toolchain, if platform name is not specified, use default toolchain:
	// Windows: default is msvc,
	// Linux: default is gcc.
	if c.platform.Name == "" {
		var toolchain = Toolchain{ctx: c}
		if err := toolchain.Detect(c.platform.Name); err != nil {
			return fmt.Errorf("detect celer.toolchain: %w", err)
		}
		c.platform.Toolchain = &toolchain
		c.platform.Toolchain.SystemName = expr.UpperFirst(runtime.GOOS)

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
			return fmt.Errorf("failed to detect celer.windows_kit.\n: %w", err)
		}
		c.platform.WindowsKit = &windowsKit
	}

	// No platform name, detect default platform.
	if c.platform.Name == "" {
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

	// Set default project name.
	if c.configData.Global.Project == "" {
		c.configData.Global.Project = "unnamed"
		c.project.Name = "unnamed"
	}

	// Set global proxy.
	if c.configData.Proxy != nil {
		os.Setenv("all_proxy", fmt.Sprintf("http://%s:%d", c.configData.Proxy.Host, c.configData.Proxy.Port))
	} else {
		os.Unsetenv("all_proxy")
	}

	if c.Global.Offline {
		color.Printf(color.Yellow, "\n================ WARNING: You're in offline mode currently! ================\n")
	}

	// Clone ports repo if empty.
	if err := c.clonePorts(); err != nil {
		return err
	}

	return nil
}

func (c *Celer) Setup() error {
	return c.platform.setup()
}

func (c *Celer) Deploy() error {
	if err := c.platform.setup(); err != nil {
		return err
	}
	if err := c.project.deploy(); err != nil {
		return err
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
	if err := project.Write(projectPath); err != nil {
		return err
	}

	return nil
}

func (c *Celer) CreatePort(nameVersion string) error {
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

func (c *Celer) CloneConf(url, branch string, force bool) error {
	// Remove existing conf repo if force is specified.
	confDir := filepath.Join(dirs.WorkspaceDir, "conf")
	if fileio.PathExists(confDir) {
		if !force {
			return fmt.Errorf("conf repo already exists, clone is skipped ... ⭐⭐⭐ you can use --force/-f to re-initialize it ⭐⭐⭐")
		}
		if err := os.RemoveAll(confDir); err != nil {
			return err
		}
	}

	// Clone conf repo.
	if err := buildtools.CheckTools(c, "git"); err != nil {
		return err
	}
	if err := git.CloneRepo("[clone conf repo]", url, branch, false, 0, confDir); err != nil {
		return fmt.Errorf("clone conf repo: %w", err)
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
	if jobs <= 0 {
		return fmt.Errorf("invalid jobs, must be greater than 0")
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

	// Init and setup.
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

func (c *Celer) SetBinaryCache(dir, token string) error {
	if err := c.readOrCreate(); err != nil {
		return err
	}

	if c.configData.BinaryCache != nil {
		dir = expr.If(dir != "", dir, c.configData.BinaryCache.Dir)
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
		bytes, err := encrypt.Encode(token)
		if err != nil {
			return fmt.Errorf("encode cache token: %w", err)
		}
		if err := os.WriteFile(tokenFile, bytes, os.ModePerm); err != nil {
			return fmt.Errorf("write cache token: %w", err)
		}
	}

	c.configData.BinaryCache = &BinaryCache{
		Dir: dir,
		ctx: c,
	}
	if err := c.save(); err != nil {
		return err
	}

	return nil
}

func (c *Celer) SetProxy(host string, port int) error {
	if strings.TrimSpace(host) == "" {
		return fmt.Errorf("proxy host is invalid")
	}
	if port <= 0 {
		return fmt.Errorf("proxy port is invalid")
	}

	if err := c.readOrCreate(); err != nil {
		return err
	}

	if c.configData.Proxy == nil {
		c.configData.Proxy = &Proxy{
			Host: host,
			Port: port,
		}
	} else {
		c.configData.Proxy.Host = host
		c.configData.Proxy.Port = port
		if err := c.save(); err != nil {
			return err
		}
	}

	return nil
}

func (c *Celer) SetCCacheEnabled(enabled bool) error {
	if err := c.readOrCreate(); err != nil {
		return err
	}

	if c.configData.CCache == nil {
		c.configData.CCache = &CCache{
			Enabled: enabled,
			MaxSize: ccacheDefaultMaxSize,
		}
	} else {
		c.configData.CCache.Enabled = enabled
	}

	if err := c.save(); err != nil {
		return err
	}

	return nil
}

func (c *Celer) SetCCacheDir(dir string) error {
	if err := c.readOrCreate(); err != nil {
		return err
	}

	if !fileio.PathExists(dir) {
		return fmt.Errorf("ccache dir does not exist: %s", dir)
	}

	if c.configData.CCache == nil {
		c.configData.CCache = &CCache{
			MaxSize: ccacheDefaultMaxSize,
			Dir:     dir,
		}
	} else {
		c.configData.CCache.Dir = dir
	}

	if err := c.save(); err != nil {
		return err
	}

	return nil
}

func (c *Celer) SetCCacheMaxSize(maxSize string) error {
	if maxSize == "" || (!strings.HasSuffix(maxSize, "M") && !strings.HasSuffix(maxSize, "G")) {
		return fmt.Errorf("ccache maxsize must end with `M` or `G`: %s", maxSize)
	}

	if err := c.readOrCreate(); err != nil {
		return err
	}

	if c.configData.CCache == nil {
		c.configData.CCache = &CCache{
			MaxSize: maxSize,
		}
	} else {
		c.configData.CCache.MaxSize = maxSize
	}

	if err := c.save(); err != nil {
		return err
	}

	return nil
}

func (c *Celer) SetCCacheRemoteStorage(remoteStorage string) error {
	if err := c.readOrCreate(); err != nil {
		return err
	}

	// Validate URL format if remote storage is provided
	if remoteStorage != "" {
		parsedURL, err := url.Parse(remoteStorage)
		if err != nil {
			return fmt.Errorf("invalid remote storage URL: %w", err)
		}
		if parsedURL.Scheme == "" || parsedURL.Host == "" {
			return fmt.Errorf("remote storage URL must include scheme and host (e.g., http://server:port/path)")
		}
	}

	if c.configData.CCache == nil {
		c.configData.CCache = &CCache{
			MaxSize:       ccacheDefaultMaxSize,
			RemoteStorage: remoteStorage,
		}
	} else {
		c.configData.CCache.RemoteStorage = remoteStorage
	}

	if err := c.save(); err != nil {
		return err
	}

	return nil
}

func (c *Celer) SetCCacheRemoteOnly(remoteOnly bool) error {
	if err := c.readOrCreate(); err != nil {
		return err
	}

	if c.configData.CCache == nil {
		c.configData.CCache = &CCache{
			MaxSize:    ccacheDefaultMaxSize,
			RemoteOnly: remoteOnly,
		}
	} else {
		c.configData.CCache.RemoteOnly = remoteOnly
	}

	if err := c.save(); err != nil {
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

func (c *Celer) save() error {
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

func (c *Celer) portsRepoUrl() string {
	portsRepo := os.Getenv("CELER_PORTS_REPO")
	if portsRepo != "" {
		return portsRepo
	}

	return defaultPortsRepo
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
		if c.Global.Offline {
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

		command := fmt.Sprintf("git clone %s %s", portsRepoUrl, portsDir)
		executor := cmd.NewExecutor("[clone ports]", command)
		if err := executor.Execute(); err != nil {
			return err
		}
	}

	return nil
}

// ======================= celer context implementation ====================== //

func (c *Celer) ProxyHostPort() (host string, port int) {
	if c.configData.Proxy != nil {
		return c.configData.Proxy.Host, c.configData.Proxy.Port
	}

	return "", 0
}

func (c *Celer) Version() string {
	return Version
}

func (c *Celer) Platform() context.Platform {
	return &c.platform
}

func (c *Celer) Project() context.Project {
	return &c.project
}

// BuildType returns lower case build type.
func (c *Celer) BuildType() string {
	return c.configData.Global.BuildType
}

func (c *Celer) RootFS() context.RootFS {
	// Must return exactly nil if RootFS is none.
	// otherwise, the result of RootFS() will not be nil.
	if c.platform.RootFS == nil {
		return nil
	}
	return c.platform.RootFS
}

func (c *Celer) Jobs() int {
	return c.Global.Jobs
}

func (c *Celer) Offline() bool {
	return c.Global.Offline
}

func (c *Celer) BinaryCache() context.BinaryCache {
	// Must return exactly nil if cache dir is none.
	// otherwise, the result of CacheDir() will not be nil.
	if c.configData.BinaryCache == nil {
		return nil
	}

	return c.configData.BinaryCache
}

func (c *Celer) Verbose() bool {
	return c.configData.Global.Verbose
}

func (c *Celer) Optimize(buildsystem, toolchain string) *context.Optimize {
	if c.project.Optimize != nil {
		return c.project.Optimize
	}

	switch toolchain {
	case "msvc", "clang-cl":
		if buildsystem == "makefiles" {
			return c.project.OptimizeGCC
		} else {
			return c.project.OptimizeMSVC
		}
	case "gcc":
		return c.project.OptimizeGCC
	case "clang":
		return c.project.OptimizeClang
	default:
		return c.project.Optimize
	}
}

func (c *Celer) CCacheEnabled() bool {
	return c.configData.CCache != nil && c.configData.CCache.Validate() == nil
}
