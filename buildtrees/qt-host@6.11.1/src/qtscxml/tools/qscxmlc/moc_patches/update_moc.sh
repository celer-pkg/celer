#!/bin/bash
# Copyright (C) 2021 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

usage() {
    cat <<EOF >&2

Usage: update_moc [gu] [moc directory]

The moc directory should be qtbase's moc source directory
(absolute or relative). The script must be run from the directory
containing the moc files (moc.h et. al).

The script has two modes:

'g' Generates updated patches of the current qtscxml-specific moc
    changes. Use this when the qtscxml-specific parts in moc files
    have changed and the patches should be updated accordingly.

'u' Updates the moc from the upstream qtbase moc.
    This mode gets the applicable qtbase moc files from the provided
    directory, and updates their qtscxml copies by applying patches to them.
    If you have not modified qtscxml moc code and just want to update the moc
    code from upstream, this is the one you should use.

Examples:
moc_patches/update_moc g ../../../qtbase/src/tools/moc
moc_patches/update_moc u ../../../qtbase/src/tools/moc

EOF
    die "$@"
}

checkFile () {
    for f
    do [[ -f "$f" ]] || die "Error: file \"$f\" does not exist."
    done
}

warn () { echo "$@" >&2; }
die () {
    [[ -h .upstream ]] && rm .upstream
    warn "$@"
    exit 1
}

generate_patch() {
    echo Generating patches recording how qscxmlc moc differs from upstream.

    # Link the upstream moc files to create a patch file with filepaths
    # that are independent of the actual used upstream moc location.
    ln -s "$MOC_DIR" .upstream

    for file in "${FILES[@]}"
    do
        checkFile "$file" ".upstream/$file"
        diff -u ".upstream/$file" "$file" > "moc_patches/$file.patch"
        echo Generated "moc_patches/$file.patch"
    done
    # tidy up
    rm .upstream
}

update_moc() {
    echo Updating qscxmlc moc from upstream by applying saved patches.

    for file in "${FILES[@]}"
    do
        checkFile "moc_patches/$file.patch" "$MOC_DIR/$file"
        echo Patching file: "$file" with "moc_patches/$file.patch"
        # overwrite the current file from upstream
        cp "$MOC_DIR/$file" "$file"
        if patch "$file" "moc_patches/$file.patch"
        then echo Patched "$file"
        else warn "Please hand-patch $file; see $file.orig and $file.rej and tidy them away when you are done."
        fi
    done
}

MODE="$1"
MOC_DIR="$2"
FILES=( "outputrevision.h" "moc.cpp" "moc.h" "generator.h" "generator.cpp" )

[[ -f moc_patches/update_moc.sh ]] || usage "Error: script must be run from the tools/qscxmlc/ directory."
[[ -n "$MOC_DIR" ]] || usage "Error: You did not specify a moc directory."
[[ -d "$MOC_DIR" ]] || usage "Error: moc directory \"$MOC_DIR\" does not exist."

case "$MODE" in
    g) generate_patch ;;
    u) update_moc ;;
    *) usage "Error: mode \"$MODE\" is not recognized." ;;
esac

echo Done
