## 55b2-sdm2ml-sdd project 

## Description

This is a the test sdd-project to provide functional verification of current software architecture of 55b2 system. 
This architecture is designed to calculate plasma parameters and send it via SDN network. 

## Software modules

It consists of seven PCFCORE IOCs used for plasma parameter calculation.

PCF0CORE through PCF5CORE are used for data aggregation from simulated hardware.
In this case, they emulate the count rates in the nds-daq-generator nds-driver and send them via SDN to an SDN subscriber application.
That application uses the SDN API to receive the data, 
applies ML models to calculate plasma parameters from the count rates, 
and sends the calculated values to the SDN Archive.

## Structure 

/b-55b2-sdn2ml-sdd - the full SDD-Project.
/SDN_VNC_Publisher_Subscriber - the example of SDN-publisher-subscriber applications.

## Authors

- Denis Guzhev