//go:build windows

package pkgcache

import (
	"fmt"
	"strings"

	"golang.org/x/sys/windows"
)

func mountedDirGID(_ string) (string, error) {
	return "", fmt.Errorf("NFS client setup is only supported on Linux")
}

// IsNFSMount check mounted dir is nfs type.
func IsNFSMount(path string) (bool, error) {
	// Get root path.
	rootPath := getRootPath(path)
	if rootPath == "" {
		return false, fmt.Errorf("invalid windows path: %s", path)
	}

	rootPtr, err := windows.UTF16PtrFromString(rootPath)
	if err != nil {
		return false, err
	}

	var fileSystemName [256]uint16
	err = windows.GetVolumeInformation(
		rootPtr,
		nil,
		0,
		nil,
		nil,
		nil,
		&fileSystemName[0],
		uint32(len(fileSystemName)),
	)

	if err != nil {
		return false, fmt.Errorf("failed to get mounted info: %v", err)
	}

	fsName := windows.UTF16ToString(fileSystemName[:])

	return strings.EqualFold(fsName, "NFS") ||
		strings.EqualFold(fsName, "Network File System"), nil
}

func getRootPath(path string) string {
	if len(path) >= 2 {
		if path[1] == ':' {
			if len(path) >= 3 && (path[2] == '\\' || path[2] == '/') {
				return path[:3]
			}
			return path[:2] + "\\"
		}
	}
	return ""
}
