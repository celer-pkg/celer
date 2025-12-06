package configs

import (
	"errors"
)

var (
	ErrInvalidBuildType        = errors.New("invalid build type, must be Release, Debug, RelWithDebInfo or MinSizeRel")
	ErrCacheDirNotConfigured   = errors.New("binary cache dir is not configured in celer.toml")
	ErrCacheDirNotExist        = errors.New("binary cache dir not exist")
	ErrCacheTokenExist         = errors.New("binary cache token already exist, if you want to change it, please remove it first manually")
	ErrCacheTokenNotConfigured = errors.New("binary cache token is not configured in celer.toml")
	ErrCacheTokenNotSpecified  = errors.New("binary cache token is not specified with `--cache-token`")
	ErrCacheTokenNotMatch      = errors.New(
		"binary cache tokens doesn't matched, please check `--binary-cache-token` and `token` in binary cache dir",
	)
	ErrCacheNotFoundWithCommit = errors.New("binary cache not found with commit")
)
