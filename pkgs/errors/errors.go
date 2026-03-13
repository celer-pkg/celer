package errors

import (
	"errors"
)

var (
	ErrPortNotFound             = errors.New("port is not found")
	ErrNoMatchedConfigFound     = errors.New("no matched config found")
	ErrRepoNotExit              = errors.New("repo not exist")
	ErrInvalidBuildType         = errors.New("invalid build type, must be Release, Debug, RelWithDebInfo or MinSizeRel")
	ErrPkgCacheDirEmpty         = errors.New("pkgcache dir is invalid")
	ErrPkgCacheDirNotExist      = errors.New("pkgcache dir not exist")
	ErrPkgCacheArtifactNotFound = errors.New("artifact cache not found with commit hash")
)

// Is same as errors.Is
func Is(err, target error) bool {
	return errors.Is(err, target)
}

// New same as errors.New
func New(text string) error {
	return errors.New(text)
}
