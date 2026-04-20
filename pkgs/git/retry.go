package git

import (
	"celer/pkgs/cmd"
	"celer/pkgs/color"
	"fmt"
	"strings"
	"time"
)

const gitRetryMaxAttempts = 3

func combinedOutput(title, repoDir string, args ...string) ([]byte, error) {
	title = fmt.Sprintf("[%s]", title)
	executor := cmd.NewExecutor(title, "git", args...)
	if repoDir != "" {
		executor.SetWorkDir(repoDir)
	}
	executor.SetMirrorOutput(true)
	output, err := executor.ExecuteOutput()
	return []byte(output), err
}

func retrySleep(attempt int) {
	time.Sleep(time.Duration(attempt) * time.Second)
}

func runWithRetry(title, repoDir string, args ...string) ([]byte, error) {
	var lastErr error
	var lastOutput []byte

	for attempt := 1; attempt <= gitRetryMaxAttempts; attempt++ {
		output, err := combinedOutput(title, repoDir, args...)
		if err == nil {
			return output, nil
		}

		lastErr = err
		lastOutput = output
		color.Printf(color.Warning, "-- Git %s failed (attempt %d/%d): %v\n", title, attempt, gitRetryMaxAttempts, err)
		if attempt < gitRetryMaxAttempts {
			retrySleep(attempt)
		}
	}

	trimmedOutput := strings.TrimSpace(string(lastOutput))
	if trimmedOutput == "" {
		return nil, fmt.Errorf("git %s failed after %d attempts -> %w", title, gitRetryMaxAttempts, lastErr)
	}
	return nil, fmt.Errorf("git %s failed after %d attempts -> %w: %s", title, gitRetryMaxAttempts, lastErr, trimmedOutput)
}
