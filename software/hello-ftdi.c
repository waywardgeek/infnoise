/* hello-ftdi.c: flash LED connected between CTS and GND.
   This example uses the libftdi API.
   Minimal error checking; written for brevity, not durability. */

#include <stdio.h>
#include <ftdi.h>

#define LED 0x08  /* CTS (brown wire on FTDI cable) */

int main()
{
    unsigned char c = 0;
    struct ftdi_context ftdic;

    /* Initialize context for subsequent function calls */
    ftdi_init(&ftdic);

    /* Open FTDI device based on FT232R vendor & product IDs */
    if(ftdi_usb_open(&ftdic, 0x0403, 0x6015) < 0) {
        puts("Can't open device");
        return 1;
    }

    /* Enable bitbang mode with a single output line */
    //ftdi_enable_bitbang(&ftdic, LED); -- obsolete
    int rc = ftdi_set_bitmode(&ftdic, 0xff, BITMODE_BITBANG);
    if(rc == -1) {
        puts("Can't enable bit-bang mode\n");
        return -1;
    } else if(rc == -2) {
        puts("USB device unavailable\n");
        return -1;
    }

    /* Endless loop: invert LED state, write output, pause 1 second */
    for(;;) {
        c = ~c;
        ftdi_write_data(&ftdic, &c, 1);
        sleep(1);
    }
}
