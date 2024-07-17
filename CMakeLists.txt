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
FILE(GLOB drv_sources1 src/*.c)
FILE(GLOB drv_sources2 src/NxpNfcRdLib/comps/phacDiscLoop/src/*.c)
#FILE(GLOB drv_sources5 src/NxpNfcRdLib/comps/phacDiscLoop/src/Sw/*.c)
FILE(GLOB drv_sources3 src/Platform/*.c)
FILE(GLOB drv_sources4 src/RTOS/*.c)
set(drv_sources ${drv_sources1} ${drv_sources2} ${drv_sources3} ${drv_sources4})
target_sources(app PRIVATE
        ${drv_sources1} ${drv_sources2} ${drv_sources3} ${drv_sources4} ${drv_sources5})
