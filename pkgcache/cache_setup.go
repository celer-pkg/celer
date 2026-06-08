package pkgcache

import (
	"errors"
	"fmt"
	"os"
	"os/user"
	"path/filepath"
	"slices"
	"strings"

	"github.com/celer-pkg/celer/buildtools"
	"github.com/celer-pkg/celer/pkgs/cmd"
	"github.com/celer-pkg/celer/pkgs/color"
	"github.com/celer-pkg/celer/pkgs/fileio"
)

const nfsUser = "celer"

var (
	nfsExportsPath = "/etc/exports"
	nfsFSTabPath   = "/etc/fstab"
	nfsCronDir     = "/etc/cron.d"

	lookupUser    = user.Lookup
	lookupGroup   = user.LookupGroup
	lookupGroupID = user.LookupGroupId
)

type NFSServerSetup struct {
	nfsDir string
}

func NewNFSServerSetup(nfsDir string) *NFSServerSetup {
	return &NFSServerSetup{nfsDir: nfsDir}
}

func (n *NFSServerSetup) Setup() error {
	// Check required tools.
	if err := checkServerTools(); err != nil {
		return err
	}

	// Clean nfs dir.
	n.nfsDir = filepath.Clean(n.nfsDir)

	// Validate: Dir must exist and be a directory.
	if err := n.validateDir(); err != nil {
		return err
	}

	// Step 0: Remove append-only attribute so we can modify directories.
	// A previous setup may have set chattr +a, which blocks chown/chmod.
	if err := n.removeAppendOnlyAttribute(); err != nil {
		return err
	}

	// Step 1: Create celer system user/group if not exists.
	if err := createSystemGroupAndUser(nfsUser, ""); err != nil {
		return err
	}

	// Step 2: Chown -R celer:celer <nfs-dir>.
	if err := n.changeDirOwnership(nfsUser); err != nil {
		return err
	}

	// Step 3: Chmod 02775 on directories (group writable + setgid), chmod 0664 on files.
	if err := n.changeModeForDirsFiles(); err != nil {
		return err
	}

	// Step 5: Add the invoking user to the celer group.
	if err := addCurrentUserToGroup(nfsUser); err != nil {
		return err
	}

	// Step 6: Export NFS config and reload.
	if err := n.exportNFSConfig(); err != nil {
		return err
	}

	// Step 4: Chattr +a on all directories (append-only: allows new files, blocks deletion).
	if err := n.assignAppendOnlyAttribute(); err != nil {
		return err
	}

	// Step 7: Install cron job to keep new directories with append-only attribute.
	if err := n.installChattrCronJob(nfsUser); err != nil {
		return err
	}

	fmt.Println() // Just a blank line.
	color.PrintSuccess("NFS cache server setup complete for %q", n.nfsDir)
	return nil
}

func (n *NFSServerSetup) Remove() error {
	n.nfsDir = filepath.Clean(n.nfsDir)

	if fileio.PathExists(n.nfsDir) {
		if err := n.removeAppendOnlyAttribute(); err != nil {
			return err
		}
	}

	if err := n.removeNFSExportEntry(); err != nil {
		return err
	}
	if err := n.removeChattrCronJob(nfsUser); err != nil {
		return err
	}
	if err := n.reloadNFSExports(); err != nil {
		return err
	}
	if err := removeSystemGroupAndUser(nfsUser); err != nil {
		return err
	}

	fmt.Println() // Just a blank line.
	color.PrintSuccess("NFS cache server setup removed for %q", n.nfsDir)
	return nil
}

type NFSClientSetup struct {
	nfsClientDir string
}

func NewNFSClientSetup(nfsClientDir string) *NFSClientSetup {
	return &NFSClientSetup{nfsClientDir: nfsClientDir}
}

func (n *NFSClientSetup) Setup() error {
	// Parse --nfs-client-dir
	mountPoint, serverExport, err := parseNFSClientDir(n.nfsClientDir)
	if err != nil {
		return err
	}

	//	Check if dir to mount NFS exist.
	mountPoint = filepath.Clean(mountPoint)
	if !fileio.PathExists(mountPoint) {
		return fmt.Errorf("directory to mount NFS is not exist: %s", mountPoint)
	}

	// Install NFS client packages.
	if err := checkClientTools(); err != nil {
		return err
	}

	// Unmount if already mounted.
	if err := n.unmountExisting(mountPoint); err != nil {
		return err
	}

	// Mount NFS dir first to validate the server export and detect its numeric GID.
	if err := n.mountNFSDir(serverExport, mountPoint); err != nil {
		return err
	}

	// Read GID of mounted dir.
	mountedGID, err := mountedDirGID(mountPoint)
	if err != nil {
		return err
	}

	// Create celer group/user with the mounted export's numeric GID.
	// NFS sec=sys checks numeric IDs, not group names.
	if err := createSystemGroupAndUser(nfsUser, mountedGID); err != nil {
		return err
	}

	// Add the current user to the celer group.
	if err := addCurrentUserToGroup(nfsUser); err != nil {
		return err
	}

	// Manage fstab entry only after the mounted share is known to be usable.
	if err := n.updateFSTabEntry(serverExport, mountPoint); err != nil {
		return err
	}

	fmt.Println() // Just a blank line.
	color.PrintSuccess("NFS cache client setup complete: %q mounted from %q", mountPoint, serverExport)
	return nil
}

