.intel_syntax noprefix

.section .rodata

.set pbits,1020
.set pbytes,128
.set plimbs,16
p:
    .quad  0xdbe34c5460e36453, 0xa1d81eebbc3d344d, 0x514ba72cb8d89fd3, 0xc2cab6a0e287f1bd
    .quad  0x642aca4d5a313709, 0x6b317c5431541f40, 0xb97c56d1de81ede5, 0x0978dbeed90a2b58
    .quad  0x7611ad4f90441c80, 0xf811d9c419ec8329, 0x4d6c594a8ad82d2d, 0xf06de2471cf9386e
    .quad  0x0683cf25db31ad5b, 0x216c22bc86f21a08, 0xd89dec879007ebd7, 0x0ece55ed427012a9

.inv_min_p_mod_r: /* -p^-1 mod 2^64 */
    .quad 0xd2c2c24160038025

.global fp_0
fp_0:
    .zero pbytes

.global fp_1
fp_1: /* 2^512 mod p */
    .quad  0x65e7ee6590e6567d, 0x40a5f2587fef86d4, 0x99f9e607b99d62f2, 0x1089df50f4f8f26d
    .quad  0x592890dd02bb585a, 0xe1b6be68b969ecb9, 0xaebe3c10395f33c3, 0x5ef9652396531f1b
    .quad  0x28d37db76b7a1b7f, 0x86d089fa474b4a3f, 0xdbce120cc7a4fff2, 0x08b3f947137340ac
    .quad  0x913f3e7c71b37ce5, 0xc7d1b17b09ec4577, 0x9d834aff6f7956b6, 0x044c4b3e968ec2b8

.global fp_four_sqrt_p
fp_four_sqrt_p:
    .quad  0xeba75c5815bb0d57, 0xfec8564a9ae457c6, 0xe362e1c2334bd738, 0x56f74a246ef0a30e
    .quad  0x4a598c9571aeb858, 0xc5617b211ccad355, 0x4fb69e4928ccc442, 0xf643475c7915859c
    .zero 64

.r_squared_mod_p: /* (2^512)^2 mod p */
    .quad  0xd6b8f146ec5055af, 0x68ac5d7707ccb03a, 0x1322c9b9837dca17, 0x4f2940830c1d2b35
    .quad  0x8c1a56e5bf96471a, 0x6cdde00636c4f801, 0x9365ec4fa327c9ac, 0xa0056a67c1de0e82
    .quad  0x8aa6fa7e6811faa8, 0x9aad9631bb760403, 0x156b34c683839b9d, 0xa5ae047480992b2c
    .quad  0xc124d930289048b5, 0x4f8a8344bbe56288, 0xe1a2eb1d838b8237, 0x057162f911ca93a3

.p_minus_2:
    .quad  0xdbe34c5460e36451, 0xa1d81eebbc3d344d, 0x514ba72cb8d89fd3, 0xc2cab6a0e287f1bd
    .quad  0x642aca4d5a313709, 0x6b317c5431541f40, 0xb97c56d1de81ede5, 0x0978dbeed90a2b58
    .quad  0x7611ad4f90441c80, 0xf811d9c419ec8329, 0x4d6c594a8ad82d2d, 0xf06de2471cf9386e
    .quad  0x0683cf25db31ad5b, 0x216c22bc86f21a08, 0xd89dec879007ebd7, 0x0ece55ed427012a9

.p_minus_1_halves:
    .quad  0xedf1a62a3071b229, 0xd0ec0f75de1e9a26, 0xa8a5d3965c6c4fe9, 0xe1655b507143f8de
    .quad  0x32156526ad189b84, 0xb598be2a18aa0fa0, 0x5cbe2b68ef40f6f2, 0x04bc6df76c8515ac
    .quad  0xbb08d6a7c8220e40, 0xfc08ece20cf64194, 0x26b62ca5456c1696, 0xf836f1238e7c9c37
    .quad  0x0341e792ed98d6ad, 0x90b6115e43790d04, 0xec4ef643c803f5eb, 0x07672af6a1380954

.section .data
.global fp_mul_count
fp_mul_count:
    .quad 0

.section .text
.p2align 4,,15

.global fp_copy
fp_copy:
    cld
    mov rcx, plimbs
    rep movsq
    ret

.global fp_set
fp_set:
    push rdi
    call uintbig_set
    pop rdi
    mov rsi, rdi
    jmp fp_enc

