addi t0, zero, 12      # 12 (0b1100)
srai t2, t0, 2         # t2 = t0 >> 2 = 3 (0b11)

addi t3, zero, -8      # -8 (0xFFFFFFFFFFFFFFF8)
srai t5, t3, 2         # t5 = -8 >> 2 → -2 (0xFFFFFFFFFFFFFFFE)

.word 0xfeedfeed        # halt