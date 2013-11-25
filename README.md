atlaas-genom
============

```
rm -f ~/openrobots/lib/pkgconfig/atlaas.pc
genom -t atlaas
./configure TCL_CPPFLAGS=-I/usr/include/tcl8.5 --prefix=$HOME/openrobots
make install

killmymodules
pkill tclserv
h2 init 21123123
# run atlaas
tclserv
eltclsh
package require genom
connect
lm atlaas

atlaas::Init  80 80 0.1 40 -40 0 0 31 1 velodyneThreeDImage pomPos
atlaas::Fuse
atlaas::Save
atlaas::Export8u

while { 1 } { atlaas::Fuse; atlaas::Export8u; after 5; }

eltclsh
package require genom
connect
lm atlaas

atlaas::Fuse
atlaas::Save

```

