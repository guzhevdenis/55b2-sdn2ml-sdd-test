#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>

#include <log.h>                /* Logging library API */
#include <sdn.h>                /* SDN library API - Class definitions */

#include "SDN_VNC_topic.h"       /* Topic definition file */

using namespace std;

static void usage(void)
{

  printf("This program sends a string over a named topic.\n");
  printf("Usage: simple_publisher [options]\n");
  printf("  -c, --count   - Sent packets count (default: 5)\n");
  printf
    ("  -d, --delay   - Time delay between packet transmission (default: 1000000, i.e. 1 sec)\n");
  printf("  -p, --payload - Sent packet payload (default: 'Hello world!')\n");
  printf("  -t, --topic   - Topic name (default: 'undefined')\n");

  return;

}

int main(int argc, char **argv)
{

  uint8_t thermProfile[SDN_VNC_TOPIC_DATATYPE_PROJECTION_MATRIX_SIZE]{1,2,3,4,5,6,7,8,9,10,55,234,45,67,23,8,3,8,12,90,5,2};
  float ionTemp[SDN_VNC_TOPIC_DATATYPE_ION_TEMP_SIZE]{22.5,9.0,45.8,0.3,5.1};
  float radInt[SDN_VNC_TOPIC_DATATYPE_RAD_INT_SIZE]{0.4,5.6,7.5,3.8,45.9,0.0};
  /*char payload[SIMPLE_TOPIC_DATATYPE_BUFFER_SIZE]="1024, 213.3, 123.4, 123.6";*/
  float ratio = 0.3;
  float eneSpec = 1.5;
  float fPower = 6.87;
  float fIntencity = 5.3456;
  uint64_t count = 5000;
  uint64_t delay = 1000000;
  uint64_t index = 0;
  char prog_name[64] = "undefined";
  char topic_name[64] = "SDN_VNC";

  SR_RET status = SR_ERROR;

  strncpy(prog_name, argv[0], 64);

  /* Parse command line arguments */
  /*if (argc > 1)
    {
      int option = 0;

      while ((option = getopt(argc, argv, "c:d:p:t:h?")) != -1)
        {
          switch (option)
            {
            case 'c':
              count = atoi(optarg);
              break;

            case 'd':
              delay = atoi(optarg);
              break;

            case 'p':
              strncpy(payload, optarg, SIMPLE_TOPIC_DATATYPE_BUFFER_SIZE);
              break;

            case 't':
              strncpy(topic_name, optarg, 64);
              break;

            case '?':
            default:
              usage();
              return 0;
            }
        }
    }*/

  /* Initialize SDN - Creates the necessary resources and verifies machine-specific configuration */
  if ((status = initializeSDN()) != SR_OK)
    {
      log_final("%s - initializeSDN() failed", prog_name);
      return 1;
    }

  /* Create topic instance and associated metadata */
  SDN_VNC_topic_datatype topic_userdata; /* Instantiate topic user data object */
  sdn_generic_metadata topic_metadata(topic_name, "N/A", 0, sizeof(SDN_VNC_topic_datatype));     /* Instantiate meta-data object */

  topic_userdata.timestamp = 0; /* Do not care yet */
  /*memcpy(topic_userdata.buffer, payload, SIMPLE_TOPIC_DATATYPE_BUFFER_SIZE);*/    /* Copy payload buffer into topic instance */
  for (int i = 0; i < SDN_VNC_TOPIC_DATATYPE_PROJECTION_MATRIX_SIZE; ++i) 
      {
	topic_userdata.thermonuclearPowerProfile[i] = thermProfile[i];
      }
  for (int j = 0; j < SDN_VNC_TOPIC_DATATYPE_ION_TEMP_SIZE; ++j) 
      {
	topic_userdata.ionicTemperature[j] = ionTemp[j];
      }
  for (int k = 0; k < SDN_VNC_TOPIC_DATATYPE_RAD_INT_SIZE; ++k) 
      {
	topic_userdata.radiationIntencity[k] = radInt[k];
      }
  topic_userdata.fuelRatio = ratio;
  topic_userdata.energeticSpectrum = eneSpec;
  topic_userdata.fullPower = fPower;
  topic_userdata.fullIntencity = fIntencity;
  

  /* Create participant and connect to topic communication channel. */
  SdnPublisher < sdn_generic_metadata, SDN_VNC_topic_datatype > publisher;
  publisher.setPublisher(topic_metadata);

  while (index < count)
    {
      topic_userdata.instance = index;
      index += 1;
      
      /* Publish topic instance */
      if ((status = publisher.publish(topic_userdata)) != SR_OK)
        {
          log_final("%s - SdnPublisher<sdn_generic_metadata,simple_topic_datatype>::publish() 			failed", prog_name);
          return 1;
        }
        fprintf(stdout, "%s - Published topic instance '%lu', fuel ratio '%.1f' enSpec '%.1f', fPower '%.2f', fInt '%.4f'\n",
                  prog_name, topic_userdata.instance, topic_userdata.fuelRatio, topic_userdata.energeticSpectrum, topic_userdata.fullPower, topic_userdata.fullIntencity);
		for (int i = 0; i < SDN_VNC_TOPIC_DATATYPE_PROJECTION_MATRIX_SIZE; ++i) 
        {
	  		fprintf(stdout, "thermonuclearPowerProfile['%d']:'%d'\n", i, topic_userdata.thermonuclearPowerProfile[i]);
        }
		for (int j = 0; j < SDN_VNC_TOPIC_DATATYPE_ION_TEMP_SIZE; ++j) 
        {
	  		fprintf(stdout, "ionTempProfile['%d']:'%.1f'\n", j, topic_userdata.ionicTemperature[j]);
        }
		for (int k = 0; k < SDN_VNC_TOPIC_DATATYPE_RAD_INT_SIZE; ++k) 
        {
	  		fprintf(stdout, "radInt['%d']:'%.1f'\n", k, topic_userdata.radiationIntencity[k]);
        }

	
        /*log_info("%s - Published topic instance '%lu' with payload '%s' with temperature '%lu' with array '%d'\n",
                   prog_name, topic_userdata.instance, topic_userdata.buffer, topic_userdata.temperature, topic_userdata.myArray);*/

      /* Wait for some configurable time */
      usleep(delay);

    }

  /* Release SDN resources */
  publisher.unsetPublisher();
  terminateSDN();

  return 0;

}
