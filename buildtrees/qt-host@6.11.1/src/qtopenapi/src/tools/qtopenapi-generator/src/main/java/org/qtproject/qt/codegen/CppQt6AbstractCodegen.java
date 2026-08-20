// Copyright (C) –2025 OpenAPI Generator contributors.
// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: Apache-2.0
// Qt-Security score:critical reason:data-parser

package org.qtproject.qt.codegen;

import io.swagger.v3.oas.models.media.Schema;
import io.swagger.v3.parser.util.SchemaTypeUtil;
import org.apache.commons.lang3.StringUtils;
import org.openapitools.codegen.*;
import org.openapitools.codegen.languages.AbstractCppCodegen;
import org.openapitools.codegen.meta.features.*;
import org.openapitools.codegen.model.ModelMap;
import org.openapitools.codegen.model.OperationMap;
import org.openapitools.codegen.model.OperationsMap;
import org.openapitools.codegen.utils.ModelUtils;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.io.File;
import java.util.*;

import static org.openapitools.codegen.utils.CamelizeOption.LOWERCASE_FIRST_LETTER;

public abstract class CppQt6AbstractCodegen extends AbstractCppCodegen implements CodegenConfig {
    private final Logger LOGGER = LoggerFactory.getLogger(CppQt6AbstractCodegen.class);
    protected static final String CPP_NAMESPACE = "cppNamespace";
    protected static final String CPP_NAMESPACE_DESC = "C++ namespace (convention: name::space::for::api).";
    protected static final String CONTENT_COMPRESSION_ENABLED = "contentCompression";
    protected static final String CONTENT_COMPRESSION_ENABLED_DESC = "Enable Compressed Content Encoding for requests and responses";
    protected Set<String> foundationClasses = new HashSet<>();
    protected String cppNamespace = "QtOpenAPI";
    protected Map<String, String> namespaces = new HashMap<>();
    protected Set<String> systemIncludes = new HashSet<>();
    protected boolean isContentCompressionEnabled = false;
    protected Set<String> nonFrameworkPrimitives = new HashSet<>();

