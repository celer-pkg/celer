package fileio

import (
	"fmt"
	"strings"
	"time"

	"github.com/celer-pkg/celer/pkgs/color"
	"github.com/celer-pkg/celer/pkgs/expr"
)

type Completed func(formattedTimeCost, formattedSize string)

type progressBar struct {
	title        string
	fileSize     int64
	currentSize  int64
	lastProgress int
	startTime    time.Time
	started      bool

	// Callbacks you can register.
	completed Completed
}

func NewProgressBar(title string, fileSize int64, completed Completed) *progressBar {
	return &progressBar{
		title:     title,
		fileSize:  fileSize,
		startTime: time.Now(),
		completed: completed,
	}
}

func (p *progressBar) Write(b []byte) (int, error) {
	n := len(b)
	p.currentSize += int64(n)

	// Unknown or invalid total size (e.g. chunked transfer with ContentLength=-1):
	// percentage and completion can't be computed, so just record the bytes.
	if p.fileSize <= 0 {
		return n, nil
	}

	progress := int(float64(p.currentSize*100) / float64(p.fileSize))
	if progress > p.lastProgress {
		// Print progress bar in a new line.
		if !p.started {
			fmt.Println()
			p.started = true
		}

		p.lastProgress = progress

		// Calculate download speed.
		now := time.Now()
		elapsedSec := now.Sub(p.startTime).Seconds()
		speed := float64(0)
		if elapsedSec > 0 {
			speed = float64(p.currentSize) / elapsedSec
		}

		// Calculate ETA.
		var eta string
		if speed > 0 && p.currentSize < p.fileSize {
			remainingBytes := float64(p.fileSize - p.currentSize)
			remainingSec := remainingBytes / speed
			eta = p.formatDuration(int64(remainingSec))
		}

		// Format speed with appropriate units.
		speedStr := expr.FormatSize(int64(speed)) + "/s"

		// Build progress bar (20 characters width)
		barWidth := 20
		filledWidth := (progress * barWidth) / 100
		var progressBar strings.Builder
		for i := range barWidth {
			if i < filledWidth {
				progressBar.WriteString("█")
			} else {
				progressBar.WriteString("░")
			}
		}

		// Truncate the title so the whole line fits within the terminal width.
		// otherwise the terminal wraps it and the \r overwrite breaks.
		title := p.title
		if maxWidth := color.TerminalWidth() - 60; maxWidth > 8 && len([]rune(title)) > maxWidth {
			title = truncateMiddle(title, maxWidth)
		}

		// Build compact progress display.
		var content string
		if eta != "" {
			content = fmt.Sprintf("[-] %s: [%s] %d%% (%s ETA:%s)",
				title,
				progressBar.String(),
				progress,
				speedStr,
				eta,
			)
		} else {
			content = fmt.Sprintf("[-] %s: [%s] %d%% (%s)",
				title,
				progressBar.String(),
				progress,
				speedStr,
			)
		}

		color.PrintInline(color.Hint, "%s", content)
		if progress == 100 {
			totalSec := time.Since(p.startTime).Seconds()
			if p.completed != nil {
				p.completed(p.formatDuration(int64(totalSec)), expr.FormatSize(p.fileSize))
			}
		}
	}

	return n, nil
}

// formatDuration converts seconds to a human-readable format (e.g., "2m 30s", "45s")
func (p *progressBar) formatDuration(seconds int64) string {
	if seconds < 60 {
		return fmt.Sprintf("%ds", seconds)
	}

	minutes := seconds / 60
	secs := seconds % 60
	if minutes < 60 {
		return fmt.Sprintf("%dm %ds", minutes, secs)
	}

	hours := minutes / 60
	mins := minutes % 60
	return fmt.Sprintf("%dh %dm %ds", hours, mins, secs)
}

// truncateMiddle shortens content to at most max runes, keeping both ends with "..."
// in the middle (e.g. "long-file-name.tar.gz" -> "long-fil...ame.tar.gz").
func truncateMiddle(content string, max int) string {
	runes := []rune(content)
	if len(runes) <= max {
		return content
	}
	if max <= 3 {
		return string(runes[:max])
	}

	left := (max - 3) / 2
	right := max - 3 - left
	return string(runes[:left]) + "..." + string(runes[len(runes)-right:])
}
