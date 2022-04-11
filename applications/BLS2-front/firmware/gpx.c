#include "global.h"


void gpx_writer_handler(void *p_context)
 {
  if(m_GPS_on)
   {
    SEGGER_RTT_printf(0,"gpx_writer_handler called.\n");
    G_gpx_write_position = true;
   }
  
 }


bool gpx_write_header(char *filename, char *track_name)
 {
     FIL file;
     UINT bw;
     //FRESULT ff_result;
     uint16_t data_len;

     char gpx_header[512];
     char gpx_header_1[] = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<gpx version=\"1.0\" xmlns=\"http://www.topografix.com/GPX/1/1\"\nxmlns:gpxtpx=\"http://www.w3.org/2001/XMLSchema-instance\"\nxmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"\nxsi:schemaLocation=\"http://www.topografix.com/GPX/1/1 http://www.topografix.com/GPX/1/1/gpx.xsd\">\n<trk><name>";
     char gpx_header_2[] = "</name>\n<trkseg>\n";

     bzero(gpx_header,512);

     SEGGER_RTT_printf(0,"gpx_write_header to %s: start\n",filename);

     while(m_SPI_mutex == true)
      {}

     m_SPI_mutex = true;
  
     f_unlink(filename);  // remove if it exists

     if(f_open(&file, filename, FA_WRITE | FA_CREATE_ALWAYS) == FR_OK)
      {
        //f_lseek(&file, f_size(&file));
        
        strcat(gpx_header,gpx_header_1);
        strcat(gpx_header,track_name);
        strcat(gpx_header,gpx_header_2);

        data_len = strlen(gpx_header);

        SEGGER_RTT_printf(0,"gpx_write_header: header data: %s, len: %d\n",gpx_header,data_len);

        f_write(&file, gpx_header, data_len, &bw);     /* Write data to the file */
        
        SEGGER_RTT_printf(0,"gpx_write_header: wrote header\n");

        f_close(&file);                                /* Close the file */

        m_SPI_mutex = false;

        if (bw != data_len)
          return false;
      }

     SEGGER_RTT_printf(0,"gpx_write_header: end\n");

     m_SPI_mutex = false;
     return true;
 }


bool gpx_write_footer(char *filename)
 {
    FIL file;
    UINT bw;
    uint16_t data_len;
    char gpx_footer[] ="</trkseg></trk></gpx>";

    while(m_SPI_mutex == true)
     {}
    m_SPI_mutex = true;

    if (f_open(&file, filename, FA_WRITE | FA_OPEN_APPEND) == FR_OK)
     {
       data_len = strlen(gpx_footer);
       int len = f_size(&file);
       f_lseek(&file, len);
       f_write(&file, gpx_footer, data_len, &bw);     /* Write data to the file */
       f_close(&file);                                /* Close the file */
       m_SPI_mutex = false;
       if (bw != data_len)
          return false;
      }
    m_SPI_mutex = false;
    return true;
 }


bool gpx_append_position_with_shock(nmea_gpgga_t *position, uint16_t shock_val, char *filename)
 {
    FIL file;
    UINT bw;
    uint16_t data_len;
    uint32_t minsec_long;
    uint32_t minsec_lat;
    char gpx_trkpt[512];
    struct tm *timeptr;
    degmin_position_t *degmin_long,*degmin_lat;
    
    degmin_long = &position->m_longitude;
    degmin_lat = &position->m_latitude;
    timeptr = &position->time;

    bzero(gpx_trkpt,sizeof(gpx_trkpt));

    minsec_long = (degmin_long->minutes/60)*10000000;
    minsec_lat = (degmin_lat->minutes/60)*10000000;
    sprintf(gpx_trkpt,"<trkpt lat=\"%d.%07d\" lon=\"%d.%07d\"><ele>%d</ele><time>%d-%02d-%02dT%02d:%02d:%02dZ</time><extensions><gpxtpx:TrackPointExtension><gpxtpx:shock>%d</gpxtpx:shock></gpxtpx:TrackPointExtension></extensions></trkpt>\n",
    //sprintf(gpx_trkpt,"<trkpt lat=\"%d.%07d\" lon=\"%d.%07d\"><ele>%d</ele><time>%d-%02d-%02dT%02dZ</time><extensions><gpxtpx:TrackPointExtension><gpxtpx:shock>%d</gpxtpx:shock></gpxtpx:TrackPointExtension></extensions></trkpt>\n",
                                                                               degmin_lat->degrees,(int)minsec_lat,degmin_long->degrees,
                                                                               (int)minsec_long,position->altitude,
                                                                               G_GPS_year+2000,G_GPS_month,G_GPS_day,
                                                                               timeptr->tm_hour,timeptr->tm_min,timeptr->tm_sec,shock_val);
    while(m_SPI_mutex == true)
     {}
    m_SPI_mutex = true;

    if (f_open(&file, filename, FA_WRITE | FA_OPEN_APPEND) == FR_OK)
     {
       //int len = f_size(&file);
       //f_lseek(&file, len);
       data_len = strlen(gpx_trkpt);
       f_write(&file, gpx_trkpt, data_len, &bw);     /* Write data to the file */
       f_close(&file);                               /* Close the file */
       m_SPI_mutex = false;
       if (bw != data_len)
          return false;
      }
    m_SPI_mutex = false;
    return true;
 }

