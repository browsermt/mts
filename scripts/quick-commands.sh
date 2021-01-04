#/bin/bash


COMMON_ARGS=(
    -m ../models/enes/model.intgemm.alphas.bin
    --vocabs ../models/enes/vocab.esen.spm ../models/enes/vocab.esen.spm
    --cpu-threads 5 
    --source-language en --target-language pl 
    --ssplit-mode paragraph
    --beam-size 1
    --skip-cost # Sets mode to translation, what does this do?
    --shortlist ../models/enes/lex.s2t.gz 50 50
)

COMMON_ENPL_ARGS=(
    -m ../models/model.npz.best-bleu-detok.npz 
    --vocabs ../models/en.32768.spm ../models/pl.32768.spm 
    --cpu-threads 5 
    --source-language en --target-language pl 
    --ssplit-mode paragraph
)



function mrest-server {
    REST_SERVER_ARGS=(
        "${COMMON_ARGS[@]}"
        --server-root ../src/service/rest/ 
        --log-level info
    )
    ./rest-server ${REST_SERVER_ARGS[@]};
}


function bergamot {
    set -x;
    BERGAMOT_ARGS=(
        "${COMMON_ARGS[@]}"
        --max-input-sentence-tokens 128
        --max-input-tokens 1000
        --log-level info
    )
    ./main ${BERGAMOT_ARGS[@]};
    set +x;
}

function data-setup {
    mkdir ../models/enes/ \
        && cd ../models/enes \
        && wget -c http://data.statmt.org/bergamot/models/esen/enes.student.tiny11.tar.gz 
        && tar xvzf enes.student.tiny11.tar.gz;
    mkdir -p ../models/esen \
        && cd ../models/esen
        && wget -c http://data.statmt.org/bergamot/models/esen/esen.student.tiny11.tar.gz
        && tar xvzf esen.student.tiny11.tar.gz;
}
