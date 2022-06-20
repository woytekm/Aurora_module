#!/opt/local/bin/python3.7

import pandas as pd
from gpxcsv import gpxtolist
from geojson import Feature, FeatureCollection, Point, LineString
import json
import sys


df1 = pd.DataFrame(gpxtolist(sys.argv[1]))
df2 = pd.DataFrame(gpxtolist(sys.argv[2]))

features = []

# columns used for constructing geojson object
#features = df.apply(lambda row: Feature(geometry=Point((float(row['lon']), float(row['lat'])), properties={"shock":row['shock'],"ele":row['ele'],"time":row['time']})),axis=1).tolist()

for df in df1,df2:
 for row in df.index:
  if(row >= df.index.size-1):
   break
  f = Feature(geometry=LineString([(df['lon'][row], df['lat'][row]), (df['lon'][row+1], df['lat'][row+1])]), properties={"shock":df['shock'][row+1],"ele":df['ele'][row+1],"time":df['time'][row+1]})
  features.append(f)

# whole geojson object
feature_collection = FeatureCollection(features=features)

out_fname = sys.argv[1].split(".")[0]

with open("combined.json", 'w', encoding='utf-8') as f:
    json.dump(feature_collection, f, ensure_ascii=False, indent=4)