.global fp_cswap
fp_cswap:
    movzx rax, dl
    neg rax
    .set k, 0
    .rept plimbs
        mov rcx, [rdi + 8*k]
        mov rdx, [rsi + 8*k]

        mov r8, rcx
        xor r8, rdx
        and r8, rax

        xor rcx, r8
        xor rdx, r8

        mov [rdi + 8*k], rcx
        mov [rsi + 8*k], rdx

        .set k, k+1
    .endr
    ret

.reduce_once:
    push rbp
    mov rbp, rdi

    mov rdi, [rbp +  0]
    sub rdi, [rip + p +  0]
    mov rsi, [rbp +  8]
    sbb rsi, [rip + p +  8]
    mov rdx, [rbp + 16]
    sbb rdx, [rip + p + 16]
    mov rcx, [rbp + 24]
    sbb rcx, [rip + p + 24]
    mov r8,  [rbp + 32]
    sbb r8,  [rip + p + 32]
    mov r9,  [rbp + 40]
    sbb r9,  [rip + p + 40]
    mov r10, [rbp + 48]
    sbb r10, [rip + p + 48]
    mov r11, [rbp + 56]
    sbb r11, [rip + p + 56]

    mov rdi, [rbp + 64]
    sbb rdi, [rip + p + 64]
    mov rsi, [rbp + 72]
    sbb rsi, [rip + p + 72]
    mov rdx, [rbp + 80]
    sbb rdx, [rip + p + 80]
    mov rcx, [rbp + 88]
    sbb rcx, [rip + p + 88]
    mov r8,  [rbp + 96]
    sbb r8,  [rip + p + 96]
    mov r9,  [rbp + 104]
    sbb r9,  [rip + p + 104]
    mov r10, [rbp + 112]
    sbb r10, [rip + p + 112]
    mov r11, [rbp + 120]
    sbb r11, [rip + p + 120]


    setnc al
    movzx rax, al
    neg rax

.macro cswap2, r, m
    xor \r, \m
    and \r, rax
    xor \m, \r
.endm

    cswap2 rdi, [rbp + 64]
    cswap2 rsi, [rbp + 72]
    cswap2 rdx, [rbp + 80]
    cswap2 rcx, [rbp + 88]
    cswap2 r8,  [rbp + 96]
    cswap2 r9,  [rbp + 104]
    cswap2 r10, [rbp + 112]
    cswap2 r11, [rbp + 120]

    mov rdi, [rbp +  0]
    sub rdi, [rip + p +  0]
    mov rsi, [rbp +  8]
    sbb rsi, [rip + p +  8]
    mov rdx, [rbp + 16]
    sbb rdx, [rip + p + 16]
    mov rcx, [rbp + 24]
    sbb rcx, [rip + p + 24]
    mov r8,  [rbp + 32]
    sbb r8,  [rip + p + 32]
    mov r9,  [rbp + 40]
    sbb r9,  [rip + p + 40]
    mov r10, [rbp + 48]
    sbb r10, [rip + p + 48]
    mov r11, [rbp + 56]
    sbb r11, [rip + p + 56]

    cswap2 rdi, [rbp +  0]
    cswap2 rsi, [rbp +  8]
    cswap2 rdx, [rbp + 16]
    cswap2 rcx, [rbp + 24]
    cswap2 r8,  [rbp + 32]
    cswap2 r9,  [rbp + 40]
    cswap2 r10, [rbp + 48]
    cswap2 r11, [rbp + 56]

    pop rbp
    ret

.global fp_add2
fp_add2:
  mov rdx, rdi
.global fp_add3
fp_add3:
  push rdi
  call uintbig_add3
  pop rdi
  jmp .reduce_once

.global fp_sub2
fp_sub2:
  mov rdx, rdi
  xchg rsi, rdx
.global fp_sub3
fp_sub3:
  push rdi
  call uintbig_sub3
  pop rdi
  neg rax

  sub rsp, pbytes

  mov rcx, [rip + p +  0]
  and rcx, rax
  mov [rsp + 0],rcx
  .set k, 1
  .rept plimbs-1
      mov rcx, [rip + p + 8*k]
      and rcx, rax
      mov [rsp + 8*k], rcx
      .set k, k+1
  .endr

  mov rcx, [rsp +  0]
  add rcx, [rdi +  0]
  mov [rdi +  0], rcx
  .set k, 1
  .rept plimbs-1
      mov rcx, [rsp + 8*k]
      adc rcx, [rdi + 8*k]
      mov [rdi + 8*k], rcx
      .set k, k+1
  .endr

  add rsp, pbytes
  ret



/* Montgomery arithmetic */

.global fp_enc
fp_enc:
    lea rdx, [rip + .r_squared_mod_p]
    jmp fp_mul3

