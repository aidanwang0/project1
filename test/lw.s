val:    .word 5
val2:   .word 10

lw t1, 0(val) #t1= 5
lw t2, 4(val) #t2 =10 

#.word 0xfeedfeed        # halt