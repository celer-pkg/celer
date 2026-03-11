package color

import (
	"fmt"
	"os"
	"strings"
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
