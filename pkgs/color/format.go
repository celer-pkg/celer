package color

import (
	"fmt"
	"os"
	"strings"

	"golang.org/x/term"
)

const ClearScreen = "\033[2J"

type silentError struct{ inner error }

func (s silentError) Error() string { return "" }
func (s silentError) Unwrap() error { return s.inner }
func (silentError) Is(target error) bool {
	_, ok := target.(silentError)
	return ok
}

var ErrSilent = silentError{}

func SprintSuccess(format string, args ...any) string {
	return Sprintf(Important, "\n[✔] ======== %s ========\n", fmt.Sprintf(format, args...))
}

func PrintSuccess(format string, args ...any) {
	Printf(Important, "\n[✔] ======== %s ========\n", fmt.Sprintf(format, args...))
}

func SprintError(err error, format string, args ...any) string {
	details := strings.ReplaceAll(err.Error(), " -> ", "\n    └─ ")
	return Sprintf(Error, "\n[✘] %s\n[☛] %s\n", fmt.Sprintf(format, args...), details)
}

func PrintError(err error, format string, args ...any) error {
	details := strings.ReplaceAll(err.Error(), " -> ", "\n    └─ ")
	Fprintf(os.Stderr, Error, "\n[✘] %s\n[☛] %s\n", fmt.Sprintf(format, args...), details)
	return silentError{inner: err}
}

func PrintWarning(format string, args ...any) {
	Printf(Warning, "\n[!] %s\n", fmt.Sprintf(format, args...))
}

func PrintPass(format string, args ...any) {
	Printf(Pass, "\n[✔] %s\n", fmt.Sprintf(format, args...))
}

func PrintInfo(format string, args ...any) {
	Printf(Info, "%s\n", fmt.Sprintf(format, args...))
}

func PrintHint(format string, args ...any) {
	Printf(Hint, "%s\n", fmt.Sprintf(format, args...))
}

func PrintInline(colorFmt *Style, format string, args ...any) {
	content := fmt.Sprintf(format, args...)

	// Keep a trailing newline out of the erase sequence so the line ends cleanly
	// after the visible content.
	newline := strings.HasSuffix(content, "\n")
	content = strings.TrimSuffix(content, "\n")

	// Erase any leftover from a previously longer in-progress line，
	// only emit it on a real terminal.
	if term.IsTerminal(int(os.Stdout.Fd())) {
		content += "\033[K"
	}
	if newline {
		content += "\n"
	}

	writeStdout(colorFmt, content, true)

	// Flush to ensure immediate display
	os.Stdout.Sync()
}

// TerminalWidth returns the current terminal width, or a 150-column fallback
// when stdout is not a terminal (e.g. piped to a file).
func TerminalWidth() int {
	width, _, err := term.GetSize(int(os.Stdout.Fd()))
	if err != nil {
		return 150
	}
	return width
}
