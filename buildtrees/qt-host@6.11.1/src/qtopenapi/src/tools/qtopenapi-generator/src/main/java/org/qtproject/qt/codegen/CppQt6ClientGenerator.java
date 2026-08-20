// Copyright (C) 2018–2025 OpenAPI Generator contributors.
// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: Apache-2.0
// Qt-Security score:critical reason:data-parser

package org.qtproject.qt.codegen;

import io.swagger.v3.oas.models.OpenAPI;
import io.swagger.v3.oas.models.info.Info;
import lombok.Setter;
import org.openapitools.codegen.*;
import org.openapitools.codegen.model.*;
import org.openapitools.codegen.meta.features.DocumentationFeature;
import org.openapitools.codegen.meta.features.GlobalFeature;
import org.openapitools.codegen.meta.features.SecurityFeature;
import org.openapitools.codegen.templating.mustache.CamelCaseAndSanitizeLambda;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import io.swagger.v3.oas.models.info.License;

import static org.openapitools.codegen.utils.StringUtils.*;

import java.util.*;
import java.io.File;

import java.nio.file.*;

public class CppQt6ClientGenerator extends CppQt6AbstractCodegen implements CodegenConfig {
    public static final String DEFAULT_PACKAGE_NAME = "Qt6OpenAPIClient";
    public static final String COMMON_LIB_NAME_OPTION = "commonLibraryName";
    public static final String DEFAULT_COMMON_LIB_NAME = "QtOpenAPICommon";
    public static final String COMMON_LIB_OPTION = "commonLibrary";
    public static final String MAKE_OPERATIONS_VIRTUAL_NAME = "makeOperationsVirtual";
    public static final String MAKE_QML_ENABLED = "enableQmlCode";
    public static final String MAKE_QML_ENABLED_DESC = "Enable registering C++ Types with the QML Type System";
    public static final String ADD_DOWNLOAD_PROGRESS = "addDownloadProgress";
    protected static final String CPP_COMMON_NAMESPACE = "cppCommonNamespace";
    protected static final String CPP_COMMON_NAMESPACE_DESC
            = "C++ namespace (convention: name::space::for::api) for the common library.";
    protected String cppCommonNamespace = "QtOpenApiCommon";

    public enum GENERATION_TYPE {
        COMMON_LIB("Gen-Common-Lib"),
        CLIENT_LIB("Gen-Client-Lib");

        public final String value;
        GENERATION_TYPE(String value) {
            this.value = value;
        }
    }
    protected String namePrefix = "";
    public static final String USE_CMAKE_FUNCTION = "useCmakeMacro";
    public static final String USE_CMAKE_FUNCTION_DESC
            = "Specifies if the 'qt_add_openapi_client' CMake macro is used to generate the code. Do not change the value manually!";
    protected String packageName = "";
    // source folder where to write the files
    protected String sourceFolder = "client";
    // source folder where to write the 'common' files
    protected String commonLibrarySourceFolder = "common";
    protected static final String USE_COMMON_LIBRARY = "enableCommonLibGeneration";
    private final Logger LOGGER = LoggerFactory.getLogger(CppQt6ClientGenerator.class);
    private final boolean makeOperationsVirtual = true;
    @Setter protected boolean addDownloadProgress = false;
    @Setter protected boolean enableQmlCode = false;
    @Setter protected String commonLibrary = GENERATION_TYPE.CLIENT_LIB.value;
    @Setter protected String commonLibraryName = DEFAULT_PACKAGE_NAME;
    @Setter protected boolean useCmakeMacro = false;
    @Setter protected String licenseName = "";

    /**
     * Configures the type of generator.
     *
     * @return  the CodegenType for this generator
     * @see     org.openapitools.codegen.CodegenType
     */
    public CodegenType getTag() {
        return CodegenType.CLIENT;
    }

    /**
     * Configures a friendly name for the generator.  This will be used by the generator
     * to select the library with the -g flag.
     *
     * @return the friendly name for the generator
     */
    public String getName() {
        return "cpp-qt6-client";
    }

