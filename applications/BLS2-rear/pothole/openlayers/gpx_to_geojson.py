#!/opt/local/bin/python3.7
import pandas as pd
from gpxcsv import gpxtolist
from geojson import Feature, FeatureCollection, Point, LineString
import shapely.geometry
import shapely.ops
import json
import sys
import os
import re 


if(len(sys.argv)<2):
 print("usage: gpx_to_geojson.py <import_directory>")
 sys.exit(0)

import_path = sys.argv[1]

hausdorff_distance_trigger = 0.000050
geom_merge_leftover_len_trigger = 6

all_features = []
new_features = []
overlapping_features = []
index = 0



def geom_remove(geojson_features, geom):

   linestrings_to_remove = []
   prev_coord = (0,0)

   print("removing geometry: ", end="")
   print(geom)

   for coord in geom.coords:
     if (prev_coord[0] == 0) and (prev_coord[1] == 0):
      pass
     else:
      linestrings_to_remove.append(prev_coord+coord)
     prev_coord = coord

   for ls in linestrings_to_remove:
    for feat in geojson_features:
      if( (ls[0] == feat.geometry.coordinates[0][0]) and
          (ls[1] == feat.geometry.coordinates[0][1]) and
          (ls[2] == feat.geometry.coordinates[1][0]) and
          (ls[3] == feat.geometry.coordinates[1][1]) ):
         print("removed vector:"+str(feat.geometry.coordinates[0][0])+","+str(feat.geometry.coordinates[0][1])+","+str(feat.geometry.coordinates[1][0])+","+str(feat.geometry.coordinates[1][1]))
         geojson_features.remove(feat)


for filename in os.listdir(import_path):
    infile = os.path.join(import_path,filename)
    if os.path.isfile(infile):
     if(re.match('.*\.gpx$',infile)):

       new_feaures = []

       print("adding "+infile)
       df = pd.DataFrame(gpxtolist(infile))
       for row in df.index:
        if(row >= df.index.size-1):
          break
        f = Feature(geometry=LineString([(df['lon'][row], df['lat'][row]), (df['lon'][row+1], df['lat'][row+1])]), properties={"shock":df['shock'][row+1],"ele":df['ele'][row+1],"time":df['time'][row+1]})

        found_overlap = False

       # vector deduplication / merging data of similar vectors
       # compare currently processed vector with the set of already processed vectors - if "similar" vector already exists in the set, merge data from processsed vector to already existing one, and discard processed vector

        ls1 = shapely.geometry.LineString([(f.geometry.coordinates[0][0],f.geometry.coordinates[0][1]),(f.geometry.coordinates[1][0],f.geometry.coordinates[1][1])])
        for test_feat in all_features:
           ls2 = shapely.geometry.LineString([(test_feat.geometry.coordinates[0][0],test_feat.geometry.coordinates[0][1]),(test_feat.geometry.coordinates[1][0],test_feat.geometry.coordinates[1][1])])
           hd = ls1.hausdorff_distance(ls2)
           if(hd<hausdorff_distance_trigger):  
             #print("vector overlap detected! hd:"+str(hd))
             overlapping_features.append(f)
             found_overlap = True

        if(not found_overlap):
         if(f not in new_features):
           new_features.append(f)

        index += 1
        print(">> "+str(index)+" <<", end="\r")

       converted_shapely_feats = []

       # remove unconnected vectors left after merging
       # print("remove unconnected")
       for vrfied_feat in new_features:
       #  connected = False
       #  for feat in new_features:
       #    if vrfied_feat != feat:
       #      if( ((vrfied_feat.geometry.coordinates[0][0] == feat.geometry.coordinates[1][0]) and
       #            (vrfied_feat.geometry.coordinates[0][1] == feat.geometry.coordinates[1][1])) or
       #          ((vrfied_feat.geometry.coordinates[1][0] == feat.geometry.coordinates[0][0]) and
       #            (vrfied_feat.geometry.coordinates[1][1] == feat.geometry.coordinates[0][1])) ):
       #             connected = True
       #  if not connected:
       #    print("found unconnected vector - removing")
       #    new_features.remove(vrfied_feat)
 
         shp_feat = shapely.geometry.LineString([(vrfied_feat.geometry.coordinates[0][0],vrfied_feat.geometry.coordinates[0][1]),(vrfied_feat.geometry.coordinates[1][0],vrfied_feat.geometry.coordinates[1][1])]) 
         converted_shapely_feats.append(shp_feat)

       # remove short, segmented geometries (leftover after merge)
       print("remove short, unconnected segments:")
       ls = shapely.ops.linemerge(converted_shapely_feats)
       try:
        for geom in ls.geoms:
          if(len(geom.coords)<geom_merge_leftover_len_trigger):
           geom_remove(new_features,geom)
       except:
        # geometry is contigous and was returned as a single LineString
        pass

    all_features.extend(new_features)


feature_collection = FeatureCollection(features=all_features)

with open("combined.json", 'w', encoding='utf-8') as f:
    json.dump(feature_collection, f, ensure_ascii=False, indent=4)


