#!/usr/bin/env python
# encoding: utf-8
# Carlos Rafael Giani, 2006 (dv)
# Tamas Pal, 2007 (folti)
# Nicolas Mercier, 2009
# Matt Clarkson, 2012
# Christopher Bolte: Created a copy to easier adjustment to crytek specific changes

import os, sys, re, tempfile
from waflib import Utils, Task, Logs, Options, Errors
from waflib.Logs import debug, warn
from waflib.Tools import ccroot, c, cxx, ar
from waflib.Configure import conf
from waflib.TaskGen import after_method, feature, before_method


@feature('cprogram', 'cshlib', 'cxxprogram', 'cxxshlib')
@after_method('apply_link')
def apply_manifest(self):
        """
        Special linker for MSVC with support for embedding manifests into DLL's
        and executables compiled by Visual Studio 2005 or probably later. Without
        the manifest file, the binaries are unusable.
        See: http://msdn2.microsoft.com/en-us/library/ms235542(VS.80).aspx
        """
        if self.env.CC_NAME == 'msvc' and self.env.MSVC_MANIFEST and getattr(self, 'link_task', None):                
                out_node = self.link_task.outputs[0]
                man_node = out_node.parent.find_or_declare(out_node.name + '.manifest')
                self.link_task.outputs.append(man_node)
                self.link_task.do_manifest = True

def exec_mf(self):
        """
        Create the manifest file
        """        
        env = self.env
        mtool = env['MT']
        if not mtool:
                return 0

        self.do_manifest = False

        outfile = self.outputs[0].abspath()

        manifest = None
        for out_node in self.outputs:
                if out_node.name.endswith('.manifest'):
                        manifest = out_node.abspath()
                        break
        if manifest is None:
                # Should never get here.  If we do, it means the manifest file was
                # never added to the outputs list, thus we don't have a manifest file
                # to embed, so we just return.
                return 0

        # embedding mode. Different for EXE's and DLL's.
        # see: http://msdn2.microsoft.com/en-us/library/ms235591(VS.80).aspx
        mode = ''
        if 'cprogram' in self.generator.features or 'cxxprogram' in self.generator.features:
                mode = '1'
        elif 'cshlib' in self.generator.features or 'cxxshlib' in self.generator.features:
                mode = '2'

        debug('msvc: embedding manifest in mode %r' % mode)

        lst = []
        lst.append(env['MT'])
        lst.extend(Utils.to_list(env['MTFLAGS']))
        lst.extend(['-manifest', manifest])
        if hasattr(self.generator, 'additional_manifests'):
			lst.append( self.generator.path.abspath() + '/' + self.generator.additional_manifests )
        lst.append('-outputresource:%s;%s' % (outfile, mode))

        lst = [lst]        
        return self.exec_command(*lst)
				
def quote_response_command(self, flag):
        if flag.find(' ') > -1:
                for x in ('/LIBPATH:', '/IMPLIB:', '/OUT:', '/I'):
                        if flag.startswith(x):
                                flag = '%s"%s"' % (x, flag[len(x):])
                                break
                else:
                        flag = '"%s"' % flag
        return flag

def exec_response_command(self, cmd, **kw):
	# not public yet
	try:
		tmp = None
		if sys.platform.startswith('win') and isinstance(cmd, list) and len(' '.join(cmd)) >= 8192:
		        tmp_files_folder = self.generator.bld.get_bintemp_folder_node().make_node('TempFiles')
		        program = cmd[0] #unquoted program name, otherwise exec_command will fail
		        cmd = [self.quote_response_command(x) for x in cmd]
		        out_file = os.path.split(cmd[-1])
		        (fd, tmp) = tempfile.mkstemp(prefix=out_file[1], dir=tmp_files_folder.abspath())
		        os.write(fd, '\r\n'.join(i.replace('\\', '\\\\') for i in cmd[1:]).encode())
		        os.close(fd)
		        cmd = [program, '@' + tmp]
		# no return here, that's on purpose
		ret = self.generator.bld.exec_command(cmd, **kw)

	finally:
		if tmp:
			try:
				os.remove(tmp)
			except OSError:
				pass # anti-virus and indexers can keep the files open -_-
	return ret

