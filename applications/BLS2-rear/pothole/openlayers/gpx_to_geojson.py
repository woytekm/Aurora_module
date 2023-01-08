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
import uuid
import np
import hausdorff


def isparrallel(ls1, ls2):
    xs1, xe1 = ls1.xy[0]
    ys1, ye1 = ls1.xy[1]
    xs2, xe2 = ls2.xy[0]
    ys2, ye2 = ls2.xy[1]

    if xs1 == xe1:
        angle1 = np.pi/2
    else:
        angle1 = np.arctan((ye1-ys1)/(xe1-xs1))
    if xs2 == xe2:
        angle2 = np.pi/2
    else:
        angle2 = np.arctan((ye2-ys2)/(xe2-xs2))

    return True if angle1 == angle2 else False


def cache_track(track,dir):
 if not os.path.exists(dir):
   try:
     os.makedirs(dir)
   except:
     print("Cannot create track cache directory.")
     sys.exit()

 with open(dir+"/"+str(track.uuid)+".json", 'w', encoding='utf-8') as f:
   json.dump(track.features, f, ensure_ascii=False, indent=4)



def load_cached_track(infile):
 nt = Track(-1)
 print("loading cached "+infile+"...")
 with open(infile, 'r', encoding='utf-8') as f:
   nt.populate_from_json(json.load(f))
 nt.update_bbox()
 return nt


def geom_remove(geojson_features, geom):

   linestrings_to_remove = []
   prev_coord = (0,0)

#   print("removing geometry: ", end="")
#   print(geom)

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
 #        print("removed vector:"+str(feat.geometry.coordinates[0][0])+","+str(feat.geometry.coordinates[0][1])+","+str(feat.geometry.coordinates[1][0])+","+str(feat.geometry.coordinates[1][1]))
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
    uuid = ""

    def __init__(self,idn):
       self.idnum = idn
       self.features = []
       self.overlaps = []
       self.uuid = ""

    def populate_from_json(self,json_array):
       for row in json_array:
         f = Feature(geometry=LineString([(row['geometry']['coordinates'][0][0], row['geometry']['coordinates'][0][1]),(row['geometry']['coordinates'][1][0],row['geometry']['coordinates'][1][1])]), properties={"shock":row['properties']['shock'],"ele":row['properties']['ele'],"time":row['properties']['time']})
         self.features.append(f)
       self.update_bbox()


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

create_cache = True
use_cache = True
cache_path = "gpx_to_geojson.cache"

# load already processed, cached geojson features
if use_cache == True:
 for filename in os.listdir(cache_path):
    infile = os.path.join(cache_path,filename)
    if os.path.isfile(infile):
      nct = load_cached_track(infile)
      tracks.append(nct)

# build output data from set of provided gpx files

for filename in os.listdir(import_path):
    infile = os.path.join(import_path,filename)
    if os.path.isfile(infile):
     if(re.match('.*\.gpx$',infile,re.IGNORECASE)):

       print("adding "+infile)
       stat_data = os.stat(infile)
       file_len = stat_data.st_size

       ids += 1
       nt = Track(ids)
       # unique track identifier UUID is derived from file name + file length. TODO: add md5 sum from first 1KB of the file to better identify it.
       nt.uuid = str(uuid.uuid5(uuid.NAMESPACE_DNS, str(file_len)+infile))
       if use_cache == True:
         if os.path.isfile(cache_path+"/"+nt.uuid+'.json'):
           print("skipping "+infile+" - already in cache")
           del nt
           continue
  
       nt.populate_from_gpxlist(gpxtolist(infile))
       print(str(len(nt.features))+' vectors')

       if use_cache == False:
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
                   shockA = f.properties['shock']
                   shockB = test_feat.properties['shock']
                   print("vector overlap detected! hd:"+str(hd)+" shock A:"+str(shockA)+", shock B:"+str(shockB)+", avg shock:"+str((shockA+shockB)/2))
                       
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
  if((create_cache == True) and (track.idnum != -1) ):  # -1 means that this is already cached track loaded from cache
    if not os.path.isfile(cache_path+"/"+track.uuid+'.json'):
      cache_track(track,cache_path)

feature_collection = FeatureCollection(features=all_features)

with open("combined.json", 'w', encoding='utf-8') as f:
    json.dump(feature_collection, f, ensure_ascii=False, indent=4)

