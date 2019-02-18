
### Infinite Noise TRNG (True Random Number Generator)

For instructions for compiling the and using Infinite Noise TRNG driver, go to the software
sub-directory, and read the [README](software/README.md) file there.  Both Linux and Windows
are supported.

The Infinite Noise TRNG is a USB key hardware true random number generator.  It uses what
I call a "Modular Entropy Multiplier" architecture (previously Infinite Noise Multiplier or
FireBug).  Besides being simple, low-cost, and fast, it is much easier to get right than
other TRNGs.  It naturally defends against influence from outside signals, such as radio
interference and power supply noise, making it simple to build securely, without requiring
an expert in analog design.  Modular entropy multipliers produce a provable and easily
measured level of entropy based on thermal noise, approximately equal to log2(K) per
output bit, where K is a gain between 1 and 2 set by two resistors around an op-amp.  A
"health monitor" can track this and verify that the output entropy is within the expected
range, which for the Infinite Noise TRNG described below is within 2% of log2(1.82).

Modular entropy multipliers are suitable for both board level implementation and ASIC
implementation.  Speed is limited by the speed of a gain stage and a comparator, and can
run in excess of 100 Mbit/second per second with high performance components.  Cheap
solutions with CMOS quad op-amps can run at 8Mbit/second.

Adjacent bits from a modular entropy multiplier are correlated, so whitening is required
before use in cryptography.  This should be done by continually reseeding a
cryptographically secure hash function such as SHA-512, Blake2b, Keccak-1600 (SHA3), or a
stream cipher such as ChaCha.  This implementation uses Keccak-1600 with cryptographically
secure reseeding of more than 400 bits of entropy at a time, overcoming a trickle in/out
problem present in the GNU/Linux /dev/random system.  Users who need many megabytes per second
of data for use in cryptography can set the outputMultiplier as high as they like, which
causes Keccak to generate outputMultiplier\*256 bits per reseeding by the Infinite Noise
TRNG.

