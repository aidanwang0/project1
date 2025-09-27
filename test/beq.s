addi t0, zero, 1 # t0 = 1 (0)
addi t1, zero, 2 # t1 = 2 (4)
bne t0, t1, 4 # actually beq (8)
addi t1, t1, 1  # (12)
.word 0xfeedfeed
