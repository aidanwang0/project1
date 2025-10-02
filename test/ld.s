addi t0, zero, -1
addi t1, zero, 0x000007FF
sw t0, 16(zero)
sw t1, 12(zero)     
ld t2, 16(zero)
.word 0xfeedfeed
