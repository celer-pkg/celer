package configs

import (
	"celer/buildsystems"
	"celer/caches"
	"celer/pkgs/dirs"
	"celer/pkgs/expr"
	"celer/pkgs/fileio"
	"celer/pkgs/git"
	"crypto/sha256"
	"fmt"
	"path/filepath"
	"strings"

	"github.com/BurntSushi/toml"
)

func (p Port) buildhash(commit string) (string, error) {
	metaInfo, err := p.buildMeta(commit)
	if err != nil {
		return "", err
	}

	return p.meta2hash(metaInfo), nil
}

func (p Port) meta2hash(metaInfo string) string {
	checksum := sha256.Sum256([]byte(metaInfo))
	return fmt.Sprintf("%x", checksum)
}

func (p Port) buildMeta(commit string) (string, error) {
	cachePort := caches.Port{
		NameVersion: p.NameVersion(),
		Platform:    p.ctx.Platform().Name,
		Project:     p.ctx.Project().Name,
		DevDep:      p.DevDep,
		BuildConfig: *p.MatchedConfig,
		Callbacks:   p,
	}

	return cachePort.BuildMeta(commit)
}

func (c Port) GenPlatformTomlString() (string, error) {
	bytes, err := toml.Marshal(c.ctx.Platform())
	if err != nil {
		return "", fmt.Errorf("marshal platform %s error: %w", c.ctx.Platform().Name, err)
	}
	return string(bytes), nil
}

func (p Port) GenPortTomlString(nameVersion string) (string, error) {
	var port Port
	if err := port.Init(p.ctx, nameVersion, p.buildType); err != nil {
		return "", err
	}

	matchedConfig := port.MatchedConfig
	port.BuildConfigs = []buildsystems.BuildConfig{*matchedConfig}

	bytes, err := toml.Marshal(port)
	if err != nil {
		return "", fmt.Errorf("marshal port %s error: %w", nameVersion, err)
	}
	return string(bytes), nil
}

func (p Port) Commit(nameVersion string) (string, error) {
	var port Port
	if err := port.Init(p.ctx, nameVersion, p.buildType); err != nil {
		return "", err
	}

	// Clone or download repo if not exist.
	if !fileio.PathExists(port.MatchedConfig.PortConfig.RepoDir) {
		if err := p.MatchedConfig.Clone(port.Package.Url, port.Package.Ref, p.Package.Archive); err != nil {
			message := expr.If(strings.HasSuffix(port.Package.Url, ".git"), "clone", "download")
			return "", fmt.Errorf("%s %s: %w", message, port.NameVersion(), err)
		}
	}

	// Get commit hash or archive checksum.
	if strings.HasSuffix(port.Package.Url, ".git") {
		commit, err := git.ReadLocalCommit(port.MatchedConfig.PortConfig.RepoDir)
		if err != nil {
			return "", fmt.Errorf("read git commit hash error: %w", err)
		}
		return commit, nil
	} else {
		archive := expr.If(port.Package.Archive != "", port.Package.Archive, filepath.Base(port.Package.Url))
		filePath := filepath.Join(dirs.DownloadedDir, archive)
		commit, err := fileio.CalculateChecksum(filePath)
		if err != nil {
			return "", fmt.Errorf("get checksum of part's archive %s error: %w", nameVersion, err)
		}
		return "file:" + commit, nil
	}
}

func (p Port) GetBuildConfig(nameVersion string) (*buildsystems.BuildConfig, error) {
	var port Port
	if err := port.Init(p.ctx, nameVersion, p.buildType); err != nil {
		return nil, err
	}
	return port.MatchedConfig, nil
}

func (p Port) CheckHostSupported(nameVersion string) bool {
	var port Port
	if err := port.Init(p.ctx, nameVersion, p.buildType); err != nil {
		return false
	}

	return port.IsHostSupported()
}
