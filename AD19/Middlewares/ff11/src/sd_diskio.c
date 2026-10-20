/**
  ******************************************************************************
  * @file    sd_diskio.c
  * @author  MCD Application Team
  * @version V1.3.0
  * @date    08-May-2015
  * @brief   SD Disk I/O driver
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2015 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software
  * distributed under the License is distributed on an "AS IS" BASIS,
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include <string.h>
#include "bsp.h"
#include "ff.h"
#include "diskio.h"
#include "sd_diskio.h"
#include "spi_flash.h"
#include "Lepton_Common_Data.h"
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Block Size in Bytes */
#define BLOCK_SIZE                  (4096)//(512)//(4096)
#define MAX_BMP_CNT                 (60)

/* Private variables ---------------------------------------------------------*/
/* Disk status */
static volatile DSTATUS Stat = STA_NOINIT;

// static char _bmp_file_list[MAX_BMP_CNT][8] = {0};
static char _save_bmp_name[32] = {0};
static char _last_bmp_name[32] = {0};
static uint8_t _bmp_file_num = 0;
static uint32_t _bmp_name_num = 0;

static FILINFO _fileinfo;

#if FF_USE_LFN
    static char _fileinfo_lfn[128];
#endif

///* Private function prototypes -----------------------------------------------*/

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Initializes a Drive
  * @param  lun : not used
  * @retval DSTATUS: Operation status
  * @attention 此处不执行初始化，需要单独调用 spi flash 初始化
  */
DSTATUS disk_initialize (
	BYTE pdrv				/* Physical drive nmuber to identify the drive */
)
{
    Stat &= ~STA_NOINIT;

    return Stat;
}

/**
  * @brief  Gets Disk Status
  * @param  lun : not used
  * @retval DSTATUS: Operation status
  */
DSTATUS disk_status (
	BYTE pdrv		/* Physical drive nmuber to identify the drive */
)
{
    Stat &= ~STA_NOINIT;
    return Stat;
}

/**
  * @brief  Reads Sector(s)
  * @param  lun : not used
  * @param  *buff: Data buffer to store read data
  * @param  sector: Sector address (LBA)
  * @param  count: Number of sectors to read (1..128)
  * @retval DRESULT: Operation result
  */
DRESULT disk_read (
	BYTE pdrv,		/* Physical drive nmuber to identify the drive */
	BYTE *buff,		/* Data buffer to store read data */
	LBA_t sector,	/* Start sector in LBA */
	UINT count		/* Number of sectors to read */
)
{
    DRESULT res = RES_OK;

    spiflash_read_buffer((uint8_t *)buff, sector * BLOCK_SIZE, count * BLOCK_SIZE);

    return res;
}

/**
  * @brief  Writes Sector(s)
  * @param  lun : not used
  * @param  *buff: Data to be written
  * @param  sector: Sector address (LBA)
  * @param  count: Number of sectors to write (1..128)
  * @retval DRESULT: Operation result
  */
#if FF_FS_READONLY == 0
DRESULT disk_write (
	BYTE pdrv,			/* Physical drive nmuber to identify the drive */
	const BYTE *buff,	/* Data to be written */
	LBA_t sector,		/* Start sector in LBA */
	UINT count			/* Number of sectors to write */
)
{
    DRESULT res = RES_OK;

    spiflash_write_buffer((uint8_t *)buff, sector * BLOCK_SIZE, count * BLOCK_SIZE);

    return res;
}
#endif /* _USE_WRITE == 1 */

/**
  * @brief  I/O control operation
  * @param  lun : not used
  * @param  cmd: Control code
  * @param  *buff: Buffer to send/receive control data
  * @retval DRESULT: Operation result
  */
DRESULT disk_ioctl (
	BYTE pdrv,		/* Physical drive nmuber (0..) */
	BYTE cmd,		/* Control code */
	void *buff		/* Buffer to send/receive control data */
)
{
    DRESULT res = RES_ERROR;

    if (Stat & STA_NOINIT)
    {
        return RES_NOTRDY;
    }

    switch (cmd)
    {
        /* Make sure that no pending write process */
        case CTRL_SYNC :
            res = RES_OK;
            break;

        /* Get number of sectors on the disk (DWORD) */
        case GET_SECTOR_COUNT :
            *(DWORD *)buff = 1024;//8192;//1024 - 1;
            res = RES_OK;
            break;

        /* Get R/W sector size (WORD) */
        case GET_SECTOR_SIZE :
            *(WORD *)buff = 4096;//512;// BLOCK_SIZE;
            res = RES_OK;
            break;

        /* Get erase block size in unit of sector (DWORD) */
        case GET_BLOCK_SIZE :
            *(DWORD *)buff = 1;//4096;// BLOCK_SIZE;
            break;

        default:
            res = RES_PARERR;
    }

    return res;
}



