	AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN

























































































































































































































































































































































































































































































































































































































































































































































































































































































        ;.arch           armv8.6-a+crc


































	;.arch_extension nodotprod
	;.arch_extension noi8mm
	;.arch_extension nosve
	;.arch_extension nosve2













































































































































        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16

        EXPORT ff_put_h264_qpel16_mc00_neon





ff_put_h264_qpel16_mc00_neon
        










        mov             w3,  #16



        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16

        EXPORT ff_put_pixels16_neon





ff_put_pixels16_neon
        










temp_label_0
      ld1             {v0.16b},  [x1], x2
        ld1             {v1.16b},  [x1], x2
        ld1             {v2.16b},  [x1], x2
        ld1             {v3.16b},  [x1], x2
        subs            w3,  w3,  #4
        st1             {v0.16b},  [x0], x2
        st1             {v1.16b},  [x0], x2
        st1             {v2.16b},  [x0], x2
        st1             {v3.16b},  [x0], x2
        bne            temp_label_0
        ret


        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16

        EXPORT ff_put_pixels16_x2_neon





ff_put_pixels16_x2_neon
        










temp_label_1
      ld1             {v0.16b, v1.16b}, [x1], x2
        ld1             {v2.16b, v3.16b}, [x1], x2
        subs            w3,  w3,  #2
        ext8             v1.16b,  v0.16b,  v1.16b,  #1
        urhadd          v0.16b, v0.16b, v1.16b
        ext8             v3.16b,  v2.16b,  v3.16b,  #1
        urhadd          v2.16b, v2.16b, v3.16b
        st1             {v0.16b}, [x0], x2
        st1             {v2.16b}, [x0], x2
        bne            temp_label_1
        ret


        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16

        EXPORT ff_put_pixels16_x2_no_rnd_neon





ff_put_pixels16_x2_no_rnd_neon
        










temp_label_2
      ld1             {v0.16b, v1.16b}, [x1], x2
        ld1             {v2.16b, v3.16b}, [x1], x2
        subs            w3,  w3,  #2
        ext8             v1.16b,  v0.16b,  v1.16b,  #1
        uhadd           v0.16b, v0.16b, v1.16b
        ext8             v3.16b,  v2.16b,  v3.16b,  #1
        uhadd           v2.16b, v2.16b, v3.16b
        st1             {v0.16b}, [x0], x2
        st1             {v2.16b}, [x0], x2
        bne            temp_label_2
        ret


        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16

        EXPORT ff_put_pixels16_y2_neon





ff_put_pixels16_y2_neon
        










        sub             w3,  w3,  #2
        ld1             {v0.16b}, [x1], x2
        ld1             {v1.16b}, [x1], x2
temp_label_3
      subs            w3,  w3,  #2
        urhadd          v2.16b, v0.16b, v1.16b
        ld1             {v0.16b}, [x1], x2
        urhadd          v3.16b, v0.16b, v1.16b
        ld1             {v1.16b}, [x1], x2
        st1             {v2.16b}, [x0], x2
        st1             {v3.16b}, [x0], x2
        bne            temp_label_3

        urhadd          v2.16b, v0.16b, v1.16b
        ld1             {v0.16b}, [x1], x2
        urhadd          v3.16b, v0.16b, v1.16b
        st1             {v2.16b},     [x0], x2
        st1             {v3.16b},     [x0], x2

        ret


        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16

        EXPORT ff_put_pixels16_y2_no_rnd_neon





ff_put_pixels16_y2_no_rnd_neon
        










        sub             w3,  w3,  #2
        ld1             {v0.16b}, [x1], x2
        ld1             {v1.16b}, [x1], x2
temp_label_4
      subs            w3,  w3,  #2
        uhadd           v2.16b, v0.16b, v1.16b
        ld1             {v0.16b}, [x1], x2
        uhadd           v3.16b, v0.16b, v1.16b
        ld1             {v1.16b}, [x1], x2
        st1             {v2.16b}, [x0], x2
        st1             {v3.16b}, [x0], x2
        bne            temp_label_4

        uhadd           v2.16b, v0.16b, v1.16b
        ld1             {v0.16b}, [x1], x2
        uhadd           v3.16b, v0.16b, v1.16b
        st1             {v2.16b},     [x0], x2
        st1             {v3.16b},     [x0], x2

        ret


        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16

        EXPORT ff_put_pixels16_xy2_neon





