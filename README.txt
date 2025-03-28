Create a keystream with 1M bits in ASCII.

Get the actual number of bits in the nist_data.txt file:

wc -m nist_data.txt

Then run the assessment:

./assess <length of keystream>

A keystream length of 1000 is adequate with 1000 sequences.

Enter 0 to select a file input.

Enter nist_data.txt

Do not run all tests. Only run the following to prevent underflow:

Frequency, Block Frequency, Cumulative Sums, Runs, Longest Run, Approximate Entropy, Serial, Linear Complexity.

Now, enter the number of sequences, select ASCII as input, and view test results in Experiments --> Algorithm
Testing --> finalAnalysisReport.txt


