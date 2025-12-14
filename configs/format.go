package configs

import (
	"celer/pkgs/color"
	"fmt"
	"strings"
)

const ClearScreen = "\033[2J"

func SprintSuccess(format string, args ...any) string {
	return color.Sprintf(color.Magenta, "\n[✔] ======== %s ========\n", fmt.Sprintf(format, args...))
}

func PrintSuccess(format string, args ...any) {
	color.Printf(color.Magenta, "\n[✔] ======== %s ========\n", fmt.Sprintf(format, args...))
}

func SprintError(err error, format string, args ...any) string {
	details := strings.ReplaceAll(err.Error(), "\n", "\n--> ")
	return color.Sprintf(color.Error, "\n[✘] %s\n[☛] %s.\n", fmt.Sprintf(format, args...), details)
}

func PrintError(err error, format string, args ...any) {
	details := strings.ReplaceAll(err.Error(), "\n", "\n--> ")
	color.Printf(color.Error, "\n[✘] %s\n[☛] %s.\n", fmt.Sprintf(format, args...), details)
}

func PrintWarning(err error, format string, args ...any) string {
	details := strings.ReplaceAll(err.Error(), "\n", "\n--> ")
	return color.Sprintf(color.Yellow, "\n[❕︎] %s\n[☛] %s.\n", fmt.Sprintf(format, args...), details)
}

func SprintWarning(err error, format string, args ...any) string {
	details := strings.ReplaceAll(err.Error(), "\n", "\n--> ")
	return color.Sprintf(color.Yellow, "\n[❕︎] %s\n[☛] %s.\n", fmt.Sprintf(format, args...), details)
}
