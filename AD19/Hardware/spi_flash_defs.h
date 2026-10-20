
#define CMD_AAI         0xAD  	    /* AAI 连续编程指令(FOR SST25VF016B) */
#define CMD_PP          0x02        /* AAI 连续编程指令(FOR SST25VF016B) */
#define CMD_DISWR       0x04        /* 禁止写, 退出AAI状态 */
#define CMD_EWRSR       0x50        /* 允许写状态寄存器的命令 */
#define CMD_WRSR        0x01  	    /* 写状态寄存器命令 */
#define CMD_WREN        0x06        /* 写使能命令 */
#define CMD_READ        0x03        /* 读数据区命令 */
#define CMD_FAST_READ   0x0B
#define CMD_RDSR        0x05        /* 读状态寄存器命令 */
#define CMD_RDSID       0x90        /* ManufactDeviceID */
#define CMD_RDLID       0x9F        /* 读器件ID命令 */
#define CMD_SE          0x20        /* 擦除扇区命令 */
#define CMD_BE          0xC7        /* 批量擦除命令 */
#define DUMMY_BYTE      0xA5        /* 哑命令，可以为任意值，用于读操作 */
#define WIP_FLAG        0x01        /* 状态寄存器中的正在编程标志（WIP) */



/* Manufacture ID and Short Device ID */
#define ZD25WQ32C_SHORT_ID      0xBA15

/* Manufacture ID and Short Device ID */
#define GD25Q32EEIGR_ID      0x0000C815

/* Manufacture ID and Long Device ID */
#define ZD25WQ32C_LONG_ID       0xBA6015



//W25X系列/Q系列芯片列表
//W25Q80  ID  0XEF13
//W25Q16  ID  0XEF14
//W25Q32  ID  0XEF15
//W25Q64  ID  0XEF16
//W25Q128 ID  0XEF17
//W25Q256 ID  0XEF18
#define W25Q80 	0XEF13
#define W25Q16 	0XEF14
#define W25Q32 	0XEF15
#define W25Q64 	0XEF16
#define W25Q128	0XEF17
#define W25Q256 0XEF18

#define NM25Q80 	0X5213
#define NM25Q16 	0X5214
#define NM25Q32 	0X5215
#define NM25Q64 	0X5216
#define NM25Q128	0X5217
#define NM25Q256 	0X5218

//指令表
// #define W25X_WriteEnable		0x06
// #define W25X_WriteDisable		0x04
// #define W25X_ReadStatusReg1		0x05
// #define W25X_ReadStatusReg2		0x35
// #define W25X_ReadStatusReg3		0x15
// #define W25X_WriteStatusReg1    0x01
// #define W25X_WriteStatusReg2    0x31
// #define W25X_WriteStatusReg3    0x11
// #define W25X_ReadData			0x03
// #define W25X_FastReadData		0x0B
// #define W25X_FastReadDual		0x3B
// #define W25X_PageProgram		0x02
// #define W25X_BlockErase			0xD8
// #define W25X_SectorErase		0x20
// #define W25X_ChipErase			0xC7
// #define W25X_PowerDown			0xB9
// #define W25X_ReleasePowerDown	0xAB
// #define W25X_DeviceID			0xAB
// #define W25X_ManufactDeviceID	0x90
// #define W25X_JedecDeviceID		0x9F
// #define W25X_Enable4ByteAddr    0xB7
// #define W25X_Exit4ByteAddr      0xE9





