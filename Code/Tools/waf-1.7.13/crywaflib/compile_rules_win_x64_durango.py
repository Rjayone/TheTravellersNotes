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

@conf
def check_win_x64_durango_installed(conf):
	"""
	Check compiler is actually installed on executing machine
	"""
	if not conf.is_option_true('auto_detect_compiler'):
		durango_compiler_folder = conf.CreateRootRelativePath('Code/SDKs/DurangoSDK/')
	
		if not os.path.exists(durango_compiler_folder):
			Logs.warn('[WARNING] Could not find Windows Durango SDK toolchain path: %s' % durango_compiler_folder)
			return False
		return True
	else:
			# try to read the path from the registry
			try:
				import _winreg
				sdk_folder_entry = _winreg.OpenKey(_winreg.HKEY_LOCAL_MACHINE, "Software\\Wow6432Node\\Microsoft\\Durango XDK", 0, _winreg.KEY_READ)
				(durango_sdk_dir,type) = _winreg.QueryValueEx(sdk_folder_entry, 'InstallPath')
				durango_sdk_xdk_dir = durango_sdk_dir + 'xdk'
				
				for path in [durango_sdk_dir, durango_sdk_xdk_dir]:
					if not os.path.exists(path):
						Logs.warn('[WARNING] Could not find Windows Durango SDK toolchain path: %s' % path)
						return False
					return True				
			except:
				Logs.warn('[WARNING] Could not find Windows Durango SDK toolchain path: %s' % path)
				return False
	
@conf
def load_win_x64_durango_common_settings(conf):
	"""
	Setup all compiler and linker settings shared over all win_x64_durango configurations
	"""
	pass
	
@conf
def load_debug_win_x64_durango_settings(conf):
	"""
	Setup all compiler and linker settings shared over all win_x64_durango configurations for
	the 'debug' configuration
	"""
	v = conf.env
	conf.load_win_x64_durango_common_settings()
	
	# Load addional shared settings
	conf.load_debug_cryengine_settings()
	conf.load_debug_msvc_settings()
	conf.load_debug_durango_settings()	
	
@conf
def load_profile_win_x64_durango_settings(conf):
	"""
	Setup all compiler and linker settings shared over all win_x64_durango configurations for
	the 'profile' configuration
	"""
	v = conf.env
	conf.load_win_x64_durango_common_settings()
	
	# Load addional shared settings
	conf.load_profile_cryengine_settings()
	conf.load_profile_msvc_settings()
	conf.load_profile_durango_settings()
	
@conf
def load_performance_win_x64_durango_settings(conf):
	"""
	Setup all compiler and linker settings shared over all win_x64_durango configurations for
	the 'performance' configuration
	"""
	v = conf.env
	conf.load_win_x64_durango_common_settings()
	
	# Load addional shared settings
	conf.load_performance_cryengine_settings()
	conf.load_performance_msvc_settings()
	conf.load_performance_durango_settings()
	
@conf
def load_release_win_x64_durango_settings(conf):
	"""
	Setup all compiler and linker settings shared over all win_x64_durango configurations for
	the 'release' configuration
	"""
	v = conf.env
	conf.load_win_x64_durango_common_settings()
	
	# Load addional shared settings
	conf.load_release_cryengine_settings()
	conf.load_release_msvc_settings()
	conf.load_release_durango_settings()
	