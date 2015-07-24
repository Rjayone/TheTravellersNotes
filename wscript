#############################################################################
## Crytek Source File
## Copyright (C) 2013, Crytek Studios
##
## Creator: Christopher Bolte
## Date: Oct 31, 2013
## Description: WAF  based build system
#############################################################################

from waflib import Configure, Logs, Utils, Node, TaskGen, Options, ConfigSet
from waflib.Build import BuildContext, CleanContext, Context
from waflib.Tools import c_aliases, c
from waflib.Task import Task,RUN_ME
from waflib.Configure import conf, ConfigurationContext
from waflib.TaskGen import after_method, before_method, feature, extension
from waflib.Errors import BuildError, WafError
from waflib.TaskGen import taskgen_method

from subprocess import call

import os
import shutil
import ConfigParser  
import traceback
try:
	import _winreg
except:
	pass

# Load globals from branch spec file if we must
from waf_branch_spec import PLATFORMS  
from waf_branch_spec import CONFIGURATIONS  

g_bootstrap_was_run = False

###############################################################################
Configure.autoconfig = True

###############################################################################
CRY_WAF_TOOL_DIR='Code/Tools/waf-1.7.13/crywaflib'

	
###############################################################################	
# List of subfolders to parse	
SUBFOLDERS =	[
	'Code',
	]
		
###############################################################################
## Configurate Options for WAF	
def options(opt):
	opt.load('cry_utils' ,tooldir=CRY_WAF_TOOL_DIR)	
	
	opt.load('project_settings', tooldir=CRY_WAF_TOOL_DIR)
	opt.load('branch_spec', tooldir=CRY_WAF_TOOL_DIR)	
	opt.load('gui_tasks' , tooldir=CRY_WAF_TOOL_DIR)
	
	###########################################
	# Load support for Uber Files
	opt.load('generate_uber_files' ,tooldir=CRY_WAF_TOOL_DIR)	
		
	###########################################
	# Load Project Generators based on host (use the custom cry versions)
	host = Utils.unversioned_sys_platform()
	
	opt.load('msvs', tooldir=CRY_WAF_TOOL_DIR)
	opt.load('msvs_override_handling', tooldir=CRY_WAF_TOOL_DIR) 
	opt.load('mscv_helper' ,tooldir=CRY_WAF_TOOL_DIR)
	
	if host == 'darwin':
		opt.load('xcode' ,tooldir=CRY_WAF_TOOL_DIR)
	if host == 'linux':
		opt.load('eclipse' ,tooldir=CRY_WAF_TOOL_DIR)
		
	# Load tools to improve dependency checking (by using compiler features)
	if host == 'win32':
		opt.load('mscv_helper', tooldir=CRY_WAF_TOOL_DIR)
	opt.load('msvcdeps', tooldir=CRY_WAF_TOOL_DIR)	
	opt.load('gccdeps', tooldir=CRY_WAF_TOOL_DIR)		
		
	#	Load QT last as we need to hook the runnable status at the lowest level
	opt.load('qt' ,tooldir=CRY_WAF_TOOL_DIR)	
	
	###########################################
	# Add custom cryengine options	
			
	opt.add_option('-p', '--project-spec', dest='project_spec', action='store', default='', help='Spec to use when building the project')
	
	# Add special command line option to prevent recursive execution of WAF
	opt.add_option('--internal-dont-check-recursive-execution', dest='internal_dont_check_recursive_execution', action='store', default='False', help='!INTERNAL ONLY! DONT USE')

	# Add options primarily used by the Visual Studio WAF Addin
	waf_addin_group = opt.add_option_group('Visual Studio WAF Addin Options')
	waf_addin_group.add_option('-a', 	'--ask-for-user-input',			dest='ask_for_user_input', 			action='store', default='True', 	help='Disables all user promps in WAF')
	waf_addin_group.add_option(				'--file-filter', 						dest='file_filter', 						action='store', default="", 			help='Only compile files matching this filter')
	waf_addin_group.add_option(				'--show-includes', 					dest='show_includes', 					action='store', default='False', 	help='Show all files included (requieres a file_filter)')
	waf_addin_group.add_option(				'--show-preprocessed-file', dest='show_preprocessed_file', 	action='store', default='False', 	help='Generate only Preprocessor Output (requieres a file_filter)')
	waf_addin_group.add_option(				'--show-disassembly', 			dest='show_disassembly', 				action='store', default='False', 	help='Generate only Assembler Output (requieres a file_filter)')	
	
	# DEPRECATED OPTIONS, only used to keep backwards compatibility
	waf_addin_group.add_option(				'--output-file', 						dest='output_file', 						action='store', default="", 			help='*DEPRECATED* Specify Output File for Disassemly or Preprocess option (requieres a file_filter)')
	waf_addin_group.add_option(				'--use-overwrite-file', 		dest='use_overwrite_file', 			action='store', default="False", 	help='*DEPRECATED* Use special BinTemp/cry_waf.configuration_overwrites to specify per target configurations')
	waf_addin_group.add_option(				'--console-mode', 					dest='console_mode', 						action='store', default="False", 	help='No Gui. Display console only')	
	
	# Lastly, load data driven settings
	opt.load('default_settings', tooldir=CRY_WAF_TOOL_DIR)
	opt.load('cryengine_modules', tooldir=CRY_WAF_TOOL_DIR)	
	
