	AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN












































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































        ;.arch           armv8.6-a+crc


































	;.arch_extension nodotprod
	;.arch_extension noi8mm
	;.arch_extension nosve
	;.arch_extension nosve2








































































































































        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16

        EXPORT ff_put_h264_chroma_mc8_neon





ff_put_h264_chroma_mc8_neon
        










        prfm            pldl1strm, [x1]
        prfm            pldl1strm, [x1, x2]
        mul             w7,  w4,  w5
        lsl             w14, w5,  #3
        lsl             w13, w4,  #3
        cmp             w7,  #0
        sub             w6,  w14, w7
        sub             w12, w13, w7
        sub             w4,  w7,  w13
        sub             w4,  w4,  w14
        add             w4,  w4,  #64
        beq            temp_label_0

        dup             v0.8b,  w4
        dup             v1.8b,  w12
        ld1             {v4.8b, v5.8b}, [x1], x2
        dup             v2.8b,  w6
        dup             v3.8b,  w7
        ext8             v5.8b,  v4.8b,  v5.8b,  #1
temp_label_1
      ld1             {v6.8b, v7.8b}, [x1], x2
        umull           v16.8h, v4.8b,  v0.8b
        umlal           v16.8h, v5.8b,  v1.8b
        ext8             v7.8b,  v6.8b,  v7.8b,  #1
        ld1             {v4.8b, v5.8b}, [x1], x2
        umlal           v16.8h, v6.8b,  v2.8b
        prfm            pldl1strm, [x1]
        ext8             v5.8b,  v4.8b,  v5.8b,  #1
        umlal           v16.8h, v7.8b,  v3.8b
        umull           v17.8h, v6.8b,  v0.8b
        subs            w3,  w3,  #2
        umlal           v17.8h, v7.8b, v1.8b
        umlal           v17.8h, v4.8b, v2.8b
        umlal           v17.8h, v5.8b, v3.8b
        prfm            pldl1strm, [x1, x2]
        rshrn           v16.8b, v16.8h, #6
        rshrn           v17.8b, v17.8h, #6
        st1             {v16.8b}, [x0], x2
        st1             {v17.8b}, [x0], x2
        bgt            temp_label_1
        ret

temp_label_2
temp_label_0
      adds            w12, w12, w6
        dup             v0.8b, w4
        beq            temp_label_3
        tst             w6,  w6
        dup             v1.8b, w12
        beq            temp_label_4

        ld1             {v4.8b}, [x1], x2
temp_label_5
      ld1             {v6.8b}, [x1], x2
        umull           v16.8h, v4.8b,  v0.8b
        umlal           v16.8h, v6.8b,  v1.8b
        ld1             {v4.8b}, [x1], x2
        umull           v17.8h, v6.8b,  v0.8b
        umlal           v17.8h, v4.8b,  v1.8b
        prfm            pldl1strm, [x1]
        rshrn           v16.8b, v16.8h, #6
        rshrn           v17.8b, v17.8h, #6
        prfm            pldl1strm, [x1, x2]
        subs            w3,  w3,  #2
        st1             {v16.8b}, [x0], x2
        st1             {v17.8b}, [x0], x2
        bgt            temp_label_5
        ret

temp_label_6
temp_label_4
      ld1             {v4.8b, v5.8b}, [x1], x2
        ld1             {v6.8b, v7.8b}, [x1], x2
        ext8             v5.8b,  v4.8b,  v5.8b,  #1
        ext8             v7.8b,  v6.8b,  v7.8b,  #1
        prfm            pldl1strm, [x1]
        subs            w3,  w3,  #2
        umull           v16.8h, v4.8b, v0.8b
        umlal           v16.8h, v5.8b, v1.8b
        umull           v17.8h, v6.8b, v0.8b
        umlal           v17.8h, v7.8b, v1.8b
        prfm            pldl1strm, [x1, x2]
        rshrn           v16.8b, v16.8h, #6
        rshrn           v17.8b, v17.8h, #6
        st1             {v16.8b}, [x0], x2
        st1             {v17.8b}, [x0], x2
        bgt            temp_label_6
        ret

