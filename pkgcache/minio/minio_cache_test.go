package minio

import (
	"bytes"
	"sync"
	"testing"
)

// TestProgressHookForwardsBytes verifies progressHook acts as a passive hook:
// it forwards data to the writer untouched and never fails, whatever the caller does.
func TestProgressHookForwardsBytes(t *testing.T) {
	var buf bytes.Buffer
	hook := &progressHook{writer: &buf}

	chunk := []byte("0123456789")
	n, err := hook.Read(chunk)
	if err != nil {
		t.Fatal(err)
	}
	if n != len(chunk) {
		t.Fatalf("expected n = %d, got %d", len(chunk), n)
	}
	if string(chunk) != "0123456789" {
		t.Fatalf("hook must not modify the data buffer, got %q", chunk)
	}
	if buf.String() != "0123456789" {
		t.Fatalf("expected forwarded bytes %q, got %q", chunk, buf.String())
	}

	// A second call (e.g. multipart parts) must not error or read anything.
	n, err = hook.Read(chunk)
	if err != nil {
		t.Fatal(err)
	}
	if n != len(chunk) {
		t.Fatalf("expected n = %d, got %d", len(chunk), n)
	}
	if buf.String() != "01234567890123456789" {
		t.Fatalf("expected twice the bytes, got %q", buf.String())
	}
}

// countingWriter accumulates the number of bytes written. It is deliberately
// not concurrency-safe: the test relies on progressHook serializing writes so
// this non-atomic counter stays accurate (and `go test -race` stays clean).
type countingWriter struct {
	n int64
}

func (w *countingWriter) Write(p []byte) (int, error) {
	w.n += int64(len(p))
	return len(p), nil
}

// TestProgressHookSerializesConcurrentReads verifies that concurrent Read calls
// (as minio's multipart uploader makes) are serialized, so no byte count is lost.
func TestProgressHookSerializesConcurrentReads(t *testing.T) {
	var writer countingWriter
	hook := &progressHook{writer: &writer}

	const (
		goroutines = 8
		iterations = 2000
		chunkLen   = 10
	)

	var wg sync.WaitGroup
	wg.Add(goroutines)
	for range goroutines {
		go func() {
			defer wg.Done()
			chunk := make([]byte, chunkLen)
			for range iterations {
				_, _ = hook.Read(chunk)
			}
		}()
	}
	wg.Wait()

	want := int64(goroutines * iterations * chunkLen)
	if writer.n != want {
		t.Fatalf("expected %d bytes forwarded, got %d", want, writer.n)
	}
}
