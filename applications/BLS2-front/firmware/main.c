/** 
 *
 * BLS2 - Bike Lighting System 2 - rear light firmware: main.c
 *
 */

#include "global.h"
#include <stdbool.h>
#include <stdint.h>


#define USE_FATFS_QSPI    1

/**
 * @brief Mass storage class user event handler
 */
static void msc_user_ev_handler(app_usbd_class_inst_t const * p_inst,
                                app_usbd_msc_user_event_t     event);


#define RAM_BLOCK_DEVICE_SIZE (380 * 512)

NRF_BLOCK_DEV_QSPI_DEFINE(
    m_block_dev_qspi,
    NRF_BLOCK_DEV_QSPI_CONFIG(
        512,
        NRF_BLOCK_DEV_QSPI_FLAG_CACHE_WRITEBACK,
        NRF_DRV_QSPI_DEFAULT_CONFIG
     ),
     NFR_BLOCK_DEV_INFO_CONFIG("Nordic", "QSPI", "1.00")
);

#define BLOCKDEV_LIST() (                                   \
    NRF_BLOCKDEV_BASE_ADDR(m_block_dev_qspi, block_dev)     \
)

#define ENDPOINT_LIST() APP_USBD_MSC_ENDPOINT_LIST(1, 1)

#define MSC_WORKBUFFER_SIZE (1024)

static nrf_atomic_u32_t m_key_events;

static bool m_usb_connected = false;


static FATFS m_filesystem;



static void fatfs_mkfs(void)
{
    FRESULT ff_result;

    if (m_usb_connected)
    {
        SEGGER_RTT_printf(0,"Unable to operate on filesystem while USB is connected\n");
        return;
    }

    SEGGER_RTT_printf(0,"\r\nCreating filesystem...\n");
    static uint8_t buf[512];
    ff_result = f_mkfs("", FM_FAT, 1024, buf, sizeof(buf));
    if (ff_result != FR_OK)
    {
        SEGGER_RTT_printf(0,"Mkfs failed (%d).\n",ff_result);
        return;
    }

    SEGGER_RTT_printf(0,"Mounting volume...\n");
    ff_result = f_mount(&m_filesystem, "", 1);
    if (ff_result != FR_OK)
    {
        SEGGER_RTT_printf(0,"Mount failed.\n");
        return;
    }

    SEGGER_RTT_printf(0,"Done\n");
}



static bool fatfs_init(void)
{
    FRESULT ff_result;
    DSTATUS disk_state = STA_NOINIT;

    memset(&m_filesystem, 0, sizeof(FATFS));

    // Initialize FATFS disk I/O interface by providing the block device.
    static diskio_blkdev_t drives[] =
    {
        DISKIO_BLOCKDEV_CONFIG(NRF_BLOCKDEV_BASE_ADDR(m_block_dev_qspi, block_dev), NULL)
    };

    diskio_blockdev_register(drives, ARRAY_SIZE(drives));

    SEGGER_RTT_printf(0,"Initializing disk 0 (QSPI)...\n");
    disk_state = disk_initialize(0);
    if (disk_state)
    {
        SEGGER_RTT_printf(0,"Disk initialization failed.\n");
        return false;
    }
  
    if(nrf_gpio_pin_read(PIN_BTN1) == 0 )
     {
      SEGGER_RTT_printf(0,"Button 1 is pressed - reinitializing internal storage...\n");
      fatfs_mkfs();
     }

    SEGGER_RTT_printf(0,"Mounting volume...\n");
    ff_result = f_mount(&m_filesystem, "", 1);
    if (ff_result != FR_OK)
    {
        if (ff_result == FR_NO_FILESYSTEM)
        {
            SEGGER_RTT_printf(0,"Mount failed. Filesystem not found. Please format device.\n");
        }
        else
        {
            SEGGER_RTT_printf(0,"Mount failed: %u\n", ff_result);
        }
        return false;
    }

    SEGGER_RTT_printf(0,"Mount successful.\n");
    return true;
}


static void fatfs_ls(void)
{
    DIR dir;
    FRESULT ff_result;
    FILINFO fno;

    if (m_usb_connected)
    {
        SEGGER_RTT_printf(0,"Unable to operate on filesystem while USB is connected\n");
        return;
    }

    SEGGER_RTT_printf(0,"\r\nListing directory: /\n");
    ff_result = f_opendir(&dir, "/");
    if (ff_result != FR_OK)
    {
        SEGGER_RTT_printf(0,"Directory listing failed: %u\n", ff_result);
        return;
    }

    uint32_t entries_count = 0;
    do
    {
        ff_result = f_readdir(&dir, &fno);
        if (ff_result != FR_OK)
        {
            SEGGER_RTT_printf(0,"Directory read failed: %u\n", ff_result);
            return;
        }

        if (fno.fname[0])
        {
            if (fno.fattrib & AM_DIR)
            {
                SEGGER_RTT_printf(0,"   <DIR>   %s\r\n",(uint32_t)fno.fname);
            }
            else
            {
                SEGGER_RTT_printf(0,"%9lu  %s\r\n", fno.fsize, (uint32_t)fno.fname);
            }
        }

        ++entries_count;
        NRF_LOG_FLUSH();
    } while (fno.fname[0]);


    SEGGER_RTT_printf(0,"Entries count: %u\r\n", entries_count);
}


