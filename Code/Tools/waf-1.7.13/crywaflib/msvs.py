#! /usr/bin/env python
# encoding: utf-8
# Avalanche Studios 2009-2011
# Thomas Nagy 2011
# Christopher Bolte: Created a copy to easier adjustment to crytek specific changes

"""
Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:

1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.

3. The name of the author may not be used to endorse or promote products
   derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE AUTHOR "AS IS" AND ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT,
INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.
"""

"""
To add this tool to your project:
def options(conf):
	opt.load('msvs')

It can be a good idea to add the sync_exec tool too.

To generate solution files:
$ waf configure msvs

To customize the outputs, provide subclasses in your wscript files:

from waflib.extras import msvs
class vsnode_target(msvs.vsnode_target):
	def get_build_command(self, props):
		# likely to be required
		return "waf.bat build"
	def collect_source(self):
		# likely to be required
		...
class msvs_bar(msvs.msvs_generator):
	def init(self):
		msvs.msvs_generator.init(self)
		self.vsnode_target = vsnode_target

The msvs class re-uses the same build() function for reading the targets (task generators),
you may therefore specify msvs settings on the context object:

def build(bld):
	bld.solution_name = 'foo.sln'
	bld.waf_command = 'waf.bat'
	bld.projects_dir = bld.srcnode.make_node('.depproj')
	bld.projects_dir.mkdir()

For visual studio 2008, the command is called 'msvs2008', and the classes
such as vsnode_target are wrapped by a decorator class 'wrap_2008' to
provide special functionality.

ASSUMPTIONS:
* a project can be either a directory or a target, vcxproj files are written only for targets that have source files
* each project is a vcxproj file, therefore the project uuid needs only to be a hash of the absolute path
"""

import os, re, sys, errno
import uuid # requires python 2.5
from waflib.Build import BuildContext
from waflib import Utils, TaskGen, Logs, Task, Context, Node, Options
from waflib.Configure import conf
import mscv_helper
	
HEADERS_GLOB = '**/(*.h|*.hpp|*.H|*.inl)'

PROJECT_TEMPLATE = r'''<?xml version="1.0" encoding="UTF-8"?>
<Project DefaultTargets="Build" ToolsVersion="4.0"
	xmlns="http://schemas.microsoft.com/developer/msbuild/2003">

	<ItemGroup Label="ProjectConfigurations">
		${for b in project.build_properties}
		<ProjectConfiguration Include="${b.configuration}|${b.platform}">
			<Configuration>${b.configuration}</Configuration>
			<Platform>${b.platform}</Platform>
		</ProjectConfiguration>
		${endfor}
	</ItemGroup>

	<PropertyGroup Label="Globals">
		<ProjectGuid>{${project.uuid}}</ProjectGuid>
		<Keyword>MakefileProj</Keyword>
		<ProjectName>${project.name}</ProjectName>
	</PropertyGroup>
	<Import Project="$(VCTargetsPath)\Microsoft.Cpp.Default.props" />

	${for b in project.build_properties}
	<PropertyGroup Condition="'$(Configuration)|$(Platform)'=='${b.configuration}|${b.platform}'" Label="Configuration">
		<ConfigurationType>Makefile</ConfigurationType>
		<OutDir>${b.outdir}</OutDir>		
		<PlatformToolset>v110</PlatformToolset>		
	</PropertyGroup>
	${endfor}
	
	<Import Project="$(VCTargetsPath)\Microsoft.Cpp.props" />
	<ImportGroup Label="ExtensionSettings">
	</ImportGroup>

	${for b in project.build_properties}
	<ImportGroup Label="PropertySheets" Condition="'$(Configuration)|$(Platform)'=='${b.configuration}|${b.platform}'">
		<Import Project="$(MSBuildProjectDirectory)\$(MSBuildProjectName).vcxproj.default.props" Condition="exists('$(MSBuildProjectDirectory)\$(MSBuildProjectName).vcxproj.default.props')"/>
		<Import Project="$(UserRootDir)\Microsoft.Cpp.$(Platform).user.props" Condition="exists('$(UserRootDir)\Microsoft.Cpp.$(Platform).user.props')" Label="LocalAppDataPlatform" />
	</ImportGroup>
	${endfor}
		
	${for b in project.build_properties}
	<PropertyGroup Condition="'$(Configuration)|$(Platform)'=='${b.configuration}|${b.platform}'">
		<NMakeBuildCommandLine>${xml:project.get_build_command(b)}</NMakeBuildCommandLine>
		<NMakeReBuildCommandLine>${xml:project.get_rebuild_command(b)}</NMakeReBuildCommandLine>
		<NMakeCleanCommandLine>${xml:project.get_clean_command(b)}</NMakeCleanCommandLine>
		<NMakeIncludeSearchPath>${xml:b.includes_search_path}</NMakeIncludeSearchPath>
		<NMakePreprocessorDefinitions>${xml:b.preprocessor_definitions};$(NMakePreprocessorDefinitions)</NMakePreprocessorDefinitions>
		<IncludePath>${xml:b.includes_search_path}</IncludePath>
		${if getattr(b, 'output_file', None)}
		<NMakeOutput>${xml:b.output_file}</NMakeOutput>
		<ExecutablePath>${xml:b.output_file}</ExecutablePath>
		${endif}
		${if not getattr(b, 'output_file', None)}
		<NMakeOutput>not_supported</NMakeOutput>
		<ExecutablePath>not_supported</ExecutablePath>
		${endif}
		${if getattr(b, 'deploy_dir', None)}
		<RemoteRoot>${xml:b.deploy_dir}</RemoteRoot>
		${endif}		
		${if b.platform == 'Durango'}
		<OutDir>${xml:project.get_output_folder('durango')}</OutDir>
		<LayoutDir>${xml:project.get_output_folder('durango')}</LayoutDir>
		<LayoutExtensionFilter>*.ilk;*.exp;*.lib;*.winmd;*.appxrecipe;*.pri</LayoutExtensionFilter>
		${if getattr(b, 'output_file_name', None)}
		<TargetName>${b.output_file_name}</TargetName>
		${endif}	
		${endif}
		${if b.platform == 'ORBIS'}
		<OutDir>${xml:project.get_output_folder('orbis')}</OutDir>
		${endif}
		${if b.platform == 'CppCheck'}
		<OutDir>${xml:project.get_output_folder('orbis')}</OutDir>
		${endif}
		${if b.platform == 'Linux X64 GCC'}
		<OutDir>${xml:project.get_output_folder('linux_x64_gcc')}</OutDir>
		${endif}
	</PropertyGroup>
	${endfor}
	
	${for b in project.build_properties}
		${if getattr(b, 'deploy_dir', None)}
	<ItemDefinitionGroup Condition="'$(Configuration)|$(Platform)'=='${b.configuration}|${b.platform}'">
		<Deploy>
			<DeploymentType>CopyToHardDrive</DeploymentType>
		</Deploy>
	</ItemDefinitionGroup>
		${endif}
	${endfor}

	<ItemGroup>
		${for x in project.source}
		<${project.get_key(x)} Include='${x.abspath()}' />
		${endfor}
	
		${for idx, x in enumerate(project.waf_command_override_files)}
		<${project.get_key(x)} Include='${x.abspath()}'>
			<WAF_CommandOverride>${project.exec_waf_command[idx][1]}</WAF_CommandOverride>
		</${project.get_key(x)}>
		${endfor}
		
	</ItemGroup>	

	<Import Project="$(VCTargetsPath)\Microsoft.Cpp.targets" />
	<Import Project="$(MSBuildProjectDirectory)\..\..\_WAF_\msbuild\waf_build.targets" />		
	
	<ImportGroup Label="ExtensionTargets">
	</ImportGroup>
</Project>
'''

