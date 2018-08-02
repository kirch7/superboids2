#!/bin/bash
if [ -z $1 ]
then
echo "please type a movie name"
exit
else
avconv -r 10 -i %d.png -vcodec libx264 -acodec aac -vf scale=-1:250 $1
fi

