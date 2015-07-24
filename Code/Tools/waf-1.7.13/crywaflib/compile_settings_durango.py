#############################################################################
## Crytek Source File
## Copyright (C) 2013, Crytek Studios
##
## Creator: Christopher Bolte
## Date: Oct 31, 2013
## Description: WAF  based build system
#############################################################################
from waflib.Configure import conf

import re, os
from waflib.TaskGen import extension,feature, before_method, after_method
from waflib import Task, Logs, Utils
import xml.etree.ElementTree as ET

@conf
def load_durango_common_settings(conf):
	"""
	Setup all compiler and linker settings shared over all durango configurations
	"""
	v = conf.env	
		
	# Since Durango is only build in a single configuration on windows, we can specify the compile tools here
	if not conf.is_option_true('auto_detect_compiler'):
		durango_sdk_dir = conf.CreateRootRelativePath('Code/SDKs/DurangoSDK/')		
		durango_sdk_xdk_dir = durango_sdk_dir
	else:
			# try to read the path from the registry
			try:
				import _winreg
				sdk_folder_entry = _winreg.OpenKey(_winreg.HKEY_LOCAL_MACHINE, "Software\\Wow6432Node\\Microsoft\\Durango XDK", 0, _winreg.KEY_READ)
				(durango_sdk_dir,type) = _winreg.QueryValueEx(sdk_folder_entry, 'InstallPath')
				durango_sdk_dir = durango_sdk_dir.encode('ascii') # Make asci string (as we get unicode)
				durango_sdk_xdk_dir = durango_sdk_dir + 'xdk'
			except:
				conf.fatal('[ERROR] Cannot find Durango XDK Registry Entry. Please verify your Durango XDK installation')
			
	# Same for both
	durango_sdk_xdk_wrl_dir = conf.CreateRootRelativePath('Code/SDKs/DurangoSDK_WRL/')
	
	v['DURANGO_XDK_DIR'] = durango_sdk_xdk_dir
	# Setup common defines for durango
	v['DEFINES'] += [ '_WIN32', '_WIN64', '_DURANGO', 'DURANGO', '_XBOX_ONE' ]
	
	# Durango requieres some addional defines 
	v['DEFINES'] += [
		'_UITHREADCTXT_SUPPORT=0',
		'_CRT_USE_WINAPI_PARTITION_APP',
		'__WRL_NO_DEFAULT_LIB__',
		'WIN32_LEAN_AND_MEAN',
		'GSI_WINSOCK2',		
		'WINAPI_FAMILY=WINAPI_FAMILY_TV_TITLE',		
		]
		
	# For durango, always create code favoring AVX 
	v['ARCH'] = ['AVX']
	

	# Set up compiler flags
	COMMON_COMPILER_FLAGS = [															
		'/favor:AMD64',	# Create Code optimized for 64 bit				
		# Add Metadata Directories with /AI
		'/AI' + durango_sdk_xdk_dir + '/crt/platform/amd64',
		'/AI' + durango_sdk_xdk_dir + '/References/CommonConfiguration/Neutral'
		]
	
	# Copy common flags to prevent modifing references
	v['CFLAGS'] += COMMON_COMPILER_FLAGS[:]
	v['CXXFLAGS'] += COMMON_COMPILER_FLAGS[:]
	
	# Linker Flags
	v['LINKFLAGS'] += [
		'/MACHINE:X64',						# Create 64 Bit Files				
		
		# Disable a whole bunch of default libraries on durango
		'/NODEFAULTLIB:advapi32.lib',
		'/NODEFAULTLIB:atl.lib',
		'/NODEFAULTLIB:atls.lib',
		'/NODEFAULTLIB:atlsd.lib',
		'/NODEFAULTLIB:atlsn.lib',
		'/NODEFAULTLIB:atlsnd.lib',
		'/NODEFAULTLIB:comctl32.lib',
		'/NODEFAULTLIB:comsupp.lib',
		'/NODEFAULTLIB:dbghelp.lib',
		'/NODEFAULTLIB:gdi32.lib',
		'/NODEFAULTLIB:gdiplus.lib',
		'/NODEFAULTLIB:guardcfw.lib',
		'/NODEFAULTLIB:kernel32.lib',
		'/NODEFAULTLIB:mmc.lib',
		'/NODEFAULTLIB:msimg32.lib',
		'/NODEFAULTLIB:msvcole.lib',
		'/NODEFAULTLIB:msvcoled.lib',
		'/NODEFAULTLIB:mswsock.lib',
		'/NODEFAULTLIB:ntstrsafe.lib',
		'/NODEFAULTLIB:ole2.lib',
		'/NODEFAULTLIB:ole2autd.lib',
		'/NODEFAULTLIB:ole2auto.lib',
		'/NODEFAULTLIB:ole2d.lib',
		'/NODEFAULTLIB:ole2ui.lib',
		'/NODEFAULTLIB:ole2uid.lib',
		'/NODEFAULTLIB:ole32.lib',
		'/NODEFAULTLIB:oleacc.lib',
		'/NODEFAULTLIB:oleaut32.lib',
		'/NODEFAULTLIB:oledlg.lib',
		'/NODEFAULTLIB:oledlgd.lib',
		'/NODEFAULTLIB:oldnames.lib',
		'/NODEFAULTLIB:runtimeobject.lib',
		'/NODEFAULTLIB:shell32.lib',
		'/NODEFAULTLIB:shlwapi.lib',
		'/NODEFAULTLIB:strsafe.lib',
		'/NODEFAULTLIB:urlmon.lib',
		'/NODEFAULTLIB:user32.lib',
		'/NODEFAULTLIB:userenv.lib',
		'/NODEFAULTLIB:uuid.lib',
		'/NODEFAULTLIB:wlmole.lib',
		'/NODEFAULTLIB:wlmoled.lib',
		]
		
	
	# Set up Library search path
	v['LIBPATH'] += [		
		durango_sdk_xdk_dir + '/lib/amd64',
		durango_sdk_xdk_dir + '/crt/lib/amd64',
		durango_sdk_xdk_dir + '/PC/lib/amd64'
		]
		
	# Setup default libraries to link
	v['LIB'] += [
		'combase', 
		'kernelx'
		]
			
	# Default Include path
	v['INCLUDES'] += [
		durango_sdk_xdk_wrl_dir + '/INCLUDE/winrt',  # WRL headers need to be included before originals
		durango_sdk_xdk_dir + '/INCLUDE/um',
		durango_sdk_xdk_dir + '/INCLUDE/shared',
		durango_sdk_xdk_dir + '/INCLUDE/winrt',		
		durango_sdk_xdk_dir + '/crt/INCLUDE',
		durango_sdk_xdk_dir + '/VC/ATLMFC/INCLUDE',
		]		
		
	# Store setting for static code analyzer	
	v['STATIC_CODE_ANALYZE_cflags'] = ['/analyze', '/WX']
	v['STATIC_CODE_ANALYZE_cxxflags'] = ['/analyze', '/WX']
	
	v['AR'] 			= durango_sdk_xdk_dir + '/vc/bin/amd64/LIB.exe'		
	v['CC']				= v['CXX'] = durango_sdk_xdk_dir + '/vc/bin/amd64/CL.exe'
	v['LINK'] 		= v['LINK_CC'] = v['LINK_CXX'] = durango_sdk_xdk_dir + '/vc/bin/amd64/LINK.exe'
	v['MAKEPGK'] 	= durango_sdk_dir + '/bin/makepkg.exe'
		
