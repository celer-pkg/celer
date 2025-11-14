package context

type Project interface {
	Init(ctx Context, projectName string) error
	GetName() string
	GetPorts() []string
	GetDefaultPlatform() string
	Write(platformPath string) error
}
