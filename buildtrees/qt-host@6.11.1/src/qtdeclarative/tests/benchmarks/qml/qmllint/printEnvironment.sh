#!/bin/bash
# Example usage: sh printEnvironment.sh <path/to/QtDesignStudio> <path/to/projects/folder>

# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

qdsInstallation=~/Qt/QtDesignStudio/
[ "$1" ] && qtInstallation="$1"

projectFolder=~/projects
[ "$2" ] && projectFolder="$2"

function qdsImportPath {
    shopt -s globstar
    echo "${qdsInstallation}"/**/qt6_design_studio_reduced_version/qml
}

importPath=$(qdsImportPath)


function printEnvironmentForRecommendedProjects {
    echo TST_QMLLINT_BENCHMARK_IMPORT_PATH="${importPath}"
    echo TST_QMLLINT_BENCHMARK_PROJECTS="${projectFolder}/qtdesign-studio/examples/OutrunHVAC:${projectFolder}/qtdesign-studio/examples/MaterialBundleDemo:${projectFolder}/qtdesign-studio/examples/EosADAS:${projectFolder}/responsive-scada"
    echo TST_QMLLINT_BENCHMARK_IMPORT_PATH_HEURISTICS=1
}

function printEnvironmentForAllProjects {
    echo TST_QMLLINT_BENCHMARK_IMPORT_PATH="${importPath}"
    echo TST_QMLLINT_BENCHMARK_PROJECTS="${projectFolder}/qtdesign-studio/examples/:${projectFolder}/responsive-scada"
    echo TST_QMLLINT_BENCHMARK_IMPORT_PATH_HEURISTICS=1
}


echo To benchmark a selection of projects, use:
printEnvironmentForRecommendedProjects

echo To benchmark all projects, use:
printEnvironmentForAllProjects
