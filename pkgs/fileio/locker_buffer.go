package fileio

import (
	"bytes"
	"sync"
)

type LockedBuffer struct {
	mutex  sync.Mutex
	buffer bytes.Buffer
}

func (l *LockedBuffer) Write(p []byte) (int, error) {
	l.mutex.Lock()
	defer l.mutex.Unlock()

	if _, err := l.buffer.Write(p); err != nil {
		return 0, err
	}
	return len(p), nil
}

func (l *LockedBuffer) String() string {
	l.mutex.Lock()
	defer l.mutex.Unlock()

	return l.buffer.String()
}
