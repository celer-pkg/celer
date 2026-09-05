//go:generate mockgen -destination=mocks/minio_client_mock.go -package=mocks github.com/minio/minio-go/v7 Client
package minio

import (
	"fmt"
	"io"
	"os"
	"strings"

	"github.com/celer-pkg/celer/context"
	"github.com/celer-pkg/celer/pkgcache"
	"github.com/celer-pkg/celer/pkgs/dirs"
	"github.com/celer-pkg/celer/pkgs/fileio"
	"github.com/minio/minio-go/v7"
	"github.com/minio/minio-go/v7/pkg/credentials"
)

// Default bucket name for celer.
const bucketName = "celer-cache"

type Progress func(percent int)

func InitPkgCache(ctx context.Context) (pkgcache.DownloadCache, pkgcache.RepoCache, pkgcache.AritifactCache, error) {
	pkgCache := ctx.PkgCache()
	if pkgCache == nil {
		return nil, nil, nil, nil
	}

	minioConfig := pkgCache.GetMinio()
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

	writable := pkgCache.GetOptions().Writable

	downloadConfig := DownloadConfig{
		ctx: ctx,
		minioCache: minioCache{
			client:     client,
			bucketName: bucketName,
		},
		cacheDir: minioConfig.GetDir(pkgcache.DirDownloads, ctx.Version()),
		writable: writable,
	}

	artifactConfig := ArtifactConfig{
		ctx: ctx,
		minioCache: minioCache{
			client:     client,
			bucketName: bucketName,
		},
		cacheDir:   minioConfig.GetDir(pkgcache.DirArtifacts, ctx.Version()),
		writable:   writable,
		maxRetries: 3,
	}

	repoConfig := RepoConfig{
		ctx: ctx,
		minioCache: minioCache{
			client:     client,
			bucketName: bucketName,
		},
		cacheDir: minioConfig.GetDir(pkgcache.DirRepos, ctx.Version()),
		writable: writable,
	}

	return &downloadConfig, &repoConfig, &artifactConfig, nil
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

func (m minioCache) UploadFile(filePath, objectName string, progress Progress) (info *minio.UploadInfo, err error) {
	file, err := os.Open(filePath)
	if err != nil {
		return nil, err
	}
	defer file.Close()

	fileStat, err := file.Stat()
	if err != nil {
		return nil, err
	}

	progressReader := &ProgressWriter{
		total:    fileStat.Size(),
		progress: progress,
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
		Progress: progressReader,
	}
	uploadInfo, err := m.client.PutObject(context.Background(), m.bucketName, objectName, file, fileStat.Size(), opts)
	if err != nil {
		return nil, err
	}

	return &uploadInfo, nil
}

// DownloadFile download file from minio into tmp dir, you may need to rename
// to your destination later after verifcation.
func (m minioCache) DownloadFile(objectName string) (string, error) {
	opts := minio.GetObjectOptions{}
	object, err := m.client.GetObject(context.Background(), m.bucketName, objectName, opts)
	if err != nil {
		return "", err
	}
	defer object.Close()

	ext := fileio.Ext(objectName)
	localFile, err := os.CreateTemp(dirs.TmpFilesDir, "celer-pkgcache-*"+ext)
	if err != nil {
		return "", err
	}
	defer localFile.Close()

	if _, err = io.Copy(localFile, object); err != nil {
		return "", err
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

type ProgressWriter struct {
	total        int64
	readSize     int64
	lastProgress int
	progress     Progress
}

func (pw *ProgressWriter) Read(p []byte) (int, error) {
	pw.readSize += int64(len(p))

	var percent int
	if pw.total > 0 {
		percent = int(float64(pw.readSize) / float64(pw.total) * 100)
	}
	if percent > pw.lastProgress {
		pw.lastProgress = percent

		if pw.progress != nil {
			pw.progress(pw.lastProgress)
		}
	}
	return len(p), nil
}
