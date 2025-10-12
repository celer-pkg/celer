package configs

import (
	"errors"
)

var (
	ErrOffline = errors.New("currently, you're in offline mode")

	ErrInvalidBuildType = errors.New("invalid build type, must be Release, Debug, RelWithDebInfo or MinSizeRel")
	ErrInvalidJobs      = errors.New("invalid jobs, must be greater than 0")

	ErrCacheDirNotConfigured   = errors.New("cache dir is not configured in celer.toml")
	ErrCacheDirNotExist        = errors.New("cache dir not exist")
	ErrCacheTokenExist         = errors.New("cache token already exist, if you want to change it, please remove it first manually")
	ErrCacheTokenNotConfigured = errors.New("cache token is not configured in celer.toml")
	ErrCacheTokenNotSpecified  = errors.New("cache token is not specified with `--cache-token`")
	ErrCacheTokenNotMatch      = errors.New(
		"cache tokens doesn't matched, please check `--cache-token` and `cache_dir.token` in celer.toml",
	)
	ErrCacheNotFoundWithCommit = errors.New("cache not found with commit")

	ErrProxyInvalidHost = errors.New("proxy host is invalid")
	ErrProxyInvalidPort = errors.New("proxy port is invalid")
)
