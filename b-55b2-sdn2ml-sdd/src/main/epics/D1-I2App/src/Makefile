TOP=../..

include $(TOP)/configure/CONFIG
#----------------------------------------
#  ADD MACRO DEFINITIONS AFTER THIS LINE
#========================================

# Use typed rset structure (see 3.16.1 release notes)
USR_CPPFLAGS += -DUSE_TYPED_RSET

#========================================
# Build the IOC application

PROD_IOC = D1-I2
# D1-I2.dbd will be created and installed
DBD += D1-I2.dbd

# D1-I2.dbd will be made up from these files:
-include $(EPICS_BASE)/mk/codac-common.mk


# Add all the support libraries needed by this IOC
-include $(EPICS_BASE)/mk/picmg.mk
-include $(EPICS_BASE)/mk/asyn.mk


D1-I2_DBD += $(CODAC_DBD)
D1-I2_LIBS += $(CODAC_LIBS)

# # SNCSEQ
# <sncProgram>_SNCFLAGS += +r -c +d
# D1-I2_DBD += <sncProgram>.dbd
# D1-I2_SRCS += <sncProgram>.stt
# D1-I2_LIBS += seq pv


# D1-I2_registerRecordDeviceDriver.cpp derives from D1-I2.dbd
D1-I2_SRCS += D1-I2_registerRecordDeviceDriver.cpp

# Build the main IOC entry point on workstation OSs.
D1-I2_SRCS_DEFAULT += D1-I2Main.cpp
D1-I2_SRCS_vxWorks += -nil-

# Add support from base/src/vxWorks if needed
#D1-I2_OBJS_vxWorks += $(EPICS_BASE_BIN)/vxComLibrary

#=============================
# Include SNL program makefile snippets

-include ../*.snlprog

#=============================
# Include user.mk makefile snippets

-include ../user.mk

# Link QSRV (pvAccess Server) if available
ifdef EPICS_QSRV_MAJOR_VERSION
    D1-I2_LIBS += qsrv
    D1-I2_LIBS += $(EPICS_BASE_PVA_CORE_LIBS)
    D1-I2_DBD += PVAServerRegister.dbd
    D1-I2_DBD += qsrv.dbd
endif

#=============================
# Finally link to the EPICS Base libraries
D1-I2_LIBS += $(EPICS_BASE_IOC_LIBS)

#===========================

include $(TOP)/configure/RULES
#----------------------------------------
#  ADD RULES AFTER THIS LINE
#----------------------------------------
#adding the support for asynDriver
-include $(EPICS_BASE)/mk/asyn.mk
#adding the support for nds3-epics
-include $(EPICS_BASE)/mk/nds3-epics.mk
