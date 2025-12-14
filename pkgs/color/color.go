package color

import (
	"fmt"
	"io"
)

var (
	Info      = New(Cyan).Format()
	Success   = New(Green, Bold).Format()
	Warning   = New(Yellow, Bold).Format()
	Error     = New(Red, Bold).Format()
	Debug     = New(BrightBlack, Italic).Format()
	Muted     = New(BrightBlack).Format()
	Title     = New(Blue, Bold).Format()
	Line      = New(Gray).Format()
	List      = New(Gray).Format()
	Summary   = New(Blue, Bold).Format()
	Important = New(BrightMagenta, Bold).Format()
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

func Print(colorFmt, message string) {
	fmt.Printf(colorFmt, message)
}

func Printf(colorFmt, format string, args ...any) {
	fmt.Printf(colorFmt, fmt.Sprintf(format, args...))
}

func Println(colorFmt, message string) {
	fmt.Printf(colorFmt+"\n", message)
}

func Sprintf(color, format string, args ...any) string {
	return fmt.Sprintf(color, fmt.Sprintf(format, args...))
}

func Sprint(color, message string) string {
	return fmt.Sprintf(color, message)
}
