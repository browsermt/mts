import re


if __name__ == '__main__':
    pattern = re.compile("real\t([0-9ms\.]*)")
    for batchsize in [512, 1024, 2048, 4096]:
        threads_entries = []
        for thread in range(1, 8+1):
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
        row = map(str, threads_entries)
        print('\t'.join(row))