###############################################################################
## Configure 'configure' step
def configure(conf):	
	###########################################
	# Load base configuration function		
	conf.load('c_config')	
	
	###########################################
	# Load common windows settings if needed
	conf.load('winres', tooldir=CRY_WAF_TOOL_DIR)
	
	# Load common platform scripts	
	conf.load('compile_settings_cryengine', tooldir=CRY_WAF_TOOL_DIR)	
	
	conf.load('compile_settings_msvc', tooldir=CRY_WAF_TOOL_DIR)
	conf.load('compile_settings_gcc', tooldir=CRY_WAF_TOOL_DIR)
	conf.load('compile_settings_clang', tooldir=CRY_WAF_TOOL_DIR)
	
	conf.load('compile_settings_windows', tooldir=CRY_WAF_TOOL_DIR)
	conf.load('compile_settings_linux', tooldir=CRY_WAF_TOOL_DIR)
	conf.load('compile_settings_linux_x86', tooldir=CRY_WAF_TOOL_DIR)
	conf.load('compile_settings_linux_x64', tooldir=CRY_WAF_TOOL_DIR)
	conf.load('compile_settings_darwin', tooldir=CRY_WAF_TOOL_DIR)
	conf.load('compile_settings_durango', tooldir=CRY_WAF_TOOL_DIR)
	conf.load('compile_settings_orbis', tooldir=CRY_WAF_TOOL_DIR)
	
	# Load CppCheck since it is a 'global' platform
	conf.load('cppcheck', tooldir=CRY_WAF_TOOL_DIR)
	
	if Utils.unversioned_sys_platform() == 'linux':
		host = 'linux_x64'
	elif Utils.unversioned_sys_platform() == 'win32':
		host = 'win_x64'
	elif Utils.unversioned_sys_platform() == 'darwin':
		host = 'darwin_x64'		
	
	###########################################
	## Load settings for all platforms
	
	platforms_done = ''
	installed_platforms = []
	for platform in conf.get_supported_platforms():

		Logs.info('[INFO] Configure "%s - [%s]"' % (platform, ', '.join(conf.get_supported_configurations())))
		
		# Load Platform Configuration (except cppcheck as it is not a *real* compiler)
		if platform != 'cppcheck':
			conf.load('compile_rules_' + host + '_' + platform, tooldir=CRY_WAF_TOOL_DIR)
			
		# Use the specialized function to load platform specifics
			function_name = 'check_%s_%s_installed' % ( host, platform )
			if not hasattr(conf, function_name):
				conf.fatal('[ERROR] Required function \'%s\' not found' % function_name )				

			# Try to load the function			
			if not getattr(conf, function_name)():
				Logs.warn('[WARNING] - Unable to locate platform "%s" on system. Disabling platform support for: %s !!!' % (platform, platform))
				continue
		
		# platform installed
		installed_platforms.append(platform)
		
		for configuration in conf.get_supported_configurations():

			conf.setenv(platform + '_' + configuration)				
			conf.init_compiler_settings()
			
			# Use the specialized function to load platform specifics
			function_name = 'load_%s_%s_%s_settings' % ( configuration, host, platform )
			if not hasattr(conf, function_name):
				conf.fatal('[ERROR] Required function \'%s\' not found' % function_name )				

			# Try to load the function			
			getattr(conf, function_name)()
		
	conf.set_supported_platforms(installed_platforms)
		
	###########################################
	# Load support for c and cxx compiler
	conf.load('c')
	conf.load('cxx')
	
	###########################################
	# Load Platform specific helpers based on host
	host = Utils.unversioned_sys_platform()	
	if host == 'darwin':
		conf.load('c_osx')
	
	###########################################
	# Create VS-Projects automatically during configure when running on windows
	if host == 'win32' and conf.is_option_true('generate_vs_projects_automatically'):
		# Workflow improvement: for all builds generate projects after the build
		# except when using the default build target 'utilities' then do it before
		if 'build' in Options.commands:
			build_cmd_idx = Options.commands.index('build')		
			Options.commands.insert(build_cmd_idx, 'msvs') 
		else:
			Options.commands.append('msvs')
	 
	###########################################
	# Load remaining tools for correct auto configure
	if host == 'win32':
		conf.load('recode', tooldir=CRY_WAF_TOOL_DIR)	
		conf.load('static_code_analyzer', tooldir=CRY_WAF_TOOL_DIR)
	
	###########################################
	# Recurse into subfolders for auto conf when any wscript changes
	conf.recurse(dirs=SUBFOLDERS)
		
	# Load Game Specific parts	
	for project in conf.game_projects():
		conf.game_project = project
		conf.recurse( conf.game_code_folder(project))

	###########################################
	# Always update uber files during configuration
	# But don't add the same command twice
	if len(Options.commands) == 0:
		Options.commands.insert(0, 'generate_uber_files')
	elif not Options.commands[0] == 'generate_uber_files':
		Options.commands.insert(0, 'generate_uber_files')
	
	# Remove timestamp files to force builds of generate_uber_files and project gen even if
	# some command after configure failes		
	try:
		generate_uber_files_timestamp = conf.get_bintemp_folder_node().make_node('generate_uber_files.timestamp')	
		os.stat(generate_uber_files_timestamp.abspath())
	except OSError:	
		pass
	else:
		generate_uber_files_timestamp.delete()
	
	try:
		project_gen_timestamp = conf.get_bintemp_folder_node().make_node('project_gen.timestamp')
		os.stat(project_gen_timestamp.abspath())
	except OSError:	
		pass
	else:
		project_gen_timestamp.delete()		

def post_command_exec(bld):	
	# [post project gen]
	if bld.cmd == 'msvs':
		project_gen_timestamp = bld.get_bintemp_folder_node().make_node('project_gen.timestamp')
		project_gen_timestamp.write('')
	# [post uberfile gen]
	elif bld.cmd == 'generate_uber_files':
		generate_uber_files_timestamp = bld.get_bintemp_folder_node().make_node('generate_uber_files.timestamp')
		generate_uber_files_timestamp.write('')
	# [post build]
	elif bld.cmd.startswith('build'):
		for message in bld.post_build_msg_info:
			Logs.info(message)
			
		for message in bld.post_build_msg_warning:
			Logs.warn(message)
			
		for message in bld.post_build_msg_error:
			Logs.error(message)
			
