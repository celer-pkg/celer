package configs

import (
	"celer/buildsystems"
	"celer/buildtools"
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

func (p Port) buildhash() (string, error) {
	builddesc, err := p.builddesc()
	if err != nil {
		return "", err
	}

	return p.desc2hash(builddesc), nil
}

func (p Port) desc2hash(builddesc string) string {
	checksum := sha256.Sum256([]byte(builddesc))
	return fmt.Sprintf("%x", checksum)
}

func (p Port) builddesc() (string, error) {
	cachePort := caches.Port{
		NameVersion: p.NameVersion(),
		Platform:    p.ctx.Platform().Name,
		Project:     p.ctx.Project().Name,
		DevDep:      p.DevDep,
		BuildConfig: *p.MatchedConfig,
		Callbacks:   p,
	}

	return cachePort.BuildDesc()
}

func (c Port) GenPlatformTomlString() (string, error) {
	bytes, err := toml.Marshal(c.ctx.Platform())
	if err != nil {
		return "", fmt.Errorf("marshal platform %s: %w", c.ctx.Platform().Name, err)
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
		return "", fmt.Errorf("marshal port %s: %w", nameVersion, err)
	}
	return string(bytes), nil
}

func (p Port) Commit(nameVersion string) (string, error) {
	var port Port
	if err := port.Init(p.ctx, nameVersion, p.buildType); err != nil {
		return "", err
	}

	// Return git commit id for git repo and calculate checksum for archive file.
	if strings.HasSuffix(port.Package.Url, ".git") {
		srcDir := filepath.Join(dirs.BuildtreesDir, nameVersion, "src")
		if err := buildtools.CheckTools("git"); err != nil {
			return "", fmt.Errorf("check git: %w", err)
		}

		commit, err := git.ReadCommit(srcDir)
		if err != nil {
			return "", fmt.Errorf("get git commit of port %s: %w", nameVersion, err)
		}
		return "git:" + commit, nil
	} else {
		fileName := expr.If(port.Package.Archive != "", port.Package.Archive, filepath.Base(port.Package.Url))
		filePath := filepath.Join(dirs.DownloadedDir, fileName)
		commit, err := fileio.CalculateChecksum(filePath)
		if err != nil {
			return "", fmt.Errorf("get checksum of part's archive %s: %w", nameVersion, err)
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
