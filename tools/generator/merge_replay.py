import argparse
import json
import os
from rich.progress import track

# python3 merge_replay.py --dir ./10_10 --output merged_replay.txt

def parse_args():
    parser = argparse.ArgumentParser()
    parser.add_argument("--dir", type=str, default="./")
    parser.add_argument("--output", type=str)
    return parser.parse_args()


if __name__ == '__main__':
    args = parse_args()
    if args.output is None:
        args.output = "merged_replay.txt"
    g = os.walk(args.dir)
    f = list()  # stored opened files
    replays = list()    # read text content from f
    print("Finding txt files...")
    for path, dir_list, file_list in g:
        for file_name in file_list:
            if file_name[-4:] == ".txt":
                print(os.path.join(args.dir, file_name))
                f.append(open(os.path.join(path, file_name), "r"))

    for _ in f:
        replays.append(_.read().split('\n'))

    out_f = open(os.path.join(args.dir, args.output), "w")
    #print(replays)[len(replays) - 1]
    step_count = len(replays[0])-1
    for step_index in track(range(step_count)):
        for replay_index in range(len(replays)):
            if replay_index == 0:   # read roadnet of each step from first replay file
                roadnet = replays[replay_index][step_index].split(";")[1]

            out_f.write(replays[replay_index]
                        [step_index].split(";")[0])
        out_f.write(";" + roadnet + "\n")

    out_f.close()

    # close all opened files
    for _ in f:
        _.close()
