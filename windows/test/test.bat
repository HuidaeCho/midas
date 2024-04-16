@echo off
set PATH=..;..\lib;%PATH%
set PROJ_LIB=..\lib

mefa inputs\fdr.tif outputs\fac.tif
mefa-lessmem inputs\fdr.tif outputs\fac-lessmem.tif
meshed inputs\fdr.tif inputs\outlets1000.shp cat outputs\wsheds1000.tif
meshed-lessmem inputs\fdr.tif inputs\outlets1000.shp cat outputs\wsheds1000-lessmem.tif
melfp inputs\fdr.tif inputs\outlets1000.shp cat outputs\lfp1000.csv
melfp -l inputs\fdr.tif inputs\outlets1000.shp cat outputs\lfp1000-lessmem.csv
meflen inputs\fdr.tif outputs\dflen.tif
meflen -L inputs\fdr.tif outputs\dflen-leastmem.tif
meflen -u inputs\fdr.tif outputs\uflen.tif
meflen -ul inputs\fdr.tif outputs\uflen-lessmem.tif
meflen -uL inputs\fdr.tif outputs\uflen-leastmem.tif
