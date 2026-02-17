package configs

import (
	"celer/context"
	"celer/pkgs/fileio"
	"crypto/sha256"
	"fmt"
	"os"
	"path/filepath"
	"strings"
)

type PackageCache struct {
	Dir string `toml:"dir"`

	// Internal field.
	ctx context.Context
}

func (b PackageCache) Validate() error {
	if b.Dir == "" {
		return fmt.Errorf("package cache dir is empty")
	}
	if !fileio.PathExists(b.Dir) {
		return fmt.Errorf("package cache dir does not exist: %s", b.Dir)
	}
	return nil
}

func (b PackageCache) Read(nameVersion, hash, destDir string) (bool, error) {
	platformName := b.ctx.Platform().GetName()
	projectName := b.ctx.Project().GetName()
	buildType := b.ctx.BuildType()
	archivePath := filepath.Join(b.Dir, platformName, projectName, buildType, nameVersion, hash)
	if !fileio.PathExists(archivePath) {
		return false, nil // not an error even not exist.
	}

	if err := os.MkdirAll(destDir, os.ModePerm); err != nil {
		return false, err
	}

	if err := fileio.Extract(archivePath, destDir); err != nil {
		return false, err
	}

	return true, nil
}

func (b PackageCache) Write(packageDir, meta string) error {
	if !fileio.PathExists(packageDir) {
		return fmt.Errorf("package dir does not exist: %s", packageDir)
	}

	// Validate packageDir format.
	parts := strings.Split(filepath.Base(packageDir), "@")
	if len(parts) != 5 {
		return fmt.Errorf("invalid package dir: %s", packageDir)
	}

	var (
		libName      = parts[0]
		libVersion   = parts[1]
		platformName = parts[2]
		projectName  = parts[3]
		buildType    = strings.ToLower(parts[4])
	)

	archiveName := fmt.Sprintf("%s@%s.tar.gz", libName, libVersion)
	archivePath := filepath.Join(os.TempDir(), archiveName)
	if err := fileio.Targz(archivePath, packageDir, false); err != nil {
		return err
	}
	defer os.Remove(archivePath)

	nameVersion := fmt.Sprintf("%s@%s", libName, libVersion)
	destDir := filepath.Join(b.Dir, platformName, projectName, buildType, nameVersion)
	metaDir := filepath.Join(destDir, "meta")

	// Calculate checksum of metadata (this would be the cache key).
	data := sha256.Sum256([]byte(meta))
	hash := fmt.Sprintf("%x", data)

	// Create the dir if not exist.
	if err := os.MkdirAll(destDir, os.ModePerm); err != nil {
		return err
	}

	// Create the meta dir if not exist.
	if err := os.MkdirAll(metaDir, os.ModePerm); err != nil {
		return err
	}

	// Move the tarball to cache dir.
	if err := fileio.CopyFile(archivePath, filepath.Join(destDir, hash+".tar.gz")); err != nil {
		return err
	}

	// Write metadata to meta dir.
	if err := os.WriteFile(filepath.Join(metaDir, hash+".meta"), []byte(meta), os.ModePerm); err != nil {
		return err
	}

	return nil
}

// Remove removes the cache for the specified platform, project, build type and name version.
func (b PackageCache) Remove(nameVersion string) error {
	platformName := b.ctx.Platform().GetName()
	projectName := b.ctx.Project().GetName()
	buildType := b.ctx.BuildType()
	pacakgeDir := filepath.Join(b.Dir, platformName, projectName, buildType, nameVersion)
	if fileio.PathExists(pacakgeDir) {
		if err := os.RemoveAll(pacakgeDir); err != nil {
			return fmt.Errorf("failed toremove cache package %s -> %w", pacakgeDir, err)
		}
	}

	return nil
}

// Exist check both archive file and build desc file exist.
func (b PackageCache) Exist(nameVersion, hash string) bool {
	platformName := b.ctx.Platform().GetName()
	projectName := b.ctx.Project().GetName()
	buildType := b.ctx.BuildType()
	archivePath := filepath.Join(b.Dir, platformName, projectName, buildType, nameVersion, hash+".tar.gz")
	metaFilePath := filepath.Join(b.Dir, platformName, projectName, buildType, nameVersion, "meta", hash+".meta")
	return fileio.PathExists(archivePath) && fileio.PathExists(metaFilePath)
}

func (b PackageCache) GetDir() string {
	return b.Dir
}
