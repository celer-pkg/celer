package configs

import "errors"

var (
	ErrCacheDirNotConfigured   = errors.New("cache dir is not configured in celer.toml")
	ErrCacheTokenNotConfigured = errors.New("cache token is not configured in celer.toml")
	ErrCacheTokenNotSpecified  = errors.New("cache token is not specified with `--cache-token`")
	ErrCacheTokenNotMatch      = errors.New(
		"cache tokens doesn't matched, please check `--cache-token` and `cache_dir.token` in celer.toml",
	)
)
