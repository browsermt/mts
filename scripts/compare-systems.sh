#!/bin/bash

SRC=$1
TGT=$2
for c in federal emea tatoeba; do 
      echo "diff -s $1/$c.sacre $2/$c.sacre";
      diff -s $1/$c.sacre $2/$c.sacre;
      echo;
      # echo sacrebleu ref-$c \< $i \> $i.sacre; 
      # if [ ! -f $i.sacre ]; then 
      #   echo sacrebleu ref-$c \<$i \>$i.sacre; 
      # fi;
    done; 
for t in wmt10 wmt11 wmt13 wmt14 wmt15 wmt16 wmt17 wmt18 wmt19; do 
    echo "diff -s $1/$t.sacre $2/$t.sacre";
    diff -s $1/$t.sacre $2/$t.sacre;
    echo;
      # echo sacrebleu -t $t -l en-de \< $FOLDER/$t \> $FOLDER/$t.sacre; 
      # if [ ! -f restored/$t.sacre ]; then 
      #   echo sacrebleu -t $t -l en-de \< restored/$t \> restored/$t.sacre; 
      # fi; 
done;

