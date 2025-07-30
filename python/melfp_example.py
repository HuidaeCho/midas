#!/usr/bin/env python3
from ctypes import *

lib = CDLL("/home/hcho/usr/local/src/midas/src/libmidas.so")

def def_c_function(lib, func_name, argtypes, restype, defaults=()):
    func = getattr(lib, func_name)
    func.argtypes = argtypes
    func.restype = restype

    def wrapped(*args):
        # fill in missing args with defaults
        full_args = args + defaults[len(args):]
        return func(*full_args)

    return wrapped

def byte(s):
    return s.encode() if s else None

c_init_midas = def_c_function(
        lib, "init_midas",
        [
            POINTER(c_int)  # num_threads
        ],
        None,
)
def init_midas(num_threads):
    num_threads = c_int(num_threads)
    c_init_midas(byref(num_threads))
    return num_threads.value

c_read_tracing_stack_size = def_c_function(
        lib, "read_tracing_stack_size",
        [
            POINTER(c_int)  # tracing_stack_size
        ],
        None,
)
def read_tracing_stack_size(tracing_stack_size):
    tracing_stack_size = c_int(tracing_stack_size)
    c_read_tracing_stack_size(byref(tracing_stack_size))
    return tracing_stack_size.value

c_melfp = def_c_function(
        lib,
        "melfp",
        [
            c_char_p,    # dir_path
            c_char_p,    # dir_opts
            c_char_p,    # encoding
            c_char_p,    # outlets_path
            c_char_p,    # outlets_layer
            c_char_p,    # outlets_opts
            c_char_p,    # id_col
            c_char_p,    # output_path
            c_char_p,    # oid_col
            c_char_p,    # lfp_name
            c_char_p,    # heads_name
            c_char_p,    # coors_path
            c_int,     # find_full
            c_int,     # use_lessmem
            c_int,     # save_outlets
            c_int,     # num_threads
            c_int      # tracing_stack_size
        ],
        c_int,
)
def melfp(dir_path,
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
          tracing_stack_size=3072):
    return c_melfp(byte(dir_path),
                   byte(dir_opts),
                   byte(encoding),
                   byte(outlets_path),
                   byte(outlets_layer),
                   byte(outlets_opts),
                   byte(id_col),
                   byte(output_path),
                   byte(oid_col),
                   byte(lfp_name),
                   byte(heads_name),
                   byte(coors_path),
                   find_full,
                   c_int(use_lessmem),
                   save_outlets,
                   c_int(num_threads),
                   c_int(tracing_stack_size))

tracing_stack_size = read_tracing_stack_size(0)
num_threads = init_midas(0)

melfp("inputs/fdr.tif",
      None,
      None,
      "inputs/outlets515152.shp",
      None,
      None,
      "cat",
      "a.gpkg",
      num_threads=num_threads,
      tracing_stack_size=tracing_stack_size)
