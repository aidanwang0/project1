li t1, 0x7FFFFFFF   # x1 = max 32-bit signed
li t2, 1             # x2 = 1
addw t3, t1, t2      # x3 = -0x80000000 (wrap-around, sign-extended)
.word 0xfeedfeed
