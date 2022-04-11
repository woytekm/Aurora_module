cat $1 | awk 'BEGIN{FS="<";}{print $2" "$9;}' | awk '{print $2" "$3" "$4;}' | sed -e s/lat=//g -e s/lon=//g -e s/gpxtpx:shock//g -e s/\[\"\>\]//g | awk 'BEGIN{print "latitude,longitude,shock";}{print $1","$2","$3;}' | grep -v ",," | grep -v "version"

