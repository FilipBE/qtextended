#!/usr/bin/python

import getopt
import sys
import os
import re
import shutil
import stat

# Edit this to include more toplevel files
toplevel_files = ['configure', 'README', 'LICENSE.GPL', 'qbluetooth.pro']

def run_fixheaders(verbose, source, dest):
	# Assume fixheaders in in the same tree as source, under scripts/fixheaders

	if verbose:
		print("Running fixheaders...")

	fixheaders = os.path.abspath(source + '/../../../../scripts/fixheaders')

	curcwd = os.getcwd()
	os.chdir(dest)
	os.system('%s -free -gpl' % (fixheaders, ))
	os.chdir(curcwd)

def clean_abort(dest):
	shutil.rmtree(dest)
	pass

def copyto(src, dst):
	shutil.copy2(src, dst)
	bits = os.stat(dst).st_mode
	bits = bits | stat.S_IWUSR
	os.chmod(dst, bits)

def copytree(src, dst, symlinks=0):
    names = os.listdir(src)
    os.mkdir(dst)
    for name in names:
        srcname = os.path.join(src, name)
        dstname = os.path.join(dst, name)
        try:
            if symlinks and os.path.islink(srcname):
                linkto = os.readlink(srcname)
                os.symlink(linkto, dstname)
            elif os.path.isdir(srcname):
                copytree(srcname, dstname, symlinks)
            else:
                copyto(srcname, dstname)
        except (IOError, os.error), why:
            print "Can't copy %s to %s: %s" % (`srcname`, `dstname`, str(why))

def find_and_copy_example_files(verbose, source, dest):
	exdir = dest + '/examples'
	if verbose:
		print "Copying example directory to: %s" % (exdir)
	try:
		copytree(source + '/standalone/examples', exdir)
	except:
		print "Couldn't copy example directory, aborting..."
		clean_abort(dest)
		sys.exit(6)

def find_and_copy_toplevel(verbose, source, dest):
	for file in toplevel_files:
		srcfile = source + '/standalone/' + file
		dstfile = dest + '/' + file
		if verbose:
			print "Copying file %s to %s" % (srcfile, dstfile)
		try:
                	copyto(srcfile, dstfile)
		except:
			print "Couldn't copy file %s to %s" % (src, dst)
			clean_abort(dst)
			sys.exit(5)

def find_and_copy_src_files(verbose, source, dest):
	# Find and read src.pro file, usually in $source/standalone/src/src.pro
	srcpro = source + '/standalone/src/src.pro'

	try:
		f = open(srcpro, 'r')
	except IOError:
		print 'Couldn\'t open %s, aborting...' % (srcpro, )
		sys.exit(3)

	regmatch = re.compile(r"(?:qbluetooth|qsdp)[A-Za-z0-9_]+.(?:cpp|h)")
	qtinclude = re.compile(r"class[ \t]+QBLUETOOTH_EXPORT[ \t]+([A-Za-z0-9_]+)")

	files = []

	for line in f:
		for res in regmatch.finditer(line):
			files.append(res.group())

	f.close()

	if len(files) == 0:
		print 'Coudn\'t find any files to process, aborting...'
		sys.exit(4)

	try:
		if not os.path.isdir(dest):
			os.makedirs(dest)
		if not os.path.isdir(dest + '/src'):
			os.makedirs(dest + '/src')
		if not os.path.isdir(dest + '/src/qtincludes'):
			os.makedirs(dest + '/src/qtincludes')
	except:
		print "Couldn't create directory: %s" % (dest,)
		sys.exit(5)

	dstpro = dest + '/src/src.pro'

	#try:
	copyto(srcpro, dstpro)
#	except:
#		print "Couldn't copy file %s to %s" % (srcpro, dstpro)
#		clean_abort(dest)
#		sys.exit(5)

	for file in files:
		srcfile = source + '/' + file
		dstfile = dest + '/src/' + file
		if verbose:
			print 'Copying file %s to %s...' % (srcfile, dstfile)
		try:
        	        copyto(srcfile, dstfile)
		except:
			print "Couldn't copy file %s to %s" % (srcfile, dstfile)
			clean_abort(dst)
			sys.exit(5)

		if file[-4:] == '_p.h' or file[-2:] != '.h':
			continue
		try:
			f = open(srcfile, 'r')
		except:
			print "Couldn't open file: %s, aborting..." % (source + '/' + file, )
			clean_abort(dest)
			sys.exit(5)

		for line in f:
			res = qtinclude.search(line)
			if res:
				if verbose:
					print 'Found exported class %s in header %s' % (res.group(1), file)

				inclname = dest + '/src/qtincludes/' + res.group(1)
				try:
					incl = open(inclname, 'w')
				except:
					print "Couldn't open %s for writing" % (inclname,)
					continue
				
				hdrbase = os.path.basename(file)
				incl.write('#include "%s"\n' % (hdrbase, ))
				incl.close()

def usage():
	print 'Useage: %s [options]' % (sys.argv[0],)
	print 'This script is intended to make a standalone package for Qtopia Bluetooth library\n'

	print 'Options:'
	print '-v, --verbose\t\tPrint extra information about steps being performed'
	print '-h, --help\t\tPrint this helpful information'
	print '-o, --destdir\t\tPut the resulting package in this directory'
	print '-s, --sourcedir\t\tThe source directory is given here, defaults to \"..\"'

if __name__ == '__main__':
	try:
		optlist, args = getopt.getopt(sys.argv[1:], 'ho:s:v', ['verbose', 'help', 'destdir=', 'sourcedir='])
	except getopt.GetoptError:
		usage()
		sys.exit(2)

	outdir = os.getcwd() + '/QtBluetooth'
	verbose = False
	sourcedir = '..'

	for o, a in optlist:
		if o in ('-v', '--verbose'):
			verbose = True
		if o in ('-h', '--help'):
			usage()
			sys.exit()
		if o in ('-o', '--destdir'):
			outdir = a
		if o in ('-s', '--sourcedir'):
			sourcedir = a

	find_and_copy_src_files(verbose=verbose, source=sourcedir, dest=outdir)
	find_and_copy_example_files(verbose=verbose, source=sourcedir, dest=outdir)
	find_and_copy_toplevel(verbose=verbose, source=sourcedir, dest=outdir)

	run_fixheaders(verbose=verbose, source=sourcedir, dest=outdir)
