######################################################
# Build configurations

include $(WORK_DIR)/lib_config.mk

######################################################
# Module library

MODULE_LIBRARY = itools.a

MODULE_SRCS = $(wildcard source/*.c)

MODULE_OBJS = $(MODULE_SRCS:.c=.o)

######################################################
# Header files

PUBLIC_HEADER_FILES = $(wildcard include/*.h)

######################################################
# Module flags

MODULE_CFLAGS =  \
	-I$(TOPDIR)/Drivers/Galois_SoC/Common/include \
	-I$(TOPDIR)/PE/Common/include \
	-I$(TOPDIR)/PE/Server/include \
	-I$(TOPDIR)/PE/Agent/include \
	-I$(TOPDIR)/PE/Vdec/include \
	-I$(TOPDIR)/VDM/include \
	-I$(TOPDIR)/Audio/AudioCommon/include \
	-I$(TOPDIR)/Common/include\
	-I$(TOPDIR)/Video/Display/include\
	-I$(SYS_BUILD_DIR)/include/CUnit \
	-I$(TOPDIR)/Audio/include \
	-I$(TOPDIR)/GraphDec/BDREGraphics/ATSC/include \
	-I$(TOPDIR)/Video/VDM/include \
	-I$(TOPDIR)/DMX/Comps/DmxComm/include \
	-I$(TOPDIR)/GraphDec/BDGraphics/include \
	-I$(TOPDIR)/GraphDec/BDREGraphics/DVB/include \
	-I$(TOPDIR)/GFXDS/GFXCommon/include \
	-I$(TOPDIR)/Video/VCommon/include \
	-I$(TOPDIR)/DRM/FIRSTBOOT_SERVER/include \
	-I$(TOPDIR)/GraphDec/DVDSPU/include \
	-I$(TOPDIR)/Video/VDM/include \
	-I$(TOPDIR)/GraphDec/BDGraphics/include \
	-I$(TOPDIR)/GFXDS/GfxManager/include \
	$(Galois_SoC_COMMON_INC) \
	-I$(TOPDIR)/Drivers/Galois_SoC/VOP/include \
	-I$(TOPDIR)/GraphDec/Common/include \
	-I$(TOPDIR)/GFXDS/GfxImage/include \
	-I$(TOPDIR)/Drivers/Galois_SoC/VPP/include \
	-I$(TOPDIR)/GraphDec/BDREGraphics/ISDB/include \
	-I$(TOPDIR)/DMX/Comps/Tsp/TspMgr/include \
	-I$(TOPDIR)/GraphDec/SubDec/include \
	-I$(TOPDIR)/Drivers/Galois_SoC/APB/include \
	-I$(TOPDIR)/GFXDS/CPU1Agent/include \
	-I$(TOPDIR)/Test/test_muteprint/include \
	-I$(TOPDIR)/DRM/IMAGETOOL_SERVER/include \
	-I$(TOPDIR)/Drivers/Galois_SoC/VIP/include \
	-I$(TOPDIR)/Drivers/Galois_SoC/AIP/include \
	-I$(TOPDIR)/DRM/Cryptolib/include \
	-I$(TOPDIR)/DRM/DRMCommon/include \
	-I$(TOPDIR)/DRM/IMAGETOOL_COMMON/include \
	-I../../DMX/Comps/Tsp/TspDrv/include \
         -I../../Drivers/Galois_SoC/Common/include

######################################################
# Unittest setting

UNITTEST_SRCS =

UNITTEST_OBJS =

######################################################
# Release setting

# released source files
RELEASE_SRCS =

# released header files
RELEASE_HEADER_FILES =

# released library
RELEASE_LIBRARY = MODULE_LIBRARY

# source files to build released library
RELEASE_LIB_SRCS = MODULE_SRCS

# object files to build released library
RELEASE_LIB_OBJS = MODULE_OBJS

######################################################
# Common dependency process

include $(TOPDIR)/build/Scripts/mk_default.in