stored_file_filter = ''
stored_output_file = ''

###############################################################################
## Run 'build' step	
def build(bld):		
	# Keep backward compatibility
	if bld.options.project_spec == 'everything':	
		bld.options.project_spec = 'trybuilder'
		
	bld.options.project_spec = bld.options.project_spec.strip() # remove spaces 
	
	# Create a post build message container
	bld.post_build_msg_info = []
	bld.post_build_msg_warning = []
	bld.post_build_msg_error = []	
	
	# Add groups to ensure all task generators who create static lib tasks are executd first
	bld.add_group('generate_static_lib')
	bld.add_group('regular_group')
	# Set back to our regular compile group
	bld.set_group('regular_group')
	
	
	###########################################
	# Load common windows settings if needed
	bld.load('winres', tooldir=CRY_WAF_TOOL_DIR)
		
	# Check if a valid spec is defined	
	if bld.cmd != 'generate_uber_files' and bld.cmd != 'msvs' and bld.cmd != 'eclipse':
		# project spec is a mandatory parameter
		if bld.options.project_spec == '':
			bld.fatal('[ERROR] No Project Spec defined. Use "-p <spec_name>" command line option to specify project spec. Valid specs are "%s". ' % bld.loaded_specs())
		
		# Ensure that the selected spec is supported on this platform
		if not bld.options.project_spec in bld.loaded_specs():
			bld.fatal('[ERROR] Invalid Project Spec (%s) defined, valid specs are %s' % (bld.options.project_spec, bld.loaded_specs()))
	
		# Check for valid single file compilation flag pairs
		if bld.is_option_true('show_preprocessed_file') and bld.options.file_filter == "":  
			bld.fatal('--show-preprocessed-file can only be used in conjunction with --file-filter')
		elif bld.is_option_true('show_disassembly') and bld.options.file_filter == "":  
			bld.fatal('--show-disassembly can only be used in conjunction with --file-filter')    
					
	###########################################
	# Check timestamps for special commands		
	if not 'generate_uber_files' in Options.commands and bld.cmd != 'generate_uber_files':
		generate_uber_files_timestamp = bld.get_bintemp_folder_node().make_node('generate_uber_files.timestamp')
		try:
			os.stat(generate_uber_files_timestamp.abspath())
		except OSError:	
			# No generate_uber file timestamp, restart command chain, prefixed with gen uber files cmd
			Options.commands = ['generate_uber_files'] + [bld.cmd] + Options.commands	
			return
	
	if not 'msvs' in Options.commands and bld.cmd != 'msvs':
		project_gen_timestamp = bld.get_bintemp_folder_node().make_node('project_gen.timestamp')
		try:
			os.stat(project_gen_timestamp.abspath())
		except OSError:	
			# No project_gen timestamp, append project_gen to commands
			if bld.is_option_true('generate_vs_projects_automatically') and  Utils.unversioned_sys_platform() == 'win32':
				if not bld.is_option_true('internal_dont_check_recursive_execution'):
					Options.commands = Options.commands + ['msvs']
					
	###########################################
	# Check for valid variant if we are not generating projects	
	if bld.cmd == 'xcode' or bld.cmd == 'msvs' or bld.cmd == 'eclipse' or bld.cmd == 'generate_uber_files':
		bld.env['PLATFORM'] = 'project_generator'
		bld.env['CONFIGURATION'] = 'project_generator'
	else:
		if not bld.variant:
			bld.fatal('please use a valid build configuration, try "waf --help"')			
		
		(platform, configuration) = bld.get_platform_and_configuration()			
		bld.env['PLATFORM'] 			= platform
		bld.env['CONFIGURATION'] 	= configuration
		
		if  not platform in bld.get_supported_platforms():
			bld.fatal( '[ERROR] - Target platform "%s" not supported. [on host platform: %s]' % (platform, Utils.unversioned_sys_platform()) )

		# check if configuration is valid in the current spec scope
		bVariantValid = False
		for conf in bld.spec_valid_configurations():
			if conf == configuration:
				bVariantValid = True
		if not bVariantValid:
			bld.fatal('[ERROR] Building Spec "%s" for "%s|%s" is not supported. Valid Configurations are: "%s".' %  (bld.options.project_spec, platform, configuration, ', '.join(bld.spec_valid_configurations())))
		
		# check if platform is valid in the current spec scope
		bVariantValid = False
		for p0 in bld.spec_valid_platforms():
			for p1 in bld.get_platform_list(bld.env['PLATFORM']):
				if p0 == p1:
					bVariantValid = True
		if not bVariantValid:
			bld.fatal('[ERROR] Building Spec "%s" for "%s|%s" is not supported.. Valid Platforms are: "%s".' %  (bld.options.project_spec, platform, configuration, ', '.join(bld.spec_valid_platforms())))

		# Ensure that, if specified, target is supported in this spec
		if bld.options.targets:
			for target in bld.options.targets.split(','):
				if not target in  bld.spec_modules():
					bld.fatal('[ERROR] Module "%s" is not configurated to be build in spec "%s" in "%s|%s"' % (target, bld.options.project_spec, platform, configuration))

					
	###########################################
	# Check if we need to start ourself recursively for IB
	if bld.InvokedWAFRecursivly():
		return

	###########################################
	bld.add_post_fun(post_command_exec)
	
	###########################################
	# Setup Output Path for Project Geneators
	bld.solution_name = bld.get_solution_name()
	bld.projects_dir = bld.get_project_output_folder()	
	
					
	###########################################
	# Load configuration overwrites
	bld.env['CONFIG_OVERWRITES'] = bld.get_solution_overrides()
	
	###########################################
	# Load Support for Recode if required
	host = Utils.unversioned_sys_platform()	
	if host == 'win32':
		bld.load('recode', tooldir=CRY_WAF_TOOL_DIR)			
		if str(bld.options.support_recode) == str(True):
			bld.enable_recode = bld.check_global_recode_enabled()
			
	###########################################
	# Add support for static code analyzer
	bld.load('static_code_analyzer', tooldir=CRY_WAF_TOOL_DIR)
		
		
	# Clear <output folder>/Image/Loose on durango as a workaround for buggy deploy
	if bld.env['PLATFORM'] == 'durango':
		for node in bld.get_output_folders(bld.env['PLATFORM']):
			image_lost_dir = node.find_dir('Image/Loose')
			if image_lost_dir:
				files = image_lost_dir.ant_glob('**/*')
				for f in files:
					f.chmod(493) # 0755
					f.delete()

	# Load Core Engine Parts (Engine, Tools, Core Shaders etc)
	bld.game_project = None
	bld.recurse(SUBFOLDERS)	
	
	# Load Game Specific parts	
	for project in bld.game_projects():
		bld.game_project = project
		bld.recurse( bld.game_code_folder(project))
		
