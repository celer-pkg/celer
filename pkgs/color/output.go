package color

import (
	"os"
	"strings"
	"sync"

	"golang.org/x/term"
)

var (
	// outputMutex serializes stdout to prevent output from concurrent tasks from mixing together.
	outputMutex sync.Mutex

	// inProgressLine reports whether the terminal's current line is an
	// in-progress print (one that did not end with '\n'). Before starting a new
	// line we clear it so a following '\n' can't orphan it.
	inProgressLine bool
)

// writeStdout writes one logical chunk to stdout under outputMutex.
func writeStdout(style *Style, content string, inline bool) {
	outputMutex.Lock()
	defer outputMutex.Unlock()

	if !inline && inProgressLine && term.IsTerminal(int(os.Stdout.Fd())) {
		os.Stdout.WriteString("\r\033[K")
	}
	if inline {
		os.Stdout.WriteString("\r")
	}
	os.Stdout.WriteString(style.Apply(content))
	inProgressLine = !strings.HasSuffix(content, "\n")
}
