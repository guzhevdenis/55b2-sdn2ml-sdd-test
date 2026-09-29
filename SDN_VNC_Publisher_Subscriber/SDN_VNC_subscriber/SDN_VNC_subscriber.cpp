
#include <iostream>
using namespace std;
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>

#include <log.h>                /* Logging library API */
#include <sdn.h>                /* SDN library API - Class definitions */

#include "SDN_VNC_topic.h"       /* Topic definition file */

static void usage(void)
{

  printf("This program receives a string over a named topic.\n");
  printf("Usage: simple_subscriber [options]\n");
  printf("  -c, --count - Received packets count (default: 1)\n");
  printf("  -t, --topic - Topic name (default: 'undefined')\n");

  return;

}

int main(int argc, char **argv)
{
  std::cout <<"Start the program" << std::endl;
  uint64_t count = 5000;
  uint64_t index = 0;
  char prog_name[64] = "undefined";
  char topic_name[64] = "SDN_VNC";

  SR_RET status = SR_ERROR;

  strncpy(prog_name, argv[0], 64);

  /* Parse command line arguments */
  /*if (argc > 1)
    {
      int option = 0;

      while ((option = getopt(argc, argv, "c:t:h?")) != -1)
        {
          switch (option)
            {
            case 'c':
              count = atoi(optarg);
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
	std::cout <<"initialization failed"<< std::endl;
      return 1;
    }
  else 
	{
		/*std::cout <<"SDN initialization is OK" <<std::endl;*/
  	}

  /* Create topic instance and associated metadata */
  SDN_VNC_topic_datatype topic_userdata; /* Instantiate topic user data object */
  sdn_generic_metadata topic_metadata(topic_name, "N/A", 0, sizeof(SDN_VNC_topic_datatype));     /* Instantiate meta-data object */

  /* Create participant and connect to topic communication channel. */
  SdnSubscriber < sdn_generic_metadata, SDN_VNC_topic_datatype > subscriber;
  subscriber.setSubscriber(topic_metadata);

  while (index < count)
    {
      	/* Receive topic instance */
      	if (((status = subscriber.receive(topic_userdata)) != SR_OK) && (status != SR_ERR_PACKET_LOST))   /* First call returns error if the publisher is started before the subscriber */
      	{	
         /*log_final("%s - SdnSubscriber<sdn_generic_metadata,simple_topic_datatype>::receive() failed", prog_name);*/
			std::cout << "SdnSubscriber<sdn_generic_metadata,SDN_VNC_topic_datatype>::receive() failed" << std::endl;
          return 1;
      	}
      	else
      	{
		fprintf(stdout, "%s - Subscribed topic instance '%lu', fuel ratio '%.1f', enSpec '%.1f', fPower '%.2f', fInt '%.4f'\n",
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
         /* log_info("%s - Received topic instance '%lu' with payload '%s'",
                   prog_name, topic_userdata.instance, topic_userdata.buffer);*/
      	}
      index += 1;

    }

  /* Release SDN resources */
  subscriber.unsetSubscriber();
  terminateSDN();

  return 0;

}