PROJECT_USER_TEMPLATE = '''<?xml version="1.0" encoding="UTF-8"?>
<Project ToolsVersion="4.0" xmlns="http://schemas.microsoft.com/developer/msbuild/2003">
	<!-- Setup Debugger working dir for Orbis debugger -->
	${for b in project.build_properties}
	${if b.platform == 'ORBIS'}
	${if b.game_project != ''}
	<PropertyGroup Condition="'$(Configuration)|$(Platform)'=='${b.configuration}|${b.platform}'">
	  <LocalDebuggerWorkingDirectory>${xml:project.get_output_folder('orbis')}</LocalDebuggerWorkingDirectory>
		<LocalDebuggerCommandArguments>-root=${xml:project.ctx.project_orbis_data_folder(b.game_project)}</LocalDebuggerCommandArguments>
	</PropertyGroup>
	${endif}
	${endif}
	${endfor}
</Project>
'''

FILTER_TEMPLATE = '''<?xml version="1.0" encoding="UTF-8"?>
<Project ToolsVersion="4.0" xmlns="http://schemas.microsoft.com/developer/msbuild/2003">
	<ItemGroup>
		${for x in project.source}
			<${project.get_key(x)} Include="${x.abspath()}">
			${if project.get_filter_name(x) != '.'}
				<Filter>${project.get_filter_name(x)}</Filter>
			${endif}
			</${project.get_key(x)}>
		${endfor}
		
		${for x in project.waf_command_override_files}
			<${project.get_key(x)} Include="${x.abspath()}">
			${if project.get_filter_name(x) != '.'}
				<Filter>${project.get_filter_name(x)}</Filter>
			${endif}
			</${project.get_key(x)}>
		${endfor}
	</ItemGroup>
	<ItemGroup>
		${for x in project.dirs()}
			<Filter Include="${x}">
				<UniqueIdentifier>{${project.make_uuid(x)}}</UniqueIdentifier>
			</Filter>
		${endfor}
	</ItemGroup>
</Project>
'''

PROJECT_2008_TEMPLATE = r'''<?xml version="1.0" encoding="UTF-8"?>
<VisualStudioProject ProjectType="Visual C++" Version="9,00"
	Name="${xml: project.name}" ProjectGUID="{${project.uuid}}"
	Keyword="MakeFileProj"
	TargetFrameworkVersion="196613">
	<Platforms>
		${if project.build_properties}
		${for b in project.build_properties}
		   <Platform Name="${xml: b.platform}" />
		${endfor}
		${else}
		   <Platform Name="Win32" />
		${endif}
	</Platforms>
	<ToolFiles>
	</ToolFiles>
	<Configurations>
		${if project.build_properties}
		${for b in project.build_properties}
		<Configuration
			Name="${xml: b.configuration}|${xml: b.platform}"
			IntermediateDirectory="$ConfigurationName"
			OutputDirectory="${xml: b.outdir}"
			ConfigurationType="0">
			<Tool
				Name="VCNMakeTool"
				BuildCommandLine="${xml: project.get_build_command(b)}"
				ReBuildCommandLine="${xml: project.get_rebuild_command(b)}"
				CleanCommandLine="${xml: project.get_clean_command(b)}"
				${if getattr(b, 'output_file', None)}
				Output="${xml: b.output_file}"
				${endif}
				PreprocessorDefinitions="${xml: b.preprocessor_definitions}"
				IncludeSearchPath="${xml: b.includes_search_path}"
				ForcedIncludes=""
				ForcedUsingAssemblies=""
				AssemblySearchPath=""
				CompileAsManaged=""
			/>
		</Configuration>
		${endfor}
		${else}
			<Configuration Name="Release|Win32" >
		</Configuration>
		${endif}
	</Configurations>
	<References>
	</References>
	<Files>
${project.display_filter()}
	</Files>
</VisualStudioProject>
'''

# Note:
# Please mirror any changes in the solution template in the msvs_override_handling.py | get_solution_overrides()
# This also include format changes such as spaces

SOLUTION_TEMPLATE = '''Microsoft Visual Studio Solution File, Format Version ${project.numver}
# Visual Studio ${project.vsver}
${for p in project.all_projects}
Project("{${p.ptype()}}") = "${p.name}", "${p.title}", "{${p.uuid}}"
EndProject${endfor}
Global
	GlobalSection(SolutionConfigurationPlatforms) = preSolution
		${if project.all_projects}
		${for (configuration, platform) in project.all_projects[0].ctx.project_configurations()}
		${configuration}|${platform} = ${configuration}|${platform}
		${endfor}
		${endif}
	EndGlobalSection
	GlobalSection(ProjectConfigurationPlatforms) = postSolution
		${for p in project.all_projects}
			${if hasattr(p, 'source')}
			${for b in p.build_properties}
				{${p.uuid}}.${b.configuration}|${b.platform}.ActiveCfg = ${b.configuration}|${b.platform}
				${if getattr(p, 'is_active', None)}
					{${p.uuid}}.${b.configuration}|${b.platform}.Build.0 = ${b.configuration}|${b.platform}
				${endif}
				${if getattr(p, 'is_deploy', None) and b.platform.lower() in p.is_deploy}
				{${p.uuid}}.${b.configuration}|${b.platform}.Deploy.0 = ${b.configuration}|${b.platform}
				${endif}
			${endfor}
			${endif}
		${endfor}
	EndGlobalSection
	GlobalSection(SolutionProperties) = preSolution
		HideSolutionNode = FALSE
	EndGlobalSection
	GlobalSection(NestedProjects) = preSolution
	${for p in project.all_projects}
		${if p.parent}
		{${p.uuid}} = {${p.parent.uuid}}		
		${endif}
	${endfor}
	EndGlobalSection
EndGlobal
'''

RECODE_TEMPLATE = r'''<?xml version="1.0" encoding="UTF-8"?>
<Project ToolsVersion="4.0" xmlns="http://schemas.microsoft.com/developer/msbuild/2003">
	<PropertyGroup>
		<RecodeBaseDir>${project.get_recode_base_dir()}</RecodeBaseDir>
		<RecodeGlobalVarExclude>${project.get_recode_global_var_exclude_list()}</RecodeGlobalVarExclude>
		<RecodeBaseDirExclusions>..\Code\SDKs;..\Code\Sandbox\SDKs</RecodeBaseDirExclusions>
		<RecodeDemunch>true</RecodeDemunch>
	</PropertyGroup>
</Project>
'''

RECODE_LICENCE_TEMPLATE = r'''
${project.get_recode_license_key()}
'''

