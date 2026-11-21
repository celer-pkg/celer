package ros

import (
	"os"
	"path/filepath"
	"strings"
	"testing"
)

func TestFixupParentPrefixPath(t *testing.T) {
	dir := t.TempDir()
	markerDir := filepath.Join(dir, "share", "ament_index", "resource_index", "parent_prefix_path")
	if err := os.MkdirAll(markerDir, os.ModePerm); err != nil {
		t.Fatal(err)
	}

	ws := "/workspace"
	staging := ws + "/tmp/staging-my_project@1.2.3-1234/aarch64-linux/my_project/release"

	cases := []struct {
		name, content, want string
	}{
		{"celer single staging", staging, "{prefix}"},
		{"ros2 form preserved", "{prefix}:/opt/ros/humble", "{prefix}:/opt/ros/humble"},
		{"staging plus external", staging + ":/opt/ros/humble", "{prefix}:/opt/ros/humble"},
	}

	for _, c := range cases {
		p := filepath.Join(markerDir, c.name)
		if err := os.WriteFile(p, []byte(c.content), os.ModePerm); err != nil {
			t.Fatal(err)
		}
		if err := fixupParentPrefixPath(p, ws); err != nil {
			t.Fatalf("%s: %v", c.name, err)
		}
		got, _ := os.ReadFile(p)
		if string(got) != c.want {
			t.Errorf("%s: got %q want %q", c.name, string(got), c.want)
		}
	}
}

func TestFixupPrefixChainScript(t *testing.T) {
	dir := t.TempDir()
	ws := "/workspace"
	staging := ws + "/tmp/staging-my_project@1.2.3-1234/aarch64-linux/my_project/release"

	sh := "# generated from colcon_core/shell/template/prefix_chain.sh.em\n\n" +
		"# source chained prefixes\n" +
		"# setting COLCON_CURRENT_PREFIX avoids relying on the build time prefix of the sourced script\n" +
		`COLCON_CURRENT_PREFIX="` + staging + "\"\n" +
		`_colcon_prefix_chain_sh_source_script "$COLCON_CURRENT_PREFIX/local_setup.sh"` + "\n" +
		"\n" +
		"# source this prefix\n" +
		`COLCON_CURRENT_PREFIX="$_colcon_prefix_chain_sh_COLCON_CURRENT_PREFIX"` + "\n" +
		`_colcon_prefix_chain_sh_source_script "$COLCON_CURRENT_PREFIX/local_setup.sh"` + "\n"

	p := filepath.Join(dir, "setup.sh")
	if err := os.WriteFile(p, []byte(sh), os.ModePerm); err != nil {
		t.Fatal(err)
	}
	if err := fixupPrefixChainScript(p, ws); err != nil {
		t.Fatal(err)
	}
	got, _ := os.ReadFile(p)
	if strings.Contains(string(got), ws) {
		t.Errorf("workspace path still present:\n%s", got)
	}
	if !strings.Contains(string(got), `# source this prefix`) {
		t.Errorf("self-source block lost:\n%s", got)
	}
	if strings.Contains(string(got), `"$COLCON_CURRENT_PREFIX/local_setup.sh"`+"\n"+`_colcon_prefix_chain_sh_source_script "$COLCON_CURRENT_PREFIX/local_setup.sh"`+"\n\n"+"# source this prefix") {
		t.Errorf("chained source line not removed:\n%s", got)
	}

	// powershell: single inline line with the path
	ps1 := "# generated from colcon_powershell/shell/template/prefix_chain.ps1.em\n" +
		`_colcon_prefix_chain_powershell_source_script "` + staging + `/local_setup.ps1"` + "\n" +
		`$env:COLCON_CURRENT_PREFIX=(Split-Path $PSCommandPath -Parent)` + "\n" +
		`_colcon_prefix_chain_powershell_source_script "$env:COLCON_CURRENT_PREFIX/local_setup.ps1"` + "\n"
	p = filepath.Join(dir, "setup.ps1")
	if err := os.WriteFile(p, []byte(ps1), os.ModePerm); err != nil {
		t.Fatal(err)
	}
	if err := fixupPrefixChainScript(p, ws); err != nil {
		t.Fatal(err)
	}
	got, _ = os.ReadFile(p)
	if strings.Contains(string(got), ws) {
		t.Errorf("powershell workspace path still present:\n%s", got)
	}
	if !strings.Contains(string(got), `$env:COLCON_CURRENT_PREFIX/local_setup.ps1`) {
		t.Errorf("powershell self-source lost:\n%s", got)
	}
}
