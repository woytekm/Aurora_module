#!/opt/local/bin/python3.7
import pandas as pd
from gpxcsv import gpxtolist
from geojson import Feature, FeatureCollection, Point, LineString
import shapely.geometry
import json
import sys
import os
import re 

import_path= 'gpx_import'
features = []
overlapping_features = []
index = 0

for filename in os.listdir(import_path):
    infile = os.path.join(import_path,filename)
    if os.path.isfile(infile):
     if(re.match('.*\.gpx$',infile)):
       print("adding "+infile)
       df = pd.DataFrame(gpxtolist(infile))
       for row in df.index:
        if(row >= df.index.size-1):
          break
        f = Feature(geometry=LineString([(df['lon'][row], df['lat'][row]), (df['lon'][row+1], df['lat'][row+1])]), properties={"shock":df['shock'][row+1],"ele":df['ele'][row+1],"time":df['time'][row+1]})

        # Does not work. It detects only identical vectors, not similar ones.
        #
        #ls1 = shapely.geometry.LineString([(f.geometry.coordinates[0][0],f.geometry.coordinates[0][1]),(f.geometry.coordinates[1][0],f.geometry.coordinates[1][1])])
        #for test_feat in features:
        #  ls2 = shapely.geometry.LineString([(test_feat.geometry.coordinates[0][0],test_feat.geometry.coordinates[0][1]),(test_feat.geometry.coordinates[1][0],test_feat.geometry.coordinates[1][1])])
        #  if(ls1.almost_equals(ls2,decimal=9)):
        #    print("vector overlap detected!")
        #    overlapping_features.append(f)         

        # A bit better one (Hausdorff distance)::
        # ls1 = shapely.geometry.LineString([(f.geometry.coordinates[0][0],f.geometry.coordinates[0][1]),(f.geometry.coordinates[1][0],f.geometry.coordinates[1][1])])
        # for test_feat in features:
        #   ls2 = shapely.geometry.LineString([(test_feat.geometry.coordinates[0][0],test_feat.geometry.coordinates[0][1]),(test_feat.geometry.coordinates[1][0],test_feat.geometry.coordinates[1][1])])
        #   hd = ls1.hausdorff_distance(ls2)
        #   if(hd<0.000089):                 # need to tweak this value
        #     print("vector overlap detected! hd:"+str(hd))
        #     overlapping_features.append(f)

        features.append(f)
        index += 1
        print(">> "+str(index)+" <<", end="\r")

feature_collection = FeatureCollection(features=features)

with open("combined.json", 'w', encoding='utf-8') as f:
    json.dump(feature_collection, f, ensure_ascii=False, indent=4)