###############################################################################
@conf
def target_clean(self):
	
	tmp_targets = self.options.targets[:]
	to_delete = []
	# Sort of recursive algorithm, find all outputs of targets
	# Repeat if new targets were added due to use directives
	while len(tmp_targets) > 0:
		new_targets = []
		
		for tgen in self.get_all_task_gen():				
			tgen.post()
			if not tgen.target in tmp_targets:
				continue
		
			for t in tgen.tasks:
				# Collect outputs
				for n in t.outputs:
					if n.is_child_of(self.bldnode):
						to_delete.append(n)
			# Check for use flag
			if hasattr(tgen, 'use'):			
				new_targets.append(tgen.use)
		# Copy new targets
		tmp_targets = new_targets[:]
		
	# Final File list to delete
	to_delete = list(set(to_delete))
	for file in to_delete:
		file.delete()
	
		
###############################################################################
# Copy pasted from MSVS..
def convert_vs_configuration_to_waf_configuration(configuration):
	if 'Debug' in configuration:
		return 'debug'
	if 'Profile' in configuration:
		return 'profile'
	if 'Release' in configuration:
		return 'release'
	if 'Performance' in configuration:
		return 'performance'
			
	return ''
		
###############################################################################		
@conf
def utilities(bld):		
	if not bld.is_option_true('ask_for_user_input'): 	
		return
		
	# Function to handle creating of new libraries
	def install_waf_addin():
		enviroment = os.environ.copy()		
		if not 'APPDATA' in enviroment:
			print 'APPDATA enviroment variable not found, WAF cannot figure out where to install the addin'
			return
		
		install_path = enviroment['APPDATA'] + '\\Microsoft\\MSEnvShared\\Addins'
		source_path = bld.path.abspath() + '\\Code\\Tools\\waf_addin\\Bin_Release'
		
		Logs.info('WAF Addin will be installed into:\n%s' % install_path)		
		bld.start_msg('Create Install directory')
		try:
			if not os.path.exists(install_path):
				os.makedirs(install_path)
		except:
			bld.end_msg('failed (%s)' % sys.exc_info()[1], color='RED')
		else:
			bld.end_msg('ok')
			
		for file in ['WAF_Addin.AddIn', 'WAF_Addin.dll', 'WAF_Addin.xml']:
			bld.start_msg('Copy %s to install directory' % file)
			# Make output writeable
			try:
				os.chmod(install_path + '\\' + file, 493) # 0755
			except:
				pass
				
			try:
				shutil.copy2(source_path + '\\' + file, install_path + '\\' + file)
			except:
				bld.end_msg('failed (%s)' % sys.exc_info()[1], color='RED')
			else:
				bld.end_msg('ok')
		Options.commands.insert(0, 'utilities')
		
	def bootstrap_update():
		bld.ExecuteBootstrap('update')
		Options.commands.insert(0, 'utilities')
		pass
	
	def regenerate_vs_solution():
		Options.commands.insert(0, 'utilities')
		Options.commands.insert(0, 'msvs')
	
	def regenerate_uber_files():
		Options.commands.insert(0, 'utilities')
		Options.commands.insert(0, 'generate_uber_files')
				
	def regenerate_config_file():
		print type(bld)
		# Remove current config file and remove some state
		config_file = bld.get_user_settings_node()
		config_file.delete()

		# Load user settings		
		bld.load_user_settings()
		
		# Afterwards return to utilies dialog
		Options.commands.insert(0, 'utilities')
		
	def waf_options():				
		Options.commands.insert(0, 'utilities')
		Options.commands.insert(0, 'show_option_dialog')
		
	# Function to handle exit requests
	def exit():
		pass
				
	if not bld.is_option_true('console_mode'):
		menu_file = ['', [ 		
		('Exit', exit) ]
		]
		
		menu_regenerate = ['Regenerate', [ 
		 ("Visual Studio Solution", regenerate_vs_solution),
		 ("Uber Files", regenerate_uber_files)
		 ]]
		
		menu_tools = ['Tools', [("WAF Options", waf_options)]]
		
		if bld.is_bootstrap_available():
			menu_tools[1] += [("Run Bootstrap Update", bootstrap_update)]
			
		#menu_tools[1] += [("Install WAF Addin", install_waf_addin)] # disabled as unmaintained		
		fn_result = bld.gui_get_waf_menu_choice([menu_tools, menu_regenerate, menu_file])
		
		if fn_result:
			fn_result()	 # Execute result
		else:
			Options.commands.insert(0, 'utilities')
			
	else:			
		choices = [ 
		    (exit, 'Exit'),
				#(install_waf_addin,Install WAF Addin), # disabled as unmaintained
				(regenerate_vs_solution, 'Regenerate Visual Studio Solution'),				
				(regenerate_uber_files, 'Regenerate Uber Files'),
				(regenerate_config_file, 'Regenerate Config File')
			]
			
		if bld.is_bootstrap_available():
			choices += [(bootstrap_update, 'Run Bootstrap Update')]
			
		print(' === Crytek WAF Build System === ')
		print(' Please use waf --help to see all valid build targets and build options')
		
		for idx, option in enumerate(choices):
			print('%s: %s' % (idx, option[1]))
		
		user_input = raw_input('Please select an option: ')
		print('')
		# Sanity check for valid user inputs
		try:					
			if int(user_input) < int(0) or int(user_input) > len(choices):
				raise			
		except:
			print('Invalid Input, please choose a value between 1 and %s', len(choices))	
			Options.commands.insert(0, 'utilities')
			return
		
		# Input is fine, exectue selection
		if int(user_input) == 1:
			return
		
		print type(bld)
		choices[int(user_input)][0]()

