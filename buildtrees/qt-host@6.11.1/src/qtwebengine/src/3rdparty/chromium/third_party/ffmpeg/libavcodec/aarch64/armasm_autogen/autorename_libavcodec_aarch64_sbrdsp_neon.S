	AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN





















































































































































































































































































































































































































































































































































































































































































































































































































































































        ;.arch           armv8.6-a+crc


































	;.arch_extension nodotprod
	;.arch_extension noi8mm
	;.arch_extension nosve
	;.arch_extension nosve2










































































































































        AREA |.rdata|, DATA, READONLY, ALIGN=5





        ALIGN 16
factors
        dcfs 1.0, -1.0, 1.0, -1.0









        AREA |.rdata|, DATA, READONLY, ALIGN=5





        ALIGN 16
phi_noise_0
        dcfs 1.0, 0.0, 1.0, 0.0









        AREA |.rdata|, DATA, READONLY, ALIGN=5





        ALIGN 16
phi_noise_1
        dcfs 0.0,  1.0,  0.0, -1.0
        dcfs 0.0, -1.0,  0.0,  1.0









        AREA |.rdata|, DATA, READONLY, ALIGN=5





        ALIGN 16
phi_noise_2
        dcfs -1.0, 0.0, -1.0, 0.0









        AREA |.rdata|, DATA, READONLY, ALIGN=5





        ALIGN 16
phi_noise_3
        dcfs 0.0, -1.0,  0.0,  1.0
        dcfs 0.0,  1.0,  0.0, -1.0


        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16

        EXPORT ff_sbr_sum64x5_neon





ff_sbr_sum64x5_neon
        










        add             x1, x0, #64*4
        add             x2, x0, #128*4
        add             x3, x0, #192*4
        add             x4, x0, #256*4
        mov             x5, #64
temp_label_0
      ld1             {v0.4s}, [x0]
        ld1             {v1.4s}, [x1], #16
        fadd            v0.4s, v0.4s, v1.4s
        ld1             {v2.4s}, [x2], #16
        fadd            v0.4s, v0.4s, v2.4s
        ld1             {v3.4s}, [x3], #16
        fadd            v0.4s, v0.4s, v3.4s
        ld1             {v4.4s}, [x4], #16
        fadd            v0.4s, v0.4s, v4.4s
        st1             {v0.4s}, [x0], #16
        subs            x5, x5, #4
        bgt            temp_label_0
        ret



        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16

        EXPORT ff_sbr_sum_square_neon





ff_sbr_sum_square_neon
        










        movi v0.4s, #0, lsl #0
temp_label_1
      ld1             {v1.4s}, [x0], #16
        fmla            v0.4s, v1.4s, v1.4s
        subs            w1, w1, #2
        bgt            temp_label_1
        faddp           v0.4s, v0.4s, v0.4s
        faddp           v0.4s, v0.4s, v0.4s
        ret



        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16

        EXPORT ff_sbr_neg_odd_64_neon





ff_sbr_neg_odd_64_neon
        










        mov             x1, x0
        movi            v5.4s, #1<<7, lsl #24
        ld2             {v0.4s, v1.4s}, [x0], #32
        eor             v1.16b, v1.16b, v5.16b
        ld2             {v2.4s, v3.4s}, [x0], #32

        st2             {v0.4s, v1.4s}, [x1], #32
        eor             v3.16b, v3.16b, v5.16b
        ld2             {v0.4s, v1.4s}, [x0], #32
        st2             {v2.4s, v3.4s}, [x1], #32
        eor             v1.16b, v1.16b, v5.16b
        ld2             {v2.4s, v3.4s}, [x0], #32

        st2             {v0.4s, v1.4s}, [x1], #32
        eor             v3.16b, v3.16b, v5.16b
        ld2             {v0.4s, v1.4s}, [x0], #32
        st2             {v2.4s, v3.4s}, [x1], #32
        eor             v1.16b, v1.16b, v5.16b
        ld2             {v2.4s, v3.4s}, [x0], #32

        st2             {v0.4s, v1.4s}, [x1], #32
        eor             v3.16b, v3.16b, v5.16b
        ld2             {v0.4s, v1.4s}, [x0], #32
        st2             {v2.4s, v3.4s}, [x1], #32
        eor             v1.16b, v1.16b, v5.16b
        ld2             {v2.4s, v3.4s}, [x0], #32
        eor             v3.16b, v3.16b, v5.16b
        st2             {v0.4s, v1.4s}, [x1], #32
        st2             {v2.4s, v3.4s}, [x1], #32
        ret



        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16

        EXPORT ff_sbr_qmf_pre_shuffle_neon





