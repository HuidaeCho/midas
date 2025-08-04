from ctypes.util import find_library
from ctypes import *

lib_path = find_library("midas")
if lib_path is None:
    raise FileNotFoundError("Cannot find libmidas")
lib = CDLL(lib_path)


def def_c_function(lib, func_name, argtypes, restype, defaults=()):
    func = getattr(lib, func_name)
    func.argtypes = argtypes
    func.restype = restype

    def wrapped(*args):
        # fill in missing args with defaults
        full_args = args + defaults[len(args) :]
        return func(*full_args)

    return wrapped


def b(s):
    return s.encode() if s else None


c_init_midas = def_c_function(
    lib,
    "init_midas",
    [POINTER(c_int)],  # num_threads
    None,
)


def init_midas(num_threads):
    num_threads = c_int(num_threads)
    c_init_midas(byref(num_threads))
    return num_threads.value


c_read_tracing_stack_size = def_c_function(
    lib,
    "read_tracing_stack_size",
    [POINTER(c_int)],  # tracing_stack_size
    None,
)


def read_tracing_stack_size(tracing_stack_size):
    tracing_stack_size = c_int(tracing_stack_size)
    c_read_tracing_stack_size(byref(tracing_stack_size))
    return tracing_stack_size.value


c_mefa = def_c_function(
    lib,
    "mefa",
    [
        c_char_p,  # dir_path
        c_char_p,  # dir_opts
        c_char_p,  # encoding
        c_char_p,  # accum_path
        c_int,  # use_lessmem
        c_int,  # compress_output
        c_int,  # num_threads
    ],
    c_int,
)


def mefa(
    dir_path,
    dir_opts,
    encoding,
    accum_path,
    use_lessmem=False,
    compress_output=False,
    num_threads=0,
):
    return c_mefa(
        b(dir_path),
        b(dir_opts),
        b(encoding),
        b(accum_path),
        use_lessmem,
        compress_output,
        num_threads,
    )


c_meshed = def_c_function(
    lib,
    "meshed",
    [
        c_char_p,  # dir_path
        c_char_p,  # dir_opts
        c_char_p,  # encoding
        c_char_p,  # outlets_path
        c_char_p,  # outlets_layer
        c_char_p,  # outlets_opts
        c_char_p,  # id_col
        c_char_p,  # wsheds_path
        c_char_p,  # hier_path
        c_int,  # use_lessmem
        c_int,  # compress_output
        c_int,  # save_outlets
        c_int,  # num_threads
        c_int,  # tracing_stack_size
    ],
    c_int,
)


def meshed(
    dir_path,
    dir_opts,
    encoding,
    outlets_path,
    outlets_layer,
    outlets_opts,
    id_col,
    wsheds_path,
    hier_path=None,
    use_lessmem=False,
    compress_output=False,
    save_outlets=False,
    num_threads=0,
    tracing_stack_size=3072,
):
    return c_meshed(
        b(dir_path),
        b(dir_opts),
        b(encoding),
        b(outlets_path),
        b(outlets_layer),
        b(outlets_opts),
        b(id_col),
        b(wsheds_path),
        b(hier_path),
        use_lessmem,
        compress_output,
        save_outlets,
        num_threads,
        tracing_stack_size,
    )


c_melfp = def_c_function(
    lib,
    "melfp",
    [
        c_char_p,  # dir_path
        c_char_p,  # dir_opts
        c_char_p,  # encoding
        c_char_p,  # outlets_path
        c_char_p,  # outlets_layer
        c_char_p,  # outlets_opts
        c_char_p,  # id_col
        c_char_p,  # output_path
        c_char_p,  # oid_col
        c_char_p,  # lfp_name
        c_char_p,  # heads_name
        c_char_p,  # coors_path
        c_int,  # find_full
        c_int,  # use_lessmem
        c_int,  # save_outlets
        c_int,  # num_threads
        c_int,  # tracing_stack_size
    ],
    c_int,
)


def melfp(
    dir_path,
    dir_opts,
    encoding,
    outlets_path,
    outlets_layer,
    outlets_opts,
    id_col,
    output_path,
    oid_col="lfp_id",
    lfp_name="lfp",
    heads_name=None,
    coors_path=None,
    find_full=False,
    use_lessmem=2,
    save_outlets=False,
    num_threads=0,
    tracing_stack_size=3072,
):
    return c_melfp(
        b(dir_path),
        b(dir_opts),
        b(encoding),
        b(outlets_path),
        b(outlets_layer),
        b(outlets_opts),
        b(id_col),
        b(output_path),
        b(oid_col),
        b(lfp_name),
        b(heads_name),
        b(coors_path),
        find_full,
        use_lessmem,
        save_outlets,
        num_threads,
        tracing_stack_size,
    )
