ram_speed_test
===============	

This program is intended for testing memory performance.

OS
--
macOS

Command-line Arguments
----------------------

-jN         Number of threads to run concurrently (default: 1)  
-b=N[KMG]   Buffer size with optional unit (K, M, or G). Default is 1G  
-nN         Number of iterations to perform (default: 10)
Examples:

./ram_speed_test -j4 -b=4G -n5

Runs 4 threads with a 4GB buffer each, and performs 5 iterations.

./ram_speed_test

Runs 1 thread with a 1GB buffer, and performs 10 iterations (default values).

How to Build
------------
Create a build folder in the source directory and enter it:
```
mkdir build
cd build
```
Compile:
```
cmake ..
make
```
After compilation, the ram_speed_test binary will appear in the build folder.

To run it:
```
./ram_speed_test
```
How to Run
If the file doesn't execute, you may need to change its permissions:

```
chmod +x ./ram_speed_test
xattr -d com.apple.quarantine ./ram_speed_test
```
Results
On average, the test shows speeds around 10–12 GB/s.

Example Results:
MacBook Pro 2017 (16GB RAM):

Total Write Speed: 9532.09 MB/s

Total Read Speed: 9547.33 MB/s

MacBook Pro 2019 (16GB RAM):

Total Write Speed: 12941.4 MB/s

Total Read Speed: 12746.9 MB/s

Notes:
Results may be lower if:

The buffer size is too large and approaches total system memory (e.g., 10GB on a 16GB system).

Many threads are running simultaneously and compete for memory bandwidth. Peak performance is often seen with a single thread.

Results may be inflated if the buffer size is too small, since the CPU caches (L1–L3) can significantly affect the results.

Cache Information:
- L1 Cache (per core):

Data: 32 KB

Instruction: 32 KB

- L2  Cache (per core):

256 KB

- L3 Cache (shared between cores):

4 MB for 2-core CPUs (e.g., i5-7360U)

6 MB for 4-core CPUs (e.g., i7-7660U or i7-7700HQ)