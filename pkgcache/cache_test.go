package pkgcache

import (
	"path/filepath"
	"testing"
)

func TestGetArtifactsDir(t *testing.T) {
	fs := FS{Dir: "/cache"}
	minioCfg := Minio{}

	tests := []struct {
		version string
		want    string
	}{
		{version: "v1.2.3", want: "artifacts-v1.2.x"},
		{version: "v1.2.9", want: "artifacts-v1.2.x"},
		{version: "v1.3.0", want: "artifacts-v1.3.x"},
		{version: "1.2", want: "artifacts-1.2.x"},
		{version: "dev", want: "artifacts-dev.x"},
	}

	for _, tt := range tests {
		wantFS := filepath.Join("/cache", tt.want)
		if got := fs.GetDir(DirArtifacts, tt.version); got != wantFS {
			t.Errorf("FS.GetDir(DirArtifacts, %q) = %q, want %q", tt.version, got, wantFS)
		}
		if got := minioCfg.GetDir(DirArtifacts, tt.version); got != tt.want {
			t.Errorf("Minio.GetDir(DirArtifacts, %q) = %q, want %q", tt.version, got, tt.want)
		}
	}
}
