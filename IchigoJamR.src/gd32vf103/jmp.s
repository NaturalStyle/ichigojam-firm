##* 
##* created by yrm (c) 2016
##* 

.data
.align 2
L.jmpdata:
    .word   0
    .word   0
    .word   0
    .word   0
    .word   0
    .word   0
    .word   0
    .word   0
    .word   0
    .word   0
    .word   0
    .word   0
    .word   0
    .word   0
    .word   0
    .word   0
    .word   0



.text
.global jmp_set
.type   jmp_set, %function
.align 2
jmp_set:
    lla x5, L.jmpdata

    sw x1, 0(x5)
    sw x2, 4(x5)
    sw x3, 8(x5)
    sw x4, 12(x5)
    sw x8, 16(x5)
    sw x9, 20(x5)
    sw x18, 24(x5)
    sw x19, 28(x5)
    sw x20, 32(x5)
    sw x21, 36(x5)
    sw x22, 40(x5)
    sw x23, 44(x5)
    sw x24, 48(x5)
    sw x25, 52(x5)
    sw x26, 56(x5)
    sw x27, 60(x5)
    
    li a0, 0
    ret



.text
.global jmp_fromint
.type   jmp_fromint, %function
.align 2
jmp_fromint:
    lla t0, L.return
    lw  t0, 0(t0)
    mv  t1, sp
L.more:
    lw   t2, 4(t1)
    beq  t0, t2, L.found
    addi t1, t1, 4
    j    L.more
L.found:
    lla  t0, L.reent
    sw   t0, 80(t1)

    lla x5, L.jmpdata
    sw   a0, 64(x5)

    ret

.align 2
L.reent:
    lla x5, L.jmpdata

    lw x1, 0(x5)
    lw x2, 4(x5)
    lw x3, 8(x5)
    lw x4, 12(x5)
    lw x8, 16(x5)
    lw x9, 20(x5)
    lw x18, 24(x5)
    lw x19, 28(x5)
    lw x20, 32(x5)
    lw x21, 36(x5)
    lw x22, 40(x5)
    lw x23, 44(x5)
    lw x24, 48(x5)
    lw x25, 52(x5)
    lw x26, 56(x5)
    lw x27, 60(x5)

    lw a0, 64(x5)

    ret


.align 2
L.return:
    .word   service_loop


