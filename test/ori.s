addi t0, zero, 0x4         # t0 = 0100
ori t1, t0, 0xA           # t1 = 0100 | 1010
.word 0xfeedfeed            # result 0x1110 = 14 = E
