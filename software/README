Compiling the Driver for Windows
--------------------------------

I compiled infnoise-win.exe using VisualStudio 2013 using Windows 7.  I downloaded the
FTD2xx drivers from FTDI.  Pipes seem almost entirely broken in Windows, so the Windows
version requires out output file to be specified on the command line.  In a cmd window,
you can type

    infnoise-win foo

and let it run for a while until you have as much random data in foo as you need.

The VisualStudio project for infnoise is in the infnoise/software/VisualStudio directory.

Using prebuilt packages for Linux
---------------------------------

Precompiled binaries can be downloaded from the releases section of the 13-37-org fork:
https://github.com/13-37-org/infnoise/releases

All packages are signed with the same PGP-Key (Key-ID: 0x4E730A3C) used for the repositories below. 
Full Fingerprint: 71AE 099B 262D C0B4 93E6 EE71 975D C25C 4E73 0A3C. You can also check the 
fingerprints at 13-37.org/pgp-keys and in the Crowd Supply campaign.

Repositories for Ubuntu, Debian and Raspbian are also available. To add them follow this procedure:

    $ wget -O 13-37.org-code.asc https://13-37.org/files/pubkey.gpg 
    
    Verify the keys fingerprint:
    
    # GPG1
    $ gpg --with-fingerprints 13-37.org-code.asc
    # GPG2:
    $ gpg2 --import-options import-show --dry-run --import < 13-37.org-code.asc
    
    $ sudo apt-key add 13-37.org-code.gpg
    
Available for Ubuntu and Debian (x86, x64 and armhf):

    $ echo "deb http://repo.13-37.org/ stable main" | sudo tee /etc/apt/sources.list.d/infnoise.list
    $ sudo apt-get update
    $ sudo apt-get install infnoise

Connect the Infinite Noise TRNG (if not already) and the service will be started via a udev rule.
Check status of driver:
    $ systemctl status infnoise

Compiling the Driver for Linux
------------------------------

The infnoise application reads random data from the Infinite Noise USB key and writes
binary data to stdout.  To compile it, you will need to install the libftdi and libusb
development libraries to compile infnoise.  In Ubuntu (probably also Debian), you can use
this command:

    $ sudo apt-get install libftdi-dev libusb-dev

These include an open source drivers for the FT240X USB chip used on the Infinite Noise
TRNG.  Once this is done, to compile the infnoise program, simply make it:

    $ make

To run the infnoise application, make sure the Infinite Noise USB stick is
plugged in, and from a shell, type:

    $ sudo ./infnoise > randbytes

The Infinite Noise USB driver uses the open source FTDI driver documented at:

    http://www.intra2net.com/en/developer/libftdi/documentation/group__libftdi.html

Note that there is a newer alpha version of the next release of the libftdi library.  I
found it runs much slower than the current libftdi1 library in Ubuntu, so I am sticking
with the stable release for now.

Usage
-----

Usage: infnoise [options]
Options are:
    --debug - turn on some debug output
    --dev-random - write entropy to /dev/random instead of stdout
    --raw - do not whiten the output
    --multiplier <value> - write 256 bits * value for each 512 bits written to the Keccak sponge
    --no-output - do not write random output data
    --daemon - run in the background. Output should be redirected to a file or
    the options should be used with --dev-random. To reduce CPU-usage addition
    af entropy is only forced after a minute rather than a second.
    --pidfile <filename> - write the process ID to a file. If --daemon is used, it is the ID of the background process.
    --serial <serial> - use Infinite Noise TRNG/FT240 with the given serial number (see --list-devices)
    --list-devices - list available devices

Note: The options --daemon and --pidfile are only implemented in the Linux version.

Examples
--------

This will fill the file randbytes with random data endlessly, so hit Ctrl+C to kill it
after a while.  If all you want to do is verify the output using the dieharder tests, you
can use:

    $ sudo ./infnoise | dieharder -g 200 -a

The program "infnoise" talks to the FT240X USB 2.0 interface chip on the USB stick.  It
uses "bitbang" mode to drive the clock signals of the Infinite Noise Multiplier, and
receives one random bit of output per byte written to the device.  These bits are
collected into bytes, and sent through a "health checker", which verifies that the bits
look basically like INM output, with about the expected level of entropy.

If the measured level of entropy deviates from the theoretical value by more than 2%, then
the infnoise application stops generating outputdata.  If it sees too many 0's or 1's in a
row, it will exit with an error code.

You can see the raw data from the INM for yourself by running

    $ sudo infnoise --raw > randbytes

Kill it after a while, and check it out with with a program like hexdump.  In general,
there should be random 0's and 1's, but rarely more than 3 1's or 0's in a row.  You can
get some good debug info using:

    $ infnoise --debug --no-output
    Generated 1048576 bits.  OK to use data.  Estimated entropy per bit: 0.871889, estimated K: 1.830057
    num1s:49.541189%, even misfires:0.137931%, odd misfires:0.137931%
    Generated 2097152 bits.  OK to use data.  Estimated entropy per bit: 0.869644, estimated K: 1.827211
    num1s:49.633114%, even misfires:0.131231%, odd misfires:0.145229%
    Generated 3145728 bits.  OK to use data.  Estimated entropy per bit: 0.867817, estimated K: 1.824899
    num1s:49.556943%, even misfires:0.133887%, odd misfires:0.133887%
    Generated 4194304 bits.  OK to use data.  Estimated entropy per bit: 0.867596, estimated K: 1.824620
    num1s:49.570250%, even misfires:0.160155%, odd misfires:0.144005%
    Generated 5242880 bits.  OK to use data.  Estimated entropy per bit: 0.867907, estimated K: 1.825014
    num1s:49.566724%, even misfires:0.125936%, odd misfires:0.130600%

