// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef TOOLS_GN_RSP_TARGET_WRITER_H_
#define TOOLS_GN_RSP_TARGET_WRITER_H_

#include <iosfwd>
#include <memory>
#include "gn/path_output.h"

class Target;
class NinjaCBinaryTargetWriter;
class ResolvedTargetData;

class RspTargetWriter {
 public:
  enum Type { NONE, OBJECTS, ARCHIVES, DEFINES, LFLAGS, LIBS, LDIR};
  RspTargetWriter(const NinjaCBinaryTargetWriter* writer,
                  const Target* target,
                  Type type,
                  std::ostream& out);
  ~RspTargetWriter();
  void Run();
  static Type strToType(const std::string& str);
  const ResolvedTargetData& resolved() const;
  void set_lflags_remove_pattern(const std::string& str) {
    lflags_remove_pattern_ = str;
  }

 private:
  Type type_;
  const Target* target_;
  const NinjaCBinaryTargetWriter* nwriter_;
  std::string lflags_remove_pattern_;
  std::ostream& out_;
  mutable std::unique_ptr<ResolvedTargetData> resolved_;
};

#endif  // TOOLS_GN_RSP_TARGET_WRITER_H_
