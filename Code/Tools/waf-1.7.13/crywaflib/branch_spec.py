#############################################################################
## Crytek Source File
## Copyright (C) 2013, Crytek Studios
##
## Creator: Christopher Bolte
## Date: Oct 31, 2013
## Description: Central configuration file for a branch, should never be
## 				integrated since it is unique for each branch
#############################################################################

########### Below are various getter to expose the global values ############
from waflib.Configure import conf
from waflib import Context, Utils, Logs
from waflib import Errors

from waf_branch_spec import RECODE_LICENSE_KEY

from waf_branch_spec import BINTEMP_FOLDER

from waf_branch_spec import COMPANY_NAME
from waf_branch_spec import COPYRIGHT

from waf_branch_spec import PLATFORMS
from waf_branch_spec import CONFIGURATIONS

#############################################################################	
@conf 
def get_recode_license_key(self):
	return RECODE_LICENSE_KEY
	
#############################################################################	
@conf 
def get_recode_base_dir(self):
	return '../'	

#############################################################################	
#############################################################################	
@conf 
def get_bintemp_folder_node(self):	
	return self.root.make_node(Context.launch_dir).make_node(BINTEMP_FOLDER)	

#############################################################################	
@conf 
def get_project_output_folder(self):
	project_folder_node = self.root.make_node(Context.launch_dir).make_node(self.options.visual_studio_solution_folder).make_node('.depproj')
	project_folder_node.mkdir()
	return project_folder_node

#############################################################################	
@conf 
def get_solution_name(self):
	return self.options.visual_studio_solution_folder + '/' + self.options.visual_studio_solution_name + '.sln'

#############################################################################	
#############################################################################	
@conf 
def get_company_name(self):
	return COMPANY_NAME	
	
#############################################################################	
@conf 
def get_product_version(self):	
	return self.options.version	

#############################################################################	
@conf 
def get_copyright(self):
	return COPYRIGHT	
		
#############################################################################
#############################################################################	
@conf
def get_supported_platforms(self):
	host = Utils.unversioned_sys_platform()	
	return PLATFORMS[host]

#############################################################################
#############################################################################	
@conf
def set_supported_platforms(self, platforms):
	host = Utils.unversioned_sys_platform()	
	PLATFORMS[host] = platforms
	
#############################################################################	
@conf
def get_supported_configurations(self):
	return CONFIGURATIONS
	
#############################################################################
#############################################################################	
@conf
def get_project_vs_filter(self, target):		
	if not hasattr(self, 'vs_project_filters'):
		self.fatal('vs_project_filters not initialized')

	if not target in self.vs_project_filters:
		self.fatal('No visual studio filter entry found for %s' % target)
		
	return self.vs_project_filters[target]
	
#############################################################################
#############################################################################	

def _load_specs(ctx):
	""" Helper function to find all specs stored in _WAF_/specs/*.json """
	if hasattr(ctx, 'loaded_specs_dict'):
		return
		
	ctx.loaded_specs_dict = {}
	spec_file_folder 	= ctx.root.make_node(Context.launch_dir).make_node('/_WAF_/specs')
	spec_files 				= spec_file_folder.ant_glob('**/*.json')
	
	for file in spec_files:
		try:
			spec = ctx.parse_json_file(file)
			spec_name = str(file).split('.')[0]
			ctx.loaded_specs_dict[spec_name] = spec
		except Exception as e:
			ctx.cry_file_error(str(e), file.abspath())	


@conf	
def loaded_specs(ctx):
	""" Get a list of the names of all specs """
	_load_specs(ctx)
	
	ret = []				
	for (spec,entry) in ctx.loaded_specs_dict.items():
		ret.append(spec)
	
	return ret

