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

APP_USBD_MSC_GLOBAL_DEF(m_app_msc,
                        0,
                        msc_user_ev_handler,
                        ENDPOINT_LIST(),
                        BLOCKDEV_LIST(),
                        MSC_WORKBUFFER_SIZE);

static nrf_atomic_u32_t m_key_events;

static bool m_usb_connected = false;


static FATFS m_filesystem;

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
        SEGGER_RTT_printf(0,"Mkfs failed.\n");
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
    char filename[16];

    if (m_usb_connected)
    {
        SEGGER_RTT_printf(0,"Unable to operate on filesystem while USB is connected");
        return;
    }

    (void)snprintf(filename, sizeof(filename), "%08x.txt", rand());

    SEGGER_RTT_printf(0,"Creating random file: %s ...", (uint32_t)filename);

    ff_result = f_open(&file, filename, FA_CREATE_ALWAYS | FA_WRITE);
    if (ff_result != FR_OK)
    {
        SEGGER_RTT_printf(0,"\r\nUnable to open or create file: %u", ff_result);
        return;
    }

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
            UNUSED_RETURN_VALUE(fatfs_init());
            app_usbd_disable();
            break;
        case APP_USBD_EVT_POWER_DETECTED:
            SEGGER_RTT_printf(0,"USB power detected");

            if (!nrf_drv_usbd_is_enabled())
            {
                //fatfs_uninit();
                app_usbd_enable();
            }
            break;
        case APP_USBD_EVT_POWER_REMOVED:
            SEGGER_RTT_printf(0,"USB power removed");
            app_usbd_stop();
            m_usb_connected = false;
            break;
        case APP_USBD_EVT_POWER_READY:
            SEGGER_RTT_printf(0,"USB ready");
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




int main(void)
{

 ret_code_t ret;

 static const app_usbd_config_t usbd_config = {
     .ev_state_proc = usbd_user_ev_handler
 };

 SEGGER_RTT_printf(0, "Firmware start\n");

 ret = app_usbd_init(&usbd_config);
 APP_ERROR_CHECK(ret);

 app_usbd_class_inst_t const * class_inst_msc = app_usbd_msc_class_inst_get(&m_app_msc);
 ret = app_usbd_class_append(class_inst_msc);
 APP_ERROR_CHECK(ret);

 SEGGER_RTT_printf(0, "USBD MSC example started.\n");

 fatfs_init();
 fatfs_ls();

 system_init();

 for (;;)
   {   
        //SEGGER_RTT_printf(0, "WFE.\n");
        // Wait for an event.
        __WFE();

        // Clear the event register.
        __SEV();
        __WFE();

        NRF_LOG_FLUSH();
        //app_sched_execute();
    }
 

}

