package generator

type config struct {
	cmakeConfig cmakeConfig
}

func (c *config) generate(installedDir string) error {
	// if c.cmakeConfig.Libname == "" {
	// 	return fmt.Errorf("lib name is empty")
	// }

	// // Set namespace to libName if it is empty.
	// if c.cmakeConfig.Namespace == "" {
	// 	c.cmakeConfig.Namespace = c.cmakeConfig.Libname
	// }

	// var template string
	// if c.cmakeConfig.Libtype == "interface" {
	// 	template = expr.If(len(c.cmakeConfig.Libraries) == 0,
	// 		"templates/interface/ConfigHeadOnly.cmake.in",
	// 		"templates/interface/Config.cmake.in",
	// 	)
	// } else {
	// 	template = "templates/Config.cmake.in"
	// }

	// bytes, err := templates.ReadFile(template)
	// if err != nil {
	// 	return err
	// }

	// // Replace the placeholders with the actual values.
	// libNameUpper := strings.ReplaceAll(c.cmakeConfig.Libname, "-", "_")
	// libNameUpper = strings.ToUpper(libNameUpper)

	// content := string(bytes)
	// content = strings.ReplaceAll(content, "@LIBNAME@", c.cmakeConfig.Libname)
	// content = strings.ReplaceAll(content, "@LIBNAME_UPPER@", libNameUpper)
	// content = strings.ReplaceAll(content, "@NAMESPACE@", c.cmakeConfig.Namespace)

	// if c.cmakeConfig.Libtype == "interface" {
	// 	if len(c.cmakeConfig.Libraries) > 0 {
	// 		var libraries []string
	// 		for _, lib := range c.cmakeConfig.Libraries {
	// 			libraries = append(libraries, fmt.Sprintf(`    "${_IMPORT_PREFIX}/lib/%s"`, lib))
	// 		}
	// 		content = strings.ReplaceAll(content, "@LIBRARIES@", strings.Join(libraries, "\n"))
	// 	}
	// } else {
	// 	if len(c.cmakeConfig.Components) > 0 {
	// 		content = strings.ReplaceAll(content, "@CONFIG_OR_MODULE_FILE@", c.cmakeConfig.Namespace+"Modules.cmake")
	// 	} else {
	// 		content = strings.ReplaceAll(content, "@CONFIG_OR_MODULE_FILE@", c.cmakeConfig.Namespace+"Targets.cmake")
	// 	}
	// }

	// // Mkdirs for writing file.
	// filePath := filepath.Join(installedDir, "lib", "cmake", c.cmakeConfig.Namespace, c.cmakeConfig.Namespace+"Config.cmake")
	// if err := os.MkdirAll(filepath.Dir(filePath), os.ModePerm); err != nil {
	// 	return err
	// }

	// // Do write file.
	// if err := os.WriteFile(filePath, []byte(content), os.ModePerm); err != nil {
	// 	return err
	// }

	return nil
}