##############################################################################
class execute_utilities(BuildContext):
	''' Util class to execute waf utilities dialog  '''
	cmd = 'utilities'
	fun = 'utilities'
	
###############################################################################
# Create Build Context Commands for multiple platforms/configurations
for platform in PLATFORMS[Utils.unversioned_sys_platform()]:
	for configuration in CONFIGURATIONS:
		# Create new class to execute build with variant
		name = CleanContext.__name__.replace('Context','').lower()
		class tmp_build(CleanContext):
			cmd = name + '_' + platform + '_' + configuration
			variant = platform + '_' + configuration
			
			def __init__(self, **kw):
				super(CleanContext, self).__init__(**kw)
				self.top_dir = kw.get('top_dir', Context.top_dir)
				
			def execute(self):
				if Configure.autoconfig:
					env = ConfigSet.ConfigSet()
					
					do_config = False
					try:
						env.load(os.path.join(Context.lock_dir, Options.lockfile))
					except Exception:
						Logs.warn('Configuring the project')
						do_config = True
					else:
						if env.run_dir != Context.run_dir:
							do_config = True
						else:
							h = 0
							for f in env['files']:
								try:						
									h = hash((h, Utils.readf(f, 'rb')))
								except (IOError, EOFError):
									pass # ignore missing files (will cause a rerun cause of the changed hash)															
							do_config = h != env.hash

					if do_config:
						Options.commands.insert(0, self.cmd)
						Options.commands.insert(0, 'configure')
						return

				# Execute custom clear command
				self.restore()
				if not self.all_envs:
					self.load_envs()
				self.recurse([self.run_dir])
	
				if self.options.targets:
					self.target_clean()
				else:
					try:
						self.clean()
					finally:
						self.store()
				
			
		# Create new class to execute clean with variant
		name = BuildContext.__name__.replace('Context','').lower()
		class tmp_clean(BuildContext):
			cmd = name + '_' + platform + '_' + configuration
			variant = platform + '_' + configuration				
	
###############################################################################	
import subprocess
import sys 
@conf
def InvokedWAFRecursivly(bld):
	if not Utils.unversioned_sys_platform() == 'win32':
		return False # Don't use recursive execution on non-windows hosts
	if bld.is_option_true('internal_dont_check_recursive_execution'):
		return False
	if 'clean_' in bld.cmd:		
		return	False
	if 'generate_uber_files' in bld.cmd:
		return False		
	if 'msvs' in bld.cmd:		
		return	False
	# Don't use IB for special single file operations
	if bld.is_option_true('show_includes'):
		return False
	if bld.is_option_true('show_preprocessed_file'):
		return False
	if bld.is_option_true('show_disassembly'):
		return False
	if bld.options.file_filter != "":
		return False		
		
	if not bld.is_option_true('use_incredibuild'):
		Logs.warn('[WARNING] Incredibuild disabled by build option')
		return False
			
	# Get correct incredibuild installation folder to not depend on PATH
	IB_settings = _winreg.OpenKey(_winreg.HKEY_LOCAL_MACHINE, "Software\\Wow6432Node\\Xoreax\\Incredibuild\\Builder", 0, _winreg.KEY_READ)
	(ib_folder,type) = _winreg.QueryValueEx(IB_settings, 'Folder')
	# Check if we can execute remotely with our current licenses
	
	if not bld.is_option_true('use_incredibuild_win') and ('win32' in bld.cmd or 'win64' in bld.cmd):
		Logs.warn('[WARNING] Incredibuild for Windows targets disabled by build option')
		return False
	
	if not bld.is_option_true('use_incredibuild_win') and ('win_x86' in bld.cmd or 'win_x64' in bld.cmd):
		Logs.warn('[WARNING] Incredibuild for Windows targets disabled by build option')
		return False
		
	if not bld.is_option_true('use_incredibuild_win') and 'durango' in bld.cmd:
		Logs.warn('[WARNING] Incredibuild for Durango targets disabled by build option')
		return False
		
	if not bld.is_option_true('use_incredibuild_orbis') and 'orbis' in bld.cmd:
		Logs.warn('[WARNING] Incredibuild for Orbis targets disabled by build option')
		return False
	
	result = subprocess.check_output([ str(ib_folder) + '/xgconsole.exe', '/QUERYLICENSE'])
		
	if not '- Make && Build Tools Extension Package' in result:
		Logs.warn('Make & Build Tools Package not found')
		
	if not '- PlayStation' in result and 'orbis' in bld.cmd:
		Logs.warn('Playstation Extension Package not found! Incredibuild will only utilize a single core which will result in a long build times.')

	if not '- Xbox One Extension Package' in result and 'durango' in bld.cmd:
		Logs.warn('Xbox One Extension Package not found! Incredibuild will only utilize a single core which will result in a long build times.')
		
		
	# Get all specific commands, but keep msvs to execute after IB has finished
	bExecuteMSVS = False
	if 'msvs' in Options.commands:
		bExecuteMSVS = True
		
	Options.commands = []
	cmd_line_args = []
	for arg in sys.argv[1:]:
		if arg == 'generate_uber_files':
			continue
		if arg == 'msvs':
			bExecuteMSVS = True
			continue
		cmd_line_args += [ arg ]
	
	if bExecuteMSVS: # Execute MSVS without IB
		Options.commands += [ 'msvs' ]
		
	command_line_options = ' '.join(cmd_line_args) # Recreate command line

	# Add special option to not start IB from within IB
	command_line_options += ' --internal-dont-check-recursive-execution=True'

	num_jobs = bld.options.incredibuild_max_cores
	
	# Build Command Line
	cry_waf = bld.path.make_node('cry_waf.exe')
	command = cry_waf.abspath() + ' --jobs=' + str(num_jobs) + ' ' + command_line_options
		
	sys.stdout.write('[WAF] Starting Incredibuild: ')
		
	if 'cppcheck' in bld.variant or 'linux' in bld.variant or 'android' in bld.variant:
		if subprocess.call([str(ib_folder) + '/xgconsole.exe', "/command=" + command, "/profile=Code\\Tools\\waf-1.7.13\\profile.xml", "/useidemonitor", "/nologo"]):
			raise BuildError()
	else:
		if subprocess.call([ str(ib_folder) + '/BuildConsole.exe', "/command=" + command, "/useidemonitor", "/nologo"]):
			raise BuildError()

	return True
	
				
