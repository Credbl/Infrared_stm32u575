#ifndef	__IMAGES_H__
#define	__IMAGES_H__

#include <stdint.h>


#define	BACKGROUND_COLOR			((uint16_t)0x0000)

/* img2lcd 工具生成数据，使用
   C语言数组
   水平扫描
   16位真彩色
   不勾选 MSB */

/* 128x128 pixel RGB565 image */
extern const uint16_t image_saber[][128];
extern const uint16_t fry_data[114][77];

extern const unsigned char gImage_COLOUR_ARCTIC[];
extern const unsigned char gImage_COLOUR_BLACKHOT[];
extern const unsigned char gImage_COLOUR_COLOUR1[];
extern const unsigned char gImage_COLOUR_COLOUR2[];
extern const unsigned char gImage_COLOUR_FUSION[];
extern const unsigned char gImage_COLOUR_GLOBOW[];
extern const unsigned char gImage_COLOUR_GREENHOT[];
extern const unsigned char gImage_COLOUR_GREYRED[];
extern const unsigned char gImage_COLOUR_ICEFIRE[];
extern const unsigned char gImage_COLOUR_INSTALERT[];
extern const unsigned char gImage_COLOUR_IRONBOW1[];
extern const unsigned char gImage_COLOUR_IRONBOW2[];
extern const unsigned char gImage_COLOUR_LAVA[];
extern const unsigned char gImage_COLOUR_RAIN[];
extern const unsigned char gImage_COLOUR_RAINBOW[];
extern const unsigned char gImage_COLOUR_REDHOT[];
extern const unsigned char gImage_COLOUR_SEPIA[];
extern const unsigned char gImage_COLOUR_WHITEHOT[];
extern const unsigned char icon_spot_center[];
extern const unsigned char icon_spot_hot[];
extern const unsigned char icon_spot_cold[];
extern const unsigned char icon_battery100[];
extern const unsigned char icon_battery60[];
extern const unsigned char icon_battery30[];
extern const unsigned char icon_battery0[];
extern const unsigned char icon_charging[];
extern const unsigned char icon_logo_dark[];
extern const unsigned char icon_logo_dark_anx[];
extern const unsigned char icon_logo_dark_laureii[];
extern const unsigned char icon_logo_dark_default[];
extern const unsigned char icon_logo_laureii_ad19u[];
extern const unsigned char icon_logo_anx_ad19u[];
extern const unsigned char icon_logo_default_ad19u[];
extern const unsigned char icon_logo_anx_ad19fs[];
extern const unsigned char icon_save_failed[];
extern const unsigned char icon_save_success[];
extern const unsigned char icon_save_select[];
extern const unsigned char icon_info_saving[];
extern const unsigned char gImage_icon_info_delete[];
extern const unsigned char gImage_icon_save_delete[];

/* 240x240 pixel RGB565	image */
// extern const	uint16_t image_knky[][240];
// extern const	uint16_t image_tek[][240];
// extern const	uint16_t image_adi1[][240];



typedef	struct IMAGE_ELEMENT
{
    uint16_t x;
    uint16_t y;
    uint16_t w;
    uint16_t h;
    const uint16_t	   *pdata;
    uint8_t	 bpp;
} IMAGE_ELEMENT;

#endif



