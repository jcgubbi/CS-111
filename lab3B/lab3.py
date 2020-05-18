#!/usr/bin/python

import sys
import csv

superb = None
groups = []
bfree = []
ifree = []
my_ifree = []
inodes = []
dirents = []
indirects = []
exitcode = 0
referenced = []
my_block_map = []
all_used = []
inodeDirs = []
class Superblock: 
	def __init__(self, line):
		self.num_blocks = int(line[1])
		self.num_inodes = int(line[2])
		self.block_size = int(line[3])
		self.inode_size = int(line[4])
		self.blocks_per_group = int(line[5])
		self.inodes_per_group = int(line[6])
		self.first_nonreserved_inode = int(line[7])

class Group:
	def __init__(self, line):
		self.group_number = int(line[1])
		self.num_blocks = int(line[2])
		self.num_inodes = int(line[3])
		self.num_free_blocks = int(line[4])
		self.num_free_inodes = int(line[5])
		self.block_num_block_bitmap = int(line[6])
		self.block_num_inode_bitmap = int(line[7])
		self.block_num_first_free = int(line[8])

class Inode:
	def __init__(self, line):
		self.number = int(line[1])
		self.type = line[2]
		self.mode = int(line[3])
		self.owner = int(line[4])
		self.group = int(line[5])
		self.link_count = int(line[6])
		self.time_change = line[7]
		self.mod_time = line[8]
		self.access_time = line[9]
		self.file_size = int(line[10])
		self.num_blocks = int(line[11])
		self.single_indirect = int(line[24])
		self.double_indirect = int(line[25])
		self.triple_indirect = int(line[26])
		self.blockm = list(map(int, line[12:24]))

class Dirent:
	def __init__(self, line):
		self.parent = int(line[1])
		self.offset = int(line[2])
		self.inode = int(line[3])
		self.entry = int(line[4])
		self.name_len = int(line[5])
		self.name = line[6]

class Indirect:
	def __init__(self, line):
		self.inode_num = int(line[1])
		self.level_indirect = int(line[2])
		self.offset = int(line[3])
		self.block_num = int(line[4])
		self.ref_block = int(line[5])

def blockChecker(block, inode, label, offset):
	if block < 0 or block > superb.num_blocks:
		print("INVALID " + label + "BLOCK " +str(block) + " IN INODE " + str(inode) + " AT OFFSET " + str(offset))
		exitcode = 2
	elif block is not 0 and block < superb.num_inodes * superb.inode_size / superb.block_size + 5:
		print("RESERVED " + label + "BLOCK " + str(block) + " IN INODE " + str(inode) + " AT OFFSET " + str(offset))
		exitcode = 2
	elif block is not 0:
		my_block_map.append(block)

def duplicates(block, inode, label, offset):
	if block in my_block_map and my_block_map.count(block) > 1:
		print("DUPLICATE "+ label +"BLOCK " + str(block) + " IN INODE " + str(inode) + " AT OFFSET " + str(offset))
		my_block_map.append(block)
		exitcode = 2
