The scripts need to be run from the software directory like this:

```$ build-scripts/build.sh x86```

and can create packages of the following types:
- DEB (Debian, Ubuntu, Raspbian)
- RPM (CentOS, Fedora)
- ArchLinux 

The packages get signed afterwards and made available as Github releases 
and via the apt repository described in software/README.

During the build the GitHub release and version information are compiled into the binary.

```
$ infnoise --version
GIT VERSION - 0.2.2
GIT COMMIT  - b428787161f7c759c70c081621ffe43f24ca73b1
GIT DATE    - 2018-03-01T23:43:26+01:00
BUILD DATE  - 2018-03-01T23:50:33+01:00
```
