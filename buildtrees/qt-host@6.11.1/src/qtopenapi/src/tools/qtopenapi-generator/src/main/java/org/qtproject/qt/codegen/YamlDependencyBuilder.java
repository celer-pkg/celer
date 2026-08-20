// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: Apache-2.0
// Qt-Security score:critical reason:data-parser

package org.qtproject.qt.codegen;

import com.fasterxml.jackson.core.StreamReadConstraints;
import com.fasterxml.jackson.databind.JsonNode;
import com.fasterxml.jackson.databind.ObjectMapper;
import com.fasterxml.jackson.dataformat.yaml.YAMLFactory;

import java.io.File;
import java.io.IOException;

import java.util.*;

import org.yaml.snakeyaml.LoaderOptions;

public class YamlDependencyBuilder
{
    private YAMLFactory yamlFactory;
    private ObjectMapper objectMapper;

    YamlDependencyBuilder()
    {
        // Extract the value of maxYamlCodePoints, if set.
        int yamlCodePoints = 0;
        try {
            String yamlCodePointsStr = System.getProperty("maxYamlCodePoints");
            yamlCodePoints = Integer.parseInt(yamlCodePointsStr);
        } catch (NumberFormatException e) {
            yamlCodePoints = 0;
        }

        if (yamlCodePoints > 0) {
            LoaderOptions loaderOptions = new LoaderOptions();
            loaderOptions.setCodePointLimit(yamlCodePoints);
            final StreamReadConstraints readConstraints = StreamReadConstraints.builder()
                                                            .maxDocumentLength(yamlCodePoints)
                                                            .build();
            yamlFactory = YAMLFactory.builder()
                            .streamReadConstraints(readConstraints)
                            .loaderOptions(loaderOptions)
                            .build();
        } else {
            yamlFactory = YAMLFactory.builder().build();
        }
        objectMapper = new ObjectMapper(yamlFactory);
    }

    // Returns the set of filenames, *including* the provided one
    public Set<String> getDependencies(String mainYamlFile)
    {
        Set<String> results = new HashSet<>();

        try {
            final File mainFile = new File(mainYamlFile);
            final String mainFilePath = mainFile.getCanonicalPath();
            final String mainFileDir = new File(mainFilePath).getParent();

            LinkedList<String> pathsToProcess = new LinkedList<String>();
            pathsToProcess.add(mainFilePath); // add first file to the list

            while (!pathsToProcess.isEmpty()) {
                // extract the first element
                final String filename = pathsToProcess.poll();
                if (results.contains(filename))
                    continue;

                final Set<String> newFiles = readOneFile(filename, mainFileDir);
                if (newFiles != null) {
                    // add the processed file to the results
                    results.add(filename);
                    // schedule more dependencies to be processed
                    pathsToProcess.addAll(newFiles);
                } // else there was an exception - probably the file does not exist
            }
        } catch (Exception e) {
            // Something went wrong, simply return what we have so far
        }

        return results;
    }

    // Reads the file and returns new dependencies
    private Set<String> readOneFile(String filePath, String currentDir)
    {
        try {
            Set<String> files = new HashSet<>();
            JsonNode node = objectMapper.readTree(new File(filePath));
            collectDeps(node, files, currentDir);
            return files;
        } catch (IOException e) {
            // Just skip the file
            return null;
        }
    }

    private static void collectDeps(JsonNode node, Set<String> refs, String currentDir) {
        if (node == null)
            return;

        if (node.isObject()) {
            node.fields().forEachRemaining(entry -> {
                if ("$ref".equals(entry.getKey()) && entry.getValue().isTextual()) {
                    String filePath = extractFileName(entry.getValue().asText(), currentDir);
                    if (filePath != null && !filePath.isEmpty())
                        refs.add(filePath);
                }
                collectDeps(entry.getValue(), refs, currentDir);
            });
        } else if (node.isArray()) {
            node.forEach(child -> collectDeps(child, refs, currentDir));
        }
    }

    // See https://swagger.io/docs/specification/v3_0/using-ref/
    // for the possible examples of $ref's
    private static String extractFileName(String ref, String currentDir)
    {
        if (ref.startsWith("#")) // ref is within the document
            return null;

        // Drop the part after '#' - that's the local pointer within the file.
        // We do not need it here.
        final int hashIdx = ref.indexOf("#");
        String uri = (hashIdx >= 0) ? ref.substring(0, hashIdx) : ref;
        if (!uri.endsWith(".yaml")) // not a yaml file
            return null;

        if (uri.startsWith("file://")) {
            // Absolute local file path.
            return uri.substring(7); // skip "file://"
        } else if (uri.startsWith("//") || uri.contains("://")) {
            // Some other sort of schema, e.g.
            // http://some.server.org/schemas/v1/NameOfTheFile.yaml
            // or
            // //anotherserver.org/files/NameOfTheFile.yaml
            //
            // In all cases we need to extract NameOfTheFile.yaml,
            // that is - from the last '/' to the end.
            final int idx = uri.lastIndexOf("/");
            uri = uri.substring(idx + 1);
        }
        // If we're here, uri contains a relative path.
        // Attach the current directory to it.
        return currentDir + File.separator + uri;
    }
}
