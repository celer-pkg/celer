package configs

import (
	"fmt"
	"net/url"
	"os"
	"path/filepath"
	"runtime"
	"strings"

	"github.com/BurntSushi/toml"
	"github.com/celer-pkg/celer/pkgs/dirs"
	"github.com/celer-pkg/celer/pkgs/errors"
	"github.com/celer-pkg/celer/pkgs/fileio"
)

func (c *Celer) readOrCreate() error {
	celerPath := filepath.Join(dirs.WorkspaceDir, "celer.toml")
	if !fileio.PathExists(celerPath) {
		// Create conf directory.
		if err := os.MkdirAll(filepath.Dir(celerPath), os.ModePerm); err != nil {
			return err
		}

		if c.Main.Jobs == 0 {
			c.Main.Jobs = runtime.NumCPU()
		}

		if c.Main.BuildType == "" {
			c.Main.BuildType = "release"
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

		if c.Main.Jobs == 0 {
			c.Main.Jobs = runtime.NumCPU()
		}

		if c.Main.BuildType == "" {
			c.Main.BuildType = "release"
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

func (c *Celer) SetBuildType(buildtype string) error {
	buildtype = strings.ToLower(buildtype)

	if buildtype != "release" && buildtype != "debug" && buildtype != "relwithdebinfo" && buildtype != "minsizerel" {
		return errors.ErrInvalidBuildType
	}

	if err := c.readOrCreate(); err != nil {
		return err
	}

	c.Main.BuildType = buildtype
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

	c.Main.Downloads = downloads
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

	c.Main.Jobs = jobs
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
	c.Main.Platform = platformName
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
	c.Main.Project = projectName

	if err := c.save(); err != nil {
		return err
	}

	return nil
}

func (c *Celer) SetOffline(offline bool) error {
	if err := c.readOrCreate(); err != nil {
		return err
	}

	c.Main.Offline = offline
	if err := c.save(); err != nil {
		return err
	}

	return nil
}

func (c *Celer) SetVerbose(vebose bool) error {
	if err := c.readOrCreate(); err != nil {
		return err
	}

	c.Main.Verbose = vebose
	if err := c.save(); err != nil {
		return err
	}

	return nil
}

func (c *Celer) SetPkgCacheDir(dir string) error {
	// Check dir empty and exist.
	if strings.TrimSpace(dir) == "" {
		return errors.ErrPkgCacheDirEmpty
	}
	if !fileio.PathExists(dir) {
		return errors.ErrPkgCacheDirNotExist
	}

	if err := c.readOrCreate(); err != nil {
		return err
	}

	// Update package cache dir.
	if c.configData.PkgCache == nil {
		c.configData.PkgCache = NewPkgCache(c, dir, false)
	}
	c.configData.PkgCache.Dir = dir

	// Rebuild internal handlers if dir is already configured.
	if err := c.configData.PkgCache.Refresh(); err != nil {
		return err
	}
	if err := c.save(); err != nil {
		return err
	}

	return nil
}

func (c *Celer) SetPkgCacheWritable(writable bool) error {
	if err := c.readOrCreate(); err != nil {
		return err
	}

	if c.configData.PkgCache == nil || strings.TrimSpace(c.configData.PkgCache.Dir) == "" {
		return errors.ErrPkgCacheDirEmpty
	}
	if !fileio.PathExists(c.configData.PkgCache.Dir) {
		return errors.ErrPkgCacheDirNotExist
	}

	// Update pkgcache wriatable.
	c.configData.PkgCache.Writable = writable
	if err := c.configData.PkgCache.Refresh(); err != nil {
		return err
	}
	if err := c.save(); err != nil {
		return err
	}

	return nil
}

func (c *Celer) CacheArtifacts(cacheArtifacts bool) error {
	if err := c.readOrCreate(); err != nil {
		return err
	}

	if c.configData.PkgCache == nil || strings.TrimSpace(c.configData.PkgCache.Dir) == "" {
		return errors.ErrPkgCacheDirEmpty
	}
	if !fileio.PathExists(c.configData.PkgCache.Dir) {
		return errors.ErrPkgCacheDirNotExist
	}

	// Update cacheArtifacts.
	c.configData.PkgCache.CacheArtifacts = cacheArtifacts
	if err := c.configData.PkgCache.Refresh(); err != nil {
		return err
	}
	if err := c.save(); err != nil {
		return err
	}

	return nil
}

func (c *Celer) CacheDownloads(cacheDownloads bool) error {
	if err := c.readOrCreate(); err != nil {
		return err
	}

	if c.configData.PkgCache == nil || strings.TrimSpace(c.configData.PkgCache.Dir) == "" {
		return errors.ErrPkgCacheDirEmpty
	}
	if !fileio.PathExists(c.configData.PkgCache.Dir) {
		return errors.ErrPkgCacheDirNotExist
	}

	// Update cachedownloads.
	c.configData.PkgCache.CacheDownloads = cacheDownloads
	if err := c.configData.PkgCache.Refresh(); err != nil {
		return err
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
