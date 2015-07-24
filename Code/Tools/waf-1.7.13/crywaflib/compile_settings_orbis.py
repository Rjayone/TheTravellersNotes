#############################################################################
## Crytek Source File
## Copyright (C) 2013, Crytek Studios
##
## Creator: Christopher Bolte
## Date: Oct 31, 2013
## Description: WAF  based build system
#############################################################################
from waflib.Configure import conf
from waflib.TaskGen import extension,feature, before_method, after_method

import os

@conf
def load_orbis_common_settings(conf):
	"""
	Setup all compiler and linker settings shared over all orbis configurations
	"""
	v = conf.env
	
	# Setup common defines for orbis
	v['DEFINES'] += [ 'ORBIS', '_ORBIS', '__ORBIS__' ]
	
	# Since Orbis is only build on windows right now it is fine to set the output format here
	v['CFLAGS'] += [ '-fdiagnostics-format=msvc' ]
	v['CXXFLAGS'] += [ '-fdiagnostics-format=msvc', '-Wno-deprecated-register' ]
	
	# Pattern to transform outputs
	v['cprogram_PATTERN'] 	= '%s.elf'
	v['cxxprogram_PATTERN'] = '%s.elf'
	v['cshlib_PATTERN'] 	= '%s.prx'
	v['cxxshlib_PATTERN'] 	= '%s.prx'
	v['cstlib_PATTERN']      = 'lib%s.a'
	v['cxxstlib_PATTERN']    = 'lib%s.a'

	# Since Orbis is only build in a single configuration on windows, we can specify the compile tools here
	if not conf.is_option_true('auto_detect_compiler'):
		orbis_sdk_folder = conf.CreateRootRelativePath('Code/SDKs/Orbis')
		
		# Adjust SCE_ORBIS_SDK_DIR for correct compiler execution when using the bootstraped one
		v.env = os.environ.copy()
		v.env['SCE_ORBIS_SDK_DIR'] 	= orbis_sdk_folder
	else:
		# Orbis stored the values in the env
		if not 'SCE_ORBIS_SDK_DIR' in os.environ:
			conf.fatal('[ERROR] Could not find environment variable "SCE_ORBIS_SDK_DIR". Please verify your Orbis SDK installation')
		orbis_sdk_folder =  os.environ['SCE_ORBIS_SDK_DIR']
		
	orbis_target_folder 		= orbis_sdk_folder + '/target'
	orbis_toolchain_folder	= orbis_sdk_folder + '/host_tools/bin'	
		
	v['INCLUDES'] += [ orbis_target_folder + '/include' ]
	v['INCLUDES'] += [ orbis_target_folder + '/include_common' ]

	v['AR'] = orbis_toolchain_folder + '/orbis-ar.exe'	
	v['CC'] = orbis_toolchain_folder + '/orbis-clang.exe'
	v['CXX'] = orbis_toolchain_folder + '/orbis-clang++.exe'
	v['LINK'] = v['LINK_CC'] = v['LINK_CXX'] = orbis_toolchain_folder + '/orbis-ld.exe'

@conf
def load_debug_orbis_settings(conf):
	"""
	Setup all compiler and linker settings shared over all orbis configurations for
	the 'debug' configuration
	"""
	v = conf.env
	conf.load_orbis_common_settings()
	
@conf
def load_profile_orbis_settings(conf):
	"""
	Setup all compiler and linker settings shared over all orbis configurations for
	the 'debug' configuration
	"""
	v = conf.env
	conf.load_orbis_common_settings()
	
@conf
def load_performance_orbis_settings(conf):
	"""
	Setup all compiler and linker settings shared over all orbis configurations for
	the 'debug' configuration
	"""
	v = conf.env
	conf.load_orbis_common_settings()
	
@conf
def load_release_orbis_settings(conf):
	"""
	Setup all compiler and linker settings shared over all orbis configurations for
	the 'debug' configuration
	"""
	v = conf.env
	conf.load_orbis_common_settings()
	
###############################################################################
###############################################################################	
@feature('copy_orbis_sys_sce_folder')
@before_method('apply_incpaths')
def add_copy_orbis_sys_sce_folder_task(self):
	"""
	Function to copy special orbis files to sys_sce folder
	"""		
	if self.env['PLATFORM'] == 'project_generator':
		return
		
	bld 			= self.bld
	platform	= bld.env['PLATFORM']
	
	for project in bld.spec_game_projects():
	
		output_folder 			= bld.get_output_folders(platform)[0]
		sce_sys_node 				= output_folder.make_node('sce_sys')
		sce_sys_trophy_node = output_folder.make_node('sce_sys/trophy')
		
		nptitle_dat 				= bld.project_orbis_nptitle_dat( project )
		params_sfo					= bld.project_orbis_param_sfo( project )
		trophy_trp					= bld.project_orbis_trophy_trp( project )
				
		self.create_task('copy_outputs', self.resource_path.make_node(nptitle_dat), sce_sys_node.make_node('nptitle.dat') ) 
		self.create_task('copy_outputs', self.resource_path.make_node(params_sfo), 	sce_sys_node.make_node('param.sfo') )
		self.create_task('copy_outputs', self.resource_path.make_node(trophy_trp), 	sce_sys_trophy_node.make_node('trophy00.trp') )

		sce_sys_node.mkdir()
		sce_sys_trophy_node.mkdir()