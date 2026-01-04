package errors

import (
	"errors"
)

var (
	ErrNoMatchedConfigFound        = errors.New("no matched config found")
	ErrInvalidBuildType            = errors.New("invalid build type, must be Release, Debug, RelWithDebInfo or MinSizeRel")
	ErrBinaryCacheDirNotConfigured = errors.New("binary cache dir is not configured in celer.toml")
	ErrBinaryCacheInvalid          = errors.New("binary cache dir is invalid")
	ErrBinaryCacheDirNotExist      = errors.New("binary cache dir not exist")
	ErrBinaryCacheTokenExists      = errors.New("binary cache token already exist, if you want to change it, please remove it first manually")

	ErrBinaryCacheTokenInvalid       = errors.New("binary cache dir is invalid")
	ErrBinaryCacheTokenNotConfigured = errors.New("binary cache token is not configured in celer.toml")
	ErrBinaryCacheTokenNotSpecified  = errors.New("binary cache token is not specified with `--cache-token`")
	ErrBinaryCacheTokenNotMatch      = errors.New(
		"binary cache tokens doesn't matched, please check `--binary-cache-token` and `token` in binary cache dir",
	)
	ErrCacheNotFoundWithCommit = errors.New("binary cache not found with commit")
)

// Is same as errors.Is
func Is(err, target error) bool {
	return errors.Is(err, target)
}

// New same as errors.New
func New(text string) error {
	return errors.New(text)
}
