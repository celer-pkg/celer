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
	"github.com/celer-pkg/celer/pkgs/color"
	"github.com/celer-pkg/celer/pkgs/dirs"
	"github.com/celer-pkg/celer/pkgs/fileio"
	"github.com/minio/minio-go/v7"
	"github.com/minio/minio-go/v7/pkg/credentials"
)

// Default bucket name for celer.
const bucketName = "celer-cache"

func InitPkgCache(ctx context.Context) (pkgcache.DownloadCache, pkgcache.RepoCache, pkgcache.AritifactCache, error) {
	pkgCacheConfig := ctx.PkgCache()
	if pkgCacheConfig == nil {
		return nil, nil, nil, nil
	}

	minioConfig := pkgCacheConfig.GetMinio()
	if minioConfig == nil {
		return nil, nil, nil, nil
	}

	// Initialize minio client object.
	client, err := minio.New(minioConfig.Host, &minio.Options{
		Creds:           credentials.NewStaticV4(minioConfig.AccessKey, minioConfig.SecretKey, ""),
		TrailingHeaders: true,
		Secure:          false,
	})
	if err != nil {
		return nil, nil, nil, err
	}

	// Probe the endpoint so config errors surface at startup: minio.New only
	// parses the host without any network I/O。
	if !ctx.Offline() {
		if _, err := client.BucketExists(context.Background(), bucketName); err != nil {
			resp := minio.ToErrorResponse(err)
			hint := "check that pkgcache.minio.host is reachable and points to the S3 API port, not the minio console port"
			if strings.Contains(resp.Message, "API port") {
				hint = fmt.Sprintf("'%s' points to the minio console port, use the S3 API port instead (e.g. :9000 instead of :9001)", minioConfig.Host)
			}
			return nil, nil, nil, fmt.Errorf("failed to reach pkgcache.minio at '%s' -> %w (%s)", minioConfig.Host, err, hint)
		}
	}

	return NewDownloadConfig(ctx, client), NewRepoConfig(ctx, client), NewArtifactConfig(ctx, client), nil
}

type minioCache struct {
	client     *minio.Client
	bucketName string
}

func (m minioCache) CreateBucketIfNotExist() error {
	found, err := m.client.BucketExists(context.Background(), m.bucketName)
	if err != nil {
		return err
	}
	if !found {
		opts := minio.MakeBucketOptions{ObjectLocking: true}
		if err := m.client.MakeBucket(context.Background(), m.bucketName, opts); err != nil {
			return err
		}
	}

	return nil
}

func (m minioCache) GetFileInfo(objectName string) (*minio.ObjectInfo, error) {
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
func (m minioCache) metaSha256(info *minio.ObjectInfo) string {
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

func (m minioCache) putObject(filePath, objectName string, hook io.Reader) (*minio.UploadInfo, error) {
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
func (m minioCache) uploadFile(kind pkgcache.Kind, filePath, objectName, displayName string) error {
	fileStat, err := os.Stat(filePath)
	if err != nil {
		return err
	}

	title := fmt.Sprintf("uploading '%s'", displayName)
	completed := func(formattedTimeCost, formattedSize string) {
		color.PrintInline(color.Success, "[✔] %-18s %-22s (%s) (%s)\n",
			fmt.Sprintf("[Store %s]", kind), displayName, formattedSize, formattedTimeCost)
	}
	progress := fileio.NewProgressBar(title, fileStat.Size(), completed)
	if _, err := m.putObject(filePath, objectName, &progressHook{writer: progress}); err != nil {
		return err
	}

	return nil
}

// uploadSilent uploads filePath to objectName without any progress output.
func (m minioCache) uploadSilent(filePath, objectName string) error {
	_, err := m.putObject(filePath, objectName, nil)
	return err
}

// downloadFile downloads objectName with a progress bar.
func (m minioCache) downloadFile(kind pkgcache.Kind, objectName, displayName string) (string, error) {
	opts := minio.GetObjectOptions{}
	object, err := m.client.GetObject(context.Background(), m.bucketName, objectName, opts)
	if err != nil {
		return "", fmt.Errorf("failed to get minio object '%s' -> %w", objectName, err)
	}
	defer object.Close()

	if err := os.MkdirAll(dirs.TmpFilesDir, os.ModePerm); err != nil {
		return "", fmt.Errorf("failed to mkdir for '%s' -> %w", dirs.TmpFilesDir, err)
	}

	localFile, err := os.CreateTemp(dirs.TmpFilesDir, "celer-pkgcache-*"+fileio.Ext(objectName))
	if err != nil {
		return "", fmt.Errorf("failed to create tmp file in %s -> %w", dirs.TmpFilesDir, err)
	}
	defer localFile.Close()

	objInfo, err := object.Stat()
	if err != nil {
		return "", fmt.Errorf("failed to get object info '%s' -> %w", objectName, err)
	}

	title := fmt.Sprintf("downloading '%s'", displayName)
	completed := func(formattedTimeCost, formattedSize string) {
		color.PrintInline(color.Success, "[✔] %-14s %-22s %s (%s)\n",
			fmt.Sprintf("[Restore %s]", kind), displayName, formattedSize, formattedTimeCost)
	}
	progress := fileio.NewProgressBar(title, objInfo.Size, completed)
	if _, err := io.Copy(io.MultiWriter(localFile, progress), object); err != nil {
		return "", fmt.Errorf("failed to download '%s' -> %w", objectName, err)
	}

	return localFile.Name(), nil
}

// downloadSilent downloads objectName to a tmp file without any progress output.
func (m minioCache) downloadSilent(objectName string) (string, error) {
	opts := minio.GetObjectOptions{}
	object, err := m.client.GetObject(context.Background(), m.bucketName, objectName, opts)
	if err != nil {
		return "", fmt.Errorf("failed to get minio object '%s' -> %w", objectName, err)
	}
	defer object.Close()

	ext := fileio.Ext(objectName)
	localFile, err := os.CreateTemp(dirs.TmpFilesDir, "celer-pkgcache-*"+ext)
	if err != nil {
		return "", fmt.Errorf("failed to create tmp file in %s -> %w", dirs.TmpFilesDir, err)
	}
	defer localFile.Close()

	if _, err := io.Copy(localFile, object); err != nil {
		return "", fmt.Errorf("failed to download '%s' -> %w", objectName, err)
	}

	return localFile.Name(), nil
}

func (m minioCache) RemoveFile(filePath string) error {
	opts := minio.RemoveObjectOptions{
		GovernanceBypass: true,
		ForceDelete:      true,
	}
	if err := m.client.RemoveObject(context.Background(), m.bucketName, filePath, opts); err != nil {
		return err
	}

	return nil
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
