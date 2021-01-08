#!/bin/bash

# Downloads the test-set of WNGT-20 data
FILES=(
    keys.xz
    ref-emea.xz
    ref-federal.xz
    ref-tatoeba.xz
    restore.py
    sources.shuf.xz
)


OUTPUT_DIR="$1"
if [ -d "$OUTPUT_DIR" ]
then
    for FILE in ${FILES[@]}
    do
        wget -c -P $OUTPUT_DIR "http://data.statmt.org/heafield/wngt20/test/${FILE}"
    done;
else
    echo "Invalid output directory: $OUTPUT_DIR"
    echo "Usage: $0 OUTPUT_DIR"
    exit 1
fi
