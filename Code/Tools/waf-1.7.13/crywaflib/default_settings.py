#############################################################################
## Crytek Source File
## Copyright (C) 2013, Crytek Studios
##
## Creator: Christopher Bolte
## Date: Oct 31, 2013
## Description: Central configuration file for a branch, should never be
## 				integrated since it is unique for each branch
#############################################################################
from waflib.Configure import conf
from waflib import Context
from waflib import Logs
from waflib import Build
from waflib import Options
from waflib import Utils

from collections import Counter

import ConfigParser 

import subprocess
import sys
import os

user_settings = None

ATTRIBUTE_CALLBACKS = {}
""" Global Registry of callbacks for options which requires special processing """

ATTRIBUTE_VERIFICATION_CALLBACKS = {}
""" Global Registry of callbacks for options which requires special processing """

ATTRIBUTE_HINT_CALLBACKS = {}
""" Global Registry of callbacks for options which requires special processing """

LOADED_OPTIONS = {}
""" Global List of already loaded options, can be used to skip initializations if a master value is set to False """

def register_attribute_callback(f):
	""" 
	Decorator function to register a callback for an options attribute.
	*note* The callback function must have the same name as the attribute *note*
	"""
	ATTRIBUTE_CALLBACKS[f.__name__] = f
	
def register_verify_attribute_callback(f):
	""" 
	Decorator function to register a callback verifying an options attribute.
	*note* The callback function must have the same name as the attribute *note*
	"""
	ATTRIBUTE_VERIFICATION_CALLBACKS[f.__name__] = f
	
def register_hint_attribute_callback(f):
	""" 
	Decorator function to register a callback verifying an options attribute.
	*note* The callback function must have the same name as the attribute *note*
	"""
	ATTRIBUTE_HINT_CALLBACKS[f.__name__] = f

def _is_user_option_true(value):
	""" Convert multiple user inputs to True, False or None	"""
	value = str(value)
	if value.lower() == 'true' or value.lower() == 't' or value.lower() == 'yes' or value.lower() == 'y' or value.lower() == '1':
		return True
	if value.lower() == 'false' or value.lower() == 'f' or value.lower() == 'no' or value.lower() == 'n' or value.lower() == '0':
		return False
		
	return None
	
def _is_user_input_allowed(ctx, option_name, value):
	""" Function to check if it is currently allowed to ask for user input """						
	# If we run a no input build, all new options must be set
	if not ctx.is_option_true('ask_for_user_input'):
		if value != '':
			return False # If we have a valid default value, keep it without user input
		else:
			ctx.fatal('No valid default value for %s, please provide a valid input on the command line' % option_name)

	return True
	
	
def _get_string_value(ctx, msg, value):
	""" Helper function to ask the user for a string value """
	msg += ' '
	while len(msg) < 53:
		msg += ' '
	msg += '['+value+']: '
	
	user_input = raw_input(msg)
	if user_input == '':	# No input -> return default
		return value
	return user_input

	
def _get_boolean_value(ctx, msg, value):
	""" Helper function to ask the user for a boolean value """
	msg += ' '
	while len(msg) < 53:
		msg += ' '
	msg += '['+value+']: '
	
	while True:
		user_input = raw_input(msg)
		
		# No input -> return default
		if user_input == '':	
			return value
			
		ret_val = _is_user_option_true(user_input)
		if ret_val != None:
			return str(ret_val)
			
		Logs.warn('Unknown input "%s"\n Acceptable values (none case sensitive):' % user_input)
		Logs.warn("True : 'true'/'t' or 'yes'/'y' or '1'")
		Logs.warn("False: 'false'/'f' or 'no'/'n' or '0'")


def _default_settings_node(ctx):
	""" Return a Node object pointing to the defaul_settings.json file """
	return ctx.root.make_node(Context.launch_dir).make_node('_WAF_/default_settings.json')
	
	
def _load_default_settings_file(ctx):
	""" Util function to load the default_settings.json file and cache it within the build context """
	if hasattr(ctx, 'default_settings'):
		return
		
	default_settings_node = _default_settings_node(ctx)	
	ctx.default_settings = ctx.parse_json_file(default_settings_node)		


def _validate_incredibuild_registry_settings(ctx):
	""" Helper function to verify the correct incredibuild settings """
	if Utils.unversioned_sys_platform() != 'win32':	
		return # Check windows registry only
		
	if not ctx.is_option_true('use_incredibuild'):
		return # No need to check IB settings if there is no IB
	
	allowUserInput = True
	if not ctx.is_option_true('ask_for_user_input'):
		allowUserInput = False
				
	if Options.options.execsolution:
		allowUserInput = False
			
	if ctx.is_option_true('internal_dont_check_recursive_execution'):
		allowUserInput = False
		
	try:
		import _winreg
		IB_settings_read_only = _winreg.OpenKey(_winreg.HKEY_LOCAL_MACHINE, "Software\\Wow6432Node\\Xoreax\\Incredibuild\\Builder", 0, _winreg.KEY_READ)
	except:
		Logs.warn('[WARNING] Cannot open registry entry "HKEY_LOCAL_MACHINE\\Software\\Wow6432Node\\Xoreax\\Incredibuild\\Builder" for reading, please check your incredibuild installation')
		return
		
	def _check_ib_setting(setting_name, required_value, desc):
		""" Helper function to read (and potentially modify a registry setting for IB """
		try:
			(data,type) = _winreg.QueryValueEx(IB_settings_read_only, setting_name)			
		except:
			import traceback
			traceback.print_exc(file=sys.stdout)
			Logs.warn('[WARNING] Cannot find a registry entry for "HKEY_LOCAL_MACHINE\\Software\\Wow6432Node\\Xoreax\\Incredibuild\\Builder\\%s"' % setting_name )
			return
		
		# Do we have the right value?
		if str(data) != required_value:		

			if not allowUserInput: # Dont try anything if no input is allowed		
				Logs.warn('[WARNING] "HKEY_LOCAL_MACHINE\\Software\\Wow6432Node\\Xoreax\\Incredibuild\\Builder\\%s" set to "%s" but should be "0"' % (setting_name, data) )
				return
			
			try: # Try to open the registry for writing
				IB_settings_writing = _winreg.OpenKey(_winreg.HKEY_LOCAL_MACHINE, "Software\\Wow6432Node\\Xoreax\\Incredibuild\\Builder", 0,  _winreg.KEY_SET_VALUE |  _winreg.KEY_READ)
			except:
				Logs.warn('[WARNING] Cannot access a registry entry "HKEY_LOCAL_MACHINE\\Software\\Wow6432Node\\Xoreax\\Incredibuild\\Builder\\%s" for writing.' % setting_name)
				Logs.warn('[WARNING] Please run cry_waf.exe as an administrator or change the value to "0" in the registry to ensure a correct operation of WAF' )
				return
				
			info_str = [('Should WAF change "HKEY_LOCAL_MACHINE\\Software\\Wow6432Node\\Xoreax\\Incredibuild\\Builder\\%s" from "%s" to "%s"?' % (setting_name, data, required_value) )]
			info_str.append(desc)
			
			# Get user choice
			if not ctx.is_option_true('console_mode'): # gui
				retVal = 'True' if ctx.gui_get_choice('\n'.join(info_str)) else 'False'
			else: # console
				retVal = _get_boolean_value(ctx, 'Input', 'Yes')		
			
			if retVal == 'True' or retVal == 'Yes':
				_winreg.SetValueEx(IB_settings_writing, setting_name,0, type, str(0))
			else:
				Logs.warn('[WARNING] WAF is running with "unsupported" IncrediBuild settings. Expect to encounter IncrediBuild errors during compilation.')
	
	_check_ib_setting('PdbForwardingMode', '0', '"PdbForwardingMode" controls how Incredibuld handles PDB files and is mandatory for WAF')
	_check_ib_setting('MaxConcurrentPDBs', '0', '"MaxConcurrentPDBs" controls how many files can be processed in parallel (an optimization also useful for MSBuild)')
	_check_ib_setting('AllowDoubleTargets', '0', '"AllowDoubleTargets" controls if remote processes can be restarted on local machine when possible\nThis option is mandatory since it causes compiler crashed with WAF')

