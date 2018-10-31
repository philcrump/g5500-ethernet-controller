#include "../main.h"

/* AZ - PA6 - ADC123_IN6 - using ADC1 */
/* EL - PA3 - ADC12_IN3 - using ADC1 */

/* AZ CW  - PB10 */
/* AZ CCW - PB4 */
/* EL UP  - PA8 */
/* EL DN  - PB5 */

#define TRACKING_AZ_CW(state)   if(state) { palSetPad(GPIOB, 10); } else { palClearPad(GPIOB, 10); }
#define TRACKING_AZ_CCW(state)  if(state) { palSetPad(GPIOB,  4); } else { palClearPad(GPIOB,  4); }

#ifdef ELEVATION_ENABLED
  #define TRACKING_EL_UP(state)   if(state) { palSetPad(GPIOA, 8); } else { palClearPad(GPIOA, 8); }
  #define TRACKING_EL_DOWN(state) if(state) { palSetPad(GPIOB, 5); } else { palClearPad(GPIOB, 5); }
#else
  #define TRACKING_EL_UP(state)   
  #define TRACKING_EL_DOWN(state) 
#endif

#define ADC_SAMPLES     16
static adcsample_t adc_samples[ADC_SAMPLES];

struct tracking_state_t tracking_state;

#define TRACKING_CALIBRATION_ENTRIES   5
/*
// G3KMI Old AZ/EL
static int16_t tracking_az_calibration[TRACKING_CALIBRATION_ENTRIES][2] = 
{  
   {0,88},
   {900,867},
   {1800,1666},
   {2700,2446},
   {3600,3235}
};
static int16_t tracking_el_calibration[TRACKING_CALIBRATION_ENTRIES][2] = 
{  
   {0,15},
   {900,1663},
   {1800,3302},
   {1800,3302},
   {1800,3302}
};
*/

static int16_t tracking_az_calibration[TRACKING_CALIBRATION_ENTRIES][2] = 
{  
   {0,355},
   {900,972},
   {1800,1590},
   {2700,2206},
   {3600,2813}
};
static int16_t tracking_el_calibration[TRACKING_CALIBRATION_ENTRIES][2] = 
{  
   {0,35},
   {900,1576},
   {1800,3156},
   {1800,3156},
   {1800,3156}
};

int16_t tracking_ddeg2raw(int16_t ddegrees, int16_t calibration_table[][2])
{
  int16_t lower_ddeg, upper_ddeg, lower_raw, upper_raw;
  int i;

  if(ddegrees <= calibration_table[0][0])
  {
    return calibration_table[0][1];
  }

  for(i=0;i<TRACKING_CALIBRATION_ENTRIES;i++)
  {
    if(calibration_table[i][0] < ddegrees)
    {
      lower_ddeg = calibration_table[i][0];
      lower_raw = calibration_table[i][1];
    }
    else
    {
      upper_ddeg = calibration_table[i][0];
      upper_raw = calibration_table[i][1];
      break;
    }
  }

  return lower_raw + (((upper_raw - lower_raw) * (ddegrees - lower_ddeg)) / (upper_ddeg - lower_ddeg));
}

int16_t tracking_raw2ddeg(int16_t raw, int16_t calibration_table[][2])
{
  int16_t lower_ddeg, upper_ddeg, lower_raw, upper_raw;
  int i;

  if(raw <= calibration_table[0][1])
  {
    return calibration_table[0][0];
  }

  for(i=0;i<TRACKING_CALIBRATION_ENTRIES;i++)
  {
    if(calibration_table[i][1] < raw)
    {
      lower_ddeg = calibration_table[i][0];
      lower_raw = calibration_table[i][1];
    }
    else
    {
      upper_ddeg = calibration_table[i][0];
      upper_raw = calibration_table[i][1];
      break;
    }
  }

  return lower_ddeg + (((upper_ddeg - lower_ddeg) * (raw - lower_raw)) / (upper_raw - lower_raw));
}

static void adc_adcerrorcallback(ADCDriver *adcp, adcerror_t err)
{
  (void)adcp;
  (void)err;
}

static const ADCConversionGroup adc_az_adcgrpcfg = {
  FALSE,
  1,
  NULL,
  adc_adcerrorcallback,
  0,                        /* CR1 */
  ADC_CR2_SWSTART,          /* CR2 */
  ADC_SMPR1_SMP_AN11(ADC_SAMPLE_3),
  0,                        /* SMPR2 */
  ADC_SQR1_NUM_CH(1),
  0,                        /* SQR2 */
  ADC_SQR3_SQ1_N(ADC_CHANNEL_IN6) /* IN6 */
};

static const ADCConversionGroup adc_el_adcgrpcfg = {
  FALSE,
  1,
  NULL,
  adc_adcerrorcallback,
  0,                        /* CR1 */
  ADC_CR2_SWSTART,          /* CR2 */
  ADC_SMPR1_SMP_AN11(ADC_SAMPLE_3),
  0,                        /* SMPR2 */
  ADC_SQR1_NUM_CH(1),
  0,                        /* SQR2 */
  ADC_SQR3_SQ1_N(ADC_CHANNEL_IN2) /* IN3 */
};

static uint16_t tracking_az_sample(void)
{
    uint32_t i;
    uint32_t result;
    
    adcStart(&ADCD1, NULL);
    
    adcConvert(&ADCD1, &adc_az_adcgrpcfg, adc_samples, ADC_SAMPLES);
    
    adcStop(&ADCD1);
    
    result = 0;
    for(i=0;i<ADC_SAMPLES;i++)
    {
        result += adc_samples[i];
    }
    result /= ADC_SAMPLES;
    
    return (uint16_t) (result & 0xFFFF);
}

