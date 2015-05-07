#############################################################################
## Crytek Source File
## Copyright (C) 2013, Crytek Studios
##
## Creator: Christopher Bolte
## Date: Oct 31, 2013
## Description: WAF  based build system
#############################################################################
	
from waflib.Configure import conf
import os

@conf
def init_compiler_settings(conf):
	v = conf.env
	# Create empty env values to ensure appending always works
	v['DEFINES'] = []
	v['INCLUDES'] = []
	v['CXXFLAGS'] = []
	v['LIB'] = []
	v['LIBPATH'] = []
	v['LINKFLAGS'] = []
	v['BINDIR'] = ''
	v['LIBDIR'] = ''
	v['PREFIX'] = ''
	
@conf
def load_cryengine_common_settings(conf):
	"""
	Setup all platform, compiler and configuration agnostic settings
	"""
	v = conf.env

	# Generate CODE_BASE_FOLDER define to allow to create absolute paths in source to use for pragma comment lib 
	code_node = conf.srcnode.make_node('Code')
	code_path = code_node.abspath()
	code_path = code_path.replace('\\', '/')
	v['DEFINES'] += [ 'CODE_BASE_FOLDER="' + code_path + '/"' ]
	
	# To allow pragma comment (lib, 'SDKs/...) uniformly, pass Code to the libpath
	v['LIBPATH'] += [ conf.CreateRootRelativePath('Code') ]
	
@conf	
def load_debug_cryengine_settings(conf):
	"""
	Setup all platform, compiler agnostic settings for the debug configuration
	"""
	v = conf.env
	conf.load_cryengine_common_settings()
	
	v['DEFINES'] += [ '_DEBUG' ]
		
@conf	
def load_profile_cryengine_settings(conf):
	"""
	Setup all platform, compiler agnostic settings for the profile configuration
	"""
	v = conf.env
	conf.load_cryengine_common_settings()
	
	v['DEFINES'] += [ '_PROFILE', 'PROFILE' ]
		
@conf	
def load_performance_cryengine_settings(conf):
	"""
	Setup all platform, compiler agnostic settings for the performance configuration
	"""
	v = conf.env
	conf.load_cryengine_common_settings()
	
	v['DEFINES'] += [ '_RELEASE', 'PERFORMANCE_BUILD' ]
	
@conf	
def load_release_cryengine_settings(conf):
	"""
	Setup all platform, compiler agnostic settings for the release configuration
	"""	
	v = conf.env
	conf.load_cryengine_common_settings()
	
	v['DEFINES'] += [ '_RELEASE' ]
	
#############################################################################	
#############################################################################

	
#############################################################################
@conf	
def set_editor_flags(self, kw):
		
	kw['includes'] = [
		'.', 
		self.CreateRootRelativePath('Code/Sandbox/Editor'),
		self.CreateRootRelativePath('Code/Sandbox/Editor/Include'),
		self.CreateRootRelativePath('Code/Sandbox/Plugins/EditorCommon'),
		self.CreateRootRelativePath('Code/CryEngine/CryCommon') ,
		self.CreateRootRelativePath('Code/SDKs/boost'),
		self.CreateRootRelativePath('Code/SDKs/XT_13_4/Include'),
		self.CreateRootRelativePath('Code/Sandbox/SDKs/Python27/x64/include'),
		self.CreateRootRelativePath('Code/SDKs/Microsoft Visual Studio Compiler/include'),
		self.CreateRootRelativePath('Code/SDKs/Microsoft Visual Studio Compiler/atlmfc/include'),
		self.CreateRootRelativePath('Code/SDKs/Microsoft Windows SDK/V8.0/Include'),
		] + kw['includes']
	
	if 'priority_includes' in kw:
		kw['includes'] = kw['priority_includes'] + kw['includes']
	
	kw['defines'] += [
		'WIN32',
		'CRY_ENABLE_RC_HELPER',
		'_AFXDLL',
		#'_CRT_SECURE_NO_WARNINGS=1',
		'_CRT_SECURE_NO_DEPRECATE=1',
		'_CRT_NONSTDC_NO_DEPRECATE=1',
		]
	
	kw['libpath'] += [	
		self.CreateRootRelativePath('Code/SDKs/Microsoft Visual Studio Compiler/atlmfc/lib/amd64'),
		self.CreateRootRelativePath('Code/SDKs/XT_13_4/lib_vc9'),
		self.CreateRootRelativePath('Code/Sandbox/SDKs/Python27/x64/libs'),
		]

###############################################################################
@conf	
def set_rc_flags(self, kw):

	kw['includes'] = [
		'.',
		self.CreateRootRelativePath('Code/CryEngine/CryCommon'),
		self.CreateRootRelativePath('Code/SDKs/boost'),
		self.CreateRootRelativePath('Code/Sandbox/Plugins/EditorCommon'),
		self.CreateRootRelativePath('Code/SDKs/Microsoft Visual Studio Compiler/include'),
		self.CreateRootRelativePath('Code/SDKs/Microsoft Visual Studio Compiler/atlmfc/include')
	] + kw['includes']

	kw['defines'] += [
			'WIN32',
			'RESOURCE_COMPILER',
			'FORCE_STANDARD_ASSERT',
			'NOT_USE_CRY_MEMORY_MANAGER',
			'_CRT_SECURE_NO_DEPRECATE=1',
			'_CRT_NONSTDC_NO_DEPRECATE=1'
			]
			
	if self.env['PLATFORM'] == 'win_x86':
		kw['libpath'] += [ self.CreateRootRelativePath('Code/SDKs/Microsoft Visual Studio Compiler/atlmfc/lib') ]
	else:
		kw['libpath'] += [ self.CreateRootRelativePath('Code/SDKs/Microsoft Visual Studio Compiler/atlmfc/lib/amd64') ]

###############################################################################
@conf	
def set_pipeline_flags(self, kw):

	kw['includes'] = [
		'.',
		self.CreateRootRelativePath('Code/CryEngine/CryCommon'),
		self.CreateRootRelativePath('Code/SDKs/Microsoft Visual Studio Compiler/include'),
		self.CreateRootRelativePath('Code/SDKs/Microsoft Visual Studio Compiler/atlmfc/include')
	] + kw['includes']

	kw['defines'] += [
			'WIN32',
			'RESOURCE_COMPILER',
			'FORCE_STANDARD_ASSERT',
			'NOT_USE_CRY_MEMORY_MANAGER',
			'_CRT_SECURE_NO_DEPRECATE=1',
			'_CRT_NONSTDC_NO_DEPRECATE=1'
			]
			
	if self.env['PLATFORM'] == 'win_x86':
		kw['libpath'] += [ self.CreateRootRelativePath('Code/SDKs/Microsoft Visual Studio Compiler/atlmfc/lib') ]
	else:
		kw['libpath'] += [ self.CreateRootRelativePath('Code/SDKs/Microsoft Visual Studio Compiler/atlmfc/lib/amd64') ]

###############################################################################
@conf
def CreateRootRelativePath(self, path):	
	"""
	Generate a path relative from the root
	"""
	result_path = self.srcnode.make_node(path)
	return result_path.abspath()
	
###############################################################################
@conf
def SettingsWrapper_Impl(self, *k, **kw):
	
	if not kw.get('files', None) and not kw.get('file_list', None) and not kw.get('regex', None):
		self.fatal("A Settings container must provide a list of verbatim file names, a waf_files file list or an regex")
		
	return kw
