package configs

import (
	"path/filepath"

	"github.com/celer-pkg/celer/context"
	"github.com/celer-pkg/celer/pkgs/dirs"
	"github.com/celer-pkg/celer/pkgs/expr"
)

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
	return c.Main.BuildType
}

func (c *Celer) LibraryFolder() string {
	return filepath.Join(c.platform.Name, c.project.Name, c.Main.BuildType)
}

func (c *Celer) Downloads() string {
	return expr.If(c.Main.Downloads != "", c.Main.Downloads, dirs.DownloadsDir)
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
	return c.Main.Jobs
}

func (c *Celer) Offline() bool {
	return c.Main.Offline
}

func (c *Celer) PkgCacheConfig() context.PkgCacheConfig {
	if c.configData.PkgCacheConfig == nil {
		return nil
	}

	return c.configData.PkgCacheConfig
}

func (c *Celer) DevCacheConfig() context.DevCacheConfig {
	return c.devCacheConfig
}

func (c *Celer) Verbose() bool {
	return c.Main.Verbose
}

func (c *Celer) InstalledDir() string {
	libraryDir := filepath.Join(c.Main.Platform, c.Main.Project, c.Main.BuildType)
	return filepath.Join(dirs.WorkspaceDir, "installed", libraryDir)
}

func (c *Celer) InstalledDevDir() string {
	return filepath.Join(dirs.WorkspaceDir, "installed", c.Platform().GetHostName()+"-dev")
}

func (c *Celer) CCacheEnabled() bool {
	return c.configData.CCache != nil && c.configData.CCache.Enabled
}

func (c *Celer) ExprVars() *context.ExprVars {
	return &c.exprVars
}

func (c *Celer) PythonConfig() context.PythonConfig {
	if c.configData.Python != nil {
		return c.configData.Python
	}
	return nil
}

func (c *Celer) Experiment() context.Experiment {
	if c.configData.Experiment != nil {
		return c.configData.Experiment
	}

	return nil
}
