package configs

import (
	"os"
	"path/filepath"
	"testing"

	"github.com/celer-pkg/celer/buildsystems"
	"github.com/celer-pkg/celer/internal/fileio"
)

func TestInstallFromPackage_MissingMetaIsCacheMiss(t *testing.T) {
	pkgDir := filepath.Join(t.TempDir(), "packages", "demo", "proj", "release", "demo@1.0")
	if err := os.MkdirAll(filepath.Join(pkgDir, "include"), os.ModePerm); err != nil {
		t.Fatalf("failed to create package dir -> %v", err)
	}
	if err := os.WriteFile(filepath.Join(pkgDir, "include", "demo.h"), []byte("// demo\n"), 0o644); err != nil {
		t.Fatalf("failed to write package file -> %v", err)
	}

	p := Port{Name: "demo", Version: "1.0", PackageDir: pkgDir}
	p.MatchedConfig = &buildsystems.BuildConfig{
		PortConfig: buildsystems.PortConfig{PackageDir: pkgDir},
	}

	installed, err := p.installFromPackage(InstallOptions{})
	if err != nil {
		t.Fatalf("installFromPackage() error = %v, want a cache miss", err)
	}
	if installed {
		t.Fatal("installFromPackage() = installed, want a cache miss")
	}
	if fileio.PathExists(pkgDir) {
		t.Fatalf("incomplete package dir %s should be removed", pkgDir)
	}
}