static void fatfs_file_create(void)
{
    FRESULT ff_result;
    FIL file;
    UINT bw;
    char filename[16];
    uint8_t random_vector[4];

    char *text = "Create a context instance (nrf_crypto_rng_context_t) that is valid for as long as the RNG is in use and optionally create a temporary buffer (nrf_crypto_rng_temp_buffer_t) that is only needed during initialization.  Initialize the RNG using nrf_crypto_rng_init after a call to nrf_crypto_init, providing a pointer to the context and a temporary buffer. Create a context instance (nrf_crypto_rng_context_t) that is valid for as long as the RNG is in use and optionally create a temporary buffer (nrf_crypto_rng_temp_buffer_t) that is only needed during initialization.  Initialize the RNG using nrf_crypto_rng_init after a call to nrf_crypto_init, providing a pointer to the context and a temporary buffer.";

    nrf_drv_rng_rand(random_vector, 4);

    if (m_usb_connected)
    {
        SEGGER_RTT_printf(0,"Unable to operate on filesystem while USB is connected");
        return;
    }

    (void)snprintf(filename, sizeof(filename), "%08lx.txt", (uint32_t)random_vector);

    SEGGER_RTT_printf(0,"Creating random file: %s ...", (uint32_t)filename);

    ff_result = f_open(&file, filename, FA_CREATE_ALWAYS | FA_WRITE);
    if (ff_result != FR_OK)
    {
        SEGGER_RTT_printf(0,"\r\nUnable to open or create file: %u", ff_result);
        return;
    }

    f_write(&file,text,strlen(text),&bw);

    ff_result = f_close(&file);
    if (ff_result != FR_OK)
    {
        SEGGER_RTT_printf(0,"\r\nUnable to close file: %u", ff_result);
        return;
    }
    NRF_LOG_RAW_INFO("done\r\n");
}


static void fatfs_uninit(void)
{
    SEGGER_RTT_printf(0,"Un-initializing disk 0 (QSPI)...");
    UNUSED_RETURN_VALUE(disk_uninitialize(0));
}


static void msc_user_ev_handler(app_usbd_class_inst_t const * p_inst,
                                app_usbd_msc_user_event_t     event)
{
    UNUSED_PARAMETER(p_inst);
    UNUSED_PARAMETER(event);
}


APP_USBD_MSC_GLOBAL_DEF(m_app_msc,
                        0,
                        msc_user_ev_handler,
                        ENDPOINT_LIST(),
                        BLOCKDEV_LIST(),
                        MSC_WORKBUFFER_SIZE);


static void usbd_user_ev_handler(app_usbd_event_type_t event)
{
    switch (event)
    {
        case APP_USBD_EVT_DRV_SUSPEND:
            break;
        case APP_USBD_EVT_DRV_RESUME:
            break;
        case APP_USBD_EVT_STARTED:
            break;
        case APP_USBD_EVT_STOPPED:
            app_usbd_disable();
            NVIC_SystemReset();
            break;
        case APP_USBD_EVT_POWER_DETECTED:
            SEGGER_RTT_printf(0,"USB power detected\n");

            if (!nrf_drv_usbd_is_enabled())
            {
                fatfs_uninit();
                app_usbd_enable();
            }
            break;
        case APP_USBD_EVT_POWER_REMOVED:
            SEGGER_RTT_printf(0,"USB power removed\n");
            app_usbd_stop();
            m_usb_connected = false;
            break;
        case APP_USBD_EVT_POWER_READY:
            SEGGER_RTT_printf(0,"USB ready\n");
            app_usbd_start();
            m_usb_connected = true;
            break;
        default:
            break;
    }
}

static void bsp_event_callback(bsp_event_t ev)
{
}

void logger(void)
 {

  if(m_prev_GPS_state != m_GPS_on)
    {
     if(m_GPS_on)
      {
        gpx_write_header(TRACK_FNAME, "BLS2 road surface quality measurements track");
        SEGGER_RTT_printf(0,"GPX file initialized.\n");
        G_gpx_wrote_footer = false;
        G_gpx_wrote_header = true;
      }
     else
      {
        sprintf(G_current_gpx_filename,"%02d%02d.gpx",G_system_time.tm_hour,G_system_time.tm_min);
        gpx_write_footer(TRACK_FNAME);
        f_rename(TRACK_FNAME,G_current_gpx_filename);

        SEGGER_RTT_printf(0,"GPX file closed.\n");
        G_gpx_wrote_header = false;
        G_gpx_wrote_footer = true;
      }
     m_prev_GPS_state = m_GPS_on;
    }

  if(((m_GPS_on&&G_gpx_wrote_header) && (G_current_position.n_satellites >= MIN_SATELLITES) && G_gpx_write_position) && (G_pos_write_delay==5) && (G_current_position.HDOP < MIN_V_HDOP))
   {
     SEGGER_RTT_printf(0,"gpx_write_header: position appended.\n");
     gpx_append_position_with_shock(&G_current_position, m_shock_val, TRACK_FNAME);
     nrf_gpio_pin_set(USER_LED_3);
     nrf_delay_us(500);
     nrf_gpio_pin_clear(USER_LED_3);
     G_gpx_write_position = false;
     
   }

  m_shock_val = 0;

  if((G_current_position.n_satellites >= MIN_SATELLITES) && (G_pos_write_delay<5))
   G_pos_write_delay++;

 }


