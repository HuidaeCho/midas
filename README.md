# Memory-Efficient I/O-Improved Drainage Analysis System (MIDAS)

Predefined flow direction encodings in GeoTIFF: power2 (default, r.terraflow, ArcGIS), taudem (d8flowdir), 45degree (r.watershed), degree<br>
![image](https://github.com/user-attachments/assets/990f0530-fded-4ee5-bfbb-85056a50ca1c)
![image](https://github.com/user-attachments/assets/a02dfc15-a825-4210-82c4-4c9296dafadc)
![image](https://github.com/user-attachments/assets/64f5c65a-c7cc-4e06-a69f-6fccd6435426)
![image](https://github.com/user-attachments/assets/fafef436-a5f2-464a-89a8-9f50a877932c)

Custom flow direction encoding is also possible by passing `-e E,SE,S,SW,W,NW,N,NE` (e.g., 1,8,7,6,5,4,3,2 for taudem).

## Binaries for Windows

* [MEFA](https://github.com/HuidaeCho/mefa) (Memory-Efficient Flow Accumulation)
* [MESHED](https://github.com/HuidaeCho/meshed) (Memory-Efficient Watershed Delineation)
* [MELFP](https://github.com/HuidaeCho/melfp) (Memory-Efficient Longest Flow Path)
* MEFLEN (Memory-Efficient Flow Length)

## Testing

```dos
git clone https://github.com/HuidaeCho/midas.git
cd midas\windows\test
pretest.bat
test.bat
```

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
  * Downstream flow length (compute time only)
    * Texas results
      * 8.2 s (24 threads)
      * 19.6 s (1 thread)
    * CONUS results
      * 4.7 m (24 threads)
      * 7.3 m (1 thread)
* OS: Linux 6.6.30
  * Downstream flow length (compute time only)
    * Texas results
      * 9.4 s (24 threads)
      * 20.0 s (1 thread)
    * CONUS results
      * 2.0 m (24 threads)
      * 4.0 m (1 thread)
