
#======================================================================
# Loading DBs
#======================================================================
cd "$(TOP)/db"
dbLoadRecords("PCF5-D1-I2-SYSM-iocD1-I2-PCF5CORE.db")

# SIMULATOR nds-daq-generator (8 count-rate PVs) and nds-sdn publisher
dbLoadRecords("NdsDaqGenerator.db", "PREFIX=PCF5-D1-I2-DAQ,DEVICE=G-5")
dbLoadRecords("NdsSdnPublisher.db", "PREFIX=PCF5-D1-I2-SDN,DEVICE=SDN-5")

#- End-of-file marker - do not delete or add lines below!
