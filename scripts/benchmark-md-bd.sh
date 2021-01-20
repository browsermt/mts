#!/bin/bash

MTS_DIR='/home/jphilip/code/mts'

WORDS=1024
INPUT_FILE="$MTS_DIR/assets/wngt20/sources.shuf"

COMMON_ARGS=(
    -m $MTS_DIR/assets/students/ende/model.intgemm.alphas.bin 
    --vocabs $MTS_DIR/assets/students/ende/vocab.deen.spm $MTS_DIR/assets/students/ende/vocab.deen.spm 
    --beam-size 1 --skip-cost --shortlist $MTS_DIR/assets/students/ende/lex.s2t.gz 50 50 
    --quiet --quiet-translation --int8shiftAlphaAll -w 128 
)



function run-mts {
    THREADS="$1"
    TAG="speed/cpu.mts.${THREADS}"
    MTS_ARGS=(
        --max-input-sentence-tokens $WORDS --max-input-tokens $WORDS --marian-decoder-alpha --ssplit-mode sentence 
        --cpu-threads ${THREADS} 
        --log ${TAG}.log -o ${TAG}.translated
    )

    $MTS_DIR/build/main "${COMMON_ARGS[@]}" "${MTS_ARGS[@]}" < $INPUT_FILE ;
}

function run-marian-decoder {
    THREADS="$1"
    TAG="speed/cpu.marian-decoder.${THREADS}"
    DECODER_ARGS=(
        --maxi-batch 1000000 --mini-batch-words $WORDS 
        --cpu-threads ${THREADS} 
        --log ${TAG}.log -o ${TAG}.translated
    )

    $MTS_DIR/build/marian-decoder "${COMMON_ARGS[@]}" "${DECODER_ARGS[@]}" < $INPUT_FILE;

}

set -x;
run-mts 64
run-marian-decoder 64
