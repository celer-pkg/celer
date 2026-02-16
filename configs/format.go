package configs

import (
	"celer/pkgs/color"
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
	return color.Sprintf(color.Important, "\n[✔] ======== %s ========\n", fmt.Sprintf(format, args...))
}

func PrintSuccess(format string, args ...any) {
	color.Printf(color.Important, "\n[✔] ======== %s ========\n", fmt.Sprintf(format, args...))
}

func SprintError(err error, format string, args ...any) string {
	details := strings.ReplaceAll(err.Error(), ": ", "\n--> ")
	return color.Sprintf(color.Error, "\n[✘] %s\n[☛] %s\n", fmt.Sprintf(format, args...), details)
}

func PrintError(err error, format string, args ...any) error {
	details := strings.ReplaceAll(err.Error(), ": ", "\n--> ")
	color.Fprintf(os.Stderr, color.Error, "\n[✘] %s\n[☛] %s\n", fmt.Sprintf(format, args...), details)
	return ErrSilent
}

func PrintWarning(err error, format string, args ...any) string {
	details := strings.ReplaceAll(err.Error(), ": ", "\n--> ")
	return color.Sprintf(color.Warning, "\n[❕︎] %s\n[☛] %s\n", fmt.Sprintf(format, args...), details)
}

func SprintWarning(err error, format string, args ...any) string {
	details := strings.ReplaceAll(err.Error(), ": ", "\n--> ")
	return color.Sprintf(color.Warning, "\n[❕︎] %s\n[☛] %s\n", fmt.Sprintf(format, args...), details)
}
