######################################################################################
# Assign engine for each intersection, road and flow and store them for each engine,
# details specified by a JSON config file (w/o engine-wise configurations)
#
# Input: A single config file in JSON format, which should include number of engines
#        and prefixes of each config file for each engine
#        as well as many other fileds... Example can be found in /Examples
# outRoadnet: Engine-wise flow file and config file, in respective directory
######################################################################################

import argparse
import json
import os
import math
import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
import random
import time
import pymetis
from rich.progress import track

# cwd: /tools/generator
# --inConfigFile config_20_20.json --dir ./20_20_m --outRoadnet out_roadnet_20_20.json --outConfigFile out_config_20_20.json


def polygonize(load_dict, id_index=''):
    if id_index:
        for intersection in track(load_dict['intersections']):
            point = intersection['point']
            ix = point['x']
            iy = point['y']
            dis = 0
            for road in intersection['roads']:
                _ = load_dict['roads'][id_index[road]]
                mx = _['midpoint']['x']
                my = _['midpoint']['y']
                length = math.sqrt((ix-mx)**2+((iy-my)**2))
                dis = dis+length
            intersection['distanceToMidpoints'] = dis
    else:
        for intersection in track(load_dict['intersections']):
            point = intersection['point']
            ix = point['x']
            iy = point['y']
            dis = 0
            for road in intersection['roads']:
                for _ in load_dict['roads']:
                    if _['id'] == road:
                        mx = _['midpoint']['x']
                        my = _['midpoint']['y']
                        length = math.sqrt((ix-mx)**2+((iy-my)**2))
                        dis = dis+length
            intersection['distanceToMidpoints'] = dis


def scatter_intersections(intersections):
    fig = plt.figure()
    ax = plt.subplot()
    # colors = ['red', 'green', 'blue', 'yellow',
    #          'cyan', 'gray', 'purple', 'orange']
    def get_colors(n): return list(map(lambda i: "#" + "%06x" %
                                       random.randint(0, 0xFFFFFF), range(n)))
    colors = get_colors(number_of_engines)
    for intersection in track(intersections):
        x = intersection['point']['x']
        y = intersection['point']['y']
        engine = intersection['engine']
        ax.scatter(x, y,  c=colors[engine], alpha=0.6)
    plt.show()


def distance_sum(intersections, n):
    distances = [0 for i in range(n)]
    for intersection in track(intersections):
        distances[intersection['engine']
                  ] += intersection['distanceToMidpoints']
    return distances


def to_adjacency_list(intersections, roads, road_id_index, intersection_id_index):
    adjacenccy_list = []
    for intersection in track(intersections):
        cur_list = []
        for road in intersection['roads']:
            _ = roads[road_id_index[road]]
            if _['startIntersection'] is not intersection['id']:
                cur_list.append(
                    intersection_id_index[_['startIntersection']])
            elif _['endIntersection'] is not intersection['id']:
                cur_list.append(
                    intersection_id_index[_['endIntersection']])
            else:
                raise Exception(
                    'start- and endIntersection both the same as ', intersection['id'])
        adjacenccy_list.append(np.array(cur_list))
    return adjacenccy_list


