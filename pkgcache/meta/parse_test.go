package meta

import "testing"

func TestPortCommit(t *testing.T) {
	root := `# -------- libexpat@2.7.1 <<< platform --------
name = "x86_64-linux"

# -------- libexpat@2.7.1 <<< port --------
[package]
url = "https://example.com/libexpat.git"
ref = "1111111111111111111111111111111111111111"

[[build_configs]]
build_system = "cmake"
`

	nested := `# -------- wayland@1.24.0 <<< platform --------
name = "x86_64-linux"

# -------- wayland@1.24.0 <<< port --------
[package]
url = "https://example.com/wayland.git"
ref = "2222222222222222222222222222222222222222"

# -------- wayland@1.24.0 <<< dependency: libexpat@2.7.1 <<< port --------
[package]
url = "https://example.com/libexpat.git"
ref = "3333333333333333333333333333333333333333"
`

	cases := []struct {
		name        string
		meta        string
		nameVersion string
		want        string
		wantOK      bool
	}{
		{"root port", root, "libexpat@2.7.1", "1111111111111111111111111111111111111111", true},
		{"nested dependency", nested, "libexpat@2.7.1", "3333333333333333333333333333333333333333", true},
		{"root of a nested meta", nested, "wayland@1.24.0", "2222222222222222222222222222222222222222", true},
		{"missing port", root, "openssl@3.0.0", "", false},
		{"no sections", "not a meta", "libexpat@2.7.1", "", false},
	}

	for _, tc := range cases {
		t.Run(tc.name, func(t *testing.T) {
			got, ok := PortCommit(tc.meta, tc.nameVersion)
			if ok != tc.wantOK || got != tc.want {
				t.Fatalf("PortCommit(%q) = (%q, %v), want (%q, %v)", tc.nameVersion, got, ok, tc.want, tc.wantOK)
			}
		})
	}
}
