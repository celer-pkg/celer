package fileio

import (
	"fmt"
	"net/http"
	"net/http/httptest"
	"os"
	"strings"
	"testing"
)

// TestDownloadRetrySuccess tests that download succeeds on first attempt.
func TestDownloadRetrySuccess(t *testing.T) {
	// Create a test server that returns success.
	server := httptest.NewServer(http.HandlerFunc(func(w http.ResponseWriter, r *http.Request) {
		w.WriteHeader(http.StatusOK)
		w.Write([]byte("test content"))
	}))
	defer server.Close()

	// Create downloader.
	download := downloader{
		Url:        server.URL + "/test.txt",
		Archive:    "test.txt",
		MaxRetries: 3,
	}

	client := &http.Client{}
	downloaded, err := download.Start(client)

	if err != nil {
		t.Fatalf("Expected success, got error: %v", err)
	}

	if downloaded == "" {
		t.Fatal("Expected downloaded file path, got empty string")
	}

	// Clean up.
	os.Remove(downloaded)
}

// TestDownloadRetryFailureCount tests that download retries exactly maxRetries times.
func TestDownloadRetryFailureCount(t *testing.T) {
	attemptCount := 0

	// Create a test server that always returns 500 error.
	server := httptest.NewServer(http.HandlerFunc(func(w http.ResponseWriter, r *http.Request) {
		attemptCount++
		w.WriteHeader(http.StatusInternalServerError)
		w.Write([]byte("server error"))
	}))
	defer server.Close()

	// Create downloader.
	maxRetries := 3
	download := downloader{
		Url:        server.URL + "/test.txt",
		Archive:    "test.txt",
		MaxRetries: maxRetries,
	}

	client := &http.Client{}
	_, err := download.Start(client)

	if err == nil {
		t.Fatal("Expected error after retries, got success")
	}

	if attemptCount != maxRetries {
		t.Fatalf("Expected %d attempts, got %d", maxRetries, attemptCount)
	}

	if !strings.Contains(err.Error(), fmt.Sprintf("failed after %d attempts", maxRetries)) {
		t.Fatalf("Expected error message to mention %d attempts, got: %v", maxRetries, err)
	}
}

// TestDownloadRetrySuccessAfterFailures tests retry succeeds on 2nd attempt.
func TestDownloadRetrySuccessAfterFailures(t *testing.T) {
	attemptCount := 0

	// Create a test server that fails once, then succeeds.
	server := httptest.NewServer(http.HandlerFunc(func(w http.ResponseWriter, r *http.Request) {
		attemptCount++
		if attemptCount < 2 {
			w.WriteHeader(http.StatusInternalServerError)
			w.Write([]byte("server error"))
			return
		}
		w.WriteHeader(http.StatusOK)
		w.Write([]byte("test content"))
	}))
	defer server.Close()

	// Create downloader.
	download := downloader{
		Url:        server.URL + "/test.txt",
		Archive:    "test.txt",
		MaxRetries: 3,
	}

	client := &http.Client{}
	downloaded, err := download.Start(client)

	if err != nil {
		t.Fatalf("Expected success after retry, got error: %v", err)
	}

	if attemptCount != 2 {
		t.Fatalf("Expected 2 attempts (1 fail + 1 success), got %d", attemptCount)
	}

	if downloaded == "" {
		t.Fatal("Expected downloaded file path, got empty string")
	}

	// Clean up.
	os.Remove(downloaded)
}

// TestDownloadRetry404NotFound tests 404 error handling.
func TestDownloadRetry404NotFound(t *testing.T) {
	attemptCount := 0

	// Create a test server that returns 404.
	server := httptest.NewServer(http.HandlerFunc(func(w http.ResponseWriter, r *http.Request) {
		attemptCount++
		w.WriteHeader(http.StatusNotFound)
		w.Write([]byte("not found"))
	}))
	defer server.Close()

	// Create downloader.
	maxRetries := 3
	d := downloader{
		Url:        server.URL + "/missing.txt",
		Archive:    "missing.txt",
		MaxRetries: maxRetries,
	}

	client := &http.Client{}
	_, err := d.Start(client)
	if err == nil {
		t.Fatal("Expected error for 404, got success")
	}

	if attemptCount != maxRetries {
		t.Fatalf("Expected %d attempts, got %d", maxRetries, attemptCount)
	}

	if !strings.Contains(err.Error(), "404") {
		t.Fatalf("Expected error message to mention 404, got: %v", err)
	}
}
