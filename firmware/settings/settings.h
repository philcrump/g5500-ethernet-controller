#ifndef __SETTINGS_H__
#define __SETTINGS_H__

typedef struct settings_preset_t {
  bool valid;
  uint16_t ddeg;
  char name[32];
} settings_preset_t;

#define SETTINGS_PRESETS_NUMBER   20

void settings_init(void);

bool settings_check_password(char *candidate);

void settings_get_presets(settings_preset_t *presets_ptr);

void settings_set_presets(settings_preset_t *presets_ptr);

#endif /* __SETTINGS_H__ */