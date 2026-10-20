#include "photo_album.h"

#include "bsp.h"
#include "ff.h"
#include "sd_diskio.h"

/* BMP 文件头。使用 __PACKED 避免编译器为结构体对齐插入填充字节。 */
typedef struct __PACKED {
  uint16_t signature;
  uint32_t fileSize;
  uint16_t reserved1;
  uint16_t reserved2;
  uint32_t dataOffset;
} PhotoBmpFileHeader;

/* BMP 信息头。本相册只支持固定尺寸的 16bit RGB565 BMP。 */
typedef struct __PACKED {
  uint32_t headerSize;
  int32_t width;
  int32_t height;
  uint16_t planes;
  uint16_t bitsPerPixel;
  uint32_t compression;
  uint32_t imageSize;
  int32_t xPixelsPerMeter;
  int32_t yPixelsPerMeter;
  uint32_t colorsUsed;
  uint32_t colorsImportant;
} PhotoBmpInfoHeader;

#define PHOTO_ALBUM_SCREEN_W (240U)
#define PHOTO_ALBUM_SCREEN_H (320U)
#define PHOTO_ALBUM_IMAGE_W (240U)
#define PHOTO_ALBUM_IMAGE_H (280U)
#define PHOTO_ALBUM_DISPLAY_Y (2U)
#define PHOTO_ALBUM_DISPLAY_H (285U)
#define PHOTO_ALBUM_STATUS_X_PAD (8U)
#define PHOTO_ALBUM_STATUS_Y (4U)
#define PHOTO_ALBUM_BMP_DATA_OFFSET (70U)
#define PHOTO_ALBUM_SELECT_Y (225U)
#define PHOTO_ALBUM_SELECT_H (60U)

/* 相册节点池：不使用 malloc，避免嵌入式系统中的内存碎片和不可预测问题。 */
static PhotoAlbumNode s_nodes[PHOTO_ALBUM_MAX_ITEMS];
/* 双向链表头、尾、当前节点。 */
static PhotoAlbumNode *s_head = 0;
static PhotoAlbumNode *s_tail = 0;
static PhotoAlbumNode *s_current = 0;
static uint16_t s_count = 0;
/* s_dirty 表示相册缓存和文件系统可能不同步，需要重新扫描目录。 */
static uint8_t s_dirty = 1;
/* 相册显示复用全局显示缓冲，先读完整张图再一次性刷屏。 */
extern uint16_t g_disp_buffer[320][240];

/* 判断文件名是否为 .bmp/.BMP；相册只管理 BMP 文件。 */
static uint8_t album_is_bmp_name(const char *name) {
  uint32_t len;
  const char *ext;

  if (name == 0) {
    return 0;
  }

  len = strlen(name);
  if (len <= 4U) {
    return 0;
  }

  ext = name + len - 4U;
  return (ext[0] == '.' && (ext[1] == 'b' || ext[1] == 'B') &&
          (ext[2] == 'm' || ext[2] == 'M') && (ext[3] == 'p' || ext[3] == 'P'));
}

/* 从文件名开头提取数字作为排序值，例如 "12.bmp" 的 order 为 12。 */
static uint32_t album_name_order(const char *name) {
  uint32_t order = 0;

  if (name == 0) {
    return 0;
  }

  while (*name >= '0' && *name <= '9') {
    order = (order * 10U) + (uint32_t)(*name - '0');
    name++;
  }

  return order;
}

/* 清空 RAM 中的相册链表，不会删除 Flash 里的真实文件。 */
static void album_clear_list(void) {
  memset(s_nodes, 0, sizeof(s_nodes));
  s_head = 0;
  s_tail = 0;
  s_current = 0;
  s_count = 0;
}

