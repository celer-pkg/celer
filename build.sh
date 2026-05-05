#!/bin/bash

export GOPROXY=https://goproxy.io,direct

# Get the latest tag sorted by version (semver)
# This works even when HEAD is not on a tag
VERSION=$(git tag --sort=-version:refname --merged 2>/dev/null | head -1)
VERSION=${VERSION:-v0.0.0}

echo "Building celer with VERSION=$VERSION"
go build -trimpath -ldflags "-s -w -X github.com/celer-pkg/celer/configs.Version=${VERSION}"

upx=$(which upx)

if [[ -x "$upx" ]]; then
    if [[ "${HOME}" == /c/Users/* ]]; then
        "$upx" --best celer.exe
    else
        "$upx" --best celer
    fi
fi