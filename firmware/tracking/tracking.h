#ifndef __TRACKING_H__
#define __TRACKING_H__

#define TRACKING_STACK_SIZE	2048

#define TRACKING_AZ_HYSTERESIS	1.5*10
#define TRACKING_AZ_OVERSHOOT   1.0*10

#define TRACKING_EL_HYSTERESIS	0.5*10
#define TRACKING_EL_OVERSHOOT   -0.1*10

struct tracking_state_t {
	int16_t az_raw;
	int16_t el_raw;

	int16_t az_ddeg;
	int16_t el_ddeg;

	int16_t az_deg;
	int16_t el_deg;

	uint8_t cw;
	uint8_t ccw;

	uint8_t up;
	uint8_t down;

	int16_t desired_az_raw;
	int16_t desired_el_raw;

	int16_t desired_az_ddeg;
	int16_t desired_el_ddeg;

	int16_t desired_az_deg;
	int16_t desired_el_deg;
};

extern struct tracking_state_t tracking_state;

void tracking_init(void);

#endif /* __TRACKING_H__ */