ff_sbr_qmf_pre_shuffle_neon
        










        add             x1, x0, #60*4
        add             x2, x0, #64*4
        mov             x3, #-16
        mov             x4, #-4
        movi            v6.4s, #1<<7, lsl #24
        ld1             {v0.2s}, [x0], #8
        st1             {v0.2s}, [x2], #8

        ld1             {v1.4s}, [x1], x3
        ld1             {v2.4s}, [x0], #16
        eor             v1.16b, v1.16b, v6.16b
        rev64           v1.4s, v1.4s
        ext8             v1.16b, v1.16b, v1.16b, #8
        st2             {v1.4s, v2.4s}, [x2], #32

        ld1             {v1.4s}, [x1], x3
        ld1             {v2.4s}, [x0], #16
        eor             v1.16b, v1.16b, v6.16b
        rev64           v1.4s, v1.4s
        ext8             v1.16b, v1.16b, v1.16b, #8
        st2             {v1.4s, v2.4s}, [x2], #32

        ld1             {v1.4s}, [x1], x3
        ld1             {v2.4s}, [x0], #16
        eor             v1.16b, v1.16b, v6.16b
        rev64           v1.4s, v1.4s
        ext8             v1.16b, v1.16b, v1.16b, #8
        st2             {v1.4s, v2.4s}, [x2], #32

        ld1             {v1.4s}, [x1], x3
        ld1             {v2.4s}, [x0], #16
        eor             v1.16b, v1.16b, v6.16b
        rev64           v1.4s, v1.4s
        ext8             v1.16b, v1.16b, v1.16b, #8
        st2             {v1.4s, v2.4s}, [x2], #32

        ld1             {v1.4s}, [x1], x3
        ld1             {v2.4s}, [x0], #16
        eor             v1.16b, v1.16b, v6.16b
        rev64           v1.4s, v1.4s
        ext8             v1.16b, v1.16b, v1.16b, #8
        st2             {v1.4s, v2.4s}, [x2], #32

        ld1             {v1.4s}, [x1], x3
        ld1             {v2.4s}, [x0], #16
        eor             v1.16b, v1.16b, v6.16b
        rev64           v1.4s, v1.4s
        ext8             v1.16b, v1.16b, v1.16b, #8
        st2             {v1.4s, v2.4s}, [x2], #32

        ld1             {v1.4s}, [x1], x3
        ld1             {v2.4s}, [x0], #16
        eor             v1.16b, v1.16b, v6.16b
        rev64           v1.4s, v1.4s
        ext8             v1.16b, v1.16b, v1.16b, #8
        st2             {v1.4s, v2.4s}, [x2], #32
        add             x1, x1, #8
        ld1             {v1.2s}, [x1], x4
        ld1             {v2.2s}, [x0], #8
        ld1             {v1.s}[3], [x1]
        ld1             {v2.s}[2], [x0]
        eor             v1.16b, v1.16b, v6.16b
        rev64           v1.4s, v1.4s
        st2             {v1.2s, v2.2s}, [x2], #16
        st2             {v1.s, v2.s}[2], [x2]
        ret



        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16

        EXPORT ff_sbr_qmf_post_shuffle_neon





ff_sbr_qmf_post_shuffle_neon
        










        add             x2, x1, #60*4
        mov             x3, #-16
        mov             x4, #32
        movi            v6.4s, #1<<7, lsl #24
