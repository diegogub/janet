/*
* Copyright (c) 2017 Calvin Rose
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to
* deal in the Software without restriction, including without limitation the
* rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
* sell copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
* FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
* IN THE SOFTWARE.
*/

#include <dst/dst.h>
#include <dst/dststl.h>

int dst_stl_print(int32_t argn, DstValue *argv, DstValue *ret) {
    (void)ret;

    int32_t i;
    for (i = 0; i < argn; ++i) {
        int32_t j, len;
        const uint8_t *vstr = dst_to_string(argv[i]);
        len = dst_string_length(vstr);
        for (j = 0; j < len; ++j) {
            putc(vstr[j], stdout);
        }
    }
    putc('\n', stdout);
    return 0;
}

int dst_stl_describe(int32_t argn, DstValue *argv, DstValue *ret) {
    (void)ret;

    int32_t i;
    for (i = 0; i < argn; ++i) {
        int32_t j, len;
        const uint8_t *vstr = dst_description(argv[i]);
        len = dst_string_length(vstr);
        for (j = 0; j < len; ++j) {
            putc(vstr[j], stdout);
        }
    }
    putc('\n', stdout);
    return 0;
}

int dst_stl_asm(int32_t argn, DstValue *argv, DstValue *ret) {
    DstAssembleOptions opts;
    DstAssembleResult res;
    if (argn < 1) {
        *ret = dst_cstringv("expected assembly source");
        return 1;
    }
    opts.source = argv[0];
    opts.flags = 0;
    res = dst_asm(opts);
    if (res.status == DST_ASSEMBLE_OK) {
        *ret = dst_wrap_function(dst_asm_func(res));
        return 0;
    } else {
        *ret = dst_wrap_string(res.error);
        return 1;
    }
}

int dst_stl_disasm(int32_t argn, DstValue *argv, DstValue *ret) {
    DstFunction *f;
    if (argn < 1 || !dst_checktype(argv[0], DST_FUNCTION)) {
        *ret = dst_cstringv("expected function");
        return 1;
    }
    f = dst_unwrap_function(argv[0]);
    *ret = dst_disasm(f->def);
    return 0;
}

int dst_stl_tuple(int32_t argn, DstValue *argv, DstValue *ret) {
    *ret = dst_wrap_tuple(dst_tuple_n(argv, argn));
    return 0;
}

int dst_stl_array(int32_t argn, DstValue *argv, DstValue *ret) {
    DstArray *array = dst_array(argn);
    array->count = argn;
    memcpy(array->data, argv, argn * sizeof(DstValue));
    *ret = dst_wrap_array(array);
    return 0;
}

int dst_stl_table(int32_t argn, DstValue *argv, DstValue *ret) {
    int32_t i;
    DstTable *table = dst_table(argn >> 1);
    if (argn & 1) {
        *ret = dst_cstringv("expected even number of arguments");
        return 1;
    }
    for (i = 0; i < argn; i += 2) {
        dst_table_put(table, argv[i], argv[i + 1]);
    }
    *ret = dst_wrap_table(table);
    return 0;
}

int dst_stl_struct(int32_t argn, DstValue *argv, DstValue *ret) {
    int32_t i;
    DstKV *st = dst_struct_begin(argn >> 1);
    if (argn & 1) {
        *ret = dst_cstringv("expected even number of arguments");
        return 1;
    }
    for (i = 0; i < argn; i += 2) {
        dst_struct_put(st, argv[i], argv[i + 1]);
    }
    *ret = dst_wrap_struct(dst_struct_end(st));
    return 0;
}

int dst_stl_fiber(int32_t argn, DstValue *argv, DstValue *ret) {
    DstFiber *fiber;
    if (argn < 1) {
        *ret = dst_cstringv("expected at least one argument");
        return 1;
    }
    if (!dst_checktype(argv[0], DST_FUNCTION)) {
        *ret = dst_cstringv("expected a function");
        return 1;
    }
    fiber = dst_fiber(64);
    dst_fiber_funcframe(fiber, dst_unwrap_function(argv[0]));
    fiber->parent = dst_vm_fiber;
    *ret = dst_wrap_fiber(fiber);
    return 0;
}

int dst_stl_buffer(int32_t argn, DstValue *argv, DstValue *ret) {
    DstBuffer *buffer = dst_buffer(10);
    int32_t i;
    for (i = 0; i < argn; ++i) {
        const uint8_t *bytes = dst_to_string(argv[i]);
        int32_t len = dst_string_length(bytes);
        dst_buffer_push_bytes(buffer, bytes, len);
    }
    *ret = dst_wrap_buffer(buffer);
    return 0;
}

int dst_stl_gensym(int32_t argn, DstValue *argv, DstValue *ret) {
    if (argn > 1) {
        *ret = dst_cstringv("expected one argument");
        return 1;
    }
    if (argn == 0) {
        *ret = dst_wrap_symbol(dst_symbol_gen(NULL, 0));
    } else {
        const uint8_t *s = dst_to_string(argv[0]);
        *ret = dst_wrap_symbol(dst_symbol_gen(s, dst_string_length(s)));
    }
    return 0;
}

