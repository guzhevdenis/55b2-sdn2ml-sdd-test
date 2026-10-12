
#======================================================================
# Loading DBs
#======================================================================
cd "$(TOP)/db"
dbLoadRecords("PCF4-D1-I2-SYSM-iocD1-I2-PCF4CORE.db")

# SIMULATOR nds-daq-generator (8 count-rate PVs) and nds-sdn publisher
dbLoadRecords("NdsDaqGenerator.db", "PREFIX=PCF4-D1-I2-DAQ,DEVICE=G-4")
dbLoadRecords("NdsSdnPublisher.db", "PREFIX=PCF4-D1-I2-SDN,DEVICE=SDN-4")

#- End-of-file marker - do not delete or add lines below!
