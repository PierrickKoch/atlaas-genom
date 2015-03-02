#! /bin/sh
# rm /tmp/velodyne
# ln -s /net/cetus/data1/robots/mana/2013-09-04-caylus/hdl-64-2/velodyne /tmp/

VELODYNE_HDL_NN=64
[ -z "$1" ] || VELODYNE_HDL_NN=$1

killmymodules
pkill tclserv
h2 end; h2 init 20123123

atlaas > atlaas.log 2>&1 &
velodyne > velodyne.log 2>&1 &
LANG=C tclserv -c > tclserv.log 2>&1 &

# wait for module to start
{ tail -f atlaas.log & } | sed -n '/all tasks are spawned/q'
{ tail -f velodyne.log & } | sed -n '/all tasks are spawned/q'

eltclsh << EOF
package require genom
connect
lm atlaas
lm velodyne

# mode model input_pcap enable_organization use_pom pos_poster
velodyne::Init VELODYNE_READ VELODYNE_HDL${VELODYNE_HDL_NN} NULL VELODYNE_TRUE VELODYNE_FALSE NULL

# base_name start_number
velodyne::SetReadModeParams /tmp/velodyne/shot 0

# size_x size_y px_res utm_x utm_y utm_z utm_zone utm_north velodyne_poster pom_poster
atlaas::Init 120 120 0.1 398369.61 4903128.61 0.0 31 1 velodyneThreeDImage NULL

while { ! [catch {velodyne::OneShot -180 180}] } {
    atlaas::Fuse
}

atlaas::MakeRegion
EOF
