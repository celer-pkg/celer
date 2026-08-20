load("@com_google_protobuf//bazel:java_lite_proto_library.bzl", "java_lite_proto_library")
load("@com_google_protobuf//bazel:java_proto_library.bzl", "java_proto_library")
load("@com_google_protobuf//bazel:proto_library.bzl", "proto_library")

# Public protos for Anonymous Tokens service.
load("@rules_cc//cc:defs.bzl", "cc_library")

package(default_visibility = ["//:__subpackages__"])

proto_library(
    name = "anonymous_tokens_proto",
    srcs = ["anonymous_tokens.proto"],
)

cc_proto_library(
    name = "anonymous_tokens_cc_proto",
    deps = [":anonymous_tokens_proto"],
)

java_proto_library(
    name = "anonymous_tokens_java_proto",
    deps = [":anonymous_tokens_proto"],
)

java_lite_proto_library(
    name = "anonymous_tokens_java_proto_lite",
    deps = [":anonymous_tokens_proto"],
)
