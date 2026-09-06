package pkgcache

import (
	"fmt"
	"path/filepath"
	"strings"
)

// ========================== pkg-cache types ========================== //

// DirType identifies a cache subdirectory.
type DirType uint8

const (
	DirRoot DirType = iota
	DirRepos
	DirArtifacts
	DirDownloads
)

type FS struct {
	Dir string `toml:"dir"`
}

func (f FS) Validate() error {
	if strings.TrimSpace(f.Dir) == "" {
		return fmt.Errorf("empty dir for pkgcache.fs")
	}

	return nil
}

func (f FS) GetDir(dirType DirType, version string) string {
	switch dirType {
	case DirArtifacts:
		return filepath.Join(f.Dir, "artifacts-"+version)
	case DirRepos:
		return filepath.Join(f.Dir, "repos")
	case DirDownloads:
		return filepath.Join(f.Dir, "downloads")
	default:
		return f.Dir
	}
}

type Minio struct {
	Host      string `toml:"host"`
	AccessKey string `toml:"access_key"`
	SecretKey string `toml:"secret_key"`
}

func (m Minio) Validate() error {
	if strings.TrimSpace(m.Host) == "" {
		return fmt.Errorf("empty host for pkgcache.minio")
	}

	if strings.TrimSpace(m.AccessKey) == "" {
		return fmt.Errorf("empty access_key for pkgcache.minio")
	}

	if strings.TrimSpace(m.SecretKey) == "" {
		return fmt.Errorf("empty secret_key for pkgcache.minio")
	}

	return nil
}

func (m Minio) GetDir(dirType DirType, version string) string {
	switch dirType {
	case DirArtifacts:
		return "artifacts-" + version

	case DirRepos:
		return "repos"

	case DirDownloads:
		return "downloads"

	default:
		return ""
	}
}

// Options holds the pkgcache options shared by all backends: fs and minio
// are mutually exclusive, so these options live here instead of being
// duplicated per backend.
type Options struct {
	Writable  bool `toml:"writable"`
	Downloads bool `toml:"downloads"`
	Artifacts bool `toml:"artifacts"`
	Repos     bool `toml:"repos"`
}

// PkgCache is the shared (typically NFS/FTP) package cache: stores/restores
// source repos and built artifacts so repeat builds skip clone and compile.
type PkgCache interface {
	GetMinio() *Minio
	GetFS() *FS
	GetOptions() Options
	GetDownloadCache() DownloadCache
	GetArtifactCache() AritifactCache
	GetRepoCache() RepoCache
}

// AritifactCache stores/restores a port's built package, keyed by name@version + build hash.
type AritifactCache interface {
	Restore(packageDir, nameVersion, buildhash string) (bool, error)
	Store(packageDir, metadata string) error
}

// RepoCache stores/restores a port's source tree, keyed by name@version + checksum.
type RepoCache interface {
	Restore(repoDir, repoUrl, repoRef, nameVersion, checksum, archiveName string) (bool, error)
	Store(repoDir, repoUrl, repoRef, nameVersion, archiveFile string) error
}

// DownloadCache stores/restores downloaded files (tools, archives), keyed by SHA256.
type DownloadCache interface {
	Restore(fileName, sha256 string) (bool, error)
	Store(fileName, sha256, srcFile string) error
}

// ========================== dev cache ========================== //

// DevCache is the config for dev cache (under the user's home dir)
// for reusing built artifacts across workspaces.
type DevCache interface {
	GetDir() string
	GetDevArtifactCache() DevAritifactCache
}

// DevAritifactCache stores/restores a port's built package under user's home dir.
type DevAritifactCache interface {
	Restore(packageDir, nameVersion, buildhash string) (bool, error)
	Store(packageDir, metadata string) error
}
