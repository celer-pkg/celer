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
	"github.com/celer-pkg/celer/pkgs/expr"
)

type downloader struct {
	url        string
	downloads  string
	archive    string
	maxRetries int
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
		return "", fmt.Errorf("cannot clean tmp files dir -> %w", err)
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
	startTime    time.Time
	lastTime     time.Time
	lastSize     int64
	started      bool
}

func NewProgressBar(fileName string, fileSize int64) *progressBar {
	now := time.Now()
	return &progressBar{
		fileName:  fileName,
		fileSize:  fileSize,
		width:     50,
		startTime: now,
		lastTime:  now,
	}
}

func (p *progressBar) Write(b []byte) (int, error) {
	n := len(b)
	p.currentSize += int64(n)
	progress := int(float64(p.currentSize*100) / float64(p.fileSize))

	if progress > p.lastProgress {
		p.lastProgress = progress

		// Print a blank line before progress bar if it's the first time to print.
		if !p.started {
			fmt.Println()
			p.started = true
		}

		// Calculate download speed
		now := time.Now()
		elapsedSec := now.Sub(p.startTime).Seconds()
		speed := float64(0)
		if elapsedSec > 0 {
			speed = float64(p.currentSize) / elapsedSec
		}

		// Calculate ETA
		eta := ""
		if speed > 0 && p.currentSize < p.fileSize {
			remainingBytes := float64(p.fileSize - p.currentSize)
			remainingSec := remainingBytes / speed
			eta = formatDuration(int64(remainingSec))
		}

		// Format speed with appropriate units
		speedStr := expr.FormatSize(int64(speed)) + "/s"

		// Build progress bar (20 characters width)
		barWidth := 20
		filledWidth := (progress * barWidth) / 100
		progressBar := ""
		for i := range barWidth {
			if i < filledWidth {
				progressBar += "█"
			} else {
				progressBar += "░"
			}
		}

		// Build compact progress display
		var content string
		if eta != "" {
			content = fmt.Sprintf("- downloading: %s [%s] %d%% (%s ETA:%s)",
				p.fileName,
				progressBar,
				progress,
				speedStr,
				eta,
			)
		} else {
			content = fmt.Sprintf("- downloading: %s [%s] %d%% (%s)",
				p.fileName,
				progressBar,
				progress,
				speedStr,
			)
		}

		color.PrintInline(color.Hint, "%s", content)
		if progress == 100 {
			totalSec := time.Since(p.startTime).Seconds()
			color.PrintInline(color.Hint, "✔ downloaded %s (%s) in %s\n",
				p.fileName,
				expr.FormatSize(p.fileSize),
				formatDuration(int64(totalSec)),
			)
		}
	}

	return n, nil
}

// formatDuration converts seconds to a human-readable format (e.g., "2m 30s", "45s")
func formatDuration(seconds int64) string {
	if seconds < 60 {
		return fmt.Sprintf("%ds", seconds)
	}

	minutes := seconds / 60
	secs := seconds % 60
	if minutes < 60 {
		return fmt.Sprintf("%dm %ds", minutes, secs)
	}

	hours := minutes / 60
	mins := minutes % 60
	return fmt.Sprintf("%dh %dm %ds", hours, mins, secs)
}
