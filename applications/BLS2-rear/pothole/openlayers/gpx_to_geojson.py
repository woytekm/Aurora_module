#!/opt/local/bin/python3.7
import pandas as pd
from gpxcsv import gpxtolist
from geojson import Feature, FeatureCollection, Point, LineString
from operator import attrgetter
import shapely.geometry
import shapely.ops
import json
import sys
import os
import re


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


class Track:

    idnum = 0
    dframe= None
    bbox_min_lon = 0
    bbox_min_lat = 0
    bbox_max_lon = 0
    bbox_max_lat = 0
    overlaps = []
    features = []

    def __init__(self,idn):
       self.idnum = idn
       self.features = []
       self.overlaps = []

    def populate_from_gpxlist(self,trackpoints):
       self.dframe = pd.DataFrame(trackpoints)
       self.bbox_min_lon = self.dframe['lon'].min()
       self.bbox_min_lat = self.dframe['lat'].min()
       self.bbox_max_lon = self.dframe['lon'].max()
       self.bbox_max_lat = self.dframe['lat'].max()

       for row in self.dframe.index:
        if(row >= self.dframe.index.size-1):
          break
        f = Feature(geometry=LineString([(self.dframe['lon'][row], self.dframe['lat'][row]), (self.dframe['lon'][row+1], self.dframe['lat'][row+1])]), properties={"shock":self.dframe['shock'][row+1],"ele":self.dframe['ele'][row+1],"time":self.dframe['time'][row+1]})
        self.features.append(f)
       self.update_bbox()

    def update_bbox(self):
       a = attrgetter("geometry.coordinates")
       self.bbox_min_lon = min([min([a(s)[0][0] for s in self.features]),min([a(s)[1][0] for s in self.features])])
       self.bbox_min_lat = min([min([a(s)[0][1] for s in self.features]),min([a(s)[1][1] for s in self.features])])
       self.bbox_max_lon = max([max([a(s)[0][0] for s in self.features]),max([a(s)[1][0] for s in self.features])])
       self.bbox_max_lat = max([max([a(s)[0][1] for s in self.features]),max([a(s)[1][1] for s in self.features])])

    def bbox(self):
       return self.bbox_min_lon,self.bbox_min_lat,self.bbox_max_lon,self.bbox_max_lat

    def overlap(self,othertrack):
       # idea from: https://rbrundritt.wordpress.com/2009/10/03/determining-if-two-bounding-boxes-overlap/
       rabx = abs(self.bbox_min_lon + self.bbox_max_lon - othertrack.bbox_min_lon - othertrack.bbox_max_lon)
       raby = abs(self.bbox_max_lat + self.bbox_min_lat - othertrack.bbox_max_lat - othertrack.bbox_min_lat)

       raxPrbx = self.bbox_max_lon - self.bbox_min_lon + othertrack.bbox_max_lon - othertrack.bbox_min_lon
       rayPrby = self.bbox_max_lat - self.bbox_min_lat + othertrack.bbox_max_lat - othertrack.bbox_min_lat

       if((rabx <= raxPrbx) and (raby <= rayPrby)):
          return True
       return False

    def overlap_bbox(self,othertrack):
       min_lon = max([self.bbox_min_lon,othertrack.bbox_min_lon])
       min_lat = max([self.bbox_min_lat,othertrack.bbox_min_lat])
       max_lon = min([self.bbox_max_lon,othertrack.bbox_max_lon])
       max_lat = min([self.bbox_max_lat,othertrack.bbox_max_lat])
       return min_lon,min_lat,max_lon,max_lat

    def add_features(self,new_features):
       self.features.extend(new_features)
       self.update_bbox()


if(len(sys.argv)<2):
 print("usage: gpx_to_geojson.py <import_directory>")
 sys.exit(0)

import_path = sys.argv[1]

hausdorff_distance_trigger = 0.000050
geom_merge_leftover_len_trigger = 6

tracks = []
all_features = []

index = 0

ids = 0

for filename in os.listdir(import_path):
    infile = os.path.join(import_path,filename)
    if os.path.isfile(infile):
     if(re.match('.*\.gpx$',infile,re.IGNORECASE)):

       print("adding "+infile)
       ids += 1
       nt = Track(ids)
       nt.populate_from_gpxlist(gpxtolist(infile))
       print(str(len(nt.features))+' vectors')

       if(ids == 1):
         tracks.append(nt)
         continue

       for track in tracks:
        if(track.idnum != nt.idnum):
          if(nt.overlap(track)):
            print("added track overlaps track in array")

            min_lon,min_lat,max_lon,max_lat = nt.overlap_bbox(track)
            overlap_bbox = shapely.geometry.Polygon([[min_lon,min_lat],[max_lon,min_lat],[max_lon,max_lat],[min_lon,max_lat]])

            new_features = []
            converted_shapely_feats = []
            ctr = 0

            for f in nt.features:
              ctr += 1
              print(str(ctr),end='\r')           
              ls1 = shapely.geometry.LineString([(f.geometry.coordinates[0][0],f.geometry.coordinates[0][1]),(f.geometry.coordinates[1][0],f.geometry.coordinates[1][1])])

              if(ls1.intersects(overlap_bbox)):
            
               for test_feat in track.features:
                 ls2 = shapely.geometry.LineString([(test_feat.geometry.coordinates[0][0],test_feat.geometry.coordinates[0][1]),(test_feat.geometry.coordinates[1][0],test_feat.geometry.coordinates[1][1])])
                 hd = ls1.hausdorff_distance(ls2)
                 if(hd<hausdorff_distance_trigger):
                   print("vector overlap detected! hd:"+str(hd))
                   geom_remove(nt.features,ls1)
        
            for vrfied_feat in nt.features:
                shp_feat = shapely.geometry.LineString([(vrfied_feat.geometry.coordinates[0][0],vrfied_feat.geometry.coordinates[0][1]),(vrfied_feat.geometry.coordinates[1][0],vrfied_feat.geometry.coordinates[1][1])])
                converted_shapely_feats.append(shp_feat)

            ls = shapely.ops.linemerge(converted_shapely_feats)

            try:
               for geom in ls.geoms:
                if(len(geom.coords)<geom_merge_leftover_len_trigger):
                  geom_remove(nt.features,geom)
            except:
               # geometry is contigous and was returned as a single LineString
               pass

       nt.update_bbox()
       tracks.append(nt) 

for track in tracks:
  all_features.extend(track.features)

feature_collection = FeatureCollection(features=all_features)

with open("combined.json", 'w', encoding='utf-8') as f:
    json.dump(feature_collection, f, ensure_ascii=False, indent=4)

