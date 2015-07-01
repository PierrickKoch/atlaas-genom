#! /bin/sh
# rm /tmp/velodyne
# ln -s /net/cetus/data1/robots/mana/2013-09-04-caylus/hdl-64-2/velodyne /tmp/

velodyne=64
[ -z "$1" ] || velodyne=$1

killmymodules
pkill tclserv
h2 end; h2 init 20123123

LANG=C tclserv -c > tclserv.log 2>&1 &
for mod in atlaas velodyne; do
    echo > $mod.log && $mod > $mod.log 2>&1 &
    { tail -f $mod.log & } | sed -n '/all tasks are spawned/q' & pid="$pid $!"
done
# wait for modules to start
wait $pid

eltclsh << EOF
package require genom
connect
lm atlaas
lm velodyne

# mode model input_pcap enable_organization use_pom pos_poster
velodyne::Init VELODYNE_READ VELODYNE_HDL${velodyne} NULL VELODYNE_TRUE VELODYNE_FALSE NULL

# base_name start_number
velodyne::SetReadModeParams /tmp/velodyne/shot 0

# size_x size_y px_res utm_x utm_y utm_z utm_zone utm_north velodyne_poster pom_poster
# Caylus
atlaas::Init 120 120 0.1 398369.61 4903128.61 357.0 31 1 velodyneThreeDImage NULL
# LAAS parking
#atlaas::Init 120 120 0.1 377016.5 4824342.9 141.0 31 1 velodyneThreeDImage NULL
# Esperce
#atlaas::Init 120 120 0.1 370523.52 4797144.85 0.0 31 1 velodyneThreeDImage NULL

while { ! [catch {velodyne::OneShot -180 180}] } {
    atlaas::Fuse
    puts -nonewline "."
}

atlaas::MakeRegion
EOF