########## stupid evil command modification: concatenate the tokens /Fx, /doc, and /x: with the next token
import waflib.Node
def exec_command_msvc(self, *k, **kw):
	"""
	Change the command-line execution for msvc programs.
	Instead of quoting all the paths and keep using the shell, we can just join the options msvc is interested in
	"""
		
	# 1) Join options that carry no space are joined e.g. /Fo FilePath -> /FoFilePath
	# 2) Join options that carry a ':' as last character : e.g. /OUT: FilePath -> /OUT:FilePath
	if isinstance(k[0], list):
		lst = []
		carry = ''
		join_with_next_list_item = ['/Fo', '/doc', '/Fi', '/Fa']	
		for a in k[0]:
			if a in  join_with_next_list_item or a[-1] == ':':
				carry = a
			else:
				lst.append(carry + a)
				carry = ''

		k = [lst]
		
	bld = self.generator.bld
	try:
		if not kw.get('cwd', None):
			kw['cwd'] = bld.cwd
	except AttributeError:
		bld.cwd = kw['cwd'] = bld.variant_dir

	ret = self.exec_response_command(k[0], **kw)
	
	if not ret and getattr(self, 'do_manifest', None):
		ret = self.exec_mf()
	return ret
		
def wrap_class(class_name):
        """
        Manifest file processing and @response file workaround for command-line length limits on Windows systems
        The indicated task class is replaced by a subclass to prevent conflicts in case the class is wrapped more than once
        """    
        cls = Task.classes.get(class_name, None)
        if not cls:
                return

        derived_class = type(class_name, (cls,), {})
        def exec_command(self, *k, **kw):
                if self.env['CC_NAME'] == 'msvc':
                        return self.exec_command_msvc(*k, **kw)
                else:
                        return super(derived_class, self).exec_command(*k, **kw)

        # Chain-up monkeypatch needed since exec_command() is in base class API
        derived_class.exec_command = exec_command

        # No chain-up behavior needed since the following methods aren't in
        # base class API
        derived_class.exec_response_command = exec_response_command
        derived_class.quote_response_command = quote_response_command
        derived_class.exec_command_msvc = exec_command_msvc
        derived_class.exec_mf = exec_mf

        return derived_class

for k in 'c cxx cprogram cxxprogram cshlib cxxshlib cstlib cxxstlib'.split():
	wrap_class(k)
	

@feature('cxxprogram', 'cxxshlib', 'cprogram', 'cshlib', 'cxx', 'c')
@after_method('apply_incpaths')
@after_method('process_pch_msvc')
def set_pdb_flags(self):			
	if not 'msvc' in (self.env.CC_NAME, self.env.CXX_NAME):
		return	

	# Not having PDBs stops CL.exe working for precompiled header when we have VCCompiler set to true for IB...
	if not self. bld.is_option_true('generate_debug_info'):
		return
	
	# Compute PDB file path
	pdb_folder = self.path.get_bld()	
	pdb_file = pdb_folder.make_node(str(self.idx) + '.vc110.pdb')
	pdb_cxxflag = '/Fd' + pdb_file.abspath() + ''

	# Make sure the PDB folder exists
	pdb_folder.mkdir()
	
	if getattr(self, 'link_task', None) and self._type != 'stlib':
		pdbnode = self.link_task.outputs[0].change_ext('.pdb')
		self.link_task.outputs.append(pdbnode)		

	# Add CXX and C Flags
	for t in getattr(self, 'compiled_tasks', []):
		t.env.append_value('CXXFLAGS', pdb_cxxflag) 
		t.env.append_value('CFLAGS', pdb_cxxflag) 

	# Add PDB also to Precompiled header
	for t in self.tasks:
		if isinstance(t,pch_msvc):
			t.env.append_value('CXXFLAGS', pdb_cxxflag) 
			t.env.append_value('CFLAGS', pdb_cxxflag)
	
#############################################################################
@feature('cxxprogram', 'cxxshlib', 'cprogram', 'cshlib', 'cxx', 'c')
@after_method('apply_link')
def apply_map_file(self):
	if not 'msvc' in (self.env.CC_NAME, self.env.CXX_NAME):
		return	

	# Dont create map files if not asked for
	if not self.bld.is_option_true('generate_map_file'):
		return
		
	if getattr(self, 'link_task', None) and self._type != 'stlib':
		map_file_node = self.link_task.outputs[0].change_ext('.map')
		self.link_task.outputs.append(map_file_node)
		self.env.append_value('LINKFLAGS', '/MAP:' + map_file_node.abspath())		

