############################################################################
## User provided PLC or fast controller driver pre configuration
############################################################################

# Enable parallel callback threads to improve 'I/O Intr' record scanning
#    see https://bugzilla.iter.org/codac/show_bug.cgi?id=10413
callbackParallelThreads

# SIMULATOR nds-daq-generator: 8 count-rate channels, no hardware
ndsLoadDriver $(PWD)/../c++/nds-daq-generator/target/lib/libnds3-NdsDaqGenerator.so
ndsCreateDevice(NdsDaqGenerator, G-0)

# nds-sdn publisher: maps G-0-CountRate00..07 into SDN topic pcf0-nds-sdn-topic
# INTERFACE=lo follows nds3-sdn EPICS example. Production 55b2-vnc-sdd PCF5CORE used enp37s0f0.
# TODO: set INTERFACE / SDN_INTERFACE_NAME to the test-stand SDN NIC if loopback is not accepted.
ndsLoadDriver $(PWD)/../c++/nds-sdn/lib/libnds3-sdn-publisher.so
ndsCreateDevice(DeviceNDSSDNPUB, "SDN-0","INTERFACE=ens192","TOPICNAME=pcf0-nds-sdn-topic","PVS_CONFIG_PATH=$(PWD)/../resources/nds-sdn-config/pcf0-nds-sdn-conf.xml")

nds setLogLevelDebug SDN-0

#- End-of-file marker - do not delete or add lines below!
