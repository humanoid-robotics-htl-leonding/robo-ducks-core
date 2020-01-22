#!/bin/bash

pwd=$(dirname $(readlink -f $0))
echo "Executing from $pwd"
cd $pwd

list="naograce naodon naoalex naomarg"

for nao in $list; do
  echo ./midas upload -c develop nao6 $nao
done
