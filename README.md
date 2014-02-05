atlaas-genom
============

```
export DEVEL_BASE=${HOME}/devel
export TCLSERV_MODULE_PATH=${DEVEL_BASE}/lib/tclserv:{ROBOTPKG_BASE}/lib/tclserv
rm -f ~/openrobots/lib/pkgconfig/atlaas.pc
genom -t atlaas
./configure TCL_CPPFLAGS=-I/usr/include/tcl8.5 --prefix=${DEVEL_BASE}
make -j8
make install

killmymodules
pkill tclserv
h2 init 20123123

# run atlaas
tclserv # -c

eltclsh
package require genom
connect
lm atlaas

atlaas::Init  120 120 0.1 377084.01 4824464.47 377084.01 4824464.47 31 1 velodyneThreeDImage pomPos
# velodyne::OneShot  -180.0 180.0
atlaas::Fuse
atlaas::Save
atlaas::Export8u
# atlaas::FillP3D

while { 1 } {
    for { set i 0 } { $i < 10 } { incr i } {
        # velodyne::OneShot  -180.0 180.0
        atlaas::Fuse; after 500;
    }
    atlaas::Export8u;
    atlaas::FillP3D;
}

eltclsh
package require genom
connect
lm atlaas
atlaas::Save

export PYTHONPATH=${PYTHONPATH}:${HOME}/devel/lib/python2.7/dist-packages
export PATH=${PATH}:${HOME}/work/gdal/gdal/swig/python/scripts
gdal_merge.py atlaas.*x*.tif

```

[![youtube](https://i2.ytimg.com/vi/k1-6gbYnmMU/sddefault.jpg "youtube")](http://youtube.com/embed/k1-6gbYnmMU?rel=0)


LICENSE
-------

[BSD 2-Clause](http://opensource.org/licenses/BSD-2-Clause)

Copyright © 2013 CNRS-LAAS