/* 按文件名前缀数字顺序插入链表，并把新节点设为当前节点。 */
static int8_t album_insert_sorted(const char *name) {
  PhotoAlbumNode *node;
  PhotoAlbumNode *pos;
  uint32_t order;

  if (name == 0 || s_count >= PHOTO_ALBUM_MAX_ITEMS) {
    return -1;
  }

  order = album_name_order(name);
  /* 当前实现用 s_count 作为静态节点池下标。删除后会重扫目录恢复干净节点池。 */
  node = &s_nodes[s_count];
  memset(node, 0, sizeof(*node));
  strncpy(node->name, name, PHOTO_ALBUM_FILE_NAME_LEN - 1U);
  node->order = order;

  /* 链表为空时，新节点同时是头、尾、当前节点。 */
  if (s_head == 0) {
    s_head = node;
    s_tail = node;
    s_current = node;
    s_count++;
    return 0;
  }

  pos = s_head;
  /* 找到第一个 order 大于当前文件 order 的节点，插入到它前面。 */
  while (pos != 0 && pos->order <= order) {
    pos = pos->next;
  }

  /* 插入到链表头部。 */
  if (pos == s_head) {
    node->next = s_head;
    s_head->prev = node;
    s_head = node;
  }
  /* 插入到链表尾部。 */
  else if (pos == 0) {
    node->prev = s_tail;
    s_tail->next = node;
    s_tail = node;
  }
  /* 插入到链表中间。 */
  else {
    node->prev = pos->prev;
    node->next = pos;
    pos->prev->next = node;
    pos->prev = node;
  }

  s_current = node;
  s_count++;
  return 0;
}

/* 计算当前图片在相册中的序号，用于显示 3/10 这样的页码。 */
static uint16_t album_current_index(void) {
  uint16_t index = 1;
  PhotoAlbumNode *node = s_head;

  while (node != 0 && node != s_current) {
    index++;
    node = node->next;
  }

  return index;
}

/* 按文件名查找链表节点，保存新图片后避免重复加入链表。 */
static PhotoAlbumNode *album_find_node(const char *name) {
  PhotoAlbumNode *node = s_head;

  while (node != 0) {
    if (strncmp(node->name, name, PHOTO_ALBUM_FILE_NAME_LEN) == 0) {
      return node;
    }
    node = node->next;
  }

  return 0;
}

/* 从双向链表中摘掉一个节点；这里只删除 RAM 节点，不删除 Flash 文件。 */
static void album_unlink_node(PhotoAlbumNode *node) {
  PhotoAlbumNode *next_current;

  if (node == 0) {
    return;
  }

  /* 删除当前节点后，优先显示下一张；如果没有下一张，则显示上一张。 */
  next_current = (node->next != 0) ? node->next : node->prev;

  /* 维护前驱节点的 next；如果没有前驱，说明删除的是头节点。 */
  if (node->prev != 0) {
    node->prev->next = node->next;
  } else {
    s_head = node->next;
  }

  /* 维护后继节点的 prev；如果没有后继，说明删除的是尾节点。 */
  if (node->next != 0) {
    node->next->prev = node->prev;
  } else {
    s_tail = node->prev;
  }

  /* 如果删除的是当前显示节点，则把当前节点移动到下一张或上一张。 */
  if (s_current == node) {
    s_current = next_current;
  }

  if (s_count > 0U) {
    s_count--;
  }

  /* 清空节点内容，避免后续误读旧文件名。 */
  memset(node, 0, sizeof(*node));
}

/* 相册为空时显示提示。 */
static void album_show_empty(void) {
  st7789_clear_dma(COLOR_RGB565_BLACK);
  st7789_draw_string(38, 136, "No pictures", Font_16x26, COLOR_RGB565_WHITE,
                     COLOR_RGB565_BLACK);
  st7789_draw_string(30, 172, "Double tap exit", Font_11x18, COLOR_RGB565_WHITE,
                     COLOR_RGB565_BLACK);
}

/* 相册出错时显示提示，例如打开失败、BMP 格式不匹配等。 */
static void album_show_error(const char *msg) {
  st7789_clear_dma(COLOR_RGB565_BLACK);
  st7789_draw_string(28, 136, msg, Font_16x26, COLOR_RGB565_WHITE,
                     COLOR_RGB565_BLACK);
  st7789_draw_string(20, 172, "Tap or swipe retry", Font_11x18,
                     COLOR_RGB565_WHITE, COLOR_RGB565_BLACK);
}

