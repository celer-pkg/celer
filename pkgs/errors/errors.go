package errors

import (
	"errors"
)

var (
	ErrPortNotFound            = errors.New("port is not found")
	ErrNoMatchedConfigFound    = errors.New("no matched config found")
	ErrRepoNotExit             = errors.New("repo not exist")
	ErrInvalidBuildType        = errors.New("invalid build type, must be Release, Debug, RelWithDebInfo or MinSizeRel")
	ErrPackageCacheInvalid     = errors.New("package cache is invalid")
	ErrPackageCacheDirNotExist = errors.New("package cache dir not exist")
	ErrCacheNotFoundWithCommit = errors.New("package cache not found with commit")
)

// Is same as errors.Is
func Is(err, target error) bool {
	return errors.Is(err, target)
}

// New same as errors.New
func New(text string) error {
	return errors.New(text)
}