    public CppQt6AbstractCodegen() {
        super();

        modifyFeatureSet(features -> features
                .excludeWireFormatFeatures(WireFormatFeature.PROTOBUF)
                .securityFeatures(EnumSet.noneOf(SecurityFeature.class))
                .excludeGlobalFeatures(
                        GlobalFeature.XMLStructureDefinitions,
                        GlobalFeature.Callbacks,
                        GlobalFeature.LinkObjects,
                        GlobalFeature.ParameterStyling,
                        GlobalFeature.MultiServer
                )
                .includeSchemaSupportFeatures(
                        SchemaSupportFeature.Polymorphism
                )
                .includeParameterFeatures(
                        ParameterFeature.Cookie
                )
        );

        // CLI options
        addOption(CPP_NAMESPACE, CPP_NAMESPACE_DESC, this.cppNamespace);
        addOption(CodegenConstants.MODEL_NAME_PREFIX, CodegenConstants.MODEL_NAME_PREFIX_DESC, this.modelNamePrefix);
        addSwitch(CONTENT_COMPRESSION_ENABLED, CONTENT_COMPRESSION_ENABLED_DESC, this.isContentCompressionEnabled);

        // For being compliant with Qt naming convention the variable always
        // starts with lower-case letter.
        // Also, starting with uppercase letter leads to clashing class and parameter names,
        // because there is no additional checks implemented.
        removeOption(super.VARIABLE_NAME_FIRST_CHARACTER_UPPERCASE_OPTION);

        // The option is defined in DefaultCodegen class,
        // but it isn't used by the Qt6 generator. Should be deleted from options list.
        removeOption(CodegenConstants.ALLOW_UNICODE_IDENTIFIERS);

        // This option is inherited from DefaultCodegen but should not be configurable in the Qt6
        // generator. In C++, parameters with default values must appear at the end of the
        // parameter list. To enforce this, sorting by required flag is always enabled.
        removeOption(CodegenConstants.SORT_PARAMS_BY_REQUIRED_FLAG);

        /*
         * Additional Properties.  These values can be passed to the templates and
         * are available in models, apis, and supporting files
         */

        // Write defaults namespace in properties so that it can be accessible in templates.
        // At this point command line has not been parsed so if value is given
        // in command line it will supersede this content
        additionalProperties.put("cppNamespace", cppNamespace);
        /*
         * Language Specific Primitives.  These types will not trigger imports by
         * the client generator
         */
        languageSpecificPrimitives = new HashSet<>(
                Arrays.asList(
                        "bool",
                        "qint32",
                        "qint64",
                        "float",
                        "double")
        );
        nonFrameworkPrimitives.addAll(languageSpecificPrimitives);

        foundationClasses.addAll(
                Arrays.asList(
                        "QString",
                        "QDate",
                        "QDateTime",
                        "QByteArray")
        );
        languageSpecificPrimitives.addAll(foundationClasses);
        super.typeMapping = new HashMap<>();
        typeMapping.put("date", "QDate");
        typeMapping.put("DateTime", "QDateTime");
        typeMapping.put("string", "QString");
        typeMapping.put("integer", "qint32");
        typeMapping.put("long", "qint64");
        typeMapping.put("boolean", "bool");
        typeMapping.put("number", "double");
        typeMapping.put("array", "QList");
        typeMapping.put("map", "QMap");
        typeMapping.put("set", "QSet");
        // mapped as "file" type for OAS 3.0
        typeMapping.put("ByteArray", "QByteArray");
        //   UUID support - possible enhancement : use QUuid instead of QString.
        //   beware though that Serialization/de-serialization of QUuid does not
        //   come out of the box and will need to be sorted out (at least imply
        //   modifications on multiple templates)
        typeMapping.put("UUID", "QString");
        typeMapping.put("URI", "QString");
        typeMapping.put("file", "QByteArray");
        typeMapping.put("binary", "QByteArray");
        importMapping = new HashMap<>();
        namespaces = new HashMap<>();

        systemIncludes.add("QString");
        systemIncludes.add("QList");
        systemIncludes.add("QMap");
        systemIncludes.add("QSet");
        systemIncludes.add("QDate");
        systemIncludes.add("QDateTime");
        systemIncludes.add("QByteArray");

        reservedWords.add("signals");
        reservedWords.add("slots");
    }

    @Override
    public void processOpts() {
        super.processOpts();
        if (additionalProperties.containsKey("cppNamespace")) {
            cppNamespace = (String) additionalProperties.get("cppNamespace");
        }
        additionalProperties.put("cppNamespaceDeclarations", cppNamespace.split("\\::"));
        if (additionalProperties.containsKey("modelNamePrefix")) {
            modelNamePrefix = (String) additionalProperties.get("modelNamePrefix");
            additionalProperties().put("modelNamePrefix", modelNamePrefix);
        }
        if (additionalProperties.containsKey(CONTENT_COMPRESSION_ENABLED)) {
            setContentCompressionEnabled(convertPropertyToBooleanAndWriteBack(CONTENT_COMPRESSION_ENABLED));
        } else {
            additionalProperties.put(CONTENT_COMPRESSION_ENABLED, isContentCompressionEnabled);
        }
        if (Boolean.TRUE.equals(
            additionalProperties.get(CodegenConstants.ALLOW_UNICODE_IDENTIFIERS))) {
            LOGGER.warn("allowUnicodeIdentifiers is not supported in the Qt6 generator due to "
                        + "MOC limitations and will be reset to false.");
        }
        setAllowUnicodeIdentifiers(false);
        if (Boolean.FALSE.equals(
            additionalProperties.get(CodegenConstants.SORT_PARAMS_BY_REQUIRED_FLAG))) {
            LOGGER.warn("setSortParamsByRequiredFlag is not supported in the Qt6 generator because "
                        + "reordering parameters may place parameters with default values before "
                        + "required ones, which is invalid in C++.");
        }
        setSortParamsByRequiredFlag(true);
    }

