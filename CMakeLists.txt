#
# Copyright (c) 2021 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

project(pn5190_nfcreader)
zephyr_include_directories(
   src/inc_example
   src/NxpNfcRdLib/comps/phacDiscLoop/src/Sw
   src/NxpNfcRdLib/intfs
   src/NxpNfcRdLib/types
   src/Platform
   src/RTOS
)
ADD_DEFINITIONS(-DPHDRIVER_nRF52840_PNEV5190B_BOARD)
ADD_DEFINITIONS(-DPH_OSAL_NULLOS)
ADD_DEFINITIONS(-DDEBUG)


FILE(GLOB drv_sources src/*.c
src/NxpNfcRdLib/comps/phacDiscLoop/src/*.c
src/Platform/*.c
src/RTOS/*.c
src/NxpNfcRdLib/comps/phNfcLib/src/*.c
src/NxpNfcRdLib/comps/phKeyStore/src/Sw/*.c
src/NxpNfcRdLib/comps/phhalHw/src/Pn5190/*.c
src/NxpNfcRdLib/comps/phpalI14443p3a/src/Sw/*.c
src/NxpNfcRdLib/comps/phalFelica/src/Sw/*.c
src/NxpNfcRdLib/comps/phacDiscLoop/src/Sw/*.c
src/NxpNfcRdLib/comps/phCryptoSym/src/Sw/*.c
src/NxpNfcRdLib/comps/phpalI14443p3b/src/Sw/*.c
src/NxpNfcRdLib/comps/phalMfc/src/Sw/*.c
src/NxpNfcRdLib/comps//phKeyStore/src/*.c
src/NxpNfcRdLib/comps//phTMIUtils/src/*.c
src/NxpNfcRdLib/comps/phpalI14443p4a/src/Sw/*.c
src/NxpNfcRdLib/comps/phalMful/src/Sw/*.c 
src/NxpNfcRdLib/comps/phCryptoRng/src/Sw/*.c 
src/NxpNfcRdLib/comps/phpalI14443p4/src/Sw/*.c 
src/NxpNfcRdLib/comps/phalMfdf/src/Sw/*.c 
src/NxpNfcRdLib/comps/phCryptoRng/src/*.c 
src/NxpNfcRdLib/comps/phpalMifare/src/Sw/*.c
src/NxpNfcRdLib/comps/phalVca/src/Sw/*.c
src/NxpNfcRdLib/comps/phCryptoSym/src/*.c
src/NxpNfcRdLib/comps/phalMfdfEVx/src/Sw/*.c
src/NxpNfcRdLib/comps/phpalFelica/src/Sw/*.c
src/NxpNfcRdLib/comps/phalVca/src/*.c
src/NxpNfcRdLib/comps/phpalSli15693/src/Sw/*.c
src/NxpNfcRdLib/comps/phalMfdfLight/src/Sw/*.c
src/NxpNfcRdLib/comps/phpalI18000p3m3/src/Sw/*.c
src/NxpNfcRdLib/comps/phalMfpEVx/src/Sw/*.c
src/NxpNfcRdLib/comps/phpalI18092mPI/src/Sw/*.c
src/NxpNfcRdLib/comps/phpalI14443p4mC/src/Sw/*.c
src/NxpNfcRdLib/comps/phpalI18092mT/src/Sw/*.c
src/NxpNfcRdLib/comps/phalMfNtag42XDna/src/Sw/*.c
src/NxpNfcRdLib/comps/phalT1T/src/Sw/*.c
src/NxpNfcRdLib/comps/phalICode/src/Sw/*.c
src/NxpNfcRdLib/comps/phalTop/src/Sw/*.c
src/NxpNfcRdLib/comps/phalI18000p3m3/src/Sw/*.c
src/NxpNfcRdLib/comps/phTools/src/*.c
)




#set(drv_sources ${drv_sources1} ${drv_sources2} ${drv_sources3} ${drv_sources4})
target_sources(app PRIVATE
        ${drv_sources})
