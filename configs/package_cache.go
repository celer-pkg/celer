package configs

import (
	"celer/context"
	"celer/pkgs/dirs"
	"celer/pkgs/fileio"
	"crypto/sha256"
	"errors"
	"fmt"
	"os"
	"path/filepath"
	"strings"
)

type PackageCache struct {
	Dir      string `toml:"dir"`
	Writable bool   `toml:"writable"`

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
	archiveDir := filepath.Join(b.Dir, platformName, projectName, buildType, nameVersion)
	archivePath := filepath.Join(archiveDir, hash+".tar.gz")
	if !fileio.PathExists(archivePath) {
		return false, nil // not an error even not exist.
	}

	// The meta file hash should be the same as hash that calcuated dynamically.
	metaPath := filepath.Join(archiveDir, "meta", hash+".meta")
	metaBytes, err := os.ReadFile(metaPath)
	if err != nil {
		if errors.Is(err, os.ErrNotExist) {
			return false, fmt.Errorf("cache archive exists but metadata is missing: %s", metaPath)
		}
		return false, err
	}
	metaHash := sha256.Sum256(metaBytes)
	if fmt.Sprintf("%x", metaHash) != hash {
		return false, fmt.Errorf("cache metadata checksum mismatch for %s", nameVersion)
	}

	// Create tmp dir for extracting inside.
	if err := dirs.CleanTmpFilesDir(); err != nil {
		return false, fmt.Errorf("failed to clean tmp files dir -> %w", err)
	}
	tempDir, err := os.MkdirTemp(dirs.TmpFilesDir, ".pkgcache-extract-*")
	if err != nil {
		return false, err
	}
	defer os.RemoveAll(tempDir)

	// Extract to a tmp dir and move back to dest dir.
	if err := fileio.Extract(archivePath, tempDir); err != nil {
		return false, err
	}
	if err := os.RemoveAll(destDir); err != nil {
		return false, err
	}
	if err := os.Rename(tempDir, destDir); err != nil {
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

	// Extract tar.gz to a tmp dir.
	archiveName := fmt.Sprintf("%s@%s.tar.gz", libName, libVersion)
	if err := dirs.CleanTmpFilesDir(); err != nil {
		return fmt.Errorf("failed to clean tmp files dir -> %w", err)
	}
	tempArchive, err := os.CreateTemp(dirs.TmpFilesDir, archiveName+".*")
	if err != nil {
		return err
	}
	tempArchivePath := tempArchive.Name()
	tempArchive.Close()
	defer os.Remove(tempArchivePath)

	if err := fileio.Targz(tempArchivePath, packageDir, false); err != nil {
		return err
	}

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

	// Move tmp file to dest archive file.
	archivePath := filepath.Join(destDir, hash+".tar.gz")
	archiveTempPath := archivePath + ".tmp"
	if err := fileio.CopyFile(tempArchivePath, archiveTempPath); err != nil {
		return err
	}
	if err := os.Rename(archiveTempPath, archivePath); err != nil {
		return err
	}

	// Write meta file to meta dir.
	metaPath := filepath.Join(metaDir, hash+".meta")
	metaTempPath := metaPath + ".tmp"
	if err := os.WriteFile(metaTempPath, []byte(meta), os.ModePerm); err != nil {
		return err
	}
	if err := os.Rename(metaTempPath, metaPath); err != nil {
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

func (b PackageCache) IsWritable() bool {
	return b.Writable
}
