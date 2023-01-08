import:
	./gpx_to_geojson.py gpx_import
#	mv combined.json www/ol/

tiles: import
	tippecanoe --cluster-distance=10 --accumulate-attribute=shock:max --force -o tiles/pothole.mbtiles combined.json

run_tileserver:
	/Users/wojtek/go/bin/mbtileserver -d tiles