FRESULT scan_files(
    char *path        /* Start node to be scanned (***also used as work area***) */
)
{
    FRESULT res;
    DIR dir;
    UINT i;
    static FILINFO fno;

#if _USE_LFN
    static char lfn[_MAX_LFN * 2 + 1];
    fno.lfname = lfn;
    fno.lfsize = sizeof(lfn);
#endif

    res = f_opendir(&dir, path);                       /* Open the directory */
    if (res == FR_OK)
    {
        for (;;)
        {
            res = f_readdir(&dir, &fno);                   /* Read a directory item */
            if (res != FR_OK || fno.fname[0] == 0)
            {
                break;    /* Break on error or end of dir */
            }
            if (fno.fattrib & AM_DIR)                      /* It is a directory */
            {
                i = strlen(path);
                sprintf(&path[i], "/%s", fno.fname);
                res = scan_files(path);                    /* Enter the directory */
                if (res != FR_OK)
                {
                    break;
                }
                path[i] = 0;
            }
            else                                           /* It is a file. */
            {
                snprintf(g_strbuff, sizeof(g_strbuff),
                    "%s/%s\n", path, fno.fname);
            }
        }
        f_closedir(&dir);
    }

    return res;
}


static FATFS g_fatfs;
static char* g_rootfs = "0:";

static void _write_file()
{
    FRESULT fr = FR_OK;

    FIL MyFile;     /* File object */
    FIL *fp = &MyFile;
    /* Open a text file */
    fr = f_open(&MyFile, "test_wr_file.txt", FA_OPEN_ALWAYS | FA_WRITE);
    if (FR_OK != fr)
    {
        return;
    }
    f_lseek(fp, f_size(fp));

    for (uint8_t i = 0; i < 10; i++)
    {
        int wrbytes = f_printf(&MyFile, "[%04d]application run again!\r\n", HAL_GetTick());
        if (-1 == wrbytes)
        {
            wrbytes += 1;
        }
        HAL_Delay(10);
    }

    f_sync(&MyFile);
    /* Close the file */
    f_close(&MyFile);
}

void ff_test(void)
{
    FIL MyFile;
    char SDPath[4];
    char path[128];
    snprintf(path, sizeof(path), "/");

    if (f_mount(&g_fatfs, g_rootfs, 1) != FR_OK)
    {
        Error_Handler();
    }
    else
    {
        //scan_files(path);
        _write_file();
    }

}

/*! 检测空闲容量，返回字节数 */
uint32_t _get_free_size(void)
{
    FATFS *fs;
    DWORD fre_clust, tot_sect, fre_sect;

    // 获取指定路径的空闲簇数和文件系统对象
    FRESULT res = f_getfree(g_rootfs, &fre_clust, &fs);
    if (res == FR_OK)
    {
        // 计算总扇区数和空闲扇区数
        tot_sect = (fs->n_fatent - 2) * fs->csize;  // 总扇区数
        fre_sect = fre_clust * fs->csize;           // 空闲扇区数

        // 假设每个扇区大小为 512 字节
        uint32_t total_bytes = tot_sect * BLOCK_SIZE;      // 总字节数
        uint32_t free_bytes = fre_sect * BLOCK_SIZE;       // 空闲字节数

        // 打印结果
       // printf("总容量: %u KB\n", total_bytes / 1024);
       // printf("剩余容量: %u KB\n", free_bytes / 1024);

        return free_bytes;
    }

    return 0;
}