temp_label_7
temp_label_3
      ld1             {v4.8b}, [x1], x2
        ld1             {v5.8b}, [x1], x2
        prfm            pldl1strm, [x1]
        subs            w3,  w3,  #2
        umull           v16.8h, v4.8b, v0.8b
        umull           v17.8h, v5.8b, v0.8b
        prfm            pldl1strm, [x1, x2]
        rshrn           v16.8b, v16.8h, #6
        rshrn           v17.8b, v17.8h, #6
        st1             {v16.8b}, [x0], x2
        st1             {v17.8b}, [x0], x2
        bgt            temp_label_7
        ret


        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16

        EXPORT ff_avg_h264_chroma_mc8_neon





ff_avg_h264_chroma_mc8_neon
        










        mov             x8,  x0
        prfm            pldl1strm, [x1]
        prfm            pldl1strm, [x1, x2]
        mul             w7,  w4,  w5
        lsl             w14, w5,  #3
        lsl             w13, w4,  #3
        cmp             w7,  #0
        sub             w6,  w14, w7
        sub             w12, w13, w7
        sub             w4,  w7,  w13
        sub             w4,  w4,  w14
        add             w4,  w4,  #64
        beq            temp_label_8

        dup             v0.8b,  w4
        dup             v1.8b,  w12
        ld1             {v4.8b, v5.8b}, [x1], x2
        dup             v2.8b,  w6
        dup             v3.8b,  w7
        ext8             v5.8b,  v4.8b,  v5.8b,  #1
temp_label_9
      ld1             {v6.8b, v7.8b}, [x1], x2
        umull           v16.8h, v4.8b,  v0.8b
        umlal           v16.8h, v5.8b,  v1.8b
        ext8             v7.8b,  v6.8b,  v7.8b,  #1
        ld1             {v4.8b, v5.8b}, [x1], x2
        umlal           v16.8h, v6.8b,  v2.8b
        prfm            pldl1strm, [x1]
        ext8             v5.8b,  v4.8b,  v5.8b,  #1
        umlal           v16.8h, v7.8b,  v3.8b
        umull           v17.8h, v6.8b,  v0.8b
        subs            w3,  w3,  #2
        umlal           v17.8h, v7.8b, v1.8b
        umlal           v17.8h, v4.8b, v2.8b
        umlal           v17.8h, v5.8b, v3.8b
        prfm            pldl1strm, [x1, x2]
        rshrn           v16.8b, v16.8h, #6
        rshrn           v17.8b, v17.8h, #6
        ld1             {v20.8b}, [x8], x2
        ld1             {v21.8b}, [x8], x2
        urhadd          v16.8b, v16.8b, v20.8b
        urhadd          v17.8b, v17.8b, v21.8b
        st1             {v16.8b}, [x0], x2
        st1             {v17.8b}, [x0], x2
        bgt            temp_label_9
        ret

temp_label_10
temp_label_8
      adds            w12, w12, w6
        dup             v0.8b, w4
        beq            temp_label_11
        tst             w6,  w6
        dup             v1.8b, w12
        beq            temp_label_12

        ld1             {v4.8b}, [x1], x2
temp_label_13
      ld1             {v6.8b}, [x1], x2
        umull           v16.8h, v4.8b,  v0.8b
        umlal           v16.8h, v6.8b,  v1.8b
        ld1             {v4.8b}, [x1], x2
        umull           v17.8h, v6.8b,  v0.8b
        umlal           v17.8h, v4.8b,  v1.8b
        prfm            pldl1strm, [x1]
        rshrn           v16.8b, v16.8h, #6
        rshrn           v17.8b, v17.8h, #6
        prfm            pldl1strm, [x1, x2]
        ld1             {v20.8b}, [x8], x2
        ld1             {v21.8b}, [x8], x2
        urhadd          v16.8b, v16.8b, v20.8b
        urhadd          v17.8b, v17.8b, v21.8b
        subs            w3,  w3,  #2
        st1             {v16.8b}, [x0], x2
        st1             {v17.8b}, [x0], x2
        bgt            temp_label_13
        ret

