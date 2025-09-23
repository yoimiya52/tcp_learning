load("@rules_proto//proto:defs.bzl", "proto_library")
load("@rules_cc//cc:defs.bzl", "cc_proto_library")

package(default_visibility = ["//visibility:public"])

cc_proto_library(
    name = "motive_cc_proto",
    deps = [
        ":motive_proto",
    ],
)

proto_library(
    name = "motive_proto",
    srcs = ["HumanoidMotion.proto"],
)