.global fp_dec
fp_dec:
    lea rdx, [rip + uintbig_1]
    jmp fp_mul3

.global fp_mul2
fp_mul2:
  mov rdx, rdi
.global fp_mul3
fp_mul3:
  push rbp
  push rbx

  incq fp_mul_count

  sub rsp,144
  mov [rsp+136],rdi
  mov rdi,rsi
  mov rsi,rdx

  /* XXX: put directly into output */
  xor rax,rax
  mov [rsp+0],rax
  mov [rsp+8],rax
  mov [rsp+16],rax
  mov [rsp+24],rax
  mov [rsp+32],rax
  mov [rsp+40],rax
  mov [rsp+48],rax
  mov [rsp+56],rax
  mov [rsp+64],rax
  mov [rsp+72],rax
  mov [rsp+80],rax
  mov [rsp+88],rax
  mov [rsp+96],rax
  mov [rsp+104],rax
  mov [rsp+112],rax
  mov [rsp+120],rax
  mov [rsp+128],rax

.macro MULSTEP, k, I0, I1, I2, I3, I4, I5, I6, I7, I8, I9, I10, I11, I12, I13, I14, I15, I16

    mov r11,[rsp+\I0]
    mov rdx, [rsi +  0]
    mulx rcx, rdx, [rdi + 8*\k]
    add rdx, r11
    mulx rcx, rdx, [rip + .inv_min_p_mod_r]

    xor rax, rax /* clear flags */

    mulx rbx, rax, [rip + p +  0]
    adox r11, rax
    mov [rsp+\I0],r11

    mov r11,[rsp+\I1]
    mulx rcx, rax, [rip + p +  8]
    adcx r11, rbx
    adox r11, rax
    mov [rsp+\I1],r11

    mov r11,[rsp+\I2]
    mulx rbx, rax, [rip + p + 16]
    adcx r11, rcx
    adox r11, rax
    mov [rsp+\I2],r11

    mov r11,[rsp+\I3]
    mulx rcx, rax, [rip + p + 24]
    adcx r11, rbx
    adox r11, rax
    mov [rsp+\I3],r11

    mov r11,[rsp+\I4]
    mulx rbx, rax, [rip + p + 32]
    adcx r11, rcx
    adox r11, rax
    mov [rsp+\I4],r11

    mov r11,[rsp+\I5]
    mulx rcx, rax, [rip + p + 40]
    adcx r11, rbx
    adox r11, rax
    mov [rsp+\I5],r11

    mov r11,[rsp+\I6]
    mulx rbx, rax, [rip + p + 48]
    adcx r11, rcx
    adox r11, rax
    mov [rsp+\I6],r11

    mov r11,[rsp+\I7]
    mulx rcx, rax, [rip + p + 56]
    adcx r11, rbx
    adox r11, rax
    mov [rsp+\I7],r11

    mov r11,[rsp+\I8]
    mulx rbx, rax, [rip + p + 64]
    adcx r11, rcx
    adox r11, rax
    mov [rsp+\I8],r11

    mov r11,[rsp+\I9]
    mulx rcx, rax, [rip + p + 72]
    adcx r11, rbx
    adox r11, rax
    mov [rsp+\I9],r11

    mov r11,[rsp+\I10]
    mulx rbx, rax, [rip + p + 80]
    adcx r11, rcx
    adox r11, rax
    mov [rsp+\I10],r11

    mov r11,[rsp+\I11]
    mulx rcx, rax, [rip + p + 88]
    adcx r11, rbx
    adox r11, rax
    mov [rsp+\I11],r11

    mov r11,[rsp+\I12]
    mulx rbx, rax, [rip + p + 96]
    adcx r11, rcx
    adox r11, rax
    mov [rsp+\I12],r11

    mov r11,[rsp+\I13]
    mulx rcx, rax, [rip + p + 104]
    adcx r11, rbx
    adox r11, rax
    mov [rsp+\I13],r11

    mov r11,[rsp+\I14]
    mulx rbx, rax, [rip + p + 112]
    adcx r11, rcx
    adox r11, rax
    mov [rsp+\I14],r11

    mov r11,[rsp+\I15]
    mulx rcx, rax, [rip + p + 120]
    adcx r11, rbx
    adox r11, rax
    mov [rsp+\I15],r11

    mov r11,[rsp+\I16]
    mov rax, 0
    adcx r11, rcx
    adox r11, rax
    mov [rsp+\I16],r11

    mov rdx, [rdi + 8*\k]

    xor rax, rax /* clear flags */

    mov r11,[rsp+\I0]
    mulx rbx, rax, [rsi +  0]
    adox r11, rax
    mov [rsp+\I0],r11

    mov r11,[rsp+\I1]
    mulx rcx, rax, [rsi +  8]
    adcx r11, rbx
    adox r11, rax
    mov [rsp+\I1],r11

    mov r11,[rsp+\I2]
    mulx rbx, rax, [rsi + 16]
    adcx r11, rcx
    adox r11, rax
    mov [rsp+\I2],r11

    mov r11,[rsp+\I3]
    mulx rcx, rax, [rsi + 24]
    adcx r11, rbx
    adox r11, rax
    mov [rsp+\I3],r11

    mov r11,[rsp+\I4]
    mulx rbx, rax, [rsi + 32]
    adcx r11, rcx
    adox r11, rax
    mov [rsp+\I4],r11

    mov r11,[rsp+\I5]
    mulx rcx, rax, [rsi + 40]
    adcx r11, rbx
    adox r11, rax
    mov [rsp+\I5],r11

    mov r11,[rsp+\I6]
    mulx rbx, rax, [rsi + 48]
    adcx r11, rcx
    adox r11, rax
    mov [rsp+\I6],r11

    mov r11,[rsp+\I7]
    mulx rcx, rax, [rsi + 56]
    adcx r11, rbx
    adox r11, rax
    mov [rsp+\I7],r11

    mov r11,[rsp+\I8]
    mulx rbx, rax, [rsi + 64]
    adcx r11, rcx
    adox r11, rax
    mov [rsp+\I8],r11

    mov r11,[rsp+\I9]
    mulx rcx, rax, [rsi + 72]
    adcx r11, rbx
    adox r11, rax
    mov [rsp+\I9],r11

    mov r11,[rsp+\I10]
    mulx rbx, rax, [rsi + 80]
    adcx r11, rcx
    adox r11, rax
    mov [rsp+\I10],r11

    mov r11,[rsp+\I11]
    mulx rcx, rax, [rsi + 88]
    adcx r11, rbx
    adox r11, rax
    mov [rsp+\I11],r11

    mov r11,[rsp+\I12]
    mulx rbx, rax, [rsi + 96]
    adcx r11, rcx
    adox r11, rax
    mov [rsp+\I12],r11

    mov r11,[rsp+\I13]
    mulx rcx, rax, [rsi + 104]
    adcx r11, rbx
    adox r11, rax
    mov [rsp+\I13],r11

    mov r11,[rsp+\I14]
    mulx rbx, rax, [rsi + 112]
    adcx r11, rcx
    adox r11, rax
    mov [rsp+\I14],r11

    mov r11,[rsp+\I15]
    mulx rcx, rax, [rsi + 120]
    adcx r11, rbx
    adox r11, rax
    mov [rsp+\I15],r11

    mov r11,[rsp+\I16]
    mov rax, 0
    adcx r11, rcx
    adox r11, rax
    mov [rsp+\I16],r11

