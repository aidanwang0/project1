addi t1, zero, 6
addi t2, zero, 0
addi t3, zero, 1
loop: 
   addi t2, t2, 1
   bne t2, t1, loop
loop2:
   sub t2, t2, t3
   bne t2, t3, loop2

.word 0xfeedfeed
