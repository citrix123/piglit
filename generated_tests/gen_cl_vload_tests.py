# Copyright 2016 Advanced Micro Devices, Inc.
#
# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and associated documentation files (the "Software"),
# to deal in the Software without restriction, including without limitation
# the rights to use, copy, modify, merge, publish, distribute, sublicense,
# and/or sell copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice (including the next
# paragraph) shall be included in all copies or substantial portions of the
# Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
# THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

from __future__ import print_function, division, absolute_import
import os
import random
import textwrap

from six.moves import range

from modules import utils

TYPES = ['char', 'uchar', 'short', 'ushort', 'int', 'uint', 'long', 'ulong', 'half', 'float', 'double']
VEC_SIZES = ['2', '4', '8', '16']

DIR_NAME = os.path.join("cl", "vload")


def gen_array(size):
    random.seed(size)
    return [str(random.randint(0, 255)) for i in range(size)]


def ext_req(type_name):
    if type_name[:6] == "double":
        return "require_device_extensions: cl_khr_fp64"
    if type_name[:4] == "half":
        return "require_device_extensions: cl_khr_fp16"
    return ""


def begin_test(suffix, type_name, mem_type, vec_sizes, addr_space):
    file_name = os.path.join(DIR_NAME, "vload{}-{}-{}.cl".format(suffix, type_name, addr_space))
    print(file_name)
    f = open(file_name, 'w')
    f.write(textwrap.dedent(("""\
    /*!
    [config]
    name: Vector load{suffix} {addr_space} {type_name}2,4,8,16
    clc_version_min: 10

    dimensions: 1
    global_size: 1 0 0
    """ + ext_req(type_name))
    .format(type_name=type_name, addr_space=addr_space, suffix=suffix)))
    for s in vec_sizes:
        size = int(s) if s != '' else 1
        data_array = gen_array(size)
        ty_name = type_name + s
        f.write(textwrap.dedent("""
        [test]
        name: vector load{suffix} {addr_space} {type_name}
        kernel_name: vload{suffix}{n}_{addr_space}
        arg_in:  0 buffer {mem_type}[{size}] 0 {gen_array}
        arg_out: 1 buffer {type_name}[2] {first_array} {gen_array}

        [test]
        name: vector load{suffix} {addr_space} offset {type_name}
        kernel_name: vload{suffix}{n}_{addr_space}_offset
        arg_in:  0 buffer {mem_type}[{offset_size}] {zeros}{gen_array}
        arg_out: 1 buffer {type_name}[2] {first_array} {gen_array}
        """.format(type_name=ty_name, mem_type=mem_type, size=size + 1,
                   zeros=("0 " * (size + 1)), offset_size=size * 2 + 1, n=s,
                   gen_array=' '.join(data_array), suffix=suffix,
                   addr_space=addr_space,
                   first_array="0 " + ' '.join(data_array[:-1]))))

    f.write(textwrap.dedent("""
    !*/
    """))
    if type_name == "double":
        f.write(textwrap.dedent("""
        #pragma OPENCL EXTENSION cl_khr_fp64: enable
        """))
    if type_name == "half":
        f.write(textwrap.dedent("""
        #pragma OPENCL EXTENSION cl_khr_fp16: enable
        """))
    return f


def gen_test_constant_global(suffix, t, mem_type, vec_sizes, addr_space):
    f = begin_test(suffix, t, mem_type, vec_sizes, addr_space)
    for s in vec_sizes:
        type_name = t + s
        f.write(textwrap.dedent("""
        kernel void vload{suffix}{n}_{addr_space}({addr_space} {mem_type} *in,
                                     global {type_name} *out) {{
            out[0] = vload{suffix}{n}(0, in);
            out[1] = vload{suffix}{n}(0, in + 1);
        }}

        kernel void vload{suffix}{n}_{addr_space}_offset({addr_space} {mem_type} *in,
                                            global {type_name} *out) {{
            out[0] = vload{suffix}{n}(1, in);
            out[1] = vload{suffix}{n}(1, in + 1);
        }}
        """.format(type_name=type_name, mem_type=mem_type, n=s, suffix=suffix,
                   addr_space=addr_space)))

    f.close()


