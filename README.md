# Memory-Efficient I/O-Improved Drainage Analysis System (MIDAS)

<!--ts-->
  * [Related projects](#related-projects)
  * [References](#references)
  * [Supported flow direction encodings](#supported-flow-direction-encodings)
  * [Installing on Windows from MSVC binaries](#installing-on-windows-from-msvc-binaries)
  * [Installing on Windows from MinGW GCC binaries](#installing-on-windows-from-mingw-gcc-binaries)
  * [Building on Windows using MSVC](#building-on-windows-using-msvc)
  * [Building on Windows using MinGW GCC](#building-on-windows-using-mingw-gcc)
  * [Building on Linux](#building-on-linux)
  * [Testing MSVC binaries on Windows using TX data](#testing-msvc-binaries-on-windows-using-tx-data)
  * [Testing MinGW GCC binaries on Windows using TX data](#testing-mingw-gcc-binaries-on-windows-using-tx-data)
  * [Benchmark results of MSVC vs. MinGW GCC vs. WSL GCC vs. Linux GCC binaries using CONUS data](#benchmark-results-of-msvc-vs-mingw-gcc-vs-wsl-gcc-vs-linux-gcc-binaries-using-conus-data)
  * [Benchmark results](#benchmark-results)
    * [MEFA](#mefa)
    * [MESHED](#meshed)
    * [MELFP](#melfp)
    * [MEFLEN](#meflen)

<!-- Created by https://github.com/ekalinin/github-markdown-toc -->
<!-- Added by: hcho, at: Mon Aug  4 08:46:58 MDT 2025 -->

<!--te-->

## Related projects

[MIDAS](https://github.com/HuidaeCho/midas) is the core C library and executables, and is required for all Python, R, and QGIS interfaces. The following projects are thin wrappers or interfaces to MIDAS's shared library.
* [MIDASFlow](https://github.com/HuidaeCho/midasflow): Python package
* [MIDASFlow-R](https://github.com/HuidaeCho/midasflow-r): R package
* [MIDAS-QGIS](https://github.com/HuidaeCho/midas-qgis): QGIS plugin

## References

* [MEFA](https://github.com/HuidaeCho/mefa) (Flow Accumulation): Huidae Cho, July 2023. Memory-Efficient Flow Accumulation Using a Look-Around Approach and Its OpenMP Parallelization. Environmental Modelling & Software 167, 105771. [doi:10.1016/j.envsoft.2023.105771](https://doi.org/10.1016/j.envsoft.2023.105771). [Author's Version](https://idea.isnew.info/publications/Memory-efficient%20flow%20accumulation%20using%20a%20look-around%20approach%20and%20its%20OpenMP%20parallelization%20-%20Cho.2023.pdf).
* [MESHED](https://github.com/HuidaeCho/meshed) (Watershed Delineation): Huidae Cho, January 2025. Avoid Backtracking and Burn Your Inputs: CONUS-Scale Watershed Delineation Using OpenMP. Environmental Modelling & Software 183, 106244. [doi:10.1016/j.envsoft.2024.106244](https://doi.org/10.1016/j.envsoft.2024.106244). [Author's Version](https://idea.isnew.info/publications/Avoid%20backtracking%20and%20burn%20your%20inputs:%20CONUS-scale%20watershed%20delineation%20using%20OpenMP%20-%20Cho.2025.pdf).
* [MELFP](https://github.com/HuidaeCho/melfp) (Longest Flow Path): Huidae Cho, September 2025. Loop Then Task: Hybridizing OpenMP Parallelism to Improve Load Balancing and Memory Efficiency in Continental-Scale Longest Flow Path Computation. Environmental Modelling & Software 193, 106630. [doi:10.1016/j.envsoft.2025.106630](https://doi.org/10.1016/j.envsoft.2025.106630). [Author's Version](https://idea.isnew.info/publications/Loop%20then%20task%20-%20Hybridizing%20OpenMP%20parallelism%20to%20improve%20load%20balancing%20and%20memory%20efficiency%20in%20continental-scale%20longest%20flow%20path%20computation%20-%20Cho.2025.pdf).

## Supported flow direction encodings

Predefined flow direction encodings in GeoTIFF: power2 (default, r.terraflow, ArcGIS), taudem (d8flowdir), 45degree (r.watershed), degree<br>
![image](https://github.com/user-attachments/assets/990f0530-fded-4ee5-bfbb-85056a50ca1c)
![image](https://github.com/user-attachments/assets/a02dfc15-a825-4210-82c4-4c9296dafadc)
![image](https://github.com/user-attachments/assets/64f5c65a-c7cc-4e06-a69f-6fccd6435426)
![image](https://github.com/user-attachments/assets/fafef436-a5f2-464a-89a8-9f50a877932c)

Custom flow direction encoding is also possible by passing `-e E,SE,S,SW,W,NW,N,NE` (e.g., 1,8,7,6,5,4,3,2 for taudem).

## Installing on Windows from MSVC binaries

1. Install [Git for Windows](https://gitforwindows.org/)
2. Install [Miniconda](https://www.anaconda.com/download/success)
```cmd
curl -O https://repo.anaconda.com/miniconda/Miniconda3-latest-Windows-x86_64.exe
mkdir C:\opt
Miniconda3-latest-Windows-x86_64.exe /S /D=C:\opt\miniconda
C:\opt\miniconda\condabin\conda.bat init
```
3. Setup Conda for MIDAS run
```cmd
conda config --add channels conda-forge
conda config --set channel_priority strict
conda create -n midas_msvc libgdal
conda activate midas_msvc
```
4. Download the source code and binaries
```cmd
cd \opt
git clone https://github.com/HuidaeCho/midas.git
```
5. Run MIDAS (add `C:\opt\midas\windows\msvc` to `PATH`)
```cmd
set PATH=%PATH%;C:\opt\midas\windows\msvc
mefa
```

## Installing on Windows from MinGW GCC binaries

1. Install [Git for Windows](https://gitforwindows.org/)
2. Install [Miniconda](https://www.anaconda.com/download/success)
```cmd
curl -O https://repo.anaconda.com/miniconda/Miniconda3-latest-Windows-x86_64.exe
mkdir C:\opt
Miniconda3-latest-Windows-x86_64.exe /S /D=C:\opt\miniconda
C:\opt\miniconda\condabin\conda.bat init
```
3. Setup Conda for MIDAS run
```cmd
conda config --add channels conda-forge
conda config --set channel_priority strict
conda create -n midas_mingw libgcc libgdal
conda activate midas_mingw
```
4. Download the source code and binaries
```cmd
cd \opt
git clone https://github.com/HuidaeCho/midas.git
```
5. Run MIDAS (add `C:\opt\midas\windows\mingw` to `PATH`)
```cmd
set PATH=%PATH%;C:\opt\midas\windows\mingw
mefa
```

## Building on Windows using MSVC

1. Install [Visual Studio Community Edition](https://visualstudio.microsoft.com/vs/community/). Select these two components:
   * MSVC v143 - VS 2022 C++ x64/x86 build tools (Latest)
   * Windows 11 SDK (10.0.26100.0)
2. Install [Git for Windows](https://gitforwindows.org/)
3. Install [Miniconda](https://www.anaconda.com/download/success)
```cmd
curl -O https://repo.anaconda.com/miniconda/Miniconda3-latest-Windows-x86_64.exe
mkdir C:\opt
Miniconda3-latest-Windows-x86_64.exe /S /D=C:\opt\miniconda
C:\opt\miniconda\condabin\conda.bat init
```
4. Start Developer Command Prompt for VS 2022
5. Setup Conda for MIDAS build
```cmd
conda config --add channels conda-forge
conda config --set channel_priority strict
conda create -n midas cmake libgdal
conda activate midas
```
6. Download the source code
```cmd
cd \opt
git clone https://github.com/HuidaeCho/midas.git
cd midas/src
```
7. Build MIDAS
```cmd
mkdir build
cd build
cmake .. > cmake.log 2>&1
msbuild midas.sln -p:Configuration=Release > msbuild.log 2>&1
```
8. Run MIDAS (add `C:\opt\midas\src\build\dist` to `PATH`)
```cmd
set PATH=%PATH%;C:\opt\midas\src\build\dist
mefa
```

## Building on Windows using MinGW GCC

1. Install [Git for Windows](https://gitforwindows.org/)
2. Install [Miniconda](https://www.anaconda.com/download/success)
```cmd
curl -O https://repo.anaconda.com/miniconda/Miniconda3-latest-Windows-x86_64.exe
mkdir C:\opt
Miniconda3-latest-Windows-x86_64.exe /S /D=C:\opt\miniconda
C:\opt\miniconda\condabin\conda.bat init
```
3. Setup Conda for MIDAS build
```cmd
conda config --add channels conda-forge
conda config --set channel_priority strict
conda create -n midas_mingw cmake make gcc binutils libgdal
conda activate midas_mingw
```
4. Download the source code
```cmd
cd \opt
git clone https://github.com/HuidaeCho/midas.git
cd midas/src
```
5. Build MIDAS
```cmd
mkdir build
cd build
cmake .. -G "MinGW Makefiles" > cmake.log 2>&1
make > make.log 2>&1
```
6. Run MIDAS (add `C:\opt\midas\src\build\dist` to `PATH`)
```cmd
set PATH=%PATH%;C:\opt\midas\src\build\dist
mefa
```

## Building on Linux

1. Install Miniconda
```bash
wget https://repo.anaconda.com/miniconda/Miniconda3-latest-Linux-x86_64.sh
chmod a+x Miniconda3-latest-Linux-x86_64.sh
mkdir ~/opt
./Miniconda3-latest-Linux-x86_64.sh -b -u -p ~/opt/miniconda
~/opt/miniconda/bin/conda init
. ~/.bashrc
```
2. Setup Conda for MIDAS build
```bash
conda config --add channels conda-forge
conda config --set channel_priority strict
conda create -n midas git cmake make gcc gdal
conda activate midas
```
3. Download the source code
```bash
cd ~/opt
git clone https://github.com/HuidaeCho/midas.git
cd midas/src
```
4. Build MIDAS
```bash
mkdir build
cd build

# LD_LIBRARY_PATH=$CONDA_PREFIX/lib and -DCMAKE_PREFIX_PATH=$CONDA_PREFIX to avoid system libraries
(
LD_LIBRARY_PATH=$CONDA_PREFIX/lib \
cmake .. \
  -DCMAKE_PREFIX_PATH=$CONDA_PREFIX \
  -DCMAKE_INSTALL_PREFIX=$HOME/usr/local &&
cmake --build . &&
cmake --install .
) &> build.log
```
5. Run MIDAS (add `$HOME/usr/local/bin` to `PATH`)
```bash
export PATH="$PATH:$HOME/usr/local/bin"
mefa
```

## Testing MSVC binaries on Windows using TX data

```dos
git clone https://github.com/HuidaeCho/midas.git
cd midas\windows\test
pretest.bat
test_msvc.bat
```

## Testing MinGW GCC binaries on Windows using TX data

```dos
git clone https://github.com/HuidaeCho/midas.git
cd midas\windows\test
pretest.bat
test_mingw.bat
```

## Benchmark results of MSVC vs. MinGW GCC vs. WSL GCC vs. Linux GCC binaries using CONUS data

System specifications
* System model and OS
  * ThinkPad X1 Carbon Gen 11
    * Windows 11 for MSVC and MinGW
    * WSL Linux 5.15.167.4-microsoft-standard-WSL2 for WSL GCC
  * ThinkPad X1 Yoga Gen 8 (identical hardware specifications)
    * Slackware CURRENT Linux 6.12.17 for Linux GCC
* CPU: Intel Core i7-1370P @ 5.20 GHz
* Cores: 14
* Logical processors: 20
* Memory: 64 GiB

MSVC
```
> melfp.exe inputs\fdr.tif inputs\outlets515152.shp cat msvc.gpkg lfpid -c msvc.csv
No output vector layers specified; Not creating msvc.gpkg
Using 20 threads...
Reading flow direction raster <inputs\fdr.tif>...
Input time for flow direction: 9198000 microsec
Reading outlets <inputs\outlets515152.shp>...
Input time for outlets: 17007000 microsec
Number of cells: 14998630400
Number of outlets: 515152
Tracing stack size for loop-then-task: 3072
Finding longest flow paths...
Computation time for longest flow paths: 622248000 microsec
Number of longest flow paths found: 521946
Writing longest flow path head coordinates <msvc.csv>...
Output time for longest flow path head coordinates: 1980000 microsec
Total elapsed time: 657635000 microsec
```
* Computation time: 622.248000 sec
* I/O time: 35.387000 sec
* Total time: 657.635000 sec

MinGW GCC
```
> melfp.exe inputs\fdr.tif inputs\outlets515152.shp cat mingw.gpkg lfpid -c mingw.csv
No output vector layers specified; Not creating mingw.gpkg
Using 20 threads...
Reading flow direction raster <inputs\fdr.tif>...
Input time for flow direction: 6823994 microsec
Reading outlets <inputs\outlets515152.shp>...
Input time for outlets: 16805443 microsec
Number of cells: 14998630400
Number of outlets: 515152
Tracing stack size for loop-then-task: 3072
Finding longest flow paths...
Computation time for longest flow paths: 120324302 microsec
Number of longest flow paths found: 521946
Writing longest flow path head coordinates <mingw.csv>...
Output time for longest flow path head coordinates: 2097205 microsec
Total elapsed time: 153459228 microsec
```
* Computation time: 120.324302 sec
* I/O time: 33.13493 sec
* Total time: 153.459228 sec

WSL GCC
```
$ melfp inputs/fdr.tif inputs/outlets515152.shp cat wsl.gpkg lfpid -c wsl.csv
No output vector layers specified; Not creating wsl.gpkg
Using 20 threads...
Reading flow direction raster <inputs/fdr.tif>...
Input time for flow direction: 7537860 microsec
Reading outlets <inputs/outlets515152.shp>...
Input time for outlets: 223358 microsec
Number of cells: 14998630400
Number of outlets: 515152
Tracing stack size for loop-then-task: 3072
Finding longest flow paths...
Computation time for longest flow paths: 38734223 microsec
Number of longest flow paths found: 521946
Writing longest flow path head coordinates <wsl.csv>...
Output time for longest flow path head coordinates: 426007 microsec
Total elapsed time: 47034415 microsec
```
* Computation time: 38.734223 sec
* I/O time: 8.300192 sec
* Total time: 47.034415 sec

Linux GCC
```
$ melfp inputs/fdr.tif inputs/outlets515152.shp cat linux.gpkg lfpid -c linux.csv
No output vector layers specified; Not creating linux.gpkg
Using 20 threads...
Reading flow direction raster <inputs/fdr.tif>...
Input time for flow direction: 1310629 microsec
Reading outlets <inputs/outlets515152.shp>...
Input time for outlets: 174350 microsec
Number of cells: 14998630400
Number of outlets: 515152
Tracing stack size for loop-then-task: 3072
Finding longest flow paths...
Computation time for longest flow paths: 21766454 microsec
Number of longest flow paths found: 521946
Writing longest flow path head coordinates <linux.csv>...
Output time for longest flow path head coordinates: 398142 microsec
Total elapsed time: 24136021 microsec
```
* Computation time: 21.766454 sec
* I/O time: 2.369567 sec
* Total time: 24.136021 sec

Computation time ranking
1. Linux GCC: fastest baseline
2. WSL GCC: 1.78x slower than Linux GCC
3. MinGW GCC: 5.53x slower than Linux GCC
4. MSVC: 28.59x slower than Linux GCC

Based on these results, Linux offers the best performance and should be used if possible. If you must use Windows, prefer WSL for better performance. If WSL is not available, MinGW GCC binaries provide the most reasonable performance among native Windows options.

## Benchmark results

### MEFA

![MEFA Graphical Abstract](https://idea.isnew.info/publications/Memory-efficient%20flow%20accumulation%20using%20a%20look-around%20approach%20and%20its%20OpenMP%20parallelization%20-%20Graphical%20abstract.png)

System specifications
* CPU: Intel Core i9-12900 @ 2.40GHz
* Cores: 16
* Logical processors: 24
* Memory: 64 GiB
* OS: Windows 11

Citation
* Huidae Cho, July 2023. Memory-Efficient Flow Accumulation Using a Look-Aroun d Approach and Its OpenMP Parallelization. Environmental Modelling & Software 167, 105771. [doi:10.1016/j.envsoft.2023.105771](https://doi.org/10.1016/j.envsoft.2023.105771).

### MESHED

![image](https://clawrim.isnew.info/wp-content/uploads/2024/07/meshed-flyer.png)

### MELFP

![image](https://github.com/HuidaeCho/midas/assets/7456117/0c1fdd15-b900-407c-b89b-503a6c0b2dc2)

### MEFLEN

![image](https://github.com/HuidaeCho/midas/assets/7456117/3a67d9ec-8649-4d24-97a7-b5d3dd5115c7)

System specifications
* CPU: Intel Core i9-12900 @ 2.40GHz
* Cores: 16
* Logical processors: 24
* Memory: 64 GiB
* OS: Windows 11
  * Downstream flow length (computation time only)
    * Texas results
      * 8.2 s (24 threads)
      * 19.6 s (1 thread)
    * CONUS results
      * 4.7 m (24 threads)
      * 7.3 m (1 thread)
* OS: Linux 6.6.30
  * Downstream flow length (computation time only)
    * Texas results
      * 9.4 s (24 threads)
      * 20.0 s (1 thread)
    * CONUS results
      * 2.0 m (24 threads)
      * 4.0 m (1 thread)
