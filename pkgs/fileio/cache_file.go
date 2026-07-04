package fileio

import (
	"crypto/sha256"
	"encoding/hex"
	"fmt"
	"io"
	"os"
	"path/filepath"

	"github.com/celer-pkg/celer/pkgs/color"
)

// ComputeSHA256 computes the SHA256 hash of a file.
func ComputeSHA256(filePath string) (string, error) {
	file, err := os.Open(filePath)
	if err != nil {
		return "", fmt.Errorf("failed to open file -> %w", err)
	}
	defer file.Close()

	hash := sha256.New()
	if _, err := io.Copy(hash, file); err != nil {
		return "", fmt.Errorf("failed to compute hash -> %w", err)
	}

	return hex.EncodeToString(hash.Sum(nil)), nil
}

// FindCachedFile searches for a file matching SHA256 in the cache directory,
// Cache file naming format: {basename}-{sha256}.{ext}
func FindCachedFile(cacheDir, fileName, sha256 string) (string, error) {
	if sha256 == "" {
		panic(fmt.Sprintf("no sha256 hash provided for %s/%s", cacheDir, fileName))
	}

	if !PathExists(cacheDir) {
		return "", nil
	}

	// Build cached filename directly: {name}-{sha256}.{ext}
	cachedFileName := fmt.Sprintf("%s-%s%s", Base(fileName), sha256, Ext(fileName))
	cachedFilePath := filepath.Join(cacheDir, cachedFileName)

	// Check if the file exists.
	if !PathExists(cachedFilePath) {
		return "", nil
	}

	color.Printf(color.Title, "\n%s\n", fmt.Sprintf("[validating file cache: %s]", fileName))
	color.PrintInline(color.Hint, "- validating with sha256: %s", sha256)

	// Verify file's sha256.
	computedHash, err := ComputeSHA256(cachedFilePath)
	if err != nil {
		color.PrintInline(color.Hint, "✘ validate with sha256: %s\n", sha256)
		return "", fmt.Errorf("failed to compute sha-256 for cached file -> %w", err)
	}
	if computedHash == sha256 {
		color.PrintInline(color.Hint, "✔ validate with sha256: %s\n", sha256)
		return cachedFilePath, nil
	}

	color.PrintInline(color.Hint, "✘ validate with sha256: %s\n", sha256)
	return "", nil
}

// SaveCachedFile saves a downloaded file to the cache directory using SHA256 in the filename.
func SaveCachedFile(srcFile, cacheDir, fileName, sha256 string, chattrFS *ChattrFS) (string, error) {
	if sha256 == "" {
		panic(fmt.Sprintf("no sha-256 provided when caching file to pkgcache for %s", fileName))
	}

	if !PathExists(cacheDir) {
		if err := chattrFS.MkdirAll(cacheDir, CacheDirPerm); err != nil {
			return "", fmt.Errorf("failed to create cache dir -> %w", err)
		}
	}

	cachedFileName := fmt.Sprintf("%s-%s%s", Base(fileName), sha256, Ext(fileName))
	cachedFilePath := filepath.Join(cacheDir, cachedFileName)

	// If cache file exists and SHA256 matches, return it directly.
	if PathExists(cachedFilePath) {
		computedHash, err := ComputeSHA256(cachedFilePath)
		if err != nil {
			return "", fmt.Errorf("failed to computer hash for cached file: %s -> %w", cachedFilePath, err)
		}
		if computedHash == sha256 {
			return cachedFilePath, nil
		}
	}

	// Copy file to cache (overwrite in-place if it exists, compatible with chattr +a).
	if err := chattrFS.CopyFile(srcFile, cachedFilePath); err != nil {
		return "", fmt.Errorf("failed to copy file to cache -> %w", err)
	}

	return cachedFilePath, nil
}

// verifySHA256 verifies if a file's SHA256 matches the expected hash.
func verifySHA256(filePath, expectedHash string) bool {
	if expectedHash == "" {
		panic("no sha256 provided for " + filePath)
	}

	computedHash, err := ComputeSHA256(filePath)
	if err != nil {
		return false
	}

	return computedHash == expectedHash
}
