#!/bin/bash

# Downloads Moby Dick from Gutenberg. There is no ground truth for this file,
# this is used to test is sentence-split mechanics work on a large corpus, and
# the translations are usable, by means of manual inspection.

OUTPUT_DIR="$1"
if [ -d "$OUTPUT_DIR" ]
then
    wget -P $OUTPUT_DIR/ -c https://www.gutenberg.org/files/2701/old/moby10b.txt
    PROCESSED_FILE="$OUTPUT_DIR/moby10b.concat.txt"
    cat $OUTPUT_DIR/moby10b.txt | sed 's/^\s*$//g' | tr '\r\n' ' ' > ${PROCESSED_FILE}
    echo "\n" >> ${PROCESSED_FILE}
else
    echo "Invalid output directory: $OUTPUT_DIR"
    echo "Usage: $0 OUTPUT_DIR"
    exit 1
fi
