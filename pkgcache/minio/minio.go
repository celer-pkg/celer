//go:generate mockgen -destination=mocks/minio_client_mock.go -package=mocks github.com/minio/minio-go/v7 Client
package minio

import (
	"fmt"
	"io"
	"os"
	"strings"
	"sync"

	"github.com/celer-pkg/celer/context"
	"github.com/celer-pkg/celer/pkgcache"
	"github.com/celer-pkg/celer/pkgs/fileio"
	"github.com/celer-pkg/celer/pkgs/logger"
	"github.com/minio/minio-go/v7"
	"github.com/minio/minio-go/v7/pkg/credentials"
)

// Default bucket name for celer.
const bucketName = "celer-cache"

func InitPkgCache(ctx context.Context) (*pkgcache.CacheConfigs, error) {
	pkgCacheConfig := ctx.PkgCache()
	if pkgCacheConfig == nil {
		return nil, nil
	}

	minioConfig := pkgCacheConfig.GetMinio()
	if minioConfig == nil {
		return nil, nil
	}

	client, err := NewClient(minioConfig, !ctx.Offline())
	if err != nil {
		return nil, err
	}

	return &pkgcache.CacheConfigs{
		ClientCache:      NewClientCache(ctx, client),
		DownloadCache:    NewDownloadConfig(ctx, client),
		RepoCache:        NewRepoConfig(ctx, client),
		AritifactCache:   NewArtifactConfig(ctx, client),
		PythonWheelCache: NewPythonWheelConfig(ctx, client),
	}, nil
}

// NewClient builds a minio client from an explicit config. probe=false skips the
// BucketExists check, so callers that only read (e.g. `celer client`) treat a missing
// bucket as an empty cache instead of failing at startup.
func NewClient(minioConfig *pkgcache.Minio, probe bool) (*minio.Client, error) {
	// Initialize minio client object.
	client, err := minio.New(minioConfig.Host, &minio.Options{
		Creds:           credentials.NewStaticV4(minioConfig.AccessKey, minioConfig.SecretKey, ""),
		TrailingHeaders: true,
		Secure:          false,
	})
	if err != nil {
		return nil, err
	}

	// Probe the endpoint so config errors surface at startup: minio.New only
	// parses the host without any network I/O.
	if !probe {
		return client, nil
	}

	if _, err := client.BucketExists(context.Background(), bucketName); err != nil {
		resp := minio.ToErrorResponse(err)
		hint := "check pkgcache.minio.host is the S3 API port, not the console port"
		switch {
		case strings.Contains(resp.Message, "API port"):
			hint = fmt.Sprintf("'%s' is the console port, use the S3 API port such as :9000", minioConfig.Host)
		case resp.Code == "SignatureDoesNotMatch" || resp.Code == "InvalidAccessKeyId":
			hint = fmt.Sprintf("secret_key mismatch for access_key '%s' or whitespace around the keys", minioConfig.AccessKey)
		case resp.Code == "AccessDenied":
			hint = fmt.Sprintf("access_key '%s' is not allowed to use the bucket, check its policy", minioConfig.AccessKey)
		}
		return nil, fmt.Errorf("failed to access pkgcache.minio -> %w (%s)", err, hint)
	}

	return client, nil
}

type Minio struct {
	client     *minio.Client
	bucketName string
}

func (m Minio) CreateBucketIfNotExist() error {
	found, err := m.client.BucketExists(context.Background(), m.bucketName)
	if err != nil {
		return err
	}
	if !found {
		// No object locking: the bucket stays mutable so celer can prune expired artifacts.
		opts := minio.MakeBucketOptions{ObjectLocking: false}
		if err := m.client.MakeBucket(context.Background(), m.bucketName, opts); err != nil {
			return err
		}
	}

	return nil
}

func (m Minio) GetFileInfo(objectName string) (*minio.ObjectInfo, error) {
	opts := minio.StatObjectOptions{}
	info, err := m.client.StatObject(context.Background(), m.bucketName, objectName, opts)
	if err != nil {
		resp := minio.ToErrorResponse(err)
		if resp.Code == minio.NoSuchBucket || resp.Code == minio.NoSuchKey {
			return nil, nil
		}
		return nil, err
	}

	return &info, nil
}

// metaSha256 returns the sha256 recorded in the object's user metadata at upload time.
func (m Minio) metaSha256(info *minio.ObjectInfo) string {
	if info == nil {
		return ""
	}
	for k, v := range info.UserMetadata {
		if strings.EqualFold(k, "sha256") {
			return v
		}
	}
	return ""
}

// RemoveObjectVersion removes an object, or one specific version of it when versionID is set.
func (m Minio) RemoveObjectVersion(objectName, versionID string) error {
	// RemoveObjects is a batch API that reads a channel of objects; feed it one
	// buffered item then close, turning it into a synchronous single-object delete.
	objectsCh := make(chan minio.ObjectInfo, 1)
	objectsCh <- minio.ObjectInfo{Key: objectName, VersionID: versionID}
	close(objectsCh)

	options := minio.RemoveObjectsOptions{}
	for removeErr := range m.client.RemoveObjects(context.Background(), m.bucketName, objectsCh, options) {
		if removeErr.Err != nil {
			return fmt.Errorf("failed to remove object '%s' version '%s' -> %w", objectName, versionID, removeErr.Err)
		}
	}
	return nil
}

