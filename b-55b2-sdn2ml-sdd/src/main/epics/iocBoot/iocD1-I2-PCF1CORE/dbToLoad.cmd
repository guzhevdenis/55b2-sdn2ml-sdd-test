
#======================================================================
# Loading DBs
#======================================================================
cd "$(TOP)/db"
dbLoadRecords("PCF1-D1-I2-SYSM-iocD1-I2-PCF1CORE.db")

# SIMULATOR nds-daq-generator (8 count-rate PVs) and nds-sdn publisher
dbLoadRecords("NdsDaqGenerator.db", "PREFIX=PCF1-D1-I2-DAQ,DEVICE=G-1")
dbLoadRecords("NdsSdnPublisher.db", "PREFIX=PCF1-D1-I2-SDN,DEVICE=SDN-1")

#- End-of-file marker - do not delete or add lines below!
