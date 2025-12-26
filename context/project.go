package context

type Project interface {
	Init(ctx Context, projectName string) error
	GetName() string
	GetPorts() []string
	GetTargetPlatform() string
	Write(platformPath string) error
}
