package cmds

import (
	"fmt"
	"os"
	"os/user"
	"path/filepath"
	"runtime"
	"strings"

	"github.com/celer-pkg/celer/buildtools"
	"github.com/celer-pkg/celer/configs"
	"github.com/celer-pkg/celer/pkgs/cmd"
	"github.com/celer-pkg/celer/pkgs/color"
	"github.com/celer-pkg/celer/pkgs/fileio"

	"github.com/spf13/cobra"
)

type setupCmd struct {
	celer        *configs.Celer
	nfsServerDir string
	nfsClientDir string
}

func (s *setupCmd) Command(celer *configs.Celer) *cobra.Command {
	s.celer = celer
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
  5. Add current user to celer group.
  6. Add NFS export to /etc/exports and reload (exportfs -ra).
  7. Install cron job to keep new directories append-only (chattr +a every minute).

Client mode (--nfs-client-dir):
  Sets up an NFS client mount for the celer package cache.
  It must be run as root on the client machine. It performs:

  1. Install NFS client packages.
  2. Mount NFS on existing directory.
  3. Add entry to /etc/fstab for persistent mounting.
  4. Mount the NFS share.

Note: group membership takes effect after re-login or running: newgrp celer

Examples:
  sudo celer setup --nfs-server-dir=/srv/celer-cache
  sudo celer setup --nfs-client-dir=/home/phil/celer-cache@10.0.8.60:/mnt/data/celer-cache`,
		Args: cobra.NoArgs,
		RunE: func(cmd *cobra.Command, args []string) error {
			return s.doSetup()
		},
	}

	command.Flags().StringVar(&s.nfsServerDir, "nfs-server-dir", "", "path to the NFS cache directory on the server")
	command.Flags().StringVar(&s.nfsClientDir, "nfs-client-dir", "", "NFS client mount setup, format: <mount_point>@<server>:<export_path>")

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
		if err := s.doServerSetup(); err != nil {
			return err
		}
	} else if s.nfsClientDir != "" {
		if runtime.GOOS != "linux" {
			return fmt.Errorf("celer setup --nfs-client-dir is only supported on Linux")
		}
		if err := s.doClientSetup(); err != nil {
			return err
		}
	}

	return nil
}

func (s *setupCmd) doServerSetup() error {
	// Check required tools.
	if err := s.checkServerTools(); err != nil {
		return err
	}

	// Clean nfs dir.
	s.nfsServerDir = filepath.Clean(s.nfsServerDir)

	// Validate: dir must exist and be a directory.
	info, err := os.Stat(s.nfsServerDir)
	if err != nil {
		return fmt.Errorf("cannot access %s -> %w", s.nfsServerDir, err)
	}
	if !info.IsDir() {
		return fmt.Errorf("%s is not a directory", s.nfsServerDir)
	}

	// Step 0: Remove append-only attribute so we can modify directories.
	// A previous setup may have set chattr +a, which blocks chown/chmod.
	title := "[remove append-only attribute (chattr -a) from existing directories]"
	cmd.NewExecutor(title, "find", s.nfsServerDir, "-type", "d", "-exec", "chattr", "-a", "{}", ";").Execute()

	// Step 1: Create celer system user/group if not exists.
	if err := s.createCelerUser(); err != nil {
		return fmt.Errorf("failed to create celer user: %w", err)
	}

	// Step 2: chown -R celer:celer <nfs-dir>.
	title = "[set ownership to celer]"
	if err := cmd.NewExecutor(title, "chown", "-R", "celer:celer", s.nfsServerDir).Execute(); err != nil {
		return fmt.Errorf("chown failed -> %w\nHint: if this is an NFS mount, run this command on the NFS server", err)
	}

	// Step 3: chmod 2775 on directories (group writable + setgid), chmod 664 on files (group can overwrite in-place).
	// Setgid ensures new files/dirs inherit the celer group automatically.
	title = "[set directory permissions (chmod 2775)]"
	if err := cmd.NewExecutor(title, "find", s.nfsServerDir, "-type", "d", "-exec", "chmod", "2775", "{}", ";").Execute(); err != nil {
		return fmt.Errorf("chmod directories failed -> %w", err)
	}
	title = "[set file permissions (chmod 664)]"
	if err := cmd.NewExecutor(title, "find", s.nfsServerDir, "-type", "f", "-exec", "chmod", "664", "{}", ";").Execute(); err != nil {
		return fmt.Errorf("chmod files failed -> %w", err)
	}

	// Step 5: Add the invoking user to the celer group.
	s.addCurrentUserToCelerGroup()

	// Step 6: Add NFS export and reload.
	if err := s.addNFSExport(); err != nil {
		return fmt.Errorf("NFS export failed -> %w", err)
	}

	// Step 4: chattr +a on all directories (append-only: allows new files, blocks deletion).
	title = "[apply append-only attribute (chattr +a) to directories]"
	if err := cmd.NewExecutor(title, "find", s.nfsServerDir, "-type", "d", "-exec", "chattr", "+a", "{}", ";").Execute(); err != nil {
		return fmt.Errorf("chattr +a failed -> %w", err)
	}

	// Step 7: Install cron job to keep new directories append-only.
	if err := s.addChattrCron(); err != nil {
		color.PrintWarning("failed to install chattr cron job: %v\nnew directories won't be automatically protected", err)
	}

	color.PrintSuccess("NFS cache server setup complete for %q", s.nfsServerDir)
	return nil
}

func (s *setupCmd) doClientSetup() error {
	// Parse --nfs-client-dir
	mountPoint, serverExport, err := parseNfsClientDir(s.nfsClientDir)
	if err != nil {
		return err
	}

	//	Check if dir to mount NFS exist.
	mountPoint = filepath.Clean(mountPoint)
	if !fileio.PathExists(mountPoint) {
		return fmt.Errorf("directory to mount NFS is not exist: %s", mountPoint)
	}

	// Install NFS client packages.
	if err := s.checkClientTools(); err != nil {
		return err
	}

	// Unmount if already mounted (idempotent, ignore error).
	title := "[unmount existing mount (if any)]"
	cmd.NewExecutor(title, "umount", mountPoint).Execute()

	// Manage fstab entry (remove old, append new).
	fstabEntry := fmt.Sprintf("%s %s nfs rw,_netdev,noatime,rsize=1048576,wsize=1048576 0 0", serverExport, mountPoint)
	title = "[remove old fstab entry (if exists)]"
	cmd.NewExecutor(title, "sed", "-i", "\\|"+serverExport+"|d", "/etc/fstab").Execute()
	title = "[add fstab entry]"
	cmd.NewExecutor(title, "sh", "-c", "echo '"+fstabEntry+"' >> /etc/fstab").Execute()

	// Mount.
	title = "[mount NFS share]"
	if err := cmd.NewExecutor(title, "mount", mountPoint).Execute(); err != nil {
		return fmt.Errorf("mount failed -> %w", err)
	}

	// Create celer group and add current user to it (needed for group write access to NFS cache).
	if err := s.createCelerUser(); err != nil {
		color.PrintWarning("failed to create celer user: %v", err)
	}
	s.addCurrentUserToCelerGroup()

	color.PrintSuccess("NFS cache client setup complete: %s mounted from %s", mountPoint, serverExport)
	return nil
}

// parseNfsClientDir parses the --nfs-client-dir flag value.
// Expected format: <mount_point>@<server>:<export_path>
func parseNfsClientDir(val string) (mountPoint, serverExport string, err error) {
	parts := strings.SplitN(val, "@", 2)
	if len(parts) != 2 || parts[0] == "" || parts[1] == "" {
		return "", "", fmt.Errorf("--nfs-client-dir format: <mount_point>@<server>:<export_path>")
	}
	if !strings.Contains(parts[1], ":") {
		return "", "", fmt.Errorf("--nfs-client-dir: server export must be in <server>:<path> format")
	}
	return parts[0], parts[1], nil
}

// checkServerTools verifies that all system packages needed by server setup are installed.
func (s *setupCmd) checkServerTools() error {
	return buildtools.CheckSystemTools([]string{
		"apt:nfs-kernel-server", // NFS server
		"apt:passwd",            // provides useradd
		"yum:nfs-utils",         // NFS server
		"yum:shadow-utils",      // provides useradd
	})
}

// checkClientTools verifies that NFS client packages are installed.
func (s *setupCmd) checkClientTools() error {
	return buildtools.CheckSystemTools([]string{
		"apt:nfs-common",
		"yum:nfs-utils",
	})
}

// createCelerUser creates the celer system user/group if it does not already exist.
func (s *setupCmd) createCelerUser() error {
	// Check if user already exists (silent).
	if _, err := user.Lookup("celer"); err == nil {
		return nil
	}

	// Create system user (no login shell, no home directory).
	title := "[create system user: celer]"
	return cmd.NewExecutor(title, "useradd", "--system", "--no-create-home", "--shell", "/usr/sbin/nologin", "celer").Execute()
}

// addCurrentUserToCelerGroup adds the invoking user (SUDO_USER) to the celer group.
// usermod -aG is idempotent: adding an already-member user is a no-op.
func (s *setupCmd) addCurrentUserToCelerGroup() {
	username := os.Getenv("SUDO_USER")
	if username == "" {
		return
	}

	title := "[add current user to celer group]"
	if _, err := cmd.NewExecutor(title, "usermod", "-aG", "celer", username).ExecuteOutput(); err != nil {
		color.PrintWarning("failed to add %s to celer group: %s\nrun manually: sudo usermod -aG celer %s", username, err, username)
		return
	}

	color.PrintHint("%s", fmt.Sprintf("✔ %s added to celer group (💡 please re-login to take effect 💡)", username))
}

// addNFSExport ensures the NFS export entry exists in /etc/exports, then reloads.
func (s *setupCmd) addNFSExport() error {
	// rw: readable and writable
	// sync: synchronous writes for better integrity
	// no_subtree_check: avoid subtree checking which can cause permission issues
	// no_root_squash: allow root user on client to have root permissions on NFS (needed for celer's chattr +a)
	exportLine := s.nfsServerDir + " *(rw,sync,no_subtree_check,no_root_squash)"

	title := "[remove old NFS export entry (if exists)]"
	cmd.NewExecutor(title, "sed", "-i", "\\|^"+s.nfsServerDir+" |d", "/etc/exports").ExecuteOutput()

	title = "[add NFS export entry to /etc/exports]"
	cmd.NewExecutor(title, "sh", "-c", "echo '"+exportLine+"' >> /etc/exports").ExecuteOutput()

	title = "[export NFS exports (exportfs -ra)]"
	return cmd.NewExecutor(title, "exportfs", "-ra").Execute()
}

// addChattrCron installs a cron job that runs "find <nfs-dir> -type d -exec chattr +a"
// every minute, ensuring directories created by NFS clients are protected.
func (s *setupCmd) addChattrCron() error {
	// * * * * * 	- everytime
	// 2>/dev/null 	- execute quietly
	cronContent := fmt.Sprintf("* * * * * root /usr/bin/find %s -type d -exec /usr/bin/chattr +a {} + 2>/dev/null\n", s.nfsServerDir)
	cronPath := "/etc/cron.d/celer-chattr"

	if err := os.WriteFile(cronPath, []byte(cronContent), 0644); err != nil {
		return fmt.Errorf("failed to write cron file -> %w", err)
	}

	return nil
}
