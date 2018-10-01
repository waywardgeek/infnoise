Name:           infnoise-tools
Version:        __VERSION__
Release:        __RELEASE__
Summary:        Infinite Noise TRNG
Group:          Applications/Security
License:        GPL
URL:            https://github.com/13-37-org/infnoise
Vendor:         13-37.org
Source:		infnoise.tar.gz
Prefix:         %{_prefix}
Packager: 	Manuel Domke
BuildRoot:      %{_tmppath}/%{name}-root

%description

%prep
tar -xzf ../SOURCES/infnoise.tar.gz

%build
cd tools
make

%install
#make DESTDIR=$RPM_BUILD_ROOT install
install -Dvm755 "tools/passgen" "$RPM_BUILD_ROOT/usr/bin/infnoise-passgen"
install -Dvm755 "tools/dice" "$RPM_BUILD_ROOT/usr/bin/infnoise-dice"
install -Dvm755 "tools/healthcheck" "$RPM_BUILD_ROOT/usr/bin/infnoise-entcheck"
install -Dvm755 "tools/entcheck" "$RPM_BUILD_ROOT/usr/bin/infnoise-healthcheck"
install -Dvm755 "tools/hex2bin" "$RPM_BUILD_ROOT/usr/bin/infnoise-hex2bin"
install -Dvm755 "tools/bin2hex" "$RPM_BUILD_ROOT/usr/bin/infnoise-bin2hex"
install -Dvm755 "tools/findlongest" "$RPM_BUILD_ROOT/usr/bin/infnoise-findlongest"
install -Dvm755 "tools/flipbits" "$RPM_BUILD_ROOT/usr/bin/infnoise-flipbits"

%clean
[ "$RPM_BUILD_ROOT" != "/" ] && rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root)
%{_bindir}/infnoise-passgen
%{_bindir}/infnoise-dice
%{_bindir}/infnoise-entcheck
%{_bindir}/infnoise-hex2bin
%{_bindir}/infnoise-bin2hex
%{_bindir}/infnoise-findlongest
%{_bindir}/infnoise-flipbits
%{_bindir}/infnoise-healthcheck
