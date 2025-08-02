#!/bin/sh
# run this script from windows in WSL
[ -d lib ] && rm -rf lib
mkdir lib
peldd	--ignore-errors -a --no-path \
	--path /mnt/c/opt/miniconda/envs/midas/Library/bin \
	msvc/midas.dll | sed '/midas.dll/d' | xargs -r cp -av -t lib