def _incredibuild_disclaimer(ctx):
	""" Helper function to show a disclaimer over incredibuild before asking for settings """
	if getattr(ctx, 'incredibuild_disclaimer_shown', False):
		return
	Logs.info('\nWAF is using Incredibuild for distributed Builds')
	Logs.info('To be able to compile with WAF, various licenses are required:')
	Logs.info('The "Make & Build Tools Extension Package"   is always needed')
	Logs.info('The "Playstation Extension Package"          is needed for PS4 Builds')
	Logs.info('The "Xbox One Extension Package"             is needed for Xbox One Builds')
	Logs.info('If some packages are missing, please ask IT')
	Logs.info('to assign the needed ones to your machine')

	ctx.incredibuild_disclaimer_shown = True
	 
	 
def _verify_incredibuild_licence(licence_name, platform_name):
	""" Helper function to check if user has a incredibuild licence """	
	try:
		result = subprocess.check_output(['xgconsole.exe', '/QUERYLICENSE'])
	except:
		error = '[ERROR] Incredibuild not found on system'
		return False, "", error
		
	if not licence_name in result:
		error = '[ERROR] Incredibuild on "%s" Disabled - Missing IB licence: "%s"' % (platform_name, licence_name)
		return False, "", error
		
	return True,"", ""

	
def _use_incredibuild(ctx, section_name, option_name, value, verification_fn):
	""" If Incredibuild should be used, check for required packages """
	
	if not ctx.is_option_true('ask_for_user_input'):
		(isValid, warning, error) = verification_fn(ctx, option_name, value)	
		if not isValid:
			return 'False'
		return value
		
	# GUI
	if not ctx.is_option_true('console_mode'):
		return ctx.gui_get_attribute(section_name, option_name, value)
		
	if not value or value != 'True':
		return value
	if not Utils.unversioned_sys_platform() == 'win32':
		return value	
		
	_incredibuild_disclaimer(ctx)	
	ctx.start_msg('Incredibuild Licence Check')
	(res, warning, error) = verification_fn(ctx, option_name, value)	
	if not res:
		if warning:
			Logs.warn(warning)
		if error:
			ctx.end_msg(error, color='YELLOW')			
		return 'False'

	ctx.end_msg('ok')
	return value

	
@register_attribute_callback	
def use_incredibuild(ctx, section_name, option_name, value):
	""" If Incredibuild should be used, check for required packages """		
	return _use_incredibuild(ctx, section_name, option_name, value, ATTRIBUTE_VERIFICATION_CALLBACKS['verify_use_incredibuild'])
	
	
@register_verify_attribute_callback
def verify_use_incredibuild(ctx, option_name, value):	
	""" Verify value for use_incredibuild """		
	if not _is_user_option_true(value):
		return (True,"","")	
	(res, warning, error) = _verify_incredibuild_licence('Make && Build Tools Extension Package', 'All Platforms')	
	return (res, warning, error)	

	
@register_attribute_callback		
def use_incredibuild_win(ctx, section_name, option_name, value):
	""" IB uses Make & Build Tools also for MSVC Builds, hence we can skip a check here """
	if LOADED_OPTIONS.get('use_incredibuild', 'False') == 'False':
		return 'False'
		
	return 'True'

	
@register_verify_attribute_callback
def verify_use_incredibuild_win(ctx, option_name, value):
	""" Verify value for verify_use_incredibuild_win """	
	if not _is_user_option_true(value):
		return (True,"","")	
	(res, warning, error) = _verify_incredibuild_licence('Make && Build Tools Extension Package', 'Windows')	
	return (res, warning, error)

	
@register_attribute_callback		
def use_incredibuild_durango(ctx, section_name, option_name, value):
	""" If Incredibuild should be used, check for requiered packages """
	if LOADED_OPTIONS.get('use_incredibuild', 'False') == 'False':
		return 'False'
		
	return _use_incredibuild(ctx, section_name, option_name, value, ATTRIBUTE_VERIFICATION_CALLBACKS['verify_use_incredibuild_durango'])
	
	
@register_verify_attribute_callback
def verify_use_incredibuild_durango(ctx, option_name, value):
	""" Verify value for verify_use_incredibuild_durango """
	if not _is_user_option_true(value):
		return (True,"","")	
	(res, warning, error) = _verify_incredibuild_licence('Xbox One Extension Package', 'Durango')	
	return (res, warning, error)

	
@register_attribute_callback		
def use_incredibuild_orbis(ctx, section_name, option_name, value):
	""" If Incredibuild should be used, check for required packages """
	if LOADED_OPTIONS.get('use_incredibuild', 'False') == 'False':
		return 'False'
		
	return _use_incredibuild(ctx, section_name, option_name, value, ATTRIBUTE_VERIFICATION_CALLBACKS['verify_use_incredibuild_orbis'])
@register_verify_attribute_callback
def verify_use_incredibuild_orbis(ctx, option_name, value):
	""" Verify value for verify_use_incredibuild_orbis """		
	if not _is_user_option_true(value):
		return (True,"","")	
	(res, warning, error) = _verify_incredibuild_licence('PlayStation', 'Orbis')	
	return (res, warning, error)
	
@register_attribute_callback	
def use_uber_files(ctx, section_name, option_name, value):
	""" Configure the usage of UberFiles """
	if not _is_user_input_allowed(ctx, option_name, value):
		Logs.info('\nUser Input disabled.\nUsing default value "%s" for option: "%s"' % (value, option_name))
		return value

	info_str = ['UberFiles significantly improve total compile time of the CryEngine']
	info_str.append('at a slight cost of single file compilation time.')
	
	# GUI
	if not ctx.is_option_true('console_mode'):
		return ctx.gui_get_attribute(section_name, option_name, value, '\n'.join(info_str))
		
	info_str.append("(Press ENTER to keep the current default value shown in [])")
	Logs.info('\n'.join(info_str))
	return _get_boolean_value(ctx, 'Should UberFiles be used', value)


