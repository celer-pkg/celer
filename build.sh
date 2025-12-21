#!/bin/bash

export GOPROXY=https://goproxy.io,direct

VERSION=$(git describe --tags --abbrev=0 2>/dev/null || echo "v0.0.0")
go build -trimpath -ldflags "-s -w -X celer/configs.Version=${VERSION}"

upx=$(which upx)

if [[ -x "$upx" ]]; then
    if [[ "${HOME}" == /c/Users/* ]]; then
        "$upx" --best celer.exe
    else
        "$upx" --best celer
    fi
fi