    /**
     * Provides an opportunity to inspect and modify operation data before the code is generated.
     */
    @Override
    public OperationsMap postProcessOperationsWithModels(OperationsMap objs, List<ModelMap> allModels) {
        objs = super.postProcessOperationsWithModels(objs, allModels);
        removeImport(objs, "#include <QList>");
        return objs;
    }

    /**
     * Returns human-friendly help for the generator.  Provide the consumer with help
     * tips, parameters here
     *
     * @return A string value for the help message
     */
    public String getHelp() {
        return "Generates a cpp-qt6-client client library.";
    }

    public CppQt6ClientGenerator() {
        super();
        modifyFeatureSet(features -> features
                .includeDocumentationFeatures(DocumentationFeature.Readme)
                .includeGlobalFeatures(GlobalFeature.ParameterizedServer)
                .includeGlobalFeatures(GlobalFeature.MultiServer)
                .includeSecurityFeatures(SecurityFeature.BasicAuth)
                .includeSecurityFeatures(SecurityFeature.ApiKey)
                .includeSecurityFeatures(SecurityFeature.BearerToken)
                .includeGlobalFeatures(GlobalFeature.ParameterStyling)
        );

        // set the output folder here
        outputFolder = "generated-code/cpp-qt6-client";

        /**
         * Models.  You can write model files using the modelTemplateFiles map.
         * if you want to create one template for file, you can do so here.
         * for multiple files for model, just put another entry in the `modelTemplateFiles` with
         * a different extension
         */
        modelTemplateFiles.put(
                    "model-header.mustache",
                    ".h");

        modelTemplateFiles.put(
                    "model-body.mustache",
                    ".cpp");

        /**
         * Api classes.  You can write classes for each Api file with the apiTemplateFiles map.
         * as with models, add multiple entries with different extensions for multiple files per
         * class
         */
        apiTemplateFiles.put(
                    "api-header.mustache",   // the template to use
                    ".h");       // the extension for each file to write

        apiTemplateFiles.put(
                   "api-body.mustache",   // the template to use
                    ".cpp");       // the extension for each file to write

        // CLI options
        addOption(CPP_COMMON_NAMESPACE, CPP_COMMON_NAMESPACE_DESC, this.cppCommonNamespace);
        addOption(CodegenConstants.PACKAGE_NAME, "C++ package (library) name.", DEFAULT_PACKAGE_NAME);
        addSwitch(ADD_DOWNLOAD_PROGRESS, "Add support for Qt download progress", this.addDownloadProgress);
        addSwitch(MAKE_QML_ENABLED, MAKE_QML_ENABLED_DESC, this.enableQmlCode);
        addSwitch(USE_CMAKE_FUNCTION, USE_CMAKE_FUNCTION_DESC, this.useCmakeMacro);
        // Common library name allows to choose a unique name for 'commonLibrary=COMMON_LIB' case.
        addOption(COMMON_LIB_NAME_OPTION, "Name of the common client library, if generated.",
                  DEFAULT_COMMON_LIB_NAME);
        // 'commonLibrary' option allows to choose the generation mode for common resources.
        // Possible generation modes:
        // 'COMMON_LIB' - generates common files as a separate library.
        // 'CLIENT_LIB' - generate client lib as a separate library.
        CliOption commonLib = new CliOption(COMMON_LIB_OPTION,
                "Generate common library for the client or not.");
        Map<String, String> commonLibOptions = new HashMap<>();
        commonLibOptions.put(GENERATION_TYPE.COMMON_LIB.value,
                "The common resources will be generated as a Common library.");
        commonLibOptions.put(GENERATION_TYPE.CLIENT_LIB.value,
                "The Client will be generated without common files at all.");
        commonLib.setEnum(commonLibOptions);
        commonLib.setDefault(this.commonLibrary);
        this.cliOptions.add(commonLib);

        /**
         * Template Location.  This is the location which templates will be read from.  The generator
         * will use the resource stream to attempt to read the templates.
         */
        templateDir = "cpp-qt6-client";
        typeMapping.put("AnyType", "QJsonValue");
        importMapping.put("QJsonValue", "#include <QtCore/qjsonvalue.h>");

        reservedWords.add("valid");
        reservedWords.add("set");

        /**
         * Additional Properties.  These values can be passed to the templates and
         * are available in models, apis, and supporting files
         */
        final String pluginVersion = CppQt6ClientGenerator.class.getPackage().getImplementationVersion();
        additionalProperties.put("pluginVersion", pluginVersion);
        additionalProperties.put("prefix", namePrefix);
        additionalProperties.put("camelcase", new CamelCaseAndSanitizeLambda(false).generator(this));
        additionalProperties.put("cppCommonNamespace", cppCommonNamespace);
    }