@register_attribute_callback		
def generate_vs_projects_automatically(ctx, section_name, option_name, value):
	""" Configure automatic project generation """
	if not _is_user_input_allowed(ctx, option_name, value):
		Logs.info('\nUser Input disabled.\nUsing default value "%s" for option: "%s"' % (value, option_name))
		return value
		
	info_str = ['Allow WAF to track changes and automatically keep Visual Studio projects up-to-date for you?']
	
	# Gui
	if not ctx.is_option_true('console_mode'):
		return ctx.gui_get_attribute(section_name, option_name, value, '\n'.join(info_str))	
	# Console
	else:		
		info_str.append('\n(Press ENTER to keep the current default value shown in [])')
		Logs.info('\n'.join(info_str))
		return _get_boolean_value(ctx, 'Enable Automatic generation of Visual Studio Projects/Solution', value)
			
@register_attribute_callback	
def auto_run_bootstrap(ctx, section_name, option_name, value):
	""" Configure automatic boostrap execution """	
	if not ctx.is_bootstrap_available():
		return 'False'
	
	if not _is_user_input_allowed(ctx, option_name, value):
		Logs.info('\nUser Input disabled.\nUsing default value "%s" for option: "%s"' % (value, option_name))
		return value
		
	# Check for P4 support
	# On failure: (i.e. no p4 available) Do not show if default has been 'False' in the first place
	# On failure: (i.e. no p4 available) Do show option if default has been 'True'
	(res, warning, error) = ATTRIBUTE_VERIFICATION_CALLBACKS['verify_auto_run_bootstrap'](ctx, option_name, 'True')
	if not res and not _is_user_option_true(value):
		return 'False';
		
	info_str = ["Automatically execute Branch Bootstrap on each build?"]
	info_str.append("[When disabled the user is responsible to keep his 3rdParty Folder up to date]")
	
	# GUI
	if not ctx.is_option_true('console_mode'):
		return ctx.gui_get_attribute(section_name, option_name, value, '\n'.join(info_str))
	
	info_str.append('\n(Press ENTER to keep the current default value shown in [])')
	Logs.info('\n'.join(info_str))
	while True:
		value = _get_boolean_value(ctx, 'Enable Automatic Execution of Branch BootStrap', value)
		(res, warning, error) = ATTRIBUTE_VERIFICATION_CALLBACKS['verify_auto_run_bootstrap'](ctx, option_name, value)
		
		if res:
			break
		else:			
			Logs.warn(error)
		
	return value

@register_verify_attribute_callback
def verify_auto_run_bootstrap(ctx, option_name, value):
	""" Verify auto run bootstrap """
	res = False
	warning = ""
	error = ""
	
	if not ctx.is_bootstrap_available():
		return (True,"","")
	
	if not _is_user_option_true(value):
		res = True
		return (res, warning, error)
	
	try:
		subprocess.check_output(['p4']) # use check output as we do not want to spam the waf cmd window
		res = True
	except CalledProcessError:
		# The process ran but did not return 0.
		# All we want to check here is that p4 exists.
		# Hence this is valid
		res = True
	except:
		error = "[ERROR] Unable to execute 'p4'"
		res = False
		
	return (res, warning, error)

	
@register_attribute_callback	
def bootstrap_dat_file(ctx, section_name, option_name, value):
	""" Configure the name of the boostrap.dat file """
	if LOADED_OPTIONS.get('auto_run_bootstrap', 'False') == 'False':
		return ''
		
	if not _is_user_input_allowed(ctx, option_name, value):
		Logs.info('\nUser Input disabled.\nUsing default value "%s" for option: "%s"' % (value, option_name))
		return value
					
	# GUI
	if not ctx.is_option_true('console_mode'):
		return ctx.gui_get_attribute(section_name, option_name, value)
	
	Logs.info("\nName of the bootstrap.dat file to use")
	Logs.info("(Press ENTER to keep the current default value shown in [])")
	
	while True:	
		file_name = _get_string_value(ctx, 'Bootstrap.dat file:', value)
		(res, warning, error) = ATTRIBUTE_VERIFICATION_CALLBACKS['verify_bootstrap_dat_file'](ctx, option_name, file_name)
		
		if res:
			break
		else:			
			Logs.warn(error)
		
	#	Valid file, return
	return file_name	


@register_verify_attribute_callback
def verify_bootstrap_dat_file(ctx, option_name, value):
	""" Verify bootstrap dat file """
	res = True
	error = ""
	
	node = ctx.root.make_node(Context.launch_dir).make_node(value)
	if not os.path.exists(node.abspath()):
		error = 'Could not find Bootstrap.dat file: "%s"' % node.abspath()
		res = False
	
	return (res,"", error)

	
def _p4user_helper(ctx, option_name, value, prefix, p4server):
	""" Util function to provide functionality to load a p4 user """
	# Try to figure out the current user to provide some default values
	default_user = ""
	try:
		result = subprocess.check_output(['p4', '-p', p4server, 'user', '-o'])
		for line in result.splitlines():
			if line.lstrip().startswith('User:'):
				default_user = line[len('User:'):].lstrip()
				break
	except:
		error = "[ERROR] Cannot execute p4 to figure out a default user name"
		return (False, "", error)
		
	# Ask user to input a value
	user = _get_string_value(ctx, prefix, default_user)
	
	if not user:
		error = "[ERROR] No 'P4 User Name' defined. Cannot execute p4 to verify user (%s)" % workspace
		return (False, "", error)
		
	if not p4server:
		error = "[ERROR] No 'P4 Server Name' defined. Cannot execute p4 to verify user (%s)" % workspace
		return (False, "", error)
		
	# Check if this user exists
	try:
		user_list = subprocess.check_output(['p4', '-p', p4server, 'users'])
	except:
		error = "[ERROR] Cannot execute p4 to verify user name (%s)" % user
		return (False, "", error)
		
	while True:			
		# Add space at the end of the user name to ensure we are checking the complete name and not just a subset.
		# Otherwise "s' would be a valid user name in the example below
		# Line example:
		# - sampelU <SampelUser@crytek.de> (Sample User) accessed 2014/11/06
		name_to_check = user + ' '			
		for line in user_list.splitlines():
			if line.lstrip().startswith(name_to_check):
				return user
				
		Logs.warn('Perforce cannot find user: "%s"' % user)
		user = _get_string_value(ctx, prefix, default_user)
			
	return user

	
