/* hello-ftdi.c: flash LED connected between CTS and GND.
   This example uses the libftdi API.
   Minimal error checking; written for brevity, not durability. */

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <ftdi.h>
#include "healthcheck.h"

#define BUFLEN 64
//#define BUFLEN 1

#define COMP1 1
#define COMP2 4
#define SWEN1 2
#define SWEN2 0
// Add bits are outputs, except COMP1 and COMP2
#define MASK (0xff & ~(1 << COMP1) & ~(1 << COMP2))

int main()
{
    struct ftdi_context ftdic;

    /* Initialize context for subsequent function calls */
    ftdi_init(&ftdic);

    /* Open FTDI device based on FT232R vendor & product IDs */
    if(ftdi_usb_open(&ftdic, 0x0403, 0x6015) < 0) {
        puts("Can't find Infinite Noise Multiplier");
        return 1;
    }

    /* Try to set high baud rate */
    int rc = ftdi_set_baudrate(&ftdic, 3000000);
    if(rc == -1) {
        puts("Invalid baud rate\n");
        return -1;
    } else if(rc == -2) {
        puts("Setting baud rate failed\n");
        return -1;
    } else if(rc == -3) {
        puts("Infinite Noise Multiplier unavailable\n");
        return -1;
    }

    /* Enable bitbang mode with 4 out, 4 in */
    //rc = ftdi_set_bitmode(&ftdic, 0xf, BITMODE_BITBANG);
    rc = ftdi_set_bitmode(&ftdic, MASK, BITMODE_SYNCBB);
    if(rc == -1) {
        puts("Can't enable bit-bang mode\n");
        return -1;
    } else if(rc == -2) {
        puts("Infinite Noise Multiplier unavailable\n");
        return -1;
    }

    /* Endless loop: invert LED state, write output, pause 1 second */
    int i = 0;
    unsigned char outBuf[BUFLEN], inBuf[BUFLEN];
    for(i = 0; i < BUFLEN; i++) {
        // Alternate Ph1 and Ph2 - maybe should have both off in between
        outBuf[i] = i & 1?  (1 << SWEN2) : (1 << SWEN1);
        //outBuf[i] = i;
    }
    i = 0;
    for(;;) {
        if(ftdi_write_data(&ftdic, outBuf, BUFLEN) != BUFLEN) {
            puts("USB write failed\n");
            return -1;
        }
        if(ftdi_read_data(&ftdic, inBuf, BUFLEN) != BUFLEN) {
            puts("USB read failed\n");
            return -1;
        }
        i++;
        if((i & 0xfff) == 0) {
            printf("read %u, last byte == %u\n", 4096*BUFLEN, inBuf[BUFLEN-1]);
        }
    }
}