#############################################################################
#############################################################################
# Special handling to be able to create response files
# Based in msvc from WAF
def quote_response_command_orbis(self, flag):
	"""
	if flag.find(' ') > -1:
		for x in ('/LIBPATH:', '/IMPLIB:', '/OUT:', '/I'):
			if flag.startswith(x):
				flag = '%s"%s"' % (x, flag[len(x):])
				break
			else:
				flag = '"%s"' % flag
		return flag
	"""
	flag = '"%s"' % flag
	return flag
	
#############################################################################
def exec_response_command_orbis(self, cmd, **kw):
	# not public yet	
	try:
		tmp = None
		if sys.platform.startswith('win') and isinstance(cmd, list) and len(' '.join(cmd)) >= 8192:			
			program = cmd[0] #unquoted program name, otherwise exec_command will fail
			cmd = [self.quote_response_command_orbis(x) for x in cmd]
			(fd, tmp) = tempfile.mkstemp()
			os.write(fd, '\r\n'.join(i.replace('\\', '\\\\') for i in cmd[1:]).encode())
			os.close(fd)
			cmd = [program, '@' + tmp]
		# no return here, that's on purpose
		ret = self.generator.bld.exec_command(cmd, **kw)
	finally:
		if tmp:
			try:
				os.remove(tmp)
			except OSError:
				pass # anti-virus and indexers can keep the files open -_-
	return ret

#############################################################################
def exec_command_orbis_clang(self, *k, **kw):	
	"""
	if isinstance(k[0], list):
		lst = []
		carry = ''
		for a in k[0]:
			if a == '/Fo' or a == '/doc' or a[-1] == ':':
				carry = a
			else:
				lst.append(carry + a)
				carry = ''
			k = [lst]
			
		if self.env['PATH']:
			env = dict(self.env.env or os.environ)
			env.update(PATH = ';'.join(self.env['PATH']))
			kw['env'] = env
	"""
	bld = self.generator.bld
	try:
		if not kw.get('cwd', None):
			kw['cwd'] = bld.cwd
	except AttributeError:
		bld.cwd = kw['cwd'] = bld.variant_dir

	return self.exec_response_command_orbis(k[0], **kw)
	
#############################################################################
def wrap_class_orbis(class_name):
	"""
	@response file workaround for command-line length limits on Windows systems
	The indicated task class is replaced by a subclass to prevent conflicts in case the class is wrapped more than once
	"""
	cls = Task.classes.get(class_name, None)

	if not cls:
		return None

	derived_class = type(class_name, (cls,), {})

	def exec_command(self, *k, **kw):
		if self.env['CC_NAME'] == 'orbis-clang' or self.env['CC_NAME'] == 'gcc':
			return self.exec_command_orbis_clang(*k, **kw)
		else:
			return super(derived_class, self).exec_command(*k, **kw)

	# Chain-up monkeypatch needed since exec_command() is in base class API
	derived_class.exec_command = exec_command

	# No chain-up behavior needed since the following methods aren't in
	# base class API
	derived_class.exec_response_command_orbis = exec_response_command_orbis
	derived_class.quote_response_command_orbis = quote_response_command_orbis
	derived_class.exec_command_orbis_clang = exec_command_orbis_clang

	return derived_class

#############################################################################
## Wrap call exec commands		
for k in 'c cxx cprogram cxxprogram cshlib cxxshlib cstlib cxxstlib'.split():
	wrap_class_orbis(k)		
	
from waflib.TaskGen import feature, after	
from waflib.Tools import c_preproc
from waflib.Task import Task