###############################################################################
def _verify_p4_user(p4user, p4server):
	""" Helper function to verify third party p4 user"""
	res = False
	warning = ""
	error = ""	
	
	if not p4user:
		error = "[ERROR] No 'P4 User Name' defined."
		return (False, warning, error)
		
	if not p4server:
		error = "[ERROR] No 'P4 Server Name' defined. Cannot execute p4 to verify user (%s)" % p4user
		return (False, warning, error)
	
	# Check if this user exists
	try:
		user_list = subprocess.check_output(['p4', '-p', p4server, 'users'])
	except:
		error = "[ERROR] Cannot execute p4 to verify user name '%s' on server:'%s'" % (p4user, p4server)
		return (False, warning, error)
		
	# Add space at the end of the user name to ensure we are checking the complete name and not just a subset.
	# Otherwise "s' would be a valid user name in the example below
	# Line example:
	# - sampelU <SampelUser@crytek.de> (Sample User) accessed 2014/11/06
	name_to_check = p4user + ' '
	for line in user_list.splitlines():
		if line.lstrip().startswith(name_to_check):
			res = True
			break
			
	if not res:
		error = "[ERROR] User '%s' not found" % p4user
			
	return res, warning, error
	
	
@register_attribute_callback	
def p4user(ctx, section_name, option_name, value):
	""" Configure a P4 User """
	if LOADED_OPTIONS.get('auto_run_bootstrap', 'False') == 'False':
		return ''
	if not _is_user_input_allowed(ctx, option_name, value):
		Logs.info('\nUser Input disabled.\nUsing default value "%s" for option: "%s"' % (value, option_name))
		return value	
		
	# GUI
	if not ctx.is_option_true('console_mode'):
		return ctx.gui_get_attribute(section_name, option_name, value)
		
	Logs.info("\nName of the perforce user to use with bootstrap")
	Logs.info("(Press ENTER to keep the current default value shown in [])")
		
	return _p4user_helper(ctx, option_name, value, "Perforce User", LOADED_OPTIONS.get('p4host', ''))
	
	
@register_verify_attribute_callback
def verify_p4user(ctx, option_name, value):
	""" Verify p4 user"""
	(res, warning, error) = _verify_p4_user(value, LOADED_OPTIONS.get('p4host', None))
	return (res, warning, error)

	
@register_attribute_callback	
def third_party_p4user(ctx, section_name, option_name, value):
	""" Configure a P4 User for the 3rdParty Space """
	if LOADED_OPTIONS.get('auto_run_bootstrap', 'False') == 'False':
		return ''
	if not LOADED_OPTIONS.get('third_party_p4host',""):
		return ''		
	if not _is_user_input_allowed(ctx, option_name, value):
		Logs.info('\nUser Input disabled.\nUsing default value "%s" for option: "%s"' % (value, option_name))
		return value	
		
	# Try to guess user name
	if not value:
		value = LOADED_OPTIONS.get('p4user', '')
		
	# GUI
	if not ctx.is_option_true('console_mode'):
		return ctx.gui_get_attribute(section_name, option_name, value)
		
	Logs.info("\nName of the perforce user to use with bootstrap")
	Logs.info("(Press ENTER to keep the current default value shown in [])")
		
	return _p4user_helper(ctx, option_name, value, "3rdParty Perforce User", LOADED_OPTIONS.get('third_party_p4host', ''))

	
@register_verify_attribute_callback
def verify_third_party_p4user(ctx, option_name, value):
	""" Verify third party p4 user"""
	(res, warning, error) = _verify_p4_user(value, LOADED_OPTIONS.get('third_party_p4host',""))
	return (res, warning, error)


def _p4_workspaces(p4user, p4server):
	""" Helper function to get p4 workspace list for p4 user on p4 server"""
	
	if not p4user:
		error = "[ERROR] No 'P4 User Name' defined. Cannot execute p4 to verify workspace"
		return ([], error)
		
	if not p4server:
		error = "[ERROR] No 'P4 Server Name' defined. Cannot execute p4 to verify workspace"
		return ([], error)
		
	try:
		workspaces = subprocess.check_output(['p4', '-p', p4server, 'workspaces', '-u', p4user])
	except:
		error = "[ERROR] Cannot execute p4 to verify workspace"
		return [], error
		
	# Format of a P4 workspace is: Client <Workspace> root <...> 'Created by <user> '
	workspace_list = []
	p4user_to_check = p4user + '.'
	for line in workspaces.splitlines():
		if line.rfind(p4user_to_check):			
			line = line[len('Client '):]
			next_space = line.find(' ')
			workspace_name = line[:next_space].strip()
			workspace_list.append(workspace_name)
		
	return (workspace_list, "")

###############################################################################
def _p4client_helper(ctx, option_name, value, prefix, p4user, p4server):
	""" Util function to provide functionality to load a p4 workspace """
	
	# Get list of workspaces
	(workspace_list, error) = _p4_workspaces(p4user, p4server)
	
	if not workspace_list:
		Log.warning(error)
		return ""
		
	# Quick selection for user
	Logs.info('\nQuick option(s) for user "%s":' %  p4user)
	p4user_to_check = p4user + '.'
	quick_select_options = {}
	counter = 0
	for workspace_name in workspace_list:
		quick_select_options[str(counter)] = workspace_name
		Logs.info('  %s: %s' % (counter, workspace_name))
		counter += 1
	
	if counter == 0:
		Logs.info('  None available for user')
	
	while True:		
	
		# Ask user to input a value
		workspace = _get_string_value(ctx, prefix, value)	
		
		# Quick select		
		if workspace.isdigit():
			quick_option = quick_select_options.get(workspace, '')
			
			if quick_option:
				return quick_option
			else:
				Logs.warn('Invalid option: "%s"' % workspace)
				continue
		
		for p4_workspace_name in workspace_list:
			# Match workspace exactly
			if p4_workspace_name ==  workspace:
					return workspace						
		
		# No match found
		Logs.warn('Perforce cannot find workspace: "%s" owned by user "%s" on server "%s"' % (workspace, p4user, p4server))

	return workspace


def _verify_p4_client(workspace, p4user, p4server):
	""" Util function to provide functionality to load a p4 workspace """
		
	ret = False
	warning = ''
	error = ''
			
	# Check if this workspace exists
	(workspace_list, error) = _p4_workspaces(p4user, p4server)
	
	if not workspace_list:
		return (False, warning, error)
		
	for workspace_name in workspace_list:		
		# Match workspace exactly
		if workspace_name == workspace:
			return (True, "", "")				

	# No match found
	error  = 'Perforce cannot find workspace: "%s"\nowned by user "%s" on server "%s"' % (workspace, p4user, p4server)	
	return (ret, warning, error)


@register_attribute_callback	
def p4client(ctx, section_name, option_name, value):
	""" Configure a P4 Client """
	if LOADED_OPTIONS.get('auto_run_bootstrap', 'False') == 'False':
		return ''
		
	if not _is_user_input_allowed(ctx, option_name, value):
		Logs.info('\nUser Input disabled.\nUsing default value "%s" for option: "%s"' % (value, option_name))
		return value	
		
	# GUI
	if not ctx.is_option_true('console_mode'):
		return ctx.gui_get_attribute(section_name, option_name, value)
		
	Logs.info("\nName of the perforce workspace to use with bootstrap")
	Logs.info("(Press ENTER to keep the current default value shown in [])")

	return _p4client_helper(ctx, option_name, value, "Perforce Workspace", LOADED_OPTIONS.get('p4user', ''), LOADED_OPTIONS.get('p4host', ''))
	
	
