#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>
#include <fcntl.h>
#include <math.h>
#include <string.h>
#include "extfs.h"

struct ext2_super_block spblock;
void preadWrapper(int ifd, void*buf, size_t size, off_t off){
	size_t read = pread(ifd, buf, size, off);
	if(read < size){
		fprintf(stderr, "Error with pread\n");
		exit(2);
	}
}
void timeWrapper(time_t time, char* answer){
	strftime(answer,20,"%m/%d/%y %H:%M:%S", gmtime(&time));
}
void recursiveDirectory(int depth, struct ext2_inode *node, char file, __u32 blnum, __u32 inum, __u32 offset, int ifd){
	__u32 block[EXT2_MIN_BLOCK_SIZE << spblock.s_log_block_size];
	preadWrapper(ifd, block, EXT2_MIN_BLOCK_SIZE << spblock.s_log_block_size, blnum * (EXT2_MIN_BLOCK_SIZE << spblock.s_log_block_size));

	for(int i = 0; i < (EXT2_MIN_BLOCK_SIZE << spblock.s_log_block_size) / 4; i++ ){
		if(block[i] == 0){
			offset++;
			continue;
		}
		if(depth == 1 && file == 'd'){
			char block2[EXT2_MIN_BLOCK_SIZE << spblock.s_log_block_size];
			struct ext2_dir_entry *dir;
			preadWrapper(ifd, block2,EXT2_MIN_BLOCK_SIZE << spblock.s_log_block_size, blnum *(EXT2_MIN_BLOCK_SIZE << spblock.s_log_block_size));
			dir = (struct ext2_dir_entry*) block2;
			for(__u32 offset2 = 0; offset2<node->i_size && dir->file_type; offset+=dir->rec_len){
				char name[EXT2_NAME_LEN + 1];
				memcpy(name, dir->name, dir->name_len);
				name[dir->name_len] = 0;
				printf("DIRENT,%d,%u,%u,%u,%u,'%s'\n",inum,offset*(EXT2_MIN_BLOCK_SIZE << spblock.s_log_block_size)+offset,dir->inode,dir->rec_len,dir->name_len,name);
			}
		}
		printf("INDIRECT,%d,%u,%u,%u,%u\n",inum,depth,offset,blnum,block[i]);
		if(depth > 1){
			recursiveDirectory(depth-1, node, file, block[i], inum, offset, ifd);
		}
		if(depth == 1){
			offset++;
		}
		if(depth == 2){
			offset += 256;
		}
		if(depth == 3){
			offset += 256 * 256;
		}
	}
}
int main(int argc, char **argv){
	if(argc != 2){
		fprintf(stderr,"Incorrect number of arguments\n");
		exit(1);
	}
	int ifd = open(argv[1], O_RDONLY);
	if(ifd<0){
		fprintf(stderr, "Invalid file systme image\n");
		exit(1);
	}

	//SUPER BLOCK
	int a = pread(ifd, &spblock,sizeof(struct ext2_super_block), 1024);
	if(a < 0){
		fprintf(stderr, "Error reading superblock\n");
		exit(2);
	}
	printf("SUPERBLOCK,%u,%u,%u,%u,%u,%u,%u\n",spblock.s_blocks_count, spblock.s_inodes_count, 1024<<spblock.s_log_block_size, spblock.s_inode_size, spblock.s_blocks_per_group, spblock.s_inodes_per_group, spblock.s_first_ino);

	//GROUP TABLE
	struct ext2_group_desc grptable;
	a = pread(ifd, &grptable,sizeof(struct ext2_group_desc), 2048);
        if(a < 0){
                fprintf(stderr, "Error reading group descriptor table\n");
                exit(2);
        }
	printf("GROUP,%u,%u,%u,%u,%u,%u,%u,%u\n",0,spblock.s_blocks_count,spblock.s_inodes_count,grptable.bg_free_blocks_count,grptable.bg_free_inodes_count,grptable.bg_block_bitmap,grptable.bg_inode_bitmap,grptable.bg_inode_table);

	//FREE BLOCK ENTRIES
	__u32 max = ceil(spblock.s_blocks_count/8);
	__u32 bitmap = grptable.bg_block_bitmap*EXT2_MIN_BLOCK_SIZE;
	int compare = 1;
	for(__u32 i = 0; i < max; i++){
		__u8 byte;
		compare = 1;
		preadWrapper(ifd,&byte,sizeof(__u8),bitmap+i);
		for(int j = 0; j < 8; j++){
			if((compare&byte) == 0){
				printf("BFREE,%u\n",8*i+j+1);
			}
			compare = compare<<1;
		}
	}

	//FREE INODE ENTRIES
	max = ceil(spblock.s_inodes_count/8);
        bitmap = grptable.bg_inode_bitmap*EXT2_MIN_BLOCK_SIZE;
        for(__u32 i = 0; i < max; i++){
       		compare = 1;
	         __u8 byte;
                preadWrapper(ifd,&byte,sizeof(__u8),bitmap+i);
                for(int j = 0; j < 8; j++){
                        if((compare&byte) == 0){
                                printf("IFREE,%u\n",8*i+j+1);
                        }
                        compare = compare<<1;
                }
        }

	//INODE SUMMARY
	__u32 offset = grptable.bg_inode_table * EXT2_MIN_BLOCK_SIZE;
	struct ext2_inode node;
	for(__u32 i = 1; i <= spblock.s_inodes_count; i++){
		preadWrapper(ifd, &node, spblock.s_inode_size, offset + (i-1)*spblock.s_inode_size);
		if(!node.i_links_count || !node.i_mode){
			continue;
		}
		char crt[20];
		char mdf[20];
		char acc[20];
		timeWrapper(node.i_ctime,crt);
		timeWrapper(node.i_mtime,mdf);
		timeWrapper(node.i_atime,acc);
		char type;
		if(node.i_mode & 0x4000){
			type = 'd';
		}
		else if(node.i_mode & 0x8000){
			type = 'f';
		}
		else if(node.i_mode & 0xA000){
			type = 's';
		}
		else{
			type = '?';
		}
		printf("INODE,%u,%c,%o,%u,%u,%u,%s,%s,%s,%u,%u",i,type,node.i_mode & 0x0FFF,node.i_uid,node.i_gid,node.i_links_count,crt,mdf,acc,node.i_size,node.i_blocks);
		if(type == 'd' || type == 'f'){
			for(int j = 0; j < EXT2_N_BLOCKS; j++){
                                printf(",%u",node.i_block[j]);
                        }
		}
		if(type == 's' && node.i_size > 60){
			for(int j = 0; j < EXT2_N_BLOCKS; j++){
				printf(",%u",node.i_block[j]);
			}
		}
		printf("\n");
		//DIRECTORY ENTRIES
		if(type == 'd'){
			__u32 offset2 = 0;
			struct ext2_dir_entry *dir;
			unsigned char bl[EXT2_MIN_BLOCK_SIZE << spblock.s_log_block_size];
			for(int j = 0; j < EXT2_NDIR_BLOCKS; j++){
				preadWrapper(ifd,bl,EXT2_MIN_BLOCK_SIZE << spblock.s_log_block_size,node.i_block[j] * (EXT2_MIN_BLOCK_SIZE << spblock.s_log_block_size));
				dir = (struct ext2_dir_entry *) bl;
				while((offset2 < node.i_size) && dir->file_type){
					if(dir->inode != 0){
						char name[EXT2_NAME_LEN + 1];
						memcpy(name, dir->name, dir->name_len);
						name[dir->name_len] = 0;
						printf("DIRENT,%d,%u,%u,%u,%u,'%s'\n",i,offset2,dir->inode,dir->rec_len,dir->name_len,name);
					}
					offset2 = offset2 + dir->rec_len;
					dir = (void*) dir + dir->rec_len;
				}
			}		
		}
		//INDIRECT BLOCK REFERENCES
		if(type == 'f' || type == 'd'){
			if(node.i_block[12] != 0){
				recursiveDirectory(1, &node, type, node.i_block[12], i, 12, ifd);
			}
			if(node.i_block[13] != 0){
				recursiveDirectory(2, &node, type, node.i_block[13], i, 268, ifd);
			}
			if(node.i_block[14] != 0){
				recursiveDirectory(3, &node, type, node.i_block[14], i, 65804, ifd);
			}
		}
	}
	exit(0);
}