#ifndef ROT_TABLE_CONFIG_H
#define ROT_TABLE_CONFIG_H

#define N_STEPS 200
#define N_MICRO_STEPS 8
#define MAX_DPHI_DT 20

#define N_MEAN_DEFAULT_HALL_READOUT 200
#define N_MEAN_DEFAULT_HALL_FINE_READOUT 5000
#define HALL_ZERO_V_VALUE 512

#define DB_DPHI_THRES 10                    // 10 is good for 1/4 step
#define REFERENCE_OFFSET_CORRECTION_AZ -5.0 // in degrees
#define REFERENCE_OFFSET_CORRECTION_EL -9.75 // in degrees

#define SERIAL_BAUD 115200

#endif