void led_timer_handler(void *p_context)
 {

  if((m_GPS_on)&&(G_current_position.n_satellites == 0))
   {
    if(led_1_blink)
      nrf_gpio_pin_set(USER_LED_1);
    else
      nrf_gpio_pin_clear(USER_LED_1);
    
    m_blink_delay++;

    if(m_blink_delay >= 2)
     {
      led_1_blink = !led_1_blink;
      m_blink_delay = 0;
     }
   }
  else if((m_GPS_on)&&(G_current_position.n_satellites < MIN_SATELLITES))
   {
    if(led_1_blink)
      nrf_gpio_pin_set(USER_LED_1);
    else
      nrf_gpio_pin_clear(USER_LED_1);
    led_1_blink = !led_1_blink;
   }
  else if(!m_GPS_on)
    nrf_gpio_pin_clear(USER_LED_1);

  if(m_usb_connected)
     nrf_gpio_pin_set(USER_LED_2);
  else
    nrf_gpio_pin_clear(USER_LED_2);

    // switch(m_led_step)
    //  {
    //   case 0:
    //    nrf_gpio_pin_clear(NRF_GPIO_PIN_MAP(0,29));
    //    nrf_gpio_pin_set(PIN_J33_08);
    //    m_led_step = 1;
    //    break;
    //   case 1:
    //    nrf_gpio_pin_clear(PIN_J33_08);
    //    nrf_gpio_pin_set(PIN_J33_10);
    //    m_led_step = 2;
    //    break;
    //   case 2:
    //    nrf_gpio_pin_clear(PIN_J33_10);
    //    nrf_gpio_pin_set(NRF_GPIO_PIN_MAP(0,29));
    //    m_led_step = 0;
    //    break;
    //  }
 
 }



int main(void)
{

 ret_code_t ret;
 //uint8_t usb_timeout = 0;

 //static const app_usbd_config_t usbd_config = {
 //    .ev_state_proc = usbd_user_ev_handler
 //};

 ret = NRF_LOG_INIT(NULL);
 APP_ERROR_CHECK(ret);
 NRF_LOG_DEFAULT_BACKENDS_INIT();

 SEGGER_RTT_printf(0, "Firmware start\n");

 SEGGER_RTT_printf(0,"clk_init()\n");
 clk_init();

 SEGGER_RTT_printf(0,"app_timer_init()\n");
 app_timer_init();

 #define LED_TIMER_INTERVAL APP_TIMER_TICKS(1000)

 m_led_timer = (app_timer_t *) malloc(sizeof(app_timer_t));
 memset(m_led_timer, 0, sizeof(app_timer_t));

 ret = app_timer_create(&m_led_timer,
                         APP_TIMER_MODE_REPEATED,
                         led_timer_handler);

 ret = app_timer_start(m_led_timer, LED_TIMER_INTERVAL, NULL);

 SEGGER_RTT_printf(0,"LED timer create code %d\n",ret);
 APP_ERROR_CHECK(ret);

 nrf_gpio_cfg_output(PIN_J33_08);
 nrf_gpio_cfg_output(PIN_J33_10);
 nrf_gpio_cfg_output(NRF_GPIO_PIN_MAP(0,29));
 nrf_gpio_pin_clear(PIN_J33_08);
 nrf_gpio_pin_clear(PIN_J33_10);
 nrf_gpio_pin_clear(NRF_GPIO_PIN_MAP(0,29));

 nrf_gpio_cfg_output(USER_LED_2);

// ret = app_usbd_init(&usbd_config);
// APP_ERROR_CHECK(ret);

// app_usbd_class_inst_t const * class_inst_msc = app_usbd_msc_class_inst_get(&m_app_msc);
// ret = app_usbd_class_append(class_inst_msc);
// APP_ERROR_CHECK(ret);

// ret = app_usbd_power_events_enable();
// APP_ERROR_CHECK(ret);

// SEGGER_RTT_printf(0, "USBD MSC feature started.\n");
 
// while(usb_timeout<USB_INIT_WAIT) {app_usbd_event_queue_process(); nrf_delay_ms(100); usb_timeout++;}

// if(m_usb_connected)
//  {
//   SEGGER_RTT_printf(0, "USB power detected, entering USB drive mode.\n");
//   while(1){app_usbd_event_queue_process();}
//  }

 system_init();

 //fatfs_init();
 //fatfs_ls();
 //LIS3DH_test();
 //LIS3DH_init();

 for (;;)
   {   
        //SEGGER_RTT_printf(0, "WFE.\n");
        // Wait for an event.
        __WFE();

        // Clear the event register.
        __SEV();
        __WFE();

        logger();
     
        //app_sched_execute();
    }
 

}

