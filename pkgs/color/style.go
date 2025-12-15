package color

import (
	"fmt"
	"strings"
)

type Style struct {
	codes []string
}

func New(codes ...string) *Style {
	return &Style{codes: codes}
}

func (s *Style) Add(code string) *Style {
	s.codes = append(s.codes, code)
	return s
}

// Methods for style attributes.
func (s *Style) Bold() *Style      { return s.Add(Bold) }
func (s *Style) Italic() *Style    { return s.Add(Italic) }
func (s *Style) Underline() *Style { return s.Add(Underline) }
func (s *Style) Dim() *Style       { return s.Add(Dim) }
func (s *Style) Reverse() *Style   { return s.Add(Reverse) }
func (s *Style) Strike() *Style    { return s.Add(Strike) }

// Functions for common colors.
func (s *Style) Red() *Style     { return s.Add(Red) }
func (s *Style) Green() *Style   { return s.Add(Green) }
func (s *Style) Yellow() *Style  { return s.Add(Yellow) }
func (s *Style) Blue() *Style    { return s.Add(Blue) }
func (s *Style) Cyan() *Style    { return s.Add(Cyan) }
func (s *Style) Magenta() *Style { return s.Add(Magenta) }
func (s *Style) White() *Style   { return s.Add(White) }
func (s *Style) Black() *Style   { return s.Add(Black) }

// Bright color methods.
func (s *Style) BrightRed() *Style     { return s.Add(BrightRed) }
func (s *Style) BrightGreen() *Style   { return s.Add(BrightGreen) }
func (s *Style) BrightYellow() *Style  { return s.Add(BrightYellow) }
func (s *Style) BrightBlue() *Style    { return s.Add(BrightBlue) }
func (s *Style) BrightCyan() *Style    { return s.Add(BrightCyan) }
func (s *Style) BrightMagenta() *Style { return s.Add(BrightMagenta) }
func (s *Style) BrightWhite() *Style   { return s.Add(BrightWhite) }
func (s *Style) BrightBlack() *Style   { return s.Add(BrightBlack) }

// Background color methods.
func (s *Style) BgRed() *Style     { return s.Add(BgRed) }
func (s *Style) BgGreen() *Style   { return s.Add(BgGreen) }
func (s *Style) BgYellow() *Style  { return s.Add(BgYellow) }
func (s *Style) BgBlue() *Style    { return s.Add(BgBlue) }
func (s *Style) BgCyan() *Style    { return s.Add(BgCyan) }
func (s *Style) BgMagenta() *Style { return s.Add(BgMagenta) }
func (s *Style) BgWhite() *Style   { return s.Add(BgWhite) }

// 256-color and RGB methods.
func (s *Style) Color256(code int) *Style {
	return s.Add(fmt.Sprintf("38;5;%d", code))
}

func (s *Style) BgColor256(code int) *Style {
	return s.Add(fmt.Sprintf("48;5;%d", code))
}

// RGB true color methods.
func (s *Style) RGB(r, g, b int) *Style {
	return s.Add(fmt.Sprintf("38;2;%d;%d;%d", r, g, b))
}

func (s *Style) BgRGB(r, g, b int) *Style {
	return s.Add(fmt.Sprintf("48;2;%d;%d;%d", r, g, b))
}

// String returns the ANSI escape sequence.
func (s *Style) String() string {
	if len(s.codes) == 0 {
		return ""
	}
	return esc + strings.Join(s.codes, ";") + "m"
}

// Format returns a format string for use with fmt.Printf.
func (s *Style) Format() string {
	if len(s.codes) == 0 {
		return "%s"
	}
	return s.String() + "%s" + reset
}

// Apply applies the style to the given text.
func (s *Style) Apply(text string) string {
	if len(s.codes) == 0 {
		return text
	}
	return s.String() + text + reset
}

// Sprint formats and applies the style.
func (s *Style) Sprint(a ...any) string {
	return s.Apply(fmt.Sprint(a...))
}

// Sprintf formats and applies the style.
func (s *Style) Sprintf(format string, a ...any) string {
	return s.Apply(fmt.Sprintf(format, a...))
}

// Convenience function - direct use.
func Color(text string, codes ...string) string {
	if len(codes) == 0 {
		return text
	}
	return esc + strings.Join(codes, ";") + "m" + text + reset
}

// Chainable convenience wrapper.
func Stylize(text string) *styleBuilder {
	return &styleBuilder{text: text}
}

type styleBuilder struct {
	text string
}

func (sb *styleBuilder) With(style *Style) string {
	if style == nil {
		return sb.text
	}
	return style.Apply(sb.text)
}