@register_verify_attribute_callback
def verify_p4client(ctx, option_name, value):
	""" Verify p4 client """
	(res, warning, error) = _verify_p4_client(value, LOADED_OPTIONS.get('p4user', ''), LOADED_OPTIONS.get('p4host', ''))
	return (res, warning, error)	
	
@register_hint_attribute_callback
def hint_p4client(ctx, section_name, option_name, value):
	""" Hint p4 client """
	(workspace_list, error) = _p4_workspaces(LOADED_OPTIONS.get('p4user', ''), LOADED_OPTIONS.get('p4host', ''))	
	return (workspace_list, workspace_list, [],"single")
	
@register_attribute_callback		
def third_party_p4client(ctx, section_name, option_name, value):
	""" Configure a ThirdParty P4 Client """
	if LOADED_OPTIONS.get('auto_run_bootstrap', 'False')== 'False':
		return ''
	if not LOADED_OPTIONS.get('third_party_p4host', ''):
		return ''
	if not _is_user_input_allowed(ctx, option_name, value):
		Logs.info('\nUser Input disabled.\nUsing default value "%s" for option: "%s"' % (value, option_name))
		return value	
	
	# GUI
	if not ctx.is_option_true('console_mode'):
		return ctx.gui_get_attribute(section_name, option_name, value)
		
	Logs.info("\nName of the 3rdparty perforce workspace to use with bootstrap")
	Logs.info("(Press ENTER to keep the current default value shown in [])")

	return _p4client_helper(ctx, option_name, value, "3rdParty Perforce Workspace", LOADED_OPTIONS.get('third_party_p4user', ''), LOADED_OPTIONS.get('third_party_p4host',''))
	
@register_verify_attribute_callback
def verify_third_party_p4client(ctx, option_name, value):
	""" Verify third party p4 client """
	(res, warning, error) = _verify_p4_client(value, LOADED_OPTIONS.get('third_party_p4user',''), LOADED_OPTIONS.get('third_party_p4host',''))
	return (res, warning, error)

###############################################################################
@register_hint_attribute_callback
def hint_third_party_p4client(ctx, section_name, option_name, value):
	""" Hint third party p4 client """
	(workspace_list, error) = _p4_workspaces(LOADED_OPTIONS.get('third_party_p4user',''), LOADED_OPTIONS.get('third_party_p4host',''))	
	return (workspace_list, workspace_list, [],"single")

###############################################################################
def _verify_p4_host(p4server):
	""" Helper function to verify p4 host """
	ret = False
	error = ''
			
	if not p4server:
		error = "[ERROR] No 'P4 Server Name' defined."
		return (False, error)

	# Check if this workspace exists
	try:
		output = subprocess.check_output(['p4', '-p', p4server, 'info'], stderr=subprocess.STDOUT)
	except Exception,e:
		error = "[ERROR] Cannot execute p4 to verify server (%s)" % p4server
		return (False, error)
		
	if output.startswith("Perforce client error"):
		error = "[ERROR] Unable to connect to p4 server: '%s'" % p4server
		return (False, error)
		
	return (True, "")


def _p4host_input_helper(ctx, section_name, option_name, value, prefix):
	""" Configure a P4 server """
	if LOADED_OPTIONS.get('auto_run_bootstrap', 'False') == 'False':
		return ''
	
	if not _is_user_input_allowed(ctx, option_name, value):
		Logs.info('\nUser Input disabled.\nUsing default value "%s" for option: "%s"' % (value, option_name))
		return value	
			
	# GUI
	if not ctx.is_option_true('console_mode'):
		return ctx.gui_get_attribute(section_name, option_name, value)
		
	Logs.info("\nAddress of the Perforce Server to use for bootstrapping.")
	Logs.info("(Press ENTER to keep the current default value shown in [])")
		
	while True:
		new_value = _get_string_value(ctx, prefix, value)
		(res, error) = _verify_p4_host(new_value)
				
		if not res:
			Logs.warn(error)
			continue
			
		return new_value

	
@register_attribute_callback	
def p4host(ctx, section_name, option_name, value):
	""" Configure a P4 server """
	return _p4host_input_helper(ctx, section_name, option_name, value, 'Perforce Server')	


@register_verify_attribute_callback
def verify_p4host(ctx, option_name, value):
	""" Verify p4 host """
	(res, error) = _verify_p4_host(value)
	return (res, "", error)	


@register_attribute_callback	
def third_party_p4host(ctx, section_name, option_name, value):
	""" Configure a p4host """
	if LOADED_OPTIONS.get('auto_run_bootstrap', '') == 'False':
		return ''
		
	if not _is_user_input_allowed(ctx, option_name, value):
		return value						
			
	info_str = ["Bootstrap supports two different modes:"]
	info_str.append("  1: Single Workspace: The 3rdParty folder is mapped into the regular workspace")
	info_str.append("  2: Multi Workspace:  The 3rdParty folder is mapped into a workspace other than the regular workspace")
	
	# GUI Choice Dialog
	if not ctx.is_option_true('console_mode'):
		info_str.append("\n Use 'Multi Workspace' mode? (recommended)")	
		mode = '2' if ctx.gui_get_choice('\n'.join(info_str)) else '1'
		if mode == '1':
			return '' # Use regular workspace i.e. leave "third_party_p4host" blank
	# Console
	else:
		info_str.append("(Press ENTER to keep the current default value shown in [])")
		Logs.info('\n'.join(info_str))
		
		while True:	
			mode = _get_string_value(ctx, 'Select Option', '2')
		
			if mode == '1':
				return '' # Use regular workspace i.e. leave "third_party_p4host" blank
			elif mode == '2':
				break
			
			Logs.warn('"%s" is not a valid option', mode)
			
	return _p4host_input_helper(ctx, section_name, option_name, value, '3rd Party Perforce Server')


@register_verify_attribute_callback
def verify_third_party_p4host(ctx, option_name, value):
	""" Verify p4 third party host """
	(res, error) = _verify_p4_host(value)
	return (res, "", error)


def _get_rootfolder_for_workspace(ctx, p4host, p4user,  p4client):
	""" Get rootfolder for p4 workspace """
	try:
		p4Info = subprocess.check_output(['p4', '-z', 'tag' ,'-p', p4host, '-u', p4user, '-c', p4client, 'info'])
	except:
		return ("", "[ERROR] Cannot execute p4 to get client_root")
		
	client_root_offset = p4Info.find('clientRoot')
	
	if client_root_offset == -1:
		return ("", "[ERROR] Unable to find client root in p4 info output.")
		
	client_root_offset = client_root_offset + len('clientRoot')
	client_root_end = p4Info.find(os.linesep, client_root_offset) # get end of line
	client_root = p4Info[client_root_offset:client_root_end]	
	client_root = client_root.strip()
	client_root = client_root.replace('\\','/')
	return client_root, ""

