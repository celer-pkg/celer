	AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN





















































































































































































































































































































































































































































































































































































































































































































































































































































































        ;.arch           armv8.6-a+crc


































	;.arch_extension nodotprod
	;.arch_extension noi8mm
	;.arch_extension nosve
	;.arch_extension nosve2














































































































































        AREA |.rdata|, DATA, READONLY, ALIGN=5





        ALIGN 16
tbl_rev128_s
        dcb           12, 13, 14, 15
        dcb            8,  9, 10, 11
        dcb            4,  5,  6,  7
        dcb            0,  1,  2,  3




        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16

        EXPORT ff_mpadsp_apply_window_fixed_neon





ff_mpadsp_apply_window_fixed_neon
        










        mov             x7,  x0
        add             x8,  x0,  #512<<2
        ld1             {v0.4s,v1.4s,v2.4s,v3.4s},  [x7],  #64
        ld1             {v4.4s,v5.4s,v6.4s,v7.4s},  [x7],  #64
        st1             {v0.4s,v1.4s,v2.4s,v3.4s},  [x8],  #64
        st1             {v4.4s,v5.4s,v6.4s,v7.4s},  [x8],  #64










        adrp            x15, tbl_rev128_s
        add             x15, x15, tbl_rev128_s










        ld1             {v27.4s}, [x15]
        lsl             x4,  x4,  #1
        add             x10, x0,  #45<<2
        add             x0,  x0,  #16<<2
        add             x1,  x1,  #16<<2
        add             x5,  x3,  x4,  lsl #5
        sub             x5,  x5,  x4            
        neg             x13, x4                 
        mov             x9,  #64<<2
        ld1r            {v16.2s}, [x2]          
        sshll v16.2d, v16.2s, #0
        movi            v29.16b, #0
        movi            v30.2d, #(1<<(16 + 23 - 15))-1
        trn1            v31.2d, v29.2d, v30.2d
        trn2            v30.2d, v30.2d, v29.2d
        trn1            v16.2d, v16.2d, v29.2d
        mov             x14, #4
temp_label_0

        mov             x8,  x0
        sub             x7,  x1,  #3<<2
        sub             x6,  x1,  x14, lsl #4
        add             x7,  x7,  x14, lsl #4
        add             x11, x6, #(32)<<2      
        add             x12, x7, #(32)<<2      
        mov             x15, #8
        movi            v17.16b, #0
        movi            v18.16b, #0
        movi            v19.16b, #0
temp_label_1

        subs            x15, x15, #1
        ld1             {v0.4s},  [x8],  x9
        ld1             {v1.4s},  [x10], x9
        ld1             {v2.4s},  [x6],  x9
        ld1             {v3.4s},  [x7],  x9
        tbl             v6.16b, {v0.16b}, v27.16b
        tbl             v7.16b, {v1.16b}, v27.16b
        ld1             {v4.4s},  [x11], x9
        ld1             {v5.4s},  [x12], x9
        smlal           v16.2d, v2.2s, v0.2s
        smlal2          v17.2d, v2.4s, v0.4s
        smlsl           v18.2d, v3.2s, v6.2s
        smlsl2          v19.2d, v3.4s, v6.4s
        smlsl           v16.2d, v4.2s, v7.2s
        smlsl2          v17.2d, v4.4s, v7.4s
        smlsl           v18.2d, v5.2s, v1.2s
        smlsl2          v19.2d, v5.4s, v1.4s
        bgt            temp_label_1

        cmp             x14, #4
        sub             x10, x10, #64<<5        

        and             v28.16b, v16.16b, v30.16b
        ext8             v28.16b, v29.16b, v28.16b, #8

        beq            temp_label_2
        add             v19.2d, v19.2d, v28.2d
        and             v28.16b,  v19.16b,  v31.16b
