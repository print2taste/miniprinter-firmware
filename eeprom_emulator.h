#pragma once
#define PAGE_SIZE  (uint16_t)0x800  /* Page size = 2KByte */
#define EEPROM_START_ADDRESS    ((uint32_t)0x0807C000) /* EEPROM emulation start address: after 64KByte of used Flash memory */
#define PAGE0_BASE_ADDRESS      ((uint32_t)(EEPROM_START_ADDRESS + 0x000))
#define PAGE0_END_ADDRESS       ((uint32_t)(EEPROM_START_ADDRESS + (4*PAGE_SIZE - 1)))
#define PAGE1_BASE_ADDRESS      ((uint32_t)(EEPROM_START_ADDRESS + 4*PAGE_SIZE))
#define PAGE1_END_ADDRESS       ((uint32_t)(EEPROM_START_ADDRESS + (8* PAGE_SIZE - 1)))
#define PAGE0                   ((uint16_t)0x0000)
#define PAGE1                   ((uint16_t)0x0001)
#define NO_VALID_PAGE           ((uint16_t)0x00AB)

#define ERASED                  ((uint16_t)0xFFFF)     /* PAGE is empty */
#define RECEIVE_DATA            ((uint16_t)0xEEEE)     /* PAGE is marked to receive data */
#define VALID_PAGE              ((uint16_t)0x0000)     /* PAGE containing valid data */

#define READ_FROM_VALID_PAGE    ((uint8_t)0x00)
#define WRITE_IN_VALID_PAGE     ((uint8_t)0x01)

#define PAGE_FULL               ((uint8_t)0x80)

#define NumbOfVar               ((uint16_t)1500)//�洢���������ֵ���ɵ��������2048�����ʹ��ҳʱ�洢���ݱ���

uint16_t EE_Init(void);
uint16_t EE_ReadVariable(uint16_t VirtAddress, uint16_t* Data);
uint16_t EE_WriteVariable(uint16_t VirtAddress, uint16_t Data);