def _spec_entry(ctx, entry, spec_name = None):
	""" Util function to load a specifc spec entry (always returns a list) """
	_load_specs(ctx)
	
	if not spec_name: # Fall back to command line specified spec if none was given
		spec_name = ctx.options.project_spec
		
	spec = ctx.loaded_specs_dict.get(spec_name, None)
	if not spec:
		ctx.fatal('[ERROR] Unknown Spec "%s", valid settings are "%s"' % (spec_name, ', '.join(ctx.loaded_specs())))

	def _to_list( value ):
		if isinstance(value,list):
			return value
		return [ value ]
		
	res = _to_list( spec.get(entry, []) )
	if hasattr(ctx, 'env'): # The options context is missing an env attribute, hence don't try to find platform settings in this case
		res += ctx.GetPlatformSpecificSettings(spec, entry, ctx.env['PLATFORM'], ctx.env['CONFIGURATION'])

	return res

@conf	
def spec_modules(ctx, spec_name = None):
	""" Get all a list of all modules needed for this spec """
	return _spec_entry(ctx, 'modules', spec_name)	

	
@conf	
def spec_game_projects(ctx, spec_name = None):
	""" get a list of all game projects in this spec """
	return _spec_entry(ctx, 'game_projects', spec_name)

	
@conf	
def spec_defines(ctx, spec_name = None):
	""" Get all a list of all defines needed for this spec """
	""" This function is deprecated """
	return _spec_entry(ctx, 'defines', spec_name)

	
@conf	
def spec_visual_studio_name(ctx, spec_name = None):
	""" Get all a the name of this spec for Visual Studio """
	visual_studio_name =  _spec_entry(ctx, 'visual_studio_name', spec_name)
	
	if not visual_studio_name:
		ctx.fatal('[ERROR] Mandatory field "visual_studio_name" missing from spec "%s"' % (spec_name or ctx.options.project_spec) )
		
	# _spec_entry always returns a list, but client code expects a single string
	assert( len(visual_studio_name) == 1 )
	return visual_studio_name[0]

@conf	
def spec_output_folder_post_fix(ctx, spec_name = None):
	""" get a list of all game projects in this spec """
	output_folder_post_fix =  _spec_entry(ctx, 'output_folder_post_fix', spec_name)
	
	# _spec_entry always returns a list, but client code expects a single string
	return output_folder_post_fix[0] if output_folder_post_fix else ""
	
@conf	
def spec_valid_configurations(ctx, spec_name = None):
	""" Get all a list of all valid configurations for this spec """
	valid_configuration =  _spec_entry(ctx, 'valid_configuration', spec_name)
	if valid_configuration == []:
		ctx.fatal('[ERROR] Mandatory field "valid_configuration" missing from spec "%s"' % (spec_name or ctx.options.project_spec))
		
	return valid_configuration
	
	
@conf	
def spec_valid_platforms(ctx, spec_name = None):
	""" Get all a list of all valid platforms for this spec """
	valid_platforms =  _spec_entry(ctx, 'valid_platforms', spec_name)
	
	if not valid_platforms:
		ctx.fatal('[ERROR] Mandatory field "valid_platforms" missing from spec "%s"' % (spec_name or ctx.options.project_spec) )
		
	return valid_platforms

	
@conf	
def spec_description(ctx, spec_name = None):
	""" Get description for this spec """
	description = _spec_entry(ctx, 'description', spec_name)
	if description == []:
		ctx.fatal('[ERROR] Mandatory field "description" missing from spec "%s"' % (spec_name or ctx.options.project_spec) )
	
	# _spec_entry always returns a list, but client code expects a single string
	assert( len(description) == 1 )
	return description[0]
	
	
@conf	
def spec_monolithic_build(ctx, spec_name = None):
	""" Return true if the current platform|configuration should be a monolithic build in this spec """
	monolithic_build = _spec_entry(ctx, 'monolithic_build', spec_name)
	if monolithic_build == []:
		return False
	
	# _spec_entry always returns a list, but client code expects a single boolean
	assert( len(monolithic_build) == 1 )
	return monolithic_build[0].lower() == "true"	
	


	
# Set global output directory	
setattr(Context.g_module, Context.OUT, BINTEMP_FOLDER)
