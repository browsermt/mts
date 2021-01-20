#!/bin/bash

FOLDER="$1"

for c in federal emea tatoeba; do 
    for i in $FOLDER/$c; do 
      echo sacrebleu ref-$c \< $i \> $i.sacre; 
      # if [ ! -f $i.sacre ]; then 
      #   echo sacrebleu ref-$c \<$i \>$i.sacre; 
      # fi;
    done; 
  done;
for t in wmt10 wmt11 wmt13 wmt14 wmt15 wmt16 wmt17 wmt18 wmt19; do 
      echo sacrebleu -t $t -l en-de \< $FOLDER/$t \> $FOLDER/$t.sacre; 
      # if [ ! -f restored/$t.sacre ]; then 
      #   echo sacrebleu -t $t -l en-de \< restored/$t \> restored/$t.sacre; 
      # fi; 
done

