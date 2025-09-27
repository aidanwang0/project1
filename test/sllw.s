addi t0, zero, 12      # 12 (0b1100)
addi t1, zero, 2       # shift= 2
sllw t2, t0, t1         # t2 = t0 << 2 = 48 (0b110000)

addi t3, zero, -8      # -8 (0xFFFFFFFFFFFFFFF8)
addi t4, zero, 2       # shift =2
sllw t5, t3, t4         # t5 = -8 << 2 = (-32)

.word 0xfeedfeed        # halt