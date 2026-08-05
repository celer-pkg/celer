package nfs

import (
	"fmt"
	"os"
	"path/filepath"
	"runtime"
	"strings"
	"testing"
)

func TestParseNFSClientDir(t *testing.T) {
	tests := []struct {
		name             string
		value            string
		wantMountPoint   string
		wantServerExport string
		wantErr          string
	}{
		{
			name:             "valid",
			value:            "/mnt/cache@server:/exports/cache",
			wantMountPoint:   "/mnt/cache",
			wantServerExport: "server:/exports/cache",
		},
		{
			name:    "missing separator",
			value:   "/mnt/cache",
			wantErr: "--nfs-client format: <mount_point>@<server>:<export_path>",
		},
		{
			name:    "empty mount point",
			value:   "@server:/exports/cache",
			wantErr: "--nfs-client format: <mount_point>@<server>:<export_path>",
		},
		{
			name:    "empty server export",
			value:   "/mnt/cache@",
			wantErr: "--nfs-client format: <mount_point>@<server>:<export_path>",
		},
		{
			name:    "server export missing colon",
			value:   "/mnt/cache@server",
			wantErr: "--nfs-client: server export must be in <server>:<path> format",
		},
	}

	for _, test := range tests {
		t.Run(test.name, func(t *testing.T) {
			gotMountPoint, gotServerExport, err := parseNFSClientDir(test.value)
			if test.wantErr != "" {
				if err == nil || err.Error() != test.wantErr {
					t.Fatalf("parseNFSClientDir() error = %v, want %q", err, test.wantErr)
				}
				return
			}
			if err != nil {
				t.Fatalf("parseNFSClientDir() error = %v", err)
			}
			if gotMountPoint != test.wantMountPoint || gotServerExport != test.wantServerExport {
				t.Fatalf("parseNFSClientDir() = (%q, %q), want (%q, %q)", gotMountPoint, gotServerExport, test.wantMountPoint, test.wantServerExport)
			}
		})
	}
}

func TestShellQuote(t *testing.T) {
	tests := []struct {
		value string
		want  string
	}{
		{value: "/srv/cache", want: "'/srv/cache'"},
		{value: "/srv/celer cache", want: "'/srv/celer cache'"},
		{value: "/srv/celer's cache", want: "'/srv/celer'\\''s cache'"},
	}

	for _, tt := range tests {
		t.Run(tt.value, func(t *testing.T) {
			if got := shellQuote(tt.value); got != tt.want {
				t.Fatalf("shellQuote(%q) = %q, want %q", tt.value, got, tt.want)
			}
		})
	}
}

func TestNFSServerSetupRemoveNFSExportEntry(t *testing.T) {
	exportsPath := filepath.Join(t.TempDir(), "exports")
	if err := os.WriteFile(exportsPath, []byte("/srv/celer-cache *(rw,sync,no_subtree_check,no_root_squash)\n/srv/other *(rw,sync)\n"), 0644); err != nil {
		t.Fatal(err)
	}

	oldNFSExportsPath := nfsExportsPath
	nfsExportsPath = exportsPath
	t.Cleanup(func() { nfsExportsPath = oldNFSExportsPath })

	setup := NewNFSServerSetup("/srv/celer-cache")
	if err := setup.removeNFSExportEntry(); err != nil {
		t.Fatal(err)
	}

	assertFileContent(t, exportsPath, "/srv/other *(rw,sync)\n")
}

func TestNFSServerSetupRemoveChattrCronJob(t *testing.T) {
	cronDir := t.TempDir()
	cronPath := filepath.Join(cronDir, "celer-chattr")
	if err := os.WriteFile(cronPath, []byte("* * * * * root /usr/bin/find /srv/cache -type d -exec /usr/bin/chattr +a {} + 2>/dev/null\n"), 0644); err != nil {
		t.Fatal(err)
	}

	oldNFSCronDir := nfsCronDir
	nfsCronDir = cronDir
	t.Cleanup(func() { nfsCronDir = oldNFSCronDir })

	setup := NewNFSServerSetup("/srv/cache")
	if err := setup.removeChattrCronJob(nfsUser); err != nil {
		t.Fatal(err)
	}
	if _, err := os.Stat(cronPath); !os.IsNotExist(err) {
		t.Fatalf("os.Stat() error = %v, want missing cron file", err)
	}
	if err := setup.removeChattrCronJob(nfsUser); err != nil {
		t.Fatal(err)
	}
}