// ListObjectsWithVersions lists every object version under prefix.
func (m Minio) ListObjectsWithVersions(prefix string) ([]minio.ObjectInfo, error) {
	var objects []minio.ObjectInfo
	opts := minio.ListObjectsOptions{Prefix: prefix, Recursive: true, WithVersions: true}
	for object := range m.client.ListObjects(context.Background(), m.bucketName, opts) {
		if object.Err != nil {
			return nil, object.Err
		}
		objects = append(objects, object)
	}
	return objects, nil
}

func (m Minio) removeObject(objectName string, versionIDs []string) error {
	if len(versionIDs) == 0 {
		return m.RemoveObjectVersion(objectName, "")
	}
	for _, versionID := range versionIDs {
		if err := m.RemoveObjectVersion(objectName, versionID); err != nil {
			return err
		}
	}
	return nil
}

func (m Minio) readObject(tmpDir, objectName string) (string, bool) {
	local, err := m.downloadSilent(tmpDir, objectName)
	if err != nil {
		return "", false
	}
	bytes, err := os.ReadFile(local)
	if err != nil {
		return "", false
	}
	return string(bytes), true
}

func (m Minio) putObject(filePath, objectName string, hook io.Reader) (*minio.UploadInfo, error) {
	file, err := os.Open(filePath)
	if err != nil {
		return nil, err
	}
	defer file.Close()

	fileStat, err := file.Stat()
	if err != nil {
		return nil, err
	}

	sha256Sum, err := fileio.SHA256Sum(filePath)
	if err != nil {
		return nil, err
	}

	// Assemble user meta data.
	opts := minio.PutObjectOptions{
		ContentType: "application/octet-stream",
		UserMetadata: map[string]string{
			"x-amz-meta-sha256": sha256Sum,
		},
		Progress: hook,
	}
	uploadInfo, err := m.client.PutObject(context.Background(), m.bucketName, objectName, file, fileStat.Size(), opts)
	if err != nil {
		return nil, err
	}

	return &uploadInfo, nil
}

// uploadFile uploads filePath to objectName with a progress bar, mirroring the
// restore side's downloadFile presentation.
func (m Minio) uploadFile(filePath, objectName, displayName string) error {
	fileStat, err := os.Stat(filePath)
	if err != nil {
		return err
	}

	title := fmt.Sprintf("uploading '%s'", displayName)
	completed := func(formattedTimeCost, formattedSize string) {
		logger.NewProgressTask(logger.OpStore, displayName).
			Complete("", formattedSize, formattedTimeCost)
	}
	progress := fileio.NewProgressBar(title, fileStat.Size(), completed)
	if _, err := m.putObject(filePath, objectName, &progressHook{writer: progress}); err != nil {
		return err
	}

	return nil
}

// uploadSilent uploads filePath to objectName without any progress output.
func (m Minio) uploadSilent(filePath, objectName string) error {
	_, err := m.putObject(filePath, objectName, nil)
	return err
}

// downloadFile downloads objectName to a task-owned tmp file inside dstDir with a
// progress bar.
func (m Minio) downloadFile(dstDir, objectName, displayName string) (string, error) {
	opts := minio.GetObjectOptions{}
	object, err := m.client.GetObject(context.Background(), m.bucketName, objectName, opts)
	if err != nil {
		return "", fmt.Errorf("failed to get minio object '%s' -> %w", objectName, err)
	}
	defer object.Close()

	localFile, err := os.CreateTemp(dstDir, "celer-pkgcache-*"+fileio.Ext(objectName))
	if err != nil {
		return "", fmt.Errorf("failed to create tmp file in %s -> %w", dstDir, err)
	}
	defer localFile.Close()

	objInfo, err := object.Stat()
	if err != nil {
		return "", fmt.Errorf("failed to get object info '%s' -> %w", objectName, err)
	}

	title := fmt.Sprintf("downloading '%s'", displayName)
	completed := func(formattedTimeCost, formattedSize string) {
		logger.NewProgressTask(logger.OpRestore, displayName).
			Complete("", formattedSize, formattedTimeCost)
	}
	progress := fileio.NewProgressBar(title, objInfo.Size, completed)
	if _, err := io.Copy(io.MultiWriter(localFile, progress), object); err != nil {
		return "", fmt.Errorf("failed to download '%s' -> %w", objectName, err)
	}

	return localFile.Name(), nil
}

// downloadSilent downloads objectName to a task-owned tmp dir.
func (m Minio) downloadSilent(dstDir, objectName string) (string, error) {
	opts := minio.GetObjectOptions{}
	object, err := m.client.GetObject(context.Background(), m.bucketName, objectName, opts)
	if err != nil {
		return "", fmt.Errorf("failed to get minio object '%s' -> %w", objectName, err)
	}
	defer object.Close()

	ext := fileio.Ext(objectName)
	localFile, err := os.CreateTemp(dstDir, "celer-pkgcache-*"+ext)
	if err != nil {
		return "", fmt.Errorf("failed to create tmp file in %s -> %w", dstDir, err)
	}
	defer localFile.Close()

	if _, err := io.Copy(localFile, object); err != nil {
		return "", fmt.Errorf("failed to download '%s' -> %w", objectName, err)
	}

	return localFile.Name(), nil
}

type progressHook struct {
	mutex  sync.Mutex
	writer io.Writer
}

func (h *progressHook) Read(p []byte) (int, error) {
	h.mutex.Lock()
	defer h.mutex.Unlock()

	if _, err := h.writer.Write(p); err != nil {
		return 0, err
	}
	return len(p), nil
}