func (n *NFSClientSetup) Remove() error {
	mountPoint, serverExport, err := parseNFSClientDir(n.nfsClientDir)
	if err != nil {
		return err
	}

	mountPoint = filepath.Clean(mountPoint)
	if err := n.unmountExisting(mountPoint); err != nil {
		return err
	}

	if err := n.removeFSTabEntry(serverExport, mountPoint); err != nil {
		return err
	}
	if err := removeSystemGroupAndUser(nfsUser); err != nil {
		return err
	}

	color.PrintSuccess("NFS cache client setup removed: %q unmounted from %q", mountPoint, serverExport)
	return nil
}

func (n *NFSServerSetup) validateDir() error {
	info, err := os.Stat(n.nfsDir)
	if err != nil {
		return fmt.Errorf("cannot access %s -> %w", n.nfsDir, err)
	}
	if !info.IsDir() {
		return fmt.Errorf("%s is not a directory", n.nfsDir)
	}
	return nil
}

func (n *NFSServerSetup) removeAppendOnlyAttribute() error {
	args := []string{n.nfsDir, "-type", "d", "-exec", "chattr", "-a", "{}", ";"}
	if _, err := cmd.NewExecutor("", "find", args...).ExecuteOutput(); err != nil {
		return fmt.Errorf("failed to remove append-only attribute (chattr -a) from %q -> %w", n.nfsDir, err)
	}

	color.PrintHint("✔ remove append-only attribute (chattr -a) from %q", n.nfsDir)
	return nil
}

func (n *NFSServerSetup) changeDirOwnership(username string) error {
	if output, err := cmd.NewExecutor("", "chown", "-R", username+":"+username, n.nfsDir).ExecuteOutput(); err != nil {
		return fmt.Errorf("failed to change ownership of %q to %q -> %s -> %w", n.nfsDir, username, output, err)
	}

	color.PrintHint("✔ change ownership of %q to %q", n.nfsDir, username)
	return nil
}

func (n *NFSServerSetup) changeModeForDirsFiles() error {
	dirPerm := fmt.Sprintf("%o", fileio.CacheDirPerm)
	filePerm := fmt.Sprintf("%o", fileio.CacheFilePerm)

	// - chmod 02775 on directories (group writable + setgid)
	// - the first "2" is to setgid, it ensures new files/dirs inherit the user group automatically.
	args := []string{n.nfsDir, "-type", "d", "-exec", "chmod", dirPerm, "{}", ";"}
	if _, err := cmd.NewExecutor("", "find", args...).ExecuteOutput(); err != nil {
		return fmt.Errorf("failed to chmod directories %q -> %w", n.nfsDir, err)
	}
	color.PrintHint("✔ set permissions of %q to %s", n.nfsDir, dirPerm)

	// chmod 0664 on files (group can overwrite in-place)
	args = []string{n.nfsDir, "-type", "f", "-exec", "chmod", filePerm, "{}", ";"}
	if _, err := cmd.NewExecutor("", "find", args...).ExecuteOutput(); err != nil {
		return fmt.Errorf("failed to chmod files %q -> %w", n.nfsDir, err)
	}
	color.PrintHint("✔ set file permissions (chmod %s): %q", filePerm, n.nfsDir)
	return nil
}

// exportNFSConfig ensures the NFS export entry exists in /etc/exports, then reloads.
func (n *NFSServerSetup) exportNFSConfig() error {
	// rw: readable and writable
	// sync: synchronous writes for better integrity
	// no_subtree_check: avoid subtree checking which can cause permission issues
	// no_root_squash: allow root user on client to have root permissions on NFS (needed for celer's chattr +a)
	exportLine := n.nfsDir + " *(rw,sync,no_subtree_check,no_root_squash)"
	if err := fileio.ReplaceContent(nfsExportsPath, exportLine, func(line string) bool {
		fields := strings.Fields(line)
		return len(fields) > 0 && fields[0] == n.nfsDir
	}); err != nil {
		return fmt.Errorf("failed to update NFS export entry -> %w", err)
	}
	color.PrintHint("✔ add NFS export entry %q to %q", exportLine, nfsExportsPath)

	return n.reloadNFSExports()
}

