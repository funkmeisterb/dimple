#!/bin/sh

# This test file relies on the programs 'oscdump' and 'oscsend' which
# are available as part of the LibLo distribution.  Currently they are
# present in the LibLo svn repository, but not yet part of a stable
# release.

# This script assumes Dimple is already running.

# Listen on port 7775.  We'll assume this is the only oscdump instance
# running, and we don't want to run it if it's already running in
# another terminal.
if ! ( ps -A | grep oscdump >/dev/null 2>&1 ); then (oscdump 7775 &); fi

oscsend localhost 7774 /world/clear
oscsend localhost 7774 /world/prism/create sfff floor 0 0 -0.2
oscsend localhost 7774 /world/floor/size fff 1 1 0.01
oscsend localhost 7774 /world/floor/color fff 0.8 0.9 0.1
oscsend localhost 7774 /world/fixed/create sss c1 floor world

oscsend localhost 7774 /world/gravity fff 0 0 -1

if [ x$1 = x ]; then
    SIZE=6
else
    SIZE=$1
fi

for i in $(seq 0 $(($SIZE * $SIZE - 1))); do
    POS=$(python -c "print (($i % $SIZE)-(($SIZE-1)/2.0))*(0.5 / $SIZE), (($i / $SIZE)-(($SIZE-1)/2.0))*(0.5 / $SIZE)")
    oscsend localhost 7774 /world/sphere/create sfff s$i $POS 0
done

