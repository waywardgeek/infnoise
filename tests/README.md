# Entropy tests for the Infinite Noise TRNG

## Test results

These test results were produced with 250MiB 
(2.000.000.000 bits) of random numbers from the device,
resulting in 100.000 FIPS blocks with 20.000 bit each - and took two weeks.

### Output speed
|Multiplier/Mode|raw|whitened (SHA-3)|/dev/random|
|----|----|----|----|
|               0 | 50 KiB/s | 42,5 KiB/s | 0,3  KiB/s  |
|               1 |    -     | 25   KiB/s | 14,0 KiB/s  |
|              10 |    -     | 250  KiB/s | 23   KiB/s  |
|             100 |    -     | 2,25 MiB/s | 20,9 KiB/s  |
|            1000 |    -     | 17,2 MiB/s | 14,0 KiB/s  |
|           10000 |    -     | 68,3 MiB/s | 4,21 KiB/s  |

(*) with an Intel i7-4558U CPU @ 2.80GHz.

### FIPS Tests

| Multiplier/Mode | raw  | failed blocks (SHA-3) | failed blocks /dev/random |
|-----------------|------|--------------|--------------|
|               0 | 9999 | 70 (0,07 %)  | 81 (0,08 %)  |
|               1 |   -  | 78 (0,08 %)  | 73 (0,07 %)  |
|              10 |   -  | 87 (0,09 %)  | 89 (0,09 %)  |
|             100 |   -  | 87 (0,09 %)  | 95 (0,1 %)    |
|            1000 |   -  | 78  (0,08 %)  | 73 (0,07 %)   |
|           10000 |   -  | 74  (0,07 %)  | 86  (0,09 %)  |

* The percentage of failed FIPS blocks should always remain below 0,1%. 

## Run your own tests

### Requirements

Make sure you have the following tools installed:

- rng-tools
- ent
- dieharder
- pv
- python (numpy, matplotlib)
- infnoise driver


In the header of the script, you can define the test parameters:
```
declare -a TEST_KBYTES=('25000')
declare -a TEST_MULTIPLIERS=('0' '1' '10' '100' '1000' '10000')
```
A full test run like this with 25MiB takes almost 2 days, mostly because of /dev/random 
tests with a multiplier of zero.

This is why by default it performs a reduced run which takes only 1-2 hours.

Tests are run for each combination of these parameters and can be started by running runtests.sh:
```
$ git clone https://github.com/manuel-domke/infnoise
$ cd tests
$ sudo ./runtests.sh
```
The output directories will be created automatically.

## Files and Directories:
Directory structure created by a test run:

	- results: 
	  - <testcase>-<multiplier>-<kbytes>-dieharder.txt
	  - <testcase>-<multiplier>-<kbytes>-ent.txt
	  - <testcase>-<multiplier>-<kbytes>-rngtest.txt
	  - plots:
	    - <testcase>-<multiplier>-<kbytes>K.out-colormap.png: 	square distribution
	    - <testcase>-<multiplier>-<kbytes>K.out-scatterplot.png:	scatter plots
	  - data:
	    - <testcase>-<multiplier>-<kbytes>K.log: 			log output of the infnoise utility
	    - <testcase>-<multiplier>-<kbytes>K-pv.log: 		log output of pv
	    - <testcase>-<multiplier>-<kbytes>K.out:			random data produced in the test run
