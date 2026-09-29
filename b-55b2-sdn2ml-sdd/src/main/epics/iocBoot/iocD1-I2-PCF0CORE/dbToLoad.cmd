
#======================================================================
# Loading DBs
#======================================================================
cd "$(TOP)/db"
dbLoadRecords("PCF0-D1-I2-SYSM-iocD1-I2-PCF0CORE.db")

# SIMULATOR nds-daq-generator (8 count-rate PVs) and nds-sdn publisher
dbLoadRecords("NdsDaqGenerator.db", "PREFIX=PCF0-D1-I2-DAQ,DEVICE=G-0")
dbLoadRecords("NdsSdnPublisher.db", "PREFIX=PCF0-D1-I2-SDN,DEVICE=SDN-0")

#- End-of-file marker - do not delete or add lines below!