@feature('cxx')
@after_method('apply_incpaths')
def process_pch_msvc(self):	
	if not 'msvc' in (self.env.CC_NAME, self.env.CXX_NAME):
		return
		
	if Utils.unversioned_sys_platform() != 'win32':
		return

	# Create Task to compile PCH
	if not getattr(self, 'pch', ''):
		return

	if not self.bld.is_option_true('use_precompiled_header'):
		return

	# Always assume only one PCH File
	pch_source = self.to_nodes(self.pch)[0]

	pch_header = pch_source.change_ext('.h')
	pch_header_name = os.path.split(pch_header.abspath())[1]

	# Generate PCH per target project idx 
	# Avoids the case where two project have the same PCH output path but compile the PCH with different compiler options i.e. defines, includes, ...
	pch_file = pch_source.change_ext('.%d.pch' % self.idx)
	pch_object = pch_source.change_ext('.%d.obj' % self.idx)
	# Create PCH Task
	self.pch_task = pch_task = self.create_task('pch_msvc', pch_source, [pch_object,pch_file])
	pch_task.env.append_value( 'PCH_NAME', pch_header_name )
	pch_task.env.append_value( 'PCH_FILE', '/Fp' + pch_file.abspath() )
	pch_task.env.append_value( 'PCH_OBJ', pch_object.abspath() )
	
	# Append PCH File to each compile task
	for t in getattr(self, 'compiled_tasks', []):	
		input_file = t.inputs[0].abspath()
		file_specific_settings = self.file_specifc_settings.get(input_file, None)
		if file_specific_settings and 'disable_pch' in file_specific_settings and file_specific_settings['disable_pch'] == True:
			continue # Don't append PCH to files for which we don't use them
			
		if getattr(t, 'disable_pch', False) == True:
			continue # Don't append PCH to files for which we don't use them
			
		if t.__class__.__name__ == 'cxx': #Is there a better way to ensure cpp only?
			pch_flag = '/Fp' + pch_file.abspath()
			pch_header = '/Yu' + pch_header_name
			t.env.append_value('CXXFLAGS', pch_header) 
			t.env.append_value('CXXFLAGS', pch_flag)

			# Append PCH to task input to ensure correct ordering
			t.dep_nodes.append(pch_object)

	
	if getattr(self, 'link_task', None):
		self.link_task.inputs.append(pch_object)
	
class pch_msvc(Task):
	run_str = '${CXX} ${PCH_CREATE_ST:PCH_NAME} ${CXXFLAGS} ${CPPPATH_ST:INCPATHS} ${DEFINES_ST:DEFINES} ${SRC} ${CXX_TGT_F}${PCH_OBJ} ${PCH_FILE}'	
	scan    = c_preproc.scan
	color   = 'BLUE'	
	
	def exec_command(self, *k, **kw):	
		return exec_command_msvc(self, *k, **kw)
	
	def exec_response_command(self, *k, **kw):	
		return exec_response_command(self, *k, **kw)	

	def quote_response_command(self, *k, **kw):	
		return quote_response_command(self, *k, **kw)

	def exec_mf(self, *k, **kw):	
		return exec_mf(self, *k, **kw)

from waflib.Tools import cxx,c
	
@feature('cxx')
@after_method('apply_incpaths')
def process_pch_orbis(self):

	if not 'orbis-clang' in (self.env.CC_NAME, self.env.CXX_NAME):
		return

	# Create Task to compile PCH
	if not getattr(self, 'pch', ''):
		return
	
	if not self.is_option_true('use_precompiled_header'):
		return
	
	# Always assume only one PCH File
	pch_source = self.to_nodes(self.pch)[0]

	pch_header = pch_source.change_ext('.h')	
	pch_file = pch_source.change_ext('.h.pch')
	
	# Create PCH Task
	self.create_task('pch_orbis', pch_source, pch_file)

	# Append PCH File to each compile task
	for t in getattr(self, 'compiled_tasks', []):	
		if t.__class__.__name__ == 'cxx': 
			# we need to get the absolute path to the pch.h.pch
			# which we then need to include as pch.h
			# Since WAF is so smart to generate the source path for the header
			# we do the name replacement outself :)
			pch_file_name = pch_file.abspath()
			pch_file_name = pch_file_name.replace('.h.pch', '.h')
			pch_flag = '-include' + pch_file_name

			t.env.append_value('CXXFLAGS', pch_flag) 	
			t.env.append_value('CFLAGS', pch_flag) 	
			
			# Append PCH to task input to ensure correct ordering
			t.dep_nodes.append(pch_file)

class pch_orbis(Task):
	run_str = '${CXX} -x c++-header ${CXXFLAGS} ${CPPPATH_ST:INCPATHS} ${DEFINES_ST:DEFINES} ${SRC} -o ${TGT}'
	scan    = c_preproc.scan
	color   = 'BLUE'
	
def strip_all_but_last_dependent_options(env, strip_list):
	# [CFLAGS] Delete all but last option in reverse as list size changes on each delete
	indices = [i for i, e in enumerate(env.CFLAGS,) if e in strip_list]	
	for idx in reversed(indices[:-1]):
		del env.CFLAGS[idx]
		
	# [CXXFLAGS] Delete all but last option in reverse as list size changes on each delete
	indices = [i for i, e in enumerate(env.CXXFLAGS,) if e in strip_list]	
	for idx in reversed(indices[:-1]):
		del env.CXXFLAGS[idx]
		
