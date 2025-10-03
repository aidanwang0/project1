addi a1, zero, 1
slli t1, a1, 63  
# t1 = 0x8000000000000000
addi a3, zero, -1
slli t3, a3, 1 
# t3 = 0xFFFFFFFFFFFFFFFE
addi a5, zero, 3
slli t5, a5, 32  
# t5 = 0x0000000300000000
addi a6, zero, 2
slli t6, a6, 63  
# t6 = 0x0000000000000000
.word 0xfeedfeed

#The first test covers the shift to the edge of the register,
# the second is a shift of a register of all 1s, 
#the third tests behavior between the 8th and 9th bytes, 
#and the last checks if the shift clears the register.