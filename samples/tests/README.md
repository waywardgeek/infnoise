# Entropy tests for the Infinite Noise TRNG

## Test results

These test results were produced with 25MiB 
(200.000.000 bits) of random numbers from the device,
resulting in 10.000 FIPS blocks with 20.000 bit each.

### Output speed
+-----------------+----------+------------+-------------+
| Multiplier/Mode |   raw    | whitened   | /dev/random |
+-----------------+----------+------------+-------------+
|               0 | 50 KiB/s | 42,5 KiB/s | 0,3  KiB/s  |
|               1 |    -     | 25   KiB/s | 14,0 KiB/s  |
|              10 |    -     | 250  KiB/s | 23   KiB/s  |
|             100 |    -     | 2,25 MiB/s | 20,9 KiB/s  |
|            1000 |    -     | 17,2 MiB/s | 14,0 KiB/s  |
|           10000 |    -     | 68,3 MiB/s | 4,21 KiB/s  |
+-----------------+----------+------------+-------------+
* CPU: Intel 

### FIPS Tests
+-----------------+------+----------+-------------+------------------+----------------------+
| Multiplier/Mode | raw  | whitened | /dev/random | Whitened (%fail) | /dev/random (% fail) |
+-----------------+------+----------+-------------+------------------+----------------------+
|               0 | 9999 |        6 |           6 |             0,06 |                 0,06 |
|               1 |   -  |        6 |          13 |             0,06 |                 0,13 |
|              10 |   -  |        6 |           8 |             0,06 |                 0,08 |
|             100 |   -  |        0 |           6 |                0 |                 0,06 |
|            1000 |   -  |        6 |          13 |             0,06 |                 0,13 |
|           10000 |   -  |       14 |           8 |             0,14 |                 0,08 |
+-----------------+------+----------+-------------+------------------+----------------------+

* The percentage of failed FIPS blocks should always remain at < 0,2%. 
While the Infinite Noise TRNG driver is active, it ensures the entropy is within 
the expected levels, so once you've confirmed your device works within its specs,
it won't change its behaviour. 
Its been designed to stop output if unexpected behaviour occurs.

## Run your own tests

### Requirements

Make sure you have the following tools installed:

- rng-tools
- ent
- dieharder
- pv
- python (numpy, matplotlib)
- infnoise driver

The tests can be run with the script "runtests.sh" - as root! 
The output directory will be created automatically.

In the header of the script, you can define the test parameters:

declare -a TEST_KBYTES=('25000')
declare -a TEST_MULTIPLIERS=('0' '1' '10' '100' '1000' '10000')

Tests are run for each combination of these parameters. 

A full test run takes almost 2 days, this is only due to /dev/random 
tests with multiplier 0. Thats why this test is disabled by default.

The reduced run, as configured by default takes only 1-2 hours.

## Files and Directories:
	- results: 
	  - <testcase>-<multiplier>-<kbytes>-dieharder.txt
	  - <testcase>-<multiplier>-<kbytes>-ent.txt
	  - <testcase>-<multiplier>-<kbytes>-rngtest.txt
	  - data:
	    - <testcase>-<multiplier>-<kbytes>K.log: 			log output of the infnoise utility
	    - <testcase>-<multiplier>-<kbytes>K-pv.log: 		log output of pv
	    - <testcase>-<multiplier>-<kbytes>K.out:			random data produced in the test run
	    - <testcase>-<multiplier>-<kbytes>K.out-colormap.png: 	square distribution
	    - <testcase>-<multiplier>-<kbytes>K.out-scatterplot.png:	scatter plots
