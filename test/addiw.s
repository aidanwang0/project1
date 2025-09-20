lui t0, 0x7FFF           # t0 = 0x7FFF0000
addi t0, t0, 0x7FF       # t0 = 0x7FFFFFFF (adds 0x7FF = 2047)
addi t1, t0, 2           # t1 = 0x80000001
addiw t2, t0, 2          # lower 32 bits: 0x7FFFFFFF + 2 = 0x80000001,  sign-extended to 64-bit = 0xFFFFFFFF80000001
.word 0xfeedfeed          # halt
