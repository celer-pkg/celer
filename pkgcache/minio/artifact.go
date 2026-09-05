package minio

import (
	"crypto/sha256"
	"fmt"
	"os"
	"path/filepath"
	"strings"
	"time"

	"github.com/celer-pkg/celer/context"
	"github.com/celer-pkg/celer/pkgs/color"
	"github.com/celer-pkg/celer/pkgs/fileio"
)

type ArtifactConfig struct {
	minioCache
	ctx        context.Context
	cacheDir   string
	writable   bool
	maxRetries int
}

// Store compresses the package dir and store in cache,
// the meta is expected to be a string and would be used to calculate the hash key for cache.
func (a ArtifactConfig) Store(packageDir, meta string) error {
	// skip when offline.
	if a.ctx.Offline() {
		return nil
	}

	if !fileio.PathExists(packageDir) {
		return fmt.Errorf("package dir does not exist: %s", packageDir)
	}

	// Validate packageDir format and extract metadata.
	// Path format: packages/platform/project/buildType/nameVersion
	parts := strings.Split(filepath.ToSlash(packageDir), "/")
	if len(parts) < 5 {
		return fmt.Errorf("invalid package dir format: %s", packageDir)
	}

	// Extract from path components.
	nameVersion := parts[len(parts)-1]
	buildType := parts[len(parts)-2]
	projectName := parts[len(parts)-3]
	platformName := parts[len(parts)-4]

	// Validate nameVersion format (should be name@version)
	versionParts := strings.Split(nameVersion, "@")
	if len(versionParts) != 2 {
		return fmt.Errorf("invalid package dir: %s", packageDir)
	}

	var (
		libName       = versionParts[0]
		libVersion    = versionParts[1]
		remoteDir     = filepath.Join(a.cacheDir, platformName, projectName, buildType, nameVersion)
		remoteMetaDir = filepath.Join(remoteDir, "metas")
	)

	// Calculate checksum of metadata，this would be the cache key.
	hashData := sha256.Sum256([]byte(meta))
	hashStr := fmt.Sprintf("%x", hashData)
	remoteArtifactPath := filepath.Join(remoteDir, hashStr+".tar.gz")

	// Compress package dir to a temp archive.
	fileName := fmt.Sprintf("%s@%s.tar.gz", libName, libVersion)
	tmpArchivePath := filepath.Join(os.TempDir(), fmt.Sprintf("celer-pkgcache-artifact-%s-%d", fileName, time.Now().UnixNano()))
	if err := fileio.Targz(tmpArchivePath, packageDir, false); err != nil {
		return fmt.Errorf("failed to compress package as archive for '%s' -> %w", nameVersion, err)
	}
	defer os.Remove(tmpArchivePath)

	// Upload the meta file before the archive.
	metaFilePath := filepath.Join(remoteMetaDir, hashStr+".meta")
	metaFile, err := os.CreateTemp(os.TempDir(), "celer-pkgcache-artifact-*.meta")
	if err != nil {
		return fmt.Errorf("failed to create tmp file to save meta for '%s' -> %w", nameVersion, err)
	}
	defer os.Remove(metaFile.Name())
	if _, err := metaFile.WriteString(meta); err != nil {
		return fmt.Errorf("failed to write meta into file for '%s' -> %w", nameVersion, err)
	}
	if _, err := a.UploadFile(metaFile.Name(), metaFilePath, nil); err != nil {
		return fmt.Errorf("failed to upload meta for '%s' to minio -> %w", nameVersion, err)
	}
	defer metaFile.Close()

	// Upload archive file with progress.
	if _, err := a.UploadFile(tmpArchivePath, remoteArtifactPath, func(percent int) {
		if percent < 100 {
			color.PrintInline(color.Hint, "[-] %s is uploading artifact archive: %d%%", fileName, percent)
		} else if percent == 100 {
			color.PrintInline(color.Pass, "[✔] %s is stored to pkgcache as artifact archive.\n", fileName)
		}
	}); err != nil {
		return fmt.Errorf("failed to upload artifact for '%s' -> %w", nameVersion, err)
	}

	return nil
}

