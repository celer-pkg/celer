package configs

import (
	"crypto/sha256"
	"fmt"
	"os"
	"path/filepath"
	"strings"

	"github.com/celer-pkg/celer/buildsystems"
	"github.com/celer-pkg/celer/pkgcache"
	"github.com/celer-pkg/celer/pkgs/errors"
	"github.com/celer-pkg/celer/pkgs/expr"
	"github.com/celer-pkg/celer/pkgs/fileio"
	"github.com/celer-pkg/celer/pkgs/git"

	"github.com/BurntSushi/toml"
)

func (p Port) buildhash() (string, error) {
	metaData, err := p.buildMeta()
	if err != nil {
		return "", err
	}

	return p.meta2hash(metaData), nil
}

func (p Port) meta2hash(metaData string) string {
	checksum := sha256.Sum256([]byte(metaData))
	return fmt.Sprintf("%x", checksum)
}

func (p Port) buildMeta() (string, error) {
	platformName := expr.If(p.DevDep || p.HostDep, p.ctx.Platform().GetHostName(), p.ctx.Platform().GetName())

	port := pkgcache.Port{
		NameVersion: p.NameVersion(),
		Platform:    platformName,
		Project:     p.ctx.Project().GetName(),
		DevDep:      p.DevDep,
		HostDev:     p.HostDep,
		BuildConfig: p.toCacheBuildConfig(p.MatchedConfig),
		Callbacks:   p,
	}

	return port.BuildMeta()
}

func (c Port) GenPlatformTomlString() (string, error) {
	if c.DevDep || c.HostDep {
		// Host/dev packages should describe the native host side instead of the
		// target cross toolchain/rootfs from the workspace platform config.
		bytes, err := toml.Marshal(struct {
			Name    string `toml:"name"`
			HostDev bool   `toml:"host_dev,omitempty"`
		}{
			Name:    c.ctx.Platform().GetHostName(),
			HostDev: true,
		})
		if err != nil {
			return "", fmt.Errorf("failed to marshal host platform %s -> %w", c.ctx.Platform().GetHostName(), err)
		}
		return string(bytes), nil
	}

	bytes, err := toml.Marshal(c.ctx.Platform())
	if err != nil {
		return "", fmt.Errorf("failed to marshal platform %s -> %w", c.ctx.Platform().GetName(), err)
	}
	return string(bytes), nil
}

func (p Port) GenPortTomlString(nameVersion string, devDep bool) (string, error) {
	var port = Port{DevDep: devDep}
	if err := port.Init(p.ctx, nameVersion); err != nil {
		return "", err
	}

	matchedConfig := port.MatchedConfig

	// The build type is one of the key fields to identify a build config.
	if matchedConfig.BuildType == "" {
		matchedConfig.BuildType = p.ctx.BuildType()
	}
	port.BuildConfigs = []buildsystems.BuildConfig{*matchedConfig}

	// Resolve the source to an immutable value for metadata.
	// If the caller's checksum differs from Init (e.g. tampered for cache miss test), use the caller's value.
	if p.Package.Checksum != "" {
		port.Package.Ref = p.Package.Checksum
	} else {
		commit, err := port.GetCommitHash(nameVersion, devDep)
		if err != nil {
			return "", err
		}
		if commit != "" {
			port.Package.Ref = commit
		}
	}
	port.Package.Checksum = ""
	port.Package.Depth = 0

	// Only export the matched build config for current platform.
	bytes, err := toml.Marshal(port)
	if err != nil {
		return "", fmt.Errorf("failed to marshal port %s -> %w", nameVersion, err)
	}
	return string(bytes), nil
}

func (p Port) GetCommitHash(nameVersion string, devDep bool) (string, error) {
	var port = Port{DevDep: devDep}
	if err := port.Init(p.ctx, nameVersion); err != nil {
		return "", err
	}

	// Check if repo cloned or downloaded.
	if !fileio.PathExists(port.MatchedConfig.PortConfig.RepoDir) {
		return "", errors.ErrRepoNotExit
	}

	// No commit hash for virtual project.
	if port.Package.Url == "_" {
		return "", nil
	}

	// Get commit hash or archive checksum.
	if strings.HasSuffix(port.Package.Url, ".git") {
		commit, err := git.GetCommitHash(port.MatchedConfig.PortConfig.RepoDir)
		if err != nil {
			return "", fmt.Errorf("failed to read git commit hash -> %w", err)
		}
		return commit, nil
	} else {
		// Get archive file path.
		var filePath string
		if after, ok := strings.CutPrefix(port.Package.Url, "file:///"); ok {
			filePath = after
		} else {
			archive := expr.If(port.Package.Archive != "", port.Package.Archive, filepath.Base(port.Package.Url))
			filePath = filepath.Join(p.ctx.Downloads(), archive)
		}

		// Auto-download source archive if missing, then continue checksum.
		if !fileio.PathExists(filePath) {
			if err := os.RemoveAll(port.MatchedConfig.PortConfig.RepoDir); err != nil {
				return "", err
			}
			archive := expr.If(port.Package.Archive != "", port.Package.Archive, filepath.Base(port.Package.Url))
			if err := port.MatchedConfig.Clone(
				port.Package.Url,
				port.Package.Ref,
				archive,
				port.Package.Depth,
			); err != nil {
				return "", fmt.Errorf("archive file is missing and auto-download failed for %s -> %w", nameVersion, err)
			}
		}

		// Calculate checksum of archive file.
		commit, err := fileio.ComputeSHA256(filePath)
		if err != nil {
			return "", fmt.Errorf("failed to get checksum of part's archive %s -> %w", nameVersion, err)
		}
		return commit, nil
	}
}

func (p Port) GetBuildConfig(nameVersion string, devDep bool) (*pkgcache.BuildConfig, error) {
	var port = Port{DevDep: devDep}
	if err := port.Init(p.ctx, nameVersion); err != nil {
		return nil, err
	}
	config := p.toCacheBuildConfig(port.MatchedConfig)
	return &config, nil
}

// CheckHostSupported Host supported means that the port can be built natively.
func (p Port) CheckHostSupported(nameVersion string) bool {
	var port = Port{DevDep: true}
	if err := port.Init(p.ctx, nameVersion); err != nil {
		return false
	}

	return port.IsHostSupported()
}

func (p Port) toCacheBuildConfig(buildConfig *buildsystems.BuildConfig) pkgcache.BuildConfig {
	return pkgcache.BuildConfig{
		Patches:         append([]string{}, buildConfig.Patches...),
		Dependencies:    append([]string{}, buildConfig.Dependencies...),
		DevDependencies: append([]string{}, buildConfig.DevDependencies...),
		BuildTools:      buildConfig.CheckTools(),
	}
}