func TestNFSClientSetupRemoveFSTabEntry(t *testing.T) {
	fstabPath := filepath.Join(t.TempDir(), "fstab")
	if err := os.WriteFile(fstabPath, []byte("server:/exports/cache /mnt/cache nfs rw,_netdev,noatime,rsize=1048576,wsize=1048576 0 0\nserver:/exports/other /mnt/other nfs rw,_netdev 0 0\n"), 0644); err != nil {
		t.Fatal(err)
	}

	oldNFSFSTabPath := nfsFSTabPath
	nfsFSTabPath = fstabPath
	t.Cleanup(func() { nfsFSTabPath = oldNFSFSTabPath })

	setup := NewNFSClientSetup("/mnt/cache@server:/exports/cache")
	if err := setup.removeFSTabEntry("server:/exports/cache", "/mnt/cache"); err != nil {
		t.Fatal(err)
	}

	assertFileContent(t, fstabPath, "server:/exports/other /mnt/other nfs rw,_netdev 0 0\n")
}

func TestNFSClientSetupMountNFSDirIncludesMountStderr(t *testing.T) {
	binDir := t.TempDir()
	// Use platform-appropriate script: .bat on Windows, shell script on Unix.
	mountPath := filepath.Join(binDir, "mount")
	mountPath, script := mountMockScript(mountPath)
	if err := os.WriteFile(mountPath, []byte(script), 0755); err != nil {
		t.Fatal(err)
	}
	// probeNFSServer runs "ping -c 1 -W 3 <host>" before mount; provide a fake.
	pingPath := filepath.Join(binDir, "ping")
	pingPath, pingScript := pingMockScript(pingPath)
	if err := os.WriteFile(pingPath, []byte(pingScript), 0755); err != nil {
		t.Fatal(err)
	}
	t.Setenv("PATH", binDir+string(os.PathListSeparator)+os.Getenv("PATH"))

	setup := NewNFSClientSetup("/mnt/cache@server:/missing")
	err := setup.mountNFSDir("server:/missing", "/mnt/cache")
	if err == nil {
		t.Fatal("mountNFSDir() error = nil")
	}

	got := err.Error()
	for _, want := range []string{
		`failed to mount "/mnt/cache" from "server:/missing"`,
		"mount.nfs: mounting server:/missing failed",
		"No such file or directory",
		"exit status 32",
	} {
		if !strings.Contains(got, want) {
			t.Fatalf("mountNFSDir() error = %q, want to contain %q", got, want)
		}
	}
}

func TestCurrentUserForGroup(t *testing.T) {
	tests := []struct {
		name     string
		sudoUser string
		user     string
		want     string
		wantErr  string
	}{
		{
			name:     "prefers SUDO_USER",
			sudoUser: "alice",
			user:     "root",
			want:     "alice",
		},
		{
			name: "falls back to USER",
			user: "bob",
			want: "bob",
		},
		{
			name:     "rejects root from SUDO_USER",
			sudoUser: "root",
			user:     "ignored",
			wantErr:  "cannot determine the invoking non-root user",
		},
		{
			name:    "rejects root from USER",
			user:    "root",
			wantErr: "cannot determine the invoking non-root user",
		},
		{
			name:    "rejects empty environment",
			wantErr: "cannot read current user from environment",
		},
	}

	for _, test := range tests {
		t.Run(test.name, func(t *testing.T) {
			t.Setenv("SUDO_USER", test.sudoUser)
			t.Setenv("USER", test.user)

			got, err := currentUserForGroup()
			if test.wantErr != "" {
				if err == nil || !strings.Contains(err.Error(), test.wantErr) {
					t.Fatalf("currentUserForGroup() error = %v, want to contain %q", err, test.wantErr)
				}
				return
			}
			if err != nil {
				t.Fatalf("currentUserForGroup() error = %v", err)
			}
			if got != test.want {
				t.Fatalf("currentUserForGroup() = %q, want %q", got, test.want)
			}
		})
	}
}

func assertFileContent(t *testing.T, path, want string) {
	t.Helper()
	got, err := os.ReadFile(path)
	if err != nil {
		t.Fatal(err)
	}
	if string(got) != want {
		t.Fatalf("file content = %q, want %q", string(got), want)
	}
}

// mountMockScript returns the path and content for a mock "mount" command
// suitable for the current OS. On Windows it appends .bat; on Unix it uses
// the bare name with a #!/bin/sh shebang.
func mountMockScript(basePath string) (string, string) {
	return mockScript(basePath,
		"mount.nfs: mounting server:/missing failed, reason given by server: No such file or directory",
		32,
	)
}

// pingMockScript returns the path and content for a mock "ping" command.
func pingMockScript(basePath string) (string, string) {
	return mockScript(basePath, "", 0)
}

func mockScript(basePath, stderrMsg string, exitCode int) (string, string) {
	if runtime.GOOS == "windows" {
		batPath := basePath + ".bat"
		script := "@echo off\r\n"
		if stderrMsg != "" {
			script += "echo " + stderrMsg + " >&2\r\n"
		}
		script += fmt.Sprintf("exit /b %d\r\n", exitCode)
		return batPath, script
	}
	script := "#!/bin/sh\n"
	if stderrMsg != "" {
		script += "printf '%s\\n' '" + stderrMsg + "' >&2\n"
	}
	script += fmt.Sprintf("exit %d\n", exitCode)
	return basePath, script
}
