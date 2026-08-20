// Copyright 2024 The ChromiumOS Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "scoped_fd.h"

#include <unistd.h>

namespace libvafake::base {

ScopedFD::ScopedFD(int fd) : scoped_fd_(fd) {}

ScopedFD::ScopedFD(ScopedFD&& other) {
  if (scoped_fd_ >= 0) {
    close(scoped_fd_);
  }
  scoped_fd_ = other.scoped_fd_;
  other.scoped_fd_ = -1;
}

ScopedFD& ScopedFD::operator=(ScopedFD&& other) {
  if (scoped_fd_ >= 0) {
    close(scoped_fd_);
  }

  scoped_fd_ = other.scoped_fd_;
  other.scoped_fd_ = -1;
  return *this;
}

ScopedFD::~ScopedFD() {
  if (scoped_fd_ >= 0) {
    close(scoped_fd_);
  }
}

int ScopedFD::get() const {
  return scoped_fd_;
}

}  // namespace libvafake::base