#!/bin/sh
# run this script from windows/msvc or windows/mingw in WSL
[ -d lib ] && rm -rf lib
mkdir lib
if [ -f midas.dll ]; then
	peldd	--ignore-errors -a --no-path \
		--path /mnt/c/opt/miniconda/envs/midas/Library/bin \
		midas.dll | sed '/midas.dll/d' | xargs -r cp -av -t lib
elif [ -f libmidas.dll ]; then
	peldd	--ignore-errors -a --no-path \
		--path /mnt/c/opt/miniconda/envs/midas_mingw/Library/bin \
		libmidas.dll | sed '/libmidas.dll/d' | xargs -r cp -av -t lib
fi