int8_t _init_bmp_file_list(void)
{
    FRESULT res;
    DIR dir;
    UINT i;

#if FF_USE_LFN
    _fileinfo.lfname = _fileinfo_lfn;
    _fileinfo.lfsize = sizeof(_fileinfo_lfn);
#endif

    _bmp_name_num = 0;
    _bmp_file_num = 0;

    res = f_opendir(&dir, "/");                       /* Open the directory */
    if (res == FR_OK)
    {
        for (;;)
        {
            res = f_readdir(&dir, &_fileinfo);                   /* Read a directory item */
            if (res != FR_OK || _fileinfo.fname[0] == 0)
            {
                break;    /* Break on error or end of dir */
            }
            if (_fileinfo.fattrib & AM_DIR)                      /* It is a directory */
            {
                continue;
            }
            else                                           /* It is a file. */
            {
                // 检查文件扩展名是否为 .bmp
                if (strlen(_fileinfo.fname) > 4
                    && strcasecmp(_fileinfo.fname + strlen(_fileinfo.fname) - 4, ".bmp") == 0)
                {
                    // 是 .bmp 文件，提取文件名
                    // strcpy(_bmp_file_list[_bmp_file_num++], _fileinfo.fname);

                    _bmp_file_num++;

                    memset(_save_bmp_name, 0, sizeof(_save_bmp_name));
                    strncpy(_save_bmp_name, _fileinfo.fname, strlen(_fileinfo.fname) - 4);

                    // 将数字字符串转换为整数
                    // uint32_t current_number = atoi(_save_bmp_name);
                    uint32_t current_number = (uint32_t)strtoul(_save_bmp_name, 0, 10);

                    if (_bmp_name_num < current_number)
                    {
                        _bmp_name_num = current_number;
                    }
                }
            }
        }
        f_closedir(&dir);
    }
    
    
   // -------------------------- 新增：创建0.dat并设为隐藏 --------------------------
    if (_bmp_name_num == 0)
    {
        FIL dat_file;
        FRESULT dat_res;
        UINT bw; 
        const uint32_t DAT_SIZE = 16 * 1024; 
        uint8_t *blank_data = (uint8_t *)&g_Lepton_Process_Buffer[0];

        // 1. 分配/初始化空白数据缓冲区（同之前逻辑）

        memset(blank_data, 0, DAT_SIZE);

        // 2. 创建0.dat文件（FA_CREATE_ALWAYS | FA_WRITE）
        dat_res = f_open(&dat_file, "0.dat", FA_CREATE_ALWAYS | FA_WRITE | FA_HIDDEN);
        if (dat_res != FR_OK)
        {
            //printf("Create 0.dat failed, result: %d\n", dat_res);
            //free(blank_data);
            return -5;
        }

        // 3. 写入16KB空白数据
        dat_res = f_write(&dat_file, blank_data, DAT_SIZE, &bw);
        if (dat_res != FR_OK || bw != DAT_SIZE)
        {
            //printf("Write 0.dat failed, result: %d, written: %d bytes\n", dat_res, bw);
            f_close(&dat_file);
            //free(blank_data);
            return -6;
        }

        // 4. 同步文件（确保数据写入磁盘，避免属性修改前文件损坏）
        dat_res = f_sync(&dat_file);
        if (dat_res != FR_OK)
        {
            //printf("Sync 0.dat failed, result: %d\n", dat_res);
            f_close(&dat_file);
            //free(blank_data);
            return -7;
        }
#if 0
        // -------------------------- 关键：设置隐藏属性 --------------------------
        // 定义文件属性结构体，仅修改"属性"字段（其他字段保持默认）
        FILINFO dat_attr;
        memset(&dat_attr, 0, sizeof(dat_attr)); // 初始化结构体
        dat_attr.fattrib = AM_HID; // 设置为隐藏属性（AM_HID = 0x02）

        // 调用f_setattr修改文件属性（参数：文件名、属性结构体）
        dat_res = f_setattr("0.dat", &dat_attr);
        if (dat_res == FR_OK)
        {
            printf("0.dat set to hidden successfully\n");
        }
        else
        {
            printf("Set 0.dat hidden failed, result: %d\n", dat_res);
            // 此处可根据需求决定是否返回错误，属性设置失败不影响文件本身，可忽略
        }
        // --------------------------------------------------------------------------
#endif
        // 5. 关闭文件并释放内存
        f_close(&dat_file);
        //free(blank_data);
        //printf("0.dat created (16KB blank + hidden)\n");
    }
    f_chmod("0:/0.dat", AM_SYS | AM_HID, AM_SYS | AM_HID);
// --------------------------------------------------------------------------

    _bmp_name_num = (0 == _bmp_name_num) ? 1 : (_bmp_name_num + 1);

    return 0;
}

