// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: Apache-2.0

package org.qtproject.qt.codegen;

import org.junit.jupiter.api.Test;
import org.openapitools.codegen.ClientOptInput;
import org.openapitools.codegen.DefaultGenerator;
import org.openapitools.codegen.config.CodegenConfigurator;
import org.qtproject.qt.codegen.CppQt6ClientGenerator;

/***
 * This test allows you to easily launch your code generation software under a debugger.
 * Then run this test under debug mode.  You will be able to step through your java code
 * and then see the results in the out directory.
 *
 * To experiment with debugging your code generator:
 * 1) Set a break point in CppQt6ClientGenerator.java in the postProcessOperationsWithModels() method.
 * 2) To launch this test in Eclipse: right-click | Debug As | JUnit Test
 *
 */
public class CppQt6ClientGeneratorTest {

  // use this test to launch you code generator in the debugger.
  // this allows you to easily set break points in CppQt6ClientGenerator.
  @Test
  public void launchCodeGenerator() {
    // to understand how the 'openapi-generator-cli' module is using 'CodegenConfigurator', have a look at the 'Generate' class:
    // https://github.com/OpenAPITools/openapi-generator/blob/master/modules/openapi-generator-cli/src/main/java/org/openapitools/codegen/cmd/Generate.java
    final CodegenConfigurator configurator = new CodegenConfigurator()
              .setGeneratorName("cpp-qt6-client") // use this codegen library
              .setInputSpec("yaml_files/petstore.yaml") // petstore test yaml file
              .setOutputDir("out/cpp-qt6-client"); // output directory

    final ClientOptInput clientOptInput = configurator.toClientOptInput();
    DefaultGenerator generator = new DefaultGenerator();
    generator.opts(clientOptInput).generate();
  }
}