def strip_all_but_last_with_prefix(env, prefix):	
	# [CFLAGS] Delete all but last option in reverse as list size changes on each delete
	indices = [i for i, e in enumerate(env.CFLAGS,) if prefix in e]	
	for idx in reversed(indices[:-1]):
		del env.CFLAGS[idx]
		
	# [CXXFLAGS] Delete all but last option in reverse as list size changes on each delete
	indices = [i for i, e in enumerate(env.CXXFLAGS,) if prefix in e]	
	for idx in reversed(indices[:-1]):
		del env.CXXFLAGS[idx]
		
def verify_options_common(env):
	#Optimization options
	strip_all_but_last_dependent_options(env, ["/O1", "/O2", "/Od", "/Ox"])	
	strip_all_but_last_with_prefix(env, "/Ob")
	strip_all_but_last_with_prefix(env, "/Oi")
	strip_all_but_last_with_prefix(env, "/Os")
	strip_all_but_last_with_prefix(env, "/Oy")
	strip_all_but_last_with_prefix(env, "/favor")

	#Code Generation               
	strip_all_but_last_with_prefix(env, "/arch")
	strip_all_but_last_with_prefix(env, "/clr")
	strip_all_but_last_with_prefix(env, "/EH")
	strip_all_but_last_with_prefix(env, "/fp")
	strip_all_but_last_dependent_options(env, ["/Gd", "/Gr", "/Gv", "/Gz"])
	strip_all_but_last_with_prefix(env, "/GL")
	strip_all_but_last_with_prefix(env, "/GR")
	strip_all_but_last_with_prefix(env, "/GS")
	strip_all_but_last_with_prefix(env, "/Gs")
	strip_all_but_last_with_prefix(env, "/Gw")
	strip_all_but_last_with_prefix(env, "/Gy")
	strip_all_but_last_with_prefix(env, "/Qpar")
	strip_all_but_last_with_prefix(env, "/Qvec")
	strip_all_but_last_with_prefix(env, "/RTC")	
	strip_all_but_last_with_prefix(env, "/volatile")

	#Language                               
	strip_all_but_last_with_prefix(env, "/vd")
	strip_all_but_last_dependent_options(env, ["/vmb", "/vmg"])
	strip_all_but_last_dependent_options(env, ["/vmm", "/vms", "/vmv"])
	strip_all_but_last_dependent_options(env, ["/Z7", "/Zi", "/ZI"])
	strip_all_but_last_dependent_options(env, ["/Za", "/Ze"])
	strip_all_but_last_with_prefix(env, "/Zp")
	strip_all_but_last_with_prefix(env, "/ZW")
	
	#Linking
	strip_all_but_last_dependent_options(env, ["/MD", "/MT", "/LD", "/MDd", "/MTd", "/LDd"])

	#Misc
	strip_all_but_last_with_prefix(env, "/errorReport")
	strip_all_but_last_with_prefix(env, "/sdl")
	strip_all_but_last_with_prefix(env, "/Zc:forScope")
	strip_all_but_last_with_prefix(env, "/Zc:wchar_t")
	strip_all_but_last_with_prefix(env, "/Zc:auto")
	strip_all_but_last_with_prefix(env, "/Zc:trigraphs")
	strip_all_but_last_with_prefix(env, "/Zc:rvalueCast")
	strip_all_but_last_with_prefix(env, "/Zc:strictStrings")
	strip_all_but_last_with_prefix(env, "/Zc:inline")
	strip_all_but_last_with_prefix(env, "/WX")
	strip_all_but_last_dependent_options(env, ["/W0", "/W1", "/W2", "/W3", "/W4"])
	
@feature('c', 'cxx')
@after_method('apply_link')
@after_method('process_pch_msvc')
def verify_compiler_options_msvc(self):
	if not 'msvc' in (self.env.CC_NAME, self.env.CXX_NAME):
		return
	
	if Utils.unversioned_sys_platform() != 'win32':
		return
		
	# Verify compiler option (strip all but last for dependant options)
	for t in getattr(self, 'compiled_tasks', []):			
		verify_options_common(t.env)

	# Verify pch_task options (strip all but last for dependant options)
	if hasattr(self, 'pch_task'):
		verify_options_common(self.pch_task.env)

	# Strip unsupported ARCH linker option
	if hasattr(self, 'link_task'):
		del self.link_task.env['ARCH']
	
	
#############################################################################
# Code for auto-recognition of Visual Studio Compiler and Windows SDK Path
# Taken from the original WAF code
#############################################################################
all_msvc_platforms = [ ('x64', 'amd64'), ('x86_amd64', 'amd64'), ('x86', 'x86') ]
"""List of msvc platforms"""

