#############################################################################
## Crytek Source File
## Copyright (C) 2013, Crytek Studios
##
## Creator: Christopher Bolte
## Date: Oct 31, 2013
## Description: WAF  based build system
#############################################################################
from waflib.Configure import conf
from waflib import Logs
import os

supported_msvc_versions = [11]

@conf
def check_win_x64_win_x64_installed(conf):
	"""
	Check compiler is actually installed on executing machine
	"""
	if not conf.is_option_true('auto_detect_compiler'):
		# Setup Tools for MSVC Compiler for internal projects using bootstrap/3rdParty	
		msvc_compiler_folder = conf.CreateRootRelativePath('Code/SDKs/Microsoft Visual Studio Compiler')
		msvc_compiler_bin_amd64_folder = msvc_compiler_folder + '/bin/amd64'
		
		ms_sdk_folder = conf.CreateRootRelativePath('Code/SDKs/Microsoft Windows SDK')
		ms_sdk_lib_folder = ms_sdk_folder + '/V8.0/Lib/Win8/um/x64'
		
		for path in [msvc_compiler_folder, msvc_compiler_bin_amd64_folder, ms_sdk_folder,ms_sdk_lib_folder]:
			if not os.path.exists(path):
				Logs.warn('[WARNING] Could not find MSVC x64 toolchain path: %s' % path)
				return False
		return True
	
	else:
		# use auto-recognition of compiler
		
		# backup env
		env_backup = conf.env.derive()
		try:
			nMinimumVersion = int(conf.options.minimum_msvc_compiler)
		except:
			Logs.warn('[WARNING] Unknown mimumum MSVC x64  compiler version "%s", valid values are "11"' % conf.options.minimum_msvc_compiler )
			return False

		if not nMinimumVersion in supported_msvc_versions:
			Logs.warn('[WARNING] Unknown mimumum MSVC x64 compiler version "%s", valid values are "11"' % conf.options.minimum_msvc_compiler )
			return False
			
		ret_value = True
		try:
			# modifies environment ...
			conf.auto_detect_mscv_compiler('msvc ' + str(nMinimumVersion) + '.0', 'x64')
		except:
			ret_value =  False
		
		# reset environment
		conf.env = env_backup
		return ret_value
	
@conf
def load_win_x64_win_x64_common_settings(conf):
	"""
	Setup all compiler and linker settings shared over all win_x64_win_x64 configurations
	"""
	v = conf.env
	
	# Add defines to indicate a win64 build
	v['DEFINES'] += [ '_WIN32', '_WIN64' ]
	
	if not conf.is_option_true('auto_detect_compiler'):
		# Setup Tools for MSVC Compiler for internal projects using bootstrap/3rdParty	
		msvc_compiler_folder = conf.CreateRootRelativePath('Code/SDKs/Microsoft Visual Studio Compiler')
		ms_sdk_folder = conf.CreateRootRelativePath('Code/SDKs/Microsoft Windows SDK')
		
		v['AR'] = msvc_compiler_folder + '/bin/LIB.exe'
		v['CC'] = msvc_compiler_folder + '/bin/amd64/CL.exe'
		v['CXX'] = msvc_compiler_folder + '/bin/amd64/CL.exe'
		v['LINK'] = v['LINK_CC'] = v['LINK_CXX'] = msvc_compiler_folder + '/bin/amd64/LINK.exe'
			
		# Setup global library search path
		v['LIBPATH'] += [ 
			msvc_compiler_folder + '/lib/amd64',
			ms_sdk_folder + '/V8.0/Lib/Win8/um/x64'
		]		

	else:
		# use auto-recognition of compiler
		try:
			nMinimumVersion = int(conf.options.minimum_msvc_compiler)
		except:
			conf.fatal('[ERROR] Unknown mimumum compiler version "%s", valid values are "%s"' % (conf.options.minimum_msvc_compiler, ",".join(supported_msvc_versions) ) )
		if not nMinimumVersion in supported_msvc_versions:
			conf.fatal('[ERROR] Unknown mimumum compiler version "%s", valid values are "%s"' % (conf.options.minimum_msvc_compiler, ",".join(supported_msvc_versions) ) )
			
		conf.auto_detect_mscv_compiler('msvc ' + str(nMinimumVersion) + '.0', 'x64')

	# Introduce the linker to generate 64 bit code
	v['LINKFLAGS'] += [ '/MACHINE:X64' ]
	v['ARFLAGS'] += [ '/MACHINE:X64' ]
		
@conf
def load_debug_win_x64_win_x64_settings(conf):
	"""
	Setup all compiler and linker settings shared over all win_x64_win_x64 configurations for
	the 'debug' configuration
	"""
	v = conf.env
	conf.load_win_x64_win_x64_common_settings()
	
	# Load addional shared settings
	conf.load_debug_cryengine_settings()
	conf.load_debug_msvc_settings()
	conf.load_debug_windows_settings()	
	
	# Link againt GPA lib for profiling
	v['INCLUDES'] += [ conf.CreateRootRelativePath('Code/SDKs/GPA/include') ]
	
@conf
def load_profile_win_x64_win_x64_settings(conf):
	"""
	Setup all compiler and linker settings shared over all win_x64_win_x64 configurations for
	the 'profile' configuration
	"""
	v = conf.env
	conf.load_win_x64_win_x64_common_settings()
	
	# Load addional shared settings
	conf.load_profile_cryengine_settings()
	conf.load_profile_msvc_settings()
	conf.load_profile_windows_settings()
	
	# Link againt GPA lib for profiling
	v['INCLUDES'] += [ conf.CreateRootRelativePath('Code/SDKs/GPA/include') ]
	
@conf
def load_performance_win_x64_win_x64_settings(conf):
	"""
	Setup all compiler and linker settings shared over all win_x64_win_x64 configurations for
	the 'performance' configuration
	"""
	v = conf.env
	conf.load_win_x64_win_x64_common_settings()
	
	# Load addional shared settings
	conf.load_performance_cryengine_settings()
	conf.load_performance_msvc_settings()
	conf.load_performance_windows_settings()
	
	# Link againt GPA lib for profiling
	v['INCLUDES'] += [ conf.CreateRootRelativePath('Code/SDKs/GPA/include') ]
	
@conf
def load_release_win_x64_win_x64_settings(conf):
	"""
	Setup all compiler and linker settings shared over all win_x64_win_x64 configurations for
	the 'release' configuration
	"""
	v = conf.env
	conf.load_win_x64_win_x64_common_settings()
	
	# Load addional shared settings
	conf.load_release_cryengine_settings()
	conf.load_release_msvc_settings()
	conf.load_release_windows_settings()
	
