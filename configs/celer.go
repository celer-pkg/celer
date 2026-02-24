package configs

import (
	"celer/buildtools"
	"celer/context"
	"celer/envs"
	"celer/pkgs/color"
	"celer/pkgs/dirs"
	"celer/pkgs/errors"
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

const (
	defaultPortsRepoUrl   = "https://github.com/celer-pkg/ports.git"
	defaultPortRepoBranch = ""
)

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
	platform    Platform
	project     Project
	expressVars ExpressVars
}

type global struct {
	ConfRepo  string `toml:"conf_repo"`
	Platform  string `toml:"platform"`
	Project   string `toml:"project"`
	BuildType string `toml:"build_type"`
	Downloads string `toml:"downloads"`
	Jobs      int    `toml:"jobs"`
	Verbose   bool   `toml:"verbose"`
	Offline   bool   `toml:"offline"`
}

type Proxy struct {
	Host string `toml:"host"`
	Port int    `toml:"port"`
}

type configData struct {
	Global       global        `toml:"global"`
	Proxy        *Proxy        `toml:"proxy,omitempty"`
	PackageCache *PackageCache `toml:"package_cache,omitempty"`
	CCache       *CCache       `toml:"ccache,omitempty"`
}

// Init initializes celer with existing platform.
func (c *Celer) Init() error {
	return c.InitWithPlatform(c.configData.Global.Platform)
}

