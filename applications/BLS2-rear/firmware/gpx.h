#ifndef GPX_H
#define GPX_H

#include "global.h"

#define MIN_SATELLITES          6
#define MIN_V_HDOP              3

#define NMEA_TIME_FORMAT        "%H%M%S"
#define NMEA_TIME_FORMAT_LEN    6

#define NMEA_DATE_FORMAT        "%d%m%y"
#define NMEA_DATE_FORMAT_LEN    6

typedef struct {
           double minutes;
           int degrees;
  } degmin_position_t;

typedef struct {
          struct tm time;
          degmin_position_t m_longitude;
          degmin_position_t m_latitude;
          char nmea_longitude[12];
          char nmea_latitude[12];
          double HDOP;
          int n_satellites;
          int altitude;
          char altitude_unit;
 } nmea_gpgga_t;

bool gpx_write_header(char *filename, char *track_name);
bool gpx_write_footer(char *filename);
bool gpx_append_position_with_shock(nmea_gpgga_t *position, uint16_t shock_val, char *filename);
void GPS_logger_handler(void *p_context);

#endif 