    @Override
    public String toModelImport(String name) {
        if (name.isEmpty()) {
            return null;
        }

        if (namespaces.containsKey(name)) {
            return "using " + namespaces.get(name) + ";";
        } else if (systemIncludes.contains(name)) {
            return "#include <" + name + ">";
        } else if (importMapping.containsKey(name)) {
            return importMapping.get(name);
        }

        String folder = modelPackage().replace("::", File.separator);
        if (!folder.isEmpty())
            folder += File.separator;

        return "#include \"" + folder + name.toLowerCase() + ".h\"";
    }

    /**
     * Optional - type declaration.  This is a String which is used by the templates to instantiate your
     * types.  There is typically special handling for different property types
     *
     * @return a string value used as the `dataType` field for model templates, `returnType` for api templates
     */
    @Override
    @SuppressWarnings("rawtypes")
    public String getTypeDeclaration(Schema p) {
        String openAPIType = getSchemaType(p);

        if (ModelUtils.isArraySchema(p)) {
            Schema inner = ModelUtils.getSchemaItems(p);
            return getSchemaType(p) + "<" + getTypeDeclaration(inner) + ">";
        } else if (ModelUtils.isMapSchema(p)) {
            Schema inner = ModelUtils.getAdditionalProperties(p);
            return getSchemaType(p) + "<QString, " + getTypeDeclaration(inner) + ">";
        } else if (ModelUtils.isBinarySchema(p)) {
            return getSchemaType(p);
        } else if (ModelUtils.isFileSchema(p)) {
            return getSchemaType(p);
        }
        if (foundationClasses.contains(openAPIType)) {
            return openAPIType;
        } else if (languageSpecificPrimitives.contains(openAPIType)) {
            return toModelName(openAPIType);
        } else {
            return openAPIType;
        }
    }

    @Override
    @SuppressWarnings("rawtypes")
    public String toDefaultValue(Schema p) {
        if (ModelUtils.isBooleanSchema(p)) {
            return "false";
        } else if (ModelUtils.isDateSchema(p)) {
            return "nullptr";
        } else if (ModelUtils.isDateTimeSchema(p)) {
            return "nullptr";
        } else if (ModelUtils.isNumberSchema(p)) {
            if (SchemaTypeUtil.FLOAT_FORMAT.equals(p.getFormat())) {
                return "0.0f";
            }
            return "0.0";
        } else if (ModelUtils.isIntegerSchema(p)) {
            if (SchemaTypeUtil.INTEGER64_FORMAT.equals(p.getFormat())) {
                return "0L";
            }
            return "0";
        } else if (ModelUtils.isMapSchema(p)) {
            Schema inner = ModelUtils.getAdditionalProperties(p);
            return "QMap<QString, " + getTypeDeclaration(inner) + ">()";
        } else if (ModelUtils.isArraySchema(p)) {
            Schema inner = ModelUtils.getSchemaItems(p);
            return "QList<" + getTypeDeclaration(inner) + ">()";
        } else if (ModelUtils.isStringSchema(p)) {
            return "QString(\"\")";
        } else if (!StringUtils.isEmpty(p.get$ref())) {
            return toModelName(ModelUtils.getSimpleRef(p.get$ref())) + "()";
        }
        return "nullptr";
    }

    @Override
    public String toModelFilename(String name) {
        return toModelName(name).toLowerCase();
    }

    /**
     * Optional - OpenAPI type conversion.  This is used to map OpenAPI types in a `Schema` into
     * either language specific types via `typeMapping` or into complex models if there is not a mapping.
     *
     * @return a string value of the type or complex model for this property
     */
    @Override
    @SuppressWarnings("rawtypes")
    public String getSchemaType(Schema p) {
        String openAPIType = super.getSchemaType(p);

        String type = null;
        if (typeMapping.containsKey(openAPIType)) {
            type = typeMapping.get(openAPIType);
            if (languageSpecificPrimitives.contains(type)) {
                return toModelName(type);
            }
            if (foundationClasses.contains(type)) {
                return type;
            }
        } else {
            type = openAPIType;
        }
        return toModelName(type);
    }

