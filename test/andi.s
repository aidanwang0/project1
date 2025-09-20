addi t0, zero, 0xF         # t0 = 1111
andi t1, t0, 0xA           # t1 = 1111 & 1010
.word 0xfeedfeed            # result 0xA