def gen_test_local_private(suffix, t, mem_type, vec_sizes, addr_space):
    f = begin_test(suffix, t, mem_type, vec_sizes, addr_space)
    for s in vec_sizes:
        size = int(s) if s != '' else 1
        type_name = t + s
        f.write(textwrap.dedent("""
        kernel void vload{suffix}{n}_{addr_space}(global {mem_type} *in,
                                     global {type_name} *out) {{
            volatile {addr_space} {mem_type} loc[{size}];
            for (int i = 0; i < {size}; ++i)
                loc[i] = in[i];

            out[0] = vload{suffix}{n}(0, ({addr_space} {mem_type}*)loc);
            out[1] = vload{suffix}{n}(0, ({addr_space} {mem_type}*)loc + 1);
        }}

        kernel void vload{suffix}{n}_{addr_space}_offset(global {mem_type} *in,
                                            global {type_name} *out) {{
            volatile {addr_space} {mem_type} loc[{offset_size}];
            for (int i = 0; i < {offset_size}; ++i)
                loc[i] = in[i];

            out[0] = vload{suffix}{n}(1, ({addr_space} {mem_type}*)loc);
            out[1] = vload{suffix}{n}(1, ({addr_space} {mem_type}*)loc + 1);
        }}
        """.format(type_name=type_name, mem_type=mem_type, n=s, suffix=suffix,
                   offset_size=size * 2 + 1, size=size + 1, addr_space=addr_space)))

    f.close()


# vload_half is special, becuase CLC won't allow us to use half type without
# cl_khr_fp16
def gen_test_local_private_half(suffix, t, vec_sizes, addr_space):
    f = begin_test(suffix, t, 'half', vec_sizes, addr_space)
    for s in vec_sizes:
        size = int(s) if s != '' else 1
        type_name = t + s
        f.write(textwrap.dedent("""
        kernel void vload{suffix}{n}_{addr_space}(global half *in,
                                     global {type_name} *out) {{
            volatile {addr_space} short loc[{size}];
            for (int i = 0; i < {size}; ++i)
                loc[i] = ((global short *)in)[i];

            out[0] = vload{suffix}{n}(0, ({addr_space} half*)loc);
            out[1] = vload{suffix}{n}(0, ({addr_space} half*)loc + 1);
        }}

        kernel void vload{suffix}{n}_{addr_space}_offset(global half *in,
                                            global {type_name} *out) {{
            volatile {addr_space} short loc[{offset_size}];
            for (int i = 0; i < {offset_size}; ++i)
                loc[i] = ((global short *)in)[i];

            out[0] = vload{suffix}{n}(1, ({addr_space} half*)loc);
            out[1] = vload{suffix}{n}(1, ({addr_space} half*)loc + 1);
        }}
        """.format(type_name=type_name, n=s, suffix=suffix,
                   offset_size=size * 2 + 1, size=size + 1, addr_space=addr_space)))


def gen_test_local(suffix, t, mem_type, vec_sizes):
    if mem_type == 'half':
        gen_test_local_private_half(suffix, t, vec_sizes, 'local')
    else:
        gen_test_local_private(suffix, t, mem_type, vec_sizes, 'local')


def gen_test_private(suffix, t, mem_type, vec_sizes):
    if mem_type == 'half':
        gen_test_local_private_half(suffix, t, vec_sizes, 'private')
    else:
        gen_test_local_private(suffix, t, mem_type, vec_sizes, 'private')


def gen_test_global(suffix, t, mem_type, vec_sizes):
    gen_test_constant_global(suffix, t, mem_type, vec_sizes, 'global')


def gen_test_constant(suffix, t, mem_type, vec_sizes):
    gen_test_constant_global(suffix, t, mem_type, vec_sizes, 'constant')


def main():
    utils.safe_makedirs(DIR_NAME)
    for t in TYPES:
        gen_test_constant('', t, t, VEC_SIZES);
        gen_test_global('', t, t, VEC_SIZES);
        gen_test_local('', t, t, VEC_SIZES);
        gen_test_private('', t, t, VEC_SIZES);

    # There's no vload_half for double type
    gen_test_constant('_half', 'float',  'half', [''] + VEC_SIZES);
    gen_test_global('_half', 'float',  'half', [''] + VEC_SIZES);
    gen_test_local('_half', 'float',  'half', [''] + VEC_SIZES);
    gen_test_private('_half', 'float',  'half', [''] + VEC_SIZES);


if __name__ == '__main__':
    main()
