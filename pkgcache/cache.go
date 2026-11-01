package pkgcache

import (
	"cmp"
	"fmt"
	"path/filepath"
	"runtime"
	"slices"
	"strings"

	"github.com/celer-pkg/celer/pkgs/expr"
)

type CacheConfigs struct {
	BinaryCache      BinaryCache
	DownloadCache    DownloadCache
	RepoCache        RepoCache
	AritifactCache   AritifactCache
	PythonWheelCache PythonWheelCache
}

// ========================== pkg-cache types ========================== //

// DirType identifies a cache subdirectory.
type DirType uint8

const (
	DirRoot DirType = iota
	DirBinaries
	DirRepos
	DirArtifacts
	DirDownloads
	DirPythonWheels
)

// FS is the config for the fs backend of pkgcache.
type FS struct {
	Dir string `toml:"dir" json:"dir"`
}

func (f FS) Validate() error {
	if strings.TrimSpace(f.Dir) == "" {
		return fmt.Errorf("empty dir for pkgcache.fs")
	}

	return nil
}

// GetDir returns the path for a cache subdir under the nfs root.
func (f FS) GetDir(dirType DirType, version string) string {
	switch dirType {
	case DirBinaries:
		return filepath.Join(f.Dir, "celers")

	case DirArtifacts:
		return filepath.Join(f.Dir, "artifacts-"+version)

	case DirRepos:
		return filepath.Join(f.Dir, "repos")

	case DirDownloads:
		return filepath.Join(f.Dir, "downloads")

	case DirPythonWheels:
		return filepath.Join(f.Dir, "python-wheels")

	default:
		return f.Dir
	}
}

