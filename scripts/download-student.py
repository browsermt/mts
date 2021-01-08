import subprocess as sp
import os
import sys
from argparse import ArgumentParser
from itertools import permutations
from urllib.parse import urljoin

if __name__ == '__main__':

    parser = ArgumentParser("Download helper for student models")
    parser.add_argument('-o', '--output-directory', help='Output directory to store models', required=True)
    args = parser.parse_args()

    pairs = [
        ("es", "en"),
        ("de", "en")
    ]

    variants = [
        'tiny11'
    ]

    base_url ="http://data.statmt.org/bergamot/models/"

    if not os.path.exists(args.output_directory):
        print("Output directory  does not exist: {}".format(args.output_directory), file=sys.stderr)
        exit(1)

    for pair in pairs:
        for direction in permutations(pair, 2):
            for variant in variants:
                pairtag = ''.join(pair)
                dirtag = ''.join(direction)
                model_file = "{dirtag}.student.{variant}.tar.gz".format(dirtag=dirtag, variant=variant)
                relative_path = os.path.join(pairtag, model_file)
                url = urljoin(base_url, relative_path)

                download_dir = os.path.join(args.output_directory, dirtag)
                if not os.path.exists(download_dir):
                    os.mkdir(download_dir)

                local_path = os.path.join(download_dir, model_file)
                commands = [
                      "wget -P {download_dir} -c {url}".format(download_dir=download_dir, url=url),
                      "cd {download_dir} && tar xf {model_file} && cd -".format(download_dir=download_dir, model_file=model_file)
                ]

                for command in commands:
                    print(command)



