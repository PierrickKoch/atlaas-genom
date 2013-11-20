atlaas-genom
============

```
rm -f ~/openrobots/lib/pkgconfig/atlaas.pc
genom -t atlaas
./configure TCL_CPPFLAGS=-I/usr/include/tcl8.5 --prefix=$HOME/openrobots
make

killmymodules
pkill tclserv
h2 init 21123123
# run atlaas
tclserv
eltclsh
package require genom
connect
lm atlaas

atlaas::Init  50 50 0.1 0 0 -20 20 0 0 velodyneThreeDImage
atlaas::Fuse
atlaas::Save

while { 1 } { atlaas::Fuse; after 50; }

eltclsh
package require genom
connect
lm atlaas

atlaas::Fuse
atlaas::Save

```

