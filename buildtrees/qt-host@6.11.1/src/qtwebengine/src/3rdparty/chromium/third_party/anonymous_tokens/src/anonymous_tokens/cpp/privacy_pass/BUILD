package(default_visibility = ["//:__subpackages__"])

licenses(["notice"])

cc_library(
    name = "rsa_bssa_public_metadata_client",
    srcs = [
        "rsa_bssa_public_metadata_client.cc",
    ],
    hdrs = [
        "rsa_bssa_public_metadata_client.h",
    ],
    deps = [
        ":token_encodings",
        "//anonymous_tokens/cpp/crypto:constants",
        "//anonymous_tokens/cpp/crypto:crypto_utils",
        "//anonymous_tokens/cpp/crypto:rsa_blinder",
        "//anonymous_tokens/cpp/shared:status_utils",
        "@boringssl//:ssl",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/status:statusor",
        "@com_google_absl//absl/strings",
    ],
)

cc_test(
    name = "rsa_bssa_public_metadata_client_test",
    srcs = ["rsa_bssa_public_metadata_client_test.cc"],
    deps = [
        ":rsa_bssa_public_metadata_client",
        ":token_encodings",
        "//anonymous_tokens/cpp/crypto:crypto_utils",
        "//anonymous_tokens/cpp/testing:utils",
        "@boringssl//:ssl",
        "@com_github_google_googletest//:gtest_main",
        "@com_google_absl//absl/status:statusor",
    ],
)

cc_library(
    name = "token_encodings",
    srcs = [
        "token_encodings.cc",
    ],
    hdrs = [
        "token_encodings.h",
    ],
    deps = [
        "//anonymous_tokens/cpp/shared:status_utils",
        "@boringssl//:ssl",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/status:statusor",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/strings:str_format",
        "@com_google_absl//absl/time",
        "@com_google_absl//absl/types:span",
    ],
)

cc_test(
    name = "token_encodings_test",
    srcs = [
        "token_encodings_test.cc",
    ],
    deps = [
        ":token_encodings",
        "//anonymous_tokens/cpp/testing:utils",
        "@com_github_google_googletest//:gtest_main",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/status:statusor",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/time",
        "@com_google_absl//absl/types:span",
    ],
)
