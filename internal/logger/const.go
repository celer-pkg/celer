package logger

// Font styles for ANSI escape codes
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

// 16 colors - foreground.
const (
	Black   = "30" // #000000
	Red     = "31" // #cd0000
	Green   = "32" // #00cd00
	Yellow  = "33" // #cdcd00
	Blue    = "34" // #0000ee
	Magenta = "35" // #cd00cd
	Cyan    = "36" // #00cdcd
	White   = "37" // #e5e5e5
	Gray    = "90" // #7f7f7f, same value as BrightBlack

	BrightBlack   = "90" // #7f7f7f
	BrightRed     = "91" // #ff0000
	BrightGreen   = "92" // #00ff00
	BrightYellow  = "93" // #ffff00
	BrightBlue    = "94" // #5c5cff
	BrightMagenta = "95" // #ff00ff
	BrightCyan    = "96" // #00ffff
	BrightWhite   = "97" // #ffffff
)

// 256 colors - foreground.
//
// Unlike the 16 colors above, these name a fixed palette entry ("38;5;n"), so
// the hex shown is exactly what you get.
const (
	Orange     = "38;5;208" // #ff8700 warmer than Yellow
	Gold       = "38;5;220" // #ffd700 bright yellow (CSS gold)
	Lime       = "38;5;46"  // #00ff00 vivid green (CSS lime)
	Aqua       = "38;5;51"  // #00ffff vivid cyan (CSS aqua)
	DodgerBlue = "38;5;33"  // #0087ff brighter, more saturated than Blue
	Fuchsia    = "38;5;201" // #ff00ff pure magenta (CSS fuchsia)
	Pink       = "38;5;213" // #ff87ff soft magenta
	Teal       = "38;5;37"  // #00afaf low-saturation blue-green
	TealLight  = "38;5;38"  // #00afd7 lighter low-saturation blue-green
	VividRed   = "38;5;196" // #ff0000 pure bright red
	Charcoal   = "38;5;240" // #585858 weakest helper text
	Slate      = "38;5;244" // #808080 neutral mid gray
	Silver     = "38;5;250" // #bcbcbc lighter secondary text
)

const (
	esc   = "\033["
	reset = esc + "0m"
)
