package color

import (
	"fmt"
	"os"
	"strings"

	"github.com/mattn/go-runewidth"
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
	Printf(Warning, "\n[!] %s", fmt.Sprintf(format, args...))
}

func PrintPass(format string, args ...any) {
	Printf(Pass, "\n[✔] %s", fmt.Sprintf(format, args...))
}

func PrintInfo(format string, args ...any) {
	Printf(Info, "\n%s", fmt.Sprintf(format, args...))
}

func PrintHint(format string, args ...any) {
	Printf(Hint, "\n%s", fmt.Sprintf(format, args...))
}

func PrintInline(colorFmt *Style, format string, args ...any) {
	content := fmt.Sprintf(format, args...)

	// Handle newline separately
	hasNewline := strings.HasSuffix(content, "\n")
	if hasNewline {
		content = strings.TrimSuffix(content, "\n")
	}

	// Calculate padding to fill the rest of the line using display width.
	padding := terminalWidth() - runewidth.StringWidth(content)
	if padding > 0 {
		content += strings.Repeat(" ", padding)
	}

	// Add back the newline if it existed
	if hasNewline {
		content += "\n"
	}

	fmt.Printf("\r"+colorFmt.Format(), content)

	// Flush to ensure immediate display
	os.Stdout.Sync()
}

func terminalWidth() int {
	width, _, err := term.GetSize(int(os.Stdout.Fd()))
	if err != nil {
		return 150
	}
	return width
}
