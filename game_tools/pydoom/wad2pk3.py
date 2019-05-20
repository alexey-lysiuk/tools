#!/usr/bin/env python

"""
Utility to convert Doom WAD files to ZDoom PK3 (zip) files.
by Devin Acker
Nov 23 2008

Licensed under WTFPL
http://sam.zoy.org/wtfpl/

"""


from omg.wad import *
from zipfile import ZipFile, ZIP_DEFLATED
from re import search, sub
from os import remove
from sys import argv

# pathlookup tells wad2pk3 which directory in the zip file to use
# for each group of lumps in the wad (based on omgifol's WAD class)

pathlookup = {'sprites': 'sprites/',
'patches': 'patches/',
'flats': 'flats/',
'colormaps': 'colormaps/',
'ztextures': 'textures/',
'maps': 'maps/',
'glmaps': 'maps/',
'music': 'music/',
'sounds': 'sounds/',
'txdefs': '',
'graphics': 'graphics/',
'data': ''}

def main(argv):
	input = WAD()

	# iterate through supplied WAD files and load them into input
	# (ignores arguments without '.wad' in them)
	for arg in argv:
		if search('.\.wad', arg):
			print '  Using ' + arg
			input.from_file(arg.replace('\\', '/'))
			# make sure the path contains forward slashes

	# Check to see whether an output pk3 file was specified or not.
	# if not, the pk3 is named after the first wad file specified 
	if (search('.\.pk3', argv[len(argv) - 1])):
		# pk3 was specified
		outfile = argv[len(argv) - 1]
	else:
		# pk3 file not specified
		outfile = sub('\.wad', '.pk3', argv[0])
		

	# begin writing the lumps to the new .pk3 file
	try:
		print '  Using ' + outfile
		output = ZipFile(outfile.replace('\\','/'), 'a', ZIP_DEFLATED)
		
		# go thru the groups of lumps and write them into the wad one at a time

		for sprite in input.sprites.items():
			# contains a special check for archvile sprites etc
			# as stated in the zdoom wiki
			output.writestr('sprites/' + sprite[0].replace('\\','^'), sprite[1].data)
		for map in input.maps.items() + input.glmaps.items():
			# maps and glmaps contain multiple members which must all be written
			# to a new wad file
			tempwad = WAD()
			tempwad.maps[map[0]] = map[1]
			tempwad.to_file('temp.wad')
			output.write('temp.wad', 'maps/' + map[0] + '.wad')
			remove('temp.wad')
	
		# the rest of the lump groups, which require no special treatment

		for lumps in input.groups:
                        if (lumps._name != 'sprites' and lumps._name != 'maps'):
                                for lumpitem in lumps.items():
                                        # write it to a directory based on which group it is in
                                        # (see pathlookup above)
                                        try: output.writestr(pathlookup[lumps._name] + lumpitem[0], lumpitem[1].data)
                                        except AttributeError: None

	finally:
		# LAST OF ALL: close pk3 file (v. important) 
		print '  Closing ' +outfile
		output.close()	


print "\n  WAD to PK3 converter - by Devin Acker\n"
if (len(argv) < 2):
	print "  Usage:"
	print "  wad2pk3 source1.wad [source2.wad ... sourceN.wad] [target.pk3]"
	print "  Merges all source .wad files into a single target .pk3 file."
	print "  If target is not specified, it will take its name from source1."
else:
	main(argv[1:])