@conf
def load_debug_durango_settings(conf):
	"""
	Setup all compiler and linker settings shared over all durango configurations for
	the 'debug' configuration
	"""
	v = conf.env
	conf.load_durango_common_settings()
	
@conf
def load_profile_durango_settings(conf):
	"""
	Setup all compiler and linker settings shared over all durango configurations for
	the 'debug' configuration
	"""
	v = conf.env
	conf.load_durango_common_settings()
	
@conf
def load_performance_durango_settings(conf):
	"""
	Setup all compiler and linker settings shared over all durango configurations for
	the 'debug' configuration
	"""
	v = conf.env
	conf.load_durango_common_settings()
	
@conf
def load_release_durango_settings(conf):
	"""
	Setup all compiler and linker settings shared over all durango configurations for
	the 'debug' configuration
	"""
	v = conf.env
	conf.load_durango_common_settings()
	
###############################################################################		
###############################################################################
COMPILE_TEMPLATE = '''def f(ctx):
	lst = []
	def xml_escape(value):
		return value.replace("&", "&amp;").replace('"', "&quot;").replace("'", "&apos;").replace("<", "&lt;").replace(">", "&gt;")

	%s

	#f = open('cmd.txt', 'w')
	#f.write(str(lst))
	#f.close()
	return ''.join(lst)
'''

