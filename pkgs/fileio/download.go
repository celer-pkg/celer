package fileio

import (
	"fmt"
	"io"
	"net/http"
	"net/url"
	"os"
	"path"
	"path/filepath"
	"regexp"
	"time"

	"github.com/celer-pkg/celer/pkgs/color"
	"github.com/celer-pkg/celer/pkgs/dirs"
)

type downloader struct {
	url        string
	downloads  string
	archive    string
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

func (d *downloader) WithArchive(archive string) {
	d.archive = archive
}

func (d *downloader) WithMaxRetries(maxRetries int) {
	d.maxRetries = maxRetries
}

// WithHeader adds a custom header to the download request.
func (d *downloader) WithHeader(key, value string) {
	if d.headers == nil {
		d.headers = make(map[string]string)
	}
	d.headers[key] = value
}

func (d downloader) Start(httpClient *http.Client) (downloaded string, err error) {
	var lastErr error
	for attempt := 1; attempt <= d.maxRetries; attempt++ {
		downloaded, err = d.startOnce(httpClient)
		if err == nil {
			return downloaded, nil
		}

		lastErr = err
		color.Printf(color.Warning, "Download failed (attempt %d/%d): %v\n", attempt, d.maxRetries, err)
		if attempt < d.maxRetries {
			time.Sleep(time.Duration(attempt) * time.Second) // Exponential backoff.
		}
	}
	return "", fmt.Errorf("download failed after %d attempts -> %w", d.maxRetries, lastErr)
}

func (d downloader) startOnce(httpClient *http.Client) (downloaded string, err error) {
	req, err := http.NewRequest("GET", d.url, nil)
	if err != nil {
		return "", fmt.Errorf("create request -> %w", err)
	}

	// Simulate a browser-like User-Agent header.
	req.Header.Set("User-Agent", "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/124.0.0.0 Safari/537.36")
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
		return "", fmt.Errorf("unexpected status: %s", resp.Status)
	}

	// Get file name.
	fileName, err := d.getFileName(d.url)
	if err != nil {
		return "", err
	}

	// Ensure tmp files dir exists always.
	if err := os.MkdirAll(dirs.TmpFilesDir, os.ModePerm); err != nil {
		return "", fmt.Errorf("cannot create tmp files dir -> %w", err)
	}
	tmpFile := filepath.Join(dirs.TmpFilesDir, fmt.Sprintf("%d_%s", time.Now().UnixNano(), fileName))
	file, err := os.Create(tmpFile)
	if err != nil {
		return "", err
	}

	// Copy to local file with progress.
	completed := func(formattedTimeCost, formattedSize string) {
		color.PrintInline(color.Hint, "[✔] %s (%s) in %s\n", d.archive+" is downloaded", formattedSize, formattedTimeCost)
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
