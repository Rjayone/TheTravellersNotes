#############################################################################
## Crytek Source File
## Copyright (C) 2013, Crytek Studios
##
## Creator: Christopher Bolte
## Date: Oct 31, 2013
## Description: WAF  based build system
#############################################################################
from waflib.Configure import conf

def load_linux_x64_common_settings(v):
	"""
	Setup all compiler and linker settings shared over all linux_x64 configurations
	"""
	
	# Add common linux x64 defines
	v['DEFINES'] += [ 'LINUX64' ]	
	
@conf
def load_debug_linux_x64_settings(conf):
	"""
	Setup all compiler and linker settings shared over all linux_x64 configurations for
	the 'debug' configuration
	"""
	v = conf.env
	load_linux_x64_common_settings(v)
	
@conf
def load_profile_linux_x64_settings(conf):
	"""
	Setup all compiler and linker settings shared over all linux_x64 configurations for
	the 'profile' configuration
	"""
	v = conf.env
	load_linux_x64_common_settings(v)
	
@conf
def load_performance_linux_x64_settings(conf):
	"""
	Setup all compiler and linker settings shared over all linux_x64 configurations for
	the 'performance' configuration
	"""
	v = conf.env
	load_linux_x64_common_settings(v)
	
@conf
def load_release_linux_x64_settings(conf):
	"""
	Setup all compiler and linker settings shared over all linux_x64 configurations for
	the 'release' configuration
	"""
	v = conf.env
	load_linux_x64_common_settings(v)
	