/*
int8_t ff_init(void)
{
    if (FATFS_LinkDriver(&SD_Driver, g_rootfs) == 0)
    {
        if (f_mount(&g_fatfs, (TCHAR const *)g_rootfs, 1) != FR_OK)
        {
            Error_Handler();
            return -1;
        }

        _init_bmp_file_list();
        return 0;
    }
    else
    {
        return -1;
    }
}
*/

int8_t ff_init(void)
{
    uint8_t retry = 3;

    // 多次重试挂载，应对首次读取不稳定
    while (retry--) {
        FRESULT res = f_mount(&g_fatfs, (TCHAR const *)g_rootfs, 1);
        if (res == FR_OK) {
            // 验证方法1：尝试获取磁盘空间，间接确认文件系统可用
            DWORD free_clusters, total_clusters;
            FATFS *fs_ptr;
            res = f_getfree((TCHAR const *)g_rootfs, &free_clusters, &fs_ptr);
            if (res == FR_OK) {
                _init_bmp_file_list();
                return 0;
            } else {
                //printf("Verify filesystem failed (f_getfree: %d), retrying...\n", res);
            }
        }
        HAL_Delay(50);  // 重试前延时，让硬件稳定
    }

    ff_format();

    // 多次重试失败，提示挂载错误
    //printf("FATFS mount failed!\n");
    return -1;
}


const char *ff_get_last_bmp_name(void)
{
    return _last_bmp_name;
}



typedef struct __PACKED
{
    uint16_t signature;      // 'BM'
    uint32_t fileSize;       // Size of the BMP file in bytes
    uint16_t reserved1;      // Reserved
    uint16_t reserved2;      // Reserved
    uint32_t dataOffset;     // Offset to the start of image data
} BMPFileHeader;

// BMP info header structure
typedef struct __PACKED
{
    uint32_t headerSize;     // Size of this header (40 bytes)
    int32_t width;           // Width of the image
    int32_t height;          // Height of the image
    uint16_t planes;         // Number of color planes (must be 1)
    uint16_t bitsPerPixel;   // Bits per pixel
    uint32_t compression;    // Compression method (3 for bitfields)
    uint32_t imageSize;      // Size of the image data
    int32_t xPixelsPerMeter; // Horizontal resolution
    int32_t yPixelsPerMeter; // Vertical resolution
    uint32_t colorsUsed;     // Number of colors in the palette
    uint32_t colorsImportant;// Number of important colors
} BMPInfoHeader;

// RGB565 bitfield masks
typedef struct __PACKED
{
    uint32_t redMask;        // Mask for red component
    uint32_t greenMask;      // Mask for green component
    uint32_t blueMask;       // Mask for blue component
    uint32_t alphaMask;      // Mask for alpha component (not used in RGB565)
} BMPBitfields;

typedef struct __PACKED
{
    unsigned char blue;
    unsigned char green;
    unsigned char red;
    unsigned char reserved;
} RGBQUAD;


RGBQUAD color_table[256];

#define COLOR_RGB565(r, g, b) (((r & 0xF8) << 8) | ((g & 0xFC) << 3) | ((b & 0xF8) >> 3))