@conf
def auto_detect_mscv_compiler(conf, version, target):
	conf.env['MSVC_VERSIONS'] = [version]
	conf.env['MSVC_TARGETS'] = [target]
	
	conf.autodetect(True)
	conf.find_msvc()	
	
@conf
def autodetect(conf, arch = False):
	v = conf.env
	
	if arch:
		compiler, version, path, includes, libdirs, arch = conf.detect_msvc(True)
		v['DEST_CPU'] = arch
	else:
		compiler, version, path, includes, libdirs = conf.detect_msvc()
    
	v['PATH'] = path
	v['INCLUDES'] = includes
	v['LIBPATH'] = libdirs
	v['MSVC_COMPILER'] = compiler
	try:
		v['MSVC_VERSION'] = float(version)
	except Exception:
		v['MSVC_VERSION'] = float(version[:-3])	

@conf
def detect_msvc(conf, arch = False):
	versions = get_msvc_versions(conf)
	return setup_msvc(conf, versions, arch)	
	
def setup_msvc(conf, versions, arch = False):
	platforms = getattr(Options.options, 'msvc_targets', '').split(',')
	if platforms == ['']:
		platforms=Utils.to_list(conf.env['MSVC_TARGETS']) or [i for i,j in all_msvc_platforms]
	desired_versions = getattr(Options.options, 'msvc_version', '').split(',')
	if desired_versions == ['']:
		desired_versions = conf.env['MSVC_VERSIONS'] or [v for v,_ in versions][::-1]
	versiondict = dict(versions)

	for version in desired_versions:
		try:
			targets = dict(versiondict [version])
			for target in platforms:
				try:
					arch,(p1,p2,p3) = targets[target]
					compiler,revision = version.rsplit(' ', 1)
					if arch:
						return compiler,revision,p1,p2,p3,arch
					else:
						return compiler,revision,p1,p2,p3
				except KeyError: continue
		except KeyError: continue
		
	#Handle x64 platform with only x86_amd64 cross-compiler support (e.g Visual Studio Express)
	for target in platforms:
		if target == 'x64':
			try:
				arch,(p1,p2,p3) = targets['x86_amd64']
				compiler,revision = version.rsplit(' ', 1)
				if arch:
					return compiler,revision,p1,p2,p3,arch
				else:
					return compiler,revision,p1,p2,p3
			except KeyError:
				pass
		
	conf.fatal('msvc: Impossible to find a valid architecture for building (in setup_msvc)')

	
MSVC_INSTALLED_VERSIONS	= []
@conf
def get_msvc_versions(conf):
	"""
	:return: list of compilers installed
	:rtype: list of string
	"""
	global MSVC_INSTALLED_VERSIONS
	if len(MSVC_INSTALLED_VERSIONS) == 0:
		lst = []
		conf.gather_wsdk_versions(lst)
		conf.gather_msvc_versions(lst)
		MSVC_INSTALLED_VERSIONS = lst
	return MSVC_INSTALLED_VERSIONS	

def gather_msvc_detected_versions():
	#Detected MSVC versions!
	version_pattern = re.compile('^(\d\d?\.\d\d?)(Exp)?$')
	detected_versions = []
	for vcver,vcvar in [('VCExpress','Exp'), ('VisualStudio','')]:
		try:
			prefix = 'SOFTWARE\\Wow6432node\\Microsoft\\'+vcver
			all_versions = Utils.winreg.OpenKey(Utils.winreg.HKEY_LOCAL_MACHINE, prefix)
		except WindowsError:
			try:
				prefix = 'SOFTWARE\\Microsoft\\'+vcver
				all_versions = Utils.winreg.OpenKey(Utils.winreg.HKEY_LOCAL_MACHINE, prefix)
			except WindowsError:
				continue

		index = 0
		while 1:
			try:
				version = Utils.winreg.EnumKey(all_versions, index)
			except WindowsError:
				break
			index = index + 1
			match = version_pattern.match(version)
			if not match:
				continue
			else:
				versionnumber = float(match.group(1))
			detected_versions.append((versionnumber, version+vcvar, prefix+"\\"+version))
	def fun(tup):
		return tup[0]

	detected_versions.sort(key = fun)
	return detected_versions
	
