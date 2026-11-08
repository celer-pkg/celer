package git

import (
	"fmt"
	"os"
	"os/exec"
	"path/filepath"
	"strings"

	"github.com/celer-pkg/celer/pkgs/dirs"
	"github.com/celer-pkg/celer/pkgs/logger"
)

const (
	maxDiffLines = 160
	maxDiffWidth = 200
)

// PrintDiff print the changed and context lines of a `git diff -U3` between two texts.
func PrintDiff(left, right []byte) error {
	tmpDir, err := dirs.NewTmpFilesDir()
	if err != nil {
		return err
	}
	defer os.RemoveAll(tmpDir)

	leftPath := filepath.Join(tmpDir, "left")
	rightPath := filepath.Join(tmpDir, "right")
	if err := os.WriteFile(leftPath, left, os.ModePerm); err != nil {
		return fmt.Errorf("failed to write %s -> %w", leftPath, err)
	}
	if err := os.WriteFile(rightPath, right, os.ModePerm); err != nil {
		return fmt.Errorf("failed to write %s -> %w", rightPath, err)
	}

	// --no-index compares two plain files, so this works outside any repository and
	// never touches one. git exits with 1 when the files differ, which is exactly the
	// case here: Output() still returns the diff it wrote.
	output, err := exec.Command("git", "diff", "--no-index", "--no-color",
		"--no-ext-diff", "--unified=3", "--", leftPath, rightPath).Output()
	// Only a real failure is an error, not the "they differ" exit code.
	if exitErr, ok := err.(*exec.ExitError); err != nil && (!ok || exitErr.ExitCode() != 1) {
		return err
	}

	var (
		lines  []string
		inHunk bool
	)
	for line := range strings.SplitSeq(string(output), "\n") {
		switch {
		case strings.HasPrefix(line, "@@"):
			inHunk = true
		case inHunk:
			lines = append(lines, line)
		}
	}

	diffTxt := strings.Join(lines, "\n")
	if diffTxt == "" {
		return nil
	}

	// Print diff warning message.
	text := fmt.Sprintf("Cached meta differs from this workspace computes:\n%s"+
		"\nNote: '-' is the meta that cached, '+' is what this workspace computes.",
		sprintDiff(diffTxt))
	logger.PrintWarning("%s", text)

	return nil
}

// sprintDiff renders a diff the way a build log is read: '+' green, '-' red,
// context muted, and capped so a large diff cannot bury the rest of the log.
func sprintDiff(diff string) string {
	if diff == "" {
		return ""
	}

	lines := strings.Split(strings.TrimSuffix(diff, "\n"), "\n")
	shown := lines
	if len(shown) > maxDiffLines {
		shown = shown[:maxDiffLines]
	}

	var builder strings.Builder
	for _, line := range shown {
		builder.WriteString(logger.Sprintf(diffLineStyle(line), "%s\n", truncateLine(line)))
	}
	if len(lines) > len(shown) {
		builder.WriteString(logger.Sprintf(logger.Hint, "... %d more lines\n", len(lines)-len(shown)))
	}
	return builder.String()
}

func diffLineStyle(line string) *logger.Style {
	switch {
	case strings.HasPrefix(line, "+"):
		return logger.Pass
	case strings.HasPrefix(line, "-"):
		return logger.New(logger.Red)
	default:
		return logger.Muted
	}
}

// truncateLine cuts a line to a width a build log can show.
func truncateLine(line string) string {
	if runes := []rune(line); len(runes) > maxDiffWidth {
		return string(runes[:maxDiffWidth]) + "..."
	}
	return line
}