/* 透明绘制字符串：只画文字像素，不覆盖图片背景。 */
static void album_draw_transparent_string(uint16_t x, uint16_t y,
                                          const char *str, FontDef font,
                                          uint16_t color) {
  uint32_t row;
  uint32_t col;
  uint32_t bits;

  while (*str != '\0') {
    if (x + font.width >= PHOTO_ALBUM_SCREEN_W) {
      break;
    }

    if (*str >= ' ' && *str <= '~') {
      for (row = 0; row < font.height; row++) {
        bits = font.data[(*str - 32) * font.height + row];
        for (col = 0; col < font.width; col++) {
          if ((bits << col) & 0x8000U) {
            st7789_draw_pixel((uint16_t)(x + col), (uint16_t)(y + row),
                              color);
          }
        }
      }
    }

    x = (uint16_t)(x + font.width);
    str++;
  }
}

static void album_show_status(void) {
  char text[16U];
  uint16_t text_w;
  uint16_t x;
  uint16_t y;

  snprintf(text, sizeof(text), "%u/%u", album_current_index(), s_count);
  text_w = (uint16_t)(strlen(text) * Font_11x18.width);
  x = ((text_w * 2U) + PHOTO_ALBUM_STATUS_X_PAD > PHOTO_ALBUM_SCREEN_W)
          ? 0U
          : (uint16_t)(PHOTO_ALBUM_SCREEN_W - (text_w * 2U) -
                       PHOTO_ALBUM_STATUS_X_PAD);
  y = (uint16_t)(PHOTO_ALBUM_DISPLAY_Y + PHOTO_ALBUM_STATUS_Y +
                 (text_w / 3U));

  album_draw_transparent_string(x, y, text, Font_11x18, COLOR_RGB565_WHITE);
}