int dst_stl_get(int32_t argn, DstValue *argv, DstValue *ret) {
    int32_t i;
    DstValue ds;
    if (argn < 1) {
        *ret = dst_cstringv("expected at least 1 argument");
        return 1;
    }
    ds = argv[0];
    for (i = 1; i < argn; i++) {
        ds = dst_get(ds, argv[i]);
        if (dst_checktype(ds, DST_NIL))
            break;
    }
    *ret = ds;
    return 0;
}

int dst_stl_put(int32_t argn, DstValue *argv, DstValue *ret) {
    DstValue ds, key, value;
    if (argn < 3) {
        *ret = dst_cstringv("expected at least 3 arguments");
        return 1;
    }
    if (dst_stl_get(argn - 2, argv, ret)) {
        return 1;
    }
    ds = *ret;
    key = argv[argn - 2];
    value = argv[argn - 1];
    dst_put(ds, key, value);
    return 0;
}

static int dst_stl_equal(int32_t argn, DstValue *argv, DstValue *ret) {
    int32_t i;
    for (i = 0; i < argn - 1; i++) {
        if (!dst_equals(argv[i], argv[i+1])) {
            *ret = dst_wrap_false();
            return 0;
        }
    }
    *ret = dst_wrap_true();
    return 0;
}

static int dst_stl_notequal(int32_t argn, DstValue *argv, DstValue *ret) {
    int32_t i;
    for (i = 0; i < argn - 1; i++) {
        if (dst_equals(argv[i], argv[i+1])) {
            *ret = dst_wrap_true();
            return 0;
        }
    }
    *ret = dst_wrap_false();
    return 0;
}

static int dst_stl_not(int32_t argn, DstValue *argv, DstValue *ret) {
    *ret = dst_wrap_boolean(argn == 0 || !dst_truthy(argv[0]));
    return 0;
}

#define DST_DEFINE_COMPARATOR(name, pred)\
static int dst_stl_##name(int32_t argn, DstValue *argv, DstValue *ret) {\
    int32_t i;\
    for (i = 0; i < argn - 1; i++) {\
        if (dst_compare(argv[i], argv[i+1]) pred) {\
            *ret = dst_wrap_false();\
            return 0;\
        }\
    }\
    *ret = dst_wrap_true();\
    return 0;\
}

DST_DEFINE_COMPARATOR(ascending, >= 0)
DST_DEFINE_COMPARATOR(descending, <= 0)
DST_DEFINE_COMPARATOR(notdescending, > 0)
DST_DEFINE_COMPARATOR(notascending, < 0)

static DstReg stl[] = {
    {"print", dst_stl_print},
    {"describe", dst_stl_describe},
    {"table", dst_stl_table},
    {"array", dst_stl_array},
    {"tuple", dst_stl_tuple},
    {"struct", dst_stl_struct},
    {"fiber", dst_stl_fiber},
    {"buffer", dst_stl_buffer},
    {"gensym", dst_stl_gensym},
    {"asm", dst_stl_asm},
    {"disasm", dst_stl_disasm},
    {"get", dst_stl_get},
    {"put", dst_stl_put},
    {"+", dst_add},
    {"-", dst_subtract},
    {"*", dst_multiply},
    {"/", dst_divide},
    {"%", dst_modulo},
    {"cos", dst_cos},
    {"sin", dst_sin},
    {"tan", dst_tan},
    {"acos", dst_acos},
    {"asin", dst_asin},
    {"atan", dst_atan},
    {"exp", dst_exp},
    {"log", dst_log},
    {"log10", dst_log10},
    {"sqrt", dst_sqrt},
    {"floor", dst_floor},
    {"ceil", dst_ceil},
    {"pow", dst_pow},
    {"=", dst_stl_equal},
    {"not=", dst_stl_notequal},
    {"<", dst_stl_ascending},
    {">", dst_stl_descending},
    {"<=", dst_stl_notdescending},
    {">=", dst_stl_notascending},
    {"|", dst_bor},
    {"&", dst_band},
    {"^", dst_bxor},
    {">>", dst_lshift},
    {"<<", dst_rshift},
    {">>>", dst_lshiftu},
    {"not", dst_stl_not}
};

DstValue dst_loadstl(int flags) {
    DstValue ret = dst_loadreg(stl, sizeof(stl)/sizeof(DstReg));
    if (flags & DST_LOAD_ROOT) {
        dst_gcroot(ret);
    }
    if (dst_checktype(ret, DST_TABLE)) {
        DstTable *v = dst_table(1);
        dst_table_put(v, dst_csymbolv("value"), ret);
        dst_put(ret, dst_csymbolv("-env-"), dst_wrap_table(v));
    }
    return ret;
}
