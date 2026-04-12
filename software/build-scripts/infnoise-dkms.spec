Name:           infnoise-dkms
Version:        __VERSION__
Release:        __RELEASE__
Summary:        Infinite Noise TRNG kernel module (DKMS)
Group:          System/Kernel
License:        GPL-2.0-or-later
URL:            https://github.com/13-37-org/infnoise
Vendor:         13-37.org
Source:		infnoise-%{version}.tar.gz
BuildArch:      noarch
Prefix:         %{_prefix}
Packager: 	Manuel Domke
BuildRoot:      %{_tmppath}/%{name}-root
Requires:       dkms, kernel-devel

%description
Kernel module for the Infinite Noise TRNG hardware random number generator.
Installed via DKMS so the module is automatically rebuilt on kernel upgrades.

%prep
tar -xzf ../SOURCES/infnoise-%{version}.tar.gz

%install
install -d "$RPM_BUILD_ROOT/usr/src/infnoise-%{version}"
for f in infnoise_main.c infnoise_health.c infnoise_keccak.c infnoise.h Makefile dkms.conf; do
    install -m 0644 "kernel/$f" "$RPM_BUILD_ROOT/usr/src/infnoise-%{version}/"
done
sed -i 's/PACKAGE_VERSION=".*"/PACKAGE_VERSION="%{version}"/' \
    "$RPM_BUILD_ROOT/usr/src/infnoise-%{version}/dkms.conf"

%post
dkms add infnoise/%{version} || :
dkms build infnoise/%{version} || :
dkms install infnoise/%{version} --force || :

%preun
dkms remove infnoise/%{version} --all || :

%clean
[ "$RPM_BUILD_ROOT" != "/" ] && rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root)
/usr/src/infnoise-%{version}/
