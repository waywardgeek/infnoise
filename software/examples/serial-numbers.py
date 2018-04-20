#!/usr/bin/python

import subprocess
import argparse

parser = argparse.ArgumentParser(description='Generate random serial numbers')
parser.add_argument('-l', dest='length', action='store', default=8,
                    help='length of each serial number', type=int)
parser.add_argument('-c', dest='count', action='store', default=10,
                    help='length of each serial number', type=int)

def main():
    args = parser.parse_args()                                                              # parse arguments
    unique_serials_generated = 0                                                            # counter for serials inserted successfully.
    while unique_serials_generated < args.count:                                            # loop until enough unique serials are created (because some could fail)
        serials_list = readHexStrings(args.count - unique_serials_generated, args.length)   # read $count hex-strings of fixed $length
        for serial in serials_list:
            if ("\n"  in serial) or (" " in serial):
                continue    # dont use serials with newlines or spaces
	    unique_serials_generated += 1                                                   # counter for serials
            serial = serial.upper()
            print(serial)

def readHexStrings(count, length):
    list = []
    infnoise_proc = subprocess.Popen(["infnoise"], stdout=subprocess.PIPE)              # run infinite noise driver and
    bin2hex_proc = subprocess.Popen(["infnoise-bin2hex"], stdin=infnoise_proc.stdout,   # feed its stdout to the bin2hex utility,
                                    stdout=subprocess.PIPE)                             # you could also skip this and do the bin2hex conversion in python
    while len(list) < count and infnoise_proc.poll() == None:
        list.append(bin2hex_proc.stdout.read(length))
    infnoise_proc.terminate()
    bin2hex_proc.terminate()
    return list

if __name__ == '__main__':
    main()
