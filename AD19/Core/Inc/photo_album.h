#ifndef __PHOTO_ALBUM_H__
#define __PHOTO_ALBUM_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#define PHOTO_ALBUM_MAX_ITEMS       (60U)
#define PHOTO_ALBUM_FILE_NAME_LEN   (32U)

typedef struct PhotoAlbumNode PhotoAlbumNode;
struct PhotoAlbumNode
{
    char name[PHOTO_ALBUM_FILE_NAME_LEN];
    uint32_t order;
    PhotoAlbumNode *prev;
    PhotoAlbumNode *next;
};

void PhotoAlbum_Init(void);
int8_t PhotoAlbum_Rebuild(void);
int8_t PhotoAlbum_AddFile(const char *name);
int8_t PhotoAlbum_Enter(void);
int8_t PhotoAlbum_ShowCurrent(void);
int8_t PhotoAlbum_ShowNext(void);
int8_t PhotoAlbum_ShowPrevious(void);
void PhotoAlbum_ShowDeleteSelect(void);
int8_t PhotoAlbum_DeleteCurrent(void);
uint16_t PhotoAlbum_Count(void);

#ifdef __cplusplus
}
#endif

#endif /* __PHOTO_ALBUM_H__ */
