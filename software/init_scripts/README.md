# Init scripts

Scripts in this folder are for use in systems where the Infnoise hardware is always present and the user wishes to run `infnoise` automatically as a system service with `--dev-random` enabled.

Should someone create distribution packages for infnoise in future, hopefully these will help!

## Contents

- `infnoise`: OpenRC, tested in Gentoo, untested in Alpine Linux, FreeBSD, TrueOS
- `infnoise.conf`: Upstart, tested in Ubuntu 14.04, 16.04 (requires upstart install)
- `infnoise.service`: Systemd, untested in Redhat, Ubuntu etc

