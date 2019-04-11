#!/bin/bash

make all

echo "Test 01: Disk Initialization > hexdump stored in init_disk.txt"
./test01
hexdump -C ../disk/vdisk > init_disk.txt
echo ""


echo "Test 02: Directory Creation > hexdump stored in directories_disk.txt"
./test02
hexdump -C ../disk/vdisk > directories_disk.txt
echo ""

echo "Test 03: File Writing > hexdump stored in files_disk.txt"
./test03
hexdump -C ../disk/vdisk > files_disk.txt
echo ""

echo "Test 04: File Reading"
./test04
echo "  Testing difference between original files and files read from vdisk:"
echo "      Small sized file diff: "
diff small.txt small_read.txt
echo "      Medium sized file diff: "
diff med.txt med_read.txt
echo "      Large sized file diff: "
diff large.txt large_read.txt
echo ""

echo "Test 05: Directory and File Removal > hexdump stored in removal_disk.txt"
./test05
hexdump -C ../disk/vdisk > removed_disk.txt
echo "  vdisk will now be restored to the initialized state"
echo "  diff for initialized state and state after removals: "
diff init_disk.txt removed_disk.txt
echo ""

echo "Test 06: File Check > file check log stored in check_log.txt"
./test06 > check_log.txt
echo ""

echo "Testing Complete."