import re


if __name__ == '__main__':
    pattern = re.compile("real\t([0-9ms\.]*)")
    for batchsize in [256, 512, 1024]:
        threads_entries = []
        for thread in [1, 2, 4, 8, 16, 24, 28]:
            for max_token in [128]:
                fname = f"threads({thread})_batch_tokens({batchsize})_max_input_sentence_tokens({max_token}).time.txt"
                with open(fname) as fp:
                    contents = fp.read()
                    match = pattern.search(contents)
                    candidate = match.group(1)
                    minute, seconds = candidate.split('m')
                    seconds = seconds.replace('s', '')

                    minute = float(minute)
                    seconds = float(seconds)

                    time = 60*minute + seconds
                    # row = map(str, [thread, batchsize, time])
                    threads_entries.append(time)
                    
                    # print('\t'.join(row))
        fmt = lambda x: '{:.2f}'.format(x)
        row = map(fmt, threads_entries)
        print('\t'.join(row))


