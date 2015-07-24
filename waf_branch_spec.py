#############################################################################
## Crytek Source File
## Copyright (C) 2013, Crytek Studios
##
## Creator: Christopher Bolte
## Date: Oct 31, 2013
## Description: Central configuration file for a branch, should never be
## 				integrated since it is unique for each branch
#############################################################################
	
######################
## Recode Settings
RECODE_LICENSE_KEY = ''

######################
## Build Layout
BINTEMP_FOLDER = 'BinTemp'

######################
## Build Configuration
COMPANY_NAME = 'Crytek GmbH'
COPYRIGHT = '(C) 2015 Crytek GmbH'
######################
## Supported branch platforms/configurations
## This is a map of host -> target platforms
PLATFORMS = {
	'darwin' : 	[ 'darwin_x64' ],
    'win32' :     [ 'win_x86', 'win_x64', 'android_arm_gcc' ],
	'linux' : 	[ 'linux_x86_gcc', 'linux_x64_gcc', 'linux_x86_clang', 'linux_x64_clang' ]
	}
	
## And a list of build configurations to generate for each supported platform	
CONFIGURATIONS = [ 'debug',	 'profile', 'performance', 'release' ]