ff_put_pixels16_xy2_neon
        










        sub             w3,  w3,  #2
        ld1             {v0.16b, v1.16b}, [x1], x2
        ld1             {v4.16b, v5.16b}, [x1], x2
        ext8             v1.16b,  v0.16b,  v1.16b,  #1
        ext8             v5.16b,  v4.16b,  v5.16b,  #1
        uaddl           v16.8h,  v0.8b,   v1.8b
        uaddl2          v20.8h,  v0.16b,  v1.16b
        uaddl           v18.8h,  v4.8b,   v5.8b
        uaddl2          v22.8h,  v4.16b,  v5.16b
temp_label_5
      subs            w3,  w3,  #2
        ld1             {v0.16b, v1.16b}, [x1], x2
        add             v24.8h,  v16.8h,  v18.8h
        ext8             v30.16b, v0.16b,  v1.16b,  #1
        add             v1.8h,   v20.8h,  v22.8h
        rshrn           v28.8b, v24.8h, #2
        rshrn2          v28.16b, v1.8h, #2
        uaddl           v16.8h,  v0.8b,   v30.8b
        ld1             {v2.16b, v3.16b}, [x1], x2
        uaddl2          v20.8h,  v0.16b,  v30.16b
        st1             {v28.16b},        [x0], x2
        add             v24.8h,  v16.8h,  v18.8h
        ext8             v3.16b,  v2.16b,  v3.16b,  #1
        add             v0.8h,   v20.8h,  v22.8h
        rshrn           v30.8b, v24.8h, #2
        rshrn2          v30.16b, v0.8h, #2
        uaddl           v18.8h,   v2.8b,  v3.8b
        uaddl2          v22.8h,   v2.16b, v3.16b
        st1             {v30.16b},        [x0], x2
        bgt            temp_label_5

        ld1             {v0.16b, v1.16b}, [x1], x2
        add             v24.8h,  v16.8h,  v18.8h
        ext8             v30.16b, v0.16b,  v1.16b,  #1
        add             v1.8h,   v20.8h,  v22.8h
        rshrn           v28.8b, v24.8h, #2
        rshrn2          v28.16b, v1.8h, #2
        uaddl           v16.8h,  v0.8b,   v30.8b
        uaddl2          v20.8h,  v0.16b,  v30.16b
        st1             {v28.16b},        [x0], x2
        add             v24.8h,  v16.8h,  v18.8h
        add             v0.8h,   v20.8h,  v22.8h
        rshrn           v30.8b, v24.8h, #2
        rshrn2          v30.16b, v0.8h, #2
        st1             {v30.16b},        [x0], x2

        ret


        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16

        EXPORT ff_put_pixels16_xy2_no_rnd_neon





ff_put_pixels16_xy2_no_rnd_neon
        










        sub             w3,  w3,  #2
        ld1             {v0.16b, v1.16b}, [x1], x2
        ld1             {v4.16b, v5.16b}, [x1], x2
        movi v26.8H, #1, lsl #0
        ext8             v1.16b,  v0.16b,  v1.16b,  #1
        ext8             v5.16b,  v4.16b,  v5.16b,  #1
        uaddl           v16.8h,  v0.8b,   v1.8b
        uaddl2          v20.8h,  v0.16b,  v1.16b
        uaddl           v18.8h,  v4.8b,   v5.8b
        uaddl2          v22.8h,  v4.16b,  v5.16b
temp_label_6
      subs            w3,  w3,  #2
        ld1             {v0.16b, v1.16b}, [x1], x2
        add             v24.8h,  v16.8h,  v18.8h
        add  v24.8H, v24.8H, v26.8H
        ext8             v30.16b, v0.16b,  v1.16b,  #1
        add             v1.8h,   v20.8h,  v22.8h
        shrn            v28.8b, v24.8h, #2
        add  v1.8H, v1.8H, v26.8H
        shrn2           v28.16b, v1.8h, #2
        uaddl           v16.8h,  v0.8b,   v30.8b
        ld1             {v2.16b, v3.16b}, [x1], x2
        uaddl2          v20.8h,  v0.16b,  v30.16b
        st1             {v28.16b},        [x0], x2
        add             v24.8h,  v16.8h,  v18.8h
        add  v24.8H, v24.8H, v26.8H
        ext8             v3.16b,  v2.16b,  v3.16b,  #1
        add             v0.8h,   v20.8h,  v22.8h
        shrn            v30.8b, v24.8h, #2
        add  v0.8H, v0.8H, v26.8H
        shrn2           v30.16b, v0.8h, #2
        uaddl           v18.8h,   v2.8b,  v3.8b
        uaddl2          v22.8h,   v2.16b, v3.16b
        st1             {v30.16b},        [x0], x2
        bgt            temp_label_6

        ld1             {v0.16b, v1.16b}, [x1], x2
        add             v24.8h,  v16.8h,  v18.8h
        add  v24.8H, v24.8H, v26.8H
        ext8             v30.16b, v0.16b,  v1.16b,  #1
        add             v1.8h,   v20.8h,  v22.8h
        shrn            v28.8b, v24.8h, #2
        add  v1.8H, v1.8H, v26.8H
        shrn2           v28.16b, v1.8h, #2
        uaddl           v16.8h,  v0.8b,   v30.8b
        uaddl2          v20.8h,  v0.16b,  v30.16b
        st1             {v28.16b},        [x0], x2
        add             v24.8h,  v16.8h,  v18.8h
        add  v24.8H, v24.8H, v26.8H
        add             v0.8h,   v20.8h,  v22.8h
        shrn            v30.8b, v24.8h, #2
        add  v0.8H, v0.8H, v26.8H
        shrn2           v30.16b, v0.8h, #2
        st1             {v30.16b},        [x0], x2

        ret



        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16

        EXPORT ff_avg_h264_qpel16_mc00_neon