/* 从 FatFs 文件系统读取 BMP，并显示到 LCD。 */
static int8_t album_read_and_show_bmp(const char *name) {
  FIL file;
  FRESULT res;
  UINT read_bytes;
  PhotoBmpFileHeader file_header;
  PhotoBmpInfoHeader info_header;
  uint16_t row;
  uint16_t src_y;
  int32_t bmp_height_abs;
  uint32_t row_bytes;
  uint32_t padding;
  uint32_t row_stride;

  /* 打开当前图片文件，底层会通过 disk_read 访问外部 Flash。 */
  res = f_open(&file, name, FA_READ | FA_OPEN_EXISTING);
  if (res != FR_OK) {
    album_show_error("Open fail");
    return -1;
  }

  /* 读取 BMP 文件头，并检查签名是否为 BM，即 0x4D42。 */
  res = f_read(&file, &file_header, sizeof(file_header), &read_bytes);
  if (res != FR_OK || read_bytes != sizeof(file_header) ||
      file_header.signature != 0x4D42U) {
    f_close(&file);
    album_show_error("Bad BMP");
    return -2;
  }

  /* 读取 BMP 信息头，用于确认宽、高、位深等信息。 */
  res = f_read(&file, &info_header, sizeof(info_header), &read_bytes);
  if (res != FR_OK || read_bytes != sizeof(info_header)) {
    f_close(&file);
    album_show_error("Read fail");
    return -3;
  }

  /* 本相册只支持 240x280 的 16bit BMP；高度可正可负，对应两种行存储方向。 */
  if (info_header.width != PHOTO_ALBUM_IMAGE_W ||
      (info_header.height != -(int32_t)PHOTO_ALBUM_IMAGE_H &&
       info_header.height != (int32_t)PHOTO_ALBUM_IMAGE_H) ||
      info_header.bitsPerPixel != 16U) {
    f_close(&file);
    album_show_error("Bad size");
    return -4;
  }

  bmp_height_abs =
      (info_header.height < 0) ? -info_header.height : info_header.height;
  row_bytes = (uint32_t)info_header.width * 2U;
  /* BMP 每行按 4 字节对齐，所以需要计算行尾 padding；RGB565 每像素 2 字节。 */
  padding = (4U - (row_bytes % 4U)) % 4U;
  row_stride = row_bytes + padding;

  /* 逐行读取到显示缓冲，读完后再一次性刷屏 */
  for (row = 0; row < PHOTO_ALBUM_IMAGE_H; row++) {
    /* height < 0 表示 BMP 自上而下存储；height > 0 表示自下而上存储。 */
    if (info_header.height < 0) {
      src_y = row;
    } else {
      src_y = (uint16_t)(bmp_height_abs - 1 - row);
    }

    /* 定位到当前要读取的 BMP 行。 */
    res =
        f_lseek(&file, file_header.dataOffset + ((uint32_t)src_y * row_stride));
    if (res != FR_OK) {
      f_close(&file);
      album_show_error("Seek fail");
      return -5;
    }

    /* 读取一行 RGB565 像素到显示缓冲。 */
    res = f_read(&file, g_disp_buffer[row], row_bytes, &read_bytes);
    if (res != FR_OK || read_bytes != row_bytes) {
      f_close(&file);
      album_show_error("Read fail");
      return -6;
    }

    /* 读图完成后，一次性 DMA 显示整张图片。 */
  }

  f_close(&file);
  st7789_draw_image_dma(0, 0, PHOTO_ALBUM_IMAGE_W, PHOTO_ALBUM_DISPLAY_Y,
                        (const uint16_t *)g_disp_buffer);
  st7789_draw_image_dma(0, PHOTO_ALBUM_DISPLAY_Y, PHOTO_ALBUM_IMAGE_W,
                        PHOTO_ALBUM_IMAGE_H,
                        (const uint16_t *)g_disp_buffer);
  for (row = PHOTO_ALBUM_DISPLAY_Y + PHOTO_ALBUM_IMAGE_H;
       row < PHOTO_ALBUM_DISPLAY_H; row++) {
    st7789_draw_image_dma(0, row, PHOTO_ALBUM_IMAGE_W, 1U,
                          g_disp_buffer[PHOTO_ALBUM_IMAGE_H - 1U]);
  }
  st7789_fill_rect_dma(0, PHOTO_ALBUM_DISPLAY_H,
                       PHOTO_ALBUM_SCREEN_W,
                       PHOTO_ALBUM_SCREEN_H - PHOTO_ALBUM_DISPLAY_H,
                       COLOR_RGB565_BLACK);
  album_show_status();
  return 0;
}

/* 显示删除确认界面：中间提示图标，底部左删右取消选择条。 */
void PhotoAlbum_ShowDeleteSelect(void) {
  if (s_current == 0) {
    return;
  }

  st7789_draw_image_dma((PHOTO_ALBUM_SCREEN_W - 0x78U) / 2U,
                        PHOTO_ALBUM_DISPLAY_Y +
                            ((PHOTO_ALBUM_IMAGE_H - 0x50U) / 2U),
                        0x78U, 0x50U,
                        (uint16_t *)gImage_icon_info_delete);
  st7789_draw_image_dma(0, PHOTO_ALBUM_SELECT_Y, PHOTO_ALBUM_SCREEN_W,
                        PHOTO_ALBUM_SELECT_H,
                        (uint16_t *)gImage_icon_save_delete);
}

/* 初始化相册模块；实际文件列表会在进入相册或显式 Rebuild 时扫描。 */
void PhotoAlbum_Init(void) {
  album_clear_list();
  s_dirty = 1;
}

/* 重新扫描 FatFs 根目录，重建相册链表。 */
int8_t PhotoAlbum_Rebuild(void) {
  FRESULT res;
  DIR dir;
  FILINFO file_info;

  album_clear_list();

  /* 打开根目录；当前保存的 BMP 文件都放在根目录下。 */
  res = f_opendir(&dir, "/");
  if (res != FR_OK) {
    s_dirty = 1;
    return -1;
  }

  while (1) {
    /* 逐个读取目录项，fname[0] == '\0' 表示目录读取完毕。 */
    res = f_readdir(&dir, &file_info);
    if (res != FR_OK || file_info.fname[0] == '\0') {
      break;
    }

    /* 只把普通 BMP 文件加入相册，忽略目录和其他类型文件。 */
    if ((file_info.fattrib & AM_DIR) == 0 &&
        album_is_bmp_name(file_info.fname)) {
      album_insert_sorted(file_info.fname);
    }
  }

  f_closedir(&dir);
  if (s_tail != 0) {
    /* 重扫后默认定位到最后一张，一般就是最新保存的图片。 */
    s_current = s_tail;
  }
  s_dirty = 0;
  return (res == FR_OK) ? 0 : -2;
}

