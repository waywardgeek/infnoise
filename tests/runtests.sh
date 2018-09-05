#!/bin/bash

service infnoise stop
service rng-tools stop

echo "4096" > /proc/sys/kernel/random/write_wakeup_threshold

declare -a TEST_KBYTES=('2500')
declare -a TEST_MULTIPLIERS=('1' '10' '100' '1000' '10000') # be careful with multiplier of 0 - tests with /dev/random take over a day.

RNGTEST=1
ENT=1
DIEHARDER=0
GRAPHS=1

function checkCommand {
	$1 $2 >/dev/null
	if [ $? -ne 0 ]; then
		echo "$1 not installed. Exiting.."
		exit 1
	fi
}

function checkPythonModule {
	python -c "import $1" >/dev/null
	if [ $? -ne 0 ]; then
		echo "python module $1 not installed. Exiting.."
		exit 1
	fi
}

function makePlots {
		python plots/colormap.py $1 &
		python plots/scatterplot.py $1 &
		sleep 3
		mv $1-colormap.png results/plots/
		mv $1-scatter.png results/plots/
}

checkCommand "pv" "-h"
checkCommand "infnoise" "-h"
if [ $RNGTEST -eq 1 ] ; then
	checkCommand "rngtest" "--help"
fi
if [ $ENT -eq 1 ] ; then
	checkCommand "ent" "-u"
fi
if [ $DIEHARDER -eq 1 ] ; then
	checkCommand "dieharder"
fi
if [ $GRAPHS -eq 1 ] ; then
	checkCommand "python" "-h"
	checkPythonModule "numpy"
	checkPythonModule "matplotlib"
fi

mkdir -p results/data
mkdir -p results/plots

for kbytes in "${TEST_KBYTES[@]}"
do
        # raw stream tests
	echo "starting RAW output tests with:"
	echo "Bytes: $kbytes K ($(($kbytes*8000)) bits)"
        infnoise --debug --raw 2> results/data/raw-$kbytes\K.log | pv -fW 2>results/data/raw-$kbytes\K-pv.log | dd count=$kbytes bs=1000 iflag=fullblock of=results/data/raw-$kbytes\K.out 2>/dev/null
	if [ $RNGTEST -eq 1 ] ; then
		rngtest < results/data/raw-$kbytes\K.out 2> results/raw-$kbytes\K-rngtest.txt
	fi
	if [ $ENT -eq 1 ] ; then
		ent results/data/raw-$kbytes\K.out > results/raw-$kbytes\K-ent.txt
	fi
	if [ $GRAPHS -eq 1 ] ; then
		makePlots results/data/raw-$kbytes\K.out
	fi
	if [ $DIEHARDER -eq 1 ] ; then
		dieharder -a -f results/data/raw-$kbytes\K.out > results/raw-$kbytes\K-dieharder.txt
	fi
	echo "---"

	for multiplier in "${TEST_MULTIPLIERS[@]}"
	do
		# whitened stream tests
		echo "starting whitened tests with: "
		echo "Bytes: $kbytes K ($(($kbytes*8000)) bits) multiplier: $multiplier"
		infnoise --debug --multiplier $multiplier 2> results/data/whitened-$multiplier-$kbytes\K.log | pv -fW 2> results/data/whitened-$multiplier-$kbytes\K-pv.log | dd count=$kbytes bs=1000 iflag=fullblock of=results/data/whitened-$multiplier-$kbytes\K.out 2>/dev/null
		if [ $RNGTEST -eq 1 ] ; then
			rngtest < results/data/whitened-$multiplier-$kbytes\K.out 2> results/whitened-$multiplier-$kbytes\K-rngtest.txt
		fi
		if [ $ENT -eq 1 ] ; then
			ent results/data/whitened-$multiplier-$kbytes\K.out > results/whitened-$multiplier-$kbytes\K-ent.txt
		fi
		if [ $GRAPHS -eq 1 ] ; then
			makePlots results/data/whitened-$multiplier-$kbytes\K.out
		fi
		if [ $DIEHARDER -eq 1 ] ; then
			dieharder -a -f results/data/whitened-$multiplier-$kbytes\K.out > results/whitened-$multiplier-$kbytes\K-dieharder.txt
		fi
		echo "---"

		# /dev/random tests
		echo "starting /dev/random tests with: "
		echo "Bytes: $kbytes K ($(($kbytes*8000)) bits) Multiplier: $multiplier"
		timeout 3s cat /dev/random >/dev/null # flush /dev/random pool
		echo -n "available entropy before test: "
		cat /proc/sys/kernel/random/entropy_avail
		infnoise --dev-random --multiplier $multiplier --debug 2> results/data/devrandom-$multiplier-$kbytes\K.log &
		sleep 3
		cat /dev/random | pv -fW 2> results/data/devrandom-$multiplier-$kbytes\K-pv.log | dd count=$kbytes bs=1000 iflag=fullblock of=results/data/devrandom-$multiplier-$kbytes\K.out 2>/dev/null
		killall infnoise
		if [ $RNGTEST -eq 1 ] ; then
			rngtest < results/data/devrandom-$multiplier-$kbytes\K.out 2> results/devrandom-$multiplier-$kbytes\K-rngtest.txt
		fi
		if [ $ENT -eq 1 ] ; then
			ent results/data/devrandom-$multiplier-$kbytes\K.out > results/devrandom-$multiplier-$kbytes\K-ent.txt
		fi
		if [ $GRAPHS -eq 1 ] ; then
			makePlots results/data/devrandom-$multiplier-$kbytes\K.out
		fi
		if [ $DIEHARDER -eq 1 ] ; then
			dieharder -a -f results/data/devrandom-$multiplier-$kbytes\K.out > results/devrandom-$multiplier-$kbytes\K-dieharder.txt
		fi
		echo "---"
	done
done