temp_label_14
temp_label_12
      ld1             {v4.8b, v5.8b}, [x1], x2
        ld1             {v6.8b, v7.8b}, [x1], x2
        ext8             v5.8b,  v4.8b,  v5.8b,  #1
        ext8             v7.8b,  v6.8b,  v7.8b,  #1
        prfm            pldl1strm, [x1]
        subs            w3,  w3,  #2
        umull           v16.8h, v4.8b, v0.8b
        umlal           v16.8h, v5.8b, v1.8b
        umull           v17.8h, v6.8b, v0.8b
        umlal           v17.8h, v7.8b, v1.8b
        prfm            pldl1strm, [x1, x2]
        rshrn           v16.8b, v16.8h, #6
        rshrn           v17.8b, v17.8h, #6
        ld1             {v20.8b}, [x8], x2
        ld1             {v21.8b}, [x8], x2
        urhadd          v16.8b, v16.8b, v20.8b
        urhadd          v17.8b, v17.8b, v21.8b
        st1             {v16.8b}, [x0], x2
        st1             {v17.8b}, [x0], x2
        bgt            temp_label_14
        ret

temp_label_15
temp_label_11
      ld1             {v4.8b}, [x1], x2
        ld1             {v5.8b}, [x1], x2
        prfm            pldl1strm, [x1]
        subs            w3,  w3,  #2
        umull           v16.8h, v4.8b, v0.8b
        umull           v17.8h, v5.8b, v0.8b
        prfm            pldl1strm, [x1, x2]
        rshrn           v16.8b, v16.8h, #6
        rshrn           v17.8b, v17.8h, #6
        ld1             {v20.8b}, [x8], x2
        ld1             {v21.8b}, [x8], x2
        urhadd          v16.8b, v16.8b, v20.8b
        urhadd          v17.8b, v17.8b, v21.8b
        st1             {v16.8b}, [x0], x2
        st1             {v17.8b}, [x0], x2
        bgt            temp_label_15
        ret


        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16

        EXPORT ff_put_h264_chroma_mc4_neon





ff_put_h264_chroma_mc4_neon
        










        prfm            pldl1strm, [x1]
        prfm            pldl1strm, [x1, x2]
        mul             w7,  w4,  w5
        lsl             w14, w5,  #3
        lsl             w13, w4,  #3
        cmp             w7,  #0
        sub             w6,  w14, w7
        sub             w12, w13, w7
        sub             w4,  w7,  w13
        sub             w4,  w4,  w14
        add             w4,  w4,  #64
        beq            temp_label_16

        dup             v24.8b,  w4
        dup             v25.8b,  w12
        ld1             {v4.8b}, [x1], x2
        dup             v26.8b,  w6
        dup             v27.8b,  w7
        ext8             v5.8b,  v4.8b,  v5.8b, #1
        trn1            v0.2s,  v24.2s, v25.2s
        trn1            v2.2s,  v26.2s, v27.2s
        trn1            v4.2s,  v4.2s,  v5.2s
temp_label_17
      ld1             {v6.8b}, [x1], x2
        ext8             v7.8b,  v6.8b,  v7.8b, #1
        trn1            v6.2s,  v6.2s,  v7.2s
        umull           v18.8h, v4.8b,  v0.8b
        umlal           v18.8h, v6.8b,  v2.8b
        ld1             {v4.8b}, [x1], x2
        ext8             v5.8b,  v4.8b,  v5.8b, #1
        trn1            v4.2s,  v4.2s,  v5.2s
        prfm            pldl1strm, [x1]
        umull           v19.8h, v6.8b,  v0.8b
        umlal           v19.8h, v4.8b,  v2.8b
        trn1            v30.2d, v18.2d, v19.2d
        trn2            v31.2d, v18.2d, v19.2d
        add             v18.8h, v30.8h, v31.8h
        rshrn           v16.8b, v18.8h, #6
        subs            w3,  w3,  #2
        prfm            pldl1strm, [x1, x2]
        st1             {v16.s}[0], [x0], x2
        st1             {v16.s}[1], [x0], x2
        bgt            temp_label_17
        ret

temp_label_18
temp_label_16
      adds            w12, w12, w6
        dup             v30.8b, w4
        beq            temp_label_19
        tst             w6,  w6
        dup             v31.8b, w12
        trn1            v0.2s,  v30.2s, v31.2s
        trn2            v1.2s,  v30.2s, v31.2s
        beq            temp_label_20

        ext8             v1.8b,  v0.8b,  v1.8b, #4
        ld1             {v4.s}[0], [x1], x2
