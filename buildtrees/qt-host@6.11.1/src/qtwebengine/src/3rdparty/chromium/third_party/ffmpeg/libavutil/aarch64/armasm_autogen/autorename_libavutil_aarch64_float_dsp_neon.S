	AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN


























































































































































































































































































































































































































































































































































































































































































































































































































































































        ;.arch           armv8.6-a+crc


































	;.arch_extension nodotprod
	;.arch_extension noi8mm
	;.arch_extension nosve
	;.arch_extension nosve2



































































































































        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16

        EXPORT ff_vector_fmul_neon





ff_vector_fmul_neon
        










temp_label_0
      subs            w3,  w3,  #16
        ld1             {v0.4s, v1.4s}, [x1], #32
        ld1             {v2.4s, v3.4s}, [x1], #32
        ld1             {v4.4s, v5.4s}, [x2], #32
        ld1             {v6.4s, v7.4s}, [x2], #32
        fmul            v16.4s, v0.4s,  v4.4s
        fmul            v17.4s, v1.4s,  v5.4s
        fmul            v18.4s, v2.4s,  v6.4s
        fmul            v19.4s, v3.4s,  v7.4s
        st1             {v16.4s, v17.4s}, [x0], #32
        st1             {v18.4s, v19.4s}, [x0], #32
        bne            temp_label_0
        ret



        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16

        EXPORT ff_vector_fmac_scalar_neon





ff_vector_fmac_scalar_neon
        










        mov             x3,  #-32
temp_label_1
      subs            w2,  w2,  #16
        ld1             {v16.4s, v17.4s}, [x0], #32
        ld1             {v18.4s, v19.4s}, [x0], x3
        ld1             {v4.4s,  v5.4s},  [x1], #32
        ld1             {v6.4s,  v7.4s},  [x1], #32
        fmla            v16.4s, v4.4s,  v0.s[0]
        fmla            v17.4s, v5.4s,  v0.s[0]
        fmla            v18.4s, v6.4s,  v0.s[0]
        fmla            v19.4s, v7.4s,  v0.s[0]
        st1             {v16.4s, v17.4s}, [x0], #32
        st1             {v18.4s, v19.4s}, [x0], #32
        bne            temp_label_1
        ret



        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16

        EXPORT ff_vector_fmul_scalar_neon





ff_vector_fmul_scalar_neon
        










        mov             w4,  #15
        bics            w3,  w2,  w4
        dup             v16.4s, v0.s[0]
        beq            temp_label_2
        ld1             {v0.4s, v1.4s}, [x1], #32
temp_label_3
      subs            w3,  w3,  #16
        fmul            v0.4s,  v0.4s,  v16.4s
        ld1             {v2.4s, v3.4s}, [x1], #32
        fmul            v1.4s,  v1.4s,  v16.4s
        fmul            v2.4s,  v2.4s,  v16.4s
        st1             {v0.4s, v1.4s}, [x0], #32
        fmul            v3.4s,  v3.4s,  v16.4s
        beq            temp_label_4
        ld1             {v0.4s, v1.4s}, [x1], #32
        st1             {v2.4s, v3.4s}, [x0], #32
        b               temp_label_3
temp_label_5
temp_label_4
      ands            w2,  w2,  #15
        st1             {v2.4s, v3.4s}, [x0], #32
        beq            temp_label_6
temp_label_7
temp_label_2
      ld1             {v0.4s}, [x1], #16
        fmul            v0.4s,  v0.4s,  v16.4s
        st1             {v0.4s}, [x0], #16
        subs            w2,  w2,  #4
        bgt            temp_label_7
temp_label_8
temp_label_6
      ret



        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16

        EXPORT ff_vector_dmul_scalar_neon





ff_vector_dmul_scalar_neon
        










        dup             v16.2d, v0.d[0]
        ld1             {v0.2d, v1.2d}, [x1], #32
temp_label_9
      subs            w2,  w2,  #8
        fmul            v0.2d,  v0.2d,  v16.2d
        ld1             {v2.2d, v3.2d}, [x1], #32
        fmul            v1.2d,  v1.2d,  v16.2d
        fmul            v2.2d,  v2.2d,  v16.2d
        st1             {v0.2d, v1.2d}, [x0], #32
        fmul            v3.2d,  v3.2d,  v16.2d
        ld1             {v0.2d, v1.2d}, [x1], #32
        st1             {v2.2d, v3.2d}, [x0], #32
        bgt            temp_label_9
        ret



        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16

        EXPORT ff_vector_fmul_window_neon