def _get_views_for_client(ctx, p4host, p4user, p4client):
	""" Get perforce folder mappings for p4 workspace """
	try:
		workspace_info = subprocess.check_output(['p4' ,'-p', p4host, '-u', p4user, '-c', p4client, 'workspace','-o'])
	except:
		return ([], "[ERROR] Cannot execute p4 to get client_root")
		
	view_offset = workspace_info.rfind('View:')
	workspace_info = workspace_info[view_offset:]
	
	if not workspace_info:
		return ([], "[ERROR] Workspace has no p4 views assigned to it.")
	
	view_list = []
	for line in workspace_info.splitlines()[1:]:
		line = line.strip()
		
		if not line:
			break
			
		# Skip info about excluded paths
		# e.g. "-//depot/Games/... //WORKSPACE_NAME/depot/Games/..."
		if line.startswith('-'):
			continue
		
		view_list.append(line)	

	view_names_list = []
	if view_list:
		for idx, view_str in enumerate(view_list):
			view_idx = view_str.rfind('//'+p4client)			
			view_name = view_str[view_idx:].strip()
			view_name = view_name.replace('"',' ') # "//WORKSPACE_NAME_WITH_SPACES/depot/Work        In Progress/..."
			view_names_list.append(view_name)
		
	return view_names_list, ""

def _get_third_party_root_user_info():
	""" Depending on previous user choice get regular or third party p4 user info for root folder """
	if LOADED_OPTIONS.get('third_party_p4host', ''):
		p4host = LOADED_OPTIONS.get('third_party_p4host', '')
		p4user = LOADED_OPTIONS.get('third_party_p4user', '')
		p4client = LOADED_OPTIONS.get('third_party_p4client', '')
	else:
		p4host = LOADED_OPTIONS.get('p4host', '')
		p4user = LOADED_OPTIONS.get('p4user', '')
		p4client = LOADED_OPTIONS.get('p4client', '')
		
	return (p4host, p4user, p4client)

def _sanitize_third_party_folder(ctx, input_rootfolder, workspace_rootfolder, p4client):
	""" Convert third party folder path to cross platform absolute path """
	# Folder case must match against fs		
	input_rootfolder_sanitized = input_rootfolder.replace('\\','/')
	
	# Remove "/..." ending
	if input_rootfolder_sanitized.endswith('/...'):
		input_rootfolder_sanitized = input_rootfolder_sanitized[:-4]
			
	if os.path.isabs(workspace_rootfolder):
		# Replace root name with root and strip leading '//'
		# i.e. //WORKSPACE_NAME/depot/WorkInProgress/...  -> C:/xxx/depot/WorkInProgress/...
		input_rootfolder_sanitized = input_rootfolder_sanitized.replace('//' + p4client, workspace_rootfolder)
		
		# No test required, workspace_rootfolder is already a valid input 
		if input_rootfolder_sanitized == workspace_rootfolder:
			return (input_rootfolder_sanitized, "")
			
		# Relative path
		if input_rootfolder_sanitized.startswith('/'):
			input_rootfolder_sanitized = workspace_rootfolder + input_rootfolder_sanitized

		# Check if working in the root
		if not input_rootfolder_sanitized.startswith(workspace_rootfolder):
			error = "Folder must be under workspace root"
			return ("", error)
	else:
		# i.e. //WORKSPACE_NAME/e:/WorkInProgress/...
		index = input_rootfolder_sanitized.rfind(":")
		if not index  == -1:
			input_rootfolder_sanitized = input_rootfolder_sanitized[index-1:]
		else:
			error = '[Error] Unable to decyper folder layout. Root is "(null)" but there is no absolute path in view "%s"', input_rootfolder_sanitized
			return ("", error)

	return (input_rootfolder_sanitized, "")
	
@register_attribute_callback		
def third_party_rootfolder(ctx, section_name, option_name, value):
	""" Configure a Bootstrap Root Folder """
	if LOADED_OPTIONS.get('auto_run_bootstrap', 'False') == 'False':
		return ''

	if not _is_user_input_allowed(ctx, option_name, value):
		Logs.info('\nUser Input disabled.\nUsing default value "%s" for option: "%s"' % (value, option_name))
		return value
					
	# GUI
	if not ctx.is_option_true('console_mode'):
		return ctx.gui_get_attribute(section_name, option_name, value)
		
	Logs.info("\nPath of the 3rd Party folder to use:")
	
	(p4host, p4user, p4client) = _get_third_party_root_user_info()
		
	# Get workspace root folder
	(workspace_rootfolder, error) = _get_rootfolder_for_workspace(ctx, p4host, p4user, p4client)
	
	if error:
		Logs.warn(error)
		return ''		
		
	Logs.info("\nWorkspace: %s" % p4client)
	Logs.info("Root: %s" % workspace_rootfolder)		
		
	# Get view list
	(view_list, error) = _get_views_for_client(ctx, p4host, p4user, p4client)
	
	if error:
		Logs.warn(error)
		return ''		
	
	if view_list:
		Logs.info("\nQuick option(s):")
		for idx, view_name in enumerate(view_list):
			Logs.info('  %s: %s' % (idx, view_name))
	
	Logs.info("\n(Enter option number or input root absolute/relative path.)")
	while True:
		# Get rootfolder
		input_rootfolder = _get_string_value(ctx, '3rdParty Rootfolder', workspace_rootfolder)
		
		if input_rootfolder.isdigit():
			try:
				input_rootfolder = view_list[int(input_rootfolder)]
			except:
				Logs.warn("Incorrect option '%s'" % input_rootfolder)
				continue
				
		# Sanitize input
		(input_rootfolder_sanitized, error) = _sanitize_third_party_folder(ctx, input_rootfolder, workspace_rootfolder, p4client)
		
		if error:
			Logs.warn(error)
			continue		
			
		# Get true absolute path
		input_rootfolder_sanitized = os.path.abspath(input_rootfolder_sanitized)
		input_rootfolder_sanitized = input_rootfolder_sanitized.replace('\\','/')
		
		(res, warning, error) = ATTRIBUTE_VERIFICATION_CALLBACKS['verify_third_party_rootfolder'](ctx, option_name, input_rootfolder_sanitized)
		if error:
			Logs.warn(error)
			continue
		
		return input_rootfolder_sanitized

@register_hint_attribute_callback		
def hint_third_party_rootfolder(ctx, section_name, option_name, value):
	""" Hint third party rootfolder """
	(p4host, p4user, p4client) = _get_third_party_root_user_info()
	(view_list_for_client, error) = _get_views_for_client(ctx, p4host, p4user, p4client)
	
	# Get workspace root folder
	(workspace_rootfolder, error) = _get_rootfolder_for_workspace(ctx, p4host, p4user, p4client)
	
	view_value_list = []
	for view in view_list_for_client:	
		# Sanitize input
		(input_rootfolder_sanitized, error) = _sanitize_third_party_folder(ctx, view, workspace_rootfolder, p4client)
		view_value_list.append(input_rootfolder_sanitized)
		
	return (view_list_for_client, view_value_list, [], "single")