/* 保存新图片后调用，把新文件加入相册缓存。 */
int8_t PhotoAlbum_AddFile(const char *name) {
  PhotoAlbumNode *node;

  if (!album_is_bmp_name(name)) {
    return -1;
  }

  if (s_dirty) {
    /* 如果相册缓存可能过期，先按文件系统真实状态重建。 */
    PhotoAlbum_Rebuild();
  }

  node = album_find_node(name);
  if (node != 0) {
    /* 文件已经在链表中，直接定位到该节点。 */
    s_current = node;
    s_dirty = 0;
    return 0;
  }

  if (album_insert_sorted(name) == 0) {
    s_dirty = 0;
    return 0;
  }

  s_dirty = 1;
  return -2;
}

/* 进入相册：必要时重扫文件系统，然后显示当前图片。 */
int8_t PhotoAlbum_Enter(void) {
  if (s_dirty || s_count == 0U) {
    PhotoAlbum_Rebuild();
  }

  if (s_count == 0U || s_current == 0) {
    album_show_empty();
    return -1;
  }

  return PhotoAlbum_ShowCurrent();
}

/* 显示当前节点对应的图片。 */
int8_t PhotoAlbum_ShowCurrent(void) {
  if (s_current == 0) {
    album_show_empty();
    return -1;
  }

  return album_read_and_show_bmp(s_current->name);
}

/* 显示下一张；到尾部后回到头部，实现循环浏览。 */
int8_t PhotoAlbum_ShowNext(void) {
  if (s_current == 0) {
    album_show_empty();
    return -1;
  }

  s_current = (s_current->next != 0) ? s_current->next : s_head;
  return PhotoAlbum_ShowCurrent();
}

/* 显示上一张；到头部后回到尾部，实现循环浏览。 */
int8_t PhotoAlbum_ShowPrevious(void) {
  if (s_current == 0) {
    album_show_empty();
    return -1;
  }

  s_current = (s_current->prev != 0) ? s_current->prev : s_tail;
  return PhotoAlbum_ShowCurrent();
}

/* 删除当前图片：先删除 FatFs 文件，再更新 RAM 链表。 */
int8_t PhotoAlbum_DeleteCurrent(void) {
  char name[PHOTO_ALBUM_FILE_NAME_LEN];
  PhotoAlbumNode *delete_node;
  FRESULT res;

  if (s_current == 0) {
    album_show_empty();
    return -1;
  }

  /* 先拷贝文件名，因为后面删除链表节点时会清空节点内容。 */
  delete_node = s_current;
  strncpy(name, delete_node->name, sizeof(name) - 1U);
  name[sizeof(name) - 1U] = '\0';

  /*
   * f_unlink 是 FatFs 的文件删除 API。
   * 它会修改目录项和 FAT 表，并通过 disk_write 写回外部 Flash。
   * 如果这里失败，不能删除 RAM 链表节点，否则内存状态会和文件系统不一致。
   */
  res = f_unlink(name);
  if (res != FR_OK) {
    album_show_error("Delete fail");
    return -2;
  }

  /* 文件系统删除成功后，再从相册链表删除当前节点，并立即重扫目录。 */
  album_unlink_node(delete_node);
  PhotoAlbum_Rebuild();
  if (s_current == 0 || s_count == 0U) {
    album_show_empty();
    return 0;
  }

  return PhotoAlbum_ShowCurrent();
}

/* 返回当前相册中的图片数量。 */
uint16_t PhotoAlbum_Count(void) { return s_count; }
