Creating a page table data struture that replicates the data structure that is used in a computer to store the connections between virtual to physical address. 

To run this program:
1. Download this zipfile from the repository
2. Open zipfile on a environment of your choice
3. Once you have the files on the environment open the terminal
4. Then in the terminal type in "make"
5. This should build object files, like "main.o", for each file and an executable called "pagingwithpr" (ignore the warnings)
6. Now feel free to type in "./pagingwithpr trace.tr 6 8", meaning this is going to create a tree with 2 levels, the first level being able to hold 6 bits and the second level being able to hold 8 bits
7. A summary of the results should output in the terminal

Note:
- To specify that you would like to see the mapping of what virtual addresses are mapped to physcial addresses specify, "./pagingwithpr -l va2pa trace.tr 6 8"