temp_label_2
      ld1             {v0.4s}, [x2], x3
        ld1             {v1.4s}, [x1], #16
        eor             v0.16b, v0.16b, v6.16b
        rev64           v0.4s, v0.4s
        ext8             v0.16b, v0.16b, v0.16b, #8
        st2             {v0.4s, v1.4s}, [x0], #32
        subs            x4, x4, #4
        bgt            temp_label_2
        ret



        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16

        EXPORT ff_sbr_qmf_deint_neg_neon





ff_sbr_qmf_deint_neg_neon
        










        add             x1, x1, #56*4
        add             x2, x0, #60*4
        mov             x3, #-32
        mov             x4, #32
        movi            v2.4s, #1<<7, lsl #24
temp_label_3
      ld2             {v0.4s, v1.4s}, [x1], x3
        eor             v0.16b, v0.16b, v2.16b
        rev64           v1.4s, v1.4s
        ext8             v1.16b, v1.16b, v1.16b, #8
        st1             {v0.4s}, [x2]
        st1             {v1.4s}, [x0], #16
        sub             x2, x2, #16
        subs            x4, x4, #4
        bgt            temp_label_3
        ret



        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16

        EXPORT ff_sbr_qmf_deint_bfly_neon





ff_sbr_qmf_deint_bfly_neon
        










        add             x2, x2, #60*4
        add             x3, x0, #124*4
        mov             x4, #64
        mov             x5, #-16
temp_label_4
      ld1             {v0.4s}, [x1], #16
        ld1             {v1.4s}, [x2], x5
        rev64           v2.4s, v0.4s
        ext8             v2.16b, v2.16b, v2.16b, #8
        rev64           v3.4s, v1.4s
        ext8             v3.16b, v3.16b, v3.16b, #8
        fadd            v1.4s, v1.4s, v2.4s
        fsub            v0.4s, v0.4s, v3.4s
        st1             {v0.4s}, [x0], #16
        st1             {v1.4s}, [x3], x5
        subs            x4, x4, #4
        bgt            temp_label_4
        ret



        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16

        EXPORT ff_sbr_hf_gen_neon





ff_sbr_hf_gen_neon
        










        sxtw            x4, w4
        sxtw            x5, w5










        adrp            x6, factors
        add             x6, x6, factors










        ld1             {v7.4s}, [x6]
        dup             v1.4s, v0.s[0]
        orr v2.8b, v1.8b, v1.8b
        mov             v2.s[2], v7.s[0]
        mov             v2.s[3], v7.s[0]
        fmul            v1.4s, v1.4s, v2.4s
        ld1             {v0.d}[0], [x3]
        ld1             {v0.d}[1], [x2]
        fmul            v0.4s, v0.4s, v1.4s
        fmul            v1.4s, v0.4s, v7.4s
        rev64           v0.4s, v0.4s
        sub             x7, x5, x4
        add             x0, x0, x4, lsl #3
        add             x1, x1, x4, lsl #3
        sub             x1, x1, #16
temp_label_5
      ld1             {v2.4s}, [x1], #16
        ld1             {v3.2s}, [x1]
        fmul            v4.4s, v2.4s, v1.4s
        fmul            v5.4s, v2.4s, v0.4s
        faddp           v4.4s, v4.4s, v4.4s
        faddp           v5.4s, v5.4s, v5.4s
        faddp           v4.4s, v4.4s, v4.4s
        faddp           v5.4s, v5.4s, v5.4s
        mov             v4.s[1], v5.s[0]
        fadd            v4.2s, v4.2s, v3.2s
        st1             {v4.2s}, [x0], #8
        sub             x1, x1, #8
        subs            x7, x7, #1
        bgt            temp_label_5
        ret



        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16

        EXPORT ff_sbr_hf_g_filt_neon





ff_sbr_hf_g_filt_neon
        










        sxtw            x3, w3
        sxtw            x4, w4
        mov             x5, #40*2*4
        add             x1, x1, x4, lsl #3
