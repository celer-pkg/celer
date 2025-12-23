package generator

type targetsBuildType struct {
	cmakeConfig cmakeConfig
}

func (g *targetsBuildType) generate(installedDir string) error {
	// Set namespace to libName if it is empty.
	// if g.cmakeConfig.Namespace == "" {
	// 	g.cmakeConfig.Namespace = g.cmakeConfig.Libname
	// }

	// if g.cmakeConfig.Libname == "" {
	// 	return fmt.Errorf("lib name is empty")
	// }

	// if g.cmakeConfig.Libtype == "" {
	// 	return fmt.Errorf("lib type is empty")
	// }

	// if g.cmakeConfig.BuildType == "" {
	// 	return fmt.Errorf("build type is empty")
	// }

	// // Validate importName for windows and soName for linux.
	// switch strings.ToLower(g.cmakeConfig.SystemName) {
	// case "windows":
	// 	if g.cmakeConfig.Libtype == "shared" && g.cmakeConfig.Impname == "" {
	// 		return fmt.Errorf("import name is empty for windows shared lib")
	// 	}

	// 	if !fileio.PathExists(filepath.Join(installedDir, "lib", g.cmakeConfig.Impname)) {
	// 		return fmt.Errorf("import name %s doesn't exists", g.cmakeConfig.Impname)
	// 	}

	// case "linux":
	// 	if g.cmakeConfig.Libtype == "shared" && g.cmakeConfig.Soname == "" {
	// 		return fmt.Errorf("so name is empty for linux shared lib")
	// 	}

	// 	if !fileio.PathExists(filepath.Join(installedDir, "lib", g.cmakeConfig.Soname)) {
	// 		return fmt.Errorf("so name %s doesn't exists", g.cmakeConfig.Soname)
	// 	}
	// }

	// template := fmt.Sprintf("templates/%s/targets-buildtype-%s.cmake.in",
	// 	strings.ToLower(g.cmakeConfig.SystemName),
	// 	strings.ToLower(g.cmakeConfig.Libtype))
	// bytes, err := templates.ReadFile(template)
	// if err != nil {
	// 	return err
	// }

	// // Replace the placeholders with the actual values.
	// libNameUpper := strings.ReplaceAll(g.cmakeConfig.Libname, "-", "_")
	// libNameUpper = strings.ToUpper(libNameUpper)

	// content := string(bytes)
	// content = strings.ReplaceAll(content, "@NAMESPACE@", g.cmakeConfig.Namespace)
	// content = strings.ReplaceAll(content, "@LIBNAME@", g.cmakeConfig.Libname)
	// content = strings.ReplaceAll(content, "@LIBNAME_UPPER@", libNameUpper)
	// content = strings.ReplaceAll(content, "@LIBTYPE@", g.cmakeConfig.Libtype)
	// content = strings.ReplaceAll(content, "@FILENAME@", g.cmakeConfig.Filename)
	// content = strings.ReplaceAll(content, "@BUILD_TYPE@", g.cmakeConfig.BuildType)
	// content = strings.ReplaceAll(content, "@BUILD_TYPE_UPPER@", strings.ToUpper(g.cmakeConfig.BuildType))

	// switch strings.ToLower(g.cmakeConfig.SystemName) {
	// case "windows":
	// 	content = strings.ReplaceAll(content, "@IMPNAME@", g.cmakeConfig.Impname)

	// case "linux":
	// 	content = strings.ReplaceAll(content, "@SO_NAME@", g.cmakeConfig.Soname)

	// default:
	// 	return fmt.Errorf("unsupported system name: %s", g.cmakeConfig.SystemName)
	// }

	// // Make dirs for writing file.
	// fileName := fmt.Sprintf("%sTargets-%s.cmake", g.cmakeConfig.Namespace, g.cmakeConfig.BuildType)
	// filePath := filepath.Join(installedDir, "lib", "cmake", g.cmakeConfig.Namespace, fileName)
	// if err := os.MkdirAll(filepath.Dir(filePath), os.ModePerm); err != nil {
	// 	return err
	// }

	// // Do write file.
	// if err := os.WriteFile(filePath, []byte(content), os.ModePerm); err != nil {
	// 	return err
	// }

	return nil
}
