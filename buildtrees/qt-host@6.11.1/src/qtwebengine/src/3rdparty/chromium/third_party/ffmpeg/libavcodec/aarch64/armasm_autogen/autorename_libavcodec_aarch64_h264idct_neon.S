	AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
























































































































































































































































































































































































































































































































































































































































































































































































































































































        ;.arch           armv8.6-a+crc


































	;.arch_extension nodotprod
	;.arch_extension noi8mm
	;.arch_extension nosve
	;.arch_extension nosve2
































































































































































        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16

        EXPORT ff_h264_idct_add_neon





ff_h264_idct_add_neon
        










L_ff_h264_idct_add_neon
        
        ld1             {v0.4h, v1.4h, v2.4h, v3.4h},  [x1]
        sxtw            x2,     w2
        movi v30.8h, #0, lsl #0

        add             v4.4h,  v0.4h,  v2.4h
        sshr            v16.4h, v1.4h,  #1
        st1             {v30.8h},    [x1], #16
        sshr            v17.4h, v3.4h,  #1
        st1             {v30.8h},    [x1], #16
        sub             v5.4h,  v0.4h,  v2.4h
        sub             v6.4h,  v16.4h, v3.4h
        add             v7.4h,  v1.4h,  v17.4h
        add             v0.4h,  v4.4h,  v7.4h
        add             v1.4h,  v5.4h,  v6.4h
        sub             v2.4h,  v5.4h,  v6.4h
        sub             v3.4h,  v4.4h,  v7.4h

        trn1            v4.4h,  v0.4h,  v1.4h
        trn2            v5.4h,  v0.4h,  v1.4h
        trn1            v6.4h,  v2.4h,  v3.4h
        trn2            v7.4h,  v2.4h,  v3.4h

        trn1            v0.2s,  v4.2s,  v6.2s
        trn2            v2.2s,  v4.2s,  v6.2s
        trn1            v1.2s,  v5.2s,  v7.2s
        trn2            v3.2s,  v5.2s,  v7.2s

        add             v4.4h,  v0.4h,  v2.4h
        ld1             {v18.s}[0], [x0], x2
        sshr            v16.4h,  v3.4h,  #1
        sshr            v17.4h,  v1.4h,  #1
        ld1             {v18.s}[1], [x0], x2
        sub             v5.4h,  v0.4h,  v2.4h
        ld1             {v19.s}[1], [x0], x2
        add             v6.4h,  v16.4h, v1.4h
        ins             v4.d[1],  v5.d[0]
        sub             v7.4h,  v17.4h, v3.4h
        ld1             {v19.s}[0], [x0], x2
        ins             v6.d[1],  v7.d[0]
        sub             x0,  x0,  x2, lsl #2
        add             v0.8h,  v4.8h,  v6.8h
        sub             v1.8h,  v4.8h,  v6.8h

        srshr           v0.8h,  v0.8h,  #6
        srshr           v1.8h,  v1.8h,  #6

        uaddw           v0.8h,  v0.8h,  v18.8b
        uaddw           v1.8h,  v1.8h,  v19.8b

        sqxtun          v0.8b, v0.8h
        sqxtun          v1.8b, v1.8h

        st1             {v0.s}[0],  [x0], x2
        st1             {v0.s}[1],  [x0], x2
        st1             {v1.s}[1],  [x0], x2
        st1             {v1.s}[0],  [x0], x2

        sub             x1,  x1,  #32
        ret



        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16

        EXPORT ff_h264_idct_dc_add_neon





ff_h264_idct_dc_add_neon
        










L_ff_h264_idct_dc_add_neon
        
        sxtw            x2,  w2
        mov             w3,       #0
        ld1r            {v2.8h},  [x1]
        strh            w3,       [x1]
        srshr           v2.8h,  v2.8h,  #6
        ld1             {v0.s}[0],  [x0], x2
        ld1             {v0.s}[1],  [x0], x2
        uaddw           v3.8h,  v2.8h,  v0.8b
        ld1             {v1.s}[0],  [x0], x2
        ld1             {v1.s}[1],  [x0], x2
        uaddw           v4.8h,  v2.8h,  v1.8b
        sqxtun          v0.8b,  v3.8h
        sqxtun          v1.8b,  v4.8h
        sub             x0,  x0,  x2, lsl #2
        st1             {v0.s}[0],  [x0], x2
        st1             {v0.s}[1],  [x0], x2
        st1             {v1.s}[0],  [x0], x2
        st1             {v1.s}[1],  [x0], x2
        ret



        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16

        EXPORT ff_h264_idct_add16_neon