###############################################################################
# Install Default variant				
for y in (BuildContext, CleanContext):
	class tmp(y):
		variant = 'utilities'
		fun = 'utilities'
		
###############################################################################
# Create Commands for backwards compatibility
if Utils.unversioned_sys_platform() == 'win32':	
		for configuration in CONFIGURATIONS:
			for context in (BuildContext, CleanContext):
				name = context.__name__.replace('Context','').lower()
				class tmp(context):
					cmd = name + '_win32_' + configuration
					variant = 'win_x86_' + configuration
		for configuration in CONFIGURATIONS:
			for context in (BuildContext, CleanContext):
				name = context.__name__.replace('Context','').lower()
				class tmp(context):
					cmd = name + '_win64_' + configuration
					variant = 'win_x64_' + configuration


@taskgen_method
def copy_files(self, source_file):
	for node in self.bld.get_output_folders(self.bld.env['PLATFORM']):
		output_node = node
		if hasattr(self, 'output_sub_folder'):
			output_node = output_node.make_node(self.output_sub_folder)
		output_node = output_node.make_node( os.path.basename(source_file.abspath()) )
	
		path = os.path.dirname( output_node.abspath() )
		if not os.path.exists( path ):
			os.makedirs( path )
		self.create_task('copy_outputs', source_file, output_node) 
	
###############################################################################
# Class to handle copying of the final outputs into the Bin folder
class copy_outputs(Task):
	color = 'YELLOW'
	
	def run(self):
		src = self.inputs[0].abspath()
		tgt = self.outputs[0].abspath()
		
		# Create output folder
		if not os.path.exists( os.path.dirname( tgt ) ):
			os.makedirs( os.path.dirname( tgt ) )

		# Make output writeable
		try:
			os.chmod(tgt, 493) # 0755
		except:
			pass
			
		try:
			shutil.copy2(src, tgt)
		except (IOError, os.error) as why:			
			self.err_msg = "%s\nCould not perform copy %s -> %s" % (str(why), src, tgt)
			return -1
		except:
			self.err_msg = "Could not perform copy %s -> %s" % (src, tgt)
			return -1
		
		return 0
		
	def runnable_status(self):
		if super(copy_outputs, self).runnable_status() == -1:
			return -1
		
		src = self.inputs[0].abspath()
		tgt = self.outputs[0].abspath()

		# If there any target file is missing, we have to copy
		try:
			stat_tgt = os.stat(tgt)
		except OSError:	
			return RUN_ME
		
		# Now compare both file stats
		try:
			stat_src = os.stat(src)				
		except OSError:
			pass
		else:
			# same size and identical timestamps -> make no copy
			if stat_src.st_mtime >= stat_tgt.st_mtime + 2 or stat_src.st_size != stat_tgt.st_size:			
				return RUN_ME
				
		# Everything fine, we can skip this task		
		return -2 # SKIP_ME
		
	
###############################################################################
# Function to generate the copy tasks for build outputs	
@after_method('set_pdb_flags')
@after_method('apply_incpaths')
@after_method('apply_map_file')
@feature('c', 'cxx')
def add_install_copy(self):
	if self.bld.cmd == "msvs":
		return

	if not getattr(self, 'link_task', None):
		return

	if self._type == 'stlib': # Do not copy static libs
		return
		
	for src in self.link_task.outputs:		
		self.copy_files(src)

	
###############################################################################
# Function to generate the EXE_VERSION_INFO defines
@after_method('apply_incpaths')
@feature('c', 'cxx')
def apply_version_info(self):
	version = str( self.bld.get_product_version() )

	version_parts = version.split('.')
	if len(version_parts) < 3:
		Logs.warn('Invalid build version (%s), falling back to (1.0.0.1)' % version )						
		version_parts = ['1', '0', '0', '1']
 
	version_parts[0] = version_parts[0].strip()
	version_parts[1] = version_parts[1].strip()
	version_parts[2] = version_parts[2].strip()
	version_parts[3] = version_parts[3].strip()

	for t in getattr(self, 'compiled_tasks', []):
		t.env.append_value('DEFINES', 'EXE_VERSION_INFO_0=' + version_parts[0])
		t.env.append_value('DEFINES', 'EXE_VERSION_INFO_1=' + version_parts[1])
		t.env.append_value('DEFINES', 'EXE_VERSION_INFO_2=' + version_parts[2])
		t.env.append_value('DEFINES', 'EXE_VERSION_INFO_3=' + version_parts[3])

