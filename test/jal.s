addi t0, zero, 1 #PC = 0
addi t1, zero, 2 #PC = 4
jal t1, skip #PC = 8 rd should = 8, PC should go to skip
addi t0, t0, 0
skip:
    addi t0, t0, 20 # t0 should contain 21
.word 0xfeedfeed
 