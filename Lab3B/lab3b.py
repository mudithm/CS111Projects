#!/usr/bin/python

#NAME: Mudith Mallajosyula
#EMAIL: mudithm@g.ucla.edu
#ID: 404937201

import sys
import csv
import os

# Global superblock
superblock = None

# free inodes set
free_inodes = set()

# free blocks set
free_blocks = set()

# referenced blocks dictionary (key: block_no; val: list of tuple(inode_no, logical_offset, level))
referenced_blocks = {}

# inodes dictionary (key: inode number; val: Inode)
inodes = {}

# dirents dictionary (key: tuple(parent inode number, name); val: Dirent)
dirents = {}

# parent inodes dictionary (key: inode num; value: parent inode num)
dir_parent_inodes = {2: 2} # initialized with special case for the root directory inode

# indirects dictionary (key: tuple(owning inode, block offset); val: Indirect)
indirects = {}

# link count dictionary (key: inode num; value: link count)
link_counts = {}

# global variable to store errors
error_code = 0

INDIRECT_TYPES = ["", "INDIRECT ", "DOUBLE INDIRECT ", "TRIPLE INDIRECT "]

# Classes to store relevant data

# Superblock, containing data about the file structure
class Superblock:
	def __init__(self, line):
		if len(line) != 7:
			sys.stderr.write("Superblock line is incomplete!\n")
			exit(1)
		self.num_blocks=int(line[0])
		self.num_inodes=int(line[1])
		self.block_size=int(line[2])
		self.inode_size=int(line[3])
		self.blocks_per_group=int(line[4])
		self.inodes_per_group=int(line[5])
		self.first_nonreserved_inode=int(line[6])

# Structure to contain inode data
class Inode:
	def __init__(self, line):
		if len(line) != 26:
			sys.stderr.write("Inode line is incomplete!\n")
			exit(1)
		self.inode_num=int(line[0])
		self.file_type=line[1]
		self.mode=int(line[2])
		self.owner=int(line[3])
		self.group=int(line[4])
		self.link_count=int(line[5])
		self.ctime=line[6]
		self.mtime=line[7]
		self.atime=line[8]
		self.file_size=int(line[9])
		self.blocks_taken=int(line[10])
		self.blocks=map(int, line[11:23])
		self.singly_ind=int(line[23])
		self.doubly_ind=int(line[24])
		self.triply_ind=int(line[25])
		#for i in range (0, 12):
		#	print self.blocks[i]

# Structure to contain directory entry data
class Dirent:
	def __init__(self, line):
		if len(line) != 6:
			sys.stderr.write("Directory entry line is incomplete!\n")
			exit(1)
		self.parent_inode_num=int(line[0])
		self.byte_offset=int(line[1])
		self.inode_num=int(line[2])
		self.entry_len=int(line[3])
		self.name_len=int(line[4])
		self.name=line[5]

# Structure to contain indirect reference data
class Indirect:
	def __init__(self, line):
		if len(line) != 5:
			sys.stderr.write("Indirect Reference line is incomplete!\n")
			exit(1)
		self.owner_inode_num=int(line[0])
		self.level_of_indirection=int(line[1])
		self.logical_block_offset=int(line[2])
		self.block_number=int(line[3])
		self.referenced_block_number=int(line[4])


# return a line without the type specifier
def getLine(line):
	return line[1:]

# parse the csv and populate the lists and structures
# before auditing
def populate_structures(file):
	# initialize globals
	global superblock, free_blocks, free_inodes, inodes, dirents, indirects

	image_data = csv.reader(file)
	for line in image_data:
		# check if line is empty
		if len(line) <= 0:
			sys.stderr.write("Empty line in input file!\n")
			exit(1)

		# get the type of data in the line, and split the line
		type = line[0]
		data = getLine(line)
		
		# populate data structures, and perform some basic checks
		if type == "SUPERBLOCK":
			if superblock != None:
				sys.stderr.write("Duplicate Superblock!")
			superblock = Superblock(data)
		elif type == "GROUP":
			pass # group data is unneccesary
		elif type == "BFREE":
			free_blocks.add(int(data[0]))
		elif type == "IFREE":
			free_inodes.add(int(data[0]))
		elif type == "INODE":
			# simple check for duplicates here
			if int(data[0]) in inodes:
				sys.stderr.write("Duplicate Inode " + data[0] + "!\n")
			inodes[int(data[0])] = Inode(data)
		elif type == "DIRENT":
			# simple check for duplicates here
			if (int(data[0]), data[5]) in dirents:
				sys.stderr.write("Duplicate Directory Entry (" + data[0]+ ", " + data[5] + ")!\n")
			dirents[(int(data[0]), data[5])] = Dirent(data)
		elif type == "INDIRECT":
			# simple check for duplicates here
			if ((int(data[0]), int(data[1]), int(data[2]))) in indirects:
				sys.stderr.write("Duplicate Indirect Entry (" + data[0] + ", " + data[2] + ")!\n")
			indirects[(int(data[0]), int(data[1]), int(data[2]))] = Indirect(data)
		else:
			sys.stderr.write("Invalid summary type in disk image summary\n")
			exit(1)

