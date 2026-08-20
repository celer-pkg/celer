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

#include "ink/storage/proto_matchers.h"

#include <string>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "google/protobuf/message.h"
#include "google/protobuf/util/field_comparator.h"
#include "google/protobuf/util/message_differencer.h"

namespace ink {
namespace {

using ::google::protobuf::Message;
using ::google::protobuf::util::DefaultFieldComparator;
using ::google::protobuf::util::MessageDifferencer;
using ::testing::Matcher;

MATCHER_P2(ProtoMatcher, expected, nans_are_equal, "") {
  MessageDifferencer proto_differ;
  std::string proto_diff;
  proto_differ.ReportDifferencesToString(&proto_diff);
  DefaultFieldComparator field_comparator;
  if (nans_are_equal) {
    field_comparator.set_treat_nan_as_equal(true);
    proto_differ.set_field_comparator(&field_comparator);
  }
  bool result = proto_differ.Compare(arg, *expected);
  *result_listener << "protos differ: " << proto_diff;
  return result;
}

}  // namespace

Matcher<Message> EqualsProto(const Message& expected) {
  return ProtoMatcher(&expected, false);
}

Matcher<Message> EqualsProtoTreatingNaNsAsEqual(const Message& expected) {
  return ProtoMatcher(&expected, true);
}

}  // namespace ink
