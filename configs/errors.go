package configs

import (
	"celer/pkgs/fileio"
	"errors"
)

var (
	ErrOffline = fileio.ErrOffline

	ErrInvalidBuildType = errors.New("invalid build type, must be Release, Debug, RelWithDebInfo or MinSizeRel")
	ErrInvalidJobNum    = errors.New("invalid job num, must be greater than 0")

	ErrCacheDirNotConfigured   = errors.New("cache dir is not configured in celer.toml")
	ErrCacheDirNotExist        = errors.New("cache dir not exist")
	ErrCacheTokenNotConfigured = errors.New("cache token is not configured in celer.toml")
	ErrCacheTokenNotSpecified  = errors.New("cache token is not specified with `--cache-token`")
	ErrCacheTokenNotMatch      = errors.New(
		"cache tokens doesn't matched, please check `--cache-token` and `cache_dir.token` in celer.toml",
	)
)