#if 0
int8_t ff_save_bmp_rgb565(const uint16_t *buffer, uint16_t w, uint16_t h)
{
    FRESULT fret = FR_OK;
    FIL MyFile;

    if (_bmp_file_num == MAX_BMP_CNT)
    {
        return -2;
    }

    if (_get_free_size() < 132 * 1024)
    {
        /* 剩余空间不足 */
        return -3;
    }


    while (1)
    {
        sprintf(_save_bmp_name, "%u.bmp", _bmp_name_num);
        fret = f_stat(_save_bmp_name, &_fileinfo);
        if (FR_NO_FILE == fret)
        {
            break;
        }
        else if (FR_OK == fret || FR_EXIST == fret)
        {
            _bmp_name_num++;
        }
        else if (FR_NO_FILESYSTEM == fret)
        {
            f_mount(&g_fatfs, g_rootfs, 1);
        }
        else if (FR_OK != fret)
        {
            return -1;
        }
        HAL_Delay(1);
    }

    fret = f_open(&MyFile, _save_bmp_name, FA_CREATE_ALWAYS | FA_WRITE);
    if (FR_OK != fret)
    {
        return -1;
    }

    const int width = w;
    const int height = h;

    const int bytesPerRow = width * 2;
    const int padding = (4 - (bytesPerRow % 4)) % 4;

    const int dataSize = (bytesPerRow + padding) * height;
    const int fileSize = sizeof(BMPFileHeader) + sizeof(BMPInfoHeader) +
        sizeof(BMPBitfields) + dataSize;

    BMPFileHeader fileHeader;
    fileHeader.signature = 0x4D42;
    fileHeader.fileSize = fileSize;
    fileHeader.reserved1 = 0;
    fileHeader.reserved2 = 0;
    fileHeader.dataOffset = sizeof(BMPFileHeader)
        + sizeof(BMPInfoHeader) + sizeof(BMPBitfields);


    BMPInfoHeader infoHeader;
    infoHeader.headerSize = sizeof(BMPInfoHeader);
    infoHeader.width = width;
    infoHeader.height = -height;
    infoHeader.planes = 1;
    infoHeader.bitsPerPixel = 16;
    infoHeader.compression = 3;
    infoHeader.imageSize = dataSize;
    infoHeader.xPixelsPerMeter = 2835; // 72 DPI
    infoHeader.yPixelsPerMeter = 2835; // 72 DPI
    infoHeader.colorsUsed = 0;
    infoHeader.colorsImportant = 0;

    BMPBitfields bitfields;
    bitfields.redMask = 0xF800;
    bitfields.greenMask = 0x07E0;
    bitfields.blueMask = 0x001F;
    bitfields.alphaMask = 0x0000;

    UINT bw = 0;
    // Write headers
    fret = f_write(&MyFile, &fileHeader, sizeof(fileHeader), &bw);
    if (FR_OK != fret || bw != sizeof(fileHeader))
    {
        return -1;
    }
    fret = f_write(&MyFile, &infoHeader, sizeof(infoHeader), &bw);
    if (FR_OK != fret || bw != sizeof(infoHeader))
    {
        return -1;
    }
    fret = f_write(&MyFile, &bitfields, sizeof(bitfields), &bw);
    if (FR_OK != fret || bw != sizeof(bitfields))
    {
        return -1;
    }

    uint8_t padding_data[3] = { 0, 0, 0 }; // 用于行对齐的填充字节
    for (int y = 0; y < height; y++)
    {
        // 直接写入一行 RGB565 数据
        fret = f_write(&MyFile, buffer, sizeof(uint16_t) * width, &bw);
        if (FR_OK != fret || bw != sizeof(uint16_t) * width)
        {
            return -1;
        }
        buffer += width;

        // 写入填充字节（如果需要）
        fret = f_write(&MyFile, padding_data, padding, &bw);
        if (FR_OK != fret || bw != padding)
        {
            return -1;
        }
    }

   // f_sync(&MyFile);
   // f_close(&MyFile);

    if (f_sync(&MyFile) != FR_OK) {
        return -1;
    }
    if (f_close(&MyFile) != FR_OK) {
        return -1;
    }

    _bmp_name_num++;
    _bmp_file_num++;

    return 0;
}
#endif

