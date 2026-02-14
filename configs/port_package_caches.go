package configs

import (
	"celer/buildsystems"
	"celer/packagecache"
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
	cachePort := packagecache.Port{
		NameVersion: p.NameVersion(),
		Platform:    p.ctx.Platform().GetName(),
		Project:     p.ctx.Project().GetName(),
		DevDep:      p.DevDep,
		Native:      p.Native || p.DevDep,
		BuildConfig: *p.MatchedConfig,
		Callbacks:   p,
	}

	return cachePort.BuildMeta(Version, commit)
}

func (c Port) GenPlatformTomlString() (string, error) {
	bytes, err := toml.Marshal(c.ctx.Platform())
	if err != nil {
		return "", fmt.Errorf("failed to marshal platform %s.\n %w", c.ctx.Platform().GetName(), err)
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
		return "", fmt.Errorf("failed to marshal port %s.\n %w", nameVersion, err)
	}
	return string(bytes), nil
}

func (p Port) Commit(nameVersion string, devDep bool) (string, error) {
	var port = Port{DevDep: devDep}
	if err := port.Init(p.ctx, nameVersion); err != nil {
		return "", err
	}

	// Clone or download repo if not exist.
	if !fileio.PathExists(port.MatchedConfig.PortConfig.RepoDir) {
		if err := port.MatchedConfig.Clone(
			port.Package.Url,
			port.Package.Ref,
			port.Package.Archive,
			port.Package.Depth); err != nil {
			message := expr.If(strings.HasSuffix(port.Package.Url, ".git"), "clone", "download")
			return "", fmt.Errorf("%s %s: %w", message, port.NameVersion(), err)
		}
	}

	// No commit hash for virtual project.
	if port.Package.Url == "_" {
		return "", nil
	}

	// Get commit hash or archive checksum.
	if strings.HasSuffix(port.Package.Url, ".git") {
		commit, err := git.ReadLocalCommit(port.MatchedConfig.PortConfig.RepoDir)
		if err != nil {
			return "", fmt.Errorf("failed to read git commit hash.\n %w", err)
		}
		return commit, nil
	} else {
		// Get archive file path.
		var filePath string
		if after, ok := strings.CutPrefix(port.Package.Url, "file:///"); ok {
			filePath = after
		} else {
			archive := expr.If(port.Package.Archive != "", port.Package.Archive, filepath.Base(port.Package.Url))
			filePath = filepath.Join(dirs.DownloadedDir, archive)
		}

		// Check if archive file exists.
		if !fileio.PathExists(filePath) {
			return "", fmt.Errorf("archive file does not exist: %s \n "+
				"please remove %s under buildtrees and try again", nameVersion, nameVersion)
		}

		// Calculate checksum of archive file.
		commit, err := fileio.CalculateChecksum(filePath)
		if err != nil {
			return "", fmt.Errorf("failed to get checksum of part's archive %s.\n %w", nameVersion, err)
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
