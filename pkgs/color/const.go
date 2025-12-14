package color

// SGR variables for ANSI escape codes
const (
	Reset     = "0"
	Bold      = "1"
	Dim       = "2"
	Italic    = "3"
	Underline = "4"
	Blink     = "5"
	Reverse   = "7"
	Hidden    = "8"
	Strike    = "9"
)

// 16 colors - foreground
const (
	Black   = "30"
	Red     = "31"
	Green   = "32"
	Yellow  = "33"
	Blue    = "34"
	Magenta = "35"
	Cyan    = "36"
	White   = "37"
	Gray    = "90"

	BrightBlack   = "90"
	BrightRed     = "91"
	BrightGreen   = "92"
	BrightYellow  = "93"
	BrightBlue    = "94"
	BrightMagenta = "95"
	BrightCyan    = "96"
	BrightWhite   = "97"
)

// 16 colors - background
const (
	BgBlack   = "40"
	BgRed     = "41"
	BgGreen   = "42"
	BgYellow  = "43"
	BgBlue    = "44"
	BgMagenta = "45"
	BgCyan    = "46"
	BgWhite   = "47"

	BgBrightBlack   = "100"
	BgBrightRed     = "101"
	BgBrightGreen   = "102"
	BgBrightYellow  = "103"
	BgBrightBlue    = "104"
	BgBrightMagenta = "105"
	BgBrightCyan    = "106"
	BgBrightWhite   = "107"
)

const (
	esc   = "\033["
	reset = esc + "0m"
)
