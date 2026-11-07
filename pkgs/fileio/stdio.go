package fileio

import (
	"io"
	"os"
)

func CaptureStdout(fn func()) (string, error) {
	reader, writer, err := os.Pipe()
	if err != nil {
		return "", err
	}

	stdout := os.Stdout
	os.Stdout = writer
	defer func() { os.Stdout = stdout }()

	fn()
	writer.Close()
	os.Stdout = stdout

	output, err := io.ReadAll(reader)
	if err != nil {
		return "", err
	}
	return string(output), nil
}