func (n *NFSServerSetup) removeNFSExportEntry() error {
	if err := fileio.RemoveContent(nfsExportsPath, func(line string) bool {
		fields := strings.Fields(line)
		return len(fields) > 0 && fields[0] == n.nfsDir
	}); err != nil {
		return fmt.Errorf("failed to remove NFS export entry -> %w", err)
	}

	color.PrintHint("✔ remove NFS export entry for %q from %q", n.nfsDir, nfsExportsPath)
	return nil
}

func (n *NFSServerSetup) reloadNFSExports() error {
	if output, err := cmd.NewExecutor("", "exportfs", "-ra").ExecuteOutput(); err != nil {
		return fmt.Errorf("failed to export NFS exports -> %s -> %w", output, err)
	}
	color.PrintHint("✔ export NFS exports (exportfs -ra)")
	return nil
}

func (n *NFSServerSetup) assignAppendOnlyAttribute() error {
	args := []string{n.nfsDir, "-type", "d", "-exec", "chattr", "+a", "{}", ";"}
	if output, err := cmd.NewExecutor("", "find", args...).ExecuteOutput(); err != nil {
		return fmt.Errorf("failed to chattr +a to %s -> %s -> %w", n.nfsDir, output, err)
	}

	color.PrintHint("✔ apply append-only attribute (chattr +a) to %s", n.nfsDir)
	return nil
}

// installChattrCronJob installs a cron job that runs "find <nfs-dir> -type d -exec chattr +a"
// every minute, ensuring directories created by NFS clients are protected.
func (n *NFSServerSetup) installChattrCronJob(username string) error {
	// * * * * * 	- everytime
	// 2>/dev/null 	- execute quietly
	cronContent := fmt.Sprintf("* * * * * root /usr/bin/find %s -type d -exec /usr/bin/chattr +a {} + 2>/dev/null\n", shellQuote(n.nfsDir))
	cronPath := filepath.Join(nfsCronDir, fmt.Sprintf("%s-chattr", username))

	if err := os.WriteFile(cronPath, []byte(cronContent), 0644); err != nil {
		return fmt.Errorf("failed to write cron file -> %w", err)
	}
	color.PrintHint("✔ add chattr cron job: %s", cronPath)
	return nil
}

func (n *NFSServerSetup) removeChattrCronJob(username string) error {
	cronPath := filepath.Join(nfsCronDir, fmt.Sprintf("%s-chattr", username))
	if err := os.Remove(cronPath); err != nil && !os.IsNotExist(err) {
		return fmt.Errorf("failed to remove cron file -> %w", err)
	}
	color.PrintHint("✔ remove chattr cron job: %s", cronPath)
	return nil
}

func (n *NFSClientSetup) updateFSTabEntry(serverExport, mountPoint string) error {
	fstabEntry := fmt.Sprintf("%s %s nfs rw,_netdev,noatime,rsize=1048576,wsize=1048576 0 0", serverExport, mountPoint)
	if err := fileio.ReplaceContent(nfsFSTabPath, fstabEntry, func(line string) bool {
		fields := strings.Fields(line)
		return len(fields) >= 2 && fields[0] == serverExport && fields[1] == mountPoint
	}); err != nil {
		return fmt.Errorf("failed to update fstab entry -> %w", err)
	}

	color.PrintHint("✔ add/update fstab entry: %q", fstabEntry)
	return nil
}

func (n *NFSClientSetup) removeFSTabEntry(serverExport, mountPoint string) error {
	if err := fileio.RemoveContent(nfsFSTabPath, func(line string) bool {
		fields := strings.Fields(line)
		return len(fields) >= 2 && fields[0] == serverExport && fields[1] == mountPoint
	}); err != nil {
		return fmt.Errorf("failed to remove fstab entry -> %w", err)
	}

	color.PrintHint("✔ remove fstab entry for %q mounted on %q", serverExport, mountPoint)
	return nil
}

func (n *NFSClientSetup) unmountExisting(mountPoint string) error {
	if err := cmd.NewExecutor("", "mountpoint", "-q", mountPoint).Execute(); err != nil {
		return nil
	}

	if output, err := cmd.NewExecutor("", "umount", mountPoint).ExecuteOutput(); err != nil {
		return fmt.Errorf("failed to unmount existing point: %s -> %s -> %w", mountPoint, output, err)
	}

	color.PrintHint("✔ unmount existing mount: %s", mountPoint)
	return nil
}

