#############################################################################
## Crytek Source File
## Copyright (C) 2013, Crytek Studios
##
## Creator: Nico Moss
## Date: Oct 27, 2014
## Description: WAF  based build system
#############################################################################
from waflib.Configure import conf
from waflib import Logs, Node, Errors
from waflib.Scripting import _is_option_true

import json, os

@conf
def is_option_true(ctx, option_name):
	""" Util function to better intrepret all flavors of true/false """
	return _is_option_true(ctx.options, option_name)
	
#############################################################################
#############################################################################
# Helper functions to handle error and warning output
@conf
def cry_error(conf, msg):
	conf.fatal("error: %s" % msg) 
	
@conf
def cry_file_error(conf, msg, filePath, lineNum = 0 ):
	if isinstance(filePath, Node.Node):
		filePath = filePath.abspath()
	if not os.path.isabs(filePath):
		filePath = conf.path.make_node(filePath).abspath()
	conf.fatal('%s(%s): error: %s' % (filePath, lineNum, msg))
	
@conf
def cry_warning(conf, msg):
	Logs.warn("warning: %s" % msg) 
	
@conf
def cry_file_warning(conf, msg, filePath, lineNum = 0 ):
	Logs.warn('%s(%s): warning: %s.' % (filePath, lineNum, msg))
	
#############################################################################
#############################################################################	
# Helper functions to json file parsing and validation

def _decode_list(data):
    rv = []
    for item in data:
        if isinstance(item, unicode):
            item = item.encode('utf-8')
        elif isinstance(item, list):
            item = _decode_list(item)
        elif isinstance(item, dict):
            item = _decode_dict(item)
        rv.append(item)
    return rv
		
def _decode_dict(data):
    rv = {}
    for key, value in data.iteritems():
        if isinstance(key, unicode):
            key = key.encode('utf-8')
        if isinstance(value, unicode):
            value = value.encode('utf-8')
        elif isinstance(value, list):
            value = _decode_list(value)
        elif isinstance(value, dict):
            value = _decode_dict(value)
        rv[key] = value
    return rv

@conf
def parse_json_file(conf, file_node):	
	try:
		file_content_raw = file_node.read()
		file_content_parsed = json.loads(file_content_raw, object_hook=_decode_dict)
		return file_content_parsed
	except Exception as e:
		line_num = 0
		exception_str = str(e)
		
		# Handle invalid last entry in list error
		if "No JSON object could be decoded" in exception_str:
			cur_line = ""
			prev_line = ""
			file_content_by_line = file_content_raw.split('\n')
			for lineIndex, line in enumerate(file_content_by_line):
			
				# Sanitize string
				cur_line = ''.join(line.split())	
				
				# Handle empty line
				if not cur_line:
					continue
				
				# Check for invalid JSON schema
				if any(substring in (prev_line + cur_line) for substring in [",]", ",}"]):
					line_num = lineIndex
					exception_str = 'Invalid JSON, last list/dictionary entry should not end with a ",". [Original exception: "%s"]' % exception_str
					break;
					
				prev_line = cur_line
	  
		# If exception has not been handled yet
		if not line_num:
			# Search for 'line' in exception and output pure string
			exception_str_list = exception_str.split(" ")
			for index, elem in enumerate(exception_str_list):
				if elem == 'line':
					line_num = exception_str_list[index+1]
					break
					
		# Raise fatal error
		conf.cry_file_error(exception_str, file_node.abspath(), line_num)
	