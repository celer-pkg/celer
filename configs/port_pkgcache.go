package configs

import (
	"crypto/sha256"
	"fmt"
	"os"
	"path/filepath"
	"strings"
	"sync"

	"github.com/celer-pkg/celer/buildsystems"
	"github.com/celer-pkg/celer/pkgcache"
	"github.com/celer-pkg/celer/pkgs/errors"
	"github.com/celer-pkg/celer/pkgs/expr"
	"github.com/celer-pkg/celer/pkgs/fileio"
	"github.com/celer-pkg/celer/pkgs/git"

	"github.com/BurntSushi/toml"
)

// metaResult caches the result of buildMeta for a given port identity.
type metaResult struct {
	meta string
	err  error
}

// Caches keyed by "nameVersion|native". These are process-lifetime:
// within a single celer invocation, port.toml content and git commits don't
// change, so buildMeta/GenPortTomlString/GetCommitHash are pure functions of
// their arguments.
var (
	buildMetaCache     sync.Map // key: string -> metaResult
	portTomlCache      sync.Map // key: string -> metaResult
	commitHashCache    sync.Map // key: string -> metaResult
	buildConfigCache   sync.Map // key: string -> *pkgcache.BuildConfig
	hostSupportedCache sync.Map // key: string -> bool
)

// ResetMetaCache clears all metadata caches. Called at the start of each celer
// command to avoid stale data across invocations.
func ResetMetaCache() {
	buildMetaCache.Range(func(k, v any) bool {
		buildMetaCache.Delete(k)
		return true
	})
	portTomlCache.Range(func(k, v any) bool {
		portTomlCache.Delete(k)
		return true
	})
	commitHashCache.Range(func(k, v any) bool {
		commitHashCache.Delete(k)
		return true
	})
	buildConfigCache.Range(func(k, v any) bool {
		buildConfigCache.Delete(k)
		return true
	})
	hostSupportedCache.Range(func(k, v any) bool {
		hostSupportedCache.Delete(k)
		return true
	})

	// Also clear the pkgcache-level buildMeta cache (the recursive one inside
	// metadata.go that caches per nameVersion|native).
	pkgcache.ResetMetaCache()
}

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
	// Try find prebuilt meta from cache first.
	key := fmt.Sprintf("%s|%t", p.NameVersion(), p.DevDep || p.HostDep)
	if v, ok := buildMetaCache.Load(key); ok {
		r := v.(metaResult)
		return r.meta, r.err
	}

	// Computer meta and save into cache.
	platformName := expr.If(p.DevDep || p.HostDep, p.ctx.Platform().GetHostName(), p.ctx.Platform().GetName())
	port := pkgcache.Port{
		NameVersion: p.NameVersion(),
		Platform:    platformName,
		Project:     p.ctx.Project().GetName(),
		DevDep:      p.DevDep,
		HostDev:     p.HostDep,
		BuildConfig: p.toPkgCacheBuildConfig(p.MatchedConfig, p.portFile),
		Callbacks:   p,
	}

	// Don't cache ErrRepoNotExit — the repo may be cloned later in the same run.
	result, err := port.BuildMeta()
	if err == nil || !errors.Is(err, errors.ErrRepoNotExit) {
		buildMetaCache.Store(key, metaResult{meta: result, err: err})
	}
	return result, err
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
	// Checksum from caller affects the result, so include it in the cache key.
	key := fmt.Sprintf("%s|%t|%s", nameVersion, devDep, p.Package.Checksum)
	if v, ok := portTomlCache.Load(key); ok {
		r := v.(metaResult)
		return r.meta, r.err
	}

	// Store err if init port failed.
	var port = Port{DevDep: devDep}
	if err := port.Init(p.ctx, nameVersion); err != nil {
		portTomlCache.Store(key, metaResult{err: err})
		return "", err
	}

	// The build type is one of the key fields to identify a build config.
	matchedConfig := port.MatchedConfig
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
			// Don't cache ErrRepoNotExit — the repo may be cloned later.
			if !errors.Is(err, errors.ErrRepoNotExit) {
				portTomlCache.Store(key, metaResult{err: err})
			}
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
		portTomlCache.Store(key, metaResult{err: err})
		return "", fmt.Errorf("failed to marshal port %s -> %w", nameVersion, err)
	}
	result := string(bytes)
	portTomlCache.Store(key, metaResult{meta: result})
	return result, nil
}

// GetCommitHash try to get commit hash from `commitHashCache` first,
// if not exist then get it by calling `doGetCommitHash`.
func (p Port) GetCommitHash(nameVersion string, native bool) (string, error) {
	key := nameVersion
	if v, ok := commitHashCache.Load(key); ok {
		r := v.(metaResult)
		return r.meta, r.err
	}

	// Don't cache ErrRepoNotExit — the repo may be cloned later in the same run.
	result, err := p.doGetCommitHash(nameVersion, native)
	if err == nil || !errors.Is(err, errors.ErrRepoNotExit) {
		commitHashCache.Store(key, metaResult{meta: result, err: err})
	}
	return result, err
}

func (p Port) doGetCommitHash(nameVersion string, native bool) (string, error) {
	var port = Port{DevDep: native}
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

func (p Port) GetBuildConfig(nameVersion string, native bool) (*pkgcache.BuildConfig, error) {
	key := fmt.Sprintf("%s|%t", nameVersion, native)
	if v, ok := buildConfigCache.Load(key); ok {
		return v.(*pkgcache.BuildConfig), nil
	}

	var port = Port{DevDep: native, HostDep: native}
	if err := port.Init(p.ctx, nameVersion); err != nil {
		return nil, err
	}

	config := p.toPkgCacheBuildConfig(port.MatchedConfig, port.portFile)
	buildConfigCache.Store(key, &config)
	return &config, nil
}

// CheckHostSupported Host supported means that the port can be built natively.
func (p Port) CheckHostSupported(nameVersion string) bool {
	if v, ok := hostSupportedCache.Load(nameVersion); ok {
		return v.(bool)
	}

	var port = Port{DevDep: true}
	if err := port.Init(p.ctx, nameVersion); err != nil {
		return false
	}

	supported := port.IsHostSupported()
	hostSupportedCache.Store(nameVersion, supported)
	return supported
}

func (p Port) toPkgCacheBuildConfig(buildConfig *buildsystems.BuildConfig, portFile string) pkgcache.BuildConfig {
	return pkgcache.BuildConfig{
		Patches:         append([]string{}, buildConfig.Patches...),
		Dependencies:    append([]string{}, buildConfig.Dependencies...),
		DevDependencies: append([]string{}, buildConfig.DevDependencies...),
		BuildTools:      buildConfig.CheckTools(),
		PortFile:        portFile,
	}
}
