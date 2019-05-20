#!/usr/bin/env python

"""
Performs palette translations on Doom engine sprites and graphics.
by Devin Acker
Nov 15 2009

Fixed for large pictures Nov 24 2009

                        _    ,-,    _
                 ,--, /: :\/': :`\/: :\
                |`;  ' `,'   `.;    `: |
                |    |     |  '  |     |.
                | :  |     | pb  |     ||
                | :. |  :  |  :  |  :  | \
                 \__/: :.. : :.. | :.. |  )
                      `---',\___/,\___/ /'
                           `==._ .. . /'
                                `-::-'

"""

import sys
from string import split
from struct import pack, unpack

def main(src, colors):

	with open(src.replace('\\', '/'), 'r+b') as pic:
	
		# set up color translations
		translations = make_trans(colors)

		# get address of each column in pic
		for column in get_columns(pic):
			for trans in translations:
				do_column(pic, column, trans)


# return a list of color translations.
# each translation is a pair of triples made up
# of start color index, end color index, and
# step amount

def make_trans(colors):
	trans = []

	for x in range(0, len(colors), 2):
		range1 = split(colors[x], ':')
		range2 = split(colors[x+1], ':')

		min1 = int(min(range1))
		max1 = int(max(range1))
		min2 = int(min(range2))
		max2 = int(max(range2))

		#print min1,max1,min2,max2

		step1 = step2 = 1

		if (max1 - min1 < max2 - min2):
			
			step2 = (max2 - min2)/(max1 - min1)
		elif (max1 - min1 > max2 - min2):
			if (max2 == min2): step2 = 0
			else: step1 = (max1 - min1)/(max2 - min2)
		elif (max1 - min1 == 0 and max2-min2 ==0):
			step2 = 0
		
		trans.append(((min1, max1, step1), (min2, max2, step2)))
		
		
	return trans
	

# get a list of offsets of each column of pixels in the picture.
# picfile	: the image file to read from
def get_columns(picfile):
	# get no. of columns in pic
	picfile.seek(0)
	width = unpack('H', picfile.read(2))[0]

	picfile.seek(8)
	return unpack(str(width) + 'L', picfile.read(4*width))

# translate each column in a picture according to a translation definition.
# picfile	: the image file to read from
# offset	: file offset (find using get_columns)
# trans		: an individual translation (find using make_trans)
def do_column(picfile, offset, trans):
	picfile.seek(offset)

	# read the row number plus the size of the post
	try: post = unpack('2B', picfile.read(2))
	except: return -1

	
	# 0xFF signifies the end of a column
	if (post[0] == 255):
		return 0
	
	elif (post[1] != 0):
		# read in an arbitrary number of bytes with a dummy byte
		# on either end
		pixels = unpack('x' + str(post[1]) + 'Bx', picfile.read(post[1] + 2))
		newpixels = tuple()
		
		for pixel in pixels:
			# if pixel value is within the source range
			if (pixel >= trans[0][0] and pixel <= trans[0][1]):
				newpixel = (pixel - trans[0][0])/trans[0][2] * trans[1][2] + trans[1][0]
				newpixels = newpixels + (newpixel,)
				#print pixel, newpixel
			else: newpixels = newpixels + (pixel,)
			
		#write the translated pixels back into the file
		if (newpixels != pixels):
			packed = pack('2Bx', post[0], post[1])
			for pixel in newpixels:
				packed = packed + pack('B', pixel)
			packed = packed + pack('x')
			
			picfile.seek(offset)
			picfile.write(packed)
		
	#move to the next post
	do_column(picfile, offset+post[1]+4, trans)
	return 0
	
if (__name__ == "__main__"):
	print "\n  Doom palette mapper - by Devin Acker\n"
	if (len(sys.argv) < 4):
		print "  Usage:"
		print "  doompal inputfile aa[:bb] cc[:dd] [ee:ff gg:hh ...]\n"
		print "  Example:"
		print "  doompal source.lmp 32:47 192:207 112:127 80:95"
		print "  Remaps colors 32-47 (red) to 192-207 (blue), and also remaps"
		print "  112-127 (green) to 80-95 (grey). Specify as many pairs of"
		print "  color ranges as you want. \n"
		print "  Use 112:127 for Doom's green palette range (useful for player"
		print "  sprites.)\n"
		print "  For more detailed examples, check out:"
		print "  http://zdoom.org/wiki/Translation"
	else:
		main(sys.argv[1], sys.argv[2:])