###############################################################################	
def wrap_execute(execute_method):
	"""
	Decorator used to set the commands that can be configured automatically
	"""
	def execute(self):		
		# Make sure to create all needed temp folders
		bin_temp = self.get_bintemp_folder_node()
		bin_temp.mkdir()
		tmp_files_folder = bin_temp.make_node('TempFiles')
		tmp_files_folder.mkdir()
		
		# Before executing any build or configure commands, check for config file
		# and for bootstrap updates
		self.load_user_settings()
		check_for_bootstrap(self)
	
		return execute_method(self)
			
	return execute
	
BuildContext.execute = wrap_execute(BuildContext.execute)
ConfigurationContext.execute = wrap_execute(ConfigurationContext.execute)

###############################################################################	
def check_for_bootstrap(bld):
	global g_bootstrap_was_run
	if g_bootstrap_was_run:
		return
	g_bootstrap_was_run = True	
	
	if not bld.is_option_true('auto_run_bootstrap'):
		return # Skip auto bootstrapping
		
	bootstrap_dat = bld.path.make_node('bootstrap.dat')
	bootstrap_timestamp = bld.get_bintemp_folder_node().make_node('bootstrap.timestamp')
	
	# Check for bootstrap.dat
	try:
		stat_bootstrap_dat = os.stat(bootstrap_dat.abspath())
	except OSError:	
		self.bld.fatal('bootstrap.dat file not found')
	
	# Check for bootstrap.timestamp, run bootstrap is it doesn't exist
	try:
		stat_bootstrap_timestamp = os.stat(bootstrap_timestamp.abspath())
		if stat_bootstrap_dat.st_mtime < stat_bootstrap_timestamp.st_mtime + 2:
			return # No need to bootstrap
	except OSError:				
		pass
		
	bld.ExecuteBootstrap('update')
	
	# Create TimeStamp File
	bootstrap_timestamp.write('')

###############################################################################	
@conf
def get_output_folders(self, platform, target_spec = None):	
	path = None	
	
	# Find the path for the current platform based on build options
	if platform == 'win_x86':
		path = self.options.out_folder_win32		
	elif platform == 'win_x64':
		path = self.options.out_folder_win64		
	elif platform == 'durango':		
		path = self.options.out_folder_durango
	elif platform == 'orbis':
		path = self.options.out_folder_orbis	
	elif platform == 'linux_x86_gcc':
		path = self.options.out_folder_linux32_gcc
	elif platform == 'linux_x86_clang':
		path = self.options.out_folder_linux32_clang	
	elif platform == 'linux_x64_gcc':
		path = self.options.out_folder_linux64_gcc
	elif platform == 'linux_x64_clang':
		path = self.options.out_folder_linux64_clang
	elif platform == 'darwin_x86':
		path = self.options.out_folder_darwin32
	elif platform == 'darwin_x64':
		path = self.options.out_folder_darwin64
	elif platform == 'android_arm_gcc':
		path = self.options.out_folder_android		
	elif platform == 'android_arm_clang':
		path = self.options.out_folder_android		
	else:
		path = 'BinUnknown'
				
	if not target_spec: # Fall back to command line specified spec if none was given
		target_spec = self.options.project_spec
		
	if target_spec:
		post_fix = self.spec_output_folder_post_fix(target_spec)
		if post_fix:
			path += post_fix

	# Correct handling for absolute paths
	if os.path.isabs(path):
		output_folder_nodes = [ self.root.make_node(path) ]
	else: # For relativ path, prefix binary output folder with game project folder
		output_folder_nodes = []
		if target_spec == None:
			target_spec = self.options.project_spec # Fall back to global spec
		output_folder_nodes += [ self.path.make_node(path)]	
		# Code below is to support multiple output folder
		#for project in self.active_projects(target_spec):
		#	project_folder = self.project_output_folder(project)
		#	output_folder_nodes += [ self.path.make_node(project_folder).make_node(path) ]
		#	output_folder_nodes += [ self.path.make_node(path)	]

		
	return output_folder_nodes

	
###############################################################################
@conf
def is_bootstrap_available(bld):
	bootstrap_path = bld.path.abspath() + '/Tools/branch_bootstrap/dist/bootstrap.exe'
	return os.path.isfile(bootstrap_path)
	
###############################################################################	
@conf
def ExecuteBootstrap(bld, command):

	if not is_bootstrap_available(bld):
		return
		
	if not bld.options.p4user or not bld.options.p4client or not bld.options.p4host:
		Logs.error( "[ERROR] - WAF Bootstrap integration not configured. Missing Perforce User Data info." )
		return
	
	host = Utils.unversioned_sys_platform()
	executable = None
	if host == 'win32':
		executable = [bld.path.abspath() + '/Tools/branch_bootstrap/dist/bootstrap.exe']
	elif host == 'darwin':
		executable = ['python3', bld.path.abspath() + '/Tools/branch_bootstrap/bootstrap.py']
	elif host == 'linux':
		executable = ['python3', bld.path.abspath() + '/Tools/branch_bootstrap/bootstrap.py']
	if executable is None:
		bld.fatal('unknown host, could not determine bootstrap executable')
        
	bootstrap_dat 	= bld.path.abspath() + '/bootstrap.dat'
	
	bintmp = bld.get_bintemp_folder_node()
	bootstrap_cfg = bintmp.make_node('bootstrap.cfg')
	bootstrap_digest = bintmp.make_node('.bootstrap.digest.pickle')
			
	Logs.info('Beginning Branch Bootstrap Operation (this might take a while)')
	
	bld.start_msg('Creating bootstrap config file (' + bootstrap_cfg.abspath() + ')')
	bootstrap_cfg.write('[common]\n'
						'verbose: no\n'
						'data_file: ' + os.path.normpath(bootstrap_dat) + '\n'	
						'\n'
						'[3rdparty]\n'
						'p4user: ' + bld.options.third_party_p4user + '\n'
						'p4client: ' + bld.options.third_party_p4client + '\n'
						'p4port: ' +  bld.options.third_party_p4host + '\n'
						'root_folder: ' + bld.options.third_party_rootfolder + '\n'
						'\n'
						'[workspace]\n'
						'p4user: ' + bld.options.p4user + '\n'
						'p4client: ' + bld.options.p4client + '\n'
						'p4port: ' +  bld.options.p4host)
	bld.end_msg('ok')

	ret = subprocess.call(
                executable + [
                        '-d' + bootstrap_dat
                        , '-c' + bootstrap_cfg.abspath(), 
                        '-m' + bootstrap_digest.abspath(), command])
	if ret == 0:
		bld.msg('branch bootstrap', 'done')
	else:
		bld.msg('branch bootstrap', 'failed', color='RED')
		sys.exit(1)	
		