// InitWithPlatform initializes celer with platform.
func (c *Celer) InitWithPlatform(platform string) error {
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
			Downloads: filepath.Join(dirs.WorkspaceDir, "downloads"),
			Jobs:      runtime.NumCPU() - 1,
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
			c.configData.Global.Platform = platform
			if err := c.platform.Init(c.configData.Global.Platform); err != nil {
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

		// Use lower case build type in celer as default.
		c.Global.BuildType = strings.ToLower(c.Global.BuildType)

		// Set default downloads if missing.
		if c.Global.Downloads == "" {
			c.Global.Downloads = filepath.Join(dirs.WorkspaceDir, "downloads")
		}

		// Set platform and init platform if specified.
		if platform != "" {
			c.configData.Global.Platform = platform
		}
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

		// Validate package cache.
		if c.configData.PackageCache != nil {
			c.configData.PackageCache.ctx = c
			if err := c.configData.PackageCache.Validate(); err != nil {
				return err
			}
		}

		// Setup ccache.
		if c.configData.CCache != nil {
			if err := c.configData.CCache.Setup(); err != nil {
				return err
			}
		}

		// Save updated.
		if err := c.save(); err != nil {
			return err
		}
	}

	// Celer support detect local toolchain, if platform name is not specified, use default toolchain:
	// Windows: default is msvc,
	// Linux: default is gcc.
	if c.configData.Global.Platform == "" {
		var toolchain = Toolchain{ctx: c}
		if err := toolchain.Detect(c.configData.Global.Platform); err != nil {
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
	if c.configData.Global.Platform == "" {
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
		color.Printf(color.Warning, "\n================ WARNING: You're in offline mode currently! ================\n")
	}

	// Init express var.
	c.expressVars.Init(c)

	// Clone ports repo if empty.
	if err := c.clonePorts(); err != nil {
		return err
	}

	return nil
}

func (c *Celer) Deploy(force bool) error {
	if err := c.project.deploy(force); err != nil {
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
			return fmt.Errorf("conf repo has local modifications, update is skipped ... ⭐⭐⭐ you can try with --force/-f ⭐⭐⭐")
		}

		if err := git.UpdateRepo("[update conf repo]", branch, confDir, force); err != nil {
			return fmt.Errorf("update conf repo -> %w", err)
		}
	} else {
		// Clone conf repo.
		if err := git.CloneRepo("[clone conf repo]", url, branch, 0, confDir); err != nil {
			return fmt.Errorf("clone conf repo -> %w", err)
		}

		if err := c.readOrCreate(); err != nil {
			return err
		}

		c.Global.ConfRepo = url
		if err := c.save(); err != nil {
			return err
		}
	}

	return nil
}

func (c *Celer) SetBuildType(buildtype string) error {
	buildtype = strings.ToLower(buildtype)

	if buildtype != "release" && buildtype != "debug" && buildtype != "relwithdebinfo" && buildtype != "minsizerel" {
		return errors.ErrInvalidBuildType
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

func (c *Celer) SetDownloads(downloads string) error {
	if !fileio.PathExists(downloads) {
		return fmt.Errorf("downloads dir to configure is not exist for %s", downloads)
	}

	if err := c.readOrCreate(); err != nil {
		return err
	}

	c.configData.Global.Downloads = downloads
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

func (c *Celer) SetPackageCacheDir(dir string) error {
	// Check dir empty and exist.
	if strings.TrimSpace(dir) == "" {
		return errors.ErrPackageCacheInvalid
	}
	if !fileio.PathExists(dir) {
		return errors.ErrPackageCacheDirNotExist
	}

	if err := c.readOrCreate(); err != nil {
		return err
	}

	// Update package cache dir.
	if c.configData.PackageCache == nil {
		c.configData.PackageCache = &PackageCache{
			ctx:      c,
			Dir:      dir,
			Writable: false,
		}
	} else {
		c.configData.PackageCache.ctx = c
		c.configData.PackageCache.Dir = dir
	}
	if err := c.save(); err != nil {
		return err
	}

	return nil
}

func (c *Celer) SetPackageCacheWritable(writable bool) error {
	if err := c.readOrCreate(); err != nil {
		return err
	}

	// If cache dir is not configured, writable is useless.
	if c.configData.PackageCache == nil {
		return errors.ErrPackageCacheDirConfigured
	}

	// Update package cache wriable.
	if c.configData.PackageCache == nil {
		c.configData.PackageCache = &PackageCache{
			ctx:      c,
			Writable: writable,
		}
	} else {
		c.configData.PackageCache.ctx = c
		c.configData.PackageCache.Writable = writable
	}

	if err := c.save(); err != nil {
		return err
	}

	return nil
}

func (c *Celer) SetProxyHost(host string) error {
	if strings.TrimSpace(host) == "" {
		return fmt.Errorf("proxy host is invalid")
	}

	if err := c.readOrCreate(); err != nil {
		return err
	}

	if c.configData.Proxy == nil {
		c.configData.Proxy = &Proxy{
			Host: host,
			Port: 0,
		}
	} else {
		c.configData.Proxy.Host = host
	}

	if err := c.save(); err != nil {
		return err
	}

	return nil
}

func (c *Celer) SetProxyPort(port int) error {
	if port <= 0 {
		return fmt.Errorf("proxy port is invalid")
	}

	if err := c.readOrCreate(); err != nil {
		return err
	}

	if c.configData.Proxy == nil {
		c.configData.Proxy = &Proxy{
			Host: "",
			Port: port,
		}
	} else {
		c.configData.Proxy.Port = port
	}

	if err := c.save(); err != nil {
		return err
	}

	return nil
}

func (c *Celer) SetCCacheEnabled(enabled bool) error {
	if err := c.readOrCreate(); err != nil {
		return err
	}

	if c.configData.CCache == nil {
		c.configData.CCache = &CCache{}
		c.configData.CCache.init()
	}
	c.configData.CCache.Enabled = enabled

	if err := c.save(); err != nil {
		return err
	}

	return nil
}

func (c *Celer) SetCCacheDir(dir string) error {
	if !fileio.PathExists(dir) {
		return fmt.Errorf("ccache dir does not exist: %s", dir)
	}

	if err := c.readOrCreate(); err != nil {
		return err
	}

	if c.configData.CCache == nil {
		c.configData.CCache = &CCache{}
		c.configData.CCache.init()
	}
	c.configData.CCache.Dir = dir

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
		c.configData.CCache = &CCache{}
		c.configData.CCache.init()
	}
	c.configData.CCache.MaxSize = maxSize

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
			return fmt.Errorf("invalid remote storage URL -> %w", err)
		}
		if parsedURL.Scheme == "" || parsedURL.Host == "" {
			return fmt.Errorf("remote storage URL must contain scheme and host (e.g., http://server:port/path)")
		}
	}

	if c.configData.CCache == nil {
		c.configData.CCache = &CCache{}
		c.configData.CCache.init()
	}
	c.configData.CCache.RemoteStorage = remoteStorage

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
		c.configData.CCache = &CCache{}
		c.configData.CCache.init()
	}
	c.configData.CCache.RemoteOnly = remoteOnly

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

		if err := git.CloneRepo("[clone ports]", portsRepoUrl, defaultPortRepoBranch, 0, portsDir); err != nil {
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

func (c *Celer) Downloads() string {
	return c.configData.Global.Downloads
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

func (c *Celer) PackageCache() context.PackageCache {
	// Must return exactly nil if cache dir is none.
	// otherwise, the result of CacheDir() will not be nil.
	if c.configData.PackageCache == nil {
		return nil
	}

	return c.configData.PackageCache
}

func (c *Celer) Verbose() bool {
	return c.configData.Global.Verbose
}

func (c *Celer) InstalledDir() string {
	return filepath.Join(dirs.WorkspaceDir, "installed", c.Global.Platform+"@"+c.Global.Project+"@"+c.Global.BuildType)
}

func (c *Celer) InstalledDevDir() string {
	return filepath.Join(dirs.WorkspaceDir, "installed", c.Platform().GetHostName()+"-dev")
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
	return c.configData.CCache != nil && c.configData.CCache.Enabled
}

func (c *Celer) Vairables() map[string]string {
	return c.expressVars.vars
}