The modular entropy multiplier architecture was invented by Peter Allan in 1999, which he
called [Firebug](http://apa.hopto.org/firebug).  I reinvented it in 2013.  As usual, most
of my good ideas are rediscoveries of existing ideas :-)  Peter has his own
version called [the Redoubler](https://github.com/alwynallan/redoubler), which
is awesome.  It really is the _right_ way to generate random bits, whether on a
board with standard parts, or on an custom chip.

### Crowd Supply campaign

![Infinite Noise Crowd Supply campaign](images/infinite-noise-crowdsupply.jpg?raw=true "Infinite Noise Crowdsupply campaign.")

With the aid of crowdfunding, the Infinite Noise will soon be produced (again)! The campaign is driven by Manuel Domke, founder of a fresh electronics manufacturing company called 13-37.org electronics. 

[Check the Crowd Supply project page for latest updates](https://www.crowdsupply.com/13-37/infinite-noise-trng)

### The Eagle open-source boards work!

Here is the first completed Infinite Noise USB key.  I offered this model on
Tindie to help get the modular entropy multiplier concept out there initially.

![Picture of Infinite Noise USB key](images/infnoise_key.jpg?raw=true "Infinite Noise USB key")

Here are the first three boards from OSH Park.  They work _exactly_ as predicted.  They all
generate 300,000 bits per second, resulting in a measured 259,000 bits of entropy per second, which is within 0.5% of the predicted value of log2(1.82).

All three boards should produce log2(1.82) = 0.864 bits of entropy per bit by design.  The first one is estimated
to produce 0.867, while the second one produces 0.868, and the third is 0.867.

![Picture of Infinite Noise Multiplier circuit board](images/INM_V1.jpg?raw=true "Infinite Noise Multiplier")

Here is the latest schematic:

![Schematic of Infinite Noise Multiplier](images/infnoise.png?raw=true "Infinite
Noise Multiplier")

Here is the latest board layout (thanks, EagleWorks!):

![Board layout of Infinite Noise Multiplier](images/infnoise_brd.png?raw=true "Infinite
Noise Multiplier")

The breadboard worked, too.  Estimated entropy per bit is 0.81 for the bread-board.  By design, it should
be 0.80, so it is very close to the prediction.  The breadboard proved out much of the theory of operation, as well os providing raw data for entropy testing.

![Breadboard of Infinite Noise Multiplier](images/infnoise_breadboard.jpg?raw=true "Infinite
Noise Multiplier")

Here is the voltage on one of the hold cap:

![Traces on left hold cap](images/CAP1.jpg?raw=true "Traces on left hold cap")

To build one of these for yourself, you can [order three boards from OSH Park for only
$3.25](https://oshpark.com/profiles/WaywardGeek), and then buy parts from [Digikey and
Mouser as described in the
BOM](https://github.com/waywardgeek/infnoise/blob/master/eagle/BOM.xlsx?raw=true).  I
designed this board to be cheap, not easy to assemble by hand.  I use 2 QFN parts and
three with 0.5mm lead pitch.  If you want to [build these yourself the way I
do](https://www.sparkfun.com/tutorials/59), consider uploading the [infnoise.brd
file](https://raw.githubusercontent.com/waywardgeek/infnoise/master/eagle/infnoise.brd) to
[OSH Stencils](https://www.oshstencils.com/) and ordering a solder paste stencil for $7.
I get the [solder paste from SparkFun](https://www.sparkfun.com/products/12878).  Kudos to
OSH Park, OSH Stencil, SmallBatchAssembly, and DigiSpark!  They're making this whole party
possible!

The total for all the parts, including boards from OSH Park, come to $5.69 each, in 1,000
unit quantities.  However, that cost is dominated by USB related parts, particularly the
FT240X chip, the USB connector, and the USB-stick enclosure.  Just the components for the
modular entropy multiplier come out to $0.97.

Cor van Wandelen was kind enough to create these scatter plots showing the non-randomness
in the raw output.
![color plot of raw data](tests/plots/infnoise-raw.bin-colormap.png?raw=true "Color plot")
![scatter plot of raw data](tests/plots/infnoise-raw-notraw-scatter.gif?raw=true "Scatter plot")

Here is a faster version that uses a more expensive op-amp from TI:

![Schematic of Infinite Noise Multiplier](infnoise_small/schematic.png?raw=true "Infinite
Noise Multiplier")

### The Problem: Noise Sensitivity, and Signal Injection

True random number generators are very difficult to get right.  Generally, they amplify a
tiny noise signal, perhaps only a microvolt in amplitude, by a factor of millions or
billions, until the signal is an unpredictable digital signal.  This signal is then
sampled to see if it's a 0 or 1.

The problem with this approach is the weak noise source can easily be influenced by other
nearby signals, which may be under the control of an attacker, or perhaps observable by an
attacker, enabling him to predict the output.  Systems built with massive amplification of tiny noise sources often require power supply filters and EMI shielding, and even then remain difficult to prove secure.  Generally, an expert analog designer is needed to get it right.

Intel's RDRAND instruction is a perfect example.  It uses rapid amplification of thermal
noise to determine the power-up state of a latch.  Unfortunately, this source of entropy
is highly power-supply and cross-talk sensitive.  Intel claims to have
carefully shielded their thermal noise source, but without a thorough pubic audit of both
the design and layout, including all potential sources of interference, it is not possible
to trust the RDRAND instruction as the source of entropy for cryptography.

With such strong sensitivity, these TRNG architectures are potential targets for signal
injection by an attacker, who can cause the TRNG to generate his desired output rather
than true random data.

### The Solution: Modular Multiplication

Unpredictable noise sources are tiny, and must be massively amplified to be used by an
TRNG.  Other TRNG architectures amplify these signals until they saturate, becoming
digital 1's and 0's.  They rely on careful design and shielding to keep outside signals
from influencing the noise source.

For example, if we amplify a tiny noise source by 1 billion in a system that saturates at
3.3V, then 1uV of noise will be amplified causing the output to be about 3.3V.  An
attacker need only introduce at least -1uV to cause the TRNG to saturate at 0V instead.
An attacker with even this tiny influence can entirely control the output.

If TRNGs used modular multiplication to amplify their noise source, this noise sensitivity
problem would go away.

If we multiply a 1uV peak by 1 billion modulo 3.3V, then the result will be about 0.3V,
which will result in a digital 0.  If an attacker subtracts 1uV, causing our noise source
to be at 0.0V, then after amplification, the output is 0V, which still results in a 0.  In
fact, without knowing the current amplitude of the noise source, there is no signal an
attacker can add to our noise source to control the output.  He may be able to flip the
output bit, but since it was already random, his signal injection fails to control the
result, which is still random.  In fact, an attacker's injected signal causes the output
to be _more_ random, since an attacker is a nice unpredictable source of entropy!
Infinite Noise Multipliers _add_ entropy from all noise sources, even those from an
attacker.

### Variations

There are currently 3 versions of Infinite Noise Multipliers documented here.  The
infnoise_small directory describes a low part-count design that works well with op-amps
which have rail-to-rail inputs and outputs.  It runs at 4MHz, outputting 0.86 bits worth of
entropy on each clock (loop gain = 1.82), for a total of over 3.4 Mbit of entropy produced
per second.  The infnoise_fast directory contains a 50% faster design that uses a few more
resistors and an additional op-amp.  This design is suitable for use with a wide range of
op-amps.  It runs at 6MHz, outputting 0.86 bits worth of entropy on each clock (loop gain =
1.82), for over 5Mbit of entropy per second.

Because Infinite Noise Mulitpliers are switched-capacitor circuits, it is important to use
components with low leakage, like the OPA4354 CMOS quad op-amp from TI.  Op-amps with
below 1nA of input bias current will enable running at lower frequencies with less power.

To reproduce these simulations, download the TINA spice simulator from Ti.com.

Here is a "small" modular entropy multiplier:

![Schematic of small Infinite Noise Multiplier](infnoise_small/schematic.png?raw=true "Small
Infinite Noise Multiplier")

Note that the upper left op-amp is used as a comparator, and must settle to either 0V or
Vsup before the lower left op-amp can multiply the input by 2X.

Here's a "fast" version that does two multiplications in parallel and uses the comparator
result to select the right one.

![Schematic of fast Infinite Noise Multiplier](infnoise_fast/schematic.png?raw=true "Small
Infinite Noise Multiplier")

There is also a [CMOS version described here](http://waywardgeek.net/RNG).

### Simulations

LTspice was used to simulate the small and fast variations.  Here are simulation waveforms
for the small version:

![Simulation of small Infinite Noise Multiplier](infnoise_small/shortsim.png?raw=true "Small
Infinite Noise Multiplier")
![Simulation of small Infinite Noise Multiplier](infnoise_small/longsim.png?raw=true "Small
Infinite Noise Multiplier")

And again for the fast version.

![Simulation of fast Infinite Noise Multiplier](infnoise_fast/shortsim.png?raw=true "Fast
Infinite Noise Multiplier")
![Simulation of fast Infinite Noise Multiplier](infnoise_fast/longsim.png?raw=true "Fast
Infinite Noise Multiplier")

### Theory of Operation

Consider how a successive-approximation A/D converter works.  Each clock cycle, we compare
the input voltage to the output of a D/A converter, and if it's higher, the next bit out
is 1, and if lower, it's a 0.  We use binary search to zero-in on the analog input value.
Here is a block diagram from Wikipedia:

![Successive approximation A/D block diagram](images/SA_ADC_block_diagram.png?raw=true "SAR
A/D Block Diagram")

There is another way to build a successive-approximation A/D that eliminates the D/A
converter.  Compare the input to Vref (1/2 supply), and if it is larger, subtract Vref
from the input.  Then multiply by 2X.  The bit out is the value of the comparator.

    Vin' = Vin >= Vref? Vin - Vref : Vin

This eliminates the D/A converter, and has no limit on how many bits we shift out.  In
reality, the only reason we do not use this architecture for real A/D converters is that
it's accuracy depends on the accuracy of the multiply by 2X operation.  A simple circuit
with 1% resistors would only achieve about a 7 bit resolution.

However, just because the bits are not accurate does not mean we can't keep shifting them
out.  What are we measuring after shifting 30 times?  It's just noise in the circuit, with
no correlation to the original Vin.  It's totally random unpredictable nonsense.  This is
the idea behind the Infinite Noise Multiplier.

If this A/D converter was perfect, both 0's and 1's should shift out with equal
probability, with no correlation between bits.  This has been verified C simulations and
dieharder.  However, due to accuracy limitations on real components, we cannot multiply by
exactly 2X every cycle.  When the loop amplification is < 2X, the entropy per output bit
is reduced, but can be easily computed.  If E is the entropy per bit, and K is the loop
amplification, then:

    E = log(K)/log(2)

or equivalently:

    E = log2(K)

This provides a simple way to calculate the entropy added to an entropy pool per bit.
The program infnoise.c directly measures the entropy of modular entropy multiplier output,
and compares this to the estimated value.  Both simulations and actual hardware show that
they correlate well.

The "fast" board level version uses two op-amps and comparator to impliment a modular
multiplication using a couple of tricks.  First, multiplication by 2 modulo Vsup is
accomplished by multiplying relative to either GND or Vsup.  When multiplying relative to
GND, a 0.2V signal becomes 0.4V.  When multiplying relative to a 3V Vsup, a 2.8V signal
becomes 2.6V, because 2.8V is 0.2V below 3V, and after 2X, it's 0.4V below 3V.  The math
comes out the same as if I'd multiplied relative to GND, and simply subtracted Vsup if the
result was > Vsup:

    Vsup - 2*(Vsup - Vin) = Vsup - 2*Vsup + 2*Vin = 2*Vin - Vsup
        = 2*Vin mod Vsup

So, we multiply by 2 either way, and only subtract out Vsup if needed.  This is identical
to multiplication modulo Vsup.  The comparator simply selects the output of one of the two
op-amps.  This is the basic analog modular multiplier.

A second trick used to create the "small" version was to notice that the output of the
comparator could be used to combine both multiplier op-amps into 1.  This abuse of the
comparator output needs to be carefully checked.  In particular, the output is generally
treated as a digital signal, but in this case, it is used as an analog singal.  Care
should be taken not to load the OUT signal significantly.

### Provable Entropy, Based on Thermal Noise

The RMS thermal noise generated by a resistor of value R is:

    Vnoise = sqrt(4*Kb*T*deltaF*R)

where Kb is Boltzmann's constant 1.3806504×10-23 J/K, T is temperature in Kelvin (about
293 for room temperature), and deltaF is the frequency range of noise being measured.

For the V1 version of the Infinite Noise boards above, the op-amp has an 8MHz unit gain
crossover, and a low load negative input with 10K Ohms in parallel with 8.2K Ohms, which
is 4.5K Ohms.  Vnoise up to unity crossover is about 24uV, and gets amplified by the
op-amp gain K of 1.82 to about 40uV, and held on a 220pF capacitor.  A 40uV change on the
hold capacitors results in a current of about 55,000 electrons, so the hold capacitors are
able to capture about 15 bits of resolution of this noise signal, which gets amplified by
K and combined with later noise samples in the hold capacitors every cycle.  The
capacitors s hold about 2^31 different charge levels in the range of 0.3V to 3V.  This is
effectively a 31-bit register which we multiply by 1.82 every cycle and add a 15 bit noise
signal.  This results in entropy shifted out just slightly less than log2(1.82), since
this entropy compression does not result in 100% pure entropy (just very close).

changes the output of the comparator several cycles later.  Each cycle, this 15-bit noise
signal is added in, far in excess of the log2(K) of entropy we shift out, enabling us to
concentrate entropy in the hold capacitors, which hold over 2^32 electrons when
discharging from the design range of 3V to 0.3V.  Note that we are not assuming successive
noise samples of this 8MHz noise, when measured at 480KHz, will be completely uncorrelated,
but only that concentrating a 15-bit representation of sequential samples will have a few
bits of "surprise", which is log2(1/probability of guessing the next sample).

### Analysis of Analog Modular Multiplication

Consider a traditional zener TRNG:

    Vzener -> Multiply by 1e9 relative to Vref -> Clamp between -Vsupply and Vsupply -> digital 0/1

For simplicity, assume our amplifier has positive and negative supplies, Vsupply, and
-Vsupply.  If Vzener\*1e9 >= 0, then the output is a digital 1, otherwize 0.

There are variations on this theme, but this is basically how Zener TRNGs work.  The math
computed by this circuit is:

    clamp(-Vsupply, Vsupply, 1e9*Vzener)

where the first two parameters are the lower and upper clamping voltages, and the third
parameter is the amplified signal.

The problem with this design is that an attacker, Malory, can override the tiny zener
noise source with radio-signals, or any of a number of attacks.  Basically, if he can find
a way to add his signal to Vzener, then the circuit does this:

    clamp(-Vsupply, Vsupply, 1e9*(Vzener + Vmallory))

If Vmallory is always just a bit larger than Vzener in magnitude, then Mallory can
completely determine the output, because Mallory can make Vzener + Vmallory greater or
less than zero at will, and after multiplying by 1e9 it the amplifier will saturate in the
direction of the sign of Vmallory.

What if we used modular multiplication instead?  Assume we represent Vzener now as a
positive voltage between 0 and Vsupply so we can use the normal mod operation.  In this
case the normal output would be:

    Vzener*1e9 mod Vsupply -> compare to Vsupply/2 -> 1 if > Vsupply/2, 0 otherwise

This is even _more_ unpredictable than the original version. Only the portion of Vzener
between +/- Vsupply/1e9 made any difference in the output before, but now we use the
entire amplitude of Vzener.  The amplitude of Vzener has additional entropy which now
further whitens the output bit.

When Mallory attacks this system, injecting Vmallory into the same node as Vzener, it
computes:

    (Vzener + Vmallory)*1e9 mod Vsupply = Vzener*1e9 + Vmallory*1e9 mod Vsupply

Let Vz = Vzenner\*1e9 mod Vsupply, and Vm = Vmallory\*1e9 mod Vsupply.  Then the output
is just:

    Vz + Vm mod Vsupply

Vz is unpredictably distributed between 0 and Vsupply, hopefully somewhat uniformly.
How can Mallory determine what to add to it to control the output?  He can not.
His interference can only _increase_ the entropy of the output, since Mallory's attack is
itself an entropy source, further randomizing the result.

### Rolling Up the Loop

A 30-long cascade of switched capacitor 2X modular multipliers is a lot of hardware!
Fortunately, it is possible to reuse the same multiplier for each stage, without even
slowing down the circuit.  In our long chain of 2X modular multipliers, we computed:

    A1(1) = 2*Vzener(0) mod Vsupply
    A2(2) = 2*A1(1) mod Vsupply
    A3(3) = 2*A2(2) mod Vsupply
    ...
    A30(30) = 2*V29(29) mod Vsupply

Here, Vzener(0) is Vzener when sampled at the first clock pulse.  Vzener(n) is the voltage
sampled on the nth clock pulse.  An(i) is the output of the nth 2X modular multiplier at
clock cycle i.  Instead of using 30 stages, what would happen if we simply fed the output
of the 2X modular multiplier stage back on itself?  We'd just have A instead of A1 ..
A30:

    A(1) = 2*Vzener(0) mod Vsupply
    A(2) = 2*(Vzener(1) + A(1) mod Vsupply 
          = 2*Vzener(1) + 4*Vzener(0) mod Vsupply
    A(3) = 2*(Vzener(2) + A(2)) mod Vsupply
          = 2*Vzener(2) + 4*Vzener(1) + 8*Vzener(0) mod Vsupply
    ...
    A(30) = 2*Vzener(29) + 4*Vzener(28) + 8*Vzener(27) + ... + 2^30*Vzener(0) mod
    Vsupply

If 2^30\*Vzener(0) mod Vsupply is an unpredictable value, then the other terms can in no
way reduce this unpredicability.  What if Mallory attacks?  In that case, at step 30, we
have:

    A(30) = 2*(Vzener(29) + Vmallory(29)) + ... + 2^30*(Vzener(0) + Vmallory(0) mod
    Vsupply
            = [2*Vzener(29) + 4*Vzener(28) + ... + 2^30*Vzener(0)] +
              [2*Vmallory(29) + 4Vmallory(28) + ... + 2^30*Vmallory(0)] mod Vsupply

The output is just the signal we had before without Mallory's influence, plus a value
injected by Mallory, mod Vsupply.  Once again if Mallory does not know what the value of
A(30) would have been, he cannot control the result.  He can only make it more random.

The value of A acts as an entropy pool, collecting entropy from all signals that impact
it's value.

### We Don't Need Zener Noise

In reality, there are many sources of unpredictable noise in every circuit.  There's large
predictable and controllable noise, like power supply noise, and tiny 1/f noise in the
multi-gigahertz range.  Shot noise, thermal noise, EMI, cross-talk... you name it, no
matter where we look, there's noise.  Infinite noise multipliers amplify them all in
parallel, and adds them together effectively in an tiny entropy pool.  Zener noise would
be just one more source of noise in a symphony of existing noise sources, and will not
enhance the resulting entropy enough to bother.

A modular entropy multiplier will amplify _every_ source of niose and amplify it until it is
larger than Vsupply.  It adds them together and amplifies them in parallel.  Every device
in the signal path loop contributes. 

With N sources of noise, the output looks like:

    V1(1) = Vnoise1(1) + Vnoise2(1) + Vnoise3(1) ... VoinseN(1)
    ...
    V1(30) = [2*Vnoise1(29) + 4*Vnoise1(28) + ... + 2^30*Vnoise1(0)] +
             [2*Vnoise2(29) + 4*Vnoise2(28) + ... + 2^30*Vnoise2(0)] +
             ...
             [2*VnoiseN(29) + 4VnoiseN(28) + ... + 2^30*VnoiseN(0)] mod Vsupply

Each individual noise sources contributes its own power-of-two sequence to the total.
A micro-volt noise source contributes nearly as strongly as a Vsupply/10 amplitude noise
source.

The mashing together of noise source data with unbounded modular multiplications leads to
awesome entropy levels.  Just how awesome?  Consider just thermal noise from one resistive
summing node (the minus terminal on op-amp in the 2X gain stage).

How correlated are successive samples?  How badly does this impact our output?  It turns
out that high correlation is OK.  What we want is high resolution contribution of noise
samples, even more than low correlation.

Suppose sample Vnoise(0) is 1.034 uV, and Vnoise(1) is 1.019 uV.  That is
some bad correlation, but 14 clock cycles later, the difference in amplitudes between
these samples will have the output toggling unpredictably.  What matters is not
correlation between noise samples, but the accuracy to which we can remember the
difference between them in our circuit.  This should be limited only by electron counts on
our hold capacitor.  It has an integer number of electrons at any time.  About 2.5 billion
electrons flow out of 100pF holding caps when charging from 0.5V to 4.5V.  That's about 31
bits of resolution.  Every time we capture noise on these caps, it adds or subtracts an
integer number of electrons.  Each electron contributes about 1.6nV on our hold cap.  So
long as we can capture noise that has significantly more than 1.6nV of unpredictability,
we should be able to keep the output generating close to 1 bit of entropy per clock.  In
this example, both noise samples had over 10X the minimum resolution in unpredictable
noise, and easily contributed a bit of entropy each to our 31-ish bit entropy pool.

### Whitening the Right Way

Data coming from true random number generators is never 100% random.  I am aware of no
exceptions.  Whitening is required in all cases before the data is suitable for use in
cryptography.

Most TRNGs build whitening into the hardware.  This is a _mistake_, at least if the
whitening cannot be turned off.  Respectable TRNGs, such as [OneRNG](http://onerng.info/)
and [Entropy Key](http://www.entropykey.co.uk/) provide hardware whitening, but also
provide access to the raw data streams for health analysis.

I prefer to follow the KISS rule when it comes to security.  The more complex the TRNG
key, the more likely it is insecure.  Therefore, the initial Infinite Noise Multiplier
does not even have a microcontroller onboard, and only returns raw data, direct from the
noise source.  Whitening is done in the Infinite Noise driver.

The Infinite Noise driver uses the reference version of the SHA3 "sponge", called Keccak,
with a 1600 bit state.  To make the state of the sponge unpredictable, it is initialized
with 20,000 bits of of Infinite Noise data before any data is output.  After that, reading
bytes from the SHA3 sponge blocks until twice as many bytes of entropy have been fed into
the sponge from the Infinite Noise TRNG.

### Non-Power-of-Two Multiplication

The circuit shown in infnoise_fast multiplies by 1.82 every clock rather than 2.0.  As
stated above, this reduces the entropy per output bit to log(1.82)/log(2) = 0.86 bits of
entropy per output bit.

Suppose in cycle 0, the noise Vnoise(0) is +/- Vsupply/2^30V every cycle.  At most 31 clock
cycles can go by before the output toggles due to Vnoise(0), because on cycle 31, it's
contribution will be 2\*Vsupply, and the remaining noise contributions can do no more than
subtract Vsupply.  This will have to be subtracted out.  The cycle x1 where we know
Vnoise(0) will have toggled the output is:

    Vnoise(0)*2^x1 >= 2*Vsupply
    2^x1 >= 2*Vsupply/Vnoise(0)
    x1 >= log(2*Vsupply/Vnoise(0))/log(2)

When multiplying by K, where 1 < K < 2, it takes more clock cycles for Vnoise(0) to reach
Vsupply, insuring that it will have changed the output.  The cycle x2 when this happens
is:

    Vnoise(0)*K^x2 >= 2*Vsupply
    K^x2 >= 2*Vsupply/Vnoise(0)
    x2 >= log(2*Vsupply/Vnoise(0))/log(K)

The ratio of the clocks it takes with amplification 2 vs K is:

    x1/x2 = [log(Vsupply/Vnoise(0))/log(2)] / [log(Vsupply/Vnoise(0))/log(K)]
          = log(K)/log(2)

It takes log(2)/log(K) more cycles to insure the output is different.
the entropy shifted out with exactly 2X amplification will be 1 bit per clock.

### Health Monitoring

Infinite Noise Multipliers output entropy at a predictable rate, which is measured
continually.  If the entropy per bit deviates from the theoretical value of log(K)/log(2)
by more than 2% during the previous 80,000 bits, then the driver stops generating output.
Some deviation is expected, since K is dependent on two resistor values, which can be off
by 1% each.  Also, a significant amplitude noise in the system, as well as "misfires", can
cause more entropy to be output than predicted.  The estimated entropy per bit are
continually estimated and compared to expected values.

Entropy per bit is measured as the log2 of one over the probability of seeing a specific
output sequence from the modular entropy multiplier.  The probability of any given output
bit is estimated by keeping a history of results, given the previous 7 bits.  Simulations
with K=1.82 show that using 16 bits rather than 7 gives only a 0.16% improvement in
prediction accuracy, so only 7 are used.

### Entropy Testing

A common program used to estimate entropy in a data stream is "ent".  A sample data file
was created this way (using one of the original V1 boards):

    $ sudo ./infnoise-v1 --raw --debug > foo
    ...
    Generated 14680064 bits.  OK to use data.  Estimated entropy per bit: 0.866710, estimated
    K: 1.823500
    num1s:49.888504%, even misfires:0.159748%, odd misfires:0.127300%
    Generated 15728640 bits.  OK to use data.  Estimated entropy per bit: 0.866856, estimated
    K: 1.823684
    num1s:49.901504%, even misfires:0.145973%, odd misfires:0.160365%
    Generated 16777216 bits.  OK to use data.  Estimated entropy per bit: 0.867010, estimated
    K: 1.823879
    num1s:49.963040%, even misfires:0.146815%, odd misfires:0.145067%

Here's ent's results on the raw data stream this run produced:

    $ ent foo
    Entropy = 7.318058 bits per byte.
    
    Optimum compression would reduce the size
    of this 2072576 byte file by 8 percent.
    
    Chi square distribution for 2072576 samples is 1510131.51, and randomly
    would exceed this value 0.01 percent of the times.
    
    Arithmetic mean value of data bytes is 127.5088 (127.5 = random).
    Monte Carlo value for Pi is 3.427100794 (error 9.09 percent).
    Serial correlation coefficient is -0.005035 (totally uncorrelated = 0.0).
    

The health monitor does a much better job at estimating the entropy than ent.  The actual
non-randomness is 60% higher than ent predicted.  Ent said 0.915 bits of entropy per bit,
while the health monitor measured 0.867.  The health monitor's entropy estimator is just a
tiny fraction of a percent higher than the model predicts, while ent is off by 4.8% .  The
design target for entropy per bit was log2(1.82) = log(1.82)/log(2) = 0.864.  This is set
by two resistors.  Based on entropy measured by the health monitor, it could compress this
file by 13.3%.

The entropy estimator is based on the model that:

- The device is not rapidly changing the sort of numbers it puts out, so history can be
  used as a guide.
- There is no special state stored in the modular entropy multiplier that could cause data
  to be different each clock cycle, other than on even/odd cycles.
- Bits further away are less correlated.

The first assumption is strong assuming an attacker is not involved.  An attacker
injecting a strong signal could mount a DoS attack, since the health monitor would detect
entropy being too high, and would disable output.  This is a conscious choice: the health
monitor could instead simply warn that entropy seems too high.  Turning off the output
when an attacker may be present seems the safer choice.

The second assumption relies on the fact that only two nodes store state in this
implementation of a modular entropy multiplier, and that the outputs are sampled from
even/odd comparator outputs on even/odd cycles.  Other TRNGs may not satisfy this
assumption if they have additional internal state.  However, a typical zener TRNG should
satisfy this assumption.

The third assumption really does require a modular entropy multiplier.  A zener TRNG would
most likely have strong 60 Hz correlations from 60 Hz noise, for example.  This is also
true of A/D converter based TRNGs.  With a modular entropy multipliers, these signal sources
are added to a signal already saturated with thermal noise, making it in no less random.
Every cycle, a new thermal noise sample is added to the state, causing less correlation
with previous states.

### Future Version

I'm really just having fun with this project.  I personally prefer this version

![Infnoise TRNG I carry](images/infnoise_current.jpg?raw=true "The TRNG I carry around")

I plan on only making this version going forward, even though I doubt it will sell very
well.  That's OK, because I don't make any money on them :-)

### Credits

Peter Allan has been very helpful in moving this project forward.  The github user known
as EagleWorks did an _amazing_ job redoing the board layout.  Somehow, my ameraturish work
just wasn't good enough for him :-)  Thanks, EagleWorks!  Cor van Wandelen created the
color and scatter plots.

### Free As in Freedom

The modular entropy multiplier architecture was invented in 1999 by Peter Allan, but was not
patented at that time.  Peter is working with me to make modular entropy multiplier/Firebug
open-source hardware, unencumbered by patents or copyright.

I reinvented with the modular entropy multiplier architecture in 2013, and the board level
versions in 2014.  I hereby renounce any claim to copyright and patent rights related to
any changes or improvements I may have made to this architecture.  Furthermore, I am aware
of no infringing patents and believe there are none.  It should be entirely safe for use
in any application.  Feel free to copy anything here, and even sell your own modular
noise multiplier based USB keys based on this work.
