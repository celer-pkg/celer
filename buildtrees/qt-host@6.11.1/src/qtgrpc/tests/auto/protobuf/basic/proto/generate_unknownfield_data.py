# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

# This script is run to generate test data for the unknown field tests,
# see tst_protobuf_unknown_field.cpp

# NOTE: "protoc" must be installed and in path, this script takes care of
# invoking the tool to generate the generated python output.
# ALSO NOTE: paths assume it is run from the directory in which it is stored.

import os, os.path
import shutil
assert os.path.exists('unknownfield.proto'), \
    "I need to be run from the directory that unknownfield.proto lives in"
# Generates unknownfield_pb2...
os.system('protoc --python_out=. --proto_path=. ./unknownfield.proto')
# ... which we subsequently include:
import unknownfield_pb2

msg = unknownfield_pb2.StringMessage()
msg.aaa = 2
msg.timestamp = 42
msg.stringField = "Hello World"

print(msg.SerializeToString().hex())

msg = unknownfield_pb2.LargeIndexStringMessage()
msg.aaa = 2
msg.timestamp = 42
msg.stringField = "Hello World"

print(msg.SerializeToString().hex())

msg = unknownfield_pb2.IntMessage()
msg.aaa = 2
msg.timestamp = 42
msg.intField = 242

print(msg.SerializeToString().hex())


msg = unknownfield_pb2.MapMessage()
msg.aaa = 2
msg.timestamp = 42
msg.mapField[1] = 2
msg.mapField[2] = 4
msg.mapField[3] = 6

print(msg.SerializeToString().hex())


msg = unknownfield_pb2.RepeatedMessage()
msg.aaa = 2
msg.timestamp = 42
msg.repeatedField.extend([1, 1, 2, 3, 5, -5, -3, -2, -1, -1])

print(msg.SerializeToString().hex())

os.remove("unknownfield_pb2.py")
shutil.rmtree("__pycache__")
