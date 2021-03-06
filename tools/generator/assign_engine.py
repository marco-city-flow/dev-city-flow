import argparse
import json
import os
import math
import heapq
# from sklearn.cluster import KMeans
import pandas as pd
import matplotlib.pyplot as plt

# --roadnetFile Shuanglong.json --dir .\tools\generator
# --roadnetFile roadnet_10_10.json --dir .\tools\generator

z
def parse_args():
    parser = argparse.ArgumentParser()
    parser.add_argument("--rowNum", type=int)
    parser.add_argument("--colNum", type=int)
    parser.add_argument("--roadnetFile", type=str)
    parser.add_argument("--dir", type=str, default="./")
    parser.add_argument("--output", type=str)
    parser.add_argument("--turn", action="store_true")
    return parser.parse_args()


def group_roads_by_length(roads, n):
    lists = [[] for _ in range(n)]
    number_list = list(roads)
    totals = [(0, i) for i in range(n)]
    heapq.heapify(totals)
    for _ in range(len(roads)):
        total, index = heapq.heappop(totals)
        lists[index].append(roads[_])
        heapq.heappush(totals, (total + roads[_]['length'], index))
        number_list[_] = index
    return number_list, lists


def group_roads_by_midpoint(roads, n):
    kmeans = KMeans(n_clusters=n,
                    init='k-means++',
                    n_init=10,
                    max_iter=300,
                    tol=0.0001,
                    verbose=0,
                    random_state=None,
                    copy_x=True,
                    algorithm='auto'
                    )
    midpoints = pd.DataFrame(data=roads, columns=['midpoint'])[
        'midpoint'].apply(pd.Series)
    kmeans.fit(midpoints)
    labels = list()
    for _ in range(len(kmeans.labels_)):
        labels.append(int(kmeans.labels_[_]))
    return labels


def group_intersections_by_point(intersections, n):
    kmeans = KMeans(n_clusters=n,
                    init='k-means++',
                    n_init=10,
                    max_iter=300,
                    tol=0.0001,
                    verbose=0,
                    random_state=None,
                    copy_x=True,
                    algorithm='auto'
                    )
    points = pd.DataFrame(data=intersections, columns=['point'])[
        'point'].apply(pd.Series)
    kmeans.fit(points)
    labels = list()
    for _ in range(len(kmeans.labels_)):
        labels.append(int(kmeans.labels_[_]))
    return labels


def polygonize(load_dict):
    for intersection in load_dict['intersections']:
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


def group_intersections_by_length(intersections, n):
    lists = [[] for _ in range(n)]
    number_list = list(intersections)
    totals = [(0, i) for i in range(n)]
    heapq.heapify(totals)
    for _ in range(len(intersections)):
        total, index = heapq.heappop(totals)
        lists[index].append(intersections[_])
        heapq.heappush(
            totals, (total + intersections[_]['distanceToMidpoints'], index))
        number_list[_] = index
    return number_list, lists


def scatter_intersections(intersections):
    fig = plt.figure()
    ax = plt.subplot()
    colors = ['red', 'green', 'blue', 'yellow',
              'cyan', 'gray', 'purple', 'orange']
    for intersection in intersections:
        x = intersection['point']['x']
        y = intersection['point']['y']
        engine = intersection['engine']
        ax.scatter(x, y,  c=colors[engine], alpha=0.6)
    plt.show()


def distance_sum(intersections, n):
    distances = [0 for i in range(n)]
    for intersection in intersections:
        distances[intersection['engine']
                  ] = distances[intersection['engine']]+intersection['distanceToMidpoints']
    return distances


if __name__ == '__main__':
    # The number of engines available
    number_of_engines = 4

    # Get arguments
    args = parse_args()
    if args.roadnetFile is None:
        if args.rowNum or args.colNum:
            args.roadnetFile = "roadnet_%d_%d%s.json" % (
                args.rowNum, args.colNum, "_turn" if args.turn else "")
        else:
            raise Exception('Invalid arguments for input file!')
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

    # Polygonize each intersection
    polygonize(load_dict)

    #index_list_intersections, list_polygon = group_intersections_by_length(load_dict['intersections'], number_of_engines)

    '''
        # Assign each road to 1 or 2 engine(s), based on clustering of midpoint and length grouping
        index_list_by_length, list_by_length = group_roads_by_length(
            load_dict['roads'], number_of_engines)
        index_list_by_midpoint = group_roads_by_midpoint(
            load_dict['roads'], number_of_engines)

        for _ in range(len(load_dict['roads'])):
            load_dict['roads'][_]['engine1'] = index_list_by_length[_]
            load_dict['roads'][_]['engine2'] = index_list_by_midpoint[_]

        
        for _ in range(number_of_engines):
            print(len(list_by_length[_]))
            print(sum(t['length'] for t in list_by_length[_]))

        with pd.option_context('display.max_rows', 500, 'display.max_columns', 5):
            print(pd.DataFrame(data=load_dict['roads'], columns=[
                'id', 'engine1', 'engine2', 'midpoint', 'length']))
    '''

    index_list_intersections = group_intersections_by_point(
        load_dict['intersections'], number_of_engines)

    # Save engine information of each engine
    for _ in range(len(load_dict['intersections'])):
        load_dict['intersections'][_]['engine'] = index_list_intersections[_]

    # Assign each road to 1 or 2 engine(s), based on grouping of connected intersections
    for road in load_dict['roads']:
        for _ in range(len(load_dict['intersections'])):
            __ = load_dict['intersections'][_]
            if __['id'] == road['startIntersection']:
                engine1 = index_list_intersections[_]
        for _ in range(len(load_dict['intersections'])):
            __ = load_dict['intersections'][_]
            if __['id'] == road['endIntersection']:
                engine2 = index_list_intersections[_]
        road['engine1'] = engine1
        road['engine2'] = engine2
        #print(road['id'], engine1, engine2)

    # Save JSON file
    if args.output:
        save_f = open(os.path.join(args.dir, args.output), "w")
    else:
        save_f = open(os.path.join(args.dir, 'output', args.roadnetFile), "w")
    json.dump(load_dict, save_f, indent=2)

    print(distance_sum(load_dict['intersections'], number_of_engines))

    # Scatter intersections
    scatter_intersections(load_dict['intersections'])
