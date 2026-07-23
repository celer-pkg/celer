package pkgcache

import (
	"errors"
	"fmt"
	"os"
	"os/user"
	"path/filepath"
	"slices"
	"strconv"
	"syscall"
	"time"

	"github.com/celer-pkg/celer/context"
	"github.com/celer-pkg/celer/pkgs/fileio"
)

const writeProbeRel = ".celer-write-probe"

var getProcessGroups = syscall.Getgroups

type cacheSubdir struct {
	name string
	path string
}

// CheckWriteAccess verifies the current process can write to pkgcache subdirs.
// It is a no-op when pkgcache is nil, dir empty/missing, writable=false or offline.
func CheckWriteAccess(ctx context.Context) error {
	cacheConfig := ctx.PkgCacheConfig()
	if cacheConfig == nil || ctx.Offline() || !cacheConfig.IsWritable() {
		return nil
	}

	cacheDir := cacheConfig.GetDir(context.PkgCacheDirRoot)
	if cacheDir == "" || !fileio.PathExists(cacheDir) {
		return nil
	}

	// No need check GID for none NFS mounted dir.
	isNfsDir, err := IsNFSMount(cacheDir)
	if err != nil {
		return fmt.Errorf("failed to check if nfs dir for %s -> %w", cacheDir, err)
	}
	if !isNfsDir {
		return nil
	}

	if err := checkProcessInCelerGroup(); err != nil {
		return err
	}

	chattrFS := fileio.NewChattrFS(cacheDir)
	for _, subDir := range []cacheSubdir{
		{name: "repos", path: cacheConfig.GetDir(context.PkgCacheDirRepos)},
		{name: "downloads", path: cacheConfig.GetDir(context.PkgCacheDirDownloads)},
		{name: "artifacts", path: cacheConfig.GetDir(context.PkgCacheDirArtifacts)},
	} {
		if err := probeWrite(chattrFS, subDir.name, subDir.path); err != nil {
			return err
		}
	}

	return nil
}

func checkProcessInCelerGroup() error {
	group, err := lookupGroup(nfsUser)
	if err != nil {
		var unknown user.UnknownGroupError
		if errors.As(err, &unknown) {
			return fmt.Errorf(`failed to check pkgcache write permission -> %q group does not exist on this machine. run "sudo celer setup --nfs-client=..."`, nfsUser)
		}
		return fmt.Errorf("failed to check pkgcache write permission -> lookup %q group -> %w", nfsUser, err)
	}

	gid, err := strconv.Atoi(group.Gid)
	if err != nil {
		return fmt.Errorf("failed to check pkgcache write permission -> invalid gid %q for group %q -> %w", group.Gid, nfsUser, err)
	}

	groups, err := getProcessGroups()
	if err != nil {
		return fmt.Errorf("failed to check pkgcache write permission -> read process groups -> %w", err)
	}
	if !slices.Contains(groups, gid) {
		return fmt.Errorf(`failed to check pkgcache write permission -> process is not in group %q (gid=%d). run "newgrp %s" or re-login after setup`,
			nfsUser, gid, nfsUser)
	}

	return nil
}

func probeWrite(chattrFS *fileio.ChattrFS, subName, subDir string) error {
	probeFile := filepath.Join(subDir, fmt.Sprintf("%s-%d", writeProbeRel, time.Nanosecond))
	if err := chattrFS.MkdirAll(filepath.Dir(probeFile), fileio.CacheDirPerm); err != nil {
		return fmt.Errorf("failed to check pkgcache write permission (%s) -> %w", subName, err)
	}

	if err := chattrFS.WriteFile(probeFile, []byte("ok"), fileio.CacheFilePerm); err != nil {
		return fmt.Errorf("failed to check pkgcache write permission(%s) -> %w", subName, err)
	}

	os.RemoveAll(probeFile)
	return nil
}