ff_avg_h264_qpel16_mc00_neon
        










        mov             w3,  #16



        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16

        EXPORT ff_avg_pixels16_neon





ff_avg_pixels16_neon
        










        mov             x12, x0
temp_label_7
      ld1             {v0.16b},  [x1], x2
        ld1             {v1.16b},  [x1], x2
        ld1             {v2.16b},  [x1], x2
        ld1             {v3.16b},  [x1], x2
        ld1             {v4.16b},  [x12], x2
        urhadd          v0.16b,  v0.16b,  v4.16b
        ld1             {v5.16b},  [x12], x2
        urhadd          v1.16b,  v1.16b,  v5.16b
        ld1             {v6.16b},  [x12], x2
        urhadd          v2.16b,  v2.16b,  v6.16b
        ld1             {v7.16b},  [x12], x2
        urhadd          v3.16b,  v3.16b,  v7.16b
        subs            w3,  w3,  #4
        st1             {v0.16b},  [x0], x2
        st1             {v1.16b},  [x0], x2
        st1             {v2.16b},  [x0], x2
        st1             {v3.16b},  [x0], x2
        bne            temp_label_7
        ret


        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16

        EXPORT ff_avg_pixels16_x2_neon





ff_avg_pixels16_x2_neon
        










temp_label_8
      ld1             {v0.16b, v1.16b}, [x1], x2
        ld1             {v2.16b, v3.16b}, [x1], x2
        subs            w3,  w3,  #2
        ext8             v1.16b,  v0.16b,  v1.16b,  #1
        urhadd          v0.16b, v0.16b, v1.16b
        ext8             v3.16b,  v2.16b,  v3.16b,  #1
        urhadd          v2.16b, v2.16b, v3.16b
        ld1             {v1.16b}, [x0], x2
        ld1             {v3.16b}, [x0]
        urhadd          v0.16b,  v0.16b,  v1.16b
        urhadd          v2.16b,  v2.16b,  v3.16b
        sub             x0,  x0,  x2
        st1             {v0.16b}, [x0], x2
        st1             {v2.16b}, [x0], x2
        bne            temp_label_8
        ret


        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16

        EXPORT ff_avg_pixels16_x2_no_rnd_neon





ff_avg_pixels16_x2_no_rnd_neon
        










temp_label_9
      ld1             {v0.16b, v1.16b}, [x1], x2
        ld1             {v2.16b, v3.16b}, [x1], x2
        subs            w3,  w3,  #2
        ext8             v1.16b,  v0.16b,  v1.16b,  #1
        uhadd           v0.16b, v0.16b, v1.16b
        ext8             v3.16b,  v2.16b,  v3.16b,  #1
        uhadd           v2.16b, v2.16b, v3.16b
        ld1             {v1.16b}, [x0], x2
        ld1             {v3.16b}, [x0]
        urhadd          v0.16b,  v0.16b,  v1.16b
        urhadd          v2.16b,  v2.16b,  v3.16b
        sub             x0,  x0,  x2
        st1             {v0.16b}, [x0], x2
        st1             {v2.16b}, [x0], x2
        bne            temp_label_9
        ret


        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16

        EXPORT ff_avg_pixels16_y2_neon





ff_avg_pixels16_y2_neon
        










        sub             w3,  w3,  #2
        ld1             {v0.16b}, [x1], x2
        ld1             {v1.16b}, [x1], x2
