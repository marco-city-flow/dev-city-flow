# Generate all-in-one JSON config file (w/ engine-wise configurations) from a JSON config file (w/o engine-wise configurations)
# Input: A single config file in JSON format, which should include number of engines and prefixes of each config file for each engine
# Output: An all-in-one config file, which should then be used to create unique config files for every engine
import argparse
import json
import os

# --inConfigFile config_100_100.json --dir ./tools/generator/100_100_m --output all_in_one_config_100_100.json


def parse_args():
    parser = argparse.ArgumentParser()
    parser.add_argument("--rowNum", type=int)
    parser.add_argument("--colNum", type=int)
    parser.add_argument("--inConfigFile", type=str)
    parser.add_argument("--dir", type=str, default="./")
    parser.add_argument("--turn", action="store_true")
    parser.add_argument("--output", type=str)
    return parser.parse_args()


if __name__ == '__main__':
    # Get arguments
    args = parse_args()
    if args.inConfigFile is None:
        if args.rowNum and args.colNum:
            args.configFile = "config_%d_%d%s.json" % (
                args.rowNum, args.colNum, "_turn" if args.turn else "")
        else:
            raise Exception('Invalid arguments for input file!')
    if args.output is None:
        args.output = "all_in_one_%s" % (args.inConfigFile)

    with open(os.path.join(args.dir, args.inConfigFile), "r") as load_f:
        config_dict = json.load(load_f)  # JSON read as dictionary
        load_f.close()

    number_of_engines = config_dict['engineNumber']
    # we currently put all config files in one directory
    configs_dir = config_dict['dir']
    flowFilePrefix = config_dict['flowFilePrefix']
    configFilePrefix = config_dict['configFilePrefix']
    roadnetLogFilePrefix = config_dict['roadnetLogFilePrefix']
    replayLogFilePrefix = config_dict['replayLogFilePrefix']

    del config_dict['engineNumber']
    del config_dict['flowFilePrefix']
    del config_dict['configFilePrefix']
    del config_dict['roadnetLogFilePrefix']
    del config_dict['replayLogFilePrefix']

    engines = [dict() for _ in range(number_of_engines)]

    for i in range(number_of_engines):
        engine_dict = dict()
        engine_dict['engineDir'] = configs_dir
        engine_dict['flowFile'] = flowFilePrefix + "_"+str(i+1)+".json"
        engine_dict['configFile'] = configFilePrefix + "_"+str(i+1)+".json"
        engine_dict['roadnetLogFile'] = roadnetLogFilePrefix + \
            "_"+str(i+1)+".json"
        engine_dict['replayLogFile'] = replayLogFilePrefix + \
            "_"+str(i+1)+".txt"
        engines[i] = engine_dict

    config_dict['engines'] = engines

    save_f = open(os.path.join(args.dir, args.output), "w")
    json.dump(config_dict, save_f, indent=2)
