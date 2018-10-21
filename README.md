# invert-reflection_thread
The threaded reflection method for finding the inverse matrix using an adjugate matrix.

Use "make" to compile.

Program supports the following command-line arguments:
  * -i input_file_name.txt - name of the input file
  * -n number - number of elements (default = 10)
  * -v - option for debugging
  * -f formula - define formula (choose from { sym ; symnul ; gilb ; 1 ; 9 }
  * -m number - maximum output size (default = 5)
  * -t number - number of threads (default = 1)
  
  The samples of using:
  
  make
  
  ./invert -n 200 -f sym -m 7 -t 8
  
  ./invert -i input.txt -t 2
  
  make clean
