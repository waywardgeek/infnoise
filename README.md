##Infinite Noise Multiplier

The Infinite Noise Multiplier (INM) is an architecture for true random number generators (TRNG).
Besides being simple, low-cost, and fast, it is easy to get right, unlike other TRNGs.

INMs are suitable for both board level implementation, and ASIC implementation.  Speed is
limited by the speed of a voltage buffer and comparator, and can run in excess of 100
Mbit/second per second with high performance components.  Cheap solutions with CMOS quad
op-amps can run at 6Mbit/second.

Adjacent bits from an INM are correlated, so whitening is required before use in
cryptography.  However, the output has a highly predictable amount of entropy for easy
estimation of bits added to an entropy pool.

### The Eagle open-source boards work!

Here's the first three boards from OSH Park.  This is the V1 version, which works
_exactly_ as predicted.  The top board I do not plan to assemble.  The middle was my first
assembly, which required a bit of rework.  The bottom was my second board, which came out
right the first time.  Both boards should produce 0.864 bits of entropy per bit.  The
first one is estimated to produce 0.867, while the second one produces 0.868.  That's
only an error vs design of 0.5%!

![Picture of Infinite Noise Multiplier circuit board](images/INM_V1.jpg?raw=true "Infinite Noise Multiplier")

Here's the latest schematic...

