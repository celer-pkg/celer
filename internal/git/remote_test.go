package git

import "testing"

// libffi's v3.4.8 is a lightweight tag: the ref points directly at the commit,
// so there is no `^{}` peeled entry and it must be resolved from the bare ref.
func TestGetRemoteRefCommit_LightweightTag(t *testing.T) {
	const wantCommit = "6a99edb8082f75e523e0d6ebaba42218b80e10c8"

	commit, err := GetRemoteRefCommit("libffi@3.4.8", "https://github.com/libffi/libffi.git", "v3.4.8")
	if err != nil {
		t.Fatal(err)
	}

	if commit != wantCommit {
		t.Fatalf("lightweight tag resolved to %s, want %s", commit, wantCommit)
	}
}

// bzip2's bzip2-1.0.7 is an annotated tag: the ref points at a tag object that
// must be peeled via `^{}` to reach the commit.
func TestGetRemoteRefCommit_AnnotatedTag(t *testing.T) {
	const wantCommit = "f319b98aade2a337c74b9a3b48c6daffb7809cda"

	commit, err := GetRemoteRefCommit("bzip2@1.0.7", "https://gitlab.com/bzip2/bzip2.git", "bzip2-1.0.7")
	if err != nil {
		t.Fatal(err)
	}

	if commit != wantCommit {
		t.Fatalf("annotated tag resolved to %s, want %s", commit, wantCommit)
	}
}
