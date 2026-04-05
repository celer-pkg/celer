package color

import (
	"fmt"
	"os"
	"strings"

	"golang.org/x/term"
)

const ClearScreen = "\033[2J"

// silentError is an error type that won't be printed by cobra
type silentError struct{}

func (silentError) Error() string { return "" }

// ErrSilent is a error should not be printed.
var ErrSilent error = silentError{}

func SprintSuccess(format string, args ...any) string {
	return Sprintf(Important, "\n[✔] ======== %s ========\n", fmt.Sprintf(format, args...))
}

func PrintSuccess(format string, args ...any) {
	Printf(Important, "\n[✔] ======== %s ========\n", fmt.Sprintf(format, args...))
}

func SprintError(err error, format string, args ...any) string {
	details := strings.ReplaceAll(err.Error(), " -> ", ":\n--> ")
	return Sprintf(Error, "\n[✘] %s\n[☛] %s\n", fmt.Sprintf(format, args...), details)
}

func PrintError(err error, format string, args ...any) error {
	details := strings.ReplaceAll(err.Error(), " -> ", ":\n--> ")
	Fprintf(os.Stderr, Error, "\n[✘] %s\n[☛] %s\n", fmt.Sprintf(format, args...), details)
	return ErrSilent
}

func PrintWarning(format string, args ...any) {
	Printf(Pass, "\n[!] -- %s", fmt.Sprintf(format, args...))
}

func PrintPass(format string, args ...any) {
	Printf(Pass, "\n[✔] -- %s", fmt.Sprintf(format, args...))
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

	// Calculate padding to fill the rest of the line
	padding := terminalWidth() - len(content)
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