# examine a single block; if valid, add to the referenced block dictionary; else, error
def check_single_block(block_no, inode_no, logical_offset, indirect_level, indirect_type):
	# initialize globals
	global error_code, referenced_blocks

	# if block is allocated
	if block_no != 0:
		# reserved block	
		if block_no < 8:
			print "RESERVED " + indirect_type + "BLOCK " + str(block_no) + " IN INODE " \
			+ str(inode_no) + " AT OFFSET " + str(logical_offset)
			error_code=2
		# block is outside of range, invalid
		elif block_no > (superblock.num_blocks - 1):
			print "INVALID " + indirect_type + "BLOCK " + str(block_no) + " IN INODE " \
			+ str(inode_no) + " AT OFFSET " + str(logical_offset)
			error_code=2
		# valid block, already referenced
		elif block_no in referenced_blocks:
			referenced_blocks[block_no].append((inode_no, logical_offset, indirect_level))
		# valid block, not yet referenced
		else:
			referenced_blocks[block_no] = [(inode_no, logical_offset, indirect_level)]

# examine a single inode and update the free inodes set
def check_single_inode(inode_no):
	global error_code, free_inodes
	
	if inode_no < 0:
		sys.stderr.write("negative inode no. something's wrong here\n")

	# if inode should be unallocated but is not in free list
	if inode_no not in free_inodes:
		if inode_no not in inodes:
			print "UNALLOCATED INODE " + str(inode_no) + " NOT ON FREELIST"
			error_code=2
		elif inodes[inode_no].file_type == 0:
			print "UNALLOCATED INODE " + str(inode_no) + " NOT ON FREELIST"
			error_code=2
			free_inodes.add(inode_no)
	# if inode is in free list but might be allocated
	else:
		if inode_no not in inodes:
			return
#			print "ALLOCATED INODE " + str(inode_no) + " ON FREELIST"
		elif inodes[inode_no].file_type != 0:
			print "ALLOCATED INODE " + str(inode_no) + " ON FREELIST"
			error_code=2
			free_inodes.remove(inode_no)


# check a single directory for link errors
def check_single_directory_links(dirent_tuple):
	global error_code,  dir_parent_inodes

	entry = dirents[dirent_tuple]
	parent = dirent_tuple[0]
	name = dirent_tuple[1]
	inode_no = entry.inode_num

	# check if inode is in valid range
	if inode_no > superblock.num_inodes:
		print "DIRECTORY INODE " + str(parent) + " NAME " + name \
		+ " INVALID INODE " + str(inode_no)
		error_code=2

	# check if inode is unallocated
	elif inode_no in free_inodes:
		print "DIRECTORY INODE " + str(parent) + " NAME " + name \
		+ " UNALLOCATED INODE " + str(inode_no)
		error_code=2
	
	# add to inode link count. get function passed with default value 0
	else:
		link_counts[inode_no] = link_counts.get(inode_no, 0) + 1

	# check if parent inode is correct
	if name == "'.'":
		# check if dirent inode is same as parent
		if inode_no != parent:
			print "DIRECTORY INODE " + str(parent) + " NAME '.'" \
			+ " LINK TO INODE " + str(inode_no) +" SHOULD BE " \
			+ str(parent)
			error_code=2
	# check if parent directory inode is correct
	elif name == "'..'":
		# check if dirent inode is same as parent
		if inode_no != dir_parent_inodes[parent]:
			print "DIRECTORY INODE " + str(parent) + " NAME '..'" \
			+ " LINK TO INODE " + str(inode_no) +" SHOULD BE " \
			+ str(dir_parent_inodes[parent])
			error_code=2

