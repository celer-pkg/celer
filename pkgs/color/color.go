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
	List      = New(Gray)
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

func Print(colorFmt *Style, message string) {
	fmt.Printf(colorFmt.Format(), message)
}

func Printf(colorFmt *Style, format string, args ...any) {
	fmt.Printf(colorFmt.Format(), fmt.Sprintf(format, args...))
}

func Println(colorFmt *Style, message string) {
	fmt.Printf(colorFmt.Format()+"\n", message)
}

func Sprintf(colorFmt *Style, format string, args ...any) string {
	return fmt.Sprintf(colorFmt.Format(), fmt.Sprintf(format, args...))
}

func Sprint(colorFmt *Style, message string) string {
	return fmt.Sprintf(colorFmt.Format(), message)
}