@conf
def gather_msvc_versions(conf, versions):
	vc_paths = []
	for (v,version,reg) in gather_msvc_detected_versions():
		try:
			try:
				msvc_version = Utils.winreg.OpenKey(Utils.winreg.HKEY_LOCAL_MACHINE, reg + "\\Setup\\VC")
			except WindowsError:
				msvc_version = Utils.winreg.OpenKey(Utils.winreg.HKEY_LOCAL_MACHINE, reg + "\\Setup\\Microsoft Visual C++")
			path,type = Utils.winreg.QueryValueEx(msvc_version, 'ProductDir')
			vc_paths.append((version, os.path.abspath(str(path))))
		except WindowsError:
			continue
    
	for version,vc_path in vc_paths:
		vs_path = os.path.dirname(vc_path)
		conf.gather_msvc_targets(versions, version, vc_path)
	pass

@conf
def gather_msvc_targets(conf, versions, version, vc_path):
	#Looking for normal MSVC compilers!
	targets = []
	if os.path.isfile(os.path.join(vc_path, 'vcvarsall.bat')):
		for target,realtarget in all_msvc_platforms[::-1]:
			try:
				targets.append((target, (realtarget, conf.get_msvc_version('msvc', version, target, os.path.join(vc_path, 'vcvarsall.bat')))))				
			except conf.errors.ConfigurationError:
				pass
	elif os.path.isfile(os.path.join(vc_path, 'Common7', 'Tools', 'vsvars32.bat')):
		try:
			targets.append(('x86', ('x86', conf.get_msvc_version('msvc', version, 'x86', os.path.join(vc_path, 'Common7', 'Tools', 'vsvars32.bat')))))
		except conf.errors.ConfigurationError:
			pass
	elif os.path.isfile(os.path.join(vc_path, 'Bin', 'vcvars32.bat')):
		try:
			targets.append(('x86', ('x86', conf.get_msvc_version('msvc', version, '', os.path.join(vc_path, 'Bin', 'vcvars32.bat')))))
		except conf.errors.ConfigurationError:
			pass
	if targets:
		versions.append(('msvc '+ version, targets))

def _get_prog_names(conf, compiler):
	if compiler=='intel':
		compiler_name = 'ICL'
		linker_name = 'XILINK'
		lib_name = 'XILIB'
	else:
		# assumes CL.exe
		compiler_name = 'CL'
		linker_name = 'LINK'
		lib_name = 'LIB'
	return compiler_name, linker_name, lib_name
	
@conf
def get_msvc_version(conf, compiler, version, target, vcvars):
	"""
	Create a bat file to obtain the location of the libraries

	:param compiler: ?
	:param version: ?
	:target: ?
	:vcvars: ?
	:return: the location of msvc, the location of include dirs, and the library paths
	:rtype: tuple of strings
	"""
	debug('msvc: get_msvc_version: %r %r %r', compiler, version, target)
	batfile = conf.bldnode.make_node('waf-print-msvc.bat')
	batfile.write("""@echo off
set INCLUDE=
set LIB=
call "%s" %s
echo PATH=%%PATH%%
echo INCLUDE=%%INCLUDE%%
echo LIB=%%LIB%%;%%LIBPATH%%
""" % (vcvars,target))
	sout = conf.cmd_and_log(['cmd', '/E:on', '/V:on', '/C', batfile.abspath()])
	lines = sout.splitlines()
    
	if not lines[0]:
		lines.pop(0)
    
	MSVC_PATH = MSVC_INCDIR = MSVC_LIBDIR = None
	for line in lines:
		if line.startswith('PATH='):
			path = line[5:]
			MSVC_PATH = path.split(';')
		elif line.startswith('INCLUDE='):
			MSVC_INCDIR = [i for i in line[8:].split(';') if i]
		elif line.startswith('LIB='):
			MSVC_LIBDIR = [i for i in line[4:].split(';') if i]
	if None in (MSVC_PATH, MSVC_INCDIR, MSVC_LIBDIR):
		conf.fatal('msvc: Could not find a valid architecture for building (get_msvc_version_3)')
    
	# Check if the compiler is usable at all.
	# The detection may return 64-bit versions even on 32-bit systems, and these would fail to run.
	env = dict(os.environ)
	env.update(PATH = path)
	compiler_name, linker_name, lib_name = _get_prog_names(conf, compiler)
	cxx = conf.find_program(compiler_name, path_list=MSVC_PATH, silent_output=True)
	cxx = conf.cmd_to_list(cxx)
    
	# delete CL if exists. because it could contain parameters wich can change cl's behaviour rather catastrophically.
	if 'CL' in env:
		del(env['CL'])
    
	try:
		try:
			conf.cmd_and_log(cxx + ['/help'], env=env)
		except Exception as e:
			debug('msvc: get_msvc_version: %r %r %r -> failure' % (compiler, version, target))
			debug(str(e))
			conf.fatal('msvc: cannot run the compiler (in get_msvc_version)')
		else:
			debug('msvc: get_msvc_version: %r %r %r -> OK', compiler, version, target)
	finally:
		conf.env[compiler_name] = ''
    
	return (MSVC_PATH, MSVC_INCDIR, MSVC_LIBDIR)

	
