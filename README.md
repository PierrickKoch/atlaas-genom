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

atlaas::Init  80 80 0.1 0 0 -40 40 31 1 velodyneThreeDImage
atlaas::Fuse
atlaas::Save

while { 1 } { atlaas::Fuse; after 5; }

eltclsh
package require genom
connect
lm atlaas

atlaas::Fuse
atlaas::Save

```