func (n *NFSClientSetup) mountNFSDir(serverExport, mountPoint string) error {
	output, err := cmd.NewExecutor("", "mount", "-t", "nfs", serverExport, mountPoint).ExecuteOutput()
	if err != nil {
		output = strings.TrimSpace(output)
		if output != "" {
			return fmt.Errorf("failed to mount %q from %q -> %s -> %w", mountPoint, serverExport, output, err)
		}
		return fmt.Errorf("failed to mount %q from %q -> %w", mountPoint, serverExport, err)
	}

	color.PrintHint("✔ mount nfs dir %q on %q", serverExport, mountPoint)
	return nil
}

// checkServerTools verifies that all system packages needed by server setup are installed.
func checkServerTools() error {
	return buildtools.CheckSystemTools([]string{
		"apt:nfs-kernel-server", // NFS server
		"apt:passwd",            // provides useradd
		"yum:nfs-utils",         // NFS server
		"yum:shadow-utils",      // provides useradd
	})
}

// checkClientTools verifies that NFS client packages are installed.
func checkClientTools() error {
	return buildtools.CheckSystemTools([]string{
		"apt:nfs-common",
		"yum:nfs-utils",
	})
}

// createSystemGroupAndUser creates the celer system user/group if it does not already exist.
// If requiredGID is set, the group must use that numeric gid.
func createSystemGroupAndUser(username, requiredGID string) error {
	// Create system group if not exist.
	groupExists := true
	group, err := lookupGroup(username)
	if err != nil {
		var unknownGroupError user.UnknownGroupError
		if !errors.As(err, &unknownGroupError) {
			return fmt.Errorf("failed to lookup %s group -> %w", username, err)
		}
		groupExists = false
	}

	// Change local GID as the same of remote GID.
	if groupExists {
		if requiredGID != "" && group.Gid != requiredGID {
			if err := changeGroupGID(username, group.Gid, requiredGID); err != nil {
				return err
			}
		} else {
			color.PrintHint("✔ system group exists: %s", username)
		}
	} else {
		var output string
		var err error

		if requiredGID != "" {
			if err := ensureGroupIDUnused(requiredGID, username); err != nil {
				return err
			}
			output, err = cmd.NewExecutor("", "groupadd", "--system", "--gid", requiredGID, username).ExecuteOutput()
		} else {
			output, err = cmd.NewExecutor("", "groupadd", "--system", username).ExecuteOutput()
		}
		if err != nil {
			return fmt.Errorf("failed to create %s group -> %s -> %w", username, output, err)
		}
		color.PrintHint("✔ create system group: %s", username)
	}

	// Check if celer user is created.
	userExists := true
	if _, err := lookupUser(username); err != nil {
		var unknownUserError user.UnknownUserError
		if !errors.As(err, &unknownUserError) {
			return fmt.Errorf("failed to lookup %s user -> %w", username, err)
		}
		userExists = false
	}
	if userExists {
		color.PrintHint("✔ system user exists: %s", username)
		return nil
	}

	// Create system user (no login shell, no home directory).
	args := []string{"--system", "--no-create-home", "--shell", "/usr/sbin/nologin", "--gid", username, username}
	if output, err := cmd.NewExecutor("", "useradd", args...).ExecuteOutput(); err != nil {
		return fmt.Errorf("failed to create %s user -> %s -> %w", username, output, err)
	}

	color.PrintHint("✔ create system user: %s", username)
	return nil
}

func ensureGroupIDUnused(requiredGID, allowedGroupName string) error {
	existingGroup, err := lookupGroupID(requiredGID)
	if err != nil {
		var unknownGroupIDError user.UnknownGroupIdError
		var unknownGroupError user.UnknownGroupError
		if !errors.As(err, &unknownGroupIDError) && !errors.As(err, &unknownGroupError) {
			return fmt.Errorf("failed to lookup group id %s -> %w", requiredGID, err)
		}
		return nil
	}

	if existingGroup.Name == allowedGroupName {
		return nil
	}

	return fmt.Errorf("gid %s is already used by local group %q", requiredGID, existingGroup.Name)
}

func changeGroupGID(groupName, oldGID, requiredGID string) error {
	if err := ensureGroupIDUnused(requiredGID, groupName); err != nil {
		return err
	}

	if output, err := cmd.NewExecutor("", "groupmod", "-g", requiredGID, groupName).ExecuteOutput(); err != nil {
		return fmt.Errorf("failed to change %s group gid from %s to %s -> %s -> %w", groupName, oldGID, requiredGID, output, err)
	}

	color.PrintHint("✔ change system group %s gid from %s to %s", groupName, oldGID, requiredGID)
	return nil
}