temp_label_3
temp_label_2

        add             v16.2d, v16.2d, v28.2d
        and             v28.16b,  v16.16b,  v31.16b
        ext8             v28.16b, v28.16b, v29.16b, #8
        shrn            v16.2s, v16.2d,  #(16 + 23 - 15)
        add             v19.2d, v19.2d, v28.2d
        and             v28.16b,  v19.16b,  v30.16b
        shrn            v19.2s, v19.2d,  #(16 + 23 - 15)
        add             v17.2d, v17.2d, v28.2d
        and             v28.16b,  v17.16b,  v30.16b
        ext8             v28.16b, v29.16b, v28.16b, #8
        add             v18.2d, v18.2d, v28.2d
        and             v28.16b,  v18.16b,  v31.16b
        add             v17.2d, v17.2d, v28.2d
        and             v28.16b,  v17.16b,  v31.16b
        ext8             v28.16b, v28.16b, v29.16b, #8
        shrn2           v16.4s, v17.2d,  #(16 + 23 - 15)
        add             v18.2d, v18.2d, v28.2d
        and             v28.16b,  v18.16b,  v30.16b
        shrn2           v19.4s, v18.2d,  #(16 + 23 - 15)
        sqxtn           v16.4h, v16.4s
        sqxtn           v18.4h, v19.4s

        st1             {v16.h}[0], [x3], x4
        beq            temp_label_4
        st1             {v18.h}[1], [x5], x13
temp_label_5
temp_label_4

        st1             {v16.h}[1], [x3], x4
        st1             {v18.h}[0], [x5], x13
        st1             {v16.h}[2], [x3], x4
        st1             {v18.h}[3], [x5], x13
        st1             {v16.h}[3], [x3], x4
        st1             {v18.h}[2], [x5], x13

        orr v16.16b, v28.16b, v28.16b

        subs            x14, x14, #1
        add             x0,  x0,  #4<<2
        sub             x10, x10, #4<<2
        bgt            temp_label_0


        add             x6,  x1,  #32<<2
        ld1             {v0.2s},  [x6],  x9
        ld1             {v1.2s},  [x0],  x9

        ld1             {v2.2s},  [x6],  x9
        ld1             {v3.2s},  [x0],  x9
        smlsl           v16.2d, v0.2s, v1.2s
        ld1             {v0.2s},  [x6],  x9
        ld1             {v1.2s},  [x0],  x9
        smlsl           v16.2d, v2.2s, v3.2s

        ld1             {v2.2s},  [x6],  x9
        ld1             {v3.2s},  [x0],  x9
        smlsl           v16.2d, v0.2s, v1.2s
        ld1             {v0.2s},  [x6],  x9
        ld1             {v1.2s},  [x0],  x9
        smlsl           v16.2d, v2.2s, v3.2s

        ld1             {v2.2s},  [x6],  x9
        ld1             {v3.2s},  [x0],  x9
        smlsl           v16.2d, v0.2s, v1.2s
        ld1             {v0.2s},  [x6],  x9
        ld1             {v1.2s},  [x0],  x9
        smlsl           v16.2d, v2.2s, v3.2s
        ld1             {v2.2s},  [x6],  x9
        ld1             {v3.2s},  [x0],  x9
        smlsl           v16.2d, v0.2s, v1.2s
        smlsl           v16.2d, v2.2s, v3.2s

        and             v28.16b, v16.16b, v30.16b
        shrn            v20.2s,  v16.2d,  #(16 + 23 - 15)
        xtn             v28.2s,  v28.2d
        sqxtn           v20.4h,  v20.4s
        st1             {v28.s}[0], [x2]        
        st1             {v20.h}[0], [x3]

        ret





        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16

        EXPORT ff_mpadsp_apply_window_float_neon





