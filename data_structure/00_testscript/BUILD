load("@rules_cc//cc:defs.bzl", "cc_binary", "cc_library")

cc_library(
    name = "parse_yaml",
    srcs = ["parse_yaml.cpp","common_utils.hpp"],
    deps = [
        "//proto_msgs:motive_cc_proto",
    ],
    copts = [
        "-std=c++17",
        "-I/usr/include/eigen3",
        "-I/usr/include/opencv4",
    ],
    linkopts = [
        "-lyaml-cpp",
        "-lopencv_core",
        "-lopencv_imgproc",
        "-lopencv_calib3d",
        "-lopencv_highgui",
        "-lopencv_imgcodecs",
    ],
)


