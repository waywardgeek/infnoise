# Init scripts

Scripts in this folder are for use in systems where the Infnoise hardware is always present and the user wishes to run `infnoise` automatically as a system service with `--dev-random` enabled.

Should someone create distribution packages for infnoise in future, hopefully these will help!

## RPM, DEB and ArchLinux packages
The prebuilt packages availble for .rpm and .deb based distros described in the [README](../README.md) are built using a combination of a systemd unit and udev rules 
which start the service when the Infinite Noise TRNG is connected and also stops it immediately on disconnect.

## Contents

- `infnoise.gentoo.openrc`: OpenRC, tested in Gentoo, untested in Alpine Linux, FreeBSD, TrueOS
- `infnoise.openrc`: OpenRC, ?
- `infnoise.conf`: Upstart, tested in Ubuntu 14.04, 16.04 (requires upstart install)
- `infnoise.service`: Systemd, works for CentOS, Ubuntu, Debian, ArchLinux
- `infnoise.service.bin`: Same as infnoise.service, binary path = /usr/bin/ - uses config file from /etc/infnoise.conf
- `infnoise.service.sbin`: Same as infnoise.service, binary path = /usr/sbin/ - uses config file from /etc/infnoise.conf
- `infnoise.conf.systemd`: Config file for the systemd service, to set multiplier and serial number of device
- `75-infnoise.rules`: udev rule to be used together with the systemd service