.endm

    MULSTEP 0, 8, 16, 24, 32, 40, 48, 56, 64, 72, 80, 88, 96, 104, 112, 120, 128, 0
    MULSTEP 1, 16, 24, 32, 40, 48, 56, 64, 72, 80, 88, 96, 104, 112, 120, 128, 0, 8
    MULSTEP 2, 24, 32, 40, 48, 56, 64, 72, 80, 88, 96, 104, 112, 120, 128, 0, 8, 16
    MULSTEP 3, 32, 40, 48, 56, 64, 72, 80, 88, 96, 104, 112, 120, 128, 0, 8, 16, 24
    MULSTEP 4, 40, 48, 56, 64, 72, 80, 88, 96, 104, 112, 120, 128, 0, 8, 16, 24, 32
    MULSTEP 5, 48, 56, 64, 72, 80, 88, 96, 104, 112, 120, 128, 0, 8, 16, 24, 32, 40
    MULSTEP 6, 56, 64, 72, 80, 88, 96, 104, 112, 120, 128, 0, 8, 16, 24, 32, 40, 48
    MULSTEP 7, 64, 72, 80, 88, 96, 104, 112, 120, 128, 0, 8, 16, 24, 32, 40, 48, 56
    MULSTEP 8, 72, 80, 88, 96, 104, 112, 120, 128, 0, 8, 16, 24, 32, 40, 48, 56, 64
    MULSTEP 9, 80, 88, 96, 104, 112, 120, 128, 0, 8, 16, 24, 32, 40, 48, 56, 64, 72
    MULSTEP 10, 88, 96, 104, 112, 120, 128, 0, 8, 16, 24, 32, 40, 48, 56, 64, 72, 80
    MULSTEP 11, 96, 104, 112, 120, 128, 0, 8, 16, 24, 32, 40, 48, 56, 64, 72, 80, 88
    MULSTEP 12, 104, 112, 120, 128, 0, 8, 16, 24, 32, 40, 48, 56, 64, 72, 80, 88, 96
    MULSTEP 13, 112, 120, 128, 0, 8, 16, 24, 32, 40, 48, 56, 64, 72, 80, 88, 96, 104
    MULSTEP 14, 120, 128, 0, 8, 16, 24, 32, 40, 48, 56, 64, 72, 80, 88, 96, 104, 112
    MULSTEP 15, 128, 0, 8, 16, 24, 32, 40, 48, 56, 64, 72, 80, 88, 96, 104, 112, 120

    mov rdi,[rsp+136]

    mov r11,[rsp+0]
    mov [rdi+0],r11
    mov r11,[rsp+8]
    mov [rdi+8],r11
    mov r11,[rsp+16]
    mov [rdi+16],r11
    mov r11,[rsp+24]
    mov [rdi+24],r11
    mov r11,[rsp+32]
    mov [rdi+32],r11
    mov r11,[rsp+40]
    mov [rdi+40],r11
    mov r11,[rsp+48]
    mov [rdi+48],r11
    mov r11,[rsp+56]
    mov [rdi+56],r11
    mov r11,[rsp+64]
    mov [rdi+64],r11
    mov r11,[rsp+72]
    mov [rdi+72],r11
    mov r11,[rsp+80]
    mov [rdi+80],r11
    mov r11,[rsp+88]
    mov [rdi+88],r11
    mov r11,[rsp+96]
    mov [rdi+96],r11
    mov r11,[rsp+104]
    mov [rdi+104],r11
    mov r11,[rsp+112]
    mov [rdi+112],r11
    mov r11,[rsp+120]
    mov [rdi+120],r11

    add rsp,144

    pop rbx
    pop rbp
    jmp .reduce_once

