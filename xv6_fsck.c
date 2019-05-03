//#include <fs.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/mman.h>
#include <time.h>
#include <sys/time.h>
#include <signal.h>
#include <string.h>
#include <sys/stat.h>

#ifndef _FS_H_
#define _FS_H_

// On-disk file system format.
// Both the kernel and user programs use this header file.

// Block 0 is unused.
// Block 1 is super block.
// Inodes start at block 2.

#define ROOTINO 1  // root i-number
#define BSIZE 512  // block size

// File system super block
struct superblock {
	uint size;         // Size of file system image (blocks)
	uint nblocks;      // Number of data blocks
	uint ninodes;      // Number of inodes.
};

#define NDIRECT 12
#define NINDIRECT (BSIZE / sizeof(uint))
#define MAXFILE (NDIRECT + NINDIRECT)

// On-disk inode structure
struct dinode {
	short type;           // File type
	short major;          // Major device number (T_DEV only)
	short minor;          // Minor device number (T_DEV only)
	short nlink;          // Number of links to inode in file system
	uint size;            // Size of file (bytes)
	uint addrs[NDIRECT+1];   // Data block addresses
};

// Inodes per block.
#define IPB           (BSIZE / sizeof(struct dinode))

// Block containing inode i
#define IBLOCK(i)     ((i) / IPB + 2)

// Bitmap bits per block
#define BPB           (BSIZE*8)

// Block containing bit for block b
#define BBLOCK(b, ninodes) (b/BPB + (ninodes)/IPB + 3)

// Directory is a file containing a sequence of dirent structures.
#define DIRSIZ 14

struct dirent {
	ushort inum;
	char name[DIRSIZ];
};

#endif // _FS_H_

