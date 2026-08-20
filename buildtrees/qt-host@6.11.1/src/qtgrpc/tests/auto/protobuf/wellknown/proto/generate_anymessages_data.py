#!/usr/bin/env python3
# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

# This script is run to generate test data for the Any message tests,
# transcribed to suitable parts of tst_protobuf_any.cpp

# NOTE: "protoc" must be installed and in path, this script takes care of
# invoking the tool to generate the generated python output.
# ALSO NOTE: paths assume it is run from the directory in which it is stored.

import os, os.path
import shutil

proto_path = '../../../shared/data/proto/anymessages.proto'
assert os.path.exists(proto_path), \
    "I need to be run from the directory that generate_anymessages_data.py lives in"
# Generates anymessages_pb2...
os.system(f'protoc --python_out=. --proto_path={os.path.dirname(proto_path)} {proto_path}')
# ... which we subsequently import:
import anymessages_pb2

msg = anymessages_pb2.AnyMessage()
print("empty")
print(msg.SerializeToString().hex())

ex = anymessages_pb2.Example()
msg = anymessages_pb2.AnyMessage()
msg.field.Pack(ex)
print("defaulted 'Example' message")
print(msg.SerializeToString().hex())

ex = anymessages_pb2.Example()
ex.str = "Hello"
ex.i = 1
ex.j = 2
ex.h = 3
ex.str2 = "World!"

msg = anymessages_pb2.AnyMessage()
msg.field.Pack(ex)
print("hello 1 2 3 world:")
print(msg.SerializeToString().hex())

ex = anymessages_pb2.Example()
ex.str = "No"
ex.str2 = "Numbers!"

msg = anymessages_pb2.AnyMessage()
msg.field.Pack(ex)
print("no 0 0 0 numbers")
print(msg.SerializeToString().hex())

ex = anymessages_pb2.Example()
ex.str = "Hello"
ex.str2 = "World!"

anymsg = anymessages_pb2.google_dot_protobuf_dot_any__pb2.Any()
anymsg.Pack(ex)

msg = anymessages_pb2.RepeatedAnyMessage()
msg.anys.append(anymsg)
msg.anys.append(anymsg)
msg.anys.append(anymsg)
print("[Hello 0 0 0 World] * 3")
print(msg.SerializeToString().hex())

msg = anymessages_pb2.TwoAnyMessage()
msg.two.Pack(ex)
print("[Empty] and [Hello 0 0 0 World!]")
print(msg.SerializeToString().hex())

msg = anymessages_pb2.TwoAnyMessage()
anymsg = anymessages_pb2.google_dot_protobuf_dot_any__pb2.Any()
msg.one.Pack(anymsg)
msg.two.Pack(ex)
print("[Nested empty] and [Hello 0 0 0 World!]")
print(msg.SerializeToString().hex())

os.remove("anymessages_pb2.py")
shutil.rmtree("__pycache__")
