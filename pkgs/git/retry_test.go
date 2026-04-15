package git

import (
	"errors"
	"strings"
	"testing"
)

func TestGitCommandWithRetrySuccessAfterFailure(t *testing.T) {
	oldRunner := gitCombinedOutput
	oldSleep := gitRetrySleep
	oldMaxAttempts := gitRetryMaxAttempts
	defer func() {
		gitCombinedOutput = oldRunner
		gitRetrySleep = oldSleep
		gitRetryMaxAttempts = oldMaxAttempts
	}()

	attempts := 0
	gitRetryMaxAttempts = 3
	gitRetrySleep = func(int) {}
	gitCombinedOutput = func(repoDir string, args ...string) ([]byte, error) {
		attempts++
		if attempts < 2 {
			return []byte("temporary failure"), errors.New("exit status 128")
		}
		return []byte("ok"), nil
	}

	output, err := runWithRetry("/tmp/repo", "fetch remote metadata", "fetch", "--tags", "origin")
	if err != nil {
		t.Fatalf("expected retry to succeed, got: %v", err)
	}
	if string(output) != "ok" {
		t.Fatalf("expected successful output, got: %q", string(output))
	}
	if attempts != 2 {
		t.Fatalf("expected 2 attempts, got %d", attempts)
	}
}

func TestGitCommandWithRetryFailureAfterMaxAttempts(t *testing.T) {
	oldRunner := gitCombinedOutput
	oldSleep := gitRetrySleep
	oldMaxAttempts := gitRetryMaxAttempts
	defer func() {
		gitCombinedOutput = oldRunner
		gitRetrySleep = oldSleep
		gitRetryMaxAttempts = oldMaxAttempts
	}()

	attempts := 0
	gitRetryMaxAttempts = 3
	gitRetrySleep = func(int) {}
	gitCombinedOutput = func(repoDir string, args ...string) ([]byte, error) {
		attempts++
		return []byte("fatal: could not resolve host"), errors.New("exit status 128")
	}

	_, err := runWithRetry("/tmp/repo", "fetch remote metadata", "fetch", "--tags", "origin")
	if err == nil {
		t.Fatal("expected retry to fail")
	}
	if attempts != 3 {
		t.Fatalf("expected 3 attempts, got %d", attempts)
	}
	if !strings.Contains(err.Error(), "failed after 3 attempts") {
		t.Fatalf("expected max-attempts error, got: %v", err)
	}
}
