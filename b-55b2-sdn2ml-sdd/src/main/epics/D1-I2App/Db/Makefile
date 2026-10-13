TOP=../..
include $(TOP)/configure/CONFIG
#----------------------------------------
#  ADD MACRO DEFINITIONS AFTER THIS LINE

#----------------------------------------------------
#  Optimization of db files using dbst (DEFAULT: NO)
#DB_OPT = YES

#----------------------------------------------------
# Create and install (or just install) into <top>/db
# databases, templates, substitutions like this
DB += PCF0-D1-I2-SYSM-iocD1-I2-PCF0CORE.db
DB += PCF0-D1-I2-SYSM-iocD1-I2-PCF0SYSM.db
DB += PCF1-D1-I2-SYSM-iocD1-I2-PCF1CORE.db
DB += PCF1-D1-I2-SYSM-iocD1-I2-PCF1SYSM.db
DB += PCF2-D1-I2-SYSM-iocD1-I2-PCF2CORE.db
DB += PCF2-D1-I2-SYSM-iocD1-I2-PCF2SYSM.db
DB += PCF3-D1-I2-SYSM-iocD1-I2-PCF3CORE.db
DB += PCF3-D1-I2-SYSM-iocD1-I2-PCF3SYSM.db
DB += PCF4-D1-I2-SYSM-iocD1-I2-PCF4CORE.db
DB += PCF4-D1-I2-SYSM-iocD1-I2-PCF4SYSM.db
DB += PCF5-D1-I2-SYSM-iocD1-I2-PCF5CORE.db
DB += PCF5-D1-I2-SYSM-iocD1-I2-PCF5SYSM.db
DB += PCF6-D1-I2-SYSM-iocD1-I2-PCF6CORE.db
DB += PCF6-D1-I2-SYSM-iocD1-I2-PCF6SYSM.db
DB += PSH0-D1-I2-SYSM-iocD1-I2-PSH0CORE.db
DB += PSH0-D1-I2-SYSM-iocD1-I2-PSH0SYSM.db

# SIMULATOR nds-daq-generator and nds-sdn (PCF0CORE..PCF5CORE)
DB += NdsDaqGenerator.db
DB += NdsSdnPublisher.db
DB += MasterStateMachines.db

# Automagically install .proto files
DB += $(notdir $(wildcard ../*.proto))

#----------------------------------------------------
# If <anyname>.db template is not named <anyname>*.template add
# <anyname>_template = <templatename>

include $(TOP)/configure/RULES
#----------------------------------------
#  ADD RULES AFTER THIS LINE