temp_label_21
      ld1             {v4.s}[1], [x1], x2
        umull           v18.8h, v4.8b,  v0.8b
        ld1             {v4.s}[0], [x1], x2
        umull           v19.8h, v4.8b,  v1.8b
        trn1            v30.2d, v18.2d, v19.2d
        trn2            v31.2d, v18.2d, v19.2d
        add             v18.8h, v30.8h, v31.8h
        prfm            pldl1strm, [x1]
        rshrn           v16.8b, v18.8h, #6
        subs            w3,  w3,  #2
        prfm            pldl1strm, [x1, x2]
        st1             {v16.s}[0], [x0], x2
        st1             {v16.s}[1], [x0], x2
        bgt            temp_label_21
        ret

temp_label_22
temp_label_20
      ld1             {v4.8b}, [x1], x2
        ld1             {v6.8b}, [x1], x2
        ext8             v5.8b,  v4.8b,  v5.8b, #1
        ext8             v7.8b,  v6.8b,  v7.8b, #1
        trn1            v4.2s,  v4.2s,  v5.2s
        trn1            v6.2s,  v6.2s,  v7.2s
        umull           v18.8h, v4.8b,  v0.8b
        umull           v19.8h, v6.8b,  v0.8b
        subs            w3,  w3,  #2
        trn1            v30.2d, v18.2d, v19.2d
        trn2            v31.2d, v18.2d, v19.2d
        add             v18.8h, v30.8h, v31.8h
        prfm            pldl1strm, [x1]
        rshrn           v16.8b, v18.8h, #6
        prfm            pldl1strm, [x1]
        st1             {v16.s}[0], [x0], x2
        st1             {v16.s}[1], [x0], x2
        bgt            temp_label_22
        ret

temp_label_23
temp_label_19
      ld1             {v4.s}[0], [x1], x2
        ld1             {v4.s}[1], [x1], x2
        umull           v18.8h, v4.8b,  v30.8b
        subs            w3,  w3,  #2
        prfm            pldl1strm, [x1]
        rshrn           v16.8b, v18.8h, #6
        prfm            pldl1strm, [x1]
        st1             {v16.s}[0], [x0], x2
        st1             {v16.s}[1], [x0], x2
        bgt            temp_label_23
        ret


        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16

        EXPORT ff_avg_h264_chroma_mc4_neon





ff_avg_h264_chroma_mc4_neon
        










        mov             x8,  x0
        prfm            pldl1strm, [x1]
        prfm            pldl1strm, [x1, x2]
        mul             w7,  w4,  w5
        lsl             w14, w5,  #3
        lsl             w13, w4,  #3
        cmp             w7,  #0
        sub             w6,  w14, w7
        sub             w12, w13, w7
        sub             w4,  w7,  w13
        sub             w4,  w4,  w14
        add             w4,  w4,  #64
        beq            temp_label_24

        dup             v24.8b,  w4
        dup             v25.8b,  w12
        ld1             {v4.8b}, [x1], x2
        dup             v26.8b,  w6
        dup             v27.8b,  w7
        ext8             v5.8b,  v4.8b,  v5.8b, #1
        trn1            v0.2s,  v24.2s, v25.2s
        trn1            v2.2s,  v26.2s, v27.2s
        trn1            v4.2s,  v4.2s,  v5.2s
temp_label_25
      ld1             {v6.8b}, [x1], x2
        ext8             v7.8b,  v6.8b,  v7.8b, #1
        trn1            v6.2s,  v6.2s,  v7.2s
        umull           v18.8h, v4.8b,  v0.8b
        umlal           v18.8h, v6.8b,  v2.8b
        ld1             {v4.8b}, [x1], x2
        ext8             v5.8b,  v4.8b,  v5.8b, #1
        trn1            v4.2s,  v4.2s,  v5.2s
        prfm            pldl1strm, [x1]
        umull           v19.8h, v6.8b,  v0.8b
        umlal           v19.8h, v4.8b,  v2.8b
        trn1            v30.2d, v18.2d, v19.2d
        trn2            v31.2d, v18.2d, v19.2d
        add             v18.8h, v30.8h, v31.8h
        rshrn           v16.8b, v18.8h, #6
        subs            w3,  w3,  #2
        prfm            pldl1strm, [x1, x2]
        ld1             {v20.s}[0], [x8], x2
        ld1             {v20.s}[1], [x8], x2
        urhadd          v16.8b, v16.8b, v20.8b
        st1             {v16.s}[0], [x0], x2
        st1             {v16.s}[1], [x0], x2
        bgt            temp_label_25
        ret

