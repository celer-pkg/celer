package(default_visibility = ["//:__subpackages__"])

licenses(["notice"])

cc_binary(
    name = "rsa_bssa_public_metadata_privacy_pass_client_demo",
    testonly = 1,
    srcs = [
        "rsa_bssa_public_metadata_privacy_pass_client_demo.cc",
    ],
    deps = [
        "//anonymous_tokens/cpp/crypto:crypto_utils",
        "//anonymous_tokens/cpp/privacy_pass:rsa_bssa_public_metadata_client",
        "//anonymous_tokens/cpp/privacy_pass:token_encodings",
        "//anonymous_tokens/cpp/testing:utils",
        "@boringssl//:ssl",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/status:statusor",
        "@com_google_absl//absl/strings",
    ],
)

cc_binary(
    name = "rsa_bssa_public_metadata_privacy_pass_server_demo",
    testonly = 1,
    srcs = [
        "rsa_bssa_public_metadata_privacy_pass_server_demo.cc",
    ],
    deps = [
        "//anonymous_tokens/cpp/crypto:crypto_utils",
        "//anonymous_tokens/cpp/privacy_pass:token_encodings",
        "//anonymous_tokens/cpp/testing:utils",
        "@boringssl//:ssl",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/status:statusor",
        "@com_google_absl//absl/strings",
    ],
)
