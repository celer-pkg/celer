// Copyright 2024 The ChromiumOS Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_SCOPED_FD_
#define BASE_SCOPED_FD_

namespace libvafake::base {

// ScopedFD is a RAII class that owns a file descriptor and, if it's valid, it
// close()s it when one of the following happens:
//
// - The ScopedFD is destroyed.
//
// - One of the move operations (constructor or assignment) is invoked. In this
//   case, the FD tracked by the destination ScopedFD is close()d if valid, and
//   the source ScopedFD is left tracking an invalid FD.
class ScopedFD {
 public:
  explicit ScopedFD(int fd);
  ScopedFD(ScopedFD&& other);
  ScopedFD& operator=(ScopedFD&& other);
  ~ScopedFD();

  int get() const;

 private:
  int scoped_fd_ = -1;
};

}  // namespace libvafake::base

#endif  // BASE_SCOPED_FD_