def check_single_directory_inodes():
	
	return 0

# check for block inconsistencies
def check_block_consistency():
	# inialize globals
	global error_code, referenced_blocks

	# check all regular inodes
	for i in inodes:
		# check direct block references
		for index, block_no in enumerate(inodes[i].blocks):
			check_single_block(block_no, i, index, 0, INDIRECT_TYPES[0])
		# check 1st level indirect block references
		check_single_block(inodes[i].singly_ind, i, 12, 1, INDIRECT_TYPES[1])
		check_single_block(inodes[i].doubly_ind, i, 268, 2, INDIRECT_TYPES[2])
		check_single_block(inodes[i].triply_ind, i, 65804, 3, INDIRECT_TYPES[3])
	
	# check second-level indirect references
	for indirect in indirects:
		ind = indirects[indirect]
		check_single_block(ind.referenced_block_number, ind.owner_inode_num,
		 ind.logical_block_offset, ind.level_of_indirection, 
		 INDIRECT_TYPES[ind.level_of_indirection])

	# check all non-reserved blocks
	for block_no in range(8, superblock.num_blocks):
		# block is both free and referenced
		if (block_no in free_blocks) and (block_no in referenced_blocks):
			print "ALLOCATED BLOCK " + str(block_no) + " ON FREELIST"
		# block is neither free nor referenced
		elif (block_no not in free_blocks) and (block_no not in referenced_blocks):
			print "UNREFERENCED BLOCK " + str(block_no)
			error_code=2
		# valid block is referenced multiple times
		elif (block_no in referenced_blocks) and (len(referenced_blocks[block_no]) > 1):
			blrefs = referenced_blocks[block_no]
			for ref in blrefs:
				print "DUPLICATE " + INDIRECT_TYPES[ref[2]] + "BLOCK " \
				+ str(block_no) + " IN INODE " + str(ref[0]) + " AT OFFSET " \
			    + str(ref[1])
		    	error_code=2



# check for inode allocation inconsistencies
def check_inode_allocation():
	global superblock, inodes
	# check non-reserved inodes
	for inode_no in range(superblock.first_nonreserved_inode, superblock.num_inodes + 1):
		check_single_inode(inode_no)

	# check reserved inodes
	for inode_no in range(1, superblock.first_nonreserved_inode):
		if inode_no in inodes:
			check_single_inode(inode_no)





# check for directory entry inconsistencies
def check_dir_consistency():
	global error_code, dir_parent_inodes, free_inodes, inodes
	
	for dir in dirents:
		entry = dirents[dir]
		if entry.inode_num <= superblock.num_inodes and entry.inode_num not in free_inodes:
			if dir[1] != "'.'" and dir[1] != "'..'":
				dir_parent_inodes[entry.inode_num] = dir[0]

	# check basic inode validity, and populate links dictionary
	for dir in dirents:
		check_single_directory_links(dir)

	# check link counts
	for inode_no in inodes:
		if inode_no in link_counts:
			# if stated link count does not match actual link count
			if link_counts[inode_no] != inodes[inode_no].link_count:
				print "INODE " + str(inode_no) + " HAS " + str(link_counts[inode_no]) \
				+ " LINKS BUT LINKCOUNT IS " + str(inodes[inode_no].link_count)
				error_code=2
			# if stated link count is nonzero but  no links were found
		else:
			if inodes[inode_no].link_count != 0:
				print "INODE " + str(inode_no) + " HAS 0 LINKS BUT LINKCOUNT IS " \
				+ str(inodes[inode_no].link_count)
				error_code=2

# main function
if __name__ == '__main__':
	# checks number of arguments
	if (len(sys.argv)) != 2:
		sys.stderr.write("Wrong number of arguments.")
		exit(1)

	# checks if file exists
	filename = sys.argv[1]
	if not os.path.isfile(filename):
		sys.stderr.write("File " + filename + " could not be found!\n")
		exit(1)

	# checks if file can be opened
	f = open(filename, 'r')
	if not f:
		sys.stderr.write("Could not open file" + filename + "\n")
		exit(1)

	# populate data structures
	populate_structures(f)

	# perform consistency checks
	check_block_consistency()
	check_inode_allocation()
	check_dir_consistency()

	# exit w/ 0 if no inconsistencies, 2 if inconsistency
	exit(error_code)