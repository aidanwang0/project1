addi t0, zero, 1 # t0 = 1 (0)
addi t1, zero, 2 # t1 = 2 (4)
beq t0, t1, skip # skip to 0xfeedfeed
addi t1, t1, 10  # (12)
skip:
.word 0xfeedfeed
