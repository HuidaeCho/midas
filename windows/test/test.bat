@echo off
set PATH=..\mingw;%PATH%

mefa inputs_TX\fdr.tif outputs_TX\fac.tif
mefa -l inputs_TX\fdr.tif outputs_TX\fac-lessmem.tif
meshed inputs_TX\fdr.tif inputs_TX\outlets1000.shp cat outputs_TX\wsheds1000.tif
meshed -l inputs_TX\fdr.tif inputs_TX\outlets1000.shp cat outputs_TX\wsheds1000-lessmem.tif
melfp inputs_TX\fdr.tif inputs_TX\outlets1000.shp cat outputs_TX\lfp1000.csv
melfp -l inputs_TX\fdr.tif inputs_TX\outlets1000.shp cat outputs_TX\lfp1000-lessmem.csv
meflen inputs_TX\fdr.tif outputs_TX\dflen.tif
meflen -L inputs_TX\fdr.tif outputs_TX\dflen-leastmem.tif
meflen -u inputs_TX\fdr.tif outputs_TX\uflen.tif
meflen -ul inputs_TX\fdr.tif outputs_TX\uflen-lessmem.tif
meflen -uL inputs_TX\fdr.tif outputs_TX\uflen-leastmem.tif
