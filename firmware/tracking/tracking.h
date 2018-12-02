#ifndef __TRACKING_H__
#define __TRACKING_H__

#define TRACKING_STACK_SIZE	2048

#define TRACKING_AZ_STOP_POSITION	180*10

#define TRACKING_AZ_HYSTERESIS	1.5*10 // deci-degrees
#define TRACKING_AZ_OVERSHOOT   1.0*10 // deci-degrees

#define TRACKING_EL_HYSTERESIS	0.5*10 // deci-degrees
#define TRACKING_EL_OVERSHOOT   -0.1*10 // deci-degrees

struct tracking_state_t {

  mutex_t state_mutex;

  int16_t az_raw;
  int16_t el_raw;

  int16_t az_ddeg;
  int16_t el_ddeg;

  bool cw;
  bool ccw;

  bool up;
  bool down;

  mutex_t desired_mutex;

  int16_t desired_az_ddeg;
  int16_t desired_el_ddeg;
};

void tracking_init(void);

void tracking_copy_state(struct tracking_state_t *state_copy);

void tracking_get_state_ddeg(int16_t *az_ddeg_ptr, int16_t *el_ddeg_ptr);

void tracking_set_desired_ddeg(int16_t *az_ddegrees_ptr, int16_t *el_ddegrees_ptr);

void tracking_set_stop(void);

#endif /* __TRACKING_H__ */