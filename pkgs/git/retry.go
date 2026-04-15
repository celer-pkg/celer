package git

import (
	"celer/pkgs/color"
	"fmt"
	"os/exec"
	"strings"
	"time"
)

var (
	gitRetryMaxAttempts = 3
	gitRetrySleep       = func(attempt int) {
		time.Sleep(time.Duration(attempt) * time.Second)
	}
	gitCombinedOutput = func(repoDir string, args ...string) ([]byte, error) {
		fullArgs := append([]string{}, args...)
		if repoDir != "" {
			fullArgs = append([]string{"-C", repoDir}, fullArgs...)
		}
		cmd := exec.Command("git", fullArgs...)
		return cmd.CombinedOutput()
	}
)

func runWithRetry(repoDir, action string, args ...string) ([]byte, error) {
	var lastErr error
	var lastOutput []byte

	for attempt := 1; attempt <= gitRetryMaxAttempts; attempt++ {
		output, err := gitCombinedOutput(repoDir, args...)
		if err == nil {
			return output, nil
		}

		lastErr = err
		lastOutput = output
		color.Printf(color.Warning, "-- Git %s failed (attempt %d/%d): %v\n", action, attempt, gitRetryMaxAttempts, err)
		if attempt < gitRetryMaxAttempts {
			gitRetrySleep(attempt)
		}
	}

	trimmedOutput := strings.TrimSpace(string(lastOutput))
	if trimmedOutput == "" {
		return nil, fmt.Errorf("git %s failed after %d attempts -> %w", action, gitRetryMaxAttempts, lastErr)
	}
	return nil, fmt.Errorf("git %s failed after %d attempts -> %w: %s", action, gitRetryMaxAttempts, lastErr, trimmedOutput)
}
