#!/bin/sh -ex

TARGET=${1:-deb}

VERSION=$(git --no-pager describe --tags --always)

case "$TARGET" in
  deb)
    rm -rf build
    mkdir -p build/DEBIAN
    mkdir -p build/usr/src/infnoise-${VERSION}

    # Copy kernel sources
    cp kernel/infnoise_main.c kernel/infnoise_health.c kernel/infnoise_keccak.c \
       kernel/infnoise.h kernel/Makefile kernel/dkms.conf \
       build/usr/src/infnoise-${VERSION}/

    # Substitute real version into dkms.conf
    sed -i "s/PACKAGE_VERSION=\".*\"/PACKAGE_VERSION=\"${VERSION}\"/" \
        build/usr/src/infnoise-${VERSION}/dkms.conf

    # Set up DEBIAN control
    cp build-scripts/control.debian.dkms build/DEBIAN/control
    echo "Version: ${VERSION}" >> build/DEBIAN/control

    # Substitute VERSION into maintainer scripts
    sed "s/\${VERSION}/${VERSION}/g" build-scripts/infnoise-dkms.postinst > build/DEBIAN/postinst
    sed "s/\${VERSION}/${VERSION}/g" build-scripts/infnoise-dkms.prerm > build/DEBIAN/prerm
    chmod 775 build/DEBIAN/postinst build/DEBIAN/prerm

    fakeroot dpkg-deb -Zxz -b build/ infnoise-dkms_${VERSION}_all.deb
    rm -rf build
    ;;

  rpm)
    VERSION_ONLY=$(git --no-pager describe --tags --always | cut -d'-' -f1)
    RELEASE=$(git --no-pager describe --tags --always | cut -d'-' -f2)

    if [ "$VERSION_ONLY" = "$RELEASE" ]; then
      RELEASE=0
    fi

    mkdir -p SOURCES
    tar -czf SOURCES/infnoise-${VERSION_ONLY}.tar.gz . --exclude="SOURCES"
    mkdir -p BUILD SPECS RPMS SRPMS

    cp build-scripts/infnoise-dkms.spec SPECS/infnoise-dkms.spec
    sed -i "s/__VERSION__/${VERSION_ONLY}/g" SPECS/infnoise-dkms.spec
    sed -i "s/__RELEASE__/${RELEASE}/g" SPECS/infnoise-dkms.spec

    rpmbuild --define "_topdir $(pwd)" -ba SPECS/infnoise-dkms.spec
    ;;

  arch)
    GITREPO=$(git config --get remote.origin.url)

    mkdir -p x86_64-dkms
    cd x86_64-dkms

    cp ../build-scripts/PKGBUILD.arch-dkms PKGBUILD
    cp ../build-scripts/INSTALL.arch-dkms INSTALL-dkms

    sed -i "s|.*source.*=.*(.*).*|source=('git+$GITREPO')|g" PKGBUILD
    echo "pkgrel=1" >> PKGBUILD

    makepkg -f
    ;;

  *)
    echo "Usage: $0 {deb|rpm|arch}"
    exit 1
    ;;
esac
