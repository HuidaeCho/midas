@echo off
mkdir inputs outputs
for %%i in (fdr.tif outlets1000.dbf outlets1000.prj outlets1000.shp outlets1000.shx) do curl https://data.isnew.info/meidas/%%i -o inputs/%%i
