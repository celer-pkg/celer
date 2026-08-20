@echo off

:: Copyright 2025 The Chromium Authors
:: Use of this source code is governed by a BSD-style license that can be
:: found in the LICENSE file.

if exist python3 (
    python3 "%~dp0protoc-gen-ts_proto.py" %*
) else (
    python "%~dp0protoc-gen-ts_proto.py" %*
)
