# AlphaMove
Solution to 2020 CAD contest Problem B: Routing with Cell Movement

## Usage
Clone the repo and make to build the executable.
Linux (Ubuntu version 16.04 LTS):
```
make linux16
make
```
Linux (Ubuntu version 18.04 LTS or 20.04 LTS):
```
make linux18
make
```
macOS:
```
make mac
make
```
```
./cell_move_router <input.txt> <output.txt>
```

## Access Order
gridList[row][col][lay]

## Team members
NTUEE 張凱鈞 范育瑋 馬健凱

## Special thanks to
Chung-Yang (Ric) Huang [ric2k1](https://github.com/ric2k1), who provided the cmdParser

## Alpha Test Report (Submitted on 2020/6/14, Received on 2020/06/30)
| Score    | case1 |          | case2 |                   | case3 |          | case4 |          | case5 |          | case3B |          | case4B |          | case5B |          | Sum | Comments                                                                 |
| -------- | ----- | -------- | ----- | ----------------- | ----- | -------- | ----- | -------- | ----- | -------- | ------ | -------- | ------ | -------- | ------ | -------- | --- | ------------------------------------------------------------------------ |
| cada0032 | 0     | net open | 0     | moving fixed cell | 0     | net open | 0     | net open | 0     | net open | 0      | net open | 0      | net open | 0      | net open | 0   | Output file format is incorrect in all cases, please check your program. |

## Beta Test Report (Submitted on 2020/07/22, Received on 2020/08/03)
| team     | case1 | case2 | case3 | case4 | case5 | case6 | case3B | case4B | case5B | case6B | Sum | Comments                                             |
| -------- | ----- | ----- | ----- | ----- | ----- | ----- | ------ | ------ | ------ | ------ | --- | ---------------------------------------------------- |
| cada0032 | 22    | 5     | 0     | 0     | 0     | 0     | 0      | 0      | 0      | 0      | 27  | Please try simple algorithms to improve net lengths. | 

## Final Report (Submitted on 2020/08/26, Received on 2020/09/21)
| team | case1 |     | case2 |     | case3 |     | case4 |     | case5 |     | case6 |     | case3B |     | case4B | | case5B |     | case6B |     | Sum   |
| -------- | ----- | --- | ----- | --- | ----- | --- | ------ | --- | ------ | --- | ------ | --- | ------ | --- | ------ | -------------- | ------ | --- | ------ | --- | ------- |
| cada0032 | 22    |     | 10    |     | 6731  |     | 833455 |     | 166903 |     | 482826 |     | 7438   |     | 0      | gGrid overFlow | 168236 |     | 421185 |     | 2086806 |
