addi t0, zero, 1
addi t1, zero, -1
bgeu t0, t1, BRANCH
addi t2, zero, 1
BRANCH:
.word 0xfeedfeed