temp_label_26
temp_label_24
      adds            w12, w12, w6
        dup             v30.8b, w4
        beq            temp_label_27
        tst             w6,  w6
        dup             v31.8b, w12
        trn1            v0.2s,  v30.2s, v31.2s
        trn2            v1.2s,  v30.2s, v31.2s
        beq            temp_label_28

        ext8             v1.8b,  v0.8b,  v1.8b, #4
        ld1             {v4.s}[0], [x1], x2
temp_label_29
      ld1             {v4.s}[1], [x1], x2
        umull           v18.8h, v4.8b,  v0.8b
        ld1             {v4.s}[0], [x1], x2
        umull           v19.8h, v4.8b,  v1.8b
        trn1            v30.2d, v18.2d, v19.2d
        trn2            v31.2d, v18.2d, v19.2d
        add             v18.8h, v30.8h, v31.8h
        prfm            pldl1strm, [x1]
        rshrn           v16.8b, v18.8h, #6
        ld1             {v20.s}[0], [x8], x2
        ld1             {v20.s}[1], [x8], x2
        urhadd          v16.8b, v16.8b, v20.8b
        subs            w3,  w3,  #2
        prfm            pldl1strm, [x1, x2]
        st1             {v16.s}[0], [x0], x2
        st1             {v16.s}[1], [x0], x2
        bgt            temp_label_29
        ret

temp_label_30
temp_label_28
      ld1             {v4.8b}, [x1], x2
        ld1             {v6.8b}, [x1], x2
        ext8             v5.8b,  v4.8b,  v5.8b, #1
        ext8             v7.8b,  v6.8b,  v7.8b, #1
        trn1            v4.2s,  v4.2s,  v5.2s
        trn1            v6.2s,  v6.2s,  v7.2s
        umull           v18.8h, v4.8b,  v0.8b
        umull           v19.8h, v6.8b,  v0.8b
        subs            w3,  w3,  #2
        trn1            v30.2d, v18.2d, v19.2d
        trn2            v31.2d, v18.2d, v19.2d
        add             v18.8h, v30.8h, v31.8h
        prfm            pldl1strm, [x1]
        rshrn           v16.8b, v18.8h, #6
        ld1             {v20.s}[0], [x8], x2
        ld1             {v20.s}[1], [x8], x2
        urhadd          v16.8b, v16.8b, v20.8b
        prfm            pldl1strm, [x1]
        st1             {v16.s}[0], [x0], x2
        st1             {v16.s}[1], [x0], x2
        bgt            temp_label_30
        ret

temp_label_31
temp_label_27
      ld1             {v4.s}[0], [x1], x2
        ld1             {v4.s}[1], [x1], x2
        umull           v18.8h, v4.8b,  v30.8b
        subs            w3,  w3,  #2
        prfm            pldl1strm, [x1]
        rshrn           v16.8b, v18.8h, #6
        ld1             {v20.s}[0], [x8], x2
        ld1             {v20.s}[1], [x8], x2
        urhadd          v16.8b, v16.8b, v20.8b
        prfm            pldl1strm, [x1]
        st1             {v16.s}[0], [x0], x2
        st1             {v16.s}[1], [x0], x2
        bgt            temp_label_31
        ret


        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16

        EXPORT ff_put_h264_chroma_mc2_neon





