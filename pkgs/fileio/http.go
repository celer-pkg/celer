package fileio

import (
	"celer/pkgs/proxy"
	"fmt"
	"net/http"
	"strings"
	"time"
)

// CheckAvailable checks if the given URL is accessible.
func CheckAvailable(url string) error {
	if after, ok := strings.CutPrefix(url, "file:///"); ok {
		url = after
		if !PathExists(url) {
			return fmt.Errorf("file not exists: %s", url)
		}

		return nil
	}

	client := http.Client{
		Timeout: 3 * time.Second,
		CheckRedirect: func(req *http.Request, via []*http.Request) error {
			return nil
		},
	}

	// Check URL availability using HEAD request.
	resp, err := client.Head(url)
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

// FileSize returns the size of the file at the given URL.
func FileSize(proxy *proxy.Proxy, downloadUrl string) (int64, error) {
	var client *http.Client
	if proxy != nil {
		client = proxy.HttpClient()
	} else {
		client = http.DefaultClient
	}

	resp, err := client.Head(downloadUrl)
	if err != nil {
		return 0, err
	}
	defer resp.Body.Close()

	return resp.ContentLength, nil
}