temp_label_10
      subs            w3,  w3,  #2
        urhadd          v2.16b, v0.16b, v1.16b
        ld1             {v0.16b}, [x1], x2
        urhadd          v3.16b, v0.16b, v1.16b
        ld1             {v1.16b}, [x1], x2
        ld1             {v4.16b}, [x0], x2
        ld1             {v5.16b}, [x0]
        urhadd          v2.16b,  v2.16b,  v4.16b
        urhadd          v3.16b,  v3.16b,  v5.16b
        sub             x0,  x0,  x2
        st1             {v2.16b}, [x0], x2
        st1             {v3.16b}, [x0], x2
        bne            temp_label_10

        urhadd          v2.16b, v0.16b, v1.16b
        ld1             {v0.16b}, [x1], x2
        urhadd          v3.16b, v0.16b, v1.16b
        ld1             {v4.16b}, [x0], x2
        ld1             {v5.16b}, [x0]
        urhadd          v2.16b,  v2.16b,  v4.16b
        urhadd          v3.16b,  v3.16b,  v5.16b
        sub             x0,  x0,  x2
        st1             {v2.16b},     [x0], x2
        st1             {v3.16b},     [x0], x2

        ret


        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16

        EXPORT ff_avg_pixels16_y2_no_rnd_neon





ff_avg_pixels16_y2_no_rnd_neon
        










        sub             w3,  w3,  #2
        ld1             {v0.16b}, [x1], x2
        ld1             {v1.16b}, [x1], x2
temp_label_11
      subs            w3,  w3,  #2
        uhadd           v2.16b, v0.16b, v1.16b
        ld1             {v0.16b}, [x1], x2
        uhadd           v3.16b, v0.16b, v1.16b
        ld1             {v1.16b}, [x1], x2
        ld1             {v4.16b}, [x0], x2
        ld1             {v5.16b}, [x0]
        urhadd          v2.16b,  v2.16b,  v4.16b
        urhadd          v3.16b,  v3.16b,  v5.16b
        sub             x0,  x0,  x2
        st1             {v2.16b}, [x0], x2
        st1             {v3.16b}, [x0], x2
        bne            temp_label_11

        uhadd           v2.16b, v0.16b, v1.16b
        ld1             {v0.16b}, [x1], x2
        uhadd           v3.16b, v0.16b, v1.16b
        ld1             {v4.16b}, [x0], x2
        ld1             {v5.16b}, [x0]
        urhadd          v2.16b,  v2.16b,  v4.16b
        urhadd          v3.16b,  v3.16b,  v5.16b
        sub             x0,  x0,  x2
        st1             {v2.16b},     [x0], x2
        st1             {v3.16b},     [x0], x2

        ret


        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16

        EXPORT ff_avg_pixels16_xy2_neon





ff_avg_pixels16_xy2_neon
        










        sub             w3,  w3,  #2
        ld1             {v0.16b, v1.16b}, [x1], x2
        ld1             {v4.16b, v5.16b}, [x1], x2
        ext8             v1.16b,  v0.16b,  v1.16b,  #1
        ext8             v5.16b,  v4.16b,  v5.16b,  #1
        uaddl           v16.8h,  v0.8b,   v1.8b
        uaddl2          v20.8h,  v0.16b,  v1.16b
        uaddl           v18.8h,  v4.8b,   v5.8b
        uaddl2          v22.8h,  v4.16b,  v5.16b
temp_label_12
      subs            w3,  w3,  #2
        ld1             {v0.16b, v1.16b}, [x1], x2
        add             v24.8h,  v16.8h,  v18.8h
        ext8             v30.16b, v0.16b,  v1.16b,  #1
        add             v1.8h,   v20.8h,  v22.8h
        rshrn           v28.8b, v24.8h, #2
        rshrn2          v28.16b, v1.8h, #2
        ld1             {v16.16b},        [x0]
        urhadd          v28.16b, v28.16b, v16.16b
        uaddl           v16.8h,  v0.8b,   v30.8b
        ld1             {v2.16b, v3.16b}, [x1], x2
        uaddl2          v20.8h,  v0.16b,  v30.16b
        st1             {v28.16b},        [x0], x2
        add             v24.8h,  v16.8h,  v18.8h
        ext8             v3.16b,  v2.16b,  v3.16b,  #1
        add             v0.8h,   v20.8h,  v22.8h
        rshrn           v30.8b, v24.8h, #2
        rshrn2          v30.16b, v0.8h, #2
        ld1             {v18.16b},        [x0]
        urhadd          v30.16b, v30.16b, v18.16b
        uaddl           v18.8h,   v2.8b,  v3.8b
        uaddl2          v22.8h,   v2.16b, v3.16b
        st1             {v30.16b},        [x0], x2
        bgt            temp_label_12

        ld1             {v0.16b, v1.16b}, [x1], x2
        add             v24.8h,  v16.8h,  v18.8h
        ext8             v30.16b, v0.16b,  v1.16b,  #1
        add             v1.8h,   v20.8h,  v22.8h
        rshrn           v28.8b, v24.8h, #2
        rshrn2          v28.16b, v1.8h, #2
        ld1             {v16.16b},        [x0]
        urhadd          v28.16b, v28.16b, v16.16b
        uaddl           v16.8h,  v0.8b,   v30.8b
        uaddl2          v20.8h,  v0.16b,  v30.16b
        st1             {v28.16b},        [x0], x2
        add             v24.8h,  v16.8h,  v18.8h
        add             v0.8h,   v20.8h,  v22.8h
        rshrn           v30.8b, v24.8h, #2
        rshrn2          v30.16b, v0.8h, #2
        ld1             {v18.16b},        [x0]
        urhadd          v30.16b, v30.16b, v18.16b
        st1             {v30.16b},        [x0], x2

        ret


        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16

        EXPORT ff_avg_pixels16_xy2_no_rnd_neon