int8_t ff_save_bmp_rgb565(const uint16_t *buffer, uint16_t w, uint16_t h)
{
    FRESULT fret = FR_OK;
    FIL MyFile;
    // 1. 新增：静态标志，记录是否为首次保存文件（初始为1，首次成功后设为0）
    static uint8_t is_first_file = 1;
    const uint8_t SYNC_RETRY_CNT = 3;  // 重试次数（3次足够覆盖多数偶发错误）
    const uint16_t SYNC_RETRY_DELAY = 50; // 重试间隔（50ms，给硬件稳定时间）

    if (_bmp_file_num == MAX_BMP_CNT)
    {
        return -2;
    }

    if (_get_free_size() < 132 * 1024)
    {
        /* 剩余空间不足 */
        return -3;
    }

    Init_SPI_buffer();

    while (1)
    {
        sprintf(_save_bmp_name, "%u.bmp", _bmp_name_num);
        fret = f_stat(_save_bmp_name, &_fileinfo);
        if (FR_NO_FILE == fret)
        {
            break;
        }
        else if (FR_OK == fret || FR_EXIST == fret)
        {
            _bmp_name_num++;
        }
        else if (FR_NO_FILESYSTEM == fret)
        {
            f_mount(&g_fatfs, g_rootfs, 1);
        }
        else if (FR_OK != fret)
        {
            return -1;
        }
        HAL_Delay(1);
    }

    fret = f_open(&MyFile, _save_bmp_name, FA_CREATE_ALWAYS | FA_WRITE);
    if (FR_OK != fret)
    {
        return -1;
    }

    // --------------------------
    // （原有BMP头、数据写入逻辑不变）
    // --------------------------
    const int width = w;
    const int height = h;
    const int bytesPerRow = width * 2;
    const int padding = (4 - (bytesPerRow % 4)) % 4;
    const int dataSize = (bytesPerRow + padding) * height;
    const int fileSize = sizeof(BMPFileHeader) + sizeof(BMPInfoHeader) + sizeof(BMPBitfields) + dataSize;

    BMPFileHeader fileHeader = {.signature = 0x4D42, .fileSize = fileSize, .reserved1 = 0, .reserved2 = 0, .dataOffset = sizeof(BMPFileHeader) + sizeof(BMPInfoHeader) + sizeof(BMPBitfields)};
    BMPInfoHeader infoHeader = {.headerSize = sizeof(BMPInfoHeader), .width = width, .height = -height, .planes = 1, .bitsPerPixel = 16, .compression = 3, .imageSize = dataSize, .xPixelsPerMeter = 2835, .yPixelsPerMeter = 2835, .colorsUsed = 0, .colorsImportant = 0};
    BMPBitfields bitfields = {.redMask = 0xF800, .greenMask = 0x07E0, .blueMask = 0x001F, .alphaMask = 0x0000};

    UINT bw = 0;
    // 写BMP头
    if (f_write(&MyFile, &fileHeader, sizeof(fileHeader), &bw) != FR_OK || bw != sizeof(fileHeader)) return -1;
    if (f_write(&MyFile, &infoHeader, sizeof(infoHeader), &bw) != FR_OK || bw != sizeof(infoHeader)) return -1;
    if (f_write(&MyFile, &bitfields, sizeof(bitfields), &bw) != FR_OK || bw != sizeof(bitfields)) return -1;

    // 写BMP数据和填充字节
    uint8_t padding_data[3] = {0, 0, 0};
    for (int y = 0; y < height; y++)
    {
        if (f_write(&MyFile, buffer, sizeof(uint16_t)*width, &bw) != FR_OK || bw != sizeof(uint16_t)*width) return -1;
        if (f_write(&MyFile, padding_data, padding, &bw) != FR_OK || bw != padding) return -1;
        buffer += width;
    }

    // 2. 增强：首次文件的f_sync重试逻辑
    FRESULT sync_res = FR_INT_ERR;
    if (is_first_file)
    {
        printf("First file sync, start retry...\n");
        // 重试循环：最多3次
        for (uint8_t retry = 0; retry < SYNC_RETRY_CNT; retry++)
        {
            sync_res = f_sync(&MyFile);
            if (sync_res == FR_OK)
            {
                printf("First file sync success (retry %d/%d)\n", retry+1, SYNC_RETRY_CNT);
                is_first_file = 0; // 首次保存成功，后续不再重试
                break;
            }
            // 重试失败：打印日志+延时
            printf("First file sync failed (retry %d/%d), result: %d\n", retry+1, SYNC_RETRY_CNT, sync_res);
            HAL_Delay(SYNC_RETRY_DELAY);
        }
        // 所有重试都失败：返回错误
        if (sync_res != FR_OK)
        {
            printf("First file sync failed after %d retries\n", SYNC_RETRY_CNT);
            f_close(&MyFile); // 失败后关闭文件，避免资源泄漏
            return -1;
        }
    }
    else
    {
        // 非首次文件：保持原有单次f_sync逻辑
        if (f_sync(&MyFile) != FR_OK)
        {
            f_close(&MyFile);
            return -1;
        }
    }

    // 3. 关闭文件（确保关闭成功）
    if (f_close(&MyFile) != FR_OK)
    {
        return -1;
    }

    _bmp_name_num++;
    _bmp_file_num++;
    strncpy(_last_bmp_name, _save_bmp_name, sizeof(_last_bmp_name) - 1);
    _last_bmp_name[sizeof(_last_bmp_name) - 1] = '\0';

    return 0;
}

