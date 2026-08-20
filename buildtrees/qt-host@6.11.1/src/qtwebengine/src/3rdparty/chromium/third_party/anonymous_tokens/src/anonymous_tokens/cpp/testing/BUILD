package(default_visibility = ["//:__subpackages__"])

cc_library(
    name = "proto_utils",
    testonly = 1,
    srcs = ["proto_utils.cc"],
    hdrs = [
        "proto_utils.h",
        "testdata_utils.h",
    ],
    data = ["//anonymous_tokens/testdata:strong_rsa_example_keys"],
    deps = [
        ":utils",
        "//anonymous_tokens/cpp/crypto:constants",
        "//anonymous_tokens/cpp/crypto:crypto_utils",
        "//anonymous_tokens/cpp/shared:status_utils",
        "//anonymous_tokens/proto:anonymous_tokens_cc_proto",
        "@boringssl//:ssl",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/status:statusor",
        "@com_google_absl//absl/strings",
    ],
)

cc_library(
    name = "utils",
    testonly = 1,
    srcs = ["utils.cc"],
    hdrs = ["utils.h"],
    deps = [
        "//anonymous_tokens/cpp/crypto:constants",
        "//anonymous_tokens/cpp/crypto:crypto_utils",
        "//anonymous_tokens/cpp/shared:status_utils",
        "@boringssl//:ssl",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/status:statusor",
        "@com_google_absl//absl/strings",
    ],
)
