from random import random
from math import sin
from math import pi

def updateA(A, Vsup, noise, offset):
    """This could be built with one opamp for the multiplier, a comparator with
    rail-to-rail outputs, and switches and caps and resistors."""
    A += noise
    Vref = Vsup/2.0 # Resistor divider
    # A 3 resistor divider is used to deal with opamp offset voltage
    # The problem is that 0.0*1.9 == 0, and if the op-amp thinks 0V is actually -1e-6*1.9V,
    # it will try to drive as low as it can, getting stuck at 0V forever.
    # About a 1% shift should be enough.
    A = Vref + 0.99*(A - Vref)
    if A > Vref:
        # Possitive offset is the bad direction in this case
        A = Vsup - (Vsup - (A+offset))*1.9
        print "\b1",
    else:
        # Negative offset is the bad direction in this case
        A = (A-offset)*1.9
        print "\b0",
    return A

A = -0.01 # Just to test that it can recover from Opamp offset voltage
Vsup = 3.3
for i in range(2000):
    # Model noise as a large predictable sin wave and a 1fV (fempto-volt) small unpredictable noise source
    # The important point is that the 1fV of noise is enough to randomize the output.
    noise = 0.1 * sin(i*2*pi/16) + 1.0e-15*(random() - 0.5)
    # In comparison, without the 1fV of noise, the same result is computed every time:
    #noise = 0.1 * sin(i*2*pi/16)
    A = updateA(A, Vsup, noise, 0.001)
