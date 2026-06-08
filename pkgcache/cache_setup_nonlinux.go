//go:build !linux

package pkgcache

import "fmt"

func mountedDirGID(path string) (string, error) {
	return "", fmt.Errorf("NFS client setup is only supported on Linux")
}
