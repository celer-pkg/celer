package context

type Port interface {
	Init(ctx Context, nameVersion string) error
	NameVersion() string
	Clone(repoUrl, repoRef, archive string) error
}