if __name__ == '__main__':
	if len(sys.argv) is not 2: 
		sys.stderr.write("Incorrect number of arguments.\n")
		exit(1)

	#read csv file and save each line into proper data structure
	file = open(sys.argv[1], 'r')
	lines = csv.reader(file)
	for line in lines:
		if len(line) is 0: 
			sys.stderr.write("Empty line in CSV.\n")
			exit(1)
		if line[0] == "INODE":
			inodes.append(Inode(line))
		elif line[0] == "SUPERBLOCK":
			superb = Superblock(line)
		elif line[0] == "GROUP":
			groups.append(Group(line))
		elif line[0] == "BFREE":
			bfree.append(int(line[1]))
		elif line[0] == "IFREE":
			ifree.append(int(line[1]))
		elif line[0] == "DIRENT":
			dirents.append(Dirent(line))
		elif line[0] == "INDIRECT":
			indirects.append(Indirect(line))
		else:
			sys.stderr.write("Invalid line in CSV file\n")
			print(line)
			exit(1)
	my_ifree = ifree
	for i in inodes:
		if i.type == 's' and i.file_size <= 60:
			continue
		blockChecker(i.single_indirect,i.number,"INDIRECT ",12)
		blockChecker(i.double_indirect,i.number,"DOUBLE INDIRECT ", 268)
		blockChecker(i.triple_indirect,i.number,"TRIPLE INDIRECT ", 65804)
		for offsetVal, blockNum in enumerate(i.blockm):
			blockChecker(blockNum, i.number, "", offsetVal)
			if blockNum is 0:
				continue
		if i.type != '0':
			if i.number in ifree:
				print("ALLOCATED INODE " + str(i.number) + " ON FREELIST")
				exitcode = 2
				my_ifree.remove(i.number)
			all_used.append(i)
		else:
			if i.number not in ifree:
				print("UNALLOCATED INODE " + str(i.number) + " NOT ON FREELIST")
				exitcode = 2
				my_ifree.append(i.number)
	for k in range (superb.first_nonreserved_inode, superb.num_inodes):
		if k not in ifree and k not in [j.number for j in inodes]:
			print("UNALLOCATED INODE " + str(k) + " NOT ON FREELIST")
			my_ifree.append(k)
			exitcode = 2
	for i in inodes:
		if i.type == 's' and i.file_size <= 60:
			continue
		duplicates(i.single_indirect,i.number,"INDIRECT ",12)
		duplicates(i.double_indirect,i.number,"DOUBLE INDIRECT ", 268)
		duplicates(i.triple_indirect,i.number,"TRIPLE INDIRECT ", 65804)
		for offsetVal, blockNum in enumerate(i.blockm):
			duplicates(blockNum, i.number, "", offsetVal)
	for d in dirents:
		if d.inode > superb.num_inodes:
			print("DIRECTORY INODE " +str(d.parent)+ " NAME " +str(d.name)+ " INVALID INODE " +str(d.inode))
			exitcode = 2
		elif d.inode in my_ifree:
			print("DIRECTORY INODE " + str(d.parent) + " NAME " + str(d.name) + " UNALLOCATED INODE " + str(d.inode))
			exitcode = 2
		else:
			inodeDirs.append(d.inode)
	for i in all_used:
		if i.number in inodeDirs:
			if i.link_count != inodeDirs.count(i.number):
				print("INODE " + str(i.number) + " HAS " + str(inodeDirs.count(i.number)) + " LINKS BUT LINKCOUNT IS " + str(i.link_count))
				exitcode = 2
		elif i.link_count is not 0:
			print("INODE " + str(i.number) + " HAS 0 LINKS BUT LINKCOUNT IS " + str(i.link_count))
			exitcode = 2
	for i in indirects:
		if i.level_indirect == 0:
			blockChecker(i.ref_block, i.inode_num, "",i.offset)
		if i.level_indirect == 1:
			blockChecker(i.ref_block, i.inode_num, "INDIRECT ",i.offset)
		if i.level_indirect == 2:
			blockChecker(i.ref_block, i.inode_num, "DOUBLE INDIRECT ",i.offset)
		if i.level_indirect == 3:
			blockChecker(i.ref_block, i.inode_num, "TRIPLE INDIRECT ",i.offset)
	for b in range (8, superb.num_blocks):
		if b not in bfree and b not in my_block_map:
			print("UNREFERENCED BLOCK " + str(b))
			exitcode = 2
		elif b in bfree and b in my_block_map:
			print("ALLOCATED BLOCK " + str(b) + " ON FREELIST")
			exitcode = 2
	parents = [None] * (superb.num_inodes+1)
	parents[2] = 2
	for d in dirents:
		if d.inode <= superb.num_inodes and d.inode not in my_ifree and d.name != "'.'" and d.name != "'..'":
			parents[d.inode] = d.parent
	for d in dirents:
		if d.name == "'.'" and d.inode != d.parent:
			print("DIRECTORY INODE " + str(d.parent) + " NAME '.' LINK TO INODE " + str(d.inode) + " SHOULD BE " + str(d.parent))
			exitcode = 2
		if d.name == "'..'" and d.inode != parents[d.parent]:
			print("DIRECTORY INODE " + str(d.parent) + " NAME '..' LINK TO INODE " + str(d.inode) + " SHOULD BE " + str(parents[d.parent]))
			exitcode = 2
	exit(exitcode)
