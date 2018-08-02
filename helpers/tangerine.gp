#set terminal png size 1920, 480
set terminal png size 3840, 960
#set terminal png size 1080,1080
#set size square
set encoding utf8

#halfrange=60.0
halfrange=150.0

xbox=240.0
ybox=60.0
xbox=xbox/2
ybox=ybox/2

set object 1 rect from -xbox,-ybox to xbox,ybox lw 3 fs empty border lc rgb "black"
set object 2 circle center 0,0 size 8.5 lw 3 fs empty border lc rgb "black"

set xrange [-xbox:xbox]
set yrange [-ybox:ybox]
#set xrange [-halfrange:halfrange]
#set yrange [-halfrange:halfrange]

typesno=`grep "Types no." $(ls 201?_*sec.dat | sort -n | tail -n 1) | awk '{print $NF}'` + 0
if (typesno == 2) {
set palette defined (0 "red", 1 "blue")
set cbrange [0:typesno-1]
} else {
set palette model HSV
set palette rgb 3,2,2
set cbrange [0:typesno]
}
unset colorbox

set output sprintf("%d", INDEX).'.png'
plot \
     DATAFILE0 u 1:2:($7/3):5 palette w circles fill solid noborder notitle