PROPERTY_SHEET_TEMPLATE = r'''<?xml version="1.0" encoding="UTF-8"?>
<Project DefaultTargets="Build" ToolsVersion="4.0"
	xmlns="http://schemas.microsoft.com/developer/msbuild/2003">
	
	${for b in project.build_properties}	
	${if getattr(b, 'output_file_name', None)}
	<PropertyGroup Condition="'$(Configuration)|$(Platform)'=='${b.configuration}|${b.platform}'">		
		<WAF_TargetFile>${xml:b.output_file}</WAF_TargetFile>		
    <TargetPath Condition="'$(WAF_TargetFile)' != ''">$(WAF_TargetFile)</TargetPath>
    <LocalDebuggerCommand Condition="'$(LocalDebuggerCommand)'==''">$(TargetPath)</LocalDebuggerCommand>		
  </PropertyGroup>
	${endif}
	${endfor}
	
	${for b in project.build_properties}	
	<ItemDefinitionGroup Condition="'$(Configuration)|$(Platform)'=='${b.configuration}|${b.platform}'">
		<ClCompile>
		
			<WAF_SingleCompilationMode>Code</WAF_SingleCompilationMode>
			<WAF_ExecPath>${xml:project.get_waf()}</WAF_ExecPath>
			<WAF_TargetSolution>${xml:project.ctx.get_solution_node().abspath()} </WAF_TargetSolution>
			
			${if getattr(b, 'target_spec', None)}
			<WAF_TargetSpec>${xml:b.target_spec}</WAF_TargetSpec>
			${endif}		
			
			${if getattr(b, 'target_config', None)}
			<WAF_TargetConfig>${xml:b.target_config}</WAF_TargetConfig>
			${endif}		
		
			${if getattr(b, 'output_file_name', None)}
			<WAF_TargetName>${xml:b.output_file_name}</WAF_TargetName>
			${endif}
			
			${if getattr(b, 'output_file', None)}
			<WAF_TargetFile>${xml:b.output_file} </WAF_TargetFile>
			${endif}
			
			${if getattr(b, 'includes_search_path', None)}
			<WAF_IncludeDirectories>${xml:b.includes_search_path}</WAF_IncludeDirectories>
			${endif}
			
			${if getattr(b, 'preprocessor_definitions', None)}
			<WAF_PreprocessorDefinitions>${xml:b.preprocessor_definitions}</WAF_PreprocessorDefinitions>
			${endif}
			
			${if getattr(b, 'deploy_dir', None)}
			<WAF_DeployDir>${xml:b.deploy_dir}</WAF_DeployDir>
			${endif}
			
			${if getattr(b, 'output_dir', None)}
			<WAF_OutputDir>${xml:b.output_dir}</WAF_OutputDir>
			${endif}
			
			${if getattr(b, 'layout_dir', None)}
			<LayoutDir>${xml:b.deploy_dir}</LayoutDir>
			${endif}
			
			${if getattr(b, 'layout_extension_filter', None)}
			<LayoutExtensionFilter>${xml:b.deploy_dir}</LayoutExtensionFilter>
			${endif}
						
			${if getattr(b, 'c_flags', None)}
			<WAF_CompilerOptions_C>${xml:b.c_flags} </WAF_CompilerOptions_C>
			${endif}
			
			${if getattr(b, 'cxx_flags', None)}
			<WAF_CompilerOptions_CXX>${xml:b.cxx_flags} </WAF_CompilerOptions_CXX>
			${endif}
			
			${if getattr(b, 'link_flags', None)}
			<WAF_LinkerOptions>${xml:b.link_flags}</WAF_LinkerOptions>
			${endif}
			
			<WAF_DisableCompilerOptimization>false</WAF_DisableCompilerOptimization>
			<WAF_ExcludeFromUberFile>false</WAF_ExcludeFromUberFile>

			<WAF_BuildCommandLine>${xml:project.get_build_command(b)}</WAF_BuildCommandLine>
			<WAF_RebuildCommandLine>${xml:project.get_rebuild_command(b)}</WAF_RebuildCommandLine>
			<WAF_CleanCommandLine>${xml:project.get_clean_command(b)}</WAF_CleanCommandLine>

		</ClCompile>
	</ItemDefinitionGroup>
	
	<ItemDefinitionGroup Condition="'$(Configuration)|$(Platform)'=='${b.configuration}|${b.platform}'">
		<Deploy>
			<DeploymentType>CopyToHardDrive</DeploymentType>
		</Deploy>
	</ItemDefinitionGroup>

	${endfor}
		
	<ItemDefinitionGroup />		
	<ItemGroup />
	
</Project>
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

@conf
def convert_waf_platform_to_vs_platform(self, platform):	
	if platform == 'win_x86':
		return 'Win32'
	if platform == 'win_x64':
		return 'x64'
		
	if platform == 'durango':
		return 'Durango'
	if platform == 'orbis':
		return 'ORBIS'
		
	if platform == 'linux_x64_gcc':
		return 'Linux X64 GCC'
	if platform == 'linux_x64_clang':
		return 'Linux X64 CLANG'
	
	if platform == 'cppcheck':
		return 'CppCheck'
		
	print('to_vs error ' + platform)
	return 'UNKNOWN'
	
@conf
def convert_vs_platform_to_waf_platform(self, platform):
	if platform == 'Win32':
		return 'win_x86'
	if platform == 'x64':
		return 'win_x64'
	
	if platform == 'Durango':
		return 'durango'
	if platform == 'ORBIS':
		return 'orbis'
		
	if platform == 'Linux X64 GCC':
		return 'linux_x64_gcc'
	
	if platform == 'Linux X64 CLANG':
		return 'linux_x64_clang'
	
	if platform == 'CppCheck':
		return 'cppcheck'
		
	print('to_waf error ' + platform)
	return 'UNKNOWN'
	
@conf
def convert_waf_configuration_to_vs_configuration(self, configuration):
	configuration = configuration.replace('debug', 		'Debug')
	configuration = configuration.replace('profile',	'Profile')
	configuration = configuration.replace('release', 	'Release')
	configuration = configuration.replace('performance', 	'Performance')
	return configuration

@conf
def convert_vs_configuration_to_waf_configuration(self, configuration):
	configuration = configuration.replace('Debug', 'debug')
	configuration = configuration.replace('Profile', 'profile')
	configuration = configuration.replace('Release', 'release')
	configuration = configuration.replace('Performance', 'performance')
	return configuration
	
@conf
def is_valid_build_target(ctx, target, waf_spec, waf_configuration,  waf_platform):
	bIncludeProject = True
	
	if not target in ctx.spec_modules(waf_spec):
		bIncludeProject = False
	if not waf_configuration in ctx.spec_valid_configurations(waf_spec):
		bIncludeProject = False
	if not any(i in ctx.get_platform_list(waf_platform) for i in ctx.spec_valid_platforms(waf_spec)):
		bIncludeProject = False
		
	# For projects which are not includes, check if it is a static module which is used by another module
	# Those are added bypassing the valid check in the waf spec
	if not bIncludeProject and hasattr(ctx, 'cry_module_users'):
		for user in ctx.cry_module_users.get(target, []):
			if user in ctx.spec_modules(waf_spec):
				bIncludeProject = True
		
	return bIncludeProject
 
def is_valid_spec(ctx, spec_name):
	""" Check if the spec should be included when generating visual studio projects """
	if ctx.options.specs_to_include_in_project_generation == '':
		return True
		
	allowed_specs = ctx.options.specs_to_include_in_project_generation.replace(' ', '').split(',')
	if spec_name in allowed_specs:
		return True
	
	return False
	
def is_valid_configuration(ctx, spec_name, configuration):
	""" Check if the current configuratation is supported in a spec """
	return configuration in ctx.spec_valid_configurations(spec_name)
		
@conf
def convert_waf_spec_to_vs_spec(ctx, spec_name):
	""" Helper to get the Visual Studio Spec name from a WAF spec name """
	return ctx.spec_visual_studio_name(spec_name)

@conf
def convert_vs_configuration_to_waf_configuration(self, configuration):
	if 'Debug' in configuration:
		return 'debug'
	if 'Profile' in configuration:
		return 'profile'
	if 'Release' in configuration:
		return 'release'
	if 'Performance' in configuration:
		return 'performance'
			
	return ''
	
@conf
def convert_vs_spec_to_waf_spec(ctx, spec):
	""" Convert a Visual Studio spec into a WAF spec """
	# Split string "[spec] config" -> "spec"
	specToTest = spec.split("[")[1].split("]")[0]			

	# Check VS spec name against all specs
	loaded_specs = ctx.loaded_specs()
	for spec_name in loaded_specs:
		if specToTest == ctx.spec_visual_studio_name(spec_name):	
			return spec_name
		
	Logs.warn('Cannot find WAF spec for "%s"' % spec)
	return ''

def create_msbuild_platform_folder(self, platform_name):
		# Hardcode MSBuild install path as there appears to be no environment/registry variable pointing to the path
		# Microsoft is not really well documented, I tried to use the Registry to find the install path for MSBuild, but this yielded:
		# $(MSBuildExtensionsPath32)\Microsoft.Cpp\v4.0\  ... where MSBuildExtensionPath32 is not defined in registry or as an env variable
		msbuild_folder_path = 'C:/Program Files (x86)/MSBuild/Microsoft.Cpp/v4.0/Platforms/' + platform_name
		try:			
			os.makedirs(msbuild_folder_path)
		except OSError as exception:
			if exception.errno != errno.EEXIST:
				self.fatal('Error: cry_waf.exe executed without "ADMIN RIGHTS". Unable to create folder "%s". Without the pysical "empty" folder, ' 
				'Visual Studio might encounter errors while parsing the MSBuild targets leading to a limited Visual Studio feature set being available.' % msbuild_folder_path)
						
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


re_blank = re.compile('(\n|\r|\\s)*\n', re.M)
def rm_blank_lines(txt):
	txt = re_blank.sub('\r\n', txt)
	return txt

BOM = '\xef\xbb\xbf'
try:
	BOM = bytes(BOM, 'iso8859-1') # python 3
except:
	pass

def stealth_write(self, data, flags='wb'):
	try:
		x = unicode
	except:
		data = data.encode('utf-8') # python 3
	else:
		data = data.decode(sys.getfilesystemencoding(), 'replace')
		data = data.encode('utf-8')

	if self.name.endswith('.vcproj') or self.name.endswith('.vcxproj'):
		data = BOM + data

	try:
		txt = self.read(flags='rb')
		if txt != data:
			raise ValueError('must write')
	except (IOError, ValueError):
		self.write(data, flags=flags)
	else:
		Logs.debug('msvs: skipping %s' % self.abspath())
Node.Node.stealth_write = stealth_write

re_quote = re.compile("[^a-zA-Z0-9-.]")
def quote(s):
	return re_quote.sub("_", s)

def xml_escape(value):
	return value.replace("&", "&amp;").replace('"', "&quot;").replace("'", "&apos;").replace("<", "&lt;").replace(">", "&gt;")

def make_uuid(v, prefix = None):
	"""
	simple utility function
	"""
	if isinstance(v, dict):
		keys = list(v.keys())
		keys.sort()
		tmp = str([(k, v[k]) for k in keys])
	else:
		tmp = str(v)
	d = Utils.md5(tmp.encode()).hexdigest().upper()
	if prefix:
		d = '%s%s' % (prefix, d[8:])
	gid = uuid.UUID(d, version = 4)
	return str(gid).upper()

def diff(node, fromnode):
	# difference between two nodes, but with "(..)" instead of ".."
	c1 = node
	c2 = fromnode

	c1h = c1.height()
	c2h = c2.height()

	lst = []
	up = 0

	while c1h > c2h:
		lst.append(c1.name)
		c1 = c1.parent
		c1h -= 1

	while c2h > c1h:
		up += 1
		c2 = c2.parent
		c2h -= 1

	while id(c1) != id(c2):
		lst.append(c1.name)
		up += 1

		c1 = c1.parent
		c2 = c2.parent

	for i in range(up):
		lst.append('(..)')
	lst.reverse()
	return tuple(lst)

class build_property(object):
	pass

class vsnode(object):
	"""
	Abstract class representing visual studio elements
	We assume that all visual studio nodes have a uuid and a parent
	"""
	def __init__(self, ctx):
		self.ctx = ctx # msvs context
		self.name = '' # string, mandatory
		self.vspath = '' # path in visual studio (name for dirs, absolute path for projects)
		self.uuid = '' # string, mandatory
		self.parent = None # parent node for visual studio nesting

	def get_waf(self):
		"""
		Override in subclasses...
		"""
		return 'cd /d "%s" & %s' % (self.ctx.srcnode.abspath(), getattr(self.ctx, 'waf_command', 'cry_waf.exe'))

	def ptype(self):
		"""
		Return a special uuid for projects written in the solution file
		"""
		pass

	def write(self):
		"""
		Write the project file, by default, do nothing
		"""
		pass

	def make_uuid(self, val):
		"""
		Alias for creating uuid values easily (the templates cannot access global variables)
		"""
		return make_uuid(val)

class vsnode_vsdir(vsnode):
	"""
	Nodes representing visual studio folders (which do not match the filesystem tree!)
	"""
	VS_GUID_SOLUTIONFOLDER = "2150E333-8FDC-42A3-9474-1A3956D46DE8"
	def __init__(self, ctx, uuid, name, vspath=''):
		vsnode.__init__(self, ctx)
		self.title = self.name = name
		self.uuid = uuid
		self.vspath = vspath or name

	def ptype(self):
		return self.VS_GUID_SOLUTIONFOLDER

def _get_filter_name(project_filter_dict, file_name):	
		""" Helper function to get a correct project filter """
		if file_name.endswith('wscript') or file_name.endswith('.waf_files'):
			return '_WAF_' # Special case for wscript files			

		if not file_name in project_filter_dict:
			return 'FILE_NOT_FOUND'
			
		project_filter = project_filter_dict[file_name]		
		project_filter = project_filter.replace('/', '\\')
		if project_filter.lower() == 'root':
			return '.'
			
		return project_filter
			
class vsnode_project(vsnode):
	"""
	Abstract class representing visual studio project elements
	A project is assumed to be writable, and has a node representing the file to write to
	"""
	VS_GUID_VCPROJ = "8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942"
	def ptype(self):
		return self.VS_GUID_VCPROJ

	def __init__(self, ctx, node):
		vsnode.__init__(self, ctx)
		self.path = node
		self.uuid = make_uuid(node.abspath())
		self.name = node.name
		self.title = self.path.abspath()
		self.source = [] # list of node objects
		self.build_properties = [] # list of properties (nmake commands, output dir, etc)
		self.waf_command_override_files = []

	def dirs(self):
		"""
		Get the list of parent folders of the source files (header files included)
		for writing the filters
		"""
		lst = []
		
		def add(x):		
			if not x.lower() == '.':
				lst.append(x)
			
			seperator_pos = x.rfind('\\')
			if seperator_pos != -1:
				add(x[:seperator_pos])
				
		
		for source in self.source:		
			add( _get_filter_name(self.project_filter, source.abspath()) )
	
		for option_file in self.waf_command_override_files:
			add( _get_filter_name(self.project_filter, option_file.abspath()) )
			
		# Remove duplicates
		lst = list(set(lst))
		return lst
		
	def write(self):
		Logs.debug('msvs: creating %r' % self.path)

		def _write(template_name, output_node):			
			template = compile_template(template_name)
			template_str = template(self)
			template_str = rm_blank_lines(template_str)			
			output_node.stealth_write(template_str)
		
		_write(PROJECT_TEMPLATE, self.path )
		_write(FILTER_TEMPLATE, self.path.parent.make_node(self.path.name + '.filters') )
		_write(PROJECT_USER_TEMPLATE, self.path.parent.make_node(self.path.name + '.user') )		
		_write(PROPERTY_SHEET_TEMPLATE, self.path.parent.make_node(self.path.name + '.default.props') )
		
	def get_key(self, node):
		"""
		required for writing the source files
		"""
		name = node.name
		if name.endswith('.cpp') or name.endswith('.c'):
			return 'ClCompile'
		return 'ClInclude'
		
	def collect_properties(self):
		"""
		Returns a list of triplet (configuration, platform, output_directory)
		"""
		ret = []
		for c in self.ctx.configurations:
			for p in self.ctx.platforms:
				x = build_property()
				x.outdir = ''

				x.configuration = c
				x.platform = p
				waf_spec = self.ctx.convert_vs_spec_to_waf_spec(c)
				active_projects = self.ctx.spec_game_projects( waf_spec )
				if len(active_projects) != 1:
					x.game_project = ''
				else:
					x.game_project = active_projects[0]

				x.preprocessor_definitions = ''
				x.includes_search_path = ''
				
				# can specify "deploy_dir" too
				ret.append(x)
		self.build_properties = ret

	def get_build_params(self, props):
		opt = '--execsolution="%s"' % self.ctx.get_solution_node().abspath()
		return (self.get_waf(), opt)

	def get_build_command(self, props):		
		params = self.get_build_params(props)
		return "%s build_" % params[0] + self.ctx.convert_vs_platform_to_waf_platform(props.platform) + '_' + self.ctx.convert_vs_configuration_to_waf_configuration(props.configuration) + ' --project-spec ' + self.ctx.convert_vs_spec_to_waf_spec(props.configuration) + " %s" % params[1]		

	def get_clean_command(self, props):
		params = self.get_build_params(props)
		return "%s clean_" % params[0] + self.ctx.convert_vs_platform_to_waf_platform(props.platform) + '_' + self.ctx.convert_vs_configuration_to_waf_configuration(props.configuration) + ' --project-spec ' + self.ctx.convert_vs_spec_to_waf_spec(props.configuration) + " %s" % params[1]		
			
	def get_output_folder(self,platform):
		output_folders = self.ctx.get_output_folders(platform)
		if len(output_folders) == 0:
			return ''
			
		output_path = output_folders[0]
		root_path = self.ctx.path		
		relative_path = output_path.path_from(self.ctx.path)

		return '$(SolutionDir)..\\' + relative_path + '\\'
		
	def get_rebuild_command(self, props):			
		params = self.get_build_params(props)
		return 	"%s" % params[0] + \
				" clean_" + self.ctx.convert_vs_platform_to_waf_platform(props.platform) + '_' + self.ctx.convert_vs_configuration_to_waf_configuration(props.configuration) + \
				" build_" + self.ctx.convert_vs_platform_to_waf_platform(props.platform) + '_' + self.ctx.convert_vs_configuration_to_waf_configuration(props.configuration) + \
				' --project-spec ' + self.ctx.convert_vs_spec_to_waf_spec(props.configuration) +\
				" %s" % params[1]		

	def get_filter_name(self, node):
		return _get_filter_name(self.project_filter, node.abspath())

class vsnode_alias(vsnode_project):
	def __init__(self, ctx, node, name):
		vsnode_project.__init__(self, ctx, node)
		self.name = name
		self.output_file = ''		
		self.project_filter = {}

class vsnode_build_all(vsnode_alias):
	"""
	Fake target used to emulate the behaviour of "make all" (starting one process by target is slow)
	This is the only alias enabled by default
	"""
	def __init__(self, ctx, node, name='_WAF_'):
		vsnode_alias.__init__(self, ctx, node, name)
		self.is_active = True
		
	def collect_properties(self):
		"""
		Visual studio projects are associated with platforms and configurations (for building especially)
		"""
		super(vsnode_build_all, self).collect_properties()	

		for x in self.build_properties:
			x.outdir = self.path.parent.abspath()
			x.preprocessor_definitions = ''
			x.includes_search_path = ''	
			x.c_flags = ''
			x.cxx_flags = ''
			x.link_flags = ''
			x.target_spec = ''
			x.target_config = ''
		
			if x.platform == 'CppCheck':
				continue			
			
			waf_spec = self.ctx.convert_vs_spec_to_waf_spec(x.configuration)
			waf_platform = self.ctx.convert_vs_platform_to_waf_platform(x.platform)
			waf_configuration = self.ctx.convert_vs_configuration_to_waf_configuration(x.configuration)			
			
			x.target_spec = waf_spec
			x.target_config = waf_platform + '_' + waf_configuration
			
		# Collect WAF files
		waf_data_dir 			= self.ctx.root.make_node(Context.launch_dir).make_node('_WAF_')
		waf_source_dir		= self.ctx.root.make_node(Context.launch_dir).make_node('Code/Tools/waf-1.7.13')
		
		waf_config_files	= waf_data_dir.ant_glob('**/*', maxdepth=0)		
		waf_spec_files 		= waf_data_dir.make_node('specs').ant_glob('**/*')	
		
		waf_scripts					= waf_source_dir.make_node('waflib').ant_glob('**/*.py')
		waf_crytek_scripts	= waf_source_dir.make_node('crywaflib').ant_glob('**/*.py')				
		
		# Remove files found in crywaflib from waf scripts
		tmp = []
		waf_crytek_scripts_files = []
		for node in waf_crytek_scripts:
			waf_crytek_scripts_files += [os.path.basename(node.abspath())]
			
		for x in waf_scripts:
			if os.path.basename(x.abspath()) in waf_crytek_scripts_files:
				continue
			tmp += [x]
		waf_scripts = tmp	
		
		for file in waf_config_files:
			self.project_filter[file.abspath()] = 'Settings'
		for file in waf_spec_files:
			self.project_filter[file.abspath()] = 'Specs'
		
		for file in waf_scripts:
			filter_name = 'Scripts'
			subdir = os.path.dirname( file.path_from( waf_source_dir.make_node('waflib') ) )			
			if subdir != '':
				filter_name += '/' + subdir
			self.project_filter[file.abspath()] = filter_name
			
		for file in waf_crytek_scripts:
			self.project_filter[file.abspath()] = 'Crytek Scripts'	
			
		self.source += waf_config_files + waf_spec_files + waf_scripts + waf_crytek_scripts
		
		# without a cpp file, VS wont compile this project
		dummy_node = self.ctx.get_bintemp_folder_node().make_node('__waf_compile_dummy__.cpp')
		self.project_filter[dummy_node.abspath()] = 'DummyCompileNode'
		self.source += [ dummy_node ]
	
		# Waf commands		
		self.exec_waf_command = [
		('show_gui', 'utilities'),
		('configure', 'show_option_dialog'),
		('generate_uber_files', 'generate_uber_files'),
		('generate_solution', 'msvs')
		]
		
		for command in self.exec_waf_command:
			executable_command = self.ctx.get_bintemp_folder_node().make_node('_waf_' + command[0] + '_.cpp')
			self.project_filter[executable_command.abspath()] = '_WAF Commands'
			self.waf_command_override_files += [ executable_command ]

class vsnode_install_all(vsnode_alias):
	"""
	Fake target used to emulate the behaviour of "make install"
	"""
	def __init__(self, ctx, node, name='install_all_projects'):
		vsnode_alias.__init__(self, ctx, node, name)

	def get_build_command(self, props):
		return "%s build install %s" % self.get_build_params(props)

	def get_clean_command(self, props):
		return "%s clean %s" % self.get_build_params(props)

	def get_rebuild_command(self, props):
		return "%s clean build install %s" % self.get_build_params(props)

class vsnode_project_view(vsnode_alias):
	"""
	Fake target used to emulate a file system view
	"""
	def __init__(self, ctx, node, name='project_view'):
		vsnode_alias.__init__(self, ctx, node, name)
		self.tg = self.ctx() # fake one, cannot remove
		self.exclude_files = Node.exclude_regs + '''