func removeSystemGroupAndUser(username string) error {
	// If the user is still in the group, groupdel may fail or leave the system in an inconsistent state.
	if err := removeCurrentUserFromGroup(username); err != nil {
		return err
	}

	// Remove system user.
	userExists := true
	if _, err := lookupUser(username); err != nil {
		var unknownUserError user.UnknownUserError
		if !errors.As(err, &unknownUserError) {
			return fmt.Errorf("failed to lookup %s user -> %w", username, err)
		}
		userExists = false
	}
	if userExists {
		if output, err := cmd.NewExecutor("", "userdel", username).ExecuteOutput(); err != nil {
			return fmt.Errorf("failed to remove %s user -> %s -> %w", username, output, err)
		}
		color.PrintHint("✔ remove system user %s", username)
	}

	// Remove system group.
	groupExists := true
	if _, err := lookupGroup(username); err != nil {
		var unknownGroupError user.UnknownGroupError
		if !errors.As(err, &unknownGroupError) {
			return fmt.Errorf("failed to lookup %s group -> %w", username, err)
		}
		groupExists = false
	}
	if groupExists {
		if output, err := cmd.NewExecutor("", "groupdel", username).ExecuteOutput(); err != nil {
			return fmt.Errorf("failed to remove %s group -> %s -> %w", username, output, err)
		}
		color.PrintHint("✔ remove system group: %s", username)
	}

	return nil
}

func removeCurrentUserFromGroup(groupName string) error {
	currentUser, err := currentUserForGroup()
	if err != nil {
		return err
	}

	groupExists := true
	if _, err := lookupGroup(groupName); err != nil {
		var unknownGroupError user.UnknownGroupError
		if !errors.As(err, &unknownGroupError) {
			return fmt.Errorf("failed to lookup %s group -> %w", groupName, err)
		}
		groupExists = false
	}
	if !groupExists {
		return nil
	}

	groups, err := cmd.NewExecutor("", "id", "-nG", currentUser).ExecuteOutput()
	if err != nil {
		return fmt.Errorf("failed to get groups for %q -> %w", currentUser, err)
	}
	userInGroup := slices.Contains(strings.Fields(groups), groupName)
	if !userInGroup {
		return nil
	}

	if output, err := cmd.NewExecutor("", "gpasswd", "-d", currentUser, groupName).ExecuteOutput(); err != nil {
		return fmt.Errorf("failed to remove %q from %q group -> %s -> %w", currentUser, groupName, output, err)
	}
	color.PrintHint("✔ remove %s from group %s", currentUser, groupName)
	return nil
}

func currentUserForGroup() (string, error) {
	if sudoUser := os.Getenv("SUDO_USER"); sudoUser != "" {
		return sudoUser, nil
	}

	if currentUser := os.Getenv("USER"); currentUser != "" {
		return currentUser, nil
	}

	return "", fmt.Errorf("cannot read current user from environment")
}

// addCurrentUserToGroup adds the invoking user to the celer group.
// Under sudo this is SUDO_USER; otherwise it falls back to USER.
// usermod -aG is idempotent: adding an already-member user is a no-op.
func addCurrentUserToGroup(groupName string) error {
	currentUser, err := currentUserForGroup()
	if err != nil {
		return err
	}

	if _, err := cmd.NewExecutor("", "usermod", "-aG", groupName, currentUser).ExecuteOutput(); err != nil {
		return fmt.Errorf("failed to add %q to %q group -> %w", currentUser, groupName, err)
	}

	color.PrintHint("✔ %s is added to group %s. 💡 run `newgrp %s` or re-login to take effect.", currentUser, groupName, groupName)
	return nil
}

// parseNFSClientDir parses the --nfs-client-dir flag value.
// Expected format: <mount_point>@<server>:<export_path>
func parseNFSClientDir(val string) (mountPoint, serverExport string, err error) {
	parts := strings.SplitN(val, "@", 2)
	if len(parts) != 2 || parts[0] == "" || parts[1] == "" {
		return "", "", fmt.Errorf("--nfs-client-dir format: <mount_point>@<server>:<export_path>")
	}
	if !strings.Contains(parts[1], ":") {
		return "", "", fmt.Errorf("--nfs-client-dir: server export must be in <server>:<path> format")
	}
	return parts[0], parts[1], nil
}

func shellQuote(value string) string {
	return "'" + strings.ReplaceAll(value, "'", "'\\''") + "'"
}