// Minio is the config for the minio backend of pkgcache.
type Minio struct {
	Host      string `toml:"host" json:"host"`
	AccessKey string `toml:"access_key" json:"access_key"`
	SecretKey string `toml:"secret_key" json:"secret_key"`
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

// GetDir returns the object key prefix for a cache subdir. Unlike the fs
// backends these are not filesystem paths, so no filepath.Join here.
func (m Minio) GetDir(dirType DirType, version string) string {
	switch dirType {
	case DirBinaries:
		return "celers"

	case DirRepos:
		return "repos"

	case DirDownloads:
		return "downloads"

	case DirPythonWheels:
		return "python-wheels"

	case DirArtifacts:
		return "artifacts-" + version

	default:
		return ""
	}
}

// PkgCache is the shared package cache: stores/restores
// source repos and built artifacts so repeat builds skip clone and compile.
// The NFS/FTP backend is configured via FS, the minio backend via Minio; the
// two are mutually exclusive.
type PkgCache interface {
	GetMinio() *Minio
	GetFS() *FS
	IsWritable() bool
	GetKeepLatestCount() int
	GetBinaryCache() BinaryCache
	GetDownloadCache() DownloadCache
	GetArtifactCache() AritifactCache
	GetRepoCache() RepoCache
	GetPythonWheelCache() PythonWheelCache
}

// ========================== celer binary cache ========================== //

// BinaryCache stores the celer binary of a version, so a node runs the one that exported
// its schedule instead of whatever its PATH happens to have.
type BinaryCache interface {
	Restore(version string) (string, error)
	Store() error
}

// ========================== artifact cache ========================== //

// PrunedArtifact identifies an artifact removed by AritifactCache.Prune.
type PrunedArtifact struct {
	BuildHash string // build hash: the artifact file name without the .tar.gz suffix
	RepoRef   string // source ref from the meta: a git commit, or an archive sha256
}

// CommitTime resolves the git commit time of a source ref. It returns ok=false
// when the ref is not a resolvable commit in the local repo.
type CommitTime func(repoRef string) (unixTime int64, ok bool)

// PruneArtifact is one stored artifact, as seen by the prune policy.
type PruneArtifact struct {
	BuildHash   string // build hash: the artifact file name without the .tar.gz suffix
	CommitHash  string // source ref from the meta: a git commit, or an archive sha256
	SortTime    int64  // ordering key: commit time, or storage time for archives
	StorageTime int64  // when the artifact was stored
}

// FilterExpiredArtifacts applies the prune policy and returns the artifacts to remove.
//
// - Only one build config is valid per port, so artifacts built from the same
// source ref are one version, and only the newest artifact of a version matters.
//
// - keepCount counts versions instead of artifact files: everything beyond the
// newest keepCount versions is removed, including older artifacts of a kept version.
func FilterExpiredArtifacts(artifacts []PruneArtifact, keepCount int) []PruneArtifact {
	// Fewer artifacts than the count can never exceed it.
	if len(artifacts) <= keepCount {
		return nil
	}

	// Newest first: by commit time, then by storage time, so the newest artifact of
	// a version always comes before its older siblings.
	ordered := slices.Clone(artifacts)
	slices.SortFunc(ordered, func(x, y PruneArtifact) int {
		if byTime := cmp.Compare(y.SortTime, x.SortTime); byTime != 0 {
			return byTime
		}
		return cmp.Compare(y.StorageTime, x.StorageTime)
	})

	// Keep the newest artifact of the first keepCount versions; every other artifact
	// expires, including the older siblings of a kept version.
	keptVersions := make(map[string]bool, keepCount)
	var expired []PruneArtifact
	for _, artifact := range ordered {
		if !keptVersions[artifact.CommitHash] && len(keptVersions) < keepCount {
			keptVersions[artifact.CommitHash] = true
		} else {
			expired = append(expired, artifact)
		}
	}
	return expired
}

// AritifactCache stores/restores a port's built package, keyed by name@version + build hash.
type AritifactCache interface {
	Exists(nameVersion, buildhash string) (bool, error)
	Restore(packageDir, nameVersion, buildsystem, buildhash string) (bool, error)
	Store(packageDir, metadata string) error
	Prune(nameVersion string, keepCount int, commitTime CommitTime) error
}

// ========================== reopo cache ========================== //

// RepoInfo holds the repo identity fields shared by Restore and Store.
type RepoInfo struct {
	RepoDir     string
	RepoUrl     string
	RepoRef     string
	NameVersion string
	Checksum    string
}

type RepoCache interface {
	Restore(cacheKey string, repoInfo RepoInfo, archiveName string, prebuilt bool) (bool, error)
	Store(cacheKey string, repoInfo RepoInfo, archiveFile string) error
}

// ========================== download cache ========================== //

// DownloadCache stores/restores downloaded files (tools, archives), keyed by
// fileName and verified by sha-256.
type DownloadCache interface {
	Restore(fileName, sha256 string) (bool, error)
	Store(fileName, sha256, srcFile string) error
}

// ========================== python wheel cache ========================== //

type PythonWheelCache interface {
	Restore(cacheKey, destDir string) (ok bool, err error)
	Store(cacheKey, wheelhouseDir string) error
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

// IsPythonDistribution reports whether name is a Python distribution artifact
// that can be used as a pip find-links candidate: a wheel or a source archive.
func IsPythonDistribution(name string) bool {
	name = strings.ToLower(name)
	switch {
	case strings.HasSuffix(name, ".whl"),
		strings.HasSuffix(name, ".tar.gz"),
		strings.HasSuffix(name, ".tgz"),
		strings.HasSuffix(name, ".zip"),
		strings.HasSuffix(name, ".tar.bz2"),
		strings.HasSuffix(name, ".tar.xz"):
		return true
	default:
		return false
	}
}

func CelerBinaryName(version string) string {
	ext := ""
	if runtime.GOOS == "windows" {
		ext = ".exe"
	}

	return fmt.Sprintf("celer-%s-%s-%s%s",
		runtime.GOOS,
		expr.FormatProcessor(runtime.GOARCH),
		version,
		ext,
	)
}
