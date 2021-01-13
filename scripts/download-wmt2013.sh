#!/bin/bash

# Downloads mozilla's test scripts.

SRC='en'
TRG='es'

OUTPUT_DIR="$1"
if [ -d "$OUTPUT_DIR" ]
then
    sacrebleu -t wmt13 -l $SRC-$TRG --echo src > $OUTPUT_DIR/newstest2013.$SRC
    head -n10 $OUTPUT_DIR/newstest2013.$SRC > $OUTPUT_DIR/newstest2013.$SRC.top10lines
    head -n100 $OUTPUT_DIR/newstest2013.$SRC > $OUTPUT_DIR/newstest2013.$SRC.top100lines
    head -n300 $OUTPUT_DIR/newstest2013.$SRC > $OUTPUT_DIR/newstest2013.$SRC.top300lines
else
    echo "Invalid output directory: $OUTPUT_DIR"
    echo "Usage: $0 OUTPUT_DIR"
    exit 1
fi