int8_t ff_save_bmp_256(const uint8_t *buffer, uint16_t w, uint16_t h)
{
    FRESULT fret = FR_OK;
    FILINFO file_info;
    FIL MyFile;

    if (_get_free_size() < 68 * 1024)
    {
        /* 剩余空间不足 */
        return -3;
    }

    while (1)
    {
        sprintf(_save_bmp_name, "%u.bmp", _bmp_name_num);
        fret = f_stat(_save_bmp_name, &_fileinfo);
        if (FR_NO_FILE == fret)
        {
            break;
        }
        else if (FR_OK == fret || FR_EXIST == fret)
        {
            _bmp_name_num++;
        }
        else if (FR_NO_FILESYSTEM == fret)
        {
            f_mount(&g_fatfs, g_rootfs, 1);
        }
        else if (FR_OK != fret)
        {
            return -1;
        }
        HAL_Delay(1);
    }

    fret = f_open(&MyFile, _save_bmp_name, FA_CREATE_ALWAYS | FA_WRITE);
    if (FR_OK != fret)
    {
        return -1;
    }


    const int width = w;
    const int height = h;

    const int bytesPerRow = width;
    const int padding = (4 - (bytesPerRow % 4)) % 4;

    const int dataSize = (bytesPerRow + padding) * height;
    const int fileSize = sizeof(BMPFileHeader) + sizeof(BMPInfoHeader) +
        sizeof(RGBQUAD) * 256 + dataSize;

    BMPFileHeader fileHeader;
    fileHeader.signature = 0x4D42;
    fileHeader.fileSize = fileSize;
    fileHeader.reserved1 = 0;
    fileHeader.reserved2 = 0;
    fileHeader.dataOffset = sizeof(BMPFileHeader)
        + sizeof(BMPInfoHeader) + sizeof(RGBQUAD) * 256;


    BMPInfoHeader infoHeader;
    infoHeader.headerSize = sizeof(BMPInfoHeader);
    infoHeader.width = width;
    infoHeader.height = -height;
    infoHeader.planes = 1;
    infoHeader.bitsPerPixel = 8;
    infoHeader.compression = 0;
    infoHeader.imageSize = dataSize;
    infoHeader.xPixelsPerMeter = 0; // 72 DPI
    infoHeader.yPixelsPerMeter = 0; // 72 DPI
    infoHeader.colorsUsed = 256;
    infoHeader.colorsImportant = 256;

    UINT bw = 0;

    // 写入文件头和信息头
    fret = f_write(&MyFile, &fileHeader, sizeof(fileHeader), &bw);
    if (FR_OK != fret || bw != sizeof(fileHeader))
    {
        return -1;
    }
    fret = f_write(&MyFile, &infoHeader, sizeof(infoHeader), &bw);
    if (FR_OK != fret || bw != sizeof(infoHeader))
    {
        return -1;
    }

    uint16_t color = 0;
    for (int i = 0; i < 256; i++)
    {
        color = REVERSE_RGB565(COLORMAP_RGB565_MSB[COLOUR_IDX][i]);
        color_table[i].red   = ((color >> 11) & 0x1F) << 3;//(color & 0xF800) >> 9;
        color_table[i].green = ((color >> 5) & 0x3F) << 2;//(color & 0x07E0) >> 3;
        color_table[i].blue  = (color & 0x1F) << 3;//(color & 0x001F);
        color_table[i].reserved = 0xFF;
    }
    fret = f_write(&MyFile, &color_table, sizeof(color_table), &bw);
    if (FR_OK != fret || bw != sizeof(color_table))
    {
        return -1;
    }

    uint8_t padding_data[3] = { 0, 0, 0 }; // 用于行对齐的填充字节
    for (int y = 0; y < height; y++)
    {
        // 直接写入一行 RGB565 数据
        fret = f_write(&MyFile, buffer, width, &bw);
        if (FR_OK != fret || bw != width)
        {
            return -1;
        }
        buffer += width;

        // 写入填充字节（如果需要）
        fret = f_write(&MyFile, padding_data, padding, &bw);
        if (FR_OK != fret || bw != padding)
        {
            return -1;
        }
    }

    f_sync(&MyFile);
    f_close(&MyFile);

    _bmp_name_num++;
    _bmp_file_num++;
    return 0;
}



