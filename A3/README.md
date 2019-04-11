Nathan Marcotte
V00876934
Assignment 3: File Systems

For my implementation of a file system I decided to go with the UNIX file system that has:
- 256 inodes
- 4096 blocks
- 512 bytes per blocks
- 2mb virtual disk

Reserved block:
0 -> Superblock
1 -> Free Block Vector
2 -> inode address map
3-9 -> Unused but Reserved

Each inode will be given it's own block and contains the addresses
for all the blocks that that file/directory points to including the use of
single and double indirect blocks.

The virtual disk (vdiskAPI.c) contains 3 functions, read_block , write_block 
and create_disk. create_disk is only used in the initialization of the disk to 
create the file that will be the virtual disk.

File.c will contain all the functions necessary to make/remove directories,
read/write/remove a file from disk, initialize the disk and all the functions
that interact with the metadata. As well there is a file_check function that can
be run to determine/correct any discrepancies for inodes/blocks be marked as used
if they are not meant to be and vice versa.

The apps folder will contains test files to show that the file system works
properly and a script to run all the test files. 

The key tradeoffs off this implementation are:
Pros
- Less prone to data loss as there is no cache to lose 
- Doesn't require a cleaner 
- No partial overwriting of files
- Very simple
Cons
- Each inode takes a block which wastes space available on disk 
- Much slower as it doesn't make use of a cache 
- Not as practical as LFS due to being slower and wasteful

Testing Parameters:

Test 01: Initializes Disk, results can be viewed in init_disk.txt
Test 02: Creates Following Directories ("~" is the root), hexdump can be viewed in directories_disk.txt
     ~/new
     ~/new/test
     ~/new/test/test2
     ~/csc360
     ~/csc360/assignments
Test 03 : Write the following files to vdisk, hexdump can be viewed in files_disk.txt
    ~/SmallFile                    -> SmallFile points to small.txt, a file that uses only direct blocks
    ~/csc360/MedFile               -> MedFile points to med.txt, a file that uses direct and single indirect blocks
    ~/csc360/assignments/LargeFile -> LargeFile points to large.txt, a file that uses direct, single and double indirect blocks
Test 04 : Read the files that were written to the vdisk in test 3 to files on the real disk 
    Correctness tested by computing diff between ("_read" suffix being the ones read from vdisk and written to real disk)
        small.txt   small_read.txt
        med.txt     med_read.txt
        large.txt   large_read.txt
Test 05: Removes all files and directories created in previous test, restoring disk to initialized state
    Result is stored in removed_disk.txt; 
    removed_disk.txt and init_disk.txt are then used in diff to show that the removal correct
Test 06: Disk runs the file check on the vdisk and stores the check log into check_log.txt

