package pkgcache

import (
	"fmt"
	"os"
	"path/filepath"
	"runtime"
	"strings"
	"syscall"
)

// WrapMkdirError enriches a mkdir failure with parent permissions and process identity.
func WrapMkdirError(path string, perm os.FileMode, err error) error {
	return wrapPathError("mkdir", path, perm, err)
}

// WrapWriteError enriches a write/create failure with parent permissions and process identity.
func WrapWriteError(path string, perm os.FileMode, err error) error {
	return wrapPathError("write", path, perm, err)
}

func wrapPathError(op, path string, perm os.FileMode, err error) error {
	if err == nil {
		return nil
	}

	var builder strings.Builder
	fmt.Fprintf(&builder, "%s %s (mode %o) -> %v", op, path, perm&07777, err)

	if info, statErr := os.Lstat(path); statErr == nil {
		entryType := "entry"
		switch {
		case info.IsDir():
			entryType = "directory"
		case info.Mode()&os.ModeSymlink != 0:
			entryType = "symlink"
		default:
			entryType = "file"
		}
		fmt.Fprintf(&builder, " -> target exists as %s (mode %o)", entryType, info.Mode().Perm())
	}

	parent := filepath.Dir(path)
	if parent != path {
		if info, statErr := os.Stat(parent); statErr != nil {
			fmt.Fprintf(&builder, " -> parent %s: stat failed -> %v", parent, statErr)
		} else {
			uid, gid := fileOwner(info)
			fmt.Fprintf(&builder, " -> parent %s: mode %o, uid=%d, gid=%d", parent, info.Mode().Perm(), uid, gid)
		}
	}

	fmt.Fprintf(&builder, " -> process uid=%d euid=%d gid=%d egid=%d groups=%s",
		os.Getuid(), os.Geteuid(), os.Getgid(), os.Getegid(), formatProcessGroups(),
	)

	return fmt.Errorf("%s", builder.String())
}

func fileOwner(info os.FileInfo) (uid, gid int) {
	switch st := info.Sys().(type) {
	case *syscall.Stat_t:
		return int(st.Uid), int(st.Gid)
	default:
		return -1, -1
	}
}

func formatProcessGroups() string {
	if runtime.GOOS == "windows" {
		return "n/a"
	}

	gids, err := syscall.Getgroups()
	if err != nil || len(gids) == 0 {
		return "unknown"
	}

	parts := make([]string, len(gids))
	for i, gid := range gids {
		parts[i] = fmt.Sprintf("%d", gid)
	}

	return strings.Join(parts, ",")
}
