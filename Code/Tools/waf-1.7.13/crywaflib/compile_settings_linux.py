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

def load_linux_common_settings(v):
	"""
	Setup all compiler and linker settings shared over all linux configurations
	"""
	
	# Add common durango defines
	v['DEFINES'] += [ 'LINUX', '__linux__' ]
	
	# Setup default libraries to always link against
	v['LIB'] = [ 'pthread', 'dl', ] 
		
	# Pattern to transform outputs
	v['cprogram_PATTERN'] 	= '%s'
	v['cxxprogram_PATTERN'] = '%s'
	v['cshlib_PATTERN'] 	= 'lib%s.so'
	v['cxxshlib_PATTERN'] 	= 'lib%s.so'
	v['cstlib_PATTERN']     = 'lib%s.a'
	v['cxxstlib_PATTERN']   = 'lib%s.a'
	
	# For now compile for SSE2 
	v['CXXFLAGS'] += [ '-msse2'] 
	v['CFLAGS'] += [ '-msse2' ]  
	
@conf
def load_debug_linux_settings(conf):
	"""
	Setup all compiler and linker settings shared over all linux configurations for
	the 'debug' configuration
	"""
	v = conf.env
	load_linux_common_settings(v)
	
@conf
def load_profile_linux_settings(conf):
	"""
	Setup all compiler and linker settings shared over all linux configurations for
	the 'profile' configuration
	"""
	v = conf.env
	load_linux_common_settings(v)
	
@conf
def load_performance_linux_settings(conf):
	"""
	Setup all compiler and linker settings shared over all linux configurations for
	the 'performance' configuration
	"""
	v = conf.env
	load_linux_common_settings(v)
	
@conf
def load_release_linux_settings(conf):
	"""
	Setup all compiler and linker settings shared over all linux configurations for
	the 'release' configuration
	"""
	v = conf.env
	load_linux_common_settings(v)
	
###############################################################################		
###############################################################################
LAUNCHER_SCRIPT='''#!/bin/bash
PATTERN="%e.%t.coredump"
CURRENT_VALUE=`cat /proc/sys/kernel/core_pattern`

if [ "$CURRENT_VALUE" != "%e.%t.coredump" ]; then
	xterm -T "Update Core Dump FileName" -e sudo sh -c "echo %e.%t.coredump > /proc/sys/kernel/core_pattern"
fi

ulimit -c unlimited
xterm -T "${project.to_launch_executable} Launcher" -hold -e './${project.to_launch_executable}'
'''

COMPILE_TEMPLATE = '''def f(project):
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
	
# Function to generate the copy tasks for build outputs	
@feature('cprogram', 'cxxprogram')
@after_method('apply_flags_msvc')
@after_method('apply_link')
def add_linux_launcher_script(self):
	if not getattr(self, 'create_linux_launcher', False):
		return
			
	if self.env['PLATFORM'] == 'project_generator':
		return

	if not getattr(self, 'link_task', None):
		self.bld.fatal('Linux Launcher is only supported for Executable Targets')		

	# Write to rc file if content is different
	for node in self.bld.get_output_folders(self.bld.env['PLATFORM']):
		
		node.mkdir()
	
		for project in self.bld.spec_game_projects():
			# Set up values for linux launcher script template		
			linux_launcher_script_file = node.make_node('Launch_'+self.bld.get_executable_name(project)+'.sh')
			self.to_launch_executable = self.bld.get_executable_name(project)
			
			
			template = compile_template(LAUNCHER_SCRIPT)	
			linux_launcher_script_content = template(self)	
			
			if not os.path.exists(linux_launcher_script_file.abspath()) or linux_launcher_script_file.read() != linux_launcher_script_content:	
				Logs.info('Updating Linux Launcher Script (%s)' % linux_launcher_script_file.abspath() )
				linux_launcher_script_file.write(linux_launcher_script_content)
			
