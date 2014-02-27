atlaas-genom
============

```
export DEVEL_BASE=${HOME}/devel
export TCLSERV_MODULE_PATH=${DEVEL_BASE}/lib/tclserv:${ROBOTPKG_BASE}/lib/tclserv
rm -f ~/openrobots/lib/pkgconfig/atlaas.pc
genom -t atlaas
./configure TCL_CPPFLAGS=-I/usr/include/tcl8.5 --prefix=${DEVEL_BASE}
make -j8
make install

killmymodules
pkill tclserv
h2 end; h2 init 20123123

# run atlaas
# or valgrind --tool=callgrind atlaas
tclserv # -c

eltclsh
package require genom
connect
lm atlaas

# TODO InitGlobalPosition / RetrieveRobotAngleAndPos
# dala-scripts/shell-scripts/start.sh; sleep 2; killmodule rflex; killmodule velodyne
atlaas::Init 120 120 0.1 377016.5 4824342.9 141.0 31 1 velodyneThreeDImage pomPos
# velodyne::OneShot -180.0 180.0
# p3d::Init pomPos
# p3d::SetP3dPoster atlaasP3dPoster
# p3d::SetDumpParams /tmp 0 P3D_DUMP_ON
# Dala::p3dGoTo 10 10

atlaas::Fuse
atlaas::Save
atlaas::Export8u
# atlaas::FillP3D

while { 1 } {
    atlaas::Fuse;
    atlaas::Export8u;
    after 200;
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

Copyright © 2013-2014 CNRS-LAAS