// Restore restores the cached package to package directory if cache hit.
func (a ArtifactConfig) Restore(packageDir, nameVersion, buildHash string) (bool, error) {
	// skip when offline.
	if a.ctx.Offline() {
		return false, nil
	}

	platformName := a.ctx.Platform().GetName()
	projectName := a.ctx.Project().GetName()
	buildType := a.ctx.BuildType()

	remoteDir := filepath.Join(a.cacheDir, platformName, projectName, buildType, nameVersion)
	remoteMetaFilePath := filepath.Join(remoteDir, "metas", buildHash+".meta")
	remoteArtifactPath := filepath.Join(remoteDir, buildHash+".tar.gz")

	// Get file meta info and check if checksum matches.
	remoteMetaInfo, err := a.GetFileInfo(remoteMetaFilePath)
	if err != nil {
		return false, fmt.Errorf("failed to get file object info '%s' -> %w", remoteMetaFilePath, err)
	}
	if remoteMetaInfo == nil {
		color.PrintWarning("======== cached artifact for %s has no metadata, it'll build from source ========", nameVersion)
		return false, nil
	} else {
		tmpMetaFile, err := a.DownloadFile(remoteMetaFilePath)
		if err != nil {
			return false, fmt.Errorf("failed to download meta file '%s' -> %w", remoteMetaFilePath, err)
		}
		defer os.Remove(tmpMetaFile)

		// Meta meta file and check if meta matches.
		metaBytes, err := os.ReadFile(tmpMetaFile)
		if err != nil {
			return false, fmt.Errorf("failed to read meta file '%s' -> %w", tmpMetaFile, err)
		}
		metaHash := sha256.Sum256(metaBytes)
		if fmt.Sprintf("%x", metaHash) != buildHash {
			return false, fmt.Errorf("cache metadata checksum mismatch for %s", nameVersion)
		}
	}

	// Get file archive info and check if checksum matches.
	remoteInfo, err := a.GetFileInfo(remoteArtifactPath)
	if err != nil {
		return false, fmt.Errorf("failed to get file info '%s' -> %w", remoteArtifactPath, err)
	}
	if remoteInfo == nil {
		color.PrintWarning("======== no artifact found for %s and it'll build from source ========", nameVersion)
		return false, nil
	}

	downloaded, err := a.DownloadFile(remoteArtifactPath)
	if err != nil {
		return false, fmt.Errorf("failed to download artifact '%s' -> %w", remoteArtifactPath, err)
	}
	defer os.Remove(downloaded)

	// Verify the downloaded content with sha265.
	if expected := a.metaSha256(remoteInfo); expected != "" {
		if got, err := fileio.SHA256Sum(downloaded); err != nil {
			return false, err
		} else if got != expected {
			color.PrintWarning("======== cached artifact for %s is corrupted, it'll build from source ========", nameVersion)
			return false, nil
		}
	}

	tempDir, err := os.MkdirTemp(os.TempDir(), "celer-pkgcache-artifact-extract-*")
	if err != nil {
		return false, err
	}
	defer os.RemoveAll(tempDir)

	// Extract to a tmp dir.
	if err := fileio.Extract(downloaded, tempDir); err != nil {
		return false, fmt.Errorf("failed to extract artifact '%s' -> %w", nameVersion, err)
	}

	// Clean package dir.
	if err := os.RemoveAll(packageDir); err != nil {
		return false, err
	}
	if err := os.MkdirAll(filepath.Dir(packageDir), os.ModePerm); err != nil {
		return false, err
	}

	// Rename extracted dir as package dir.
	if err := os.Rename(tempDir, packageDir); err != nil {
		return false, err
	}

	return true, nil
}
