package fileio

import (
	"crypto/tls"
	"fmt"
	"net/http"
	"net/url"
	"strings"
	"time"
)

// CheckAccessible checks if the given URL is accessible.
func CheckAccessible(url string) error {
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
func FileSize(httpClient *http.Client, downloadUrl string) (int64, error) {
	resp, err := httpClient.Head(downloadUrl)
	if err != nil {
		return 0, err
	}

	defer resp.Body.Close()
	return resp.ContentLength, nil
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