ff_avg_pixels16_xy2_no_rnd_neon
        










        sub             w3,  w3,  #2
        ld1             {v0.16b, v1.16b}, [x1], x2
        ld1             {v4.16b, v5.16b}, [x1], x2
        movi v26.8H, #1, lsl #0
        ext8             v1.16b,  v0.16b,  v1.16b,  #1
        ext8             v5.16b,  v4.16b,  v5.16b,  #1
        uaddl           v16.8h,  v0.8b,   v1.8b
        uaddl2          v20.8h,  v0.16b,  v1.16b
        uaddl           v18.8h,  v4.8b,   v5.8b
        uaddl2          v22.8h,  v4.16b,  v5.16b
temp_label_13
      subs            w3,  w3,  #2
        ld1             {v0.16b, v1.16b}, [x1], x2
        add             v24.8h,  v16.8h,  v18.8h
        add  v24.8H, v24.8H, v26.8H
        ext8             v30.16b, v0.16b,  v1.16b,  #1
        add             v1.8h,   v20.8h,  v22.8h
        shrn            v28.8b, v24.8h, #2
        add  v1.8H, v1.8H, v26.8H
        shrn2           v28.16b, v1.8h, #2
        ld1             {v16.16b},        [x0]
        urhadd          v28.16b, v28.16b, v16.16b
        uaddl           v16.8h,  v0.8b,   v30.8b
        ld1             {v2.16b, v3.16b}, [x1], x2
        uaddl2          v20.8h,  v0.16b,  v30.16b
        st1             {v28.16b},        [x0], x2
        add             v24.8h,  v16.8h,  v18.8h
        add  v24.8H, v24.8H, v26.8H
        ext8             v3.16b,  v2.16b,  v3.16b,  #1
        add             v0.8h,   v20.8h,  v22.8h
        shrn            v30.8b, v24.8h, #2
        add  v0.8H, v0.8H, v26.8H
        shrn2           v30.16b, v0.8h, #2
        ld1             {v18.16b},        [x0]
        urhadd          v30.16b, v30.16b, v18.16b
        uaddl           v18.8h,   v2.8b,  v3.8b
        uaddl2          v22.8h,   v2.16b, v3.16b
        st1             {v30.16b},        [x0], x2
        bgt            temp_label_13

        ld1             {v0.16b, v1.16b}, [x1], x2
        add             v24.8h,  v16.8h,  v18.8h
        add  v24.8H, v24.8H, v26.8H
        ext8             v30.16b, v0.16b,  v1.16b,  #1
        add             v1.8h,   v20.8h,  v22.8h
        shrn            v28.8b, v24.8h, #2
        add  v1.8H, v1.8H, v26.8H
        shrn2           v28.16b, v1.8h, #2
        ld1             {v16.16b},        [x0]
        urhadd          v28.16b, v28.16b, v16.16b
        uaddl           v16.8h,  v0.8b,   v30.8b
        uaddl2          v20.8h,  v0.16b,  v30.16b
        st1             {v28.16b},        [x0], x2
        add             v24.8h,  v16.8h,  v18.8h
        add  v24.8H, v24.8H, v26.8H
        add             v0.8h,   v20.8h,  v22.8h
        shrn            v30.8b, v24.8h, #2
        add  v0.8H, v0.8H, v26.8H
        shrn2           v30.16b, v0.8h, #2
        ld1             {v18.16b},        [x0]
        urhadd          v30.16b, v30.16b, v18.16b
        st1             {v30.16b},        [x0], x2

        ret



        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16

        EXPORT ff_put_h264_qpel8_mc00_neon