ff_put_h264_chroma_mc2_neon
        










        prfm            pldl1strm, [x1]
        prfm            pldl1strm, [x1, x2]
        orr             w7,  w4,  w5
        cbz             w7,  temp_label_32

        mul             w7,  w4,  w5
        lsl             w14, w5,  #3
        lsl             w13, w4,  #3
        sub             w6,  w14, w7
        sub             w12, w13, w7
        sub             w4,  w7,  w13
        sub             w4,  w4,  w14
        add             w4,  w4,  #64
        dup             v0.8b,  w4
        dup             v2.8b,  w12
        dup             v1.8b,  w6
        dup             v3.8b,  w7
        trn1            v0.4h,  v0.4h,  v2.4h
        trn1            v1.4h,  v1.4h,  v3.4h
temp_label_33

        ld1             {v4.s}[0],  [x1], x2
        ld1             {v4.s}[1],  [x1], x2
        rev64           v5.2s,  v4.2s
        ld1             {v5.s}[1],  [x1]
        ext8             v6.8b,  v4.8b,  v5.8b,  #1
        ext8             v7.8b,  v5.8b,  v4.8b,  #1
        trn1            v4.4h,  v4.4h,  v6.4h
        trn1            v5.4h,  v5.4h,  v7.4h
        umull           v16.8h, v4.8b,  v0.8b
        umlal           v16.8h, v5.8b,  v1.8b
        rev64           v17.4s, v16.4s
        add             v16.8h, v16.8h, v17.8h
        rshrn           v16.8b, v16.8h, #6
        st1             {v16.h}[0], [x0], x2
        st1             {v16.h}[2], [x0], x2
        subs            w3,  w3,  #2
        bgt            temp_label_33
        ret

temp_label_34
temp_label_32

        ld1             {v16.h}[0], [x1], x2
        ld1             {v16.h}[1], [x1], x2
        st1             {v16.h}[0], [x0], x2
        st1             {v16.h}[1], [x0], x2
        subs            w3,  w3,  #2
        bgt            temp_label_34
        ret


        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16

        EXPORT ff_avg_h264_chroma_mc2_neon





ff_avg_h264_chroma_mc2_neon
        










        prfm            pldl1strm, [x1]
        prfm            pldl1strm, [x1, x2]
        orr             w7,  w4,  w5
        cbz             w7,  temp_label_35

        mul             w7,  w4,  w5
        lsl             w14, w5,  #3
        lsl             w13, w4,  #3
        sub             w6,  w14, w7
        sub             w12, w13, w7
        sub             w4,  w7,  w13
        sub             w4,  w4,  w14
        add             w4,  w4,  #64
        dup             v0.8b,  w4
        dup             v2.8b,  w12
        dup             v1.8b,  w6
        dup             v3.8b,  w7
        trn1            v0.4h,  v0.4h,  v2.4h
        trn1            v1.4h,  v1.4h,  v3.4h
temp_label_36

        ld1             {v4.s}[0],  [x1], x2
        ld1             {v4.s}[1],  [x1], x2
        rev64           v5.2s,  v4.2s
        ld1             {v5.s}[1],  [x1]
        ext8             v6.8b,  v4.8b,  v5.8b,  #1
        ext8             v7.8b,  v5.8b,  v4.8b,  #1
        trn1            v4.4h,  v4.4h,  v6.4h
        trn1            v5.4h,  v5.4h,  v7.4h
        umull           v16.8h, v4.8b,  v0.8b
        umlal           v16.8h, v5.8b,  v1.8b
        ld1             {v18.h}[0], [x0], x2
        ld1             {v18.h}[2], [x0]
        sub             x0,  x0,  x2
        rev64           v17.4s, v16.4s
        add             v16.8h, v16.8h, v17.8h
        rshrn           v16.8b, v16.8h, #6
        urhadd          v16.8b, v16.8b, v18.8b
        st1             {v16.h}[0], [x0], x2
        st1             {v16.h}[2], [x0], x2
        subs            w3,  w3,  #2
        bgt            temp_label_36
        ret

temp_label_37
temp_label_35

        ld1             {v16.h}[0], [x1], x2
        ld1             {v16.h}[1], [x1], x2
        ld1             {v18.h}[0], [x0], x2
        ld1             {v18.h}[1], [x0]
        sub             x0,  x0,  x2
        urhadd          v16.8b, v16.8b, v18.8b
        st1             {v16.h}[0], [x0], x2
        st1             {v16.h}[1], [x0], x2
        subs            w3,  w3,  #2
        bgt            temp_label_37
        ret

























	END
