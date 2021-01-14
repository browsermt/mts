import re


if __name__ == '__main__':
    pattern = re.compile("real\t([0-9ms\.]*)")
    threads = [1, 2, 4, 8, 12, 16, 20, 24, 28] 
    print('\t'.join(map(str, [0] + threads)))
    for batchsize in [128, 256, 512, 1024, 2048]:
        threads_entries = []
        fmt = lambda x: '{:.2f}'.format(x)
        for thread in threads:
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
        row = list(map(fmt, threads_entries))

        print('\t'.join([str(batchsize)] + row))


