package configs

import (
	"celer/context"
	"celer/pkgs/dirs"
	"celer/pkgs/expr"
	"celer/pkgs/fileio"
	"fmt"
	"os"
	"path/filepath"
	"runtime"
	"strings"

	"github.com/BurntSushi/toml"
)

type Platform struct {
	RootFS     *RootFS     `toml:"rootfs"`
	Toolchain  *Toolchain  `toml:"toolchain"`
	WindowsKit *WindowsKit `toml:"windows_kit"`

	// Internal fields.
	Name string `toml:"-"`
	ctx  context.Context
}

func (p *Platform) Init(platformName string) error {
	// Init internal fields.
	p.Name = platformName

	// Check if platform name is empty.
	platformName = strings.TrimSpace(platformName)
	if platformName == "" {
		return fmt.Errorf("platform name is empty")
	}

	// Check if platform file exists, but ignore "gcc" and "clang".
	if platformName != "gcc" &&
		platformName != "msvc" &&
		platformName != "clang" &&
		platformName != "clang-cl" {
		platformPath := filepath.Join(dirs.ConfPlatformsDir, platformName+".toml")
		if !fileio.PathExists(platformPath) {
			return fmt.Errorf("platform does not exist: %s", platformName)
		}

		// Read conf/celer.toml
		bytes, err := os.ReadFile(platformPath)
		if err != nil {
			return err
		}
		if err := toml.Unmarshal(bytes, p); err != nil {
			return fmt.Errorf("failed to read %s.\n %w", platformPath, err)
		}
	}

	if p.RootFS != nil {
		p.RootFS.ctx = p.ctx
		if err := p.RootFS.Validate(); err != nil {
			return err
		}
	}
	if p.Toolchain != nil {
		p.Toolchain.ctx = p.ctx
		if err := p.Toolchain.Validate(); err != nil {
			return err
		}
	}

	return nil
}

func (p Platform) GetName() string {
	return p.Name
}

func (p Platform) GetHostName() string {
	var arch string
	switch runtime.GOARCH {
	case "amd64":
		arch = "x86_64"
	case "386":
		arch = "x86"
	case "arm":
		arch = "arm"
	case "arm64":
		arch = "aarch64"
	default:
		panic("unsupported architecture: " + runtime.GOARCH)
	}

	switch runtime.GOOS {
	case "windows":
		return arch + "-windows"

	case "darwin":
		return arch + "-darwin"

	case "linux":
		return arch + "-linux"

	default:
		panic("unsupported operating system: " + runtime.GOOS)
	}
}

func (p Platform) GetToolchain() context.Toolchain {
	return p.Toolchain
}

func (p *Platform) Write(platformPath string) error {
	// Create empty platform.
	p.RootFS = &RootFS{
		PkgConfigPath: []string{},
		IncludeDirs:   []string{},
		LibDirs:       []string{},
	}
	p.Toolchain = &Toolchain{}

	bytes, err := toml.Marshal(p)
	if err != nil {
		return err
	}

	// Check if conf/celer.toml exists.
	if fileio.PathExists(platformPath) {
		return fmt.Errorf("%s is already exists", platformPath)
	}

	// Make sure the parent directory exists.
	parentDir := filepath.Dir(platformPath)
	if err := os.MkdirAll(parentDir, os.ModePerm); err != nil {
		return err
	}
	return os.WriteFile(platformPath, bytes, os.ModePerm)
}

// Setup build envs.
func (p *Platform) Setup() error {
	// Repair rootfs if not empty.
	if p.RootFS != nil {
		if err := p.RootFS.CheckAndRepair(); err != nil {
			return fmt.Errorf("failed to check and repair rootfs.\n %w", err)
		}
	}

	// WindowsKit can be detected automatically.
	if runtime.GOOS == "windows" && p.WindowsKit == nil {
		var windowsKit WindowsKit
		if err := windowsKit.Detect(&p.Toolchain.MSVC); err != nil {
			return fmt.Errorf("failed to detect celer.windows_kit.\n: %w", err)
		}
		p.WindowsKit = &windowsKit
	}

	// Repair toolchain.
	if p.Toolchain == nil {
		panic("Toolchain should not be empty, it may specified in platform or automatically detected.")
	}
	if err := p.Toolchain.CheckAndRepair(false); err != nil {
		return fmt.Errorf("failed to check and repair toolchain.\n %w", err)
	}

	// Only for Windows MSVC.
	if runtime.GOOS == "windows" {
		if p.Toolchain.Name == "msvc" ||
			p.Toolchain.Name == "clang" ||
			p.Toolchain.Name == "clang-cl" {
			p.Toolchain.MSVC.VCVars = filepath.Join(p.Toolchain.rootDir, "VC", "Auxiliary", "Build", "vcvarsall.bat")
		}
	}

	// Generate toolchain file.
	if err := p.ctx.GenerateToolchainFile(); err != nil {
		return fmt.Errorf("failed to generate toolchain file.\n %w", err)
	}

	return nil
}

func (p *Platform) detectToolchain(platformName string) error {
	// Detect toolchain.
	var toolchain = Toolchain{ctx: p.ctx}
	if err := toolchain.Detect(platformName); err != nil {
		return fmt.Errorf("detect celer.toolchain: %w", err)
	}
	p.Toolchain = &toolchain
	p.Toolchain.SystemName = expr.UpperFirst(runtime.GOOS)
	p.Toolchain.SystemProcessor = runtime.GOARCH

	// WindowsKit can be detected automatically.
	if runtime.GOOS == "windows" && p.WindowsKit == nil {
		var windowsKit WindowsKit
		if err := windowsKit.Detect(&p.Toolchain.MSVC); err != nil {
			return fmt.Errorf("failed to detect celer.windows_kit.\n %w", err)
		}
		p.WindowsKit = &windowsKit
	}

	// Assign standard toolchain name.
	switch runtime.GOOS {
	case "windows":
		if p.Toolchain.Name == "msvc" || p.Toolchain.Name == "clang" || p.Toolchain.Name == "clang-cl" {
			p.Name = "x86_64-windows"
		} else {
			return fmt.Errorf("unsupported toolchian %s", p.Toolchain.Name)
		}

	case "linux":
		p.Name = "x86_64-linux"
	}

	return nil
}