    @Override
    public String toVarName(String name) {
        // sanitize name
        String varName = name;
        varName = sanitizeName(name);

        // if it's all upper case, convert to lower case
        if (varName.matches("^[A-Z_]*$")) {
            varName = varName.toLowerCase(Locale.ROOT);
        }

        // camelize (lower first character) the variable name
        varName = org.openapitools.codegen.utils.StringUtils.camelize(varName, LOWERCASE_FIRST_LETTER);

        // for reserved word or word starting with number, append _
        if (isReservedWord(varName) || varName.matches("^\\d.*")) {
            varName = escapeReservedWord(varName);
        }

        return varName;
    }

    @Override
    public String toParamName(String name) {
        return toVarName(name);
    }

    @Override
    public String getTypeDeclaration(String str) {
        return str;
    }

    @Override
    protected boolean needToImport(String type) {
        return StringUtils.isNotBlank(type) && !defaultIncludes.contains(type)
                && !nonFrameworkPrimitives.contains(type);
    }

    @Override
    public OperationsMap postProcessOperationsWithModels(OperationsMap objs, List<ModelMap> allModels) {
        OperationMap objectMap = objs.getOperations();
        List<CodegenOperation> operations = objectMap.getOperation();

        List<Map<String, String>> imports = objs.getImports();
        Map<String, CodegenModel> codegenModels = new HashMap<>();

        for (ModelMap moObj : allModels) {
            CodegenModel mo = moObj.getModel();
            if (mo.isEnum) {
                codegenModels.put(mo.classname, mo);
            }
        }
        for (CodegenOperation operation : operations) {
            if (operation.returnType != null) {
                if (codegenModels.containsKey(operation.returnType)) {
                    operation.vendorExtensions.put("x-returns-enum", true);
                }
            }
            // Check all return parameter baseType if there is a necessity to include, include it if not
            // already done
            if (operation.returnBaseType != null && needToImport(operation.returnBaseType)) {
                if (!isIncluded(operation.returnBaseType, imports)) {
                    imports.add(createMapping("import", operation.returnBaseType));
                }
            }
            List<CodegenParameter> params = new ArrayList<>();
            if (operation.allParams != null) params.addAll(operation.allParams);

            // Check all parameter baseType if there is a necessity to include, include it if not
            // already done
            for (CodegenParameter param : params) {
                if (param.isPrimitiveType && needToImport(param.baseType)) {
                    if (!isIncluded(param.baseType, imports)) {
                        imports.add(createMapping("import", param.baseType));
                    }
                }
            }
            if (operation.pathParams != null) {
                // We use QString to pass path params, add it to include
                if (!isIncluded("QString", imports)) {
                    imports.add(createMapping("import", "QString"));
                }
                // Look for unsupported path parameter styles.
                final Set<String> unsupportedPathStyles = new HashSet<>(
                        Arrays.asList("form", "spaceDelimited", "pipeDelimited", "deepObject"));
                for (CodegenParameter param : operation.pathParams) {
                    if (param.style != null) {
                        final String paramStyle = param.style;
                        if (unsupportedPathStyles.contains(paramStyle)) {
                            // Invalid style.
                            final String msg = String.format("'%s' style is invalid for path "
                                                             + "parameters.%nAllowed styles are: "
                                                             + "'matrix', 'label' and 'simple'.%n"
                                                             + "Falling back to the default style "
                                                             + "'simple'.", paramStyle);
                            LOGGER.warn("{}: {}", operation.operationId, msg);
                            param.vendorExtensions.put("x-warningMessage",
                                                       msg.replaceAll("\\n", "\\\\n")
                                                          .replaceAll("\\r", "\\\\r"));
                            param.style = "simple";
                        }
                    }
                }
            }

            // Look for unsupported query parameter styles or invalid style/explode combinations.
            if (operation.queryParams != null) {
                final Set<String> unsupportedQueryStyles = new HashSet<>(
                        Arrays.asList("matrix", "label", "simple"));
                for (CodegenParameter param : operation.queryParams) {
                    if (param.style != null) {
                        final String paramStyle = param.style;
                        String msg = null;
                        if (unsupportedQueryStyles.contains(paramStyle)) {
                            // Invalid style.
                            msg = String.format("'%s' style is invalid for query parameters.%n"
                                                + "Allowed styles are: 'form', 'spaceDelimited',"
                                                + " 'pipeDelimited' and 'deepObject'.%nFalling "
                                                + "back to the default style 'form'.",
                                                paramStyle);
                            param.style = "form";
                        } else if (paramStyle.equals("deepObject") && !param.isModel
                                   && !param.isFreeFormObject && !param.isAnyType) {
                            // Valid query style, invalid for non-object types.
                            msg = "'deepObject' style is only valid for parameters of type "
                                  + "'object'.\nFalling back to the default style: 'form'.";
                            param.style = "form";
                        } else if (param.isPrimitiveType && (paramStyle.equals("pipeDelimited")
                                   || paramStyle.equals("spaceDelimited"))) {
                            // Valid query style, invalid for primitive types.
                            msg = String.format("'%s' style is invalid for primitive parameters."
                                                + "%nFalling back to the default style: 'form'.",
                                                paramStyle);
                            param.style = "form";
                        } else if ((paramStyle.equals("deepObject") && !param.isExplode)
                                   || ((paramStyle.equals("pipeDelimited")
                                   || paramStyle.equals("spaceDelimited")) && param.isExplode)) {
                            // Invalid style/explode combinations.
                            msg = String.format("Invalid combination for query parameter '%s': "
                                                + "style=%s, explode=%b.%nUsing "
                                                + "valid explode=%b instead.",
                                                param.paramName, paramStyle,
                                                param.isExplode, !param.isExplode);
                            param.isExplode = !param.isExplode;
                        }
                        if (msg != null) {
                            LOGGER.warn("{}: {}", operation.operationId, msg);
                            param.vendorExtensions.put("x-warningMessage",
                                                       msg.replaceAll("\\n", "\\\\n")
                                                          .replaceAll("\\r", "\\\\r"));
                        }
                    }
                }
            }

            // Look for unsupported styles for header parameters
            if (operation.headerParams != null) {
                // header parameters can only use style=="simple"
                final String expectedStyle = "simple";
                for (CodegenParameter param : operation.headerParams) {
                    if (param.style != null && !param.style.equals(expectedStyle)) {
                        String msg = String.format("'%s' style is invalid for header parameters.%n"
                                                   + "Falling back to the default style '%s'.",
                                                   param.style, expectedStyle);
                        param.style = expectedStyle;
                        param.vendorExtensions.put("x-warningMessage",
                                                   msg.replaceAll("\\n", "\\\\n")
                                                      .replaceAll("\\r", "\\\\r"));
                        LOGGER.warn("{}: {}", operation.operationId, msg);
                    }
                }
            }
            // Look for unsupported styles for cookie parameters
            if (operation.cookieParams != null) {
                // cookie parameters can only use style=="form"
                final String expectedStyle = "form";
                for (CodegenParameter param : operation.cookieParams) {
                    if (param.style != null && !param.style.equals(expectedStyle)) {
                        String msg = String.format("'%s' style is invalid for cookie parameters.%n"
                                                   + "Falling back to the default style '%s'.",
                                                   param.style, expectedStyle);
                        param.style = expectedStyle;
                        param.vendorExtensions.put("x-warningMessage",
                                                   msg.replaceAll("\\n", "\\\\n")
                                                      .replaceAll("\\r", "\\\\r"));
                        LOGGER.warn("{}: {}", operation.operationId, msg);
                    }
                }
            }
        }
        if (isIncluded("QMap", imports)) {
            // Maps uses QString as key
            if (!isIncluded("QString", imports)) {
                imports.add(createMapping("import", "QString"));
            }
        }
        return objs;
    }

    @Override
    public boolean isDataTypeString(String dataType) {
        return "QString".equals(dataType);
    }

    private Map<String, String> createMapping(String key, String value) {
        Map<String, String> customImport = new HashMap<>();
        customImport.put(key, toModelImport(value));
        return customImport;
    }

    private boolean isIncluded(String type, List<Map<String, String>> imports) {
        boolean included = false;
        String inclStr = toModelImport(type);
        for (Map<String, String> importItem : imports) {
            if (importItem.containsValue(inclStr)) {
                included = true;
                break;
            }
        }
        return included;
    }

    public void setContentCompressionEnabled(boolean flag) {
        this.isContentCompressionEnabled = flag;
    }
}
