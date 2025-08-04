#!/usr/bin/env python3
from midas import *

tracing_stack_size = read_tracing_stack_size(0)
num_threads = init_midas(0)

mefa("inputs/fdr.tif", None, None, "accum.tif", num_threads=num_threads)

meshed(
    "inputs/fdr.tif",
    None,
    None,
    "inputs/outlets515152.shp",
    None,
    None,
    "cat",
    "wsheds.tif",
    num_threads=num_threads,
    tracing_stack_size=tracing_stack_size,
)

melfp(
    "inputs/fdr.tif",
    None,
    None,
    "inputs/outlets515152.shp",
    None,
    None,
    "cat",
    "lfp.gpkg",
    num_threads=num_threads,
    tracing_stack_size=tracing_stack_size,
)
