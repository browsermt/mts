#/bin/bash

export MKL_NUM_THREADS=1
export NUMEXPR_NUM_THREADS=1
export OMP_NUM_THREADS=1


STUDENTS="../assets/students"

COMMON_ARGS=(
    -m $STUDENTS/enes/model.intgemm.alphas.bin
    --vocabs $STUDENTS/enes/vocab.esen.spm $STUDENTS/enes/vocab.esen.spm
    --ssplit-mode paragraph
    --beam-size 1 --skip-cost # Sets mode to translation, what does this do?
    --shortlist $STUDENTS/enes/lex.s2t.gz 50 50
    # --queue-timeout 5 # ms timeout
    --int8shiftAlphaAll # What does this do?
)

function bergamot {
    BERGAMOT_ARGS=(
        "${COMMON_ARGS[@]}"
        --cpu-threads 6
        --log-level all
        --max-input-sentence-tokens 128
        --max-input-tokens 512
    )
    echo "./main ${BERGAMOT_ARGS[@]}";
    ./main ${BERGAMOT_ARGS[@]};
}

function bergamot-profile {
    BENCHMARK_OUTPUTS="benchmark-outputs"; mkdir -p $BENCHMARK_OUTPUTS;

    THREADS=$1;
    MAX_INPUT_TOKENS=$2;
    BATCH_TOKENS=$3;

    PROFILE_ARGS=(
        --max-input-sentence-tokens "$MAX_INPUT_TOKENS"
        --max-input-tokens "$BATCH_TOKENS"
        --cpu-threads "$THREADS"
        --log-level off
    )

    TAG="threads(${THREADS})_batch_tokens($BATCH_TOKENS)_max_input_sentence_tokens($MAX_INPUT_TOKENS)"
    echo $TAG;
    { time ./main ${COMMON_ARGS[@]} ${PROFILE_ARGS[@]} < ../assets/moby-dick/moby10b.concat.txt;}  &> $BENCHMARK_OUTPUTS/$TAG.time.txt ;
    # gprof ./main gmon.out > $BENCHMARK_OUTPUTS/$TAG.analysis.txt;
}

function run-combos {
    MAX_INPUT_TOKENS="128"
    for THREAD in 28 24 20 16 12 8 4 2 1
    do
        for BATCH_TOKENS in 2048 1024 512 256 128
        do 
            bergamot-profile $THREAD $MAX_INPUT_TOKENS $BATCH_TOKENS;
        done;
    done;
}

function data-setup {
    (mkdir -p ../models/enes/ \
        && cd ../models/enes \
        && wget -c http://data.statmt.org/bergamot/models/esen/enes.student.tiny11.tar.gz \
        && tar xvzf enes.student.tiny11.tar.gz) ;
    (mkdir -p ../models/esen \
        && cd ../models/esen \
        && wget -c http://data.statmt.org/bergamot/models/esen/esen.student.tiny11.tar.gz \
        && tar xvzf esen.student.tiny11.tar.gz );
}
