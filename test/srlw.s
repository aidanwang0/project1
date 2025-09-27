addi t0, zero, 12      # 12 (0b1100)
addi t1, zero, 2       # shift= 2
srlw t2, t0, t1         # t2 = t0 >> 2 = 3 (0b11)

addi t3, zero, -4      # t0 = -4 (0xFFFFFFFF)
addi t4, zero, 1       # shift = 1
srlw t5, t3, t4        # t5 = t3 >> 1  = 0x000000007FFFFFFE (2147483646)

.word 0xfeedfeed        # halt