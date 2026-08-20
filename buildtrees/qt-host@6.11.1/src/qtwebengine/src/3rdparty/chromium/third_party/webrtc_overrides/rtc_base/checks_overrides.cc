// Copyright 2022 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <string_view>

#include "base/logging.h"
#include "base/logging/log_severity.h"
#include "third_party/webrtc/rtc_base/checks.h"

namespace webrtc::webrtc_checks_impl {

RTC_NORETURN void WriteFatalLog(std::string_view output) {
  LOG(FATAL) << output;
#if !defined(_MSC_VER) || defined(__clang__)
  __builtin_unreachable();
#else
  __assume(0);
#endif
}

RTC_NORETURN void WriteFatalLog(const char* file,
                                int line,
                                std::string_view output) {
  {
    logging::LogMessage msg(file, line, logging::LOGGING_FATAL);
    msg.stream() << output;
  }
#if !defined(_MSC_VER) || defined(__clang__)
  __builtin_unreachable();
#else
  __assume(0);
#endif
}

}  // namespace webrtc::webrtc_checks_impl
