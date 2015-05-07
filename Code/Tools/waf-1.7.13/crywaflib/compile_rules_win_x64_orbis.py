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

@conf
def check_win_x64_orbis_installed(conf):
	"""
	Check compiler is actually installed on executing machine
	"""
	return True
	
@conf
def load_win_x64_orbis_common_settings(conf):
	"""
	Setup all compiler and linker settings shared over all win_x64_orbis configurations
	"""
	pass
	
@conf
def load_debug_win_x64_orbis_settings(conf):
	"""
	Setup all compiler and linker settings shared over all win_x64_orbis configurations for
	the 'debug' configuration
	"""
	v = conf.env
	conf.load_win_x64_orbis_common_settings()
	
	# Load addional shared settings
	conf.load_debug_cryengine_settings()
	conf.load_debug_clang_settings()
	conf.load_debug_orbis_settings()	
	
@conf
def load_profile_win_x64_orbis_settings(conf):
	"""
	Setup all compiler and linker settings shared over all win_x64_orbis configurations for
	the 'profile' configuration
	"""
	v = conf.env
	conf.load_win_x64_orbis_common_settings()
	
	# Load addional shared settings
	conf.load_profile_cryengine_settings()
	conf.load_profile_clang_settings()
	conf.load_profile_orbis_settings()
	
@conf
def load_performance_win_x64_orbis_settings(conf):
	"""
	Setup all compiler and linker settings shared over all win_x64_orbis configurations for
	the 'performance' configuration
	"""
	v = conf.env
	conf.load_win_x64_orbis_common_settings()
	
	# Load addional shared settings
	conf.load_performance_cryengine_settings()
	conf.load_performance_clang_settings()
	conf.load_performance_orbis_settings()
	
@conf
def load_release_win_x64_orbis_settings(conf):
	"""
	Setup all compiler and linker settings shared over all win_x64_orbis configurations for
	the 'release' configuration
	"""
	v = conf.env
	conf.load_win_x64_orbis_common_settings()
	
	# Load addional shared settings
	conf.load_release_cryengine_settings()
	conf.load_release_clang_settings()
	conf.load_release_orbis_settings()
	