@conf
def gather_wsdk_versions(conf, versions):
	"""
	Use winreg to add the msvc versions to the input list

	:param versions: list to modify
	:type versions: list
	"""
	version_pattern = re.compile('^v..?.?\...?.?')
	try:
		all_versions = Utils.winreg.OpenKey(Utils.winreg.HKEY_LOCAL_MACHINE, 'SOFTWARE\\Wow6432node\\Microsoft\\Microsoft SDKs\\Windows')
	except WindowsError:
		try:
			all_versions = Utils.winreg.OpenKey(Utils.winreg.HKEY_LOCAL_MACHINE, 'SOFTWARE\\Microsoft\\Microsoft SDKs\\Windows')
		except WindowsError:
			return
	index = 0
	while 1:
		try:
			version = Utils.winreg.EnumKey(all_versions, index)
		except WindowsError:
			break
		index = index + 1
		if not version_pattern.match(version):
			continue
		try:
			msvc_version = Utils.winreg.OpenKey(all_versions, version)
			path,type = Utils.winreg.QueryValueEx(msvc_version,'InstallationFolder')
		except WindowsError:
			continue
		if os.path.isfile(os.path.join(path, 'bin', 'SetEnv.cmd')):
			targets = []
			for target,arch in all_msvc_platforms:
				try:
					targets.append((target, (arch, conf.get_msvc_version('wsdk', version, '/'+target, os.path.join(path, 'bin', 'SetEnv.cmd')))))
				except conf.errors.ConfigurationError:
					pass
			versions.append(('wsdk ' + version[1:], targets))
	pass
			
@conf
def find_msvc(conf):
	"""Due to path format limitations, limit operation only to native Win32. Yeah it sucks."""
	if sys.platform == 'cygwin':
		conf.fatal('MSVC module does not work under cygwin Python!')

	# the autodetection is supposed to be performed before entering in this method
	v = conf.env
	path = v['PATH']
	compiler = v['MSVC_COMPILER']
	version = v['MSVC_VERSION']

	compiler_name, linker_name, lib_name = _get_prog_names(conf, compiler)
	v.MSVC_MANIFEST = (compiler == 'msvc' and version >= 8) or (compiler == 'wsdk' and version >= 6) or (compiler == 'intel' and version >= 11)

	# compiler
	cxx = None
	if v['CXX']: cxx = v['CXX']
	elif 'CXX' in conf.environ: cxx = conf.environ['CXX']
	cxx = conf.find_program(compiler_name, var='CXX', path_list=path, silent_output=True)

	# before setting anything, check if the compiler is really msvc
	env = dict(conf.environ)
	if path: env.update(PATH = ';'.join(path))
	if not conf.cmd_and_log([cxx] + ['/nologo', '/help'], env=env):
		conf.fatal('the msvc compiler could not be identified')

	# c/c++ compiler
	v['CC'] = v['CXX'] = cxx
	v['CC_NAME'] = v['CXX_NAME'] = 'msvc'

	# linker
	if not v['LINK_CXX']:
		link = conf.find_program(linker_name, path_list=path, silent_output=True)
		if link: v['LINK_CXX'] = link
		else: conf.fatal('%s was not found (linker)' % linker_name)
		v['LINK'] = link

	if not v['LINK_CC']:
		v['LINK_CC'] = v['LINK_CXX']

	# staticlib linker
	if not v['AR']:
		stliblink = conf.find_program(lib_name, path_list=path, var='AR', silent_output=True)
		if not stliblink: return
		v['ARFLAGS'] = ['/NOLOGO']

	# manifest tool. Not required for VS 2003 and below. Must have for VS 2005 and later
	if v.MSVC_MANIFEST:
		conf.find_program('MT', path_list=path, var='MT', silent_output=True)
		v['MTFLAGS'] = ['/NOLOGO']

	try:
		conf.load('winres')
	except Errors.WafError:
		warn('Resource compiler not found. Compiling resource file is disabled')		