int8_t ff_save_bmp_raw(const uint16_t *buffer, uint16_t w, uint16_t h)
{
    FRESULT fret = FR_OK;
    FIL MyFile;

    if (_bmp_file_num == MAX_BMP_CNT)
    {
        return -2;
    }

    if (_get_free_size() < 132 * 1024)
    {
        /* 剩余空间不足 */
        return -3;
    }


    while (1)
    {
        sprintf(_save_bmp_name, "%u.dat", _bmp_name_num);
        fret = f_stat(_save_bmp_name, &_fileinfo);
        if (FR_NO_FILE == fret)
        {
            break;
        }
        else if (FR_OK == fret || FR_EXIST == fret)
        {
            _bmp_name_num++;
        }
        else if (FR_NO_FILESYSTEM == fret)
        {
            f_mount(&g_fatfs, g_rootfs, 1);
        }
        else if (FR_OK != fret)
        {
            return -1;
        }
        HAL_Delay(1);
    }

    fret = f_open(&MyFile, _save_bmp_name, FA_CREATE_ALWAYS | FA_WRITE);
    if (FR_OK != fret)
    {
        return -1;
    }

    const int width = w;
    const int height = h;

    const int bytesPerRow = width * 2;
    const int padding = (4 - (bytesPerRow % 4)) % 4;



    UINT bw = 0;


    uint8_t padding_data[3] = { 0, 0, 0 }; // 用于行对齐的填充字节
    for (int y = 0; y < height; y++)
    {
        // 直接写入一行 RGB565 数据
        fret = f_write(&MyFile, buffer, sizeof(uint16_t) * width, &bw);
        if (FR_OK != fret || bw != sizeof(uint16_t) * width)
        {
            return -1;
        }
        buffer += width;

        // 写入填充字节（如果需要）
        fret = f_write(&MyFile, padding_data, padding, &bw);
        if (FR_OK != fret || bw != padding)
        {
            return -1;
        }
    }

    f_sync(&MyFile);
    f_close(&MyFile);

    _bmp_name_num++;
    _bmp_file_num++;

    return 0;
}


BYTE work[FF_MAX_SS];
int8_t ff_format(void)
{
    FRESULT fret = FR_OK;

    sprintf(g_strbuff, "formating...");
    st7789_draw_string(30, 10, g_strbuff, Font_11x18, COLOR_RGB565_WHITE, COLOR_RGB565_BLACK);

    // fret = f_unmount(g_rootfs);
    // if(FR_OK != fret)
    // {
    //     sprintf(g_strbuff, "f_unmount:%d", fret);
    //     st7789_draw_string(30, 10, g_strbuff, Font_11x18, COLOR_RGB565_WHITE, COLOR_RGB565_BLACK);
    //     Error_Handler();
    //     return -1;
    // }
    MKFS_PARM param = {
        .fmt = FM_FAT,
        .n_fat = 2
    };

    fret = f_mkfs(g_rootfs, &param, work, sizeof(work));
    if(FR_OK != fret)
    {
        sprintf(g_strbuff, "f_mkfs:%d", fret);
        st7789_draw_string(30, 10, g_strbuff, Font_11x18, COLOR_RGB565_WHITE, COLOR_RGB565_BLACK);
        Error_Handler();
        return -1;
    }
    else
    {
        sprintf(g_strbuff, "format ok!");
        st7789_draw_string(30, 10, g_strbuff, Font_11x18, COLOR_RGB565_WHITE, COLOR_RGB565_BLACK);
    }

    if (f_mount(&g_fatfs, (TCHAR const *)g_rootfs, 1) != FR_OK)
    {
        sprintf(g_strbuff, "f_mount:%d", fret);
        st7789_draw_string(30, 10, g_strbuff, Font_11x18, COLOR_RGB565_WHITE, COLOR_RGB565_BLACK);
        Error_Handler();
        return -1;
    }
    else
    {
        sprintf(g_strbuff, "mount ok!");
        st7789_draw_string(30, 10, g_strbuff, Font_11x18, COLOR_RGB565_WHITE, COLOR_RGB565_BLACK);
    }

    st7789_clear_dma(COLOR_RGB565_GREEN);
   sprintf(g_strbuff, "%s",__DATE__);
   st7789_draw_string(20, 220, g_strbuff, Font_11x18, COLOR_RGB565_WHITE, COLOR_RGB565_BLACK);

    return 0;
}


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

