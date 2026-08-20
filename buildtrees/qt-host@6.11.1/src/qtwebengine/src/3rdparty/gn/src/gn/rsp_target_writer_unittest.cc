// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "gn/rsp_target_writer.h"
#include <iostream>
#include "gn/ninja_c_binary_target_writer.h"
#include "gn/test_with_scheduler.h"
#include "gn/test_with_scope.h"
#include "util/test/test.h"

using RspTargetWriterTest = TestWithScheduler;

TEST_F(RspTargetWriterTest, WriteRspInfo) {
  TestWithScope setup;
  Err err;

  setup.build_settings()->SetBuildDir(SourceDir("//out/Debug/"));

  Target source_set_target(setup.settings(),
                           Label(SourceDir("//foo1/"), "foo1"));
  source_set_target.set_output_type(Target::SOURCE_SET);
  source_set_target.visibility().SetPublic();
  source_set_target.sources().push_back(SourceFile("//foo1/input1.cc"));
  source_set_target.sources().push_back(SourceFile("//foo1/input2.cc"));
  source_set_target.SetToolchain(setup.toolchain());
  ASSERT_TRUE(source_set_target.OnResolved(&err));

  TestTarget static_lib_target1(setup, "//foo5:bar", Target::STATIC_LIBRARY);
  static_lib_target1.sources().push_back(SourceFile("//foo5/input1.cc"));
  static_lib_target1.config_values().arflags().push_back("--bar");
  static_lib_target1.set_complete_static_lib(true);
  ASSERT_TRUE(static_lib_target1.OnResolved(&err));

  TestTarget static_lib_target2(setup, "//foo6:bar", Target::STATIC_LIBRARY);
  static_lib_target2.sources().push_back(SourceFile("//foo6/input1.cc"));
  static_lib_target2.config_values().arflags().push_back("--bar");
  static_lib_target2.set_complete_static_lib(true);
  ASSERT_TRUE(static_lib_target2.OnResolved(&err));

  Target shared_lib_target(setup.settings(),
                           Label(SourceDir("//foo2/"), "foo2"));

  shared_lib_target.rsp_types().push_back("NOTUSED");
  shared_lib_target.set_output_type(Target::SHARED_LIBRARY);
  shared_lib_target.set_output_extension(std::string("so.1"));
  shared_lib_target.set_output_dir(SourceDir("//out/Debug/foo/"));
  shared_lib_target.sources().push_back(SourceFile("//foo2/input1.cc"));
  shared_lib_target.sources().push_back(SourceFile("//foo2/input 2.cc"));
  shared_lib_target.sources().push_back(SourceFile("//foo 2/input 3.cc"));
  shared_lib_target.config_values().libs().push_back(
      LibFile(SourceFile("//foo/libfoo3.a")));
  shared_lib_target.config_values().libs().push_back(LibFile("foo4"));
  shared_lib_target.config_values().lib_dirs().push_back(
      SourceDir("//foo/bar/"));
  shared_lib_target.public_deps().push_back(
      LabelTargetPair(&source_set_target));
  shared_lib_target.public_deps().push_back(
      LabelTargetPair(&static_lib_target1));
  shared_lib_target.public_deps().push_back(
      LabelTargetPair(&static_lib_target2));
  shared_lib_target.config_values().ldflags().push_back("-fooBAR");
  shared_lib_target.config_values().ldflags().push_back(
      "--sysroot=/foo/../bar");
  shared_lib_target.config_values().ldflags().push_back("/INCREMENTAL:NO");
  shared_lib_target.SetToolchain(setup.toolchain());
  ASSERT_TRUE(shared_lib_target.OnResolved(&err));

  std::ostringstream ninja_out;
  NinjaCBinaryTargetWriter writer(&shared_lib_target, ninja_out);
  writer.Run();

  const char ninja_expected[] =
      "defines =\n"
      "include_dirs =\n"
      "root_out_dir = .\n"
      "target_out_dir = obj/foo2\n"
      "target_output_name = libfoo2\n"
      "\n"
      "build obj/foo2/libfoo2.input1.o: cxx ../../foo2/input1.cc\n"
      "build obj/foo2/libfoo2.input$ 2.o: cxx ../../foo2/input$ 2.cc\n"
      "build obj/foo$ 2/libfoo2.input$ 3.o: cxx ../../foo$ 2/input$ 3.cc\n"
      "\n"
      "build foo2.stamp: stamp | obj/foo2/libfoo2.input1.o "
      "obj/foo2/libfoo2.input$ 2.o"
      " obj/foo$ 2/libfoo2.input$ 3.o"
      " obj/foo1/foo1.input1.o obj/foo1/foo1.input2.o obj/foo5/libbar.a "
      "obj/foo6/libbar.a"
      " ../../foo/libfoo3.a || obj/foo1/foo1.stamp\n"
      "  ldflags = -fooBAR /INCREMENTAL:NO -L../../foo/bar\n"
      "  libs = ../../foo/libfoo3.a -lfoo4\n"
      "  frameworks =\n"
      "  swiftmodules =\n"
      "  output_extension = .so.1\n"
      "  output_dir = foo\n";

  std::ostringstream objects_out;
  RspTargetWriter rsp_object_writer(&writer, &shared_lib_target,
                                    RspTargetWriter::OBJECTS, objects_out);
  rsp_object_writer.Run();

  const char objects_expected[] =
      "\"//out/Debug/obj/foo2/libfoo2.input1.o\"\n"
      "\"//out/Debug/obj/foo2/libfoo2.input 2.o\"\n"
      "\"//out/Debug/obj/foo 2/libfoo2.input 3.o\"\n"
      "\"//out/Debug/obj/foo1/foo1.input1.o\"\n"
      "\"//out/Debug/obj/foo1/foo1.input2.o\"\n";

  EXPECT_EQ(objects_expected, objects_out.str());

  std::ostringstream lflags_out;
  RspTargetWriter rsp_lflags_writer(&writer, &shared_lib_target,
                                    RspTargetWriter::LFLAGS, lflags_out);
  rsp_lflags_writer.set_lflags_remove_pattern("(--sysroot=)(/\\S*?\\s)");
  rsp_lflags_writer.Run();
  const char lflags_expected[] = " -fooBAR /INCREMENTAL:NO";

  EXPECT_EQ(lflags_expected, lflags_out.str());

  std::ostringstream ldir_out;
  RspTargetWriter rsp_ldir_writer(&writer, &shared_lib_target,
                                    RspTargetWriter::LDIR, ldir_out);
  rsp_ldir_writer.Run();

  const char ldir_expected[] = " -L../../foo/bar";

  EXPECT_EQ(ldir_expected, ldir_out.str());

  std::ostringstream archives_out;
  RspTargetWriter rsp_archive_writer(&writer, &shared_lib_target,
                                     RspTargetWriter::ARCHIVES, archives_out);
  rsp_archive_writer.Run();

  const char archives_expected[] =
      "\"//out/Debug/obj/foo5/libbar.a\"\n"
      "\"//out/Debug/obj/foo6/libbar.a\"\n";
  EXPECT_EQ(archives_expected, archives_out.str());

  std::ostringstream libs_out;
  RspTargetWriter rsp_libs_writer(&writer, &shared_lib_target,
                                  RspTargetWriter::LIBS, libs_out);
  rsp_libs_writer.Run();

  const char libs_expected[] = "\"../../foo/libfoo3.a\" -lfoo4";
  EXPECT_EQ(libs_expected, libs_out.str());
}
