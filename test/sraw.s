lui t0, 0x1       # t0 = 0x0000000100000000
addi t1, zero, 1  

sra t2, t0, t1    # 64-bit shift t2 = 0x0000000080000000
sraw t3, t0, t1   # 32-bit arithmetic shift = 0x00000000 >> 1 = 0

.word 0xfeedfeed   # halt
