# Copyright 2019 Google LLC
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      https://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

load("@com_google_api_codegen//rules_gapic:gapic.bzl", "gapic_srcjar", "unzipped_srcjar")

def _cc_gapic_postprocessed_srcjar_impl(ctx):
    gapic_zip = ctx.file.gapic_zip
    output_main = ctx.outputs.main
    output_main_h = ctx.outputs.main_h

    output_dir_name = ctx.label.name
    output_dir_path = "%s/%s" % (output_main.dirname, output_dir_name)

    script = """
    unzip -q {gapic_zip} -d {output_dir_path}
    # TODO: Call formatter here
    pushd {output_dir_path}
    zip -q -r {output_dir_name}-h.srcjar . -i ./*.gapic.h
    find . -name "*.gapic.h" -delete
    zip -q -r {output_dir_name}.srcjar . -i ./*.cc -i ./*.h
    popd
    mv {output_dir_path}/{output_dir_name}-h.srcjar {output_main_h}
    mv {output_dir_path}/{output_dir_name}.srcjar {output_main}
    rm -rf {output_dir_path}
    """.format(
        gapic_zip = gapic_zip.path,
        output_dir_name = output_dir_name,
        output_dir_path = output_dir_path,
        output_main = output_main.path,
        output_main_h = output_main_h.path,
    )

    ctx.actions.run_shell(
        inputs = [gapic_zip],
        command = script,
        outputs = [output_main, output_main_h],
    )

_cc_gapic_postprocessed_srcjar = rule(
    _cc_gapic_postprocessed_srcjar_impl,
    attrs = {
        "gapic_zip": attr.label(mandatory = True, allow_single_file = True),
    },
    outputs = {
        "main": "%{name}.srcjar",
        "main_h": "%{name}-h.srcjar",
    },
)

def cc_gapic_srcjar(name, src, package, **kwargs):
    raw_srcjar_name = "%s_raw" % name

    gapic_srcjar(
        name = raw_srcjar_name,
        src = src,
        package = package,
        output_suffix = ".zip",
        gapic_generator = Label("//generator:protoc-gen-cpp_gapic"),
        **kwargs
    )

    _cc_gapic_postprocessed_srcjar(
        name = name,
        gapic_zip = ":%s" % raw_srcjar_name,
        **kwargs
    )

def cc_gapic_library(name, src, package, deps = [], **kwargs):
    srcjar_name = "%s_srcjar" % name

    cc_gapic_srcjar(
        name = srcjar_name,
        src = src,
        package = package,
        **kwargs
    )

    actual_deps = deps + [
        "@com_google_gapic_generator_cpp//gax:gax",
    ]

    main_file = ":%s.srcjar" % srcjar_name
    main_dir = "%s_main" % srcjar_name

    unzipped_srcjar(
        name = main_dir,
        srcjar = main_file,
        extension = "",
        **kwargs
    )

    main_h_file = ":%s-h.srcjar" % srcjar_name
    main_h_dir = "%s_h_main" % srcjar_name

    unzipped_srcjar(
        name = main_h_dir,
        srcjar = main_h_file,
        extension = "",
        **kwargs
    )

    native.cc_library(
        name = name,
        srcs = [":%s" % main_dir],
        deps = actual_deps,
        hdrs = [":%s" % main_h_dir],
        includes = [main_h_dir],
        **kwargs
    )
