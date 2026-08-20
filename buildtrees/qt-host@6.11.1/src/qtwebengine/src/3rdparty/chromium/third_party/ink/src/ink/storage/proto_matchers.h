// Copyright 2024-2025 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef INK_STORAGE_PROTO_MATCHERS_H_
#define INK_STORAGE_PROTO_MATCHERS_H_

#include "gtest/gtest.h"
#include "google/protobuf/message.h"

namespace ink {

// Simple proto matchers using MessageDifferencer.
//
// The default message comparison mode is EQUALS:
// https://protobuf.dev/reference/cpp/api-docs/google.protobuf.util.message_differencer/#MessageDifferencer.MessageFieldComparison.details
//
// This requires fields to be set in both messages to be considered equal.
// Default values are not considered.  If needed, add additional matchers for
// EQUIVALENT, which will consider default values as set.
//
// Replace with official proto matchers when they are open source:
// github.com/google/googletest/issues/1761.
testing::Matcher<google::protobuf::Message> EqualsProto(const google::protobuf::Message& expected);
testing::Matcher<google::protobuf::Message> EqualsProtoTreatingNaNsAsEqual(
    const google::protobuf::Message& expected);

}  // namespace ink

#endif  // INK_STORAGE_PROTO_MATCHERS_H_