ff_put_h264_qpel8_mc00_neon
        










        mov             w3,  #8



        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16

        EXPORT ff_put_pixels8_neon





ff_put_pixels8_neon
        










temp_label_14
      ld1             {v0.8b}, [x1], x2
        ld1             {v1.8b}, [x1], x2
        ld1             {v2.8b}, [x1], x2
        ld1             {v3.8b}, [x1], x2
        subs            w3,  w3,  #4
        st1             {v0.8b}, [x0], x2
        st1             {v1.8b}, [x0], x2
        st1             {v2.8b}, [x0], x2
        st1             {v3.8b}, [x0], x2
        bne            temp_label_14
        ret


        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16

        EXPORT ff_put_pixels8_x2_neon





ff_put_pixels8_x2_neon
        










temp_label_15
      ld1             {v0.8b, v1.8b}, [x1], x2
        ext8             v1.8b,  v0.8b,  v1.8b,  #1
        ld1             {v2.8b, v3.8b}, [x1], x2
        ext8             v3.8b,  v2.8b,  v3.8b,  #1
        subs            w3,  w3,  #2
        urhadd          v0.8b, v0.8b, v1.8b
        urhadd          v2.8b, v2.8b, v3.8b
        st1             {v0.8b}, [x0], x2
        st1             {v2.8b}, [x0], x2
        bne            temp_label_15
        ret


        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16

        EXPORT ff_put_pixels8_x2_no_rnd_neon





ff_put_pixels8_x2_no_rnd_neon
        










temp_label_16
      ld1             {v0.8b, v1.8b}, [x1], x2
        ext8             v1.8b,  v0.8b,  v1.8b,  #1
        ld1             {v2.8b, v3.8b}, [x1], x2
        ext8             v3.8b,  v2.8b,  v3.8b,  #1
        subs            w3,  w3,  #2
        uhadd           v0.8b, v0.8b, v1.8b
        uhadd           v2.8b, v2.8b, v3.8b
        st1             {v0.8b}, [x0], x2
        st1             {v2.8b}, [x0], x2
        bne            temp_label_16
        ret


        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16

        EXPORT ff_put_pixels8_y2_neon





ff_put_pixels8_y2_neon
        










        sub             w3,  w3,  #2
        ld1             {v0.8b},  [x1], x2
        ld1             {v1.8b},  [x1], x2
temp_label_17
      subs            w3,  w3,  #2
        urhadd          v4.8b, v0.8b, v1.8b
        ld1             {v0.8b},  [x1], x2
        urhadd          v5.8b, v0.8b, v1.8b
        ld1             {v1.8b},  [x1], x2
        st1             {v4.8b},     [x0], x2
        st1             {v5.8b},     [x0], x2
        bne            temp_label_17

        urhadd          v4.8b, v0.8b, v1.8b
        ld1             {v0.8b},  [x1], x2
        urhadd          v5.8b, v0.8b, v1.8b
        st1             {v4.8b},     [x0], x2
        st1             {v5.8b},     [x0], x2

        ret


        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16

        EXPORT ff_put_pixels8_y2_no_rnd_neon





ff_put_pixels8_y2_no_rnd_neon
        










        sub             w3,  w3,  #2
        ld1             {v0.8b},  [x1], x2
        ld1             {v1.8b},  [x1], x2
temp_label_18
      subs            w3,  w3,  #2
        uhadd           v4.8b, v0.8b, v1.8b
        ld1             {v0.8b},  [x1], x2
        uhadd           v5.8b, v0.8b, v1.8b
        ld1             {v1.8b},  [x1], x2
        st1             {v4.8b},     [x0], x2
        st1             {v5.8b},     [x0], x2
        bne            temp_label_18

        uhadd           v4.8b, v0.8b, v1.8b
        ld1             {v0.8b},  [x1], x2
        uhadd           v5.8b, v0.8b, v1.8b
        st1             {v4.8b},     [x0], x2
        st1             {v5.8b},     [x0], x2

        ret


        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16

        EXPORT ff_put_pixels8_xy2_neon





ff_put_pixels8_xy2_neon
        










        sub             w3,  w3,  #2
        ld1             {v0.16b},     [x1], x2
        ld1             {v1.16b},     [x1], x2
        ext8             v4.16b,  v0.16b,  v4.16b,  #1
        ext8             v6.16b,  v1.16b,  v6.16b,  #1
        uaddl           v16.8h,  v0.8b,  v4.8b
        uaddl           v17.8h,  v1.8b,  v6.8b