int main(int argc, char*argv[])
{

	if(argc !=2)
	{
		fprintf(stderr,"Usage: xv6_fsck <file_system_image>.\n");
		exit(1);
	}

	int fd = open(argv[1],O_RDONLY);

	if(fd < 0)
	{
		fprintf(stderr,"image not found.\n");
		exit(1);
	}

	int r;
	struct stat sbuf;

	r = fstat(fd,&sbuf);
	if(r!=0)
	{
		exit(1);
	}

	void* ptr = mmap(NULL, sbuf.st_size, PROT_READ, MAP_PRIVATE,fd,0);

	if(ptr==MAP_FAILED)
	{
		exit(1);
	}

	struct superblock *sb;
	sb = (struct superblock*)(ptr + BSIZE);

	struct dinode *di_node = (struct dinode*)(ptr + (2*BSIZE));

	//1
	for(int i=0; i < sb->ninodes ; i++)
	{
		if(di_node->type != 0 && di_node->type != 1 && di_node->type != 2 && di_node->type != 3)
		{

			fprintf(stderr, "ERROR: bad inode.\n");
			exit(1);
		}

		di_node++;
	}
	//2
	struct dinode *di_node2 = (struct dinode*)(ptr + (2*BSIZE));
	//uint numBitBlocks = (sb->size -1 + BPB)/BPB;
	//uint inode_blks = (sb->ninodes + IPB - 1) / IPB;
	//uint lowerBound =inode_blks + numBitBlocks + 3 ;
	for(int i=0; i < sb->ninodes ; i++)
	{

		if(di_node2->type !=0)
		{
			if(di_node2->addrs[NDIRECT]!=0)
			{
				for(int j=0 ; j<= NDIRECT ; j++)
				{
					if(di_node2->addrs[j] >= sb->size /*|| di_node2->addrs[j] < lowerBound*/)
					{
						fprintf(stderr,"ERROR: bad direct address in inode.\n");
						exit(1);
					}


				}
			}
		}


		di_node2++;

	}
	/*
	   struct dinode *di_node3 = (struct dinode*)(ptr + (2*BSIZE));
	   for(int i=0; i < sb->ninodes ; i++)
	   {

	   if(di_node3->type !=0)
	   {
	   if(di_node3->addrs[NDIRECT]!=0)
	   {

	   if(di_node3->addrs[NDIRECT] >= sb->size || di_node3->addrs[NDIRECT] < lowerBound)
	   {

	   fprintf(stderr,"ERROR: bad indirect address in inode.\n");
	   exit(1);
	   }



	   }
	   }

	   di_node3++;
	   }
	 */


	struct dinode *di_node3 = (struct dinode*)(ptr + (2*BSIZE));
	for(int i=0; i < sb->ninodes ; i++)
	{
		if(di_node3->type !=0)
		{
			uint *x = (uint*)(ptr + di_node3->addrs[NDIRECT]*BSIZE);

			for(int j =0 ; j< NINDIRECT ; j++)
			{
				if(x[j] >= sb->size /*|| x[j] < lowerBound*/)
				{

					fprintf(stderr,"ERROR: bad indirect address in inode.\n");
					exit(1);

				}
				//x++;
			}
		}
		di_node3++;
	}











	struct dinode *di_node4 = (struct dinode*)(ptr + (2*BSIZE));
	for(int i=0; i < sb->ninodes ; i++)
	{
		if(i==1 && di_node4->type!=1)
		{
			fprintf(stderr,"ERROR: root directory does not exist.\n");
			exit(1);
		}
		di_node4++;
	}


	struct dirent *dr;



	struct dinode *di_node5 = (struct dinode*)(ptr + (2*BSIZE));
	for(int i=0 ; i< sb->ninodes ; i++)
	{
		if(i==1)
		{

			dr  = (struct dirent*) (ptr + di_node5->addrs[0]*BSIZE);

			int a = strcmp(dr->name, ".");
			if(a!=0)
			{
				fprintf(stderr,"ERROR: directory not properly formatted.\n");
				exit(1);
			}

			dr++;

			int b = strcmp(dr->name, "..");
			if(b!=0)
			{
				fprintf(stderr,"ERROR: directory not properly formatted.\n");
				exit(1);
			}
			if(dr->inum!=1 && i==1)
			{
				fprintf(stderr,"ERROR: root directory does not exist.\n");
				exit(1);
			}

		}
		di_node5++;
	}

	//Code to set up an array containing all the data of the bitm

	/*

	   struct dinode *di_node6 = (struct dinode*)(ptr + (2*BSIZE));
	   int direct[sb->nblocks] ;
	   int indirect[sb->nblocks];
	   int direct_count = 0;
	   int indirect_count = 0;
	   for(int i=0 ; i < sb->ninodes ; i++)
	   {
	   if(di_node6->type!=0)
	   {
	   for(int j=0 ; j < NDIRECT ; j++)
	   {
	   if(di_node6->addrs[j]!=0)
	   {

	   data[direct_count] = di_node6->addrs[j];
	   direct_count++;
	   }


	   }





	 */


	struct dinode *di_node6 = (struct dinode*)(ptr + (2*BSIZE));
	uint direct[sbuf.st_size/BSIZE] ;
	for(int i=0 ; i<sbuf.st_size/BSIZE ; i++)
	{
		direct[i] = 0;
	}
	uint indirect[sbuf.st_size/BSIZE];
	for(int i=0 ; i<sbuf.st_size/BSIZE ; i++)
	{
		indirect[i] = 0;
	}
	int direct_count = 0;
	int indirect_count = 0;
	for(uint i=0 ; i < sb->ninodes ; i++)
	{
		if(di_node6->type!=0)
		{
			for(uint j=0 ; j <= NDIRECT ; j++)
			{
				if(di_node6->addrs[j]!=0)
				{

					direct[direct_count] = di_node6->addrs[j];
					direct_count++;
				}

			}
			uint *x = (uint*)(ptr + di_node6->addrs[NDIRECT]*BSIZE);
			for(uint j=0 ; j< NINDIRECT ; j++ )
			{

				if(x[j]!=0)
				{
					indirect[indirect_count] = x[j];
					indirect_count++;
				}
			}



		}

		di_node6++;

	}


	char *bitmap = (char*) (ptr + 3*BSIZE + (sb->ninodes/IPB)*BSIZE);
	uint bit_data[sbuf.st_size/BSIZE];
	for(int i=0 ; i<sbuf.st_size/BSIZE ; i++)
	{
		bit_data[i] = 0;
	}
	uint size = BBLOCK(sbuf.st_size/BSIZE , sb->ninodes);
	for(int i =size+1; i < (sbuf.st_size/BSIZE) ; i++)
	{
		char row = bitmap[i/8];

		if((row >> ((i%8)) & 0x01) == 0)
		{
			bit_data[i] = 0;
		}
		else
		{
			bit_data[i] = 1;
		}

	}


	//5 


	//Checking with direct blocks
	for(int i=0; i< direct_count ; i++)
	{
		uint temp = direct[i];
		if(bit_data[temp] != 1)
		{
			fprintf(stderr, "ERROR: address used by inode but marked free in bitmap.\n");
			exit(1);
		}

	}

	//Checking with indirect blocks

	for(int i=0; i< indirect_count ; i++)
	{
		uint temp = indirect[i];
		if(bit_data[temp] != 1)
		{
			fprintf(stderr, "ERROR: address used by inode but marked free in bitmap.\n");
			exit(1);
		}

	}


	//6

	for(int i =size+1; i < (sbuf.st_size/BSIZE) ; i++)
	{
		if(bit_data[i] == 1)
		{
			int block_number = i;
			int flag = 0;
			for(int j=0 ; j < direct_count ; j++)
			{
				if(direct[j] == block_number)
				{
					flag = 1;
				}
			}

			for(int j=0 ; j< indirect_count ; j++)
			{
				if(indirect[j] == block_number)
				{
					flag = 1;
				}
			}

			if(flag == 0)
			{
				fprintf(stderr, "ERROR: bitmap marks block in use but it is not in use.\n");
				exit(1);
			}


		}
	}



	//7
	for(int i=0 ; i < direct_count ; i++)
	{
		uint compare = direct[i];
		for(int j = 0 ; j< direct_count ; j++)
		{
	                                                                //directory_array[directory_count] = temp;
                                                                //directory_count++;
                                                        //}
                                                //}
                                        

                                
		if(compare == direct[j] && i!=j)
			{
				fprintf(stderr, "ERROR: direct address used more than once.\n");
				exit(1);
			}
		}
	}

	//8

	for(int i=0 ; i < indirect_count ; i++)
	{
		uint compare = indirect[i];
		for(int j = 0 ; j< indirect_count ; j++)
		{
			if(compare == indirect[j] && i!=j)
			{
				fprintf(stderr, "ERROR: indirect address used more than once.\n");
				exit(1);
			}
		}
	}


	//Set up the directory array

	struct dinode *di_node7 = (struct dinode*)(ptr + (2*BSIZE));
	struct dirent *dr2;
	int directory_array[sbuf.st_size/BSIZE] ;
	int directory_count = 0;
	for(int i=0 ; i< sbuf.st_size/BSIZE ; i++)
	{
		directory_array[i] = 0;
	}

	for(int i=0 ; i< sb->ninodes ; i++, di_node7++)
	{
		if(di_node7->type == 1)
		{
			for(int j=0 ; j < NDIRECT ; j++)
			{
				if(di_node7->addrs[j]!=0)
				{

					dr2  = (struct dirent*) (ptr + di_node7->addrs[j]*BSIZE);
					for(int k =0 ; k< BSIZE/16 ; k++)
					{
						if(j==0 && k<2)
							continue;
						if(dr2[k].inum !=0)
						{

							int temp = dr2[k].inum;
							int present =0;
							for(int s =0 ; s<directory_count ; s++)
							{
								if(temp == directory_array[s])
								{
									present = 1;
								}
							}

							if(present ==0)
							{
								directory_array[directory_count] = temp;
								directory_count++;
							}
						}
					}

				}
			}
			if (di_node7->addrs[NDIRECT] == 0)
				continue;
			uint *x = (uint*)(ptr + di_node7->addrs[NDIRECT]*BSIZE);
			//struct dirent *y = (struct dirent*)(ptr + (*x)*BSIZE) ; 
			for(uint j=0 ; j< NINDIRECT ; j++ )
			{

				if(x[j]!=0)
				{
					struct dirent *dr2 = (struct dirent*)(ptr + (x[j])*BSIZE) ;

					//dr2  = (struct diirent*) (ptr + *(y+j)*BSIZE);
					for(int k =0 ; k< BSIZE/16 ; k++)
					{
						if(dr2[k].inum !=0)
						{

							int temp = dr2[k].inum;
							int present =0;
							for(int s =0 ; s<directory_count ; s++)
							{
								if(temp == directory_array[s])
								{
									present = 1;
								}
							}

							if(present ==0)
							{

								directory_array[directory_count] = temp;
								directory_count++;
							}
						}
					}

				}
			}
		}


	}


	//9
	struct dinode *di_node8 = (struct dinode*)(ptr + (2*BSIZE));
	for(int i=0; i<sb->ninodes ; i++)
	{
		if(i>1)
		{
			int flag =0;
			if(di_node8->type !=0)
			{
				for(int j=0 ; j<directory_count ; j++)
				{

					if( i == directory_array[j])
					{

						flag = 1;


					}
				}
				if(flag == 0)
				{
					fprintf(stderr, "ERROR: inode marked use but not found in a directory.\n");
					exit(1);
				}
			}
		}
		di_node8++;
	}



	//10

	//struct dinode *di_node9 = (struct dinode*)(ptr + (2*BSIZE));

	for(int i=0 ; i<directory_count ; i++)
	{

		int a = directory_array[i] ;
		struct dinode *curr = (struct dinode*)( ptr + 2*BSIZE + a*sizeof(struct dinode));
		if(curr->type==0)
		{
			fprintf(stderr, "ERROR: inode referred to in directory but marked free.\n");
			exit(1);


		}
	}
	

	//Set up for 11
	 struct dinode *di_node9 = (struct dinode*)(ptr + (2*BSIZE));
        struct dirent *dr3;
        int file_array[sbuf.st_size/BSIZE] ;

	  int file_count = 0;
        for(int i=0 ; i< sbuf.st_size/BSIZE ; i++)
        {
                file_array[i] = 0;
        }

	 for(int i=0 ; i< sb->ninodes ; i++, di_node9++)
        {
                if(di_node9->type == 1)
                {
                        for(int j=0 ; j < NDIRECT ; j++)
                        {
                                if(di_node9->addrs[j]!=0)
                                {

                                        dr3  = (struct dirent*) (ptr + di_node9->addrs[j]*BSIZE);
                                        for(int k =0 ; k< BSIZE/16 ; k++)
                                        {
                                                if(j==0 && k<2)
                                                        continue;
                                                if(dr3[k].inum !=0)
                                                {

                                                        int temp = dr3[k].inum;
                                                        int present =0;
                                                        for(int s =0 ; s<file_count ; s++)
                                                        {
                                                                if(temp == file_array[s])
                                                                {
                                                                        present = 1;
									file_array[temp]++;
                                                                }
                                                        }

                                                        if(present ==0)
                                                        {
                                                                file_array[temp]++;
                                                                file_count++;
                                                        }
                                                }
                                        }

                                }
                        }
                        if (di_node9->addrs[NDIRECT] == 0)
                                continue;
                        uint *x = (uint*)(ptr + di_node9->addrs[NDIRECT]*BSIZE);
                        //struct dirent *y1 = (struct dirent*)(ptr + (*x)*BSIZE) ;
                        for(uint j=0 ; j< NINDIRECT ; j++ )
                        {

                                if(x[j]!=0)
                                {
                                        struct dirent *dr2 = (struct dirent*)(ptr + (x[j])*BSIZE) ;

                                        //dr2  = (struct diirent*) (ptr + *(y+j)*BSIZE);
                                        for(int k =0 ; k< BSIZE/16 ; k++)
                                        {
                                                if(dr2[k].inum !=0)
                                                {

                                                        int temp = dr2[k].inum;
                                                        int present =0;
                                                        for(int s =0 ; s<file_count ; s++)
                                                        {
                                                                if(temp == file_array[s])
                                                                {
                                                                        present = 1;
									file_array[temp]++;
                                                                }
                                                        }

                                                        if(present ==0)
                                                        {

						                file_array[temp]++;
                                                                file_count++;
                                                        }
                                                }
                                        }

                                }

			}
		}
	}
	//11						
	struct dinode *di_node10 = (struct dinode*)(ptr + (2*BSIZE));
	for(int i=0 ; i<sb->ninodes ; i++)
	{
		if(di_node10->type == 2)
		{
			if(di_node10->nlink != file_array[i])
			{
				fprintf(stderr,"ERROR: bad reference count for file.\n");
				exit(1);
			}
		}
	   di_node10++;
	}		

	//12
	struct dinode *di_node12 = (struct dinode*)(ptr + (2*BSIZE));
	for(int i=0 ; i<sb->ninodes ; i++)
	{
		if(di_node12->type == 1)
		{
			if(file_array[i] > 1)
			{
			   fprintf(stderr,"ERROR: directory appears more than once in file system.\n");
			   exit(1);
		  	}
		}
	   di_node12++;
	}
						








	

	exit(0);

}
