package buildtools

import (
	"os"
	"path/filepath"
	"runtime"
	"testing"
)

func TestIsPackageInstalled(t *testing.T) {
	venv := t.TempDir()
	sitePackages := fakeSitePackages(t, venv)

	mkdir(t, filepath.Join(sitePackages, "meson-1.11.2.dist-info"))
	mkdir(t, filepath.Join(sitePackages, "unpackage")) // Import directory only, no metadata.

	var tests = []struct {
		name    string
		spec    string
		want    bool
		because string
	}{
		{name: "the pinned version", spec: "meson==1.11.2", want: true},
		{name: "another version", spec: "meson==1.12.1", want: false, because: "1.11.2 is installed, 1.12.1 is not"},
		{name: "an unpinned spec with a version installed", spec: "meson", want: true},
		{name: "an unpinned spec with only the import directory", spec: "unpackage", want: true},
		{name: "a pinned spec with only the import directory", spec: "unpackage==1.0.0", want: false},
		{name: "an absent package", spec: "ninja==1.12.1", want: false},
	}

	for _, test := range tests {
		t.Run(test.name, func(t *testing.T) {
			if got := isPackageInstalled(test.spec, venv); got != test.want {
				t.Fatalf("isPackageInstalled(%q) = %t, want %t (%s)", test.spec, got, test.want, test.because)
			}
		})
	}
}

// fakeSitePackages creates the site-packages layout of a virtualenv and returns it.
func fakeSitePackages(t *testing.T, venvDir string) string {
	t.Helper()

	dir := filepath.Join(venvDir, "lib", "python3.10", "site-packages")
	if runtime.GOOS == "windows" {
		dir = filepath.Join(venvDir, "Lib", "site-packages")
	}
	mkdir(t, dir)
	return dir
}

func mkdir(t *testing.T, dir string) {
	t.Helper()
	if err := os.MkdirAll(dir, os.ModePerm); err != nil {
		t.Fatal(err)
	}
}
