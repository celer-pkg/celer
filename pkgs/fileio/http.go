package fileio

import (
	"fmt"
	"net/http"
	"net/url"
	"strings"
)

var (
	ProxyAddress string
	ProxyPort    int
)

// CheckAvailable checks if the given URL is accessible.
func CheckAvailable(filePath string) error {
	if strings.HasPrefix(filePath, "file:///") {
		filePath = strings.TrimPrefix(filePath, "file:///")
		if !PathExists(filePath) {
			return fmt.Errorf("file not exists: %s", filePath)
		}

		return nil
	}

	client := http.DefaultClient

	// Check URL availability using HEAD request.
	resp, err := client.Head(filePath)
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
func FileSize(downloadUrl string) (int64, error) {
	var client *http.Client
	if ProxyAddress != "" && ProxyPort != 0 {
		client = &http.Client{
			Transport: &http.Transport{
				Proxy: http.ProxyURL(&url.URL{
					Scheme: "http",
					Host:   fmt.Sprintf("%s:%d", ProxyAddress, ProxyPort),
				}),
			},
		}
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
