############################################################################
## Autosave monitor post setup
############################################################################

cd "${TOP}/iocBoot/$(IOC)"
create_monitor_set("iocD1-I2-PCF1SYSM.req",30,"P=$(AUTOSAVE_SYSM_PV_PREFIX)")

#- End-of-file marker - do not delete or add lines below!