waf-1.7.*
waf3-1.7.*/**
.waf-1.7.*
.waf3-1.7.*/**
**/*.sdf
**/*.suo
**/*.ncb
**/%s
		''' % Options.lockfile

	def collect_source(self):
		# this is likely to be slow
		self.source = self.ctx.srcnode.ant_glob('**', excl=self.exclude_files)

	def get_build_command(self, props):
		params = self.get_build_params(props) + (self.ctx.cmd,)
		return "%s %s %s" % params

	def get_clean_command(self, props):
		return ""

	def get_rebuild_command(self, props):
		return self.get_build_command(props)

class vsnode_target(vsnode_project):
	"""
	Visual studio project representing a targets (programs, libraries, etc) and bound
	to a task generator
	"""
	def __init__(self, ctx, tg):
		"""
		A project is more or less equivalent to a file/folder
		"""
		base = getattr(ctx, 'projects_dir', None) or tg.path
		node = base.make_node(quote(tg.name) + ctx.project_extension) # the project file as a Node
		vsnode_project.__init__(self, ctx, node)
		self.name = quote(tg.name)
		self.tg     = tg  # task generator
		if getattr(tg, 'need_deploy', None):
			if not isinstance(tg.need_deploy, list):
				self.is_deploy = [ tg.need_deploy ]
			else:
				self.is_deploy = tg.need_deploy
		self.project_filter = self.tg.project_filter

	def get_build_params(self, props):
		"""
		Override the default to add the target name
		"""
		opt = '--execsolution="%s"' % self.ctx.get_solution_node().abspath()
		if getattr(self, 'tg', None):
			opt += " --targets=%s" % self.tg.name
		return (self.get_waf(), opt)

	def collect_source(self):
		tg = self.tg
		source_files = tg.to_nodes(getattr(tg, 'source', []))
		include_dirs = Utils.to_list(getattr(tg, 'msvs_includes', []))
		waf_file_entries = self.tg.waf_file_entries;
		
		include_files = []
		""""
		for x in include_dirs:
			if isinstance(x, str):
				x = tg.path.find_node(x)
			if x:
				lst = [y for y in x.ant_glob(HEADERS_GLOB, flat=False)]
				include_files.extend(lst)
		"""
		# remove duplicates
		self.source.extend(list(set(source_files + include_files)))
		self.source += [ tg.path.make_node('wscript') ] + waf_file_entries
		self.source.sort(key=lambda x: x.abspath())
		
	def ConvertToDict(self, var):
		# dict type
		if isinstance(var, dict):
			return dict(var)
			
		# list type
		if isinstance(var, list):
			return dict(var)

		# class type
		try:
			return dict(var.__dict__)
		except:
			return None	
		
	def GetPlatformSettings(self, target_platform, target_configuration, name, settings):
		"""
		Util function to apply flags based on current platform
		"""		
		
		result = []
		platforms = [target_platform]		
		# Append common win platform for windows hosts
		if target_platform == 'win_x86' or target_platform == 'win_x64':
			platforms.append('win')
		if target_platform == 'linux_x86_gcc' or target_platform == 'linux_x64_gcc' or target_platform == 'linux_x86_clang' or target_platform == 'linux_x64_clang':
			platforms.append('linux')
		if target_platform == 'linux_x86_gcc' or target_platform == 'linux_x86_clang':
			platforms.append('linux_x86')
		if target_platform == 'linux_x64_gcc' or target_platform == 'linux_x64_clang':
			platforms.append('linux_x64')
		if target_platform == 'darwin_x86' or target_platform == 'darwin_x64':
			platforms.append('darwin')	
		
		settings_dict = self.ConvertToDict(settings)
		
		if not settings_dict:
			print ("WAF_ERROR: Unsupported type '%s' for 'settings' variable encountered." % type(settings))
			return
					
		# add non platform specific settings
		try:	
			if isinstance(settings_dict[name],list):
				result += settings_dict[name]
			else:
			  result += [settings_dict[name]]
		except:
			pass				
		
		# add per configuration flags	
		configuration_specific_name = (name + '_' + target_configuration)
		try:	
			if isinstance(settings_dict[configuration_specific_name],list):
				result += settings_dict[configuration_specific_name]
			else:
			  result += [settings_dict[configuration_specific_name]]
		except:
			pass

		# add per platform flags	
		for platform in platforms:
			platform_specific_name = (name + '_' + platform)					
			try:	
				if isinstance(settings_dict[platform_specific_name],list):
					result += settings_dict[platform_specific_name]
				else:
					result += [settings_dict[platform_specific_name]]
			except:
				pass
				
		# add per platform_configuration flags	
		for platform in platforms:
			platform_configuration_specific_name = (name + '_' + platform + '_' + target_configuration)
			try:	
				if isinstance(settings_dict[platform_configuration_specific_name],list):
					result += settings_dict[platform_configuration_specific_name]
				else:
					result += [settings_dict[platform_configuration_specific_name]]
			except:
				pass					
	
		return result		
	
	def collect_properties(self):
		"""
		Visual studio projects are associated with platforms and configurations (for building especially)
		"""
		super(vsnode_target, self).collect_properties()	
		target = self.tg.target
		
		for x in self.build_properties:		
			x.outdir = self.path.parent.abspath()
			x.preprocessor_definitions = ''
			x.includes_search_path = ''	
			x.c_flags = ''
			x.cxx_flags = ''
			x.link_flags = ''
			x.target_spec = ''
			x.target_config = ''

			if x.platform == 'CppCheck':
				continue
			
			if not hasattr(self.tg ,'link_task') and 'create_static_library' not in self.tg.features:
				continue
			
			if True:				
				waf_spec = self.ctx.convert_vs_spec_to_waf_spec(x.configuration)
				waf_platform = self.ctx.convert_vs_platform_to_waf_platform(x.platform)
				waf_configuration = self.ctx.convert_vs_configuration_to_waf_configuration(x.configuration)
										
				# Check if this project is supported for the current spec
				if not self.ctx.is_valid_build_target(target, waf_spec, waf_configuration, waf_platform):
					# Has to be valid file name. Otherwise the solution will not be able to load in VS
					output_file_name = 'Error__Invalid_startup_project__%s__Selected_project_not_supported_for_configuration__%s__' % (self.tg.target.replace(' ', '_'), x.configuration.strip().replace(' ', '_').replace('[', '_').replace(']','_'))
					x.output_file = output_file_name
					x.output_file_name = output_file_name
					x.includes_search_path = ""
					x.preprocessor_definitions = ""
					x.c_flags = ''
					x.cxx_flags = ''
					x.link_flags = ''
					x.target_spec = ''
					continue

				current_env = self.ctx.all_envs[waf_platform + '_' + waf_configuration]
				mscv_helper.verify_options_common(current_env)
				x.c_flags = " ".join(current_env['CFLAGS']) # convert list to space separated string
				x.cxx_flags = " ".join(current_env['CXXFLAGS']) # convert list to space separated string
				x.link_flags = " ".join(current_env['LINKFLAGS']) # convert list to space separated string
				
				x.target_spec = waf_spec
				x.target_config = waf_platform + '_' + waf_configuration
							
				# Get startup project		
				projects = self.ctx.spec_game_projects(waf_spec)

				# For VS projects with a no or single game project target
				if len(projects) <= 1:
					if hasattr(self.tg, 'link_task'): # no link task implies special static modules which are static libraries
						link_task_pattern_name = self.tg.link_task.__class__.__name__ + '_PATTERN'
					else:
						link_task_pattern_name = 'cxxstlib_PATTERN'

					pattern = current_env[link_task_pattern_name]
					
					output_folder_nodes = self.ctx.get_output_folders(waf_platform, waf_spec)
					output_folder_node = output_folder_nodes[0]
					if hasattr(self.tg, 'output_sub_folder'):
						output_folder_node = output_folder_node.make_node(self.tg.output_sub_folder)
					
					output_file_name = self.tg.output_file_name
				  
					# Single game project target
					if len(projects) == 1 and projects[0] != '':		
						project = projects[0]
						x.startup_project = project
						if getattr(self.tg, 'is_launcher', False):
							output_file_name = self.ctx.get_executable_name(project)
						elif getattr(self.tg, 'is_dedicated_server', False):
							output_file_name = self.ctx.get_dedicated_server_executable_name(project)
					
					# Save project info 
					output_file = output_folder_node.make_node(output_file_name)
					x.output_file = pattern % output_file.abspath()					
					x.output_file_name = output_file_name
					x.output_path = os.path.dirname(x.output_file)
					x.assembly_name = output_file_name
					x.output_type = os.path.splitext(x.output_file)[1][1:] # e.g. ".dll" to "dll"
					
				else: # Project with multiple game project targets
					# Save project info 
					output_file_name = 'Error__Invalid_startup_project__%s__Selected_project_has_multiple_targets' % (target.replace(' ', '_'))
					x.output_file = output_file_name
					x.output_file_name = output_file_name
					x.includes_search_path = ""
					x.preprocessor_definitions = ""

				# Collect all defines for this configuration
				define_list =  list(current_env['DEFINES'])

				define_list += self.GetPlatformSettings( waf_platform, waf_configuration, 'defines', self.tg )
				x.preprocessor_definitions = ';'.join(define_list)
								
				include_list = list(current_env['INCLUDES'])
				include_list += self.GetPlatformSettings( waf_platform, waf_configuration, 'includes', self.tg )
				
				# Add Visual Studio Include path to allow working with .rc files (else some headers are not found)
				include_list += [ self.ctx.CreateRootRelativePath('Code/SDKs/Microsoft Windows SDK/V8.0/Include') ]
				include_list += [ self.ctx.CreateRootRelativePath('Code/SDKs/Microsoft Visual Studio Compiler/include') ]
				include_list += [ self.ctx.CreateRootRelativePath('Code/SDKs/Microsoft Visual Studio Compiler/atlmfc/include') ]
				
				# make sure we only have absolute path for intellisense
				# If we don't have a absolute path, assume a relative one, hence prefix the path with the taskgen path and comvert it into an absolute one
				for i in range(len(include_list)):
					if not os.path.isabs( include_list[i] ):
						include_list[i] = os.path.abspath( self.tg.path.abspath() + '/' + include_list[i] )
								
				x.includes_search_path = ';'.join( include_list )
				
class msvs_generator(BuildContext):
	'''generates a visual studio 2012 solution'''
	cmd = 'msvs'
	fun = 'build'

	def init(self):
		"""
		Some data that needs to be present
		"""
		if not getattr(self, 'configurations', None):
			build_configurations = self.get_supported_configurations()
			self.configurations = []
			for spec in self.loaded_specs():
				if not is_valid_spec(self, spec):
					continue
				for conf in build_configurations:
					if not is_valid_configuration(self ,spec, conf):
						continue
					solution_conf_name = '[' + self.convert_waf_spec_to_vs_spec(spec) + '] ' + conf
					solution_conf_name_vs = self.convert_waf_configuration_to_vs_configuration( solution_conf_name )
					self.configurations.append(solution_conf_name_vs)			
		if not getattr(self, 'platforms', None):
			self.platforms = []
			for platform in self.get_supported_platforms():
				self.platforms.append(self.convert_waf_platform_to_vs_platform(platform))
		if not getattr(self, 'all_projects', None):
			self.all_projects = []
		if not getattr(self, 'project_extension', None):
			self.project_extension = '.vcxproj'
		if not getattr(self, 'projects_dir', None):
			self.projects_dir = self.srcnode.make_node('.depproj')
			self.projects_dir.mkdir()

		# bind the classes to the object, so that subclass can provide custom generators
		if not getattr(self, 'vsnode_vsdir', None):
			self.vsnode_vsdir = vsnode_vsdir
		if not getattr(self, 'vsnode_target', None):
			self.vsnode_target = vsnode_target
		if not getattr(self, 'vsnode_build_all', None):
			self.vsnode_build_all = vsnode_build_all
		if not getattr(self, 'vsnode_install_all', None):
			self.vsnode_install_all = vsnode_install_all
		if not getattr(self, 'vsnode_project_view', None):
			self.vsnode_project_view = vsnode_project_view

		self.numver = '12.00'
		self.vsver  = '2012'

		# Create folders to make VS think we have a linux/cpp check target
		create_msbuild_platform_folder(self, 'Linux X64 GCC')
		create_msbuild_platform_folder(self, 'Linux X64 CLANG')
		create_msbuild_platform_folder(self, 'CppCheck')		
	
	def execute(self):
		"""
		Entry point
		"""
		self.restore()
		if not self.all_envs:
			self.load_envs()
			
		self.load_user_settings()
		Logs.info("[WAF] Executing 'msvs' in '%s'" % (self.variant_dir)	)
		self.recurse([self.run_dir])

		# user initialization
		self.init()

		# two phases for creating the solution
		self.collect_projects() # add project objects into "self.all_projects"
		self.write_files() # write the corresponding project and solution files
		self.post_build()

	def collect_projects(self):
		"""
		Fill the list self.all_projects with project objects
		Fill the list of build targets
		"""
		self.collect_targets()
		self.add_aliases()
		self.collect_dirs()
		default_project = getattr(self, 'default_project', None)
		def sortfun(x):
			if x.name == default_project:
				return ''
			return getattr(x, 'path', None) and x.path.abspath() or x.name
		self.all_projects.sort(key=sortfun)

	def write_files(self):
		"""
		Write the project and solution files from the data collected
		so far. It is unlikely that you will want to change this
		"""
		for p in self.all_projects:
			p.write()

		# and finally write the solution file
		node = self.get_solution_node()
		node.parent.mkdir()
		Logs.warn('Creating %r' % node)
		template1 = compile_template(SOLUTION_TEMPLATE)
		sln_str = template1(self)
		sln_str = rm_blank_lines(sln_str)
		node.stealth_write(sln_str)
		
		# Write recode file
		recode_node = node.change_ext('.recode')
		Logs.warn('Creating %r' % recode_node)
		recode_template = compile_template(RECODE_TEMPLATE)				
		recode_str = recode_template(self)
		recode_str = rm_blank_lines(recode_str)
		recode_node.stealth_write(recode_str)
		
		# Write recode licence file
		recode_lic_node = node.parent.make_node('recode.lic')
		Logs.warn('Creating %r' % recode_lic_node)
		recode_lic_template = compile_template(RECODE_LICENCE_TEMPLATE)				
		recode_lic_str = recode_lic_template(self)
		recode_lic_str = rm_blank_lines(recode_lic_str)
		recode_lic_node.stealth_write(recode_lic_str)
		
	def get_solution_node(self):
		"""
		The solution filename is required when writing the .vcproj files
		return self.solution_node and if it does not exist, make one
		"""
		try:
			return self.solution_node
		except:
			pass

		solution_name = getattr(self, 'solution_name', None)
		if not solution_name:
			solution_name = getattr(Context.g_module, Context.APPNAME, 'project') + '.sln'
		if os.path.isabs(solution_name):
			self.solution_node = self.root.make_node(solution_name)
		else:
			self.solution_node = self.srcnode.make_node(solution_name)
		return self.solution_node

	def project_configurations(self):
		"""
		Helper that returns all the pairs (config,platform)
		"""
		ret = []
		for c in self.configurations:
			for p in self.platforms:
				ret.append((c, p))
		return ret

	def collect_targets(self):
		"""
		Process the list of task generators
		"""
		for g in self.groups:

			for tg in g:
				
				if not isinstance(tg, TaskGen.task_gen):
					continue
				
				# only include projects which are valid for any spec which we have
				if self.options.specs_to_include_in_project_generation != '':			
					bSkipModule = True
					allowed_specs = self.options.specs_to_include_in_project_generation.replace(' ', '').split(',')
					for spec_name in allowed_specs:

						if tg.target in self.spec_modules(spec_name):
							bSkipModule = False
							break
							
					if bSkipModule:
						continue

				if not hasattr(tg, 'msvs_includes'):
					tg.msvs_includes = tg.to_list(getattr(tg, 'includes', [])) + tg.to_list(getattr(tg, 'export_includes', []))
				tg.post()

				p = self.vsnode_target(self, tg)
				p.collect_source() # delegate this processing
				p.collect_properties()
				self.all_projects.append(p)

	def add_aliases(self):
		"""
		Add a specific target that emulates the "make all" necessary for Visual studio when pressing F7
		We also add an alias for "make install" (disabled by default)
		"""
		base = getattr(self, 'projects_dir', None) or self.tg.path

		node_project = base.make_node('_WAF_' + self.project_extension) # Node.		
		p_build = self.vsnode_build_all(self, node_project)
		p_build.collect_properties()		
		self.all_projects.append(p_build)


	def collect_dirs(self):
		"""
		Create the folder structure in the Visual studio project view
		"""
		seen = {}

		for p in self.all_projects[:]: # iterate over a copy of all projects
			if not getattr(p, 'tg', None):
				# but only projects that have a task generator
				continue
				
			if getattr(p, 'parent', None):
				# aliases already have parents
				continue

			filter_name = self.get_project_vs_filter(p.name)
							
			proj = p
			filter_list = filter_name.split('/')
			filter_list.reverse()
			for f in filter_list:				
				if f in seen:						
					proj.parent = seen[f]
					break
				else:						
					n = proj.parent = seen[f] = self.vsnode_vsdir(self, make_uuid(f), f)
					self.all_projects.append(n)
					proj = n

		
def wrap_2008(cls):
	class dec(cls):
		def __init__(self, *k, **kw):
			cls.__init__(self, *k, **kw)
			self.project_template = PROJECT_2008_TEMPLATE

		def display_filter(self):

			root = build_property()
			root.subfilters = []
			root.sourcefiles = []
			root.source = []
			root.name = ''

			@Utils.run_once
			def add_path(lst):
				if not lst:
					return root
				child = build_property()
				child.subfilters = []
				child.sourcefiles = []
				child.source = []
				child.name = lst[-1]

				par = add_path(lst[:-1])
				par.subfilters.append(child)
				return child

			for x in self.source:
				# this crap is for enabling subclasses to override get_filter_name
				tmp = self.get_filter_name(x.parent)
				tmp = tmp != '.' and tuple(tmp.split('\\')) or ()
				par = add_path(tmp)
				par.source.append(x)

			def display(n):
				buf = []
				for x in n.source:
					buf.append('<File RelativePath="%s" FileType="%s"/>\n' % (xml_escape(x.abspath()), self.get_key(x)))
				for x in n.subfilters:
					buf.append('<Filter Name="%s">' % xml_escape(x.name))
					buf.append(display(x))
					buf.append('</Filter>')
				return '\n'.join(buf)

			return display(root)

		def get_key(self, node):
			"""
			If you do not want to let visual studio use the default file extensions,
			override this method to return a value:
				0: C/C++ Code, 1: C++ Class, 2: C++ Header File, 3: C++ Form,
				4: C++ Control, 5: Text File, 6: DEF File, 7: IDL File,
				8: Makefile, 9: RGS File, 10: RC File, 11: RES File, 12: XSD File,
				13: XML File, 14: HTML File, 15: CSS File, 16: Bitmap, 17: Icon,
				18: Resx File, 19: BSC File, 20: XSX File, 21: C++ Web Service,
				22: ASAX File, 23: Asp Page, 24: Document, 25: Discovery File,
				26: C# File, 27: eFileTypeClassDiagram, 28: MHTML Document,
				29: Property Sheet, 30: Cursor, 31: Manifest, 32: eFileTypeRDLC
			"""
			return ''

		def write(self):
			Logs.debug('msvs: creating %r' % self.path)
			template1 = compile_template(self.project_template)
			proj_str = template1(self)
			proj_str = rm_blank_lines(proj_str)
			self.path.stealth_write(proj_str)

	return dec

class msvs_2008_generator(msvs_generator):
	'''generates a visual studio 2008 solution'''
	cmd = 'msvs2008'
	fun = msvs_generator.fun

	def init(self):
		if not getattr(self, 'project_extension', None):
			self.project_extension = '_2008.vcproj'
		if not getattr(self, 'solution_name', None):
			self.solution_name = getattr(Context.g_module, Context.APPNAME, 'project') + '_2008.sln'

		if not getattr(self, 'vsnode_target', None):
			self.vsnode_target = wrap_2008(vsnode_target)
		if not getattr(self, 'vsnode_build_all', None):
			self.vsnode_build_all = wrap_2008(vsnode_build_all)
		if not getattr(self, 'vsnode_install_all', None):
			self.vsnode_install_all = wrap_2008(vsnode_install_all)
		if not getattr(self, 'vsnode_project_view', None):
			self.vsnode_project_view = wrap_2008(vsnode_project_view)

		msvs_generator.init(self)
		self.numver = '10.00'
		self.vsver  = '2008'

def options(ctx):
	"""
	If the msvs option is used, try to detect if the build is made from visual studio
	"""
	ctx.add_option('--execsolution', action='store', help='when building with visual studio, use a build state file')

	old = BuildContext.execute
	def override_build_state(ctx):
		def lock(rm, add):
			uns = ctx.options.execsolution.replace('.sln', rm)
			uns = ctx.root.make_node(uns)
			try:
				uns.delete()
			except:
				pass

			uns = ctx.options.execsolution.replace('.sln', add)
			uns = ctx.root.make_node(uns)
			try:
				uns.write('')
			except:
				pass

		if ctx.options.execsolution:
			ctx.launch_dir = Context.top_dir # force a build for the whole project (invalid cwd when called by visual studio)
			lock('.lastbuildstate', '.unsuccessfulbuild')
			old(ctx)
			lock('.unsuccessfulbuild', '.lastbuildstate')
		else:
			old(ctx)
	BuildContext.execute = override_build_state
	