###############################################################################
@register_verify_attribute_callback
def verify_third_party_rootfolder(ctx, option_name, value):
	""" Verify third party rootfolder for regular P4 user or specific third party P4 user """
	warning = ''
	error = ''

	# Load options
	p4host = LOADED_OPTIONS.get('p4host', '')
	p4user = LOADED_OPTIONS.get('p4user', '')
	p4client = LOADED_OPTIONS.get('p4client', '')
	
	third_party_p4host = LOADED_OPTIONS.get('third_party_p4host', '')
	third_party_p4user = LOADED_OPTIONS.get('third_party_p4user', '')
	third_party_p4client = LOADED_OPTIONS.get('third_party_p4client', '')
		
	# Verify third party workspace usage
	use_third_party_workspace = False	
	if not third_party_p4host or not third_party_p4user or not third_party_p4client:
		use_third_party_workspace = False
	else:
		ret_third_party_p4host = ATTRIBUTE_VERIFICATION_CALLBACKS['verify_third_party_p4host'](ctx, 'third_party_p4host', third_party_p4host)
		ret_third_party_p4user = ATTRIBUTE_VERIFICATION_CALLBACKS['verify_third_party_p4user'](ctx, 'third_party_p4user', third_party_p4user)
		ret_third_party_p4client = ATTRIBUTE_VERIFICATION_CALLBACKS['verify_third_party_p4client'](ctx, 'third_party_p4client', third_party_p4client)
		
		if ret_third_party_p4host and ret_third_party_p4user and ret_third_party_p4client:
			use_third_party_workspace = True
		else:
			error ='[ERROR] Unable to verify 3rd party rootfolder. Invalid Third Party Workspace:\nHost:%s\nUser:%s\nClient:%s' % (third_party_p4host, third_party_p4user, third_party_p4client)
			return (False,warning,error)

	# Verify p4 user info
	if not use_third_party_workspace:
		ret_p4host = ATTRIBUTE_VERIFICATION_CALLBACKS['verify_p4host'](ctx, 'p4host', p4host)
		ret_p4user = ATTRIBUTE_VERIFICATION_CALLBACKS['verify_p4user'](ctx, 'p4user', p4user)
		ret_p4client =  ATTRIBUTE_VERIFICATION_CALLBACKS['verify_p4client'](ctx, 'p4client', p4client)
		
		if not ret_p4host or not ret_p4user or not ret_p4client:
			error ='[ERROR] Unable to verify 3rd party rootfolder. Invalid Perforce User Workspace:\nHost:%s\nUser:%s\nClient:%s' % (p4host, p4user, p4client)
			return (False,warning,error)
			
	# Override settings
	if use_third_party_workspace:
		p4host = third_party_p4host
		p4user = third_party_p4user
		p4client = third_party_p4client
			
	# Get workspace root folder
	(rootfolder, error) = _get_rootfolder_for_workspace(ctx, p4host, p4user, p4client)
	if error:
		return (False, warning, error)

	# Ensure target folder starts with root folder (do not check for '(null)' root types)
	if os.path.isabs(rootfolder) and not value.startswith(rootfolder):
		if value.lower().startswith(rootfolder.lower()):
			# Hit user if the case is incorrect
			error = '[ERROR] Case missmatch.\nRoot:  %s\nTarget:%s' % (rootfolder, value)
			return (False,  warning, error)
		
		error ='[ERROR] Root folder not under workspace root:\nWorkspace:"%s"\nWorkspace root:"%s"\nTarget:"%s"' % (p4client, rootfolder, value)
		return (False,warning,error)
	
	#[ASK] Should root folder exist on disc?
	## Folder must exist on disk
	#if not os.path.exists(value):
	#	error = '[ERROR] Root folder does not exist on disk. ("%s")' % value
	#	return (False, warning, error)
	
	return (True, warning, error)	


###############################################################################
@register_attribute_callback		
def specs_to_include_in_project_generation(ctx, section_name, option_name, value):	
	""" Configure all Specs which should be included in Visual Studio """
	if LOADED_OPTIONS.get('generate_vs_projects_automatically', 'False') == 'False':
		return ''
		
	if not ctx.is_option_true('ask_for_user_input'): # allow null value for specs list
		return value	
	
	info_str = ['Specify which specs to include when generating Visual Studio projects.']
	info_str.append('Each spec defines a list of dependent project for that modules .')
	
	# GUI
	if not ctx.is_option_true('console_mode'):
		return ctx.gui_get_attribute(section_name, option_name, value, '\n'.join(info_str))
	
	# Console	
	info_str.append("\nQuick option(s) (separate by comma):")
	
	spec_list = ctx.loaded_specs()
	spec_list.sort()
	for idx , spec in enumerate(spec_list):
		output = '   %s: %s: ' % (idx, spec)
		while len(output) < 25:
			output += ' '
		output += ctx.spec_description(spec)
		info_str.append(output)
		
	info_str.append("(Press ENTER to keep the current default value shown in [])")
	Logs.info('\n'.join(info_str))
	
	while True:
		specs = _get_string_value(ctx, 'Comma separated spec list', value)		
		spec_input_list = specs.replace(' ', '').split(',')
				
		# Replace quick options
		options_valid = True
		for spec_idx, spec_name in enumerate(spec_input_list):
			if spec_name.isdigit():
				option_idx = int(spec_name)
				try:
					spec_input_list[spec_idx] = spec_list[option_idx]
				except:
					Logs.warn('[WARNING] - Invalid option: "%s"' % option_idx)
					options_valid = False
					
		if not options_valid:
			continue
					
		specs = ','.join(spec_input_list)	
		(res, warning, error) = ATTRIBUTE_VERIFICATION_CALLBACKS['verify_specs_to_include_in_project_generation'](ctx, option_name, specs)
		
		if error:
			Logs.warn(error)
			continue
			
		return specs


###############################################################################
@register_verify_attribute_callback
def verify_specs_to_include_in_project_generation(ctx, option_name, value):	
	""" Configure all Specs which should be included in Visual Studio """	
	if not value:
		if not ctx.is_option_true('ask_for_user_input'): # allow null value for specs list
			return (True, "", "")
		error = ' [ERROR] - Empty spec list not supported'
		return (False, "", error)
		
	spec_list =  ctx.loaded_specs()
	spec_input_list = value.strip().replace(' ', '').split(',')	
	
	# Get number of occurrences per item in list
	num_of_occurrences = Counter(spec_input_list)	
	
	for input in spec_input_list:
		# Ensure spec is valid
		if not input in spec_list:
			error = ' [ERROR] Unkown spec: "%s".' % input
			return (False, "", error)
		# Ensure each spec only exists once in list
		elif not num_of_occurrences[input] == 1:
			error = ' [ERROR] Multiple occurrences of "%s" in final spec value: "%s"' % (input, value)
			return (False, "", error)
		
	return True, "", ""	


