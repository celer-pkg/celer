package errors

import (
	"errors"
)

var (
	ErrNoMatchedConfigFound         = errors.New("no matched config found")
	ErrRepoNotExit                  = errors.New("repo not exist")
	ErrInvalidBuildType             = errors.New("invalid build type, must be Release, Debug, RelWithDebInfo or MinSizeRel")
	ErrPackageCacheDirNotConfigured = errors.New("package cache dir is not configured in celer.toml")
	ErrPackageCacheInvalid          = errors.New("package cache dir is invalid")
	ErrPackageCacheDirNotExist      = errors.New("package cache dir not exist")
	ErrPackageCacheTokenExists      = errors.New("package cache token already exist, if you want to change it, please remove it first manually")

	ErrPackageCacheTokenInvalid       = errors.New("package cache dir is invalid")
	ErrPackageCacheTokenNotConfigured = errors.New("package cache token is not configured in celer.toml")
	ErrPackageCacheTokenNotSpecified  = errors.New("package cache token is not specified with `--cache-token`")
	ErrPackageCacheTokenNotMatch      = errors.New(
		"package cache tokens doesn't matched, please check `--package-cache-token` and `token` in package cache dir",
	)
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
