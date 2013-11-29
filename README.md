atlaas-genom
============

```
export DEVEL_BASE=${HOME}/devel
export TCLSERV_MODULE_PATH=${DEVEL_BASE}/lib/tclserv:{ROBOTPKG_BASE}/lib/tclserv
rm -f ~/openrobots/lib/pkgconfig/atlaas.pc
genom -t atlaas
./configure TCL_CPPFLAGS=-I/usr/include/tcl8.5 --prefix=${DEVEL_BASE}
make install

killmymodules
pkill tclserv
h2 init 20123123

# run atlaas
tclserv -c

eltclsh
package require genom
connect
lm atlaas

# atlaas::Init  80 80 0.1 40 -40 0 0 31 1 velodyneThreeDImage pomPos
atlaas::Init 90 90 0.1 0 0 0 0 31 1 velodyneThreeDImage pomPos
atlaas::Fuse
atlaas::Save
atlaas::Export8u

while { 1 } {
    for { set i 0 } { $i < 10 } { incr i } {
        atlaas::Fuse; after 300;
    }
    atlaas::Export8u;
}

eltclsh
package require genom
connect
lm atlaas
atlaas::Save
```

[![youtube](https://i2.ytimg.com/vi/k1-6gbYnmMU/sddefault.jpg "youtube")](http://youtube.com/embed/k1-6gbYnmMU?rel=0)


LICENSE
-------

[BSD 2-Clause](http://opensource.org/licenses/BSD-2-Clause)

Copyright © 2013 CNRS-LAAS
