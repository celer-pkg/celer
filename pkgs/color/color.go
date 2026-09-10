package color

import (
	"fmt"
	"io"
)

var (
	Info      = New(Cyan)
	Success   = New(Green, Bold)
	Warning   = New(Yellow, Bold)
	Error     = New(Red, Bold)
	Debug     = New(BrightBlack, Italic)
	Muted     = New(BrightBlack)
	Title     = New(Blue, Bold)
	Line      = New(Gray)
	Pass      = New(Green)
	Hint      = New(Gray)
	Summary   = New(Blue, Bold)
	Important = New(BrightMagenta, Bold)
)

func NewWriter(w io.Writer, colorFmt string) *Writer {
	return &Writer{
		writer:   w,
		colorFmt: colorFmt,
	}
}

type Writer struct {
	writer   io.Writer
	colorFmt string
}

func (w *Writer) Write(p []byte) (n int, err error) {
	coloredOutput := fmt.Sprintf(w.colorFmt, string(p))
	_, err = w.writer.Write([]byte(coloredOutput))
	if err != nil {
		return 0, err
	}

	return len(p), nil
}

func Print(style *Style, message string) {
	writeStdout(style, message, false)
}

func Printf(style *Style, format string, args ...any) {
	writeStdout(style, fmt.Sprintf(format, args...), false)
}

func Fprintf(w io.Writer, style *Style, format string, args ...any) {
	fmt.Fprintf(w, style.Format(), fmt.Sprintf(format, args...))
}

func Println(style *Style, message string) {
	writeStdout(style, message+"\n", false)
}

func Sprintf(style *Style, format string, args ...any) string {
	return fmt.Sprintf(style.Format(), fmt.Sprintf(format, args...))
}

func Sprint(style *Style, message string) string {
	return fmt.Sprintf(style.Format(), message)
}
