#!/bin/sh -ex

VERSION=`git --no-pager describe --tags --always | cut -d'-' -f1`
PKGREL=`git --no-pager describe --tags --always | cut -d'-' -f2`

GITREPO=`git config --get remote.origin.url`

if [ $VERSION == $PKGREL ]; then # this is a release
	PKGREL=0
fi

SIGNPACKAGE=1

while test $# -gt 0
do
    case "$1" in
        --notsigned) SIGNPACKAGE=0
            ;;
    esac
    shift
done

# x86_64
mkdir -p x86_64
cd x86_64

cp ../PKGBUILD.arch PKGBUILD
cp ../INSTALL.arch INSTALL

sed -i "s|.*source.*=.*(.*).*|source=('git+$GITREPO')|g" PKGBUILD
#echo "pkgver=$VERSION.$PKGREL" >> PKGBUILD
echo "pkgrel=1" >> PKGBUILD
echo "arch=('x86_64')" >> PKGBUILD

makepkg -f

if [ $SIGNPACKAGE -eq 1 ]; then
	PKGEXT='.pkg.tar.xz' makepkg --packagelist  | xargs -L1 gpg --sign
fi
