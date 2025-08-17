package configs

import (
	"celer/pkgs/fileio"
	"crypto/sha256"
	"fmt"
	"os"
	"path/filepath"
	"strings"
)

type CacheDir struct {
	Dir string `toml:"dir"`
}

func (c CacheDir) Validate() error {
	if c.Dir == "" {
		return fmt.Errorf("cache dir is empty")
	}
	if !fileio.PathExists(c.Dir) {
		return fmt.Errorf("cache dir %s does not exist", c.Dir)
	}
	return nil
}

func (c CacheDir) Read(platformName, projectName, buildType, nameVersion, buildhash, destDir string) (bool, error) {
	archivePath := filepath.Join(c.Dir, platformName, projectName, buildType, nameVersion, buildhash)
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

func (c CacheDir) Write(packageDir, builddesc string) error {
	if !fileio.PathExists(packageDir) {
		return fmt.Errorf("package dir %s does not exist", packageDir)
	}

	// Create a tarball from package dir.
	parts := strings.Split(filepath.Base(packageDir), "@")
	if len(parts) != 5 {
		return fmt.Errorf("invalid package dir: %s", packageDir)
	}

	var (
		libName      = parts[0]
		libVersion   = parts[1]
		platformName = parts[2]
		projectName  = parts[3]
		buildType    = parts[4]
	)

	archiveName := fmt.Sprintf("%s@%s.tar.gz", libName, libVersion)
	destPath := filepath.Join(os.TempDir(), archiveName)
	if err := fileio.Targz(destPath, packageDir, false); err != nil {
		return err
	}
	defer os.Remove(destPath)

	nameVersion := fmt.Sprintf("%s@%s", libName, libVersion)
	destDir := filepath.Join(c.Dir, platformName, projectName, buildType, nameVersion)
	hashDir := filepath.Join(destDir, "hash")

	// Calculate checksum of description.
	data := sha256.Sum256([]byte(builddesc))
	buildhash := fmt.Sprintf("%x", data)

	// Create the dir if not exist.
	if err := os.MkdirAll(destDir, os.ModePerm); err != nil {
		return err
	}

	// Create the hash dir if not exist.
	if err := os.MkdirAll(hashDir, os.ModePerm); err != nil {
		return err
	}

	// Move the tarball to cache dir.
	if err := fileio.CopyFile(destPath, filepath.Join(destDir, buildhash+".tar.gz")); err != nil {
		return err
	}

	// Write description to hash dir.
	if err := os.WriteFile(filepath.Join(hashDir, buildhash+".txt"), []byte(builddesc), os.ModePerm); err != nil {
		return err
	}

	defer os.Remove(destPath)
	return nil
}

// Remove removes the cache for the specified platform, project, build type and name version.
func (c CacheDir) Remove(platformName, projectName, buildType, nameVersion string) error {
	pacakgeDir := filepath.Join(c.Dir, platformName, projectName, buildType, nameVersion)
	if fileio.PathExists(pacakgeDir) {
		if err := os.RemoveAll(pacakgeDir); err != nil {
			return fmt.Errorf("remove cache package %s error: %w", pacakgeDir, err)
		}
	}

	return nil
}

// Exist check both archive file and build desc file exist.
func (c CacheDir) Exist(platformName, projectName, buildType, nameVersion, buildhash string) bool {
	archivePath := filepath.Join(c.Dir, platformName, projectName, buildType, nameVersion, buildhash+".tar.gz")
	buildDescPath := filepath.Join(c.Dir, platformName, projectName, buildType, nameVersion, "hash", buildhash+".txt")
	return fileio.PathExists(archivePath) && fileio.PathExists(buildDescPath)
}
