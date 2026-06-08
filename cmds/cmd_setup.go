package cmds

import (
	"fmt"
	"os"
	"runtime"

	"github.com/celer-pkg/celer/configs"
	"github.com/celer-pkg/celer/pkgcache"

	"github.com/spf13/cobra"
)

type setupCmd struct {
	nfsServerDir string
	nfsClientDir string
	remove       bool
}

func (s *setupCmd) Command(celer *configs.Celer) *cobra.Command {
	command := &cobra.Command{
		Use:   "setup",
		Short: "One-time NFS cache setup (server or client, run with sudo).",
		Long: `One-time NFS cache setup for server or client.

Server mode (--nfs-server-dir):
  Prepares an NFS-shared directory for use as a celer package cache.
  It must be run as root on the NFS server. It performs:

  1. Create "celer" system user/group if it does not exist.
  2. chown -R celer:celer <nfs-dir>
  3. chmod 2775 on all directories (group writable + setgid), chmod 664 on all files (group can overwrite in-place).
  4. chattr +a on all directories (append-only: allows new files, blocks deletion).
  5. Add the invoking non-root user to celer group (uses SUDO_USER when run via sudo).
  6. Add NFS export to /etc/exports and reload (exportfs -ra).
  7. Install cron job to keep new directories append-only (chattr +a every minute).

Client mode (--nfs-client-dir):
  Sets up an NFS client mount for the celer package cache.
  It must be run as root on the client machine. It performs:

  1. Install NFS client packages.
  2. Mount the NFS share on the existing local directory.
  3. Validate that local celer group gid matches the mounted export gid (NFS sec=sys checks numeric gids).
  4. Add the invoking non-root user to celer group (uses SUDO_USER when run via sudo).
  5. Add entry to /etc/fstab for persistent mounting.

Note: group membership takes effect after re-login or running: newgrp celer

Examples:
  sudo celer setup --nfs-server-dir=/srv/celer-cache
  sudo celer setup --nfs-client-dir=/home/phil/celer-cache@10.0.8.60:/mnt/data/celer-cache
  sudo celer setup --nfs-server-dir=/srv/celer-cache --remove
  sudo celer setup --nfs-client-dir=/home/phil/celer-cache@10.0.8.60:/mnt/data/celer-cache --remove`,
		Args: cobra.NoArgs,
		RunE: func(cmd *cobra.Command, args []string) error {
			return s.doSetup()
		},
	}

	command.Flags().StringVar(&s.nfsServerDir, "nfs-server-dir", "", "path to the NFS cache directory on the server")
	command.Flags().StringVar(&s.nfsClientDir, "nfs-client-dir", "", "NFS client mount setup, format: <mount_point>@<server>:<export_path>")
	command.Flags().BoolVar(&s.remove, "remove", false, "remove NFS server/client setup configuration")

	command.SilenceErrors = true
	command.SilenceUsage = true
	return command
}

func (s *setupCmd) doSetup() error {
	if os.Geteuid() != 0 {
		return fmt.Errorf("celer setup must be run as root (use sudo)")
	}

	if s.nfsServerDir == "" && s.nfsClientDir == "" {
		return fmt.Errorf("at least one of --nfs-server-dir or --nfs-client-dir is required")
	}

	if s.nfsServerDir != "" {
		if runtime.GOOS != "linux" {
			return fmt.Errorf("celer setup --nfs-server-dir is only supported on Linux")
		}
		serverSetup := pkgcache.NewNFSServerSetup(s.nfsServerDir)
		if s.remove {
			return serverSetup.Remove()
		}
		if err := serverSetup.Setup(); err != nil {
			return err
		}
	} else if s.nfsClientDir != "" {
		if runtime.GOOS != "linux" {
			return fmt.Errorf("celer setup --nfs-client-dir is only supported on Linux")
		}
		clientSetup := pkgcache.NewNFSClientSetup(s.nfsClientDir)
		if s.remove {
			return clientSetup.Remove()
		}
		if err := clientSetup.Setup(); err != nil {
			return err
		}
	}

	return nil
}
