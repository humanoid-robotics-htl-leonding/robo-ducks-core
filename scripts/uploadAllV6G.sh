#!/bin/bash

pwd=$(dirname $(readlink -f $0))
echo "Executing from $pwd"
cd $pwd

list="naolou naodon naoalex naomarg"

for nao in $list; do
  echo "===== $nao ====="
  ./midas upload -c develop nao6 $nao
done
