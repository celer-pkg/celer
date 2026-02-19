package configs

import (
	"celer/buildsystems"
	"celer/packagecache"
	"celer/pkgs/errors"
	"celer/pkgs/expr"
	"celer/pkgs/fileio"
	"celer/pkgs/git"
	"crypto/sha256"
	"fmt"
	"os"
	"path/filepath"
	"strings"

	"github.com/BurntSushi/toml"
)

func (p Port) buildhash(commit string) (string, error) {
	metaData, err := p.buildMeta(commit)
	if err != nil {
		return "", err
	}

	return p.meta2hash(metaData), nil
}

func (p Port) meta2hash(metaData string) string {
	checksum := sha256.Sum256([]byte(metaData))
	return fmt.Sprintf("%x", checksum)
}

func (p Port) buildMeta(commit string) (string, error) {
	port := packagecache.Port{
		NameVersion: p.NameVersion(),
		Platform:    p.ctx.Platform().GetName(),
		Project:     p.ctx.Project().GetName(),
		DevDep:      p.DevDep,
		HostDev:     p.HostDep,
		BuildConfig: *p.MatchedConfig,
		Callbacks:   p,
	}

	return port.BuildMeta(commit)
}

func (c Port) GenPlatformTomlString() (string, error) {
	bytes, err := toml.Marshal(c.ctx.Platform())
	if err != nil {
		return "", fmt.Errorf("failed to marshal platform %s -> %w", c.ctx.Platform().GetName(), err)
	}
	return string(bytes), nil
}

func (p Port) GenPlatformChecksums() (toolchainChecksum, rootfsChecksum string, err error) {
	return p.ctx.Platform().GetArchiveChecksums()
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
		commit, err := git.ReadLocalCommit(port.MatchedConfig.PortConfig.RepoDir)
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
			if err := port.MatchedConfig.Clone(port.Package.Url, port.Package.Ref, archive, port.Package.Depth); err != nil {
				return "", fmt.Errorf("archive file is missing and auto-download failed for %s -> %w", nameVersion, err)
			}
		}

		// Calculate checksum of archive file.
		commit, err := fileio.CalculateChecksum(filePath)
		if err != nil {
			return "", fmt.Errorf("failed to get checksum of part's archive %s -> %w", nameVersion, err)
		}
		return "file:" + commit, nil
	}
}

func (p Port) GetBuildConfig(nameVersion string, devDep bool) (*buildsystems.BuildConfig, error) {
	var port = Port{DevDep: devDep}
	if err := port.Init(p.ctx, nameVersion); err != nil {
		return nil, err
	}
	return port.MatchedConfig, nil
}

// CheckHostSupported Host supported means that the port can be built natively.
func (p Port) CheckHostSupported(nameVersion string) bool {
	var port = Port{DevDep: true}
	if err := port.Init(p.ctx, nameVersion); err != nil {
		return false
	}

	return port.IsHostSupported()
}