def parse_args():
    parser = argparse.ArgumentParser()
    parser.add_argument("--rowNum", type=int)
    parser.add_argument("--colNum", type=int)
    parser.add_argument("--inConfigFile", type=str)
    parser.add_argument("--dir", type=str, default="./")
    parser.add_argument("--turn", action="store_true")
    parser.add_argument("--outRoadnet", type=str)
    parser.add_argument("--outConfigFile", type=str)
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
    if args.outConfigFile is None:
        args.outConfigFile = "out_config_%s" % (args.inConfigFile)
    # default outRoadnet will be set later...

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

    if args.outRoadnet is None:
        args.outRoadnet = "out_%s" % (config_dict['roadnetFile'])
    config_dict['roadnetFile'] = args.outRoadnet

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

    # Save config file (will be used by multiprocessor)
    with open(os.path.join(args.dir, args.outConfigFile), "w") as save_f:
        json.dump(config_dict, save_f, indent=2)
        save_f.close()

 ###########################################

    number_of_engines = len(config_dict['engines'])
    engine_dicts = config_dict['engines']

    with open(os.path.join(config_dict['dir'], config_dict['roadnetFile']), "r") as load_f:
        load_dict = json.load(load_f)  # JSON read as dictionary
        load_f.close()
    # Calculate length and midpoint
    for road in load_dict['roads']:
        dx = road['points'][0]['x'] - road['points'][1]['x']
        dy = road['points'][0]['y'] - road['points'][1]['y']
        mx = (road['points'][0]['x'] + road['points'][1]['x'])/2
        my = (road['points'][0]['y'] + road['points'][1]['y'])/2
        length = math.sqrt((dx**2)+(dy**2))
        road['length'] = length
        road['midpoint'] = {'x': mx, 'y': my}

    print('intersections: ', len(load_dict['intersections']))
    print('roads: ', len(load_dict['roads']))

    road_id_index = {load_dict['roads'][i]['id']: i for i in range(len(load_dict['roads']))}
    intersection_id_index = {load_dict['intersections'][i]['id']: i for i in range(
        len(load_dict['intersections']))}

    # Polygonize each intersection
    print('Polygonizing intersections...')
    polygonize(load_dict, id_index=road_id_index)

    # To adjacency list
    adjacency_list = to_adjacency_list(
        load_dict['intersections'], load_dict['roads'], road_id_index, intersection_id_index)
    n_cuts, membership = pymetis.part_graph(
        number_of_engines, adjacency=adjacency_list)
    nodes_part = [np.argwhere(np.array(membership) == _).ravel()
                  for _ in range(number_of_engines)]
    for _ in nodes_part:
        print(_)

    # Save engine information of each intersection
    print('Saving engine information to dict...')
    for _ in range(len(nodes_part)):
        for intersection in nodes_part[_]:
            load_dict['intersections'][intersection]['engine'] = _

    # Assign each road to 1 or 2 engine(s), based on grouping of connected intersections
    print('Assigning engines to each road...')
    for road in load_dict['roads']:
        road['engine1'] = load_dict['intersections'][intersection_id_index[road['startIntersection']]]['engine']
        road['engine2'] = load_dict['intersections'][intersection_id_index[road['endIntersection']]]['engine']
        #print(road['id'], engine1, engine2)

    # All-in-one flow file to engine-wise flow files
    with open(os.path.join(config_dict['dir'], config_dict['flowFile']), "r") as load_f:
        flow_dict = json.load(load_f)  # JSON read as dictionary
        load_f.close()
    flows = list([] for _ in range(number_of_engines))
    for flow in flow_dict:
        road = load_dict['roads'][road_id_index[flow['route'][0]]]
        flows[road['engine1']].append(flow)
    for _ in range(number_of_engines):
        print('Saving JSON file for engine ' + str(_+1) + '...')
        save_f = open(os.path.join(
            engine_dicts[_]['engineDir'], engine_dicts[_]['flowFile']), "w")
        json.dump(flows[_], save_f, indent=2)

    # Generate config file for each engine
    for _ in range(number_of_engines):
        engine_config = config_dict.copy()
        engine_config['dir'] = engine_config['engines'][_]['engineDir']
        engine_config['flowFile'] = engine_config['engines'][_]['flowFile']
        engine_config['roadnetLogFile'] = engine_config['engines'][_]['roadnetLogFile']
        engine_config['replayLogFile'] = engine_config['engines'][_]['replayLogFile']
        del engine_config['engines']
        print(engine_config)
        # dir/engineDir/configFile
        with open(os.path.join(engine_dicts[_]['engineDir'], engine_dicts[_]['configFile']), "w") as config_w:
            json.dump(engine_config, config_w, indent=2)
            config_w.close()

    # Save roadnet JSON file
    print('Saving roadnet JSON file...')
    if args.outRoadnet:
        save_f = open(os.path.join(args.dir, args.outRoadnet), "w")
    else:
        save_f = open(os.path.join(
            args.dir, 'outRoadnet', args.configFile), "w")
    json.dump(load_dict, save_f, indent=2)

    print('Distance sum of each intersection:')
    print(distance_sum(load_dict['intersections'], number_of_engines))

    # Scatter intersections
    print('Scatterring intersections...')
    scatter_intersections(load_dict['intersections'])
