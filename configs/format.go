package configs

import (
	"celer/pkgs/color"
	"fmt"
)

const ClearScreen = "\033[2J"

func SprintSuccess(format string, args ...any) string {
	return color.Sprintf(color.Magenta, "\n[✔] ======== %s ========\n", fmt.Sprintf(format, args...))
}

func PrintSuccess(format string, args ...any) {
	color.Printf(color.Magenta, "\n[✔] ======== %s ========\n", fmt.Sprintf(format, args...))
}

func SprintError(err error, format string, args ...any) string {
	return color.Sprintf(color.Red, "\n[✘] %s\n[☛] %s.\n", fmt.Sprintf(format, args...), err)
}

func PrintError(err error, format string, args ...any) {
	color.Printf(color.Red, "\n[✘] %s\n[☛] %s.\n", fmt.Sprintf(format, args...), err)
}

func PrintWarning(err error, format string, args ...any) string {
	return color.Sprintf(color.Yellow, "\n[❕︎] %s\n[☛] %s.\n", fmt.Sprintf(format, args...), err)
}

func SprintWarning(err error, format string, args ...any) {
	color.Printf(color.Yellow, "\n[❕︎] %s\n[☛] %s.\n", fmt.Sprintf(format, args...), err)
}
