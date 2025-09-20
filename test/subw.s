addi t0, zero, 1           # t0 = 0x00000001
addi t1, zero, 2           # t1 = 0x00000002
subw t2, t0, t1            # t2 = 0xFFFFFFFF (-1 sign-extended to 64-bit)
.word 0xfeedfeed