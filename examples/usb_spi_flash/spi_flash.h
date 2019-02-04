#ifndef	__SPI_FLASH_H__
#define __SPI_FLASH_H__

void Flash_ReleaseDeepPowerdown();

uint8_t Flash_ReadStatusRegister();

void Flash_ReadJEDECID();

void Flash_EraseChip();

// Starts a flash erase and returns immediately. Use Flash_Busy() to poll for
// completion
void Flash_EraseChip_NonBlocking();

// Erase a single (normally 4k) sector of the flash chip.
void Flash_EraseSector(uint32_t address);

void Flash_EraseBlock32K(uint32_t address);
void Flash_EraseBlock64K(uint32_t address);

// Returns 1 if the flash is processing a command (erase), or 0 if it is finished.
__bit Flash_Busy();


void Flash_Read(uint32_t address, uint16_t length, uint8_t *flashData);
void Flash_Write(uint32_t address,
                 uint16_t length,
                 uint8_t  *flashData);

#define CMD_READJEDECID             (0x9F)
#define CMD_READSTATUSREGISTER      (0x05)

#define CMD_WRITE_ENABLE            (0x06)
#define CMD_WRITE_DISABLE           (0x04)

#define CMD_READ                    (0x03)
#define CMD_ARRAYREAD               (0x0B)
#define CMD_PAGEPROGRAM             (0x02)

#define CMD_SECTOR_ERASE            (0x20)
#define CMD_BLOCK_ERASE_32K         (0x52)
#define CMD_BLOCK_ERASE_64K         (0xD8)
#define CMD_CHIP_ERASE              (0x60)

#define CMD_DEEP_POWERDOWN          (0xB9)
#define CMD_RELEASE_DEEP_POWERDOWN  (0xAB)

#define STATUS_REG_SRP          (1<<7)
#define STATUS_REG_B            (1<<5)
#define STATUS_REG_BP1          (1<<3)
#define STATUS_REG_BP2          (1<<2)
#define STATUS_REG_WEL          (1<<1)
#define STATUS_REG_BUSY         (1<<0)

#endif

