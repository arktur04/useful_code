"This program is designed to test disk read/write performance using the mmap function."

OS
---
macOS

Operation
---------
The program sequentially writes files of sizes: 100, 512, 1024, 2048, 4096, 8192 MB and measures time of writing and reading the file, using **mmap**.

Usage
-----
Usage: ./mmap_speed_test [-s=0|1] [-n=0|1] [-h]
  -s=0     Use MS_ASYNC
  -s=1     Use MS_SYNC (default)
  -n=0     F_NOCACHE=0
  -n=1     F_NOCACHE=1 (default)
  -h       Show the help message

Expected results
----------------
On MacBook Pro 2017, the following results were acquired:
(with flags MS_SYNC and MS_ASYNC appropriately)

Results 
-------
1. MS_SYNC, MAP_SHARED, F_NOCACHE=1

| Size, MB | Write+msync, MB/s | Read, MB/s |
| ----------- | ----------- |----|
| 100 | 513.503 | 1614.47 |
| 512 | 591.876 | 1550.13 |
| 1024 | 611.296 | 1559.88 |
| 2048 | 569.909 | 1411.49 |
| 4096 | 180.094 | 1317 |
| 8192 | 146.129 | 272.321 |

2. MS_ASYNC, MAP_SHARED, F_NOCACHE=1

| Size, MB | Write+msync, MB/s | Read, MB/s |
| ----------- | ----------- |----|
| 100 | 483.286 | 1834.57 |
| 512 | 483.147 | 1493.21 |
| 1024 | 539.159 | 1379.06 |
| 2048 | 421.915 | 1423.98 |
| 4096 | 155.11 | 1334.19 |
| 8192 | 145.226 | 277.303 |
