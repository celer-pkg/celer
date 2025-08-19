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
	"os"
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

	// Git port.
	if strings.HasSuffix(port.Package.Url, ".git") {
		if err := buildtools.CheckTools("git"); err != nil {
			return "", fmt.Errorf("check git error: %w", err)
		}

		// Clone git repo if not exists.
		if !fileio.PathExists(port.MatchedConfig.PortConfig.RepoDir) {
			if err := git.CloneRepo("[clone "+nameVersion+"]",
				port.Package.Url, port.Package.Ref,
				port.MatchedConfig.PortConfig.RepoDir); err != nil {
				return "", fmt.Errorf("clone git repo error: %w", err)
			}
		}

		commit, err := git.ReadLocalCommit(port.MatchedConfig.PortConfig.RepoDir)
		if err != nil {
			return "", fmt.Errorf("read git commit hash error: %w", err)
		}

		return commit, nil
	} else { // Non-git port.
		archive := expr.If(port.Package.Archive != "", port.Package.Archive, filepath.Base(port.Package.Url))
		filePath := filepath.Join(dirs.DownloadedDir, archive)

		// Check and repair resource.
		repair := fileio.NewRepair(port.Package.Url, archive, ".", dirs.TmpFilesDir)
		if err := repair.CheckAndRepair(); err != nil {
			return "", err
		}
		if repair.Repaired {
			// Remove repor dir.
			if err := os.RemoveAll(port.MatchedConfig.PortConfig.RepoDir); err != nil {
				return "", fmt.Errorf("remove repo dir error: %w", err)
			}

			// Move extracted files to source dir.
			entities, err := os.ReadDir(dirs.TmpFilesDir)
			if err != nil || len(entities) == 0 {
				return "", fmt.Errorf("cannot find extracted files under tmp dir")
			}
			if len(entities) == 1 {
				srcDir := filepath.Join(dirs.TmpFilesDir, entities[0].Name())
				if err := fileio.RenameDir(srcDir, port.MatchedConfig.PortConfig.RepoDir); err != nil {
					return "", err
				}
			} else if len(entities) > 1 {
				if err := fileio.RenameDir(dirs.TmpFilesDir, port.MatchedConfig.PortConfig.RepoDir); err != nil {
					return "", err
				}
			}

			// Init as git repo for tracking file change.
			if err := git.InitRepo(port.MatchedConfig.PortConfig.RepoDir, "init for tracking file change"); err != nil {
				return "", err
			}
		}

		// Return the checksum of archive file.
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
