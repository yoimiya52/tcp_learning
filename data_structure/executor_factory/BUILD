load("@rules_cc//cc:defs.bzl", "cc_library", "cc_binary")

cc_library(
    name = "script_manager_lib",
    srcs = [
        "src/config_parser.cpp",
        "src/exec_context.cpp",
        "src/flow_executor.cpp",
        "src/script_executor.cpp",
        "src/script_factory.cpp",
    ],
    hdrs = glob([
        "include/*.h",
    ]),
    includes = ["include"], 
    deps = [
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/container:flat_hash_map",
    ],
    visibility = ["//visibility:public"],
)

cc_binary(
    name = "script_manager",
    srcs = ["main.cpp"],
    copts = [
        "-std=c++17",
        "-finput-charset=utf-8",
        "-fexec-charset=utf-8",
    ],
    deps = [
        ":script_manager_lib",
    ],
)