ff_vector_fmul_window_neon
        










        sxtw            x4,  w4                 
        sub             x2,  x2,  #8
        sub             x5,  x4,  #2
        add             x2,  x2,  x5, lsl #2    
        add             x6,  x3,  x5, lsl #3    
        add             x5,  x0,  x5, lsl #3    
        mov             x7,  #-16
        ld1             {v0.4s},  [x1], #16     
        ld1             {v2.4s},  [x3], #16     
        ld1             {v1.4s},  [x2], x7      
temp_label_10
      ld1             {v3.4s},  [x6], x7      
        subs            x4,  x4,  #4
        fmul            v17.4s, v0.4s,  v2.4s   
        rev64           v4.4s,  v1.4s
        rev64           v5.4s,  v3.4s
        rev64           v17.4s, v17.4s
        ext8             v4.16b,  v4.16b,  v4.16b,  #8 
        ext8             v5.16b,  v5.16b,  v5.16b,  #8 
        ext8             v17.16b, v17.16b, v17.16b, #8 
        fmul            v16.4s, v0.4s,  v5.4s  
        fmla            v17.4s, v1.4s,  v3.4s  
        beq            temp_label_11
        ld1             {v0.4s},  [x1], #16
        fmls            v16.4s, v4.4s,  v2.4s  
        st1             {v17.4s}, [x5], x7
        ld1             {v2.4s},  [x3], #16
        ld1             {v1.4s},  [x2], x7
        st1             {v16.4s}, [x0], #16
        b               temp_label_10
temp_label_12
temp_label_11

        fmls            v16.4s, v4.4s,  v2.4s  
        st1             {v17.4s}, [x5], x7
        st1             {v16.4s}, [x0], #16
        ret



        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16

        EXPORT ff_vector_fmul_add_neon





ff_vector_fmul_add_neon
        










        ld1             {v0.4s, v1.4s},  [x1], #32
        ld1             {v2.4s, v3.4s},  [x2], #32
        ld1             {v4.4s, v5.4s},  [x3], #32
temp_label_13
      subs            w4,  w4,  #8
        fmla            v4.4s,  v0.4s,  v2.4s
        fmla            v5.4s,  v1.4s,  v3.4s
        beq            temp_label_14
        ld1             {v0.4s, v1.4s},  [x1], #32
        ld1             {v2.4s, v3.4s},  [x2], #32
        st1             {v4.4s, v5.4s},  [x0], #32
        ld1             {v4.4s, v5.4s},  [x3], #32
        b               temp_label_13
temp_label_15
temp_label_14
      st1             {v4.4s, v5.4s},  [x0], #32
        ret



        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16

        EXPORT ff_vector_fmul_reverse_neon





ff_vector_fmul_reverse_neon
        










        sxtw            x3,  w3
        add             x2,  x2,  x3,  lsl #2
        sub             x2,  x2,  #32
        mov             x4, #-32
        ld1             {v2.4s, v3.4s},  [x2], x4
        ld1             {v0.4s, v1.4s},  [x1], #32
temp_label_16
      subs            x3,  x3,  #8
        rev64           v3.4s,  v3.4s
        rev64           v2.4s,  v2.4s
        ext8             v3.16b, v3.16b, v3.16b,  #8
        ext8             v2.16b, v2.16b, v2.16b,  #8
        fmul            v16.4s, v0.4s,  v3.4s
        fmul            v17.4s, v1.4s,  v2.4s
        beq            temp_label_17
        ld1             {v2.4s, v3.4s},  [x2], x4
        ld1             {v0.4s, v1.4s},  [x1], #32
        st1             {v16.4s, v17.4s},  [x0], #32
        b               temp_label_16
temp_label_18
temp_label_17
      st1             {v16.4s, v17.4s},  [x0], #32
        ret



        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16

        EXPORT ff_butterflies_float_neon





ff_butterflies_float_neon
        










temp_label_19
      ld1             {v0.4s}, [x0]
        ld1             {v1.4s}, [x1]
        subs            w2,  w2,  #4
        fsub            v2.4s,   v0.4s,  v1.4s
        fadd            v3.4s,   v0.4s,  v1.4s
        st1             {v2.4s}, [x1],   #16
        st1             {v3.4s}, [x0],   #16
        bgt            temp_label_19
        ret



        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16

        EXPORT ff_scalarproduct_float_neon





ff_scalarproduct_float_neon
        










        movi v2.4s, #0, lsl #0
temp_label_20
      ld1             {v0.4s}, [x0],   #16
        ld1             {v1.4s}, [x1],   #16
        subs            w2,      w2,     #4
        fmla            v2.4s,   v0.4s,  v1.4s
        bgt            temp_label_20
        faddp           v0.4s,   v2.4s,  v2.4s
        faddp           s0,      v0.2s
        ret




	END