temp_label_6
      ld1             {v0.2s}, [x1], x5
        ld1             {v1.s}[0], [x2], #4
        fmul            v2.4s, v0.4s, v1.s[0]
        st1             {v2.2s}, [x0], #8
        subs            x3, x3, #1
        bgt            temp_label_6
        ret



        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16

        EXPORT ff_sbr_autocorrelate_neon





ff_sbr_autocorrelate_neon
        










        mov             x2, #38










        adrp            x3, factors
        add             x3, x3, factors










        ld1             {v0.4s}, [x3]
        movi v1.4s, #0, lsl #0
        movi v2.4s, #0, lsl #0
        movi v3.4s, #0, lsl #0
        ld1             {v4.2s}, [x0], #8
        ld1             {v5.2s}, [x0], #8
        fmul            v16.2s, v4.2s, v4.2s
        fmul            v17.2s, v5.2s, v4.s[0]
        fmul            v18.2s, v5.2s, v4.s[1]
temp_label_7
      ld1             {v5.d}[1], [x0], #8
        fmla            v1.2s, v4.2s, v4.2s
        fmla            v2.4s, v5.4s, v4.s[0]
        fmla            v3.4s, v5.4s, v4.s[1]
        mov             v4.d[0], v5.d[0]
        mov             v5.d[0], v5.d[1]
        subs            x2, x2, #1
        bgt            temp_label_7
        fmul            v19.2s, v4.2s, v4.2s
        fmul            v20.2s, v5.2s, v4.s[0]
        fmul            v21.2s, v5.2s, v4.s[1]
        fadd            v22.4s, v2.4s, v20.4s
        fsub            v22.4s, v22.4s, v17.4s
        fadd            v23.4s, v3.4s, v21.4s
        fsub            v23.4s, v23.4s, v18.4s
        rev64           v23.4s, v23.4s
        fmul            v23.4s, v23.4s, v0.4s
        fadd            v22.4s, v22.4s, v23.4s
        st1             {v22.4s}, [x1], #16
        fadd            v23.2s, v1.2s, v19.2s
        fsub            v23.2s, v23.2s, v16.2s
        faddp           v23.2s, v23.2s, v23.2s
        st1             {v23.s}[0], [x1]
        add             x1, x1, #8
        rev64           v3.2s, v3.2s
        fmul            v3.2s, v3.2s, v0.2s
        fadd            v2.2s, v2.2s, v3.2s
        st1             {v2.2s}, [x1]
        add             x1, x1, #16
        faddp           v1.2s, v1.2s, v1.2s
        st1             {v1.s}[0], [x1]
        ret




        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16

        EXPORT ff_sbr_hf_apply_noise_0_neon





ff_sbr_hf_apply_noise_0_neon
        




















        adrp            x9, phi_noise_0
        add             x9, x9, phi_noise_0










        ld1             {v1.4s}, [x9]
        sxtw            x3, w3
        sxtw            x5, w5










        adrp            x7, ff_sbr_noise_table
        add             x7, x7, ff_sbr_noise_table










        add             x3, x3, #1
temp_label_8
      and             x3, x3, #0x1ff
        add             x8, x7, x3, lsl #3
        add             x3, x3, #2
        ld1             {v2.4s}, [x0]
        ld1             {v3.2s}, [x1], #8
        ld1             {v4.2s}, [x2], #8
        ld1             {v5.4s}, [x8]
        orr v6.16b, v2.16b, v2.16b
        zip1            v3.4s, v3.4s, v3.4s
        zip1            v4.4s, v4.4s, v4.4s
        fmla            v6.4s, v1.4s, v3.4s
        fmla            v2.4s, v5.4s, v4.4s
        fcmeq           v7.4s, v3.4s, #0
        bif             v2.16b, v6.16b, v7.16b
        st1             {v2.4s}, [x0], #16
        subs            x5, x5, #2
        bgt            temp_label_8
        ret



        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16

        EXPORT ff_sbr_hf_apply_noise_1_neon