    @Override
    public void preprocessOpenAPI(OpenAPI openAPI) {
        super.preprocessOpenAPI(openAPI);

        final Info info = openAPI.getInfo();
        if (info != null) {
            License license = info.getLicense();
            if (license != null) {
                licenseName = license.getName();
            }
        }
        additionalProperties.put(CodegenConstants.LICENSE_NAME, licenseName);
    }

    @Override
    public void processOpts() {
        super.processOpts();

        String genVersion = (String) additionalProperties.get("generatorVersion");
        // Currently the generatorVersion property is populated *after* processOpts()
        // is called, so we need to take it into account, and extract the
        // version information on our own. However, that might change in future, and
        // that's why we still read the property above.
        if (genVersion == null)
            genVersion = DefaultGenerator.class.getPackage().getImplementationVersion();

        final String[] versionParts = genVersion.split("\\.");
        additionalProperties.put("generatorVersionMajor",
                                 versionParts.length > 0 ? versionParts[0] : "0");
        additionalProperties.put("generatorVersionMinor",
                                 versionParts.length > 1 ? versionParts[1] : "0");
        additionalProperties.put("generatorVersionPatch",
                                 versionParts.length > 2 ? versionParts[2] : "0");

        if (additionalProperties.containsKey("cppCommonNamespace")) {
            cppCommonNamespace = (String) additionalProperties.get("cppCommonNamespace");
        }
        additionalProperties.put("cppCommonNamespaceDeclarations", cppCommonNamespace.split("\\::"));

        packageName = (String) additionalProperties.getOrDefault(CodegenConstants.PACKAGE_NAME, DEFAULT_PACKAGE_NAME);
        commonLibraryName = (String) additionalProperties.getOrDefault(COMMON_LIB_NAME_OPTION,
                                                                       DEFAULT_COMMON_LIB_NAME);

        additionalProperties.put(MAKE_OPERATIONS_VIRTUAL_NAME, makeOperationsVirtual);

        if (additionalProperties.containsKey(MAKE_QML_ENABLED)) {
            setEnableQmlCode(convertPropertyToBooleanAndWriteBack(MAKE_QML_ENABLED));
        } else {
            additionalProperties.put(MAKE_QML_ENABLED, enableQmlCode);
        }

        if (additionalProperties.containsKey(USE_CMAKE_FUNCTION)) {
            setUseCmakeMacro(convertPropertyToBooleanAndWriteBack(USE_CMAKE_FUNCTION));
        } else {
            additionalProperties.put(USE_CMAKE_FUNCTION, useCmakeMacro);
        }

        if (additionalProperties.containsKey(ADD_DOWNLOAD_PROGRESS)) {
            setAddDownloadProgress(convertPropertyToBooleanAndWriteBack(ADD_DOWNLOAD_PROGRESS));
        } else {
            additionalProperties.put(ADD_DOWNLOAD_PROGRESS, addDownloadProgress);
        }

        additionalProperties.put(CodegenConstants.PACKAGE_NAME, packageName);
        final String lowerPackageName = packageName.toLowerCase();
        additionalProperties.put("packageNameLowerCase", lowerPackageName);
        additionalProperties.put("packageNameUpperCase", packageName.toUpperCase());
        if (additionalProperties.containsKey(COMMON_LIB_OPTION)
                && !additionalProperties.get(COMMON_LIB_OPTION).toString().isEmpty()) {
            setCommonLibrary(additionalProperties.get(COMMON_LIB_OPTION).toString());
        } else {
            additionalProperties.put(COMMON_LIB_OPTION, this.commonLibrary);
        }
        // If common library mode is ON, then make sense to handle the common library name.
        if (commonLibrary.equals(GENERATION_TYPE.COMMON_LIB.value)) {
            if (additionalProperties.containsKey(COMMON_LIB_NAME_OPTION)) {
                setCommonLibraryName(additionalProperties.get(COMMON_LIB_NAME_OPTION).toString());
            } else {
                additionalProperties.put(COMMON_LIB_NAME_OPTION, this.commonLibraryName);
            }
        }
        // The 'enableCommonLibGeneration' mustache-key required to enable/disable
        // common library generation.
        additionalProperties.put(USE_COMMON_LIBRARY,
                                 commonLibrary.equals(GENERATION_TYPE.COMMON_LIB.value));

        // If we're using a CMake macro and building the common library, then
        // assume that we're doing it as a part of Qt build, and enable some
        // specific features.
        // This additional property intentionally cannot be directly controlled
        // from the command line.
        if (useCmakeMacro && commonLibrary.equals(GENERATION_TYPE.COMMON_LIB.value)) {
            additionalProperties.put("buildingQtSources", true);
        }

        String lowerCasePrefix = "";
        if (additionalProperties.containsKey("prefix")) {
            namePrefix = additionalProperties.get("prefix").toString();
            lowerCasePrefix = namePrefix.toLowerCase();
            additionalProperties.put("prefixLowerCase", lowerCasePrefix);
        }
        supportingFiles.clear();
        supportingFiles.add(new SupportingFile("README.mustache",
                sourceFolder, "README.md"));
        supportingFiles.add(new SupportingFile("CMakeConfig.mustache",
                sourceFolder, "config.cmake.in"));
        if (!this.useCmakeMacro) {
            supportingFiles.add(new SupportingFile("CMakeLists.txt.mustache",
                    sourceFolder, "CMakeLists.txt"));
        }
        supportingFiles.add(new SupportingFile("exports.mustache",
                sourceFolder, lowerPackageName + "exports.h"));
        supportingFiles.add(new SupportingFile("doc/Doxyfile.in.mustache",
                sourceFolder, "doc/Doxyfile.in"));
        typeMapping.put("object", namePrefix + "Object");
        typeMapping.put("file", namePrefix + "HttpFileElement");
        importMapping.put(namePrefix + "HttpFileElement", "#include \""
                + lowerCasePrefix + "httpfileelement.h\"");

        if (commonLibrary.equals(GENERATION_TYPE.CLIENT_LIB.value)) {
            LOGGER.info("Skipping ./common/* templates generation. 'Skip-Common-Files' is ON.");
        } else {
            if (this.useCmakeMacro) {
                modelTemplateFiles.clear();
                apiTemplateFiles.clear();
            }
            supportingFiles.add(new SupportingFile("common/api-base-header.mustache",
                    commonLibrarySourceFolder, lowerCasePrefix + "baseapi.h"));
            supportingFiles.add(new SupportingFile("common/api-base-body.mustache",
                    commonLibrarySourceFolder, lowerCasePrefix + "baseapi.cpp"));
            supportingFiles.add(new SupportingFile("common/helpers-header.mustache",
                    commonLibrarySourceFolder, lowerCasePrefix + "helpers.h"));
            supportingFiles.add(new SupportingFile("common/helpers-body.mustache",
                    commonLibrarySourceFolder, lowerCasePrefix + "helpers.cpp"));
            supportingFiles.add(new SupportingFile("common/httprequest.h.mustache",
                    commonLibrarySourceFolder, lowerCasePrefix + "httprequest.h"));
            supportingFiles.add(new SupportingFile("common/httprequest.cpp.mustache",
                    commonLibrarySourceFolder, lowerCasePrefix + "httprequest.cpp"));
            supportingFiles.add(new SupportingFile("common/httpfileelement.h.mustache",
                    commonLibrarySourceFolder, lowerCasePrefix + "httpfileelement.h"));
            supportingFiles.add(new SupportingFile("common/httpfileelement.cpp.mustache",
                    commonLibrarySourceFolder, lowerCasePrefix + "httpfileelement.cpp"));
            supportingFiles.add(new SupportingFile("common/object-header.mustache",
                    commonLibrarySourceFolder, lowerCasePrefix + "object.h"));
                    supportingFiles.add(new SupportingFile("common/object-body.mustache",
                    commonLibrarySourceFolder, lowerCasePrefix + "object.cpp"));
            supportingFiles.add(new SupportingFile("common/enum-header.mustache",
                    commonLibrarySourceFolder, lowerCasePrefix + "enum.h"));
                    supportingFiles.add(new SupportingFile("common/enum-body.mustache",
                    commonLibrarySourceFolder, lowerCasePrefix + "enum.cpp"));
            supportingFiles.add(new SupportingFile("common/common-exports.mustache",
                    commonLibrarySourceFolder, lowerCasePrefix + lowerPackageName + "commonexports.h"));
            supportingFiles.add(new SupportingFile("common/common-global.mustache",
                    commonLibrarySourceFolder, lowerCasePrefix + "commonglobal.h"));
            supportingFiles.add(new SupportingFile("common/serverconfiguration.h.mustache",
                    commonLibrarySourceFolder, lowerCasePrefix + "serverconfiguration.h"));
            supportingFiles.add(new SupportingFile("common/serverconfiguration.cpp.mustache",
                    commonLibrarySourceFolder, lowerCasePrefix + "serverconfiguration.cpp"));
            supportingFiles.add(new SupportingFile("common/servervariable.h.mustache",
                    commonLibrarySourceFolder, lowerCasePrefix + "servervariable.h"));
            supportingFiles.add(new SupportingFile("common/servervariable.cpp.mustache",
                    commonLibrarySourceFolder, lowerCasePrefix + "servervariable.cpp"));
            supportingFiles.add(new SupportingFile("common/CMakeConfig.mustache",
                    commonLibrarySourceFolder, "config.cmake.in"));
            if (!this.useCmakeMacro) {
                supportingFiles.add(new SupportingFile("common/CMakeLists.txt.mustache",
                        commonLibrarySourceFolder, "CMakeLists.txt"));
            }
        }

        // Get all the dependencies from the main yaml file and write them
        // into outputFolder/.openapi-generator/yaml_deps.txt
        // Intentionally do it only when useCmakeMacro=true.
        if (useCmakeMacro) {
            final String inputSpec = this.getInputSpec();
            YamlDependencyBuilder depBuilder = new YamlDependencyBuilder();
            Set<String> yamlDeps = depBuilder.getDependencies(inputSpec);
            try {
                Path yamlDepFile = Paths.get(outputFolder + File.separator
                                            + ".openapi-generator" + File.separator
                                            + "yaml_deps.txt");
                Files.createDirectories(yamlDepFile.getParent());
                Files.write(yamlDepFile, yamlDeps);
            } catch (Exception e) {
                // Failed to write the file - let CMake handle it
            }
        }
    }