reg_act = re.compile(r"(?P<backslash>\\)|(?P<dollar>\$\$)|(?P<subst>\$\{(?P<code>[^}]*?)\})", re.M)
def compile_template(line):
	"""
	Compile a template expression into a python function (like jsps, but way shorter)
	"""
	extr = []
	def repl(match):
		g = match.group
		if g('dollar'): return "$"
		elif g('backslash'):
			return "\\"
		elif g('subst'):
			extr.append(g('code'))
			return "<<|@|>>"
		return None
	
	line2 = reg_act.sub(repl, line)
	params = line2.split('<<|@|>>')
	assert(extr)	

	indent = 0
	buf = []
	app = buf.append

	def app(txt):		
		buf.append(indent * '\t' + txt)

	for x in range(len(extr)):
		if params[x]:
			app("lst.append(%r)" % params[x])

		f = extr[x]
		if f.startswith('if') or f.startswith('for'):
			app(f + ':')
			indent += 1
		elif f.startswith('py:'):
			app(f[3:])
		elif f.startswith('endif') or f.startswith('endfor'):
			indent -= 1
		elif f.startswith('else') or f.startswith('elif'):
			indent -= 1
			app(f + ':')
			indent += 1
		elif f.startswith('xml:'):
			app('lst.append(xml_escape(%s))' % f[4:])
		else:
			#app('lst.append((%s) or "cannot find %s")' % (f, f))
			app('lst.append(%s)' % f)

	if extr:
		if params[-1]:
			app("lst.append(%r)" % params[-1])
		
	fun = COMPILE_TEMPLATE % "\n\t".join(buf)
	#print(fun)
	return Task.funex(fun)
	

@feature('durango_title_id')
@before_method('apply_incpaths')
def add_create_durango_title_id(self):
	"""
	Function to a special durango_title_id.h header
	"""		
	
	if self.bld.env['PLATFORM'] != 'durango':
		return
	
	output_folder_node 	= self.path.get_bld()	
	output_node 				= output_folder_node.make_node('durango_title_id.h')
	
	# Add bintemp folder to include path
	self.includes += [ output_folder_node.abspath() ]
	
	# Create task to generate the new file
	tsk = self.create_task('create_durango_title_id', self.bld._projects_node(), output_node )
	
	for t in getattr(self, 'compiled_tasks', []):
		t.run_after.add(tsk)
		
###############################################################################
class create_durango_title_id(Task.Task):
	"""
	A task to convert the template appxmanifest into the final version
	"""
	color =  'YELLOW'
	
	def run(self):
		
		# useful shortcuts		
		bld			= self.generator.bld
		project = self.generator.game_project
		
		# Create content of durango_title_id file
		durango_title_id_content	 =	'/* Auto generated file, to change those values, please adjust _WAF_/projects.json */\n'
		durango_title_id_content	+=	'#ifndef _DURANGO_TITLE_ID_H_\n'
		durango_title_id_content	+=	'#define _DURANGO_TITLE_ID_H_\n'
		durango_title_id_content	+=	'\n'
		durango_title_id_content	+=	'#define DURANGO_LIVE_TITLE_ID            (0x'+bld.project_durango_titleid(project)+')\n'
		durango_title_id_content	+=	'#define DURANGO_LIVE_SERVICE_CONFIG_ID  L"'+bld.project_durango_scid(project)+'"\n'
		durango_title_id_content	+=	'\n'
		durango_title_id_content	+=	'#endif //_DURANGO_TITLE_ID_H_\n'
		
		self.outputs[0].write( durango_title_id_content )
	
	def runnable_status(self):
		if super(create_durango_title_id, self).runnable_status() == -1:
			return -1
		src = self.inputs[0].abspath()
		tgt = self.outputs[0].abspath()

		# If there any target file is missing, we have to copy
		try:
			stat_tgt = os.stat(tgt)
		except OSError:	
			return -3 # RUN_ME
		
		# Now compare both file stats
		try:
			stat_src = os.stat(src)				
		except OSError:
			pass
		else:
			# same size and identical timestamps -> make no copy
			if stat_src.st_mtime >= stat_tgt.st_mtime + 2:			
				return -3 # RUN_ME
				
		# Everything fine, we can skip this task		
		return -2 # SKIP_ME
			
			
