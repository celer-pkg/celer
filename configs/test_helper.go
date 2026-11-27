package configs

import "github.com/celer-pkg/celer/context"

type fakeContext struct {
	context.Context
	buildType string
}

func (f fakeContext) BuildType() string           { return f.buildType }
func (f fakeContext) Platform() context.Platform  { return fakePlatform{} }
func (f fakeContext) Project() context.Project    { return fakeProject{} }
func (f fakeContext) ExprVars() *context.ExprVars { return nil }

type fakePlatform struct {
	context.Platform
}

func (fakePlatform) GetName() string     { return "test-platform" }
func (fakePlatform) GetHostName() string { return "test-host" }

type fakeProject struct {
	context.Project
}

func (fakeProject) GetName() string { return "test-project" }
