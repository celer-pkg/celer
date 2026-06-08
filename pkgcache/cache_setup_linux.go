//go:build linux

package pkgcache

import (
	"fmt"
	"os"
	"strconv"
	"syscall"
)

// mountedDirGID reads the numeric GID of a mounted directory.
func mountedDirGID(path string) (string, error) {
	info, err := os.Stat(path)
	if err != nil {
		return "", fmt.Errorf("failed to stat mounted NFS dir %q -> %w", path, err)
	}

	stat, ok := info.Sys().(*syscall.Stat_t)
	if !ok {
		return "", fmt.Errorf("failed to read numeric gid for mounted NFS dir %q", path)
	}

	return strconv.FormatUint(uint64(stat.Gid), 10), nil
}
