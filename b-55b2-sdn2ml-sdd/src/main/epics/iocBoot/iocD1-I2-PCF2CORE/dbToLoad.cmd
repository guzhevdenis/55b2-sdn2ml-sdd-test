
#======================================================================
# Loading DBs
#======================================================================
cd "$(TOP)/db"
dbLoadRecords("PCF2-D1-I2-SYSM-iocD1-I2-PCF2CORE.db")

# SIMULATOR nds-daq-generator (8 count-rate PVs) and nds-sdn publisher
dbLoadRecords("NdsDaqGenerator.db", "PREFIX=PCF2-D1-I2-DAQ,DEVICE=G-2")
dbLoadRecords("NdsSdnPublisher.db", "PREFIX=PCF2-D1-I2-SDN,DEVICE=SDN-2")

#- End-of-file marker - do not delete or add lines below!
