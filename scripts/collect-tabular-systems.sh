#!/bin/bash

DIRS=(
    restored-mts 
    restored-marian-decoder
)

for DIR in ${DIRS[@]}; do
    for t in wmt10 wmt11 wmt13 wmt14 wmt15 wmt16 wmt17 wmt18 wmt19; do 
        BLEU=$(cat $DIR/$t.sacre  | awk '{print $3;}');
        echo -n "$BLEU ";
          # echo sacrebleu -t $t -l en-de \< $FOLDER/$t \> $FOLDER/$t.sacre; 
          # if [ ! -f restored/$t.sacre ]; then 
          #   echo sacrebleu -t $t -l en-de \< restored/$t \> restored/$t.sacre; 
          # fi; 
    done;
    for c in federal emea tatoeba; do 
        BLEU=$(cat $DIR/$c.sacre  | awk '{print $3;}');
        echo -n "$BLEU ";
    done
    echo ""

done