ff_sbr_hf_apply_noise_1_neon
        




















        adrp            x9, phi_noise_1
        add             x9, x9, phi_noise_1










        and             x4, x4, #1
        add             x9, x9, x4, lsl #4
        ld1             {v1.4s}, [x9]
        sxtw            x3, w3
        sxtw            x5, w5










        adrp            x7, ff_sbr_noise_table
        add             x7, x7, ff_sbr_noise_table










        add             x3, x3, #1
temp_label_9
      and             x3, x3, #0x1ff
        add             x8, x7, x3, lsl #3
        add             x3, x3, #2
        ld1             {v2.4s}, [x0]
        ld1             {v3.2s}, [x1], #8
        ld1             {v4.2s}, [x2], #8
        ld1             {v5.4s}, [x8]
        orr v6.16b, v2.16b, v2.16b
        zip1            v3.4s, v3.4s, v3.4s
        zip1            v4.4s, v4.4s, v4.4s
        fmla            v6.4s, v1.4s, v3.4s
        fmla            v2.4s, v5.4s, v4.4s
        fcmeq           v7.4s, v3.4s, #0
        bif             v2.16b, v6.16b, v7.16b
        st1             {v2.4s}, [x0], #16
        subs            x5, x5, #2
        bgt            temp_label_9
        ret



        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16

        EXPORT ff_sbr_hf_apply_noise_2_neon





ff_sbr_hf_apply_noise_2_neon
        




















        adrp            x9, phi_noise_2
        add             x9, x9, phi_noise_2










        ld1             {v1.4s}, [x9]
        sxtw            x3, w3
        sxtw            x5, w5










        adrp            x7, ff_sbr_noise_table
        add             x7, x7, ff_sbr_noise_table










        add             x3, x3, #1
temp_label_10
      and             x3, x3, #0x1ff
        add             x8, x7, x3, lsl #3
        add             x3, x3, #2
        ld1             {v2.4s}, [x0]
        ld1             {v3.2s}, [x1], #8
        ld1             {v4.2s}, [x2], #8
        ld1             {v5.4s}, [x8]
        orr v6.16b, v2.16b, v2.16b
        zip1            v3.4s, v3.4s, v3.4s
        zip1            v4.4s, v4.4s, v4.4s
        fmla            v6.4s, v1.4s, v3.4s
        fmla            v2.4s, v5.4s, v4.4s
        fcmeq           v7.4s, v3.4s, #0
        bif             v2.16b, v6.16b, v7.16b
        st1             {v2.4s}, [x0], #16
        subs            x5, x5, #2
        bgt            temp_label_10
        ret



        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16

        EXPORT ff_sbr_hf_apply_noise_3_neon





ff_sbr_hf_apply_noise_3_neon
        




















        adrp            x9, phi_noise_3
        add             x9, x9, phi_noise_3










        and             x4, x4, #1
        add             x9, x9, x4, lsl #4
        ld1             {v1.4s}, [x9]
        sxtw            x3, w3
        sxtw            x5, w5










        adrp            x7, ff_sbr_noise_table
        add             x7, x7, ff_sbr_noise_table










        add             x3, x3, #1
temp_label_11
      and             x3, x3, #0x1ff
        add             x8, x7, x3, lsl #3
        add             x3, x3, #2
        ld1             {v2.4s}, [x0]
        ld1             {v3.2s}, [x1], #8
        ld1             {v4.2s}, [x2], #8
        ld1             {v5.4s}, [x8]
        orr v6.16b, v2.16b, v2.16b
        zip1            v3.4s, v3.4s, v3.4s
        zip1            v4.4s, v4.4s, v4.4s
        fmla            v6.4s, v1.4s, v3.4s
        fmla            v2.4s, v5.4s, v4.4s
        fcmeq           v7.4s, v3.4s, #0
        bif             v2.16b, v6.16b, v7.16b
        st1             {v2.4s}, [x0], #16
        subs            x5, x5, #2
        bgt            temp_label_11
        ret




	IMPORT ff_sbr_noise_table
	END