static uint16_t tracking_el_sample(void)
{
    uint32_t i;
    uint32_t result;
    
    adcStart(&ADCD1, NULL);
    
    adcConvert(&ADCD1, &adc_el_adcgrpcfg, adc_samples, ADC_SAMPLES);
    
    adcStop(&ADCD1);
    
    result = 0;
    for(i=0;i<ADC_SAMPLES;i++)
    {
        result += adc_samples[i];
    }
    result /= ADC_SAMPLES;
    
    return (uint16_t) (result & 0xFFFF);
}

THD_FUNCTION(tracking, arg)
{
  (void) arg;

  chRegSetThreadName("tracking");

  while(1)
  {
    tracking_state.az_raw = tracking_az_sample();
    tracking_state.el_raw = tracking_el_sample();

    tracking_state.az_ddeg = tracking_raw2ddeg(tracking_state.az_raw, tracking_az_calibration); // TODO: AZ Calibration
    tracking_state.el_ddeg = tracking_raw2ddeg(tracking_state.el_raw, tracking_el_calibration);

    tracking_state.az_deg = (tracking_state.az_ddeg + 5) / 10;
    tracking_state.el_deg = (tracking_state.el_ddeg + 5) / 10;

    /* Azimuth */
    if(!tracking_state.cw && !tracking_state.ccw)
    {
      /* Stopped, so allow hysteresis */
      if(tracking_state.desired_az_ddeg > tracking_state.az_ddeg + TRACKING_AZ_HYSTERESIS)
      {
        tracking_state.cw = 1;
        tracking_state.ccw = 0;
      }
      else if(tracking_state.desired_az_ddeg < tracking_state.az_ddeg - TRACKING_AZ_HYSTERESIS)
      {
        tracking_state.cw = 0;
        tracking_state.ccw = 1;
      }
      else
      {
        tracking_state.cw = 0;
        tracking_state.ccw = 0;
      }
    }
    else
    {
      /* Currently in motion, so allow overshoot */
      if(tracking_state.cw && (tracking_state.desired_az_ddeg <= tracking_state.az_ddeg + TRACKING_AZ_OVERSHOOT))
      {
        tracking_state.cw = 0;
        tracking_state.ccw = 0;
      }
      else if(tracking_state.ccw && (tracking_state.desired_az_ddeg >= tracking_state.az_ddeg - TRACKING_AZ_OVERSHOOT))
      {
        tracking_state.cw = 0;
        tracking_state.ccw = 0;
      }
    }

    /* Elevation */
    if(!tracking_state.up && !tracking_state.down)
    {
      /* Stopped, so allow hysteresis */
      if(tracking_state.desired_el_ddeg > tracking_state.el_ddeg + TRACKING_EL_HYSTERESIS)
      {
        tracking_state.up = 1;
        tracking_state.down = 0;
      }
      else if(tracking_state.desired_el_ddeg < tracking_state.el_ddeg - TRACKING_EL_HYSTERESIS)
      {
        tracking_state.up = 0;
        tracking_state.down = 1;
      }
      else
      {
        tracking_state.up = 0;
        tracking_state.down = 0;
      }
    }
    else
    {
      /* Currently in motion, so allow overshoot */
      if(tracking_state.up && (tracking_state.desired_el_ddeg <= tracking_state.el_ddeg - TRACKING_EL_OVERSHOOT))
      {
        tracking_state.up = 0;
        tracking_state.down = 0;
      }
      else if(tracking_state.down && (tracking_state.desired_el_ddeg >= tracking_state.el_ddeg + TRACKING_EL_OVERSHOOT))
      {
        tracking_state.up = 0;
        tracking_state.down = 0;
      }
    }

    TRACKING_AZ_CW(tracking_state.cw);
    TRACKING_AZ_CCW(tracking_state.ccw);

    #ifdef ELEVATION_ENABLED
      TRACKING_EL_UP(tracking_state.up);
      TRACKING_EL_DOWN(tracking_state.down);
    #else
      TRACKING_EL_UP(0);
      TRACKING_EL_DOWN(0);
    #endif

    chThdSleepMilliseconds(50);
  }
}

THD_WORKING_AREA(wa_tracking, TRACKING_STACK_SIZE);

void tracking_init(void)
{

  tracking_state.az_raw = tracking_az_sample();
  tracking_state.el_raw = tracking_el_sample();

  tracking_state.az_ddeg = tracking_raw2ddeg(tracking_state.az_raw, tracking_az_calibration); // TODO: AZ Calibration
  tracking_state.el_ddeg = tracking_raw2ddeg(tracking_state.el_raw, tracking_el_calibration);

  tracking_state.az_deg = (tracking_state.az_ddeg + 5) / 10;
  tracking_state.el_deg = (tracking_state.el_ddeg + 5) / 10;

  tracking_state.desired_az_raw = tracking_state.az_raw;
  tracking_state.desired_el_raw = tracking_state.el_raw;

  tracking_state.desired_az_ddeg = tracking_state.az_ddeg;
  tracking_state.desired_el_ddeg = tracking_state.el_ddeg;

  tracking_state.cw = 0;
  tracking_state.ccw = 0;
  TRACKING_AZ_CW(0);
  TRACKING_AZ_CCW(0);

  tracking_state.up = 0;
  tracking_state.down = 0;
  TRACKING_EL_UP(0);
  TRACKING_EL_DOWN(0);

  chThdCreateStatic(wa_tracking, sizeof(wa_tracking), NORMALPRIO + 1, tracking, NULL);
}
