
## Compile:
```sh
gcc chac.c -o chac
```

## Run:

```sh
./chac [-n nonce] input_file output_file
```

To change the key change line 88.

About ChaC

We implemented a smaller version of ChaCha that uses 256 bits instead of 512 bits. We initially thought this would be more efficient because it's half the size, but we have since realized that it is equally as computationally expensive, as twice the number of blocks will need to be "shuffled".

Thus, we decided to approach the problem by reducing the average number of rounds used to generate a keystream.


Each block will generate 4 x 128-bit keystreams, instead of generating a new 128-bit key with each block. We hope that this change will significantly reduce the computation requirements of the algorithm while remaining relatively secure.


Our block is structured as follows: k is a 128-bit key, ct is a 32-bit counter, n is a 64-bit nonce, and C is a 32-bit constant, fixed to the value of 0x43686143.

---------------------
| k1 | k2 | k3 | k4 |
|----|----|----|----|
| ct | n1 | n2 | C  |
---------------------



Here's a brief description of our new stream cipher.


We can generate a keystream using the following algorithm:

Let Round(a, n) represent the operation of running n rounds on the input a

Block 1 (256-bit matrix)

      k0 = Round(Block 1, 20)

      k1 = Round(k0, 2)

      k2 = Round(k1, 2)

      k3 = Round(k2, 2)



Block 2: (256-bit matrix)

      k4 = Round(Block 2, 20)

      k5 = Round(k4, 2)

      k6 = Round(k5, 2)

      k7 = Round(k6, 2)



      ...



Then, we XOR the generated keystream with the input message to create the cypher text. The example above, consisting of two blocks, generates a 2 * 4 * 256 = 2048-bit keystream. A rough analysis shows that the above algorithm is at least 3 times as performant as ChaCha.



