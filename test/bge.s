addi t0, zero, 1
addi t1, zero, -1
bge t0, t1, BRANCH
addi t0, t0, 9
BRANCH:
.word 0xfeedfeed