    /**
     * Location to write model files.  You can use the modelPackage() as defined when the class is
     * instantiated
     */
    public String modelFileFolder() {
        return outputFolder + "/" + sourceFolder + "/" + modelPackage().replace("::", File.separator);
    }

    /**
     * Location to write api files.  You can use the apiPackage() as defined when the class is
     * instantiated
     */
    @Override
    public String apiFileFolder() {
        return outputFolder + "/" + sourceFolder + "/" + apiPackage().replace("::", File.separator);
    }

    /**
     * override with any special text escaping logic to handle unsafe
     * characters so as to avoid code injection
     *
     * @param input String to be cleaned up
     * @return string with unsafe characters removed or escaped
     */
    @Override
    public String escapeUnsafeCharacters(String input) {
        //TODO: check that this logic is safe to escape unsafe characters to avoid code injection
        return input;
    }

    /**
     * Escape single and/or double quote to avoid code injection
     *
     * @param input String to be cleaned up
     * @return string with quotation mark removed or escaped
     */
    public String escapeQuotationMark(String input) {
        //TODO: check that this logic is safe to escape quotation mark to avoid code injection
        return input.replace("\"", "\\\"");
    }

    @Override
    public String toApiFilename(String name) {
        final String suffix =
                (apiNameSuffix != null && !apiNameSuffix.isEmpty()) ? apiNameSuffix : "Api";
        final String apiFile = modelNamePrefix + sanitizeName(name) + suffix ;
        return apiFile.toLowerCase();
    }
}
