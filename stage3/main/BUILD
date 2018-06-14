cc_library(
    name = "hello-greet",
    srcs = ["hello-greet.cc"],
    hdrs = ["hello-greet.h"],
)

cc_binary(
    name = "hello-world",
    srcs = ["hello-world.cc"],
    deps = [
        ":hello-greet",
        "//lib:hello-time",
    ],
    copts = ["-Ilib",],
    #linkopts最终会加入到文件中@bazel-out/k8-fastbuild/bin/main/hello-world-2.params
    linkopts = ["-Wl,-Bdynamic -ldl -lm -lc -lz -lrt -lpthread"],
)
