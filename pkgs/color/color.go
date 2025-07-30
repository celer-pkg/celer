package color

import (
	"fmt"
	"io"
)

const (
	Red     string = "\033[31m%s\033[0m"
	Green   string = "\033[32m%s\033[0m"
	Yellow  string = "\033[33m%s\033[0m"
	Blue    string = "\033[34m%s\033[0m"
	Magenta string = "\033[35m%s\033[0m"
	Cyan    string = "\033[36m%s\033[0m"
	Gray    string = "\033[90m%s\033[0m"
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

func Printf(colorFmt, format string, args ...interface{}) {
	fmt.Printf(colorFmt, fmt.Sprintf(format, args...))
}

func Println(colorFmt, message string) {
	fmt.Printf(colorFmt+"\n", message)
}

func Sprintf(color, format string, args ...interface{}) string {
	return fmt.Sprintf(color, fmt.Sprintf(format, args...))
}
