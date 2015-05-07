#############################################################################
## Crytek Source File
## Copyright (C) 2013, Crytek Studios
##
## Creator: Christopher Bolte
## Date: Oct 31, 2013
## Description: WAF  based build system
#############################################################################
from waflib.Configure import conf

@conf
def load_gcc_common_settings(conf):
	"""
	Setup all compiler/linker flags with are shared over all targets using the gcc compiler
	
	!!! But not the actual compiler, since the compiler depends on the target !!!
	"""
	v = conf.env
	
	# Figure out GCC compiler version
	try:
		conf.get_cc_version( [ v['CC'] ], gcc=True)
	except: # Can happen if we don't have an GCC installed (code drop stripped of GCC cross compiler)
		conf.env.CC_VERSION = (0,0,0)

	# AR Tools
	v['ARFLAGS'] = 'rcs'
	v['AR_TGT_F'] = ''
	
	# CC/CXX Compiler	
	v['CC_NAME']	= v['CXX_NAME'] 	= 'gcc'	
	v['CC_SRC_F']  	= v['CXX_SRC_F']	= []
	v['CC_TGT_F'] 	= v['CXX_TGT_F']	= ['-c', '-o']
	
	v['CPPPATH_ST'] 	= '-I%s'
	v['DEFINES_ST'] 	= '-D%s'

	# Linker
	v['CCLNK_SRC_F'] = v['CXXLNK_SRC_F'] = []
	v['CCLNK_TGT_F'] = v['CXXLNK_TGT_F'] = '-o'
	
	v['LIB_ST'] 		= '-l%s'
	v['LIBPATH_ST'] 	= '-L%s'
	v['STLIB_ST'] 		= '-l%s'
	v['STLIBPATH_ST'] 	= '-L%s'
	
	# shared library settings	
	v['CFLAGS_cshlib'] = v['CFLAGS_cxxshlib']		= ['-fpic']	
	v['CXXFLAGS_cshlib'] = v['CXXFLAGS_cxxshlib']	= ['-fpic']
		
	v['LINKFLAGS_cshlib'] 	= ['-shared']
	v['LINKFLAGS_cxxshlib'] = ['-shared']
	
	# static library settings	
	v['CFLAGS_cstlib'] = v['CFLAGS_cxxstlib']	= []	 
	v['CXXFLAGS_cstlib'] = v['CXXFLAGS_cxxstlib']	= []
	
	v['LINKFLAGS_cxxstlib']	= ['-Wl,-Bstatic']
	v['LINKFLAGS_cxxshtib'] = ['-Wl,-Bstatic']
		
	# Set common compiler flags	
	COMMON_COMPILER_FLAGS = [
		'-Wall',					# Generate more warnings
		'-Werror',					# Tread Warnings as Errors
		'-ffast-math',				# Enable fast math
		
		'-fvisibility=hidden',			
		
		# Disable some warnings		
		'-Wno-char-subscripts',				
		'-Wno-unknown-pragmas',
		'-Wno-unused-variable',
		'-Wno-unused-value',
		'-Wno-parentheses',
		'-Wno-switch',
		'-Wno-unused-function',
		'-Wno-multichar',
		'-Wno-format-security',
		'-Wno-empty-body',
		'-Wno-comment',		
		'-Wno-char-subscripts',
		'-Wno-sign-compare',	
		'-Wno-narrowing',
		'-Wno-write-strings',
		'-Wno-format',								
		
		'-Wno-strict-aliasing',
		'-Wno-unused-but-set-variable',
		'-Wno-maybe-uninitialized',
		'-Wno-strict-overflow',
		'-Wno-uninitialized',				
		'-Wno-unused-local-typedefs',
		]

	if conf.env.CC_VERSION[0] >= '4' and conf.env.CC_VERSION[1] >= '8' and conf.env.CC_VERSION[2] >= '0':
		COMMON_COMPILER_FLAGS += [
			'-Wno-unused-result',
			'-Wno-sizeof-pointer-memaccess',
			'-Wno-array-bounds',
		]
		
	# Copy common flags to prevent modifing references
	v['CFLAGS'] += COMMON_COMPILER_FLAGS[:]
	
	
	v['CXXFLAGS'] += COMMON_COMPILER_FLAGS[:] + [	
		'-fno-rtti',				# Disable RTTI
		'-fno-exceptions',			# Disable Exceptions	
		'-fvisibility-inlines-hidden',
		
		# Disable some C++ specific warnings	
		'-Wno-invalid-offsetof',
		'-Wno-reorder',		
		'-Wno-conversion-null',
		'-Wno-overloaded-virtual',		
		'-Wno-c++0x-compat',
	]
	
	v['DEFINES'] += ['GCC_NO_CPP11']	
	
	# Linker Flags
	v['LINKFLAGS'] += []	
	
	v['SHLIB_MARKER'] 	= '-Wl,-Bdynamic'
	v['SONAME_ST'] 		= '-Wl,-h,%s'
	v['STLIB_MARKER'] 	= '-Wl,-Bstatic'
	
	# Compile options appended if compiler optimization is disabled
	v['COMPILER_FLAGS_DisableOptimization'] = [ '-O0' ]	
	
	# Compile options appended if debug symbols are generated	
	v['COMPILER_FLAGS_DebugSymbols'] = [ '-g' ]	
	
	# Linker flags when building with debug symbols
	v['LINKFLAGS_DebugSymbols'] = []
	
	# Store settings for show includes option
	v['SHOWINCLUDES_cflags'] = ['-H']
	v['SHOWINCLUDES_cxxflags'] = ['-H']
	
	# Store settings for preprocess to file option
	v['PREPROCESS_cflags'] = ['-E', '-dD']
	v['PREPROCESS_cxxflags'] = ['-E', '-dD']
	v['PREPROCESS_cc_tgt_f'] = ['-o']
	v['PREPROCESS_cxx_tgt_f'] = ['-o']
	
	# Store settings for preprocess to file option
	v['DISASSEMBLY_cflags'] = ['-S', '-fverbose-asm']
	v['DISASSEMBLY_cxxflags'] = ['-S', '-fverbose-asm']
	v['DISASSEMBLY_cc_tgt_f'] = ['-o']
	v['DISASSEMBLY_cxx_tgt_f'] = ['-o']

