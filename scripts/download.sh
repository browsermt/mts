#!/bin/bash

set -x;

if [ -z "$1" ]
then
    echo "Error: No directory specified"
else
    OUTPUT_DIR="$1";
    mkdir -p $OUTPUT_DIR
    mkdir -p $OUTPUT_DIR/wngt20 && bash download-wngt20.sh $OUTPUT_DIR/wngt20
    mkdir -p $OUTPUT_DIR/moby-dick/ && bash download-moby10b.sh $OUTPUT_DIR/moby-dick
    python3 download-student.py -o $OUTPUT_DIR/students | bash
fi