This prints some basic stats after every 2^20 (about 1 million) bits generated.  They are
run through the health checker which reports "OK to use data" because the measured entropy
is within 2% of the exepcted value.  Estimated K is the gain of the op-amp stage that we
expect to result in the measured level of entropy.  For this design, 1.82 is the design
target, set by a 10K Ohm resistor and an 8.2K Ohm resistor.  Generally, estimated entropy
is slightly higher than the design target due slight deviation from ideal operation.  The
largest source of this non-ideal operation is what I call "misfires", where the output of
the comparator changes in a clock cycle when the inputs are held steady.  The rate of
missfires is reported for even and odd bits, because there is an even and odd comparator
generating bits on alternate cycles, and the two circuits can behave slightly differently.
The number of 1's should ideally be 50%, but due primarily to resistor ratio
imperfections, the actual range is from about 49.5% to 50.5%.

If you need provably random data, which may be comforting in applications such as
one-time-pads, you can compress the bits from the Infinite Noise TRNG 2-to-1 through the
Keccack sponge, cryptographically whitening while reducing the non-randomness to levels
that cannot ever be detected using:

    $ infnoise --multiplier 1 > randbytes

This will take twice as long by default, but every 512 bits are tested to insure they are
not more likely to occur than 1 in 2^433.  With --multiplier 1, only 256 bits are read
from the sponge for each 512 put in.

For most cryptograpy applications, such as generating keys for RSA, we can count on the
cryptographic strength of Keccack-1600 to produce cryptographically undetectably
non-random data when securely seeded with the Infinite Noise TRNG.  The Keccak sponge is
seeded with 512 bits from the Infinite Noise TRNG every time, where the data is tested for
at least 433 bits of estimated entropy.  However, but the amount of data to squeeze from
the sponge is user selectable.  If you want 100MiB/second of Keccak (SHA3) data that is
cryptographically reseeded every 2 megabytes, you can use:

    $ sudo ./infnoise --multiplier 65536 | yourApplication

For even higher seeded CPRNG data rates, bug me and I'll include the "optimized" version
of Keccack, rather than the reference version, or possibly Blake2b which is even faster
than Keccak.

Running as a Deamon
-------------------

Some people need to feed random bits into /dev/random.  This simplest way, and
the way I do it when generating keys, is:

    $ sudo ./infnoise --dev-random --daemon

This tells infnoise to run as a daemon, writing random data to /dev/random.  It
writes 512 bits at once, containing over 400 bits of entropy.  This randomizes
the state all at once.  If the entropy pool is "full" (above the watermark read
from /proc/sys/kernel/random/poolsize), it waits for up to 1 second for it to
fall below the watermark, and after that writes another 512 bits anyway.

This means it runs at full speed when needed, and when not, it randomizes
/dev/random once per second anyway.

Sample init scripts are provided in `software/init_scripts` as a starting point
for setting up system services using this approach.

The other approach people use is rngd.  I had a poor experience with this
program, so I do not use it.  In Ubuntu 14.04, I found that when I told it to
use hardware sources, it also enabled the RDRAND instruction by default, which
is so fast, it never read from any other source!  Rngd also has some minor
issues that are less severe, but these issues in rngd are enough that I shy
away from it.

That said, a lot of people use it, and feel free to follow them.  To do this
you need to create a "named fifo" using the mkfifo command.  Then, you can run
infnoise in the background, writing to the fifo.  Rngd has flags for reading
from a file which will work for this case.  Be careful to also disable RDRAND,
or you will only get Intel's numbers.

Udev rules
----------

This is thanks to user Abigail on github.  If you want to automatically feed
random data into /dev/random when the TRNG is plugged in, you can ask Linux to
do this by creating a file in etc/udev/rules.d. 
It relies on the systemd service "infnoise.service" provided under init_scripts, as udev is not designed to start long-running processes. 

SUBSYSTEM=="tty", ATTRS{idVendor}=="0403", ATTRS{idProduct}=="6015", SYMLINK+="infnoise" 
ACTION=="add", SUBSYSTEM=="usb", ATTRS{idVendor}=="0403", ATTRS{idProduct}=="6015" ,TAG+="systemd", ENV{SYSTEMD_WANTS}="infnoise.service"

This also adds a symlink so the device removal can also be reacted on.

I personally run the infnoise tool by hand from a bash shell, typically to test devices like this:

$ sudo ./infnoise --debug --no-output

To avoid having to type 'sudo' each time, I created the following udev rules,
which worked on my particular Ubuntu 14.04 based laptop:

$ cat 30-infnoise.rules
SUBSYSTEM=="usb", ATTRS{idProduct}=="6015", ATTRS{idVendor}=="0403", GROUP="dialout", MODE="0664"

Note that my username is in the dialout group.
