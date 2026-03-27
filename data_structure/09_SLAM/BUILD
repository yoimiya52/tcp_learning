load("@rules_cc//cc:defs.bzl", "cc_binary", "cc_library")

cc_binary(
    name = "eigen_main",
    srcs = ["eigenMatrix.cpp"],
    copts = [
        "-I/usr/include/eigen3",
        "-I/usr/local/include/pangolin",
    ],
    linkopts = [
        "-L/usr/local/lib",               
        "-lpangolin",                      
        "-lGL",                           
        "-lGLEW",                                                   
        "-lX11",                         
        "-lpthread",                   
    ],
)



