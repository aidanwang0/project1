addi t0, zero, 1 #0 t0 = 1
addi t1, zero, 2 #4 t1 = 2
jalr t1, t1, 0xe #8 # store 12 in t1, jump to 16
addi t0, t0, 1 #12
addi t0, t0, 20 #16
.word 0xfeedfeed
 