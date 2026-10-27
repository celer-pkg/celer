package fileio

import (
	"errors"
	"fmt"
	"io"
	"net/http"
	"net/url"
	"os"
	"path"
	"path/filepath"
	"regexp"
	"time"

	"github.com/celer-pkg/celer/pkgs/dirs"
	"github.com/celer-pkg/celer/pkgs/logger"
)

const (
	defaultUserAgent  = "celer"
	fallbackUserAgent = "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/131.0.0.0 Safari/537.36"
)

type httpStatusError struct {
	code   int
	status string
}

func (e httpStatusError) Error() string {
	return fmt.Sprintf("unexpected status: %s", e.status)
}

type downloader struct {
	url        string
	downloads  string
	archive    string
	tmpDir     string
	maxRetries int
	headers    map[string]string
}

func NewDownloader(url, downloads string) *downloader {
	return &downloader{
		url:        url,
		downloads:  downloads,
		maxRetries: 3,
	}
}

func (d *downloader) SetArchive(archive string) *downloader {
	d.archive = archive
	return d
}

func (d *downloader) SetMaxRetries(maxRetries int) *downloader {
	d.maxRetries = maxRetries
	return d
}

func (d *downloader) SetTmpDir(tmpDir string) *downloader {
	d.tmpDir = tmpDir
	return d
}

// AppendHeader adds a custom header to the download request.
func (d *downloader) AppendHeader(key, value string) {
	if d.headers == nil {
		d.headers = make(map[string]string)
	}
	d.headers[key] = value
}

func (d downloader) Start(httpClient *http.Client) (downloaded string, err error) {
	var lastErr error
	for attempt := 1; attempt <= d.maxRetries; attempt++ {
		downloaded, err = d.startOnce(httpClient, defaultUserAgent)

		// Some hosts reject unknown clients with 403. A real browser UA usually
		// passes, so give it one try before counting this attempt as failed.
		var statusErr *httpStatusError
		if errors.As(err, &statusErr) && statusErr.code == http.StatusForbidden {
			logger.PrintWarning("Download rejected with 403, retrying with a browser User-Agent")
			downloaded, err = d.startOnce(httpClient, fallbackUserAgent)
		}

		if err == nil {
			return downloaded, nil
		}

		lastErr = err
		logger.Printf(logger.Warning, "Download failed (attempt %d/%d): %v\n", attempt, d.maxRetries, err)
		if attempt < d.maxRetries {
			time.Sleep(time.Duration(attempt) * time.Second) // Exponential backoff.
		}
	}
	return "", fmt.Errorf("download failed after %d attempts -> %w", d.maxRetries, lastErr)
}

func (d downloader) startOnce(httpClient *http.Client, userAgent string) (downloaded string, err error) {
	req, err := http.NewRequest("GET", d.url, nil)
	if err != nil {
		return "", fmt.Errorf("create request -> %w", err)
	}

	req.Header.Set("User-Agent", userAgent)
	req.Header.Set("Accept", "*/*")
	req.Header.Set("Connection", "keep-alive")
	for k, v := range d.headers {
		req.Header.Set(k, v)
	}

	// Do http request.
	resp, err := httpClient.Do(req)
	if err != nil {
		return "", err
	}
	defer resp.Body.Close()

	// Check if url valid.
	if resp.StatusCode != http.StatusOK {
		return "", &httpStatusError{code: resp.StatusCode, status: resp.Status}
	}

	// Get file name.
	fileName, err := d.getFileName(d.url)
	if err != nil {
		return "", err
	}

	// Download into a task-owned tmp dir so concurrent downloads never clash.
	var downloadTmpDir string
	if d.tmpDir != "" {
		downloadTmpDir, err = os.MkdirTemp(d.tmpDir, "")
	} else {
		downloadTmpDir, err = dirs.NewTmpFilesDir()
	}
	if err != nil {
		return "", fmt.Errorf("cannot create download tmp dir -> %w", err)
	}
	defer os.RemoveAll(downloadTmpDir)

	tmpFile := filepath.Join(downloadTmpDir, fileName)
	file, err := os.Create(tmpFile)
	if err != nil {
		return "", err
	}

	// Copy to local file with progress.
	completed := func(formattedTimeCost, formattedSize string) {
		NewProgressTask(OpDownload, d.archive).
			Complete("", formattedSize, formattedTimeCost)
	}
	progress := NewProgressBar("download: "+fileName, resp.ContentLength, completed)
	if _, err := io.Copy(io.MultiWriter(file, progress), resp.Body); err != nil {
		file.Close()
		return "", err
	}

	// Close file before moving it.
	if err := file.Close(); err != nil {
		return "", err
	}

	// Move temp file to downloaded directory.
	downloaded = filepath.Join(d.downloads, fileName)
	if err := os.MkdirAll(filepath.Dir(downloaded), os.ModePerm); err != nil {
		return "", err
	}
	if err := os.Rename(tmpFile, downloaded); err != nil {
		return "", err
	}

	// Rename downloaded file if specified and not same as downloaded file.
	if d.archive != "" && d.archive != fileName {
		renamedFile := filepath.Join(d.downloads, d.archive)
		if err := os.Rename(downloaded, renamedFile); err != nil {
			return "", err
		}
		downloaded = renamedFile
	}

	return downloaded, nil
}

func (d downloader) getFileName(downloadURL string) (string, error) {
	// Read file name from URL.
	u, err := url.Parse(downloadURL)
	if err != nil {
		return "", err
	}
	filename := path.Base(u.Path)
	if filename != "." && filename != "/" {
		return filename, nil
	}

	// Read file name from http header.
	resp, err := http.Head(downloadURL)
	if err != nil {
		return "", err
	}
	defer resp.Body.Close()

	re := regexp.MustCompile(`filename=["]?([^"]+)["]?`)
	header := resp.Header.Get("Content-Disposition")
	match := re.FindStringSubmatch(header)
	if len(match) > 1 {
		return match[1], nil
	}
	return "", nil
}
