#include "../main.h"

// STM32F429
#define SETTINGS_FLASH_ADDR         ((uint32_t)0x080E0000)
#define SETTINGS_FLASH_LEN          0x20000

#define SETTINGS_FLASH_SECTOR_ID    11

#define MMIO16(addr)                (*(volatile uint16_t *)(addr))
#define MMIO32(addr)                (*(volatile uint32_t *)(addr))

#define FLASH_KEYR_KEY1             ((uint32_t)0x45670123)
#define FLASH_KEYR_KEY2             ((uint32_t)0xcdef89ab)

typedef struct settings_t {
  char password[32];
  settings_preset_t preset[SETTINGS_PRESETS_NUMBER];
} settings_t;

typedef struct storage_settings_t {
  settings_t settings;
  uint32_t crc32;
} storage_settings_t;

static storage_settings_t settings_storage;
static mutex_t settings_mutex;

static void flash_lock(void)
{
  FLASH->CR |= FLASH_CR_LOCK;
}

static void flash_unlock(void)
{
  /* Clear the unlock state. */
  FLASH->CR |= FLASH_CR_LOCK;

  /* Authorize the FPEC access. */
  FLASH->KEYR = FLASH_KEYR_KEY1;
  FLASH->KEYR = FLASH_KEYR_KEY2;
}

static uint32_t flash_get_status_flags(void)
{
  return FLASH->SR & (FLASH_SR_PGSERR |
          FLASH_SR_EOP |
          FLASH_SR_WRPERR |
          FLASH_SR_BSY);
}

static void flash_wait_for_last_operation(void)
{
  while ((flash_get_status_flags() & FLASH_SR_BSY) == FLASH_SR_BSY);
}

__attribute__ ((unused)) static void flash_program_half_word(uintptr_t address, uint16_t data)
{
  flash_wait_for_last_operation();
  FLASH->CR |= FLASH_CR_PG;
  MMIO16(address) = data;
  flash_wait_for_last_operation();
  FLASH->CR &= ~FLASH_CR_PG;
}

static void flash_program_word(uint32_t address, uint32_t data)
{
  //flash_program_half_word(address, (uint16_t)data);
  //flash_program_half_word(address+2, (uint16_t)(data>>16));

  flash_wait_for_last_operation();
  FLASH->CR |= FLASH_CR_PG;
  MMIO32(address) = data;
  flash_wait_for_last_operation();
  FLASH->CR &= ~FLASH_CR_PG;
}

static void flash_erase_sector(uint32_t sector_id)
{
  flash_wait_for_last_operation();
  FLASH->CR |= FLASH_CR_SER | (FLASH_CR_SNB & ((sector_id) << 3));
  FLASH->CR |= FLASH_CR_STRT;
  flash_wait_for_last_operation();
  FLASH->CR &= ~FLASH_CR_SER;
}

static void settings_write(void)
{
  uint8_t i;
  uintptr_t* ptr;

  settings_storage.crc32 = crc32((uint8_t*)&(settings_storage.settings), sizeof(settings_t));

  // Erase the current settings
  flash_unlock();
  flash_erase_sector(SETTINGS_FLASH_SECTOR_ID);

  // Loop through the settings_t word-by-word and write to flash
  ptr = (uintptr_t *)&settings_storage;
  for(i = 0; i < sizeof(storage_settings_t)/sizeof(uintptr_t); i++)
  {
    flash_program_word(SETTINGS_FLASH_ADDR + 4*i, *ptr++);
  }

  // Relock flash
  flash_lock();
}

static void settings_read(void)
{
  uintptr_t i;
  uintptr_t* ptr;

  // Read through the settings_t
  ptr = (uintptr_t *)&settings_storage;
  for(i = 0; i < sizeof(storage_settings_t)/sizeof(uintptr_t); i++)
  {
    *ptr++ = *(uintptr_t *)(SETTINGS_FLASH_ADDR + 4*i);
  }
}

static bool settings_checkcrc(void)
{
  return (crc32((uint8_t*)&(settings_storage.settings), sizeof(settings_t)) == settings_storage.crc32);
}

static void settings_restore_defaults(void)
{
  strncpy(settings_storage.settings.password, "g5500", 31);

  for(int i=0; i<SETTINGS_PRESETS_NUMBER; i++)
  {
    settings_storage.settings.preset[i].valid = false;
  }
}

void settings_init(void)
{
  chMtxObjectInit(&settings_mutex);

  chMtxLock(&settings_mutex);

  settings_read();

  if(!settings_checkcrc())
  {
    settings_restore_defaults();

    settings_write();
  }

  chMtxUnlock(&settings_mutex);
}

bool settings_check_password(char *candidate)
{
  chMtxLock(&settings_mutex);

  if(0 != strncmp(candidate, settings_storage.settings.password, strlen(settings_storage.settings.password)))
  {
    chMtxUnlock(&settings_mutex);
    return false;
  }
  chMtxUnlock(&settings_mutex);
  return true;
}

void settings_set_password(char *new_password)
{
  chMtxLock(&settings_mutex);

  strncpy(settings_storage.settings.password, new_password, 31);
  settings_write();

  chMtxUnlock(&settings_mutex);
}

void settings_get_presets(settings_preset_t *presets_ptr)
{
  chMtxLock(&settings_mutex);

  memcpy((uint8_t *)presets_ptr, (uint8_t *)settings_storage.settings.preset, sizeof(settings_preset_t) * SETTINGS_PRESETS_NUMBER);

  chMtxUnlock(&settings_mutex);
}

void settings_set_presets(settings_preset_t *presets_ptr)
{
  chMtxLock(&settings_mutex);

  memcpy((uint8_t *)settings_storage.settings.preset, (uint8_t *)presets_ptr, sizeof(settings_preset_t) * SETTINGS_PRESETS_NUMBER);
  settings_write();
  
  chMtxUnlock(&settings_mutex);
}