.global fp_sq1
fp_sq1:
    mov rsi, rdi
.global fp_sq2
fp_sq2:
    /* TODO implement optimized Montgomery squaring */
    mov rdx, rsi
    jmp fp_mul3

/* (obviously) not constant time in the exponent! */
.fp_pow:
    push rbx
    mov rbx, rsi
    push r12
    push r13
    push rdi
    sub rsp, pbytes

    mov rsi, rdi
    mov rdi, rsp
    call fp_copy

    mov rdi, [rsp + pbytes]
    lea rsi, [rip + fp_1]
    call fp_copy

.macro POWSTEP, k
        mov r13, [rbx + 8*\k]
        xor r12, r12

        0:
        test r13, 1
        jz 1f

        mov rdi, [rsp + pbytes]
        mov rsi, rsp
        call fp_mul2

        1:
        mov rdi, rsp
        call fp_sq1

        shr r13

        inc r12
        test r12, 64
        jz 0b
.endm

    POWSTEP 0
    POWSTEP 1
    POWSTEP 2
    POWSTEP 3
    POWSTEP 4
    POWSTEP 5
    POWSTEP 6
    POWSTEP 7
    POWSTEP 8
    POWSTEP 9
    POWSTEP 10
    POWSTEP 11
    POWSTEP 12
    POWSTEP 13
    POWSTEP 14
    POWSTEP 15

    add rsp, pbytes+8
    pop r13
    pop r12
    pop rbx
    ret

/* TODO use a better addition chain? */
.global fp_inv
fp_inv:
    lea rsi, [rip + .p_minus_2]
    jmp .fp_pow

/* TODO use a better addition chain? */
.global fp_issquare
fp_issquare:
    push rdi
    lea rsi, [rip + .p_minus_1_halves]
    call .fp_pow
    pop rdi

    xor rax, rax
    .set k, 0
    .rept plimbs
        mov rsi, [rdi + 8*k]
        xor rsi, [rip + fp_1 + 8*k]
        or rax, rsi
        .set k, k+1
    .endr
    test rax, rax
    setz al
    movzx rax, al
    ret

/* not constant time (but this shouldn't leak anything of importance) */
.global fp_random
fp_random:

    push rdi
    mov rsi, pbytes
    call randombytes
    pop rdi
    mov rax, 1
    shl rax, (pbits % 64)
    dec rax
    and [rdi + pbytes-8], rax

    .set k, plimbs-1
    .rept plimbs
        mov rax, [rip + p + 8*k]
        cmp [rdi + 8*k], rax
        jge fp_random
        jl 0f
        .set k, k-1
    .endr
    0:
    ret
