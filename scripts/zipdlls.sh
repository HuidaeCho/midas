#!/bin/sh
# run this script from windows in WSL
for cc in msvc mingw; do
	lib=lib_$cc
	env=midas_$cc
	[ -d $lib ] && rm -rf $lib
	mkdir $lib

	case $cc in
	msvc)
		dll=$cc/midas.dll
		;;
	mingw)
		dll=$cc/libmidas.dll
		;;
	esac

	peldd	--ignore-errors -a --no-path \
		--path /mnt/c/opt/miniconda/envs/$env/Library/bin \
		$dll | sed '/midas.dll/d' | xargs -r cp -av -t $lib
	[ -f $lib.zip ] && rm -f $lib.zip
	zip -r $lib.zip $lib
	rm -rf $lib
done
