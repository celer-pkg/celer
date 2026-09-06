package minio

import (
	"fmt"
	"os"
	"path/filepath"
	"testing"

	"github.com/minio/minio-go/v7"
	"github.com/minio/minio-go/v7/pkg/credentials"
)

const (
	testEndpoint        = "192.168.2.19:9000"
	testAccessKeyID     = "UkGTjvo0bwhwplZJeyrJ"
	testSecretAccessKey = "SBDBhv9Qtw8D6e7ILWWpeX7wZITWCKmNvvuZGwuJ"
)

func createMinioClient() (*minio.Client, error) {
	return minio.New(testEndpoint, &minio.Options{
		Creds:  credentials.NewStaticV4(testAccessKeyID, testSecretAccessKey, ""),
		Secure: false,
	})
}

func TestCreateBucketIfNotExist(t *testing.T) {
	client, err := createMinioClient()
	if err != nil {
		t.Fatal(err)
	}

	minioCache := minioCache{
		client: client,
	}

	if err := minioCache.CreateBucketIfNotExist(); err != nil {
		t.Fatal(err)
	}
}

func TestCheckFileIfExist(t *testing.T) {
	client, err := createMinioClient()
	if err != nil {
		t.Fatal(err)
	}

	minioCache := minioCache{
		client: client,
	}

	info, err := minioCache.GetFileInfo("not_exist_file")
	if err != nil {
		t.Fatal(err)
	}

	t.Logf("exist: %v", info != nil)
}

func TestUploadFile(t *testing.T) {
	client, err := createMinioClient()
	if err != nil {
		t.Fatal(err)
	}

	minioCache := minioCache{
		client: client,
	}

	if err := minioCache.CreateBucketIfNotExist(); err != nil {
		t.Fatal(err)
	}

	currentDir, err := os.Getwd()
	if err != nil {
		panic(fmt.Errorf("cannot get current dir -> %w", err))
	}

	filePath := filepath.Join(currentDir, "testdata", "upload_test.txt")
	info, err := minioCache.UploadFile(filePath, "aaa/bbb/ccc", func(percent int) {
		t.Logf("progress updated: %d", percent)
	})
	if err != nil {
		t.Fatal(err)
	}

	t.Logf("upload success: %s", info.Location)
}

func TestRemvoeFile(t *testing.T) {

}

func TestProgressWriterIsPassiveHook(t *testing.T) {
	var lastPercent int
	pw := &ProgressWriter{
		total:    10,
		progress: func(p int) { lastPercent = p },
	}

	chunk := []byte("0123456789")
	n, err := pw.Read(chunk)
	if err != nil {
		t.Fatal(err)
	}
	if n != len(chunk) {
		t.Fatalf("expected n = %d, got %d", len(chunk), n)
	}
	if string(chunk) != "0123456789" {
		t.Fatalf("hook must not modify the data buffer, got %q", chunk)
	}
	if lastPercent != 100 {
		t.Fatalf("expected progress 100, got %d", lastPercent)
	}

	// A second call (e.g. multipart parts) must not error or read anything.
	n, err = pw.Read(chunk)
	if err != nil {
		t.Fatal(err)
	}
	if n != len(chunk) {
		t.Fatalf("expected n = %d, got %d", len(chunk), n)
	}
}