@conf
def load_debug_gcc_settings(conf):
	"""
	Setup all compiler/linker flags with are shared over all targets using the gcc compiler
	for the "debug" configuration	
	"""	
	v = conf.env	
	
	conf.load_gcc_common_settings()
	
	COMPILER_FLAGS = [
		'-O0',		# No optimization
		]
	
	v['CFLAGS'] += COMPILER_FLAGS
	v['CXXFLAGS'] += COMPILER_FLAGS
	
@conf
def load_profile_gcc_settings(conf):
	"""
	Setup all compiler/linker flags with are shared over all targets using the gcc compiler
	for the "profile" configuration	
	"""
	v = conf.env
	conf.load_gcc_common_settings()
	
	COMPILER_FLAGS = [
		'-O2',
		]
	
	v['CFLAGS'] += COMPILER_FLAGS
	v['CXXFLAGS'] += COMPILER_FLAGS	
	
@conf
def load_performance_gcc_settings(conf):
	"""
	Setup all compiler/linker flags with are shared over all targets using the gcc compiler
	for the "performance" configuration	
	"""
	v = conf.env
	conf.load_gcc_common_settings()
	
	COMPILER_FLAGS = [
		'-O2',
		]
	
	v['CFLAGS'] += COMPILER_FLAGS
	v['CXXFLAGS'] += COMPILER_FLAGS
	
@conf
def load_release_gcc_settings(conf):
	"""
	Setup all compiler/linker flags with are shared over all targets using the gcc compiler
	for the "release" configuration	
	"""
	v = conf.env
	conf.load_gcc_common_settings()
	
	COMPILER_FLAGS = [
		'-O2',
		]
	
	v['CFLAGS'] += COMPILER_FLAGS
	v['CXXFLAGS'] += COMPILER_FLAGS	
	