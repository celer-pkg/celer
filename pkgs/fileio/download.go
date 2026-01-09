package fileio

import (
	"celer/pkgs/color"
	"celer/pkgs/dirs"
	"celer/pkgs/expr"
	"fmt"
	"io"
	"net/http"
	"net/url"
	"os"
	"path"
	"path/filepath"
	"regexp"
	"time"
)

type downloader struct {
	url     string
	archive string
}

func (d *downloader) SetArchive(archive string) *downloader {
	d.archive = archive
	return d
}

func (d downloader) Start(httpClient *http.Client) (downloaded string, err error) {
	return d.startWithRetry(httpClient, 3)
}

func (d downloader) startWithRetry(httpClient *http.Client, maxRetries int) (downloaded string, err error) {
	var lastErr error
	for attempt := 1; attempt <= maxRetries; attempt++ {
		downloaded, err = d.startOnce(httpClient)
		if err == nil {
			return downloaded, nil
		}
		lastErr = err
		color.Printf(color.Warning, "-- Download failed (attempt %d/%d): %v\n", attempt, maxRetries, err)
		if attempt < maxRetries {
			time.Sleep(time.Duration(attempt) * time.Second) // Exponential backoff.
		}
	}
	return "", fmt.Errorf("download failed after %d attempts.\n %w", maxRetries, lastErr)
}

func (d downloader) startOnce(httpClient *http.Client) (downloaded string, err error) {
	req, err := http.NewRequest("GET", d.url, nil)
	if err != nil {
		return "", fmt.Errorf("create request: %w", err)
	}

	// Simulate a browser-like User-Agent header.
	req.Header.Set("User-Agent", "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/124.0.0.0 Safari/537.36")
	req.Header.Set("Accept", "*/*")
	req.Header.Set("Connection", "keep-alive")

	// Do http request.
	resp, err := httpClient.Do(req)
	if err != nil {
		return "", err
	}
	defer resp.Body.Close()

	// Check if url valid.
	if resp.StatusCode != http.StatusOK {
		return "", fmt.Errorf("unexpected status: %s", resp.Status)
	}

	// Get file name.
	fileName, err := getFileName(d.url)
	if err != nil {
		return "", err
	}

	// Create clean temp directory.
	if err := dirs.CleanTmpFilesDir(); err != nil {
		return "", fmt.Errorf("cannot create clean tmp dir: %w", err)
	}

	// Build download file path.
	tmpFile := filepath.Join(dirs.TmpFilesDir, fmt.Sprintf("%d_%s", time.Now().Unix(), fileName))
	file, err := os.Create(tmpFile)
	if err != nil {
		return "", err
	}

	// Copy to local file with progress.
	progress := NewProgressBar(fileName, resp.ContentLength)
	if _, err := io.Copy(io.MultiWriter(file, progress), resp.Body); err != nil {
		file.Close()
		return "", err
	}

	// Close file before moving it.
	if err := file.Close(); err != nil {
		return "", err
	}

	// Move temp file to downloaded directory.
	downloaded = filepath.Join(dirs.DownloadedDir, fileName)
	if err := os.MkdirAll(filepath.Dir(downloaded), os.ModePerm); err != nil {
		return "", err
	}
	if err := os.Rename(tmpFile, downloaded); err != nil {
		return "", err
	}

	// Rename downloaded file if specified and not same as downloaded file.
	if d.archive != "" && d.archive != fileName {
		renamedFile := filepath.Join(dirs.DownloadedDir, d.archive)
		if err := os.Rename(downloaded, renamedFile); err != nil {
			return "", err
		}
		downloaded = renamedFile
	}

	return downloaded, nil
}

func getFileName(downloadURL string) (string, error) {
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

type progressBar struct {
	fileName     string
	fileSize     int64
	currentSize  int64
	width        int
	lastProgress int
}

func NewProgressBar(fileName string, fileSize int64) *progressBar {
	return &progressBar{
		fileName: fileName,
		fileSize: fileSize,
		width:    50,
	}
}

func (p *progressBar) Write(b []byte) (int, error) {
	n := len(b)
	p.currentSize += int64(n)
	progress := int(float64(p.currentSize*100) / float64(p.fileSize))

	if progress > p.lastProgress {
		p.lastProgress = progress

		content := fmt.Sprintf("Downloading: %s -------- %d%% (%s/%s)",
			p.fileName,
			progress,
			expr.FormatSize(p.currentSize),
			expr.FormatSize(p.fileSize),
		)

		expr.PrintInline(content)
		if progress == 100 {
			fmt.Println()
		}
	}

	return n, nil
}