temp_label_19
      subs            w3,  w3,  #2
        ld1             {v0.16b},     [x1], x2
        add             v18.8h, v16.8h,  v17.8h
        ext8             v4.16b,  v0.16b,  v4.16b,  #1
        uaddl           v16.8h,  v0.8b,  v4.8b
        rshrn           v5.8b, v18.8h, #2
        ld1             {v1.16b},     [x1], x2
        add             v18.8h, v16.8h,  v17.8h
        st1             {v5.8b},     [x0], x2
        rshrn           v7.8b, v18.8h, #2
        ext8             v6.16b,  v1.16b,  v6.16b,  #1
        uaddl           v17.8h,  v1.8b,   v6.8b
        st1             {v7.8b},     [x0], x2
        bgt            temp_label_19

        ld1             {v0.16b},     [x1], x2
        add             v18.8h, v16.8h, v17.8h
        ext8             v4.16b, v0.16b, v4.16b,  #1
        uaddl           v16.8h,  v0.8b, v4.8b
        rshrn           v5.8b, v18.8h, #2
        add             v18.8h, v16.8h, v17.8h
        st1             {v5.8b},     [x0], x2
        rshrn           v7.8b, v18.8h, #2
        st1             {v7.8b},     [x0], x2

        ret


        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16

        EXPORT ff_put_pixels8_xy2_no_rnd_neon





ff_put_pixels8_xy2_no_rnd_neon
        










        sub             w3,  w3,  #2
        ld1             {v0.16b},     [x1], x2
        ld1             {v1.16b},     [x1], x2
        movi v19.8H, #1, lsl #0
        ext8             v4.16b,  v0.16b,  v4.16b,  #1
        ext8             v6.16b,  v1.16b,  v6.16b,  #1
        uaddl           v16.8h,  v0.8b,  v4.8b
        uaddl           v17.8h,  v1.8b,  v6.8b
temp_label_20
      subs            w3,  w3,  #2
        ld1             {v0.16b},     [x1], x2
        add             v18.8h, v16.8h,  v17.8h
        ext8             v4.16b,  v0.16b,  v4.16b,  #1
        add  v18.8H, v18.8H, v19.8H
        uaddl           v16.8h,  v0.8b,  v4.8b
        shrn            v5.8b, v18.8h, #2
        ld1             {v1.16b},     [x1], x2
        add             v18.8h, v16.8h,  v17.8h
        add  v18.8H, v18.8H, v19.8H
        st1             {v5.8b},     [x0], x2
        shrn            v7.8b, v18.8h, #2
        ext8             v6.16b,  v1.16b,  v6.16b,  #1
        uaddl           v17.8h,  v1.8b,   v6.8b
        st1             {v7.8b},     [x0], x2
        bgt            temp_label_20

        ld1             {v0.16b},     [x1], x2
        add             v18.8h, v16.8h, v17.8h
        ext8             v4.16b, v0.16b, v4.16b,  #1
        add  v18.8H, v18.8H, v19.8H
        uaddl           v16.8h,  v0.8b, v4.8b
        shrn            v5.8b, v18.8h, #2
        add             v18.8h, v16.8h, v17.8h
        add  v18.8H, v18.8H, v19.8H
        st1             {v5.8b},     [x0], x2
        shrn            v7.8b, v18.8h, #2
        st1             {v7.8b},     [x0], x2

        ret



        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16

        EXPORT ff_avg_h264_qpel8_mc00_neon





ff_avg_h264_qpel8_mc00_neon
        










        mov             w3,  #8



        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16

        EXPORT ff_avg_pixels8_neon





ff_avg_pixels8_neon
        










temp_label_21
      ld1             {v0.8b}, [x1], x2
        ld1             {v1.8b}, [x1], x2
        ld1             {v2.8b}, [x1], x2
        ld1             {v3.8b}, [x1], x2
        ld1             {v4.8b}, [x0], x2
        urhadd          v0.8b,  v0.8b,  v4.8b
        ld1             {v5.8b}, [x0], x2
        urhadd          v1.8b,  v1.8b,  v5.8b
        ld1             {v6.8b}, [x0], x2
        urhadd          v2.8b,  v2.8b,  v6.8b
        ld1             {v7.8b}, [x0], x2
        urhadd          v3.8b,  v3.8b,  v7.8b
        sub             x0,  x0,  x2,  lsl #2
        subs            w3,  w3,  #4
        st1             {v0.8b}, [x0], x2
        st1             {v1.8b}, [x0], x2
        st1             {v2.8b}, [x0], x2
        st1             {v3.8b}, [x0], x2
        bne            temp_label_21
        ret


        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16

        EXPORT ff_avg_pixels8_x2_neon