![Schematic of Infinite Noise Multiplier](images/infnoise.png?raw=true "Infinite
Noise Multiplier")

Here's the latest board layout...

![Board layout of Infinite Noise Multiplier](images/infnoise_brd.png?raw=true "Infinite
Noise Multiplier")

The breadboard worked, too, though the boards are even closer to the theoretical
operation.  They all generate 300,000 bits per second, resulting in 259,000 bits or
entropy per second.  Estimated entropy per bit is 0.81 for the bread-board.  By design, it
should be 0.80, so it is very close to the prediction!  Part of what it took to get it
working so closely with the model was tuning the hold capacitors (now 220pF, rather than
100pF), and the baud rate of the FT240X USB interface chip, which controls the speed of
clocking the INM.  Slowing the clocks down by setting the baud rate to 30,000 and
increasing the hold capacitors seems to have helped reduce what I call "misfires" by about
7X.  If the voltage on a hold cap is still moving when we open the switch, it is likely
that the comparator feeding the op-amp has not yet settled, likely because it's inputs are
close in value.  It will continue moving after the switch closes, and may settle to the
opposite of the digital value we read.  This is an unpredictable situation not dealth with
in the model, so reducing it improved matching with the model.  Misfires were occuring 9%
of the time, and now occur about 1.2% of the time.

The breadboard proved out much of the theory of operation, as well os providing raw data
for entropy testing.

![Breadboard of Infinite Noise Multiplier](images/infnoise_breadboard.jpg?raw=true "Infinite
Noise Multiplier")

Here's the voltage on the left hold cap:

![Traces on left hold cap](images/CAP1.jpg?raw=true "Traces on left hold cap")

Here's the voltage on the right hold cap:

![Traces on right hold cap](images/CAP2.jpg?raw=true "Traces on right hold cap")

If you are interested in building this version of the Infinite Noise Multiplier, you may
be interested in opening eagle/BOM.ods in the git repo above.  It has all the Digikey and Mouser
parts along with cost/1000 units.  The total for all the parts, including boards from OSH
Park, come to $5.44 each, in 1,000 unit quantities.  However, that cost is dominated by
USB related parts, particularly the FT240X chip, the USB connector, and the USB-stick
enclosure.  Just the components for the INM come out to $0.99.

Here is a faster version that uses a more expensive op-amp from TI:

![Schematic of Infinite Noise Multiplier](infnoise_small/schematic.png?raw=true "Infinite
Noise Multiplier")

### The Problem: Noise Sensitivity, and Signal Injection

True random number generators are very difficult to get right.  Generally, they amplify a
tiny noise signal, perhaps only a microvolt in amplitude, by a factor of millions or
billions, until the signal is an unpredictable digital signal.  This signal is then
sampled to see if it's a 0 or 1.

The problem with this aproach is the weak noise source can easily be influenced by other
nearby signals, which may be under the control of an attacker, or perhaps observable by an
attacker, enabling him to predict the output.

Systems built with massive amplification of tiny noise sources often require power supply
filters and EMI shielding, and even then remain difficult to prove secure.

Intel's RDRAND instruction is a perfect example.  It uses rapid amplification of thermal
noise to determine the power-up state of a latch.  Unfortunately, this source of entropy
is highly power-supply, cross-talk, and substrate current sensitive.  Intel claims to have
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
which will result in a ditital 0.  If an attacker subtracts 1uV, causing our noise source
to be at 0.0V, then after amplification, the output is 0V, which still results in a 0.  In
fact, without knowing the current amplituded of the noise source, there is no signal an
attacker can add to our noise source to control the output.  He may be able to flip the
output bit, but since it was already random, his signal injection fails to control the
result, which is still random.  In fact, an attacker's injected signal causes the output
to be _more_ random, since an attacker is a nice unpredictable source of entropy!
Infinite Noise Multipliers _add_ entropy from all noise sources, even those from an
attacker.

### Variations

There are currently 3 versions of Infinite Noise Multipliers documented here.  The
infnoise_small directory describes a low part-count design that works well with op-amps
which have rail-to-rail inputs and outputs.  It runs at 4MHz, outputing 0.86 bits worth of
entropy on each clock (loop gain = 1.82), for a total of over 3.4 Mbit of entropy produced
per second.  The infnoise_fast directory contains a 50% faster design that uses a few more
resistors and an additional op-amp.  This design is suitable for use with a wide range of
op-amps.  It runs at 6MHz, outputing 0.86 bits worth of entropy on each clock (loop gain =
1.82), for over 5Mbit of entropy per second.

Because Infinite Noise Mulitpliers are switched-capacitor circuits, it is important to use
components with low leakage, like the OPA4354 CMOS quad op-amp from TI.  Op-amps with
below 1nA of input bias current will enable running at lower frequencies with less power.

To reproduce these simulations, download the TINA spice simulator from Ti.com.

Here's a "small" INM:

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
for the small verision:

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

Consider how a successive-aproximation A/D converter works.  Each clock cycle, we compare
the input voltage to the output of a D/A converter, and if it's higher, the next bit out
is 1, and if lower, it's a 0.  We use binary search to zero-in on the analog input value.
Here is a block diagram from Wikipedia:

![Successive aproximation A/D block diagram](images/SA_ADC_block_diagram.png?raw=true "SAR
A/D Block Diagram")

There is another way to build a successive-aproximation A/D that eliminates the D/A
converter.  Compare the input to Vref (1/2 supply), and if it is larger, subtract Vref
from the input.  Then multiply by 2X.  The bit out is the value of the comparator.

    Vin' = Vin >= Vref? Vin - Vref : Vin

This eliminates the D/A conveter, and has no limit on how many bits we shift out.  In
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
The program infnoise.c directly measures the entropy of INM output, and compares this to
the estimated value.  Both simulations and actual hardware show that they correlate well.

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

### Analisys of Analog Modular Multiplication

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

If Vmallory is always just a bit larger than Vzener in magnitued, then Mallory can
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

Vz is unpredicably distributed between 0 and Vsupply, hopefully somewhat uniformly.
How can Mallory determine what to add to it to control the output?  He can not.
His interference can only _increase_ the entropy of the output, since Mallory's attack is
itself an entropy source, further randomizing the result.

### Rolling Up the Loop

A 30-long cascade of switched capacitor 2X modular multipliers is a lot of hardware!
Fortunately, it is possible to reuse the same multplier for each stage, without even
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
predictable and controlable noise, like power supply noise, and tiny 1/f noise in the
multi-gigahertz range.  Shot noise, thermal noise, EMI, cross-talk... you name it, no
matter where we look, there's noise.  Infinite noise multipliers amplify them all in
parallel, and adds them together effectively in an tiny entropy pool.  Zener noise would
be just one more source of noise in a symphony of existing noise sources, and will not
enhance the resulting entropy enough to bother.

An INM will amplify _every_ source of niose and amplify it until it is larger than
Vsupply.  It adds them together and amplifies them in parallel.  Every device in the
signal path loop contributes. 

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

The mashing together of noise source data with unbounded modular multiplicationes leads to
awesome entropy levels.  Just how awesome?  Consider just thermal noise from one resistive
summing node (the minus terminal on op-amp in the 2X gain stage).

The RMS thermal noise generated by a resistor of value R is:

    Vnoise = sqrt(4*Kb*T*deltaF*R)

where Kb is Boltzmann's constant 1.3806504×10-23 J/K, T is temperature in Kelvin (about
293 for room temperature), and deltaF is the frequency range of noise being measured.  We
are only interested in noise components that are above the clock frequency, yet low enough
to pass through our op-amp.  If the clock frequency is F, consider the range from F to
10\*F.  If CLK is 100 MHz, the noise in the 100 MHz to 1000 MHz range at room temperature
generated by a 5K Ohm resistor at a high impedence summing node is about 0.27mV.  Even if
the op-amp rolls off with a 1st order pole right at 100MHz (very unlikely if we're
operating at a 100MHz clock), over 27uV of RMS noise should still get through.  Once
latched into the sample-and-hold on the output of the op-amp, this voltage is simply part
of our signal A, and gets multiplied by 2X no more than 12 times before causing changes
the output.

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
bits of resolution.  Every time we caputure noise on these caps, it adds or subtracts an
integer number of electrons.  Each electron contributes about 1.6nV on our hold cap.  So
long as we can capture noise that has significantly more than 1.6nV of unpredictability,
we should be able to keep the output generating close to 1 bit of entropy per clock.  In
this example, both noise samples had over 10X the minimum resolution in unpreditable
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
noise source.  Whitening is done in the INM driver.

The INM driver uses the reference version of the SHA3 "sponge", called Keccak, with a 1600
bit state.  To make the state of the sponge unpredictable, it is initialized with 20,000
bits of of INM data before any data is output.  After that, reading bytes from the SHA3
sponge blocks until twice as many bytes of entropy have been fed into the sponge from the
INM.

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
by more than 5% during the previous 80,000 bits, then the driver stops generating output.
Some deviation is expected, since K is dependent on two resistor values, which can be off
by 1% each.  Also, a significant amplitude noise in the system, as well as "misfires", can
cause more entropy to be output than predicted.  The estimated entropy per bit are
continually estimated and compared to expected values.

Entropy per bit is measured as the log2 of one over the probability of seeing a specific
output sequence from the INM.  The probability of any given output bit is estimated by
keeping a history of results, given the previous 7 bits.  Simulations with K=1.82 show
that using 16 bits rather than 7 gives only a 0.16% improvement in prediction accuracy, so
only 7 are used.

### Free As in Freedom

I, Bill Cox, came up with the original CMOS based Infinite Noise Multiplier architecture
in 2013, and the board level versions in 2014.  I hereby renounce any claim to copyright
and patent rights related to this architecture.  I'm giving it away emphatically freely.
Furthermore, I am aware of no infringing patents and believe there are none.  It should be
entirely safe for use in any application.
