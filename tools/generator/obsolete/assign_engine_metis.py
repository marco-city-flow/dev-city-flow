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

# --roadnetFile Shuanglong.json --dir .\tools\generator
# --roadnetFile roadnet_10_10.json --dir .\tools\generator
# --roadnetFile nanjingmega.json --dir .\tools\generator --engineNum 200


def parse_args():
    parser = argparse.ArgumentParser()
    parser.add_argument("--rowNum", type=int)
    parser.add_argument("--colNum", type=int)
    parser.add_argument("--engineNum", type=int, default=4)
    parser.add_argument("--roadnetFile", type=str)
    parser.add_argument("--dir", type=str, default="./")
    parser.add_argument("--output", type=str)
    parser.add_argument("--turn", action="store_true")
    return parser.parse_args()


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
    k=0
    for intersection in track(intersections):
        if k%20==0:
            x = intersection['point']['x']
            y = intersection['point']['y']
            engine = intersection['engine']
            ax.scatter(x, y,  c=colors[engine], alpha=0.6)
        k+=1
    plt.show()


def distance_sum(intersections, n):
    distances = [0 for i in range(n)]
    for intersection in track(intersections):
        distances[intersection['engine']] += intersection['distanceToMidpoints']
    return distances


def to_adjacency_list(intersections, roads, road_id_index,intersection_id_index):
    adjacenccy_list = []
    for intersection in track(intersections):
        cur_list = []
        for road in intersection['roads']:
            _ = roads[road_id_index[road]]
            # if _['startIntersection'] is not intersection['id']:
            #     cur_list.append(
            #         intersection_id_index[_['startIntersection']])
            # elif _['endIntersection'] is not intersection['id']:
            #     cur_list.append(
            #         intersection_id_index[_['endIntersection']])
            # else:
            #     raise Exception(
            #         'start- and endIntersection both the same as ', intersection['id'])
            if _['endIntersection'] == intersection['id']:
                pass
            else:
                cur_list.append(
                    intersection_id_index[_['endIntersection']])
        # if len(cur_list) == 8:
        #     adjacenccy_list.append(np.array(cur_list[:4]))
        # if len(cur_list) == 2:
        #     adjacenccy_list.append(np.array(cur_list[:1]))
        adjacenccy_list.append(np.array(cur_list))
    return adjacenccy_list


if __name__ == '__main__':
    # Get arguments
    args = parse_args()
    if args.roadnetFile is None:
        if args.rowNum and args.colNum:
            args.roadnetFile = "roadnet_%d_%d%s.json" % (
                args.rowNum, args.colNum, "_turn" if args.turn else "")
        else:
            raise Exception('Invalid arguments for input file!')
    number_of_engines = args.engineNum

    with open(os.path.join(args.dir, args.roadnetFile), "r") as load_f:
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
    num = 0
    for i in adjacency_list:
        num += len(i)
    print('adjacency list: ', num)
    # print('adjacency list: ', adjacency_list)
    import networkx as nx
    G = nx.Graph()
    list_net_nodes = [i for i in range(len(adjacency_list))]
    list_net_edges = []
    # pos={}
    for i,node in enumerate(adjacency_list):
        for j in node:
            list_net_edges.append((i,j))
        # pos[i] = (int(load_dict['intersections'][i]['point']['x']),int(load_dict['intersections'][i]['point']['y']))
    G.add_nodes_from(list_net_nodes)
    G.add_edges_from(list_net_edges)
    # nx.draw(G, pos, with_labels=True)
    # plt.show()

    # modi_adjacency_list = []
    # xadj = [0]
    # weight = []
    # for i in adjacency_list:
    #     for j in i:
    #         modi_adjacency_list.append(j)
    #     xadj.append(len(modi_adjacency_list))
    #     weight.append(10)
    # n_cuts, membership = pymetis.part_graph(
    #     nparts=number_of_engines, adjncy=modi_adjacency_list,xadj=xadj,eweights=weight,recursive=True)
    # n_cuts, membership = pymetis.part_graph(
        # nparts=number_of_engines, adjacency=adjacency_list,recursive=True)
    import metis
    n_cuts, membership = metis.part_graph(
        graph=G, nparts=number_of_engines, recursive=False)
    print('number of cuts: ', n_cuts)
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
    edge_cut = 0
    same_eng = 0
    for road in load_dict['roads']:
        road['engine1'] = load_dict['intersections'][intersection_id_index[road['startIntersection']]]['engine']
        road['engine2'] = load_dict['intersections'][intersection_id_index[road['endIntersection']]]['engine']
        if road['engine1'] != road['engine2']:
            edge_cut += 1
        else:
            same_eng += 1
        #print(road['id'], engine1, engine2)
    print("Number of edge cut: " + str(edge_cut))
    print("Number of same_eng: " + str(same_eng))
        #print(road['id'], engine1, engine2)

    # Save JSON file
    print('Saving JSON file...')
    if args.output:
        save_f = open(os.path.join(args.dir, args.output), "w")
    else:
        save_f = open(os.path.join(args.dir, 'output', args.roadnetFile), "w")
    json.dump(load_dict, save_f, indent=2)

    print('Distance sum of each intersection:')
    print(distance_sum(load_dict['intersections'], number_of_engines))

    # Scatter intersections
    print('Scatterring intersections...')
    scatter_intersections(load_dict['intersections'])
