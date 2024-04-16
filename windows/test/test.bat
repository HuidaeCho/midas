@echo off
set PATH=..;..\lib;%PATH%
set PROJ_LIB=..\lib

mefa fdr.tif outputs\fac.tif
mefa-lessmem fdr.tif outputs\fac-lessmem.tif
meshed fdr.tif outlets1000.shp cat outputs\wsheds1000.tif
meshed-lessmem fdr.tif outlets1000.shp cat outputs\wsheds1000-lessmem.tif
melfp fdr.tif outlets1000.shp cat outputs\lfp1000.csv
melfp -l fdr.tif outlets1000.shp cat outputs\lfp1000-lessmem.csv
meflen fdr.tif outputs\dflen.tif
meflen -L fdr.tif outputs\dflen-leastmem.tif
meflen -u fdr.tif outputs\uflen.tif
meflen -ul fdr.tif outputs\uflen-lessmem.tif
meflen -uL fdr.tif outputs\uflen-leastmem.tif