###############################################################################	
@feature('cprogram', 'cxxprogram')
@after_method('apply_flags_msvc')
@after_method('apply_link')
def add_create_app_data(self):
	"""
	Function to generate the copy tasks for build outputs	
	"""
	if not getattr(self, 'create_appdata', False):
		return
			
	if self.env['PLATFORM'] == 'project_generator':
		return
		
	if not self.env['PLATFORM'] == 'durango':
		self.bld.fatal('AppXManifest is only supported for Durango Targets')
	if not getattr(self, 'link_task', None):
		self.bld.fatal('AppXManifest is only supported for Executable Targets')		
	
	# Useful shortcuts
	bld 					= self.bld
	project_name	= self.project_name
	
	# Get appxmanifest template node from projects.json
	appxmanifest_template_node = self.resource_path.make_node( bld.project_durango_appxmanifest( project_name ) )
	
	
	# Create folder and output node for appxmanifest
	appxmanifest_node = appxmanifest_template_node.get_bld()
	appxmanifest_node.parent.mkdir()
			
	# Create output node for appdata.bin	
	appdata_node = appxmanifest_node.parent.make_node('appdata.bin')
		
	# Create task to convert the template appxmanifest into the final one
	self.create_task('create_appxmanifest', [appxmanifest_template_node, bld._projects_node()] , appxmanifest_node )
	self.copy_files( appxmanifest_node )
	
	# Create task to create appdata.bin
	appdata_tsk = self.create_task('create_app_data', appxmanifest_node, appdata_node )
	appdata_tsk.env.append_value( 'OUTPUT_FOLDER', appdata_node.parent.abspath() )		
	self.copy_files( appdata_node )
	
	# Create tasks to copy resouces like logso from resource folder into bintemp
	for resource in 'logo small_logo splash_screen store_logo'.split():
		r 	= getattr(bld, 'project_durango_' + resource)(project_name)
		tsk = self.copy_files( self.resource_path.make_node(r) )

	# Figure out which DLLs we must copy
	root = ET.fromstring( appxmanifest_template_node.read() )

	def _get_xml_subnodes(xml_nodes, entry):
		n = []
		for x in xml_nodes:
			for e in x:
				if entry in e.tag:
					n += [e]
		return n
		
	nodes = _get_xml_subnodes( [ root ], 'Extensions')
	nodes = _get_xml_subnodes(nodes, 'Extension')
	nodes = _get_xml_subnodes(nodes, 'InProcessServer')
	nodes = _get_xml_subnodes(nodes, 'Path')

	dlls_to_copy  = []
	for node in nodes:
		dlls_to_copy += [node.text]
	
	# Find DLLs we might need to copy in durango SDK
	durango_sdk_extensions 			= bld.env['DURANGO_XDK_DIR'] + '/Extensions'
	durango_sdk_extensions_node = bld.root.make_node(durango_sdk_extensions)
	extension_files     				= durango_sdk_extensions_node.ant_glob('**/*.dll')

	# Now copy the found DLLs to the output folder
	for file in extension_files:
		for dll in dlls_to_copy:
			if dll in file.abspath():
				self.copy_files( file )



###############################################################################
class create_appxmanifest(Task.Task):
	"""
	A task to convert the template appxmanifest into the final version
	"""
	color =  'YELLOW'
	
	def run(self):	
		# useful shortcuts
		tg		= self.generator
		bld		= self.generator.bld
		
		appxmanifest_template_node 	= self.inputs[0] # Assume inputs[0] is the appxmanifest template
		appxmanifest_final_node			= self.outputs[0]

		# Set up configuration values from projects.json
		project_name					= tg.project_name
		self.executable_name 	= tg.output_file_name + '.exe'
		self.package_name 		= bld.project_durango_package_name(project_name)
		self.version 					= bld.project_durango_version(project_name)
		self.display_name 		= bld.project_durango_display_name(project_name)
		self.publisher_name 	= bld.project_durango_publisher_name(project_name)
		self.description 			= bld.project_durango_description(project_name)
		self.foreground_text 	= bld.project_durango_foreground_text(project_name)
		self.background_color = bld.project_durango_background_color(project_name)
		self.app_id 					= bld.project_durango_app_id(project_name)		
		self.titleid 					= bld.project_durango_titleid(project_name)
		self.scid 						= bld.project_durango_scid(project_name)
		
		self.logo 						= os.path.basename( bld.project_durango_logo(project_name) )
		self.small_logo 			= os.path.basename( bld.project_durango_small_logo(project_name) )
		self.splash_screen 		= os.path.basename( bld.project_durango_splash_screen(project_name) )
		self.store_logo 			= os.path.basename( bld.project_durango_store_logo(project_name) )
		
		# Parse the template and fill in values
		appxmanifest_template 		= appxmanifest_template_node.read()
		template 									= compile_template(appxmanifest_template)	
		appxmanifest_file_content = template(self)	
		
		# Write out final version of appxmanifest.xml
		appxmanifest_final_node.write( appxmanifest_file_content )

		
###############################################################################			
class create_app_data(Task.Task):
	"""
	A Task to execute makepkg to transform a Appxmanifest.xml into an appdata.bin
	"""
	color =  'YELLOW'
	run_str = '${MAKEPGK} appdata /f ${SRC} /pd ${OUTPUT_FOLDER}'