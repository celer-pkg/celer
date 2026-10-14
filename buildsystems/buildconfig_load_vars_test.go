package buildsystems

import (
	"strings"
	"testing"

	"github.com/celer-pkg/celer/context"
)

func TestLoadVars_ExpandsOptions(t *testing.T) {
	var exprVars context.ExprVars
	exprVars.Put("SHARED", "global-val")

	build := &BuildConfig{
		ExprVars: exprVars,
		Vars:     []string{"BUILD_EXAMPLES=OFF"},
		Options:  []string{"-DARGS_BUILD_EXAMPLES=${BUILD_EXAMPLES}"},
	}

	if err := build.loadVars(); err != nil {
		t.Fatalf("loadVars: %v", err)
	}
	build.expandOptions()

	if got := build.Options[0]; got != "-DARGS_BUILD_EXAMPLES=OFF" {
		t.Fatalf("option = %q, want -DARGS_BUILD_EXAMPLES=OFF", got)
	}

	// Vars may reference other (global) vars and already-defined vars.
	if got, ok := build.ExprVars.Lookup("SHARED"); !ok || got != "global-val" {
		t.Fatalf("global var SHARED lost after loadVars: %q (%v)", got, ok)
	}
}

func TestLoadVars_PerConfigIsolation(t *testing.T) {
	// Both configs start from the same shared (port-level) ExprVars.
	var shared context.ExprVars
	shared.Put("PORT", "args")

	configA := &BuildConfig{
		ExprVars: shared,
		Vars:     []string{"BUILD_EXAMPLES=OFF"},
		Options:  []string{"-DARGS_BUILD_EXAMPLES=${BUILD_EXAMPLES}"},
	}
	configB := &BuildConfig{
		ExprVars: shared,
		Options:  []string{"-DARGS_BUILD_EXAMPLES=${BUILD_EXAMPLES}"},
	}

	if err := configA.loadVars(); err != nil {
		t.Fatalf("loadVars: %v", err)
	}
	configA.expandOptions()
	configB.expandOptions()

	// configA got its var substituted.
	if got := configA.Options[0]; got != "-DARGS_BUILD_EXAMPLES=OFF" {
		t.Fatalf("configA option = %q, want -DARGS_BUILD_EXAMPLES=OFF", got)
	}

	// configB, which never declared the var, must not see it: ${BUILD_EXAMPLES}
	// stays unexpanded.
	if got := configB.Options[0]; got != "-DARGS_BUILD_EXAMPLES=${BUILD_EXAMPLES}" {
		t.Fatalf("configB option = %q, var leaked across build configs", got)
	}

	// The shared map itself must remain uncontaminated.
	if _, ok := shared.Lookup("BUILD_EXAMPLES"); ok {
		t.Fatal("BUILD_EXAMPLES leaked back into the shared port-level ExprVars")
	}
}

func TestLoadVars_SkipsMalformedEntries(t *testing.T) {
	var exprVars context.ExprVars
	config := &BuildConfig{
		ExprVars: exprVars,
		Vars: []string{
			"GOOD=on",
			"no-equal-sign",
			"=emptykey",
			`QUOTED="value"`,
		},
		Options: []string{"${GOOD}|${QUOTED}"},
	}

	if err := config.loadVars(); err != nil {
		t.Fatalf("loadVars: %v", err)
	}
	config.expandOptions()

	if got := config.Options[0]; got != "on|value" {
		t.Fatalf("option = %q, want on|value", got)
	}
}

// TestLoadVars_ConflictWithExistingVar verifies that declaring a var whose key
// already exists (global/project/port-fixed) is rejected, and that on failure
// the build config's ExprVars is left untouched.
func TestLoadVars_ConflictWithExistingVar(t *testing.T) {
	var exprVars context.ExprVars
	exprVars.Put("WORKSPACE_DIR", "/some/workspace") // simulates a global var

	b := &BuildConfig{
		ExprVars: exprVars,
		Vars:     []string{"WORKSPACE_DIR=/override"},
		Options:  []string{"${WORKSPACE_DIR}"},
	}

	err := b.loadVars()
	if err == nil {
		t.Fatal("expected conflict error for WORKSPACE_DIR, got nil")
	}
	if !strings.Contains(err.Error(), "WORKSPACE_DIR") {
		t.Fatalf("error should mention WORKSPACE_DIR, got: %v", err)
	}

	// ExprVars must be unchanged: still the original, no override applied.
	if got, ok := b.ExprVars.Lookup("WORKSPACE_DIR"); !ok || got != "/some/workspace" {
		t.Fatalf("ExprVars should be untouched on failure, got %q (%v)", got, ok)
	}
}