ff_mpadsp_apply_window_float_neon
        










        mov             x7,  x0
        add             x8,  x0,  #512<<2
        ld1             {v0.4s,v1.4s,v2.4s,v3.4s},  [x7],  #64
        ld1             {v4.4s,v5.4s,v6.4s,v7.4s},  [x7],  #64
        st1             {v0.4s,v1.4s,v2.4s,v3.4s},  [x8],  #64
        st1             {v4.4s,v5.4s,v6.4s,v7.4s},  [x8],  #64










        adrp            x15, tbl_rev128_s
        add             x15, x15, tbl_rev128_s










        ld1             {v27.4s}, [x15]
        lsl             x4,  x4,  #2
        add             x10, x0,  #45<<2
        add             x0,  x0,  #16<<2
        add             x1,  x1,  #16<<2
        add             x5,  x3,  x4,  lsl #5
        sub             x5,  x5,  x4            
        neg             x13, x4                 
        mov             x9,  #64<<2
        movi v16.4s, #0, lsl #0
        movi v28.4s, #0, lsl #0
        mov             x14, #4
temp_label_6

        mov             x8,  x0
        sub             x7,  x1,  #3<<2
        sub             x6,  x1,  x14, lsl #4
        add             x7,  x7,  x14, lsl #4
        add             x11, x6, #(32)<<2      
        add             x12, x7, #(32)<<2      
        mov             x15, #8
        movi            v17.16b, #0
        movi            v18.16b, #0
        movi            v19.16b, #0
temp_label_7

        subs            x15, x15, #1
        ld1             {v0.4s},  [x8],  x9
        ld1             {v1.4s},  [x10], x9
        ld1             {v2.4s},  [x6],  x9
        ld1             {v3.4s},  [x7],  x9
        tbl             v6.16b, {v0.16b}, v27.16b
        tbl             v7.16b, {v1.16b}, v27.16b
        ld1             {v4.4s},  [x11], x9
        ld1             {v5.4s},  [x12], x9
        fmla            v16.4s, v2.4s, v0.4s
        fmls            v18.4s, v3.4s, v6.4s
        fmls            v16.4s, v4.4s, v7.4s
        fmls            v18.4s, v5.4s, v1.4s
        bgt            temp_label_7

        cmp             x14, #4
        sub             x10, x10, #64<<5        

        ext8             v18.16b, v18.16b, v18.16b, #8

        st1             {v16.s}[0], [x3], x4
        beq            temp_label_8
        st1             {v18.s}[1], [x5], x13
temp_label_9
temp_label_8

        st1             {v16.s}[1], [x3], x4
        st1             {v18.s}[0], [x5], x13
        st1             {v16.s}[2], [x3], x4
        st1             {v18.s}[3], [x5], x13
        st1             {v16.s}[3], [x3], x4
        st1             {v18.s}[2], [x5], x13

        orr v16.16b, v28.16b, v28.16b

        subs            x14, x14, #1
        add             x0,  x0,  #4<<2
        sub             x10, x10, #4<<2
        bgt            temp_label_6


        add             x6,  x1,  #32<<2
        ld1             {v0.2s},  [x6],  x9
        ld1             {v1.2s},  [x0],  x9

        ld1             {v2.2s},  [x6],  x9
        ld1             {v3.2s},  [x0],  x9
        fmls            v16.4s, v0.4s, v1.4s
        ld1             {v0.2s},  [x6],  x9
        ld1             {v1.2s},  [x0],  x9
        fmls            v16.4s, v2.4s, v3.4s

        ld1             {v2.2s},  [x6],  x9
        ld1             {v3.2s},  [x0],  x9
        fmls            v16.4s, v0.4s, v1.4s
        ld1             {v0.2s},  [x6],  x9
        ld1             {v1.2s},  [x0],  x9
        fmls            v16.4s, v2.4s, v3.4s

        ld1             {v2.2s},  [x6],  x9
        ld1             {v3.2s},  [x0],  x9
        fmls            v16.4s, v0.4s, v1.4s
        ld1             {v0.2s},  [x6],  x9
        ld1             {v1.2s},  [x0],  x9
        fmls            v16.4s, v2.4s, v3.4s
        ld1             {v2.2s},  [x6],  x9
        ld1             {v3.2s},  [x0],  x9
        fmls            v16.4s, v0.4s, v1.4s
        fmls            v16.4s, v2.4s, v3.4s

        st1             {v16.s}[0], [x3]

        ret



	END
