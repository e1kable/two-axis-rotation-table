#ifndef ROT_TABLE_CONFIG_H
#define ROT_TABLE_CONFIG_H

// Axis configuration
#define N_STEPS_AZ 200
#define N_MICRO_STEPS_AZ 16
#define MAX_DPHI_DT_AZ 40.0

#define N_STEPS_EL 200
#define N_MICRO_STEPS_EL 16
#define MAX_DPHI_DT_EL 40.0

// Hall and Referencing configuration
#define REFERENCE_ANGULAR_VELOCITY 5

#ifdef ARDUINO_ARCH_RENESAS_UNO
#define N_MEAN_DEFAULT_HALL_READOUT 1000
#define N_MEAN_DEFAULT_HALL_FINE_READOUT 6000
#endif

#ifdef ARDUINO_AVR_UNO
#define N_MEAN_DEFAULT_HALL_READOUT 100
#define N_MEAN_DEFAULT_HALL_FINE_READOUT 400
#endif

#define DB_DPHI_THRES 10
#define REFERENCE_OFFSET_CORRECTION_AZ -3.8  // in degrees
#define REFERENCE_OFFSET_CORRECTION_EL -11.5 // in degrees
#define REFERENCE_MAGNET_SIZE_DEG 20.0       // in degrees
#define REFERENCE_MOVING_AVG_FILT_SIZE 5

// Slope config
#define IS_ENABLE_SLOPING 1
#define T_SLOPE 1.0                // in s; time until MAX_DPHI_DT_xx is reached
#define SLOPING_MAX_SLEEP_TIME 150 // in ms

#define SERIAL_BAUD 115200

#endif
