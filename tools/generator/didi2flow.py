import argparse
import os
import json
import math
from rich.progress import track
import sumolib
from LngLatTransfer import LngLatTransfer

# cwd: /tools/generator
# --inFile gps_small --dir . --roadnetXML Chengdu.net.xml
# OBSOLETE: --inFile gps_small --dir .\tools\generator --roadnetJSON megaChengdu.json

# input lon, lat, get closest edge in sumo network


def get_closest_edge_id(net, radius, lon, lat):
    tol = 50
    it = 0
    x, y = net.convertLonLat2XY(lon, lat)
    edges = net.getNeighboringEdges(x, y, radius)
    # pick the closest edge
    while len(edges) == 0:
        if(it >= tol):
            raise Exception('Max iteration %d exceeded...' % (tol))
        radius *= 2
        edges = net.getNeighboringEdges(x, y, radius)
        it += 1

    # edge is not comparable
    distancesAndEdges = sorted([(dist, edge)
                                for edge, dist in edges], key=lambda x: x[0])
    dist, closestEdge = distancesAndEdges[0]
    return closestEdge.getID()


def parse_args():
    parser = argparse.ArgumentParser()
    parser.add_argument("--inFile", type=str)
    parser.add_argument("--roadnetJSON", type=str)
    parser.add_argument("--roadnetXML", type=str)
    parser.add_argument("--dir", type=str, default=".")
    parser.add_argument("--outFlowFile", type=str)
    return parser.parse_args()


if __name__ == '__main__':
    # Get arguments
    args = parse_args()
    if args.inFile is None:
        raise Exception('Invalid arguments for input file!')
    if args.roadnetJSON is None and args.roadnetXML is None:
        raise Exception('Invalid arguments for roadnet file!')
    if args.outFlowFile is None:
        args.outFlowFile = "%s_flow.json" % (args.inFile)

    '''# read JSON
        with open(os.path.join(args.dir, args.roadnetFile), "r") as load_f:
            roads = json.load(load_f)['roads']  # JSON read as dictionary
    '''

    od = list()

    with open(os.path.join(args.dir, args.inFile), "r") as lines:
        last_record = [0, 0, 0, 0, 0]
        record = [0, 0, 0, 0, 0]
        print('Reading lines from file...')
        for line in lines:
            last_record = record
            record = line.strip().split(',')
            if record[1] != last_record[1]:
                if(last_record[0]):
                    od.append(last_record)
                od.append(record)
            # for _ in record:
                # print(_)
        od.append(line.strip().split(','))

    # convert GCJ-02 to WGS-84
    print("Converting GCJ-02 to WGS-84...")
    coord_transfer = LngLatTransfer()
    for record in track(od):
        record[3], record[4] = coord_transfer.GCJ02_to_WGS84(
            float(record[3]), float(record[4]))

    flow = [None for _ in range(len(od) // 2)]

    # read sumo net
    print("Reading SUMO net...")
    net = sumolib.net.readNet(os.path.join(args.dir, args.roadnetXML))
    radius = 2

    # init flow
    for od_index in range(len(od) // 2):
        flow[od_index] = {
            "vehicle": {
                "length": 5.0,
                "width": 2.0,
                "maxPosAcc": 2.0,
                "maxNegAcc": 4.5,
                "usualPosAcc": 2.0,
                "usualNegAcc": 4.5,
                "minGap": 2.5,
                "maxSpeed": 16.67,
                "headwayTime": 1.5
            },
            "route": [],
            "interval": 2.0,
            "startTime": int(float(od[2 * od_index][2])),
            "endTime": int(float(od[2 * od_index+1][2]))
        }

    '''
    # brute-force, too slow
    for od_index in track(range(len(od) // 2)):
        xx = od[2 * od_index][3]
        yy = od[2 * od_index][4]
        min_d = 100000
        min_i = ''
        for road in roads:
            p1 = road['points'][0]
            p2 = road['points'][1]
            result = (p1['x'] - xx) * (p2['y'] - yy) - \
                (p2['x'] - xx) * (p1['y'] - yy)
            if abs(result) < min_d:
                min_d = result
                min_i = road['id']
        flow[od_index]['route'].append(min_i)

        xx = od[2 * od_index + 1][3]
        yy = od[2 * od_index + 1][4]
        min_d = 100000
        min_i = ''
        for road in roads:
            p1 = road['points'][0]
            p2 = road['points'][1]
            result = (p1['x'] - xx) * (p2['y'] - yy) - \
                (p2['x'] - xx) * (p1['y'] - yy)
            if abs(result) < min_d:
                min_d = result
                min_i = road['id']
        flow[od_index]['route'].append(min_i)
    '''

    # find closest edge for OD
    # OPTIONAL: improve performance with RTree
    # https://sumo.dlr.de/docs/Tools/Sumolib.html#locate_nearby_edges_based_on_the_geo-coordinate
    print("Calculating closest road for each OD...")
    for od_index in track(range(len(od) // 2)):
        lon = od[2 * od_index][3]
        lat = od[2 * od_index][4]
        flow[od_index]['route'].append(
            get_closest_edge_id(net, radius, lon, lat))

        lon = od[2 * od_index + 1][3]
        lat = od[2 * od_index + 1][4]
        flow[od_index]['route'].append(
            get_closest_edge_id(net, radius, lon, lat))

    # Save roadnet JSON file
    print('Saving roadnet JSON file...')
    if args.outFlowFile:
        save_f = open(os.path.join(args.dir, args.outFlowFile), "w")
    json.dump(flow, save_f, indent=2)