ff_avg_pixels8_x2_neon
        










temp_label_22
      ld1             {v0.8b, v1.8b}, [x1], x2
        ext8             v1.8b,  v0.8b,  v1.8b,  #1
        ld1             {v2.8b, v3.8b}, [x1], x2
        ext8             v3.8b,  v2.8b,  v3.8b,  #1
        subs            w3,  w3,  #2
        urhadd          v0.8b, v0.8b, v1.8b
        urhadd          v2.8b, v2.8b, v3.8b
        ld1             {v4.8b},     [x0], x2
        ld1             {v5.8b},     [x0]
        urhadd          v0.8b,   v0.8b,   v4.8b
        urhadd          v2.8b,   v2.8b,   v5.8b
        sub             x0,  x0,  x2
        st1             {v0.8b}, [x0], x2
        st1             {v2.8b}, [x0], x2
        bne            temp_label_22
        ret


        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16

        EXPORT ff_avg_pixels8_y2_neon





ff_avg_pixels8_y2_neon
        










        sub             w3,  w3,  #2
        ld1             {v0.8b},  [x1], x2
        ld1             {v1.8b},  [x1], x2
temp_label_23
      subs            w3,  w3,  #2
        urhadd          v4.8b, v0.8b, v1.8b
        ld1             {v0.8b},  [x1], x2
        urhadd          v5.8b, v0.8b, v1.8b
        ld1             {v1.8b},  [x1], x2
        ld1             {v2.8b},     [x0], x2
        ld1             {v3.8b},     [x0]
        urhadd          v4.8b,  v4.8b,  v2.8b
        urhadd          v5.8b,  v5.8b,  v3.8b
        sub             x0,  x0,  x2
        st1             {v4.8b},     [x0], x2
        st1             {v5.8b},     [x0], x2
        bne            temp_label_23

        urhadd          v4.8b, v0.8b, v1.8b
        ld1             {v0.8b},  [x1], x2
        urhadd          v5.8b, v0.8b, v1.8b
        ld1             {v2.8b},     [x0], x2
        ld1             {v3.8b},     [x0]
        urhadd          v4.8b,  v4.8b,  v2.8b
        urhadd          v5.8b,  v5.8b,  v3.8b
        sub             x0,  x0,  x2
        st1             {v4.8b},     [x0], x2
        st1             {v5.8b},     [x0], x2

        ret


        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16

        EXPORT ff_avg_pixels8_xy2_neon





ff_avg_pixels8_xy2_neon
        










        sub             w3,  w3,  #2
        ld1             {v0.16b},     [x1], x2
        ld1             {v1.16b},     [x1], x2
        ext8             v4.16b,  v0.16b,  v4.16b,  #1
        ext8             v6.16b,  v1.16b,  v6.16b,  #1
        uaddl           v16.8h,  v0.8b,  v4.8b
        uaddl           v17.8h,  v1.8b,  v6.8b
temp_label_24
      subs            w3,  w3,  #2
        ld1             {v0.16b},     [x1], x2
        add             v18.8h, v16.8h,  v17.8h
        ext8             v4.16b,  v0.16b,  v4.16b,  #1
        uaddl           v16.8h,  v0.8b,  v4.8b
        rshrn           v5.8b, v18.8h, #2
        ld1             {v1.16b},     [x1], x2
        add             v18.8h, v16.8h,  v17.8h
        ld1             {v7.8b},     [x0]
        urhadd          v5.8b,  v5.8b,  v7.8b
        st1             {v5.8b},     [x0], x2
        rshrn           v7.8b, v18.8h, #2
        ld1             {v5.8b},     [x0]
        urhadd          v7.8b,  v7.8b,  v5.8b
        ext8             v6.16b,  v1.16b,  v6.16b,  #1
        uaddl           v17.8h,  v1.8b,   v6.8b
        st1             {v7.8b},     [x0], x2
        bgt            temp_label_24

        ld1             {v0.16b},     [x1], x2
        add             v18.8h, v16.8h, v17.8h
        ext8             v4.16b, v0.16b, v4.16b,  #1
        uaddl           v16.8h,  v0.8b, v4.8b
        rshrn           v5.8b, v18.8h, #2
        add             v18.8h, v16.8h, v17.8h
        ld1             {v7.8b},     [x0]
        urhadd          v5.8b,  v5.8b,  v7.8b
        st1             {v5.8b},     [x0], x2
        rshrn           v7.8b, v18.8h, #2
        ld1             {v5.8b},     [x0]
        urhadd          v7.8b,  v7.8b,  v5.8b
        st1             {v7.8b},     [x0], x2

        ret




	END
