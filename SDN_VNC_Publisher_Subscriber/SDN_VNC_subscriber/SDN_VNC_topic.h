#ifndef SDN_VNC_TOPIC_H
#define SDN_VNC_TOPIC_H

#include <sdn.h>                /* SDN library API - Class definitions */

#define SDN_VNC_TOPIC_DATATYPE_BUFFER_SIZE   128
#define SDN_VNC_TOPIC_DATATYPE_RESERVED_SIZE 112
#define SDN_VNC_TOPIC_DATATYPE_PROJECTION_MATRIX_SIZE 2500
#define SDN_VNC_TOPIC_DATATYPE_ION_TEMP_SIZE 5
#define SDN_VNC_TOPIC_DATATYPE_RAD_INT_SIZE 6

class PACKED SDN_VNC_topic_datatype:public sdn_generic_userdata
{

public:
  uint64_t instance;            /* Topic instance number */
  uint64_t timestamp;           /* Absolute time [nsec] */
  uint8_t thermonuclearPowerProfile[SDN_VNC_TOPIC_DATATYPE_PROJECTION_MATRIX_SIZE];
  float fuelRatio;
  float energeticSpectrum;
  float fullPower;
  float fullIntencity;
  float radiationIntencity[SDN_VNC_TOPIC_DATATYPE_RAD_INT_SIZE];
  float ionicTemperature[SDN_VNC_TOPIC_DATATYPE_ION_TEMP_SIZE]; 
  

private:
  uint8_t reserved[SDN_VNC_TOPIC_DATATYPE_RESERVED_SIZE];      /* Reserved for future use */

};

#endif /* SDN_VNC_TOPIC_H */
