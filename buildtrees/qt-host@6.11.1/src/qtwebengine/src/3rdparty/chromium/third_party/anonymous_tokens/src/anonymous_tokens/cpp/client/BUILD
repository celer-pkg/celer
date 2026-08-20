package(default_visibility = ["//:__subpackages__"])

licenses(["notice"])

cc_library(
    name = "anonymous_tokens_public_key_client",
    srcs = ["anonymous_tokens_public_key_client.cc"],
    hdrs = ["anonymous_tokens_public_key_client.h"],
    deps = [
        "//anonymous_tokens/cpp/shared:proto_utils",
        "//anonymous_tokens/cpp/shared:status_utils",
        "//anonymous_tokens/proto:anonymous_tokens_cc_proto",
        "@com_google_absl//absl/container:flat_hash_set",
        "@com_google_absl//absl/status:statusor",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/time",
        "@com_google_absl//absl/types:optional",
    ],
)

cc_test(
    name = "anonymous_tokens_public_key_client_test",
    srcs = ["anonymous_tokens_public_key_client_test.cc"],
    deps = [
        ":anonymous_tokens_public_key_client",
        "//anonymous_tokens/cpp/shared:proto_utils",
        "//anonymous_tokens/cpp/shared:status_utils",
        "//anonymous_tokens/cpp/testing:utils",
        "//anonymous_tokens/proto:anonymous_tokens_cc_proto",
        "@com_github_google_googletest//:gtest_main",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/strings",
    ],
)

cc_library(
    name = "anonymous_tokens_rsa_bssa_client",
    srcs = ["anonymous_tokens_rsa_bssa_client.cc"],
    hdrs = ["anonymous_tokens_rsa_bssa_client.h"],
    deps = [
        "//anonymous_tokens/cpp/crypto:anonymous_tokens_pb_openssl_converters",
        "//anonymous_tokens/cpp/crypto:crypto_utils",
        "//anonymous_tokens/cpp/crypto:rsa_blinder",
        "//anonymous_tokens/cpp/shared:proto_utils",
        "//anonymous_tokens/cpp/shared:status_utils",
        "//anonymous_tokens/proto:anonymous_tokens_cc_proto",
        "@com_google_absl//absl/container:flat_hash_map",
        "@com_google_absl//absl/container:flat_hash_set",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/status:statusor",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/time",
        "@com_google_absl//absl/types:optional",
    ],
)

cc_test(
    name = "anonymous_tokens_rsa_bssa_client_test",
    srcs = ["anonymous_tokens_rsa_bssa_client_test.cc"],
    deps = [
        ":anonymous_tokens_rsa_bssa_client",
        "//anonymous_tokens/cpp/crypto:constants",
        "//anonymous_tokens/cpp/crypto:rsa_blind_signer",
        "//anonymous_tokens/cpp/shared:proto_utils",
        "//anonymous_tokens/cpp/shared:status_utils",
        "//anonymous_tokens/cpp/testing:proto_utils",
        "//anonymous_tokens/cpp/testing:utils",
        "//anonymous_tokens/proto:anonymous_tokens_cc_proto",
        "@boringssl//:ssl",
        "@com_github_google_googletest//:gtest_main",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/status:statusor",
        "@com_google_absl//absl/time",
    ],
)

cc_library(
    name = "anonymous_tokens_redemption_client",
    srcs = ["anonymous_tokens_redemption_client.cc"],
    hdrs = ["anonymous_tokens_redemption_client.h"],
    deps = [
        "//anonymous_tokens/cpp/crypto:constants",
        "//anonymous_tokens/proto:anonymous_tokens_cc_proto",
        "@com_google_absl//absl/container:flat_hash_map",
        "@com_google_absl//absl/container:flat_hash_set",
        "@com_google_absl//absl/status:statusor",
    ],
)

cc_test(
    name = "anonymous_tokens_redemption_client_test",
    srcs = ["anonymous_tokens_redemption_client_test.cc"],
    deps = [
        ":anonymous_tokens_redemption_client",
        "//anonymous_tokens/cpp/testing:utils",
        "//anonymous_tokens/proto:anonymous_tokens_cc_proto",
        "@com_github_google_googletest//:gtest_main",
    ],
)
