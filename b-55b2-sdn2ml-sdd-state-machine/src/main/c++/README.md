# NDS test drivers for 55b2-sdn2ml-sdd

SIMULATOR stack for PCF0CORE..PCF5CORE. This is **not** production data acquisition.

## Drivers

| Directory | Role | Notes |
|-----------|------|--------|
| `nds-daq-generator` | Simulated count-rate source. 8 PVs `CountRate00`..`CountRate07`, thread update every 100 ms. 
| `nds-sdn` | Subscribes to NDS PVs from nds-daq-generator 


## HMI

Do **not** put project OPI files under `src/main/boy/mimics/templates`. 
Eclipse `.project` links that name to `/opt/codac/opi/boy/resources/templates`, so CSS looks for `/55b2-sdn2ml-sdd/src/main/boy/mimics/templates/<file>` and never sees the git file.

- [PCFCORE_SimStand_Block.opi](../boy/mimics/PCFCORE_SimStand_Block.opi) — one PCFCORE panel (`$(DAQ)`, `$(SDN)`, `$(IOC_NAME)`)
- [ITER-D1-I2-SimStand.opi](../boy/mimics/ITER-D1-I2-SimStand.opi) — 2x3 linking containers
- [ITER-D1-I2_Mimic.opi](../boy/mimics/ITER-D1-I2_Mimic.opi) — same six containers (CBS I2 / navigation)

## SDN configuration

Per-IOC NDS-DAQ NDS-SDN mapping: `src/main/resources/pcfN-nds-sdn-conf.xml`  
Per-IOC topic definition: `src/main/resources/pcfN-nds-sdn-topic.xml`

## Network configuration

- in userPreDriverConf.cmd file ndsCreateDevice parameter `INTERFACE=lo` indicates the network interface to be used for SDN. It needs to be configured in production in this file,  
with `codac-configure sdn_interface` and  `export SDN_INTERFACE_NAME`. 

- Topic XML must be installed where CODAC SDN discovery can find it (default location /etc/opt/codac/sdn)

- To archive several topics at once, need to add them in `/etc/opt/codac/sdn_archiver/sdn-archiver_config.xml`. To start archiving:

```bash
sdn-archiver-service start
```

## Authors

- Denis Guzhev