###############################################################################
@register_hint_attribute_callback
def hint_specs_to_include_in_project_generation(ctx, section_name, option_name, value):
	""" Hint list of specs for projection generation """
	spec_list = sorted(ctx.loaded_specs())
	desc_list = []
	
	for spec in spec_list:
		desc_list.append(ctx.spec_description(spec))			

	return (spec_list, spec_list, desc_list, "multi")


###############################################################################
def options(ctx):
	""" Create options for all entries in default_settings.json """
	_load_default_settings_file(ctx)
			
	default_settings_file = _default_settings_node(ctx).abspath()
		
	# Load default settings
	for settings_group, settings_list in ctx.default_settings.items():		
		group = ctx.add_option_group(settings_group)		

		# Iterate over all options in this group
		for settings in settings_list:
			
			# Do some basic sanity checking for each option
			if not settings.get('attribute', None):
				ctx.cry_file_error('One Option in group %s is missing mandatory setting "attribute"' % settings_group, default_settings_file)
			if not settings.get('long_form', None):
				ctx.cry_file_error('Option "%s" is missing mandatory setting "long_form"' % settings['attribute'], default_settings_file)
			if not settings.get('description', None):
				ctx.cry_file_error('Option "%s" is missing mandatory setting "description"' % settings['attribute'], default_settings_file)
				
			#	Add option with its default value
			if settings.get('short_form'):
				group.add_option(settings['short_form'], settings['long_form'],  dest=settings['attribute'], action='store', default=str(settings.get('default_value', '')), help=settings['description'])
			else:
				group.add_option(settings['long_form'],  dest=settings['attribute'], action='store', default=str(settings.get('default_value', '')), help=settings['description'])							
			
###############################################################################
@conf
def get_user_settings_node(ctx):
	""" Return a Node object pointing to the user_settings.options file """
	return ctx.root.make_node(Context.launch_dir).make_node('_WAF_/user_settings.options')
	
###############################################################################
@conf 
def	load_user_settings(ctx):
	""" Apply all loaded options if they are different that the default value, and no cmd line value is presented """
	global user_settings

	_load_default_settings_file(ctx)

	write_user_settings		= False	
	user_settings 				= ConfigParser.ConfigParser()	
	user_setting_file 		= ctx.get_user_settings_node().abspath()
	new_options   				= {}
		
	# Load existing user settings
	if not os.path.exists( user_setting_file ):
		write_user_settings = True	# No file, hence we need to write it
	else:
		user_settings.read( [user_setting_file] )
		
	# Load settings and check for newly set ones
	for section_name, settings_list in ctx.default_settings.items():
		
		# Add not already present sections
		if not user_settings.has_section(section_name):
			user_settings.add_section(section_name)
			write_user_settings = True
			
		# Iterate over all options in this group
		for settings in settings_list:
			option_name 	= settings['attribute']
			default_value = settings.get('default_value', '')

			# Load the value from user settings if it is already present	
			if  user_settings.has_option(section_name, option_name):
				value = user_settings.get(section_name, settings['attribute'])
				LOADED_OPTIONS[ option_name ] = value
			else:
				# Add info about newly added option
				if not new_options.has_key(section_name):
					new_options[section_name] = []
				
				new_options[section_name].append(option_name)
				
				# Load value for current option and stringify it
				value = settings.get('default_value', '')
				if getattr(ctx.options, option_name) != value:
					value = getattr(ctx.options, option_name)
					
				if not isinstance(value, str):
					value = str(value)
					
				if	ATTRIBUTE_CALLBACKS.get(option_name, None):					
					value = ATTRIBUTE_CALLBACKS[option_name](ctx, section_name, settings['attribute'], value)
				
				(isValid, warning, error) = ctx.verify_settings_option(option_name, value)
								
				# Add option
				if isValid:
					user_settings.set( section_name, settings['attribute'], str(value) )
					LOADED_OPTIONS[ option_name ] = value
					write_user_settings = True

			# Check for settings provided by the cmd line
			long_form			= settings['long_form']
			short_form		= settings.get('short_form', None)
			
			# Settings on cmdline should have priority, do a sub string match to batch both --option=<SomeThing> and --option <Something>			
			bOptionSetOnCmdLine = False
			for arg in sys.argv:
				if long_form in arg:	
					bOptionSetOnCmdLine = True
					value = getattr(ctx.options, option_name)
					break
			for arg in sys.argv:
				if short_form and short_form in arg:
					bOptionSetOnCmdLine = True
					value = getattr(ctx.options, option_name)
					break
					
			# Remember option for internal processing
			if bOptionSetOnCmdLine:
				LOADED_OPTIONS[ option_name ] = value			
			elif user_settings.has_option(section_name, option_name): # Load all settings not coming form the cmd line from the config file
				setattr(ctx.options, option_name, user_settings.get(section_name, option_name))

	# Write user settings
	if write_user_settings:
		ctx.save_user_settings(user_settings)

  # Verify IB registry settings after loading all options
	_validate_incredibuild_registry_settings(ctx)
			
	return (user_settings, new_options)


###############################################################################
@conf
def get_default_settings(ctx, section_name, setting_name):
	default_value = ""
	default_description = ""
	
	if not hasattr(ctx, 'default_settings'):
		return (default_value, default_description)
		
	for settings_group, settings_list in ctx.default_settings.items():
		if settings_group == section_name:
			for settings in settings_list:
				if settings['attribute'] == setting_name:
					default_value = settings.get('default_value', '')
					default_description = settings.get('description', '')
					break
			break
	
	return (default_value, default_description)


###############################################################################
@conf 
def	save_user_settings(ctx, config_parser):
	# Write user settings	
	with open(ctx.get_user_settings_node().abspath(), 'wb') as configfile:
		config_parser.write(configfile)

		
###############################################################################
@conf
def verify_settings_option(ctx, option_name, value):		
	verify_fn_name = "verify_" + option_name	
	if ATTRIBUTE_VERIFICATION_CALLBACKS.get(verify_fn_name, None):
		return ATTRIBUTE_VERIFICATION_CALLBACKS[verify_fn_name](ctx, option_name, value)
	else:
		return (True, "", "")

	
###############################################################################
@conf
def hint_settings_option(ctx, section_name, option_name, value):		
	verify_fn_name = "hint_" + option_name	
	if ATTRIBUTE_HINT_CALLBACKS.get(verify_fn_name, None):
		return ATTRIBUTE_HINT_CALLBACKS[verify_fn_name](ctx, section_name, option_name, value)
	else:
		return ([], [], [], "single")

	
###############################################################################
@conf
def set_settings_option(ctx, section_name, option_name, value):	
	global user_settings
	user_settings.set(section_name, option_name, str(value))
	LOADED_OPTIONS[option_name] = value	