from waflib.Configure import conf
	
@conf
def read_file_list(bld, file):
	file_node = bld.path.make_node(file)	

	return bld.parse_json_file(file_node)
	
@conf
def get_platform_and_configuration(bld):
	# Assume that the configuration begins after the last '_'
	platform =  "_".join( bld.variant.split("_")[:-1] )
	configuration = bld.variant.split("_")[-1]
	
	# Keep Backward compatibility with old names
	if platform == '_win32':
		platform = 'win_x86'
	if platform == '_win64':
		platform = 'win_x64'

	return (platform, configuration)

@feature('link_to_output_folder')
@after_method('process_source')
def link_to_output_folder(self):
	"""
	Task Generator for tasks which generate symbolic links from the source to the dest folder
	"""
	return # Disabled for now
	
	if self.bld.env['PLATFORM'] == 'project_generator':
		return # Dont create links during project generation

	if sys.getwindowsversion()[0] < 6:
		self.bld.fatal('not supported')
		
	# Compute base relative path (from <Root> to taskgen wscript
	relative_base_path = self.path.path_from(self.bld.path)
	
	# TODO: Need to handle absolute path here correctly
	spec_name = self.bld.options.project_spec			
	for project in self.bld.active_projects(spec_name):
		project_folder = self.bld.project_output_folder(project)
		for file in self.source:
			# Build output folder
			relativ_file_path = file.path_from(self.path)
			
			output_node = self.bld.path.make_node(project_folder)
			output_node = output_node.make_node(relative_base_path)
			output_node = output_node.make_node(relativ_file_path)
			
			path = os.path.dirname( output_node.abspath() )
			if not os.path.exists( path ):
				os.makedirs( path )
			
			self.create_task('create_symbol_link', file, output_node) 
	
import ctypes
###############################################################################
# Class to handle copying of the final outputs into the Bin folder
class create_symbol_link(Task):
	color = 'YELLOW'
	
	def run(self):	
		src = self.inputs[0].abspath()
		tgt = self.outputs[0].abspath()
			
		# Make output writeable
		try:
			os.chmod(tgt, 493) # 0755
		except:
			pass
			
		try:
			kdll = ctypes.windll.LoadLibrary("kernel32.dll")
			res = kdll.CreateSymbolicLinkA(tgt, src, 0)
		except:
			self.generator.bld.fatal("File Link Error (%s -> %s( (%s)" % (src, tgt, sys.exc_info()[0]))
		
		return 0
		
	def runnable_status(self):	
		if super(create_symbol_link, self).runnable_status() == -1:
			return -1
		
		src = self.inputs[0].abspath()
		tgt = self.outputs[0].abspath()

		# If there any target file is missing, we have to copy
		try:
			stat_tgt = os.stat(tgt)
		except OSError:			
			return RUN_ME
		
		# Now compare both file stats
		try:
			stat_src = os.stat(src)				
		except OSError:
			pass
		else:
			# same size and identical timestamps -> make no copy
			if stat_src.st_mtime >= stat_tgt.st_mtime + 2:			
				return RUN_ME
				
		# Everything fine, we can skip this task		
		return -2 # SKIP_ME	


@feature('cxx', 'c', 'cprogram', 'cxxprogram', 'cshlib', 'cxxshlib', 'cstlib', 'cxxstlib')
@after_method('apply_link')
def add_compiler_dependency(self):
	""" Helper function to ensure each compile task depends on the compiler """
	if self.env['PLATFORM'] == 'project_generator':
		return
		
	# Create nodes for compiler and linker
	c_compiler_node = self.bld.root.make_node( self.env['CC'] )
	cxx_compiler_node = self.bld.root.make_node( self.env['CXX'] )
	linker_node = self.bld.root.make_node( self.env['LINK'] )

	# Let all compile tasks depend on the compiler
	for t in getattr(self, 'compiled_tasks', []):
		if os.path.isabs( self.env['CC'] ):
			t.dep_nodes.append(c_compiler_node)
		if os.path.isabs( self.env['CXX'] ):
			t.dep_nodes.append(cxx_compiler_node)
		
	# Let all link tasks depend on the linker
	if getattr(self, 'link_task', None):
		if os.path.isabs(  self.env['LINK'] ):
			self.link_task.dep_nodes.append(linker_node)

###############################################################################
def show_option_dialog(ctx):
	ctx.gui_modify_user_options()
		
###############################################################################
class execute_waf_options_dialog(BuildContext):
	''' Util class to execute waf options dialog  '''
	cmd = 'show_option_dialog'
	fun = 'show_option_dialog'