ff_h264_idct_add16_neon
        










        mov             x12, x30
        mov             x6,  x0         
        mov             x5,  x1         
        mov             x1,  x2         
        mov             w9,  w3         










        adrp            x7, scan8
        add             x7, x7, scan8










        mov             x10, #16










        adrp            x13, L_ff_h264_idct_dc_add_neon
        add             x13, x13, L_ff_h264_idct_dc_add_neon




















        adrp            x14, L_ff_h264_idct_add_neon
        add             x14, x14, L_ff_h264_idct_add_neon










temp_label_0
      mov             w2,  w9
        ldrb            w3,  [x7], #1
        ldrsw           x0,  [x5], #4
        ldrb            w3,  [x4,  w3,  uxtw #0]
        subs            w3,  w3,  #1
        blt            temp_label_1
        ldrsh           w3,  [x1]
        add             x0,  x0,  x6
        ccmpeq w3, #0, #4
        cselne x15, x13, x14
        blr             x15
temp_label_2
temp_label_1
      subs            x10, x10, #1
        add             x1,  x1,  #32
        bne            temp_label_0
        ret             x12



        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16

        EXPORT ff_h264_idct_add16intra_neon





ff_h264_idct_add16intra_neon
        










        mov             x12, x30
        mov             x6,  x0         
        mov             x5,  x1         
        mov             x1,  x2         
        mov             w9,  w3         










        adrp            x7, scan8
        add             x7, x7, scan8










        mov             x10, #16










        adrp            x13, L_ff_h264_idct_dc_add_neon
        add             x13, x13, L_ff_h264_idct_dc_add_neon




















        adrp            x14, L_ff_h264_idct_add_neon
        add             x14, x14, L_ff_h264_idct_add_neon










temp_label_3
      mov             w2,  w9
        ldrb            w3,  [x7], #1
        ldrsw           x0,  [x5], #4
        ldrb            w3,  [x4,  w3,  uxtw #0]
        add             x0,  x0,  x6
        cmp             w3,  #0
        ldrsh           w3,  [x1]
        cseleq x15, x13, x14
        ccmpeq w3, #0, #0
        beq            temp_label_4
        blr             x15
temp_label_5
temp_label_4
      subs            x10, x10, #1
        add             x1,  x1,  #32
        bne            temp_label_3
        ret             x12



        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16

        EXPORT ff_h264_idct_add8_neon





ff_h264_idct_add8_neon
        










        stp             x19, x20, [sp, #-0x40]!
        mov             x12, x30
        ldp             x6,  x15, [x0]          
        add             x5,  x1,  #16*4         
        add             x9,  x2,  #16*32        
        mov             w19, w3                 










        adrp            x13, L_ff_h264_idct_dc_add_neon
        add             x13, x13, L_ff_h264_idct_dc_add_neon




















        adrp            x14, L_ff_h264_idct_add_neon
        add             x14, x14, L_ff_h264_idct_add_neon




















        adrp            x7, scan8
        add             x7, x7, scan8
	add x7, x7, #16










        mov             x10, #0
        mov             x11, #16
temp_label_6
      mov             w2,  w19
        ldrb            w3,  [x7, x10]          
        ldrsw           x0,  [x5, x10, lsl #2]  
        ldrb            w3,  [x4, w3,  uxtw #0]    
        add             x0,  x0,  x6            
        add             x1,  x9,  x10, lsl #5   
        cmp             w3,  #0
        ldrsh           w3,  [x1]               
        cseleq x20, x13, x14
        ccmpeq w3, #0, #0
        beq            temp_label_7
        blr             x20
temp_label_8
temp_label_7
      add             x10, x10, #1
        cmp             x10, #4
        cseleq x10, x11, x10     
        cseleq x6, x15, x6
        cmp             x10, #20
        blt            temp_label_6
        ldp             x19, x20, [sp], #0x40
        ret             x12




        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16

        EXPORT ff_h264_idct8_add_neon





ff_h264_idct8_add_neon
        










L_ff_h264_idct8_add_neon
        
        movi v19.8h, #0, lsl #0
        sxtw            x2,       w2
        ld1             {v24.8h, v25.8h}, [x1]
        st1             {v19.8h},  [x1],   #16
        st1             {v19.8h},  [x1],   #16
        ld1             {v26.8h, v27.8h}, [x1]
        st1             {v19.8h},  [x1],   #16
        st1             {v19.8h},  [x1],   #16
        ld1             {v28.8h, v29.8h}, [x1]
        st1             {v19.8h},  [x1],   #16
        st1             {v19.8h},  [x1],   #16

        sshr            v18.8h, v26.8h, #1
        add             v16.8h, v24.8h, v28.8h
        ld1             {v30.8h, v31.8h}, [x1]
        st1             {v19.8h}, [x1],  #16
        st1             {v19.8h}, [x1],  #16
        sub             v17.8h,  v24.8h, v28.8h
        sshr            v19.8h,  v30.8h, #1
        sub             v18.8h,  v18.8h,  v30.8h
        add             v19.8h,  v19.8h,  v26.8h
        add             v26.8h, v17.8h, v18.8h
        sub             v28.8h, v17.8h, v18.8h
        add             v24.8h, v16.8h, v19.8h
        sub             v30.8h,  v16.8h, v19.8h
        sub             v16.8h, v29.8h, v27.8h
        add             v17.8h, v31.8h, v25.8h
        sub             v18.8h,  v31.8h, v25.8h
        add             v19.8h, v29.8h, v27.8h
        sub             v16.8h, v16.8h, v31.8h
        sub             v17.8h, v17.8h, v27.8h
        add             v18.8h,  v18.8h,  v29.8h
        add             v19.8h, v19.8h, v25.8h
        sshr            v25.8h, v25.8h, #1
        sshr            v27.8h, v27.8h, #1
        sshr            v29.8h, v29.8h, #1
        sshr            v31.8h, v31.8h, #1
        sub             v16.8h, v16.8h, v31.8h
        sub             v17.8h, v17.8h, v27.8h
        add             v18.8h,  v18.8h,  v29.8h
        add             v19.8h, v19.8h, v25.8h
        sshr            v25.8h, v16.8h, #2
        sshr            v27.8h, v17.8h, #2
        sshr            v29.8h, v18.8h,  #2
        sshr            v31.8h, v19.8h, #2
        sub             v19.8h, v19.8h, v25.8h
        sub             v18.8h,  v27.8h, v18.8h
        add             v17.8h, v17.8h, v29.8h
        add             v16.8h, v16.8h, v31.8h
        sub             v31.8h, v24.8h, v19.8h
        add             v24.8h, v24.8h, v19.8h
        add             v25.8h, v26.8h, v18.8h
        sub             v18.8h, v26.8h, v18.8h
        add             v26.8h, v28.8h, v17.8h
        add             v27.8h, v30.8h, v16.8h
        sub             v29.8h, v28.8h, v17.8h
        sub             v28.8h, v30.8h, v16.8h
        trn1            v6.8h,  v24.8h,  v25.8h
        trn2            v7.8h,  v24.8h,  v25.8h
        trn1            v25.8h,  v26.8h,  v27.8h
        trn2            v27.8h,  v26.8h,  v27.8h
        trn1            v24.8h,  v28.8h,  v29.8h
        trn2            v29.8h,  v28.8h,  v29.8h
        trn1            v26.8h,  v18.8h,  v31.8h
        trn2            v31.8h,  v18.8h,  v31.8h

        trn1            v28.4s,  v24.4s,  v26.4s
        trn2            v26.4s,  v24.4s,  v26.4s
        trn1            v18.4s,  v29.4s,  v31.4s
        trn2            v31.4s,  v29.4s,  v31.4s
        trn1            v29.4s,  v7.4s,  v27.4s
        trn2            v7.4s,  v7.4s,  v27.4s
        trn1            v27.4s,  v6.4s,  v25.4s
        trn2            v6.4s,  v6.4s,  v25.4s

        trn1            v24.2d,  v27.2d,  v28.2d
        trn2            v28.2d,  v27.2d,  v28.2d

        trn1            v25.2d,  v29.2d,  v18.2d
        trn2            v29.2d,  v29.2d,  v18.2d

        trn2            v18.2d,  v6.2d,  v26.2d
        trn1            v26.2d,  v6.2d,  v26.2d

        trn1            v27.2d,  v7.2d,  v31.2d
        trn2            v31.2d,  v7.2d,  v31.2d

        sshr            v30.8h, v26.8h, #1
        sshr            v19.8h, v18.8h, #1
        add             v16.8h, v24.8h, v28.8h
        sub             v17.8h, v24.8h, v28.8h
        sub             v30.8h, v30.8h, v18.8h
        add             v19.8h, v19.8h, v26.8h
        add             v26.8h, v17.8h, v30.8h
        sub             v28.8h, v17.8h, v30.8h
        add             v24.8h, v16.8h, v19.8h
        sub             v18.8h,  v16.8h, v19.8h
        sub             v16.8h, v29.8h, v27.8h
        add             v17.8h, v31.8h, v25.8h
        sub             v30.8h,  v31.8h, v25.8h
        add             v19.8h, v29.8h, v27.8h
        sub             v16.8h, v16.8h, v31.8h
        sub             v17.8h, v17.8h, v27.8h
        add             v30.8h,  v30.8h,  v29.8h
        add             v19.8h, v19.8h, v25.8h
        sshr            v25.8h, v25.8h, #1
        sshr            v27.8h, v27.8h, #1
        sshr            v29.8h, v29.8h, #1
        sshr            v31.8h, v31.8h, #1
        sub             v16.8h, v16.8h, v31.8h
        sub             v17.8h, v17.8h, v27.8h
        add             v30.8h,  v30.8h,  v29.8h
        add             v19.8h, v19.8h, v25.8h
        sshr            v25.8h, v16.8h, #2
        sshr            v27.8h, v17.8h, #2
        sshr            v29.8h, v30.8h,  #2
        sshr            v31.8h, v19.8h, #2
        sub             v19.8h, v19.8h, v25.8h
        sub             v30.8h,  v27.8h, v30.8h
        add             v17.8h, v17.8h, v29.8h
        add             v16.8h, v16.8h, v31.8h
        sub             v31.8h, v24.8h, v19.8h
        add             v24.8h, v24.8h, v19.8h
        add             v25.8h, v26.8h, v30.8h
        sub             v30.8h, v26.8h, v30.8h
        add             v26.8h, v28.8h, v17.8h
        sub             v29.8h, v28.8h, v17.8h
        add             v27.8h, v18.8h, v16.8h
        sub             v28.8h, v18.8h, v16.8h

        mov             x3,  x0
        srshr           v24.8h, v24.8h, #6
        ld1             {v0.8b},     [x0], x2
        srshr           v25.8h, v25.8h, #6
        ld1             {v1.8b},     [x0], x2
        srshr           v26.8h, v26.8h, #6
        ld1             {v2.8b},     [x0], x2
        srshr           v27.8h, v27.8h, #6
        ld1             {v3.8b},     [x0], x2
        srshr           v28.8h, v28.8h, #6
        ld1             {v4.8b},     [x0], x2
        srshr           v29.8h, v29.8h, #6
        ld1             {v5.8b},     [x0], x2
        srshr           v30.8h, v30.8h, #6
        ld1             {v6.8b},     [x0], x2
        srshr           v31.8h, v31.8h, #6
        ld1             {v7.8b},     [x0], x2
        uaddw           v24.8h, v24.8h, v0.8b
        uaddw           v25.8h, v25.8h, v1.8b
        uaddw           v26.8h, v26.8h, v2.8b
        sqxtun          v0.8b,  v24.8h
        uaddw           v27.8h, v27.8h, v3.8b
        sqxtun          v1.8b,  v25.8h
        uaddw           v28.8h, v28.8h, v4.8b
        sqxtun          v2.8b,  v26.8h
        st1             {v0.8b},     [x3], x2
        uaddw           v29.8h, v29.8h, v5.8b
        sqxtun          v3.8b,  v27.8h
        st1             {v1.8b},     [x3], x2
        uaddw           v30.8h, v30.8h, v6.8b
        sqxtun          v4.8b,  v28.8h
        st1             {v2.8b},     [x3], x2
        uaddw           v31.8h, v31.8h, v7.8b
        sqxtun          v5.8b,  v29.8h
        st1             {v3.8b},     [x3], x2
        sqxtun          v6.8b,  v30.8h
        sqxtun          v7.8b,  v31.8h
        st1             {v4.8b},     [x3], x2
        st1             {v5.8b},     [x3], x2
        st1             {v6.8b},     [x3], x2
        st1             {v7.8b},     [x3], x2

        sub             x1,  x1,  #128
        ret



        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16

        EXPORT ff_h264_idct8_dc_add_neon





ff_h264_idct8_dc_add_neon
        










L_ff_h264_idct8_dc_add_neon
        
        mov             w3,       #0
        sxtw            x2,       w2
        ld1r            {v31.8h}, [x1]
        strh            w3,       [x1]
        ld1             {v0.8b},  [x0], x2
        srshr           v31.8h, v31.8h, #6
        ld1             {v1.8b},     [x0], x2
        ld1             {v2.8b},     [x0], x2
        uaddw           v24.8h, v31.8h, v0.8b
        ld1             {v3.8b},     [x0], x2
        uaddw           v25.8h, v31.8h, v1.8b
        ld1             {v4.8b},     [x0], x2
        uaddw           v26.8h, v31.8h, v2.8b
        ld1             {v5.8b},     [x0], x2
        uaddw           v27.8h, v31.8h, v3.8b
        ld1             {v6.8b},     [x0], x2
        uaddw           v28.8h, v31.8h, v4.8b
        ld1             {v7.8b},     [x0], x2
        uaddw           v29.8h, v31.8h, v5.8b
        uaddw           v30.8h, v31.8h, v6.8b
        uaddw           v31.8h, v31.8h, v7.8b
        sqxtun          v0.8b,  v24.8h
        sqxtun          v1.8b,  v25.8h
        sqxtun          v2.8b,  v26.8h
        sqxtun          v3.8b,  v27.8h
        sub             x0,  x0,  x2, lsl #3
        st1             {v0.8b},     [x0], x2
        sqxtun          v4.8b,  v28.8h
        st1             {v1.8b},     [x0], x2
        sqxtun          v5.8b,  v29.8h
        st1             {v2.8b},     [x0], x2
        sqxtun          v6.8b,  v30.8h
        st1             {v3.8b},     [x0], x2
        sqxtun          v7.8b,  v31.8h
        st1             {v4.8b},     [x0], x2
        st1             {v5.8b},     [x0], x2
        st1             {v6.8b},     [x0], x2
        st1             {v7.8b},     [x0], x2
        ret



        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16

        EXPORT ff_h264_idct8_add4_neon





ff_h264_idct8_add4_neon
        










        mov             x12, x30
        mov             x6,  x0
        mov             x5,  x1
        mov             x1,  x2
        mov             w2,  w3










        adrp            x7, scan8
        add             x7, x7, scan8










        mov             w10, #16










        adrp            x13, L_ff_h264_idct8_dc_add_neon
        add             x13, x13, L_ff_h264_idct8_dc_add_neon




















        adrp            x14, L_ff_h264_idct8_add_neon
        add             x14, x14, L_ff_h264_idct8_add_neon










temp_label_9
      ldrb            w9,  [x7], #4
        ldrsw           x0,  [x5], #16
        ldrb            w9,  [x4, w9, uxtw #0]
        subs            w9,  w9,  #1
        blt            temp_label_10
        ldrsh           w11,  [x1]
        add             x0,  x6,  x0
        ccmpeq w11, #0, #4
        cselne x15, x13, x14
        blr             x15
temp_label_11
temp_label_10
      subs            w10, w10, #4
        add             x1,  x1,  #128
        bne            temp_label_9
        ret             x12










        AREA |.rdata|, DATA, READONLY, ALIGN=5





        ALIGN 16
scan8
        dcb           4+ 1*8, 5+ 1*8, 4+ 2*8, 5+ 2*8
        dcb           6+ 1*8, 7+ 1*8, 6+ 2*8, 7+ 2*8
        dcb           4+ 3*8, 5+ 3*8, 4+ 4*8, 5+ 4*8
        dcb           6+ 3*8, 7+ 3*8, 6+ 4*8, 7+ 4*8
        dcb           4+ 6*8, 5+ 6*8, 4+ 7*8, 5+ 7*8
        dcb           6+ 6*8, 7+ 6*8, 6+ 7*8, 7+ 7*8
        dcb           4+ 8*8, 5+ 8*8, 4+ 9*8, 5+ 9*8
        dcb           6+ 8*8, 7+ 8*8, 6+ 9*8, 7+ 9*8
        dcb           4+11*8, 5+11*8, 4+12*8, 5+12*8
        dcb           6+11*8, 7+11*8, 6+12*8, 7+12*8
        dcb           4+13*8, 5+13*8, 4+14*8, 5+14*8
        dcb           6+13*8, 7+13*8, 6+14*8, 7+14*8



	END
