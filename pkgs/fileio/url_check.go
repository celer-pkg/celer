package fileio

import (
	"crypto/tls"
	"fmt"
	"net"
	"net/http"
	"net/url"
	"strings"
	"time"

	"github.com/celer-pkg/celer/pkgs/color"
)

// CheckAccessible checks if the given URL is accessible,
// the url can be "http://", "https://", "ssh@" and "ftp://".
func CheckAccessible(url string) error {
	if after, ok := strings.CutPrefix(url, "file:///"); ok {
		url = after
		if !PathExists(url) {
			return fmt.Errorf("file not exists: %s", url)
		}

		return nil
	}

	switch {
	case strings.HasPrefix(url, "ssh://"):
		return checkSSHAccessible(url)

	case strings.HasPrefix(url, "http://"), strings.HasPrefix(url, "https://"):
		return checkHTTPAccessible(url)

	case strings.HasPrefix(url, "ftp://"):
		return checkFTPAccessible(url)

	default:
		return fmt.Errorf("unsupported url format: %s", url)
	}
}

// FileSize returns the size of the file at the given URL.
func FileSize(httpClient *http.Client, downloadUrl string) (int64, error) {
	const maxRetries = 3
	var lastErr error
	for attempt := 1; attempt <= maxRetries; attempt++ {
		resp, err := httpClient.Head(downloadUrl)
		if err == nil {
			resp.Body.Close()
			return resp.ContentLength, nil
		}

		lastErr = err
		color.Printf(color.Warning, "-- Get filesize failed (attempt %d/%d): %v\n", attempt, maxRetries, err)
		if attempt < maxRetries {
			time.Sleep(time.Duration(attempt) * time.Second) // Exponential backoff.
		}
	}

	return 0, lastErr
}

func httpClient(host string, port int) *http.Client {
	if host == "" || port == 0 {
		return http.DefaultClient
	}

	return &http.Client{
		Transport: &http.Transport{
			Proxy: http.ProxyURL(&url.URL{
				Scheme: "http",
				Host:   fmt.Sprintf("%s:%d", host, port),
			}),
			TLSClientConfig: &tls.Config{
				InsecureSkipVerify: false,
			},
		},
	}
}

func checkHTTPAccessible(httpUrl string) error {
	client := http.Client{
		Timeout: 3 * time.Second,
		CheckRedirect: func(req *http.Request, via []*http.Request) error {
			return nil
		},
	}

	// Check URL availability using HEAD request.
	resp, err := client.Head(httpUrl)
	if err != nil {
		return err
	}
	defer resp.Body.Close()

	// Check if the status code is in the 2xx range.
	if resp.StatusCode >= 200 && resp.StatusCode < 400 {
		return nil
	}

	return fmt.Errorf("status code: %d", resp.StatusCode)
}

func checkSSHAccessible(sshURL string) error {
	parts := strings.SplitN(strings.TrimPrefix(sshURL, "ssh://"), "/", 2)
	hostPort := strings.SplitN(parts[0], "@", 2)

	// Get address with format: "host:port"
	addrPort := hostPort[len(hostPort)-1]
	if !strings.Contains(addrPort, ":") {
		addrPort += ":22" // default ssh port.
	}

	// Test availability.
	conn, err := net.DialTimeout("tcp", addrPort, 3*time.Second)
	if err != nil {
		return fmt.Errorf("SSH unreachable: %w", err)
	}
	conn.Close()
	return nil
}

func checkFTPAccessible(ftpURL string) error {
	parts := strings.SplitN(strings.TrimPrefix(ftpURL, "ftp://"), "/", 2)
	hostPort := strings.SplitN(parts[0], "@", 2)

	// Get address with format: "host:port"
	addrPort := hostPort[len(hostPort)-1]
	if !strings.Contains(addrPort, ":") {
		addrPort += ":21" // default ftp port.
	}

	// Test availability.
	conn, err := net.DialTimeout("tcp", addrPort, 3*time.Second)
	if err != nil {
		return fmt.Errorf("FTP unreachable: %w", err)
	}
	conn.Close()
	return nil
}
