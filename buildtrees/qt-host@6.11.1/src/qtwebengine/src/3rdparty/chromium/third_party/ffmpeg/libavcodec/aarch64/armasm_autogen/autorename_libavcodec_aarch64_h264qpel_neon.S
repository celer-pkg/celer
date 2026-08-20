	AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
























































































































































































































































































































































































































































































































































































































































































































































































































































































        ;.arch           armv8.6-a+crc


































	;.arch_extension nodotprod
	;.arch_extension noi8mm
	;.arch_extension nosve
	;.arch_extension nosve2
































































































































































        












        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16


put_h264_qpel16_h_lowpass_neon_packed
        mov             x4,  x30
        mov             x12, #16
        mov             x3,  #8
        bl              put_h264_qpel8_h_lowpass_neon
        sub             x1,  x1,  x2, lsl #4
        add             x1,  x1,  #8
        mov             x12, #16
        mov             x30, x4
        b               put_h264_qpel8_h_lowpass_neon




        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16


put_h264_qpel16_h_lowpass_neon
        mov             x13, x30
        mov             x12, #16
        bl              put_h264_qpel8_h_lowpass_neon
        sub             x0,  x0,  x3, lsl #4
        sub             x1,  x1,  x2, lsl #4
        add             x0,  x0,  #8
        add             x1,  x1,  #8
        mov             x12, #16
        mov             x30, x13



        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16


put_h264_qpel8_h_lowpass_neon
temp_label_0
      ld1             {v28.8b, v29.8b}, [x1], x2
        ld1             {v16.8b, v17.8b}, [x1], x2
        subs            x12, x12, #2
        ext8             v2.8b,      v28.8b, v29.8b, #2
        ext8             v3.8b,      v28.8b, v29.8b, #3
        uaddl           v2.8h,      v2.8b,     v3.8b
        ext8             v4.8b,      v28.8b, v29.8b, #1
        ext8             v5.8b,      v28.8b, v29.8b, #4
        uaddl           v4.8h,      v4.8b,     v5.8b
        ext8             v1.8b,      v28.8b, v29.8b, #5
        uaddl           v28.8h,  v28.8b, v1.8b
        ext8             v0.8b,      v16.8b, v17.8b, #2
        mla             v28.8h,  v2.8h,     v6.h[1]
        ext8             v1.8b,      v16.8b, v17.8b, #3
        uaddl           v0.8h,      v0.8b,     v1.8b
        ext8             v1.8b,      v16.8b, v17.8b, #1
        mls             v28.8h,  v4.8h,     v6.h[0]
        ext8             v3.8b,      v16.8b, v17.8b, #4
        uaddl           v1.8h,      v1.8b,     v3.8b
        ext8             v2.8b,      v16.8b, v17.8b, #5
        uaddl           v16.8h,  v16.8b, v2.8b
        mla             v16.8h,  v0.8h,     v6.h[1]
        mls             v16.8h,  v1.8h,     v6.h[0]
        sqrshrun        v28.8b,  v28.8h, #5
        sqrshrun        v16.8b,  v16.8h, #5
        st1             {v28.8b},    [x0], x3
        st1             {v16.8b},    [x0], x3
        bne            temp_label_0
        ret


        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16


avg_h264_qpel16_h_lowpass_neon
        mov             x13, x30
        mov             x12, #16
        bl              avg_h264_qpel8_h_lowpass_neon
        sub             x0,  x0,  x3, lsl #4
        sub             x1,  x1,  x2, lsl #4
        add             x0,  x0,  #8
        add             x1,  x1,  #8
        mov             x12, #16
        mov             x30, x13



        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16


avg_h264_qpel8_h_lowpass_neon
temp_label_1
      ld1             {v28.8b, v29.8b}, [x1], x2
        ld1             {v16.8b, v17.8b}, [x1], x2
        subs            x12, x12, #2
        ext8             v2.8b,      v28.8b, v29.8b, #2
        ext8             v3.8b,      v28.8b, v29.8b, #3
        uaddl           v2.8h,      v2.8b,     v3.8b
        ext8             v4.8b,      v28.8b, v29.8b, #1
        ext8             v5.8b,      v28.8b, v29.8b, #4
        uaddl           v4.8h,      v4.8b,     v5.8b
        ext8             v1.8b,      v28.8b, v29.8b, #5
        uaddl           v28.8h,  v28.8b, v1.8b
        ext8             v0.8b,      v16.8b, v17.8b, #2
        mla             v28.8h,  v2.8h,     v6.h[1]
        ext8             v1.8b,      v16.8b, v17.8b, #3
        uaddl           v0.8h,      v0.8b,     v1.8b
        ext8             v1.8b,      v16.8b, v17.8b, #1
        mls             v28.8h,  v4.8h,     v6.h[0]
        ext8             v3.8b,      v16.8b, v17.8b, #4
        uaddl           v1.8h,      v1.8b,     v3.8b
        ext8             v2.8b,      v16.8b, v17.8b, #5
        uaddl           v16.8h,  v16.8b, v2.8b
        mla             v16.8h,  v0.8h,     v6.h[1]
        mls             v16.8h,  v1.8h,     v6.h[0]
        sqrshrun        v28.8b,  v28.8h, #5
        sqrshrun        v16.8b,  v16.8h, #5
        ld1             {v2.8b},    [x0], x3
        ld1             {v3.8b},    [x0]
        urhadd          v28.8b, v28.8b,  v2.8b
        urhadd          v16.8b, v16.8b, v3.8b
        sub             x0,  x0,  x3
        st1             {v28.8b},    [x0], x3
        st1             {v16.8b},    [x0], x3
        bne            temp_label_1
        ret




        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16


put_h264_qpel16_h_lowpass_l2_neon
        mov             x13, x30
        mov             x12, #16
        bl              put_h264_qpel8_h_lowpass_l2_neon
        sub             x0,  x0,  x2, lsl #4
        sub             x1,  x1,  x2, lsl #4
        sub             x3,  x3,  x2, lsl #4
        add             x0,  x0,  #8
        add             x1,  x1,  #8
        add             x3,  x3,  #8
        mov             x12, #16
        mov             x30, x13



        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16


put_h264_qpel8_h_lowpass_l2_neon
temp_label_2
      ld1             {v26.8b, v27.8b}, [x1], x2
        ld1             {v16.8b, v17.8b}, [x1], x2
        ld1             {v28.8b},     [x3], x2
        ld1             {v29.8b},     [x3], x2
        subs            x12, x12, #2
        ext8             v2.8b,      v26.8b, v27.8b, #2
        ext8             v3.8b,      v26.8b, v27.8b, #3
        uaddl           v2.8h,      v2.8b,     v3.8b
        ext8             v4.8b,      v26.8b, v27.8b, #1
        ext8             v5.8b,      v26.8b, v27.8b, #4
        uaddl           v4.8h,      v4.8b,     v5.8b
        ext8             v1.8b,      v26.8b, v27.8b, #5
        uaddl           v26.8h,  v26.8b, v1.8b
        ext8             v0.8b,      v16.8b, v17.8b, #2
        mla             v26.8h,  v2.8h,     v6.h[1]
        ext8             v1.8b,      v16.8b, v17.8b, #3
        uaddl           v0.8h,      v0.8b,     v1.8b
        ext8             v1.8b,      v16.8b, v17.8b, #1
        mls             v26.8h,  v4.8h,     v6.h[0]
        ext8             v3.8b,      v16.8b, v17.8b, #4
        uaddl           v1.8h,      v1.8b,     v3.8b
        ext8             v2.8b,      v16.8b, v17.8b, #5
        uaddl           v27.8h,  v16.8b, v2.8b
        mla             v27.8h,  v0.8h,     v6.h[1]
        mls             v27.8h,  v1.8h,     v6.h[0]
        sqrshrun        v26.8b,  v26.8h, #5
        sqrshrun        v27.8b,  v27.8h, #5
        urhadd          v26.8b, v26.8b, v28.8b
        urhadd          v27.8b, v27.8b, v29.8b
        st1             {v26.8b},     [x0], x2
        st1             {v27.8b},     [x0], x2
        bne            temp_label_2
        ret


        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16


avg_h264_qpel16_h_lowpass_l2_neon
        mov             x13, x30
        mov             x12, #16
        bl              avg_h264_qpel8_h_lowpass_l2_neon
        sub             x0,  x0,  x2, lsl #4
        sub             x1,  x1,  x2, lsl #4
        sub             x3,  x3,  x2, lsl #4
        add             x0,  x0,  #8
        add             x1,  x1,  #8
        add             x3,  x3,  #8
        mov             x12, #16
        mov             x30, x13



        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16


avg_h264_qpel8_h_lowpass_l2_neon
temp_label_3
      ld1             {v26.8b, v27.8b}, [x1], x2
        ld1             {v16.8b, v17.8b}, [x1], x2
        ld1             {v28.8b},     [x3], x2
        ld1             {v29.8b},     [x3], x2
        subs            x12, x12, #2
        ext8             v2.8b,      v26.8b, v27.8b, #2
        ext8             v3.8b,      v26.8b, v27.8b, #3
        uaddl           v2.8h,      v2.8b,     v3.8b
        ext8             v4.8b,      v26.8b, v27.8b, #1
        ext8             v5.8b,      v26.8b, v27.8b, #4
        uaddl           v4.8h,      v4.8b,     v5.8b
        ext8             v1.8b,      v26.8b, v27.8b, #5
        uaddl           v26.8h,  v26.8b, v1.8b
        ext8             v0.8b,      v16.8b, v17.8b, #2
        mla             v26.8h,  v2.8h,     v6.h[1]
        ext8             v1.8b,      v16.8b, v17.8b, #3
        uaddl           v0.8h,      v0.8b,     v1.8b
        ext8             v1.8b,      v16.8b, v17.8b, #1
        mls             v26.8h,  v4.8h,     v6.h[0]
        ext8             v3.8b,      v16.8b, v17.8b, #4
        uaddl           v1.8h,      v1.8b,     v3.8b
        ext8             v2.8b,      v16.8b, v17.8b, #5
        uaddl           v27.8h,  v16.8b, v2.8b
        mla             v27.8h,  v0.8h,     v6.h[1]
        mls             v27.8h,  v1.8h,     v6.h[0]
        sqrshrun        v26.8b,  v26.8h, #5
        sqrshrun        v27.8b,  v27.8h, #5
        urhadd          v26.8b, v26.8b, v28.8b
        urhadd          v27.8b, v27.8b, v29.8b
        ld1             {v2.8b},      [x0], x2
        ld1             {v3.8b},      [x0]
        urhadd          v26.8b, v26.8b, v2.8b
        urhadd          v27.8b, v27.8b, v3.8b
        sub             x0,  x0,  x2
        st1             {v26.8b},     [x0], x2
        st1             {v27.8b},     [x0], x2
        bne            temp_label_3
        ret



        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16


put_h264_qpel16_v_lowpass_neon_packed
        mov             x4,  x30
        mov             x2,  #8
        bl              put_h264_qpel8_v_lowpass_neon
        sub             x1,  x1,  x3, lsl #2
        bl              put_h264_qpel8_v_lowpass_neon
        sub             x1,  x1,  x3, lsl #4
        sub             x1,  x1,  x3, lsl #2
        add             x1,  x1,  #8
        bl              put_h264_qpel8_v_lowpass_neon
        sub             x1,  x1,  x3, lsl #2
        mov             x30, x4
        b               put_h264_qpel8_v_lowpass_neon




        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16


put_h264_qpel16_v_lowpass_neon
        mov             x4,  x30
        bl              put_h264_qpel8_v_lowpass_neon
        sub             x1,  x1,  x3, lsl #2
        bl              put_h264_qpel8_v_lowpass_neon
        sub             x0,  x0,  x2, lsl #4
        add             x0,  x0,  #8
        sub             x1,  x1,  x3, lsl #4
        sub             x1,  x1,  x3, lsl #2
        add             x1,  x1,  #8
        bl              put_h264_qpel8_v_lowpass_neon
        sub             x1,  x1,  x3, lsl #2
        mov             x30, x4



        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16


put_h264_qpel8_v_lowpass_neon
        ld1             {v16.8b}, [x1], x3
        ld1             {v17.8b}, [x1], x3
        ld1             {v18.8b}, [x1], x3
        ld1             {v19.8b}, [x1], x3
        ld1             {v20.8b}, [x1], x3
        ld1             {v21.8b}, [x1], x3
        ld1             {v22.8b}, [x1], x3
        ld1             {v23.8b}, [x1], x3
        ld1             {v24.8b}, [x1], x3
        ld1             {v25.8b}, [x1], x3
        ld1             {v26.8b}, [x1], x3
        ld1             {v27.8b}, [x1], x3
        ld1             {v28.8b}, [x1]

        uaddl           v2.8h,      v18.8b, v19.8b
        uaddl           v0.8h,      v19.8b, v20.8b
        uaddl           v4.8h,      v17.8b, v20.8b
        uaddl           v1.8h,      v18.8b, v21.8b
        uaddl           v16.8h,  v16.8b, v21.8b
        uaddl           v17.8h,  v17.8b, v22.8b
        mla             v16.8h,  v2.8h,     v6.h[1]
        mls             v16.8h,  v4.8h,     v6.h[0]
        mla             v17.8h,  v0.8h,     v6.h[1]
        mls             v17.8h,  v1.8h,     v6.h[0]
        sqrshrun        v16.8b,  v16.8h, #5
        sqrshrun        v17.8b,  v17.8h, #5
        uaddl           v2.8h,      v20.8b, v21.8b
        uaddl           v0.8h,      v21.8b, v22.8b
        uaddl           v4.8h,      v19.8b, v22.8b
        uaddl           v1.8h,      v20.8b, v23.8b
        uaddl           v18.8h,  v18.8b, v23.8b
        uaddl           v19.8h,  v19.8b, v24.8b
        mla             v18.8h,  v2.8h,     v6.h[1]
        mls             v18.8h,  v4.8h,     v6.h[0]
        mla             v19.8h,  v0.8h,     v6.h[1]
        mls             v19.8h,  v1.8h,     v6.h[0]
        sqrshrun        v18.8b,  v18.8h, #5
        sqrshrun        v19.8b,  v19.8h, #5
        uaddl           v2.8h,      v22.8b, v23.8b
        uaddl           v0.8h,      v23.8b, v24.8b
        uaddl           v4.8h,      v21.8b, v24.8b
        uaddl           v1.8h,      v22.8b, v25.8b
        uaddl           v20.8h,  v20.8b, v25.8b
        uaddl           v21.8h,  v21.8b, v26.8b
        mla             v20.8h,  v2.8h,     v6.h[1]
        mls             v20.8h,  v4.8h,     v6.h[0]
        mla             v21.8h,  v0.8h,     v6.h[1]
        mls             v21.8h,  v1.8h,     v6.h[0]
        sqrshrun        v20.8b,  v20.8h, #5
        sqrshrun        v21.8b,  v21.8h, #5
        uaddl           v2.8h,      v24.8b, v25.8b
        uaddl           v0.8h,      v25.8b, v26.8b
        uaddl           v4.8h,      v23.8b, v26.8b
        uaddl           v1.8h,      v24.8b, v27.8b
        uaddl           v22.8h,  v22.8b, v27.8b
        uaddl           v23.8h,  v23.8b, v28.8b
        mla             v22.8h,  v2.8h,     v6.h[1]
        mls             v22.8h,  v4.8h,     v6.h[0]
        mla             v23.8h,  v0.8h,     v6.h[1]
        mls             v23.8h,  v1.8h,     v6.h[0]
        sqrshrun        v22.8b,  v22.8h, #5
        sqrshrun        v23.8b,  v23.8h, #5

        st1             {v16.8b}, [x0], x2
        st1             {v17.8b}, [x0], x2
        st1             {v18.8b}, [x0], x2
        st1             {v19.8b}, [x0], x2
        st1             {v20.8b}, [x0], x2
        st1             {v21.8b}, [x0], x2
        st1             {v22.8b}, [x0], x2
        st1             {v23.8b}, [x0], x2

        ret


        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16


avg_h264_qpel16_v_lowpass_neon
        mov             x4,  x30
        bl              avg_h264_qpel8_v_lowpass_neon
        sub             x1,  x1,  x3, lsl #2
        bl              avg_h264_qpel8_v_lowpass_neon
        sub             x0,  x0,  x2, lsl #4
        add             x0,  x0,  #8
        sub             x1,  x1,  x3, lsl #4
        sub             x1,  x1,  x3, lsl #2
        add             x1,  x1,  #8
        bl              avg_h264_qpel8_v_lowpass_neon
        sub             x1,  x1,  x3, lsl #2
        mov             x30, x4



        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16


avg_h264_qpel8_v_lowpass_neon
        ld1             {v16.8b}, [x1], x3
        ld1             {v17.8b}, [x1], x3
        ld1             {v18.8b}, [x1], x3
        ld1             {v19.8b}, [x1], x3
        ld1             {v20.8b}, [x1], x3
        ld1             {v21.8b}, [x1], x3
        ld1             {v22.8b}, [x1], x3
        ld1             {v23.8b}, [x1], x3
        ld1             {v24.8b}, [x1], x3
        ld1             {v25.8b}, [x1], x3
        ld1             {v26.8b}, [x1], x3
        ld1             {v27.8b}, [x1], x3
        ld1             {v28.8b}, [x1]

        uaddl           v2.8h,      v18.8b, v19.8b
        uaddl           v0.8h,      v19.8b, v20.8b
        uaddl           v4.8h,      v17.8b, v20.8b
        uaddl           v1.8h,      v18.8b, v21.8b
        uaddl           v16.8h,  v16.8b, v21.8b
        uaddl           v17.8h,  v17.8b, v22.8b
        mla             v16.8h,  v2.8h,     v6.h[1]
        mls             v16.8h,  v4.8h,     v6.h[0]
        mla             v17.8h,  v0.8h,     v6.h[1]
        mls             v17.8h,  v1.8h,     v6.h[0]
        sqrshrun        v16.8b,  v16.8h, #5
        sqrshrun        v17.8b,  v17.8h, #5
        uaddl           v2.8h,      v20.8b, v21.8b
        uaddl           v0.8h,      v21.8b, v22.8b
        uaddl           v4.8h,      v19.8b, v22.8b
        uaddl           v1.8h,      v20.8b, v23.8b
        uaddl           v18.8h,  v18.8b, v23.8b
        uaddl           v19.8h,  v19.8b, v24.8b
        mla             v18.8h,  v2.8h,     v6.h[1]
        mls             v18.8h,  v4.8h,     v6.h[0]
        mla             v19.8h,  v0.8h,     v6.h[1]
        mls             v19.8h,  v1.8h,     v6.h[0]
        sqrshrun        v18.8b,  v18.8h, #5
        sqrshrun        v19.8b,  v19.8h, #5
        uaddl           v2.8h,      v22.8b, v23.8b
        uaddl           v0.8h,      v23.8b, v24.8b
        uaddl           v4.8h,      v21.8b, v24.8b
        uaddl           v1.8h,      v22.8b, v25.8b
        uaddl           v20.8h,  v20.8b, v25.8b
        uaddl           v21.8h,  v21.8b, v26.8b
        mla             v20.8h,  v2.8h,     v6.h[1]
        mls             v20.8h,  v4.8h,     v6.h[0]
        mla             v21.8h,  v0.8h,     v6.h[1]
        mls             v21.8h,  v1.8h,     v6.h[0]
        sqrshrun        v20.8b,  v20.8h, #5
        sqrshrun        v21.8b,  v21.8h, #5
        uaddl           v2.8h,      v24.8b, v25.8b
        uaddl           v0.8h,      v25.8b, v26.8b
        uaddl           v4.8h,      v23.8b, v26.8b
        uaddl           v1.8h,      v24.8b, v27.8b
        uaddl           v22.8h,  v22.8b, v27.8b
        uaddl           v23.8h,  v23.8b, v28.8b
        mla             v22.8h,  v2.8h,     v6.h[1]
        mls             v22.8h,  v4.8h,     v6.h[0]
        mla             v23.8h,  v0.8h,     v6.h[1]
        mls             v23.8h,  v1.8h,     v6.h[0]
        sqrshrun        v22.8b,  v22.8h, #5
        sqrshrun        v23.8b,  v23.8h, #5
        ld1             {v24.8b},  [x0], x2
        ld1             {v25.8b}, [x0], x2
        ld1             {v26.8b}, [x0], x2
        urhadd          v16.8b, v16.8b, v24.8b
        ld1             {v27.8b}, [x0], x2
        urhadd          v17.8b, v17.8b, v25.8b
        ld1             {v28.8b}, [x0], x2
        urhadd          v18.8b, v18.8b, v26.8b
        ld1             {v29.8b}, [x0], x2
        urhadd          v19.8b, v19.8b, v27.8b
        ld1             {v30.8b}, [x0], x2
        urhadd          v20.8b, v20.8b, v28.8b
        ld1             {v31.8b}, [x0], x2
        urhadd          v21.8b, v21.8b, v29.8b
        urhadd          v22.8b, v22.8b, v30.8b
        urhadd          v23.8b, v23.8b, v31.8b
        sub             x0,  x0,  x2,  lsl #3

        st1             {v16.8b}, [x0], x2
        st1             {v17.8b}, [x0], x2
        st1             {v18.8b}, [x0], x2
        st1             {v19.8b}, [x0], x2
        st1             {v20.8b}, [x0], x2
        st1             {v21.8b}, [x0], x2
        st1             {v22.8b}, [x0], x2
        st1             {v23.8b}, [x0], x2

        ret




        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16


put_h264_qpel16_v_lowpass_l2_neon
        mov             x4,  x30
        bl              put_h264_qpel8_v_lowpass_l2_neon
        sub             x1,  x1,  x3, lsl #2
        bl              put_h264_qpel8_v_lowpass_l2_neon
        sub             x0,  x0,  x3, lsl #4
        sub             x12, x12, x2, lsl #4
        add             x0,  x0,  #8
        add             x12, x12, #8
        sub             x1,  x1,  x3, lsl #4
        sub             x1,  x1,  x3, lsl #2
        add             x1,  x1,  #8
        bl              put_h264_qpel8_v_lowpass_l2_neon
        sub             x1,  x1,  x3, lsl #2
        mov             x30, x4



        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16


put_h264_qpel8_v_lowpass_l2_neon
        ld1             {v16.8b}, [x1], x3
        ld1             {v17.8b}, [x1], x3
        ld1             {v18.8b}, [x1], x3
        ld1             {v19.8b}, [x1], x3
        ld1             {v20.8b}, [x1], x3
        ld1             {v21.8b}, [x1], x3
        ld1             {v22.8b}, [x1], x3
        ld1             {v23.8b}, [x1], x3
        ld1             {v24.8b}, [x1], x3
        ld1             {v25.8b}, [x1], x3
        ld1             {v26.8b}, [x1], x3
        ld1             {v27.8b}, [x1], x3
        ld1             {v28.8b}, [x1]

        uaddl           v2.8h,      v18.8b, v19.8b
        uaddl           v0.8h,      v19.8b, v20.8b
        uaddl           v4.8h,      v17.8b, v20.8b
        uaddl           v1.8h,      v18.8b, v21.8b
        uaddl           v16.8h,  v16.8b, v21.8b
        uaddl           v17.8h,  v17.8b, v22.8b
        mla             v16.8h,  v2.8h,     v6.h[1]
        mls             v16.8h,  v4.8h,     v6.h[0]
        mla             v17.8h,  v0.8h,     v6.h[1]
        mls             v17.8h,  v1.8h,     v6.h[0]
        sqrshrun        v16.8b,  v16.8h, #5
        sqrshrun        v17.8b,  v17.8h, #5
        uaddl           v2.8h,      v20.8b, v21.8b
        uaddl           v0.8h,      v21.8b, v22.8b
        uaddl           v4.8h,      v19.8b, v22.8b
        uaddl           v1.8h,      v20.8b, v23.8b
        uaddl           v18.8h,  v18.8b, v23.8b
        uaddl           v19.8h,  v19.8b, v24.8b
        mla             v18.8h,  v2.8h,     v6.h[1]
        mls             v18.8h,  v4.8h,     v6.h[0]
        mla             v19.8h,  v0.8h,     v6.h[1]
        mls             v19.8h,  v1.8h,     v6.h[0]
        sqrshrun        v18.8b,  v18.8h, #5
        sqrshrun        v19.8b,  v19.8h, #5
        uaddl           v2.8h,      v22.8b, v23.8b
        uaddl           v0.8h,      v23.8b, v24.8b
        uaddl           v4.8h,      v21.8b, v24.8b
        uaddl           v1.8h,      v22.8b, v25.8b
        uaddl           v20.8h,  v20.8b, v25.8b
        uaddl           v21.8h,  v21.8b, v26.8b
        mla             v20.8h,  v2.8h,     v6.h[1]
        mls             v20.8h,  v4.8h,     v6.h[0]
        mla             v21.8h,  v0.8h,     v6.h[1]
        mls             v21.8h,  v1.8h,     v6.h[0]
        sqrshrun        v20.8b,  v20.8h, #5
        sqrshrun        v21.8b,  v21.8h, #5
        uaddl           v2.8h,      v24.8b, v25.8b
        uaddl           v0.8h,      v25.8b, v26.8b
        uaddl           v4.8h,      v23.8b, v26.8b
        uaddl           v1.8h,      v24.8b, v27.8b
        uaddl           v22.8h,  v22.8b, v27.8b
        uaddl           v23.8h,  v23.8b, v28.8b
        mla             v22.8h,  v2.8h,     v6.h[1]
        mls             v22.8h,  v4.8h,     v6.h[0]
        mla             v23.8h,  v0.8h,     v6.h[1]
        mls             v23.8h,  v1.8h,     v6.h[0]
        sqrshrun        v22.8b,  v22.8h, #5
        sqrshrun        v23.8b,  v23.8h, #5

        ld1             {v24.8b},  [x12], x2
        ld1             {v25.8b},  [x12], x2
        ld1             {v26.8b},  [x12], x2
        ld1             {v27.8b},  [x12], x2
        ld1             {v28.8b},  [x12], x2
        urhadd          v16.8b, v24.8b, v16.8b
        urhadd          v17.8b, v25.8b, v17.8b
        ld1             {v29.8b},  [x12], x2
        urhadd          v18.8b, v26.8b, v18.8b
        urhadd          v19.8b, v27.8b, v19.8b
        ld1             {v30.8b}, [x12], x2
        urhadd          v20.8b, v28.8b, v20.8b
        urhadd          v21.8b, v29.8b, v21.8b
        ld1             {v31.8b}, [x12], x2
        urhadd          v22.8b, v30.8b, v22.8b
        urhadd          v23.8b, v31.8b, v23.8b


        st1             {v16.8b}, [x0], x3
        st1             {v17.8b}, [x0], x3
        st1             {v18.8b}, [x0], x3
        st1             {v19.8b}, [x0], x3
        st1             {v20.8b}, [x0], x3
        st1             {v21.8b}, [x0], x3
        st1             {v22.8b}, [x0], x3
        st1             {v23.8b}, [x0], x3

        ret


        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16


avg_h264_qpel16_v_lowpass_l2_neon
        mov             x4,  x30
        bl              avg_h264_qpel8_v_lowpass_l2_neon
        sub             x1,  x1,  x3, lsl #2
        bl              avg_h264_qpel8_v_lowpass_l2_neon
        sub             x0,  x0,  x3, lsl #4
        sub             x12, x12, x2, lsl #4
        add             x0,  x0,  #8
        add             x12, x12, #8
        sub             x1,  x1,  x3, lsl #4
        sub             x1,  x1,  x3, lsl #2
        add             x1,  x1,  #8
        bl              avg_h264_qpel8_v_lowpass_l2_neon
        sub             x1,  x1,  x3, lsl #2
        mov             x30, x4



        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16


avg_h264_qpel8_v_lowpass_l2_neon
        ld1             {v16.8b}, [x1], x3
        ld1             {v17.8b}, [x1], x3
        ld1             {v18.8b}, [x1], x3
        ld1             {v19.8b}, [x1], x3
        ld1             {v20.8b}, [x1], x3
        ld1             {v21.8b}, [x1], x3
        ld1             {v22.8b}, [x1], x3
        ld1             {v23.8b}, [x1], x3
        ld1             {v24.8b}, [x1], x3
        ld1             {v25.8b}, [x1], x3
        ld1             {v26.8b}, [x1], x3
        ld1             {v27.8b}, [x1], x3
        ld1             {v28.8b}, [x1]

        uaddl           v2.8h,      v18.8b, v19.8b
        uaddl           v0.8h,      v19.8b, v20.8b
        uaddl           v4.8h,      v17.8b, v20.8b
        uaddl           v1.8h,      v18.8b, v21.8b
        uaddl           v16.8h,  v16.8b, v21.8b
        uaddl           v17.8h,  v17.8b, v22.8b
        mla             v16.8h,  v2.8h,     v6.h[1]
        mls             v16.8h,  v4.8h,     v6.h[0]
        mla             v17.8h,  v0.8h,     v6.h[1]
        mls             v17.8h,  v1.8h,     v6.h[0]
        sqrshrun        v16.8b,  v16.8h, #5
        sqrshrun        v17.8b,  v17.8h, #5
        uaddl           v2.8h,      v20.8b, v21.8b
        uaddl           v0.8h,      v21.8b, v22.8b
        uaddl           v4.8h,      v19.8b, v22.8b
        uaddl           v1.8h,      v20.8b, v23.8b
        uaddl           v18.8h,  v18.8b, v23.8b
        uaddl           v19.8h,  v19.8b, v24.8b
        mla             v18.8h,  v2.8h,     v6.h[1]
        mls             v18.8h,  v4.8h,     v6.h[0]
        mla             v19.8h,  v0.8h,     v6.h[1]
        mls             v19.8h,  v1.8h,     v6.h[0]
        sqrshrun        v18.8b,  v18.8h, #5
        sqrshrun        v19.8b,  v19.8h, #5
        uaddl           v2.8h,      v22.8b, v23.8b
        uaddl           v0.8h,      v23.8b, v24.8b
        uaddl           v4.8h,      v21.8b, v24.8b
        uaddl           v1.8h,      v22.8b, v25.8b
        uaddl           v20.8h,  v20.8b, v25.8b
        uaddl           v21.8h,  v21.8b, v26.8b
        mla             v20.8h,  v2.8h,     v6.h[1]
        mls             v20.8h,  v4.8h,     v6.h[0]
        mla             v21.8h,  v0.8h,     v6.h[1]
        mls             v21.8h,  v1.8h,     v6.h[0]
        sqrshrun        v20.8b,  v20.8h, #5
        sqrshrun        v21.8b,  v21.8h, #5
        uaddl           v2.8h,      v24.8b, v25.8b
        uaddl           v0.8h,      v25.8b, v26.8b
        uaddl           v4.8h,      v23.8b, v26.8b
        uaddl           v1.8h,      v24.8b, v27.8b
        uaddl           v22.8h,  v22.8b, v27.8b
        uaddl           v23.8h,  v23.8b, v28.8b
        mla             v22.8h,  v2.8h,     v6.h[1]
        mls             v22.8h,  v4.8h,     v6.h[0]
        mla             v23.8h,  v0.8h,     v6.h[1]
        mls             v23.8h,  v1.8h,     v6.h[0]
        sqrshrun        v22.8b,  v22.8h, #5
        sqrshrun        v23.8b,  v23.8h, #5

        ld1             {v24.8b},  [x12], x2
        ld1             {v25.8b},  [x12], x2
        ld1             {v26.8b},  [x12], x2
        ld1             {v27.8b},  [x12], x2
        ld1             {v28.8b},  [x12], x2
        urhadd          v16.8b, v24.8b, v16.8b
        urhadd          v17.8b, v25.8b, v17.8b
        ld1             {v29.8b},  [x12], x2
        urhadd          v18.8b, v26.8b, v18.8b
        urhadd          v19.8b, v27.8b, v19.8b
        ld1             {v30.8b}, [x12], x2
        urhadd          v20.8b, v28.8b, v20.8b
        urhadd          v21.8b, v29.8b, v21.8b
        ld1             {v31.8b}, [x12], x2
        urhadd          v22.8b, v30.8b, v22.8b
        urhadd          v23.8b, v31.8b, v23.8b

        ld1             {v24.8b}, [x0], x3
        ld1             {v25.8b}, [x0], x3
        ld1             {v26.8b}, [x0], x3
        urhadd          v16.8b, v16.8b, v24.8b
        ld1             {v27.8b}, [x0], x3
        urhadd          v17.8b, v17.8b, v25.8b
        ld1             {v28.8b}, [x0], x3
        urhadd          v18.8b, v18.8b, v26.8b
        ld1             {v29.8b}, [x0], x3
        urhadd          v19.8b, v19.8b, v27.8b
        ld1             {v30.8b}, [x0], x3
        urhadd          v20.8b, v20.8b, v28.8b
        ld1             {v31.8b}, [x0], x3
        urhadd          v21.8b, v21.8b, v29.8b
        urhadd          v22.8b, v22.8b, v30.8b
        urhadd          v23.8b, v23.8b, v31.8b
        sub             x0,  x0,  x3,  lsl #3

        st1             {v16.8b}, [x0], x3
        st1             {v17.8b}, [x0], x3
        st1             {v18.8b}, [x0], x3
        st1             {v19.8b}, [x0], x3
        st1             {v20.8b}, [x0], x3
        st1             {v21.8b}, [x0], x3
        st1             {v22.8b}, [x0], x3
        st1             {v23.8b}, [x0], x3

        ret



        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16


put_h264_qpel8_hv_lowpass_neon_top
        movz            w12, #20, lsl #16
        movk            w12, #5
        mov             v6.s[0], w12
        ld1             {v16.8h}, [x1], x3
        ld1             {v17.8h}, [x1], x3
        ld1             {v18.8h}, [x1], x3
        ld1             {v19.8h}, [x1], x3
        ld1             {v20.8h}, [x1], x3
        ld1             {v21.8h}, [x1], x3
        ld1             {v22.8h}, [x1], x3
        ld1             {v23.8h}, [x1], x3
        ld1             {v24.8h}, [x1], x3
        ld1             {v25.8h}, [x1], x3
        ld1             {v26.8h}, [x1], x3
        ld1             {v27.8h}, [x1], x3
        ld1             {v28.8h}, [x1]
        ext8             v0.16b,     v16.16b, v16.16b, #2
        ext8             v1.16b,     v16.16b, v16.16b, #3
        uaddl           v0.8h,      v0.8b,      v1.8b
        ext8             v2.16b,     v16.16b, v16.16b, #1
        ext8             v3.16b,     v16.16b, v16.16b, #4
        uaddl           v2.8h,      v2.8b,      v3.8b
        ext8             v30.16b,    v16.16b, v16.16b, #5
        uaddl           v16.8h,  v16.8b,  v30.8b
        ext8             v4.16b,     v17.16b, v17.16b, #2
        mla             v16.8h,  v0.8h,      v6.h[1]
        ext8             v5.16b,     v17.16b, v17.16b, #3
        uaddl           v4.8h,      v4.8b,      v5.8b
        ext8             v7.16b,     v17.16b, v17.16b, #1
        mls             v16.8h,  v2.8h,      v6.h[0]
        ext8             v0.16b,     v17.16b, v17.16b, #4
        uaddl           v7.8h,      v7.8b,      v0.8b
        ext8             v31.16b,    v17.16b, v17.16b, #5
        uaddl           v17.8h,  v17.8b,  v31.8b
        mla             v17.8h,  v4.8h,      v6.h[1]
        mls             v17.8h,  v7.8h,      v6.h[0]
        ext8             v0.16b,     v18.16b, v18.16b, #2
        ext8             v1.16b,     v18.16b, v18.16b, #3
        uaddl           v0.8h,      v0.8b,      v1.8b
        ext8             v2.16b,     v18.16b, v18.16b, #1
        ext8             v3.16b,     v18.16b, v18.16b, #4
        uaddl           v2.8h,      v2.8b,      v3.8b
        ext8             v30.16b,    v18.16b, v18.16b, #5
        uaddl           v18.8h,  v18.8b,  v30.8b
        ext8             v4.16b,     v19.16b, v19.16b, #2
        mla             v18.8h,  v0.8h,      v6.h[1]
        ext8             v5.16b,     v19.16b, v19.16b, #3
        uaddl           v4.8h,      v4.8b,      v5.8b
        ext8             v7.16b,     v19.16b, v19.16b, #1
        mls             v18.8h,  v2.8h,      v6.h[0]
        ext8             v0.16b,     v19.16b, v19.16b, #4
        uaddl           v7.8h,      v7.8b,      v0.8b
        ext8             v31.16b,    v19.16b, v19.16b, #5
        uaddl           v19.8h,  v19.8b,  v31.8b
        mla             v19.8h,  v4.8h,      v6.h[1]
        mls             v19.8h,  v7.8h,      v6.h[0]
        ext8             v0.16b,     v20.16b, v20.16b, #2
        ext8             v1.16b,     v20.16b, v20.16b, #3
        uaddl           v0.8h,      v0.8b,      v1.8b
        ext8             v2.16b,     v20.16b, v20.16b, #1
        ext8             v3.16b,     v20.16b, v20.16b, #4
        uaddl           v2.8h,      v2.8b,      v3.8b
        ext8             v30.16b,    v20.16b, v20.16b, #5
        uaddl           v20.8h,  v20.8b,  v30.8b
        ext8             v4.16b,     v21.16b, v21.16b, #2
        mla             v20.8h,  v0.8h,      v6.h[1]
        ext8             v5.16b,     v21.16b, v21.16b, #3
        uaddl           v4.8h,      v4.8b,      v5.8b
        ext8             v7.16b,     v21.16b, v21.16b, #1
        mls             v20.8h,  v2.8h,      v6.h[0]
        ext8             v0.16b,     v21.16b, v21.16b, #4
        uaddl           v7.8h,      v7.8b,      v0.8b
        ext8             v31.16b,    v21.16b, v21.16b, #5
        uaddl           v21.8h,  v21.8b,  v31.8b
        mla             v21.8h,  v4.8h,      v6.h[1]
        mls             v21.8h,  v7.8h,      v6.h[0]
        ext8             v0.16b,     v22.16b, v22.16b, #2
        ext8             v1.16b,     v22.16b, v22.16b, #3
        uaddl           v0.8h,      v0.8b,      v1.8b
        ext8             v2.16b,     v22.16b, v22.16b, #1
        ext8             v3.16b,     v22.16b, v22.16b, #4
        uaddl           v2.8h,      v2.8b,      v3.8b
        ext8             v30.16b,    v22.16b, v22.16b, #5
        uaddl           v22.8h,  v22.8b,  v30.8b
        ext8             v4.16b,     v23.16b, v23.16b, #2
        mla             v22.8h,  v0.8h,      v6.h[1]
        ext8             v5.16b,     v23.16b, v23.16b, #3
        uaddl           v4.8h,      v4.8b,      v5.8b
        ext8             v7.16b,     v23.16b, v23.16b, #1
        mls             v22.8h,  v2.8h,      v6.h[0]
        ext8             v0.16b,     v23.16b, v23.16b, #4
        uaddl           v7.8h,      v7.8b,      v0.8b
        ext8             v31.16b,    v23.16b, v23.16b, #5
        uaddl           v23.8h,  v23.8b,  v31.8b
        mla             v23.8h,  v4.8h,      v6.h[1]
        mls             v23.8h,  v7.8h,      v6.h[0]
        ext8             v0.16b,     v24.16b, v24.16b, #2
        ext8             v1.16b,     v24.16b, v24.16b, #3
        uaddl           v0.8h,      v0.8b,      v1.8b
        ext8             v2.16b,     v24.16b, v24.16b, #1
        ext8             v3.16b,     v24.16b, v24.16b, #4
        uaddl           v2.8h,      v2.8b,      v3.8b
        ext8             v30.16b,    v24.16b, v24.16b, #5
        uaddl           v24.8h,  v24.8b,  v30.8b
        ext8             v4.16b,     v25.16b, v25.16b, #2
        mla             v24.8h,  v0.8h,      v6.h[1]
        ext8             v5.16b,     v25.16b, v25.16b, #3
        uaddl           v4.8h,      v4.8b,      v5.8b
        ext8             v7.16b,     v25.16b, v25.16b, #1
        mls             v24.8h,  v2.8h,      v6.h[0]
        ext8             v0.16b,     v25.16b, v25.16b, #4
        uaddl           v7.8h,      v7.8b,      v0.8b
        ext8             v31.16b,    v25.16b, v25.16b, #5
        uaddl           v25.8h,  v25.8b,  v31.8b
        mla             v25.8h,  v4.8h,      v6.h[1]
        mls             v25.8h,  v7.8h,      v6.h[0]
        ext8             v0.16b,     v26.16b, v26.16b, #2
        ext8             v1.16b,     v26.16b, v26.16b, #3
        uaddl           v0.8h,      v0.8b,      v1.8b
        ext8             v2.16b,     v26.16b, v26.16b, #1
        ext8             v3.16b,     v26.16b, v26.16b, #4
        uaddl           v2.8h,      v2.8b,      v3.8b
        ext8             v30.16b,    v26.16b, v26.16b, #5
        uaddl           v26.8h,  v26.8b,  v30.8b
        ext8             v4.16b,     v27.16b, v27.16b, #2
        mla             v26.8h,  v0.8h,      v6.h[1]
        ext8             v5.16b,     v27.16b, v27.16b, #3
        uaddl           v4.8h,      v4.8b,      v5.8b
        ext8             v7.16b,     v27.16b, v27.16b, #1
        mls             v26.8h,  v2.8h,      v6.h[0]
        ext8             v0.16b,     v27.16b, v27.16b, #4
        uaddl           v7.8h,      v7.8b,      v0.8b
        ext8             v31.16b,    v27.16b, v27.16b, #5
        uaddl           v27.8h,  v27.8b,  v31.8b
        mla             v27.8h,  v4.8h,      v6.h[1]
        mls             v27.8h,  v7.8h,      v6.h[0]
        ext8             v0.16b,     v28.16b, v28.16b, #2
        ext8             v1.16b,     v28.16b, v28.16b, #3
        uaddl           v0.8h,      v0.8b,      v1.8b
        ext8             v2.16b,     v28.16b, v28.16b, #1
        ext8             v3.16b,     v28.16b, v28.16b, #4
        uaddl           v2.8h,      v2.8b,      v3.8b
        ext8             v30.16b,    v28.16b, v28.16b, #5
        uaddl           v28.8h,  v28.8b,  v30.8b
        ext8             v4.16b,     v29.16b, v29.16b, #2
        mla             v28.8h,  v0.8h,      v6.h[1]
        ext8             v5.16b,     v29.16b, v29.16b, #3
        uaddl           v4.8h,      v4.8b,      v5.8b
        ext8             v7.16b,     v29.16b, v29.16b, #1
        mls             v28.8h,  v2.8h,      v6.h[0]
        ext8             v0.16b,     v29.16b, v29.16b, #4
        uaddl           v7.8h,      v7.8b,      v0.8b
        ext8             v31.16b,    v29.16b, v29.16b, #5
        uaddl           v29.8h,  v29.8b,  v31.8b
        mla             v29.8h,  v4.8h,      v6.h[1]
        mls             v29.8h,  v7.8h,      v6.h[0]

        saddl           v5.4s,      v18.4h,  v19.4h
        saddl2          v1.4s,      v18.8h,  v19.8h
        saddl           v6.4s,      v17.4h,  v20.4h
        saddl2          v2.4s,      v17.8h,  v20.8h
        saddl           v0.4s,      v16.4h,  v21.4h
        saddl2          v4.4s,      v16.8h,  v21.8h

        shl             v3.4s,  v5.4s,  #4
        shl             v5.4s,  v5.4s,  #2
        shl             v7.4s,  v6.4s,  #2
        add             v5.4s,  v5.4s,  v3.4s
        add             v6.4s,  v6.4s,  v7.4s

        shl             v3.4s,  v1.4s,  #4
        shl             v1.4s,  v1.4s,  #2
        shl             v7.4s,  v2.4s,  #2
        add             v1.4s,  v1.4s,  v3.4s
        add             v2.4s,  v2.4s,  v7.4s

        add             v5.4s,  v5.4s,  v0.4s
        sub             v5.4s,  v5.4s,  v6.4s

        add             v1.4s,  v1.4s,  v4.4s
        sub             v1.4s,  v1.4s,  v2.4s

        rshrn           v5.4h,  v5.4s,  #10
        rshrn2          v5.8h,  v1.4s,  #10

        sqxtun          v16.8b,  v5.8h
        saddl           v5.4s,      v19.4h,  v20.4h
        saddl2          v1.4s,      v19.8h,  v20.8h
        saddl           v6.4s,      v18.4h,  v21.4h
        saddl2          v2.4s,      v18.8h,  v21.8h
        saddl           v0.4s,      v17.4h,  v22.4h
        saddl2          v4.4s,      v17.8h,  v22.8h

        shl             v3.4s,  v5.4s,  #4
        shl             v5.4s,  v5.4s,  #2
        shl             v7.4s,  v6.4s,  #2
        add             v5.4s,  v5.4s,  v3.4s
        add             v6.4s,  v6.4s,  v7.4s

        shl             v3.4s,  v1.4s,  #4
        shl             v1.4s,  v1.4s,  #2
        shl             v7.4s,  v2.4s,  #2
        add             v1.4s,  v1.4s,  v3.4s
        add             v2.4s,  v2.4s,  v7.4s

        add             v5.4s,  v5.4s,  v0.4s
        sub             v5.4s,  v5.4s,  v6.4s

        add             v1.4s,  v1.4s,  v4.4s
        sub             v1.4s,  v1.4s,  v2.4s

        rshrn           v5.4h,  v5.4s,  #10
        rshrn2          v5.8h,  v1.4s,  #10

        sqxtun          v17.8b,  v5.8h

        saddl           v5.4s,      v20.4h,  v21.4h
        saddl2          v1.4s,      v20.8h,  v21.8h
        saddl           v6.4s,      v19.4h,  v22.4h
        saddl2          v2.4s,      v19.8h,  v22.8h
        saddl           v0.4s,      v18.4h,  v23.4h
        saddl2          v4.4s,      v18.8h,  v23.8h

        shl             v3.4s,  v5.4s,  #4
        shl             v5.4s,  v5.4s,  #2
        shl             v7.4s,  v6.4s,  #2
        add             v5.4s,  v5.4s,  v3.4s
        add             v6.4s,  v6.4s,  v7.4s

        shl             v3.4s,  v1.4s,  #4
        shl             v1.4s,  v1.4s,  #2
        shl             v7.4s,  v2.4s,  #2
        add             v1.4s,  v1.4s,  v3.4s
        add             v2.4s,  v2.4s,  v7.4s

        add             v5.4s,  v5.4s,  v0.4s
        sub             v5.4s,  v5.4s,  v6.4s

        add             v1.4s,  v1.4s,  v4.4s
        sub             v1.4s,  v1.4s,  v2.4s

        rshrn           v5.4h,  v5.4s,  #10
        rshrn2          v5.8h,  v1.4s,  #10

        sqxtun          v18.8b,  v5.8h
        saddl           v5.4s,      v21.4h,  v22.4h
        saddl2          v1.4s,      v21.8h,  v22.8h
        saddl           v6.4s,      v20.4h,  v23.4h
        saddl2          v2.4s,      v20.8h,  v23.8h
        saddl           v0.4s,      v19.4h,  v24.4h
        saddl2          v4.4s,      v19.8h,  v24.8h

        shl             v3.4s,  v5.4s,  #4
        shl             v5.4s,  v5.4s,  #2
        shl             v7.4s,  v6.4s,  #2
        add             v5.4s,  v5.4s,  v3.4s
        add             v6.4s,  v6.4s,  v7.4s

        shl             v3.4s,  v1.4s,  #4
        shl             v1.4s,  v1.4s,  #2
        shl             v7.4s,  v2.4s,  #2
        add             v1.4s,  v1.4s,  v3.4s
        add             v2.4s,  v2.4s,  v7.4s

        add             v5.4s,  v5.4s,  v0.4s
        sub             v5.4s,  v5.4s,  v6.4s

        add             v1.4s,  v1.4s,  v4.4s
        sub             v1.4s,  v1.4s,  v2.4s

        rshrn           v5.4h,  v5.4s,  #10
        rshrn2          v5.8h,  v1.4s,  #10

        sqxtun          v19.8b,  v5.8h

        saddl           v5.4s,      v22.4h,  v23.4h
        saddl2          v1.4s,      v22.8h,  v23.8h
        saddl           v6.4s,      v21.4h,  v24.4h
        saddl2          v2.4s,      v21.8h,  v24.8h
        saddl           v0.4s,      v20.4h,  v25.4h
        saddl2          v4.4s,      v20.8h,  v25.8h

        shl             v3.4s,  v5.4s,  #4
        shl             v5.4s,  v5.4s,  #2
        shl             v7.4s,  v6.4s,  #2
        add             v5.4s,  v5.4s,  v3.4s
        add             v6.4s,  v6.4s,  v7.4s

        shl             v3.4s,  v1.4s,  #4
        shl             v1.4s,  v1.4s,  #2
        shl             v7.4s,  v2.4s,  #2
        add             v1.4s,  v1.4s,  v3.4s
        add             v2.4s,  v2.4s,  v7.4s

        add             v5.4s,  v5.4s,  v0.4s
        sub             v5.4s,  v5.4s,  v6.4s

        add             v1.4s,  v1.4s,  v4.4s
        sub             v1.4s,  v1.4s,  v2.4s

        rshrn           v5.4h,  v5.4s,  #10
        rshrn2          v5.8h,  v1.4s,  #10

        sqxtun          v20.8b,  v5.8h
        saddl           v5.4s,      v23.4h,  v24.4h
        saddl2          v1.4s,      v23.8h,  v24.8h
        saddl           v6.4s,      v22.4h,  v25.4h
        saddl2          v2.4s,      v22.8h,  v25.8h
        saddl           v0.4s,      v21.4h,  v26.4h
        saddl2          v4.4s,      v21.8h,  v26.8h

        shl             v3.4s,  v5.4s,  #4
        shl             v5.4s,  v5.4s,  #2
        shl             v7.4s,  v6.4s,  #2
        add             v5.4s,  v5.4s,  v3.4s
        add             v6.4s,  v6.4s,  v7.4s

        shl             v3.4s,  v1.4s,  #4
        shl             v1.4s,  v1.4s,  #2
        shl             v7.4s,  v2.4s,  #2
        add             v1.4s,  v1.4s,  v3.4s
        add             v2.4s,  v2.4s,  v7.4s

        add             v5.4s,  v5.4s,  v0.4s
        sub             v5.4s,  v5.4s,  v6.4s

        add             v1.4s,  v1.4s,  v4.4s
        sub             v1.4s,  v1.4s,  v2.4s

        rshrn           v5.4h,  v5.4s,  #10
        rshrn2          v5.8h,  v1.4s,  #10

        sqxtun          v21.8b,  v5.8h

        saddl           v5.4s,      v24.4h,  v25.4h
        saddl2          v1.4s,      v24.8h,  v25.8h
        saddl           v6.4s,      v23.4h,  v26.4h
        saddl2          v2.4s,      v23.8h,  v26.8h
        saddl           v0.4s,      v22.4h,  v27.4h
        saddl2          v4.4s,      v22.8h,  v27.8h

        shl             v3.4s,  v5.4s,  #4
        shl             v5.4s,  v5.4s,  #2
        shl             v7.4s,  v6.4s,  #2
        add             v5.4s,  v5.4s,  v3.4s
        add             v6.4s,  v6.4s,  v7.4s

        shl             v3.4s,  v1.4s,  #4
        shl             v1.4s,  v1.4s,  #2
        shl             v7.4s,  v2.4s,  #2
        add             v1.4s,  v1.4s,  v3.4s
        add             v2.4s,  v2.4s,  v7.4s

        add             v5.4s,  v5.4s,  v0.4s
        sub             v5.4s,  v5.4s,  v6.4s

        add             v1.4s,  v1.4s,  v4.4s
        sub             v1.4s,  v1.4s,  v2.4s

        rshrn           v5.4h,  v5.4s,  #10
        rshrn2          v5.8h,  v1.4s,  #10

        sqxtun          v22.8b,  v5.8h
        saddl           v5.4s,      v25.4h,  v26.4h
        saddl2          v1.4s,      v25.8h,  v26.8h
        saddl           v6.4s,      v24.4h,  v27.4h
        saddl2          v2.4s,      v24.8h,  v27.8h
        saddl           v0.4s,      v23.4h,  v28.4h
        saddl2          v4.4s,      v23.8h,  v28.8h

        shl             v3.4s,  v5.4s,  #4
        shl             v5.4s,  v5.4s,  #2
        shl             v7.4s,  v6.4s,  #2
        add             v5.4s,  v5.4s,  v3.4s
        add             v6.4s,  v6.4s,  v7.4s

        shl             v3.4s,  v1.4s,  #4
        shl             v1.4s,  v1.4s,  #2
        shl             v7.4s,  v2.4s,  #2
        add             v1.4s,  v1.4s,  v3.4s
        add             v2.4s,  v2.4s,  v7.4s

        add             v5.4s,  v5.4s,  v0.4s
        sub             v5.4s,  v5.4s,  v6.4s

        add             v1.4s,  v1.4s,  v4.4s
        sub             v1.4s,  v1.4s,  v2.4s

        rshrn           v5.4h,  v5.4s,  #10
        rshrn2          v5.8h,  v1.4s,  #10

        sqxtun          v23.8b,  v5.8h

        ret




        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16


put_h264_qpel8_hv_lowpass_neon
        mov             x10, x30
        bl              put_h264_qpel8_hv_lowpass_neon_top

        st1             {v16.8b},     [x0], x2
        st1             {v17.8b},     [x0], x2
        st1             {v18.8b},     [x0], x2
        st1             {v19.8b},     [x0], x2
        st1             {v20.8b},     [x0], x2
        st1             {v21.8b},     [x0], x2
        st1             {v22.8b},     [x0], x2
        st1             {v23.8b},     [x0], x2

        ret             x10


        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16


avg_h264_qpel8_hv_lowpass_neon
        mov             x10, x30
        bl              put_h264_qpel8_hv_lowpass_neon_top
        ld1             {v0.8b},      [x0], x2
        ld1             {v1.8b},      [x0], x2
        ld1             {v2.8b},      [x0], x2
        urhadd          v16.8b, v16.8b, v0.8b
        ld1             {v3.8b},      [x0], x2
        urhadd          v17.8b, v17.8b, v1.8b
        ld1             {v4.8b},      [x0], x2
        urhadd          v18.8b, v18.8b, v2.8b
        ld1             {v5.8b},      [x0], x2
        urhadd          v19.8b, v19.8b, v3.8b
        ld1             {v6.8b},      [x0], x2
        urhadd          v20.8b, v20.8b, v4.8b
        ld1             {v7.8b},      [x0], x2
        urhadd          v21.8b, v21.8b, v5.8b
        urhadd          v22.8b, v22.8b, v6.8b
        urhadd          v23.8b, v23.8b, v7.8b
        sub             x0,  x0,  x2,  lsl #3

        st1             {v16.8b},     [x0], x2
        st1             {v17.8b},     [x0], x2
        st1             {v18.8b},     [x0], x2
        st1             {v19.8b},     [x0], x2
        st1             {v20.8b},     [x0], x2
        st1             {v21.8b},     [x0], x2
        st1             {v22.8b},     [x0], x2
        st1             {v23.8b},     [x0], x2

        ret             x10




        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16


put_h264_qpel8_hv_lowpass_l2_neon
        mov             x10, x30
        bl              put_h264_qpel8_hv_lowpass_neon_top

        ld1             {v0.8b, v1.8b},  [x2], #16
        ld1             {v2.8b, v3.8b},  [x2], #16
        urhadd          v0.8b,  v0.8b,  v16.8b
        urhadd          v1.8b,  v1.8b,  v17.8b
        ld1             {v4.8b, v5.8b},  [x2], #16
        urhadd          v2.8b,  v2.8b,  v18.8b
        urhadd          v3.8b,  v3.8b,  v19.8b
        ld1             {v6.8b, v7.8b},  [x2], #16
        urhadd          v4.8b,  v4.8b,  v20.8b
        urhadd          v5.8b,  v5.8b,  v21.8b
        urhadd          v6.8b,  v6.8b,  v22.8b
        urhadd          v7.8b,  v7.8b,  v23.8b
        st1             {v0.8b},      [x0], x3
        st1             {v1.8b},      [x0], x3
        st1             {v2.8b},      [x0], x3
        st1             {v3.8b},      [x0], x3
        st1             {v4.8b},      [x0], x3
        st1             {v5.8b},      [x0], x3
        st1             {v6.8b},      [x0], x3
        st1             {v7.8b},      [x0], x3

        ret             x10


        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16


avg_h264_qpel8_hv_lowpass_l2_neon
        mov             x10, x30
        bl              put_h264_qpel8_hv_lowpass_neon_top

        ld1             {v0.8b, v1.8b},  [x2], #16
        ld1             {v2.8b, v3.8b},  [x2], #16
        urhadd          v0.8b,  v0.8b,  v16.8b
        urhadd          v1.8b,  v1.8b,  v17.8b
        ld1             {v4.8b, v5.8b},  [x2], #16
        urhadd          v2.8b,  v2.8b,  v18.8b
        urhadd          v3.8b,  v3.8b,  v19.8b
        ld1             {v6.8b, v7.8b},  [x2], #16
        urhadd          v4.8b,  v4.8b,  v20.8b
        urhadd          v5.8b,  v5.8b,  v21.8b
        urhadd          v6.8b,  v6.8b,  v22.8b
        urhadd          v7.8b,  v7.8b,  v23.8b
        ld1             {v16.8b},     [x0], x3
        ld1             {v17.8b},     [x0], x3
        ld1             {v18.8b},     [x0], x3
        urhadd          v0.8b,  v0.8b,  v16.8b
        ld1             {v19.8b},     [x0], x3
        urhadd          v1.8b,  v1.8b,  v17.8b
        ld1             {v20.8b},     [x0], x3
        urhadd          v2.8b,  v2.8b,  v18.8b
        ld1             {v21.8b},     [x0], x3
        urhadd          v3.8b,  v3.8b,  v19.8b
        ld1             {v22.8b},     [x0], x3
        urhadd          v4.8b,  v4.8b,  v20.8b
        ld1             {v23.8b},     [x0], x3
        urhadd          v5.8b,  v5.8b,  v21.8b
        urhadd          v6.8b,  v6.8b,  v22.8b
        urhadd          v7.8b,  v7.8b,  v23.8b
        sub             x0,  x0,  x3,  lsl #3
        st1             {v0.8b},      [x0], x3
        st1             {v1.8b},      [x0], x3
        st1             {v2.8b},      [x0], x3
        st1             {v3.8b},      [x0], x3
        st1             {v4.8b},      [x0], x3
        st1             {v5.8b},      [x0], x3
        st1             {v6.8b},      [x0], x3
        st1             {v7.8b},      [x0], x3

        ret             x10




        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16


put_h264_qpel16_hv_lowpass_neon
        mov             x13, x30
        bl              put_h264_qpel8_hv_lowpass_neon
        sub             x1,  x1,  x3, lsl #2
        bl              put_h264_qpel8_hv_lowpass_neon
        sub             x1,  x1,  x3, lsl #4
        sub             x1,  x1,  x3, lsl #2
        add             x1,  x1,  #8
        sub             x0,  x0,  x2, lsl #4
        add             x0,  x0,  #8
        bl              put_h264_qpel8_hv_lowpass_neon
        sub             x1,  x1,  x3, lsl #2
        mov             x30, x13
        b               put_h264_qpel8_hv_lowpass_neon



        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16


put_h264_qpel16_hv_lowpass_l2_neon
        mov             x13, x30
        sub             x2,  x4,  #256
        bl              put_h264_qpel8_hv_lowpass_l2_neon
        sub             x1,  x1,  x3, lsl #2
        bl              put_h264_qpel8_hv_lowpass_l2_neon
        sub             x1,  x1,  x3, lsl #4
        sub             x1,  x1,  x3, lsl #2
        add             x1,  x1,  #8
        sub             x0,  x0,  x3, lsl #4
        add             x0,  x0,  #8
        bl              put_h264_qpel8_hv_lowpass_l2_neon
        sub             x1,  x1,  x3, lsl #2
        mov             x30, x13
        b               put_h264_qpel8_hv_lowpass_l2_neon


        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16


avg_h264_qpel16_hv_lowpass_neon
        mov             x13, x30
        bl              avg_h264_qpel8_hv_lowpass_neon
        sub             x1,  x1,  x3, lsl #2
        bl              avg_h264_qpel8_hv_lowpass_neon
        sub             x1,  x1,  x3, lsl #4
        sub             x1,  x1,  x3, lsl #2
        add             x1,  x1,  #8
        sub             x0,  x0,  x2, lsl #4
        add             x0,  x0,  #8
        bl              avg_h264_qpel8_hv_lowpass_neon
        sub             x1,  x1,  x3, lsl #2
        mov             x30, x13
        b               avg_h264_qpel8_hv_lowpass_neon



        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16


avg_h264_qpel16_hv_lowpass_l2_neon
        mov             x13, x30
        sub             x2,  x4,  #256
        bl              avg_h264_qpel8_hv_lowpass_l2_neon
        sub             x1,  x1,  x3, lsl #2
        bl              avg_h264_qpel8_hv_lowpass_l2_neon
        sub             x1,  x1,  x3, lsl #4
        sub             x1,  x1,  x3, lsl #2
        add             x1,  x1,  #8
        sub             x0,  x0,  x3, lsl #4
        add             x0,  x0,  #8
        bl              avg_h264_qpel8_hv_lowpass_l2_neon
        sub             x1,  x1,  x3, lsl #2
        mov             x30, x13
        b               avg_h264_qpel8_hv_lowpass_l2_neon




        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16

        EXPORT ff_put_h264_qpel8_mc10_neon





ff_put_h264_qpel8_mc10_neon
        










        movz            w3, #20, lsl #16
        movk            w3, #5
        mov             v6.s[0], w3
        mov             x3,  x1
        sub             x1,  x1,  #2
        mov             x12, #8
        b               put_h264_qpel8_h_lowpass_l2_neon



        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16

        EXPORT ff_put_h264_qpel8_mc20_neon





ff_put_h264_qpel8_mc20_neon
        










        movz            w3, #20, lsl #16
        movk            w3, #5
        mov             v6.s[0], w3
        sub             x1,  x1,  #2
        mov             x3,  x2
        mov             x12, #8
        b               put_h264_qpel8_h_lowpass_neon



        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16

        EXPORT ff_put_h264_qpel8_mc30_neon





ff_put_h264_qpel8_mc30_neon
        










        movz            w3, #20, lsl #16
        movk            w3, #5
        mov             v6.s[0], w3
        add             x3,  x1,  #1
        sub             x1,  x1,  #2
        mov             x12, #8
        b               put_h264_qpel8_h_lowpass_l2_neon



        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16

        EXPORT ff_put_h264_qpel8_mc01_neon





ff_put_h264_qpel8_mc01_neon
        










        mov             x14, x30
        mov             x12, x1
put_h264_qpel8_mc01
        movz            w3, #20, lsl #16
        movk            w3, #5
        mov             v6.s[0], w3
        mov             x3,  x2
        sub             x1,  x1,  x2, lsl #1
        bl              put_h264_qpel8_v_lowpass_l2_neon
        ret             x14



        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16

        EXPORT ff_put_h264_qpel8_mc11_neon





ff_put_h264_qpel8_mc11_neon
        










        mov             x14, x30
        mov             x8,  x0
        mov             x9,  x1
put_h264_qpel8_mc11
        movz            w3, #20, lsl #16
        movk            w3, #5
        mov             v6.s[0], w3
        mov             x11, sp
        sub             sp,  sp,  #64
        mov             x0,  sp
        sub             x1,  x1,  #2
        mov             x3,  #8
        mov             x12, #8
        bl              put_h264_qpel8_h_lowpass_neon
        mov             x0,  x8
        mov             x3,  x2
        mov             x12, sp
        sub             x1,  x9,  x2, lsl #1
        mov             x2,  #8
        bl              put_h264_qpel8_v_lowpass_l2_neon
        mov             sp,  x11
        ret             x14



        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16

        EXPORT ff_put_h264_qpel8_mc21_neon





ff_put_h264_qpel8_mc21_neon
        










        mov             x14, x30
        mov             x8,  x0
        mov             x9,  x1
put_h264_qpel8_mc21
        movz            w3, #20, lsl #16
        movk            w3, #5
        mov             v6.s[0], w3
        mov             x11, sp
        sub             sp,  sp,  #(8*8+16*12)
        sub             x1,  x1,  #2
        mov             x3,  #8
        mov             x0,  sp
        mov             x12, #8
        bl              put_h264_qpel8_h_lowpass_neon
        mov             x4,  x0
        mov             x0,  x8
        sub             x1,  x9,  x2, lsl #1
        sub             x1,  x1,  #2
        mov             x3,  x2
        sub             x2,  x4,  #64
        bl              put_h264_qpel8_hv_lowpass_l2_neon
        mov             sp,  x11
        ret             x14



        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16

        EXPORT ff_put_h264_qpel8_mc31_neon





ff_put_h264_qpel8_mc31_neon
        










        add             x1,  x1,  #1
        mov             x14, x30
        mov             x8,  x0
        mov             x9,  x1
        sub             x1,  x1,  #1
        b               put_h264_qpel8_mc11



        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16

        EXPORT ff_put_h264_qpel8_mc02_neon





ff_put_h264_qpel8_mc02_neon
        










        mov             x14, x30
        movz            w3, #20, lsl #16
        movk            w3, #5
        mov             v6.s[0], w3
        sub             x1,  x1,  x2, lsl #1
        mov             x3,  x2
        bl              put_h264_qpel8_v_lowpass_neon
        ret             x14



        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16

        EXPORT ff_put_h264_qpel8_mc12_neon





ff_put_h264_qpel8_mc12_neon
        










        mov             x14, x30
        mov             x8,  x0
        mov             x9,  x1
put_h264_qpel8_mc12
        movz            w3, #20, lsl #16
        movk            w3, #5
        mov             v6.s[0], w3
        mov             x11, sp
        sub             sp,  sp,  #(8*8+16*12)
        sub             x1,  x1,  x2, lsl #1
        mov             x3,  x2
        mov             x2,  #8
        mov             x0,  sp
        bl              put_h264_qpel8_v_lowpass_neon
        mov             x4,  x0
        mov             x0,  x8
        sub             x1,  x9,  x3, lsl #1
        sub             x1,  x1,  #2
        sub             x2,  x4,  #64
        bl              put_h264_qpel8_hv_lowpass_l2_neon
        mov             sp,  x11
        ret             x14



        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16

        EXPORT ff_put_h264_qpel8_mc22_neon





ff_put_h264_qpel8_mc22_neon
        










        mov             x14, x30
        mov             x11, sp
        sub             x1,  x1,  x2, lsl #1
        sub             x1,  x1,  #2
        mov             x3,  x2
        bl              put_h264_qpel8_hv_lowpass_neon
        mov             sp,  x11
        ret             x14



        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16

        EXPORT ff_put_h264_qpel8_mc32_neon





ff_put_h264_qpel8_mc32_neon
        










        mov             x14, x30
        mov             x8,  x0
        mov             x9,  x1
        add             x1,  x1,  #1
        b               put_h264_qpel8_mc12



        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16

        EXPORT ff_put_h264_qpel8_mc03_neon





ff_put_h264_qpel8_mc03_neon
        










        mov             x14, x30
        add             x12, x1,  x2
        b               put_h264_qpel8_mc01



        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16

        EXPORT ff_put_h264_qpel8_mc13_neon





ff_put_h264_qpel8_mc13_neon
        










        mov             x14, x30
        mov             x8,  x0
        mov             x9,  x1
        add             x1,  x1,  x2
        b               put_h264_qpel8_mc11



        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16

        EXPORT ff_put_h264_qpel8_mc23_neon





ff_put_h264_qpel8_mc23_neon
        










        mov             x14, x30
        mov             x8,  x0
        mov             x9,  x1
        add             x1,  x1,  x2
        b               put_h264_qpel8_mc21



        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16

        EXPORT ff_put_h264_qpel8_mc33_neon





ff_put_h264_qpel8_mc33_neon
        










        add             x1,  x1,  #1
        mov             x14, x30
        mov             x8,  x0
        mov             x9,  x1
        add             x1,  x1,  x2
        sub             x1,  x1,  #1
        b               put_h264_qpel8_mc11


        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16

        EXPORT ff_avg_h264_qpel8_mc10_neon





ff_avg_h264_qpel8_mc10_neon
        










        movz            w3, #20, lsl #16
        movk            w3, #5
        mov             v6.s[0], w3
        mov             x3,  x1
        sub             x1,  x1,  #2
        mov             x12, #8
        b               avg_h264_qpel8_h_lowpass_l2_neon



        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16

        EXPORT ff_avg_h264_qpel8_mc20_neon





ff_avg_h264_qpel8_mc20_neon
        










        movz            w3, #20, lsl #16
        movk            w3, #5
        mov             v6.s[0], w3
        sub             x1,  x1,  #2
        mov             x3,  x2
        mov             x12, #8
        b               avg_h264_qpel8_h_lowpass_neon



        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16

        EXPORT ff_avg_h264_qpel8_mc30_neon





ff_avg_h264_qpel8_mc30_neon
        










        movz            w3, #20, lsl #16
        movk            w3, #5
        mov             v6.s[0], w3
        add             x3,  x1,  #1
        sub             x1,  x1,  #2
        mov             x12, #8
        b               avg_h264_qpel8_h_lowpass_l2_neon



        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16

        EXPORT ff_avg_h264_qpel8_mc01_neon





ff_avg_h264_qpel8_mc01_neon
        










        mov             x14, x30
        mov             x12, x1
avg_h264_qpel8_mc01
        movz            w3, #20, lsl #16
        movk            w3, #5
        mov             v6.s[0], w3
        mov             x3,  x2
        sub             x1,  x1,  x2, lsl #1
        bl              avg_h264_qpel8_v_lowpass_l2_neon
        ret             x14



        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16

        EXPORT ff_avg_h264_qpel8_mc11_neon





ff_avg_h264_qpel8_mc11_neon
        










        mov             x14, x30
        mov             x8,  x0
        mov             x9,  x1
avg_h264_qpel8_mc11
        movz            w3, #20, lsl #16
        movk            w3, #5
        mov             v6.s[0], w3
        mov             x11, sp
        sub             sp,  sp,  #64
        mov             x0,  sp
        sub             x1,  x1,  #2
        mov             x3,  #8
        mov             x12, #8
        bl              put_h264_qpel8_h_lowpass_neon
        mov             x0,  x8
        mov             x3,  x2
        mov             x12, sp
        sub             x1,  x9,  x2, lsl #1
        mov             x2,  #8
        bl              avg_h264_qpel8_v_lowpass_l2_neon
        mov             sp,  x11
        ret             x14



        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16

        EXPORT ff_avg_h264_qpel8_mc21_neon





ff_avg_h264_qpel8_mc21_neon
        










        mov             x14, x30
        mov             x8,  x0
        mov             x9,  x1
avg_h264_qpel8_mc21
        movz            w3, #20, lsl #16
        movk            w3, #5
        mov             v6.s[0], w3
        mov             x11, sp
        sub             sp,  sp,  #(8*8+16*12)
        sub             x1,  x1,  #2
        mov             x3,  #8
        mov             x0,  sp
        mov             x12, #8
        bl              put_h264_qpel8_h_lowpass_neon
        mov             x4,  x0
        mov             x0,  x8
        sub             x1,  x9,  x2, lsl #1
        sub             x1,  x1,  #2
        mov             x3,  x2
        sub             x2,  x4,  #64
        bl              avg_h264_qpel8_hv_lowpass_l2_neon
        mov             sp,  x11
        ret             x14



        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16

        EXPORT ff_avg_h264_qpel8_mc31_neon





ff_avg_h264_qpel8_mc31_neon
        










        add             x1,  x1,  #1
        mov             x14, x30
        mov             x8,  x0
        mov             x9,  x1
        sub             x1,  x1,  #1
        b               avg_h264_qpel8_mc11



        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16

        EXPORT ff_avg_h264_qpel8_mc02_neon





ff_avg_h264_qpel8_mc02_neon
        










        mov             x14, x30
        movz            w3, #20, lsl #16
        movk            w3, #5
        mov             v6.s[0], w3
        sub             x1,  x1,  x2, lsl #1
        mov             x3,  x2
        bl              avg_h264_qpel8_v_lowpass_neon
        ret             x14



        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16

        EXPORT ff_avg_h264_qpel8_mc12_neon





ff_avg_h264_qpel8_mc12_neon
        










        mov             x14, x30
        mov             x8,  x0
        mov             x9,  x1
avg_h264_qpel8_mc12
        movz            w3, #20, lsl #16
        movk            w3, #5
        mov             v6.s[0], w3
        mov             x11, sp
        sub             sp,  sp,  #(8*8+16*12)
        sub             x1,  x1,  x2, lsl #1
        mov             x3,  x2
        mov             x2,  #8
        mov             x0,  sp
        bl              put_h264_qpel8_v_lowpass_neon
        mov             x4,  x0
        mov             x0,  x8
        sub             x1,  x9,  x3, lsl #1
        sub             x1,  x1,  #2
        sub             x2,  x4,  #64
        bl              avg_h264_qpel8_hv_lowpass_l2_neon
        mov             sp,  x11
        ret             x14



        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16

        EXPORT ff_avg_h264_qpel8_mc22_neon





ff_avg_h264_qpel8_mc22_neon
        










        mov             x14, x30
        mov             x11, sp
        sub             x1,  x1,  x2, lsl #1
        sub             x1,  x1,  #2
        mov             x3,  x2
        bl              avg_h264_qpel8_hv_lowpass_neon
        mov             sp,  x11
        ret             x14



        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16

        EXPORT ff_avg_h264_qpel8_mc32_neon





ff_avg_h264_qpel8_mc32_neon
        










        mov             x14, x30
        mov             x8,  x0
        mov             x9,  x1
        add             x1,  x1,  #1
        b               avg_h264_qpel8_mc12



        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16

        EXPORT ff_avg_h264_qpel8_mc03_neon





ff_avg_h264_qpel8_mc03_neon
        










        mov             x14, x30
        add             x12, x1,  x2
        b               avg_h264_qpel8_mc01



        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16

        EXPORT ff_avg_h264_qpel8_mc13_neon





ff_avg_h264_qpel8_mc13_neon
        










        mov             x14, x30
        mov             x8,  x0
        mov             x9,  x1
        add             x1,  x1,  x2
        b               avg_h264_qpel8_mc11



        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16

        EXPORT ff_avg_h264_qpel8_mc23_neon





ff_avg_h264_qpel8_mc23_neon
        










        mov             x14, x30
        mov             x8,  x0
        mov             x9,  x1
        add             x1,  x1,  x2
        b               avg_h264_qpel8_mc21



        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16

        EXPORT ff_avg_h264_qpel8_mc33_neon





ff_avg_h264_qpel8_mc33_neon
        










        add             x1,  x1,  #1
        mov             x14, x30
        mov             x8,  x0
        mov             x9,  x1
        add             x1,  x1,  x2
        sub             x1,  x1,  #1
        b               avg_h264_qpel8_mc11




        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16

        EXPORT ff_put_h264_qpel16_mc10_neon





ff_put_h264_qpel16_mc10_neon
        










        movz            w3, #20, lsl #16
        movk            w3, #5
        mov             v6.s[0], w3
        mov             x3,  x1
        sub             x1,  x1,  #2
        b               put_h264_qpel16_h_lowpass_l2_neon



        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16

        EXPORT ff_put_h264_qpel16_mc20_neon





ff_put_h264_qpel16_mc20_neon
        










        movz            w3, #20, lsl #16
        movk            w3, #5
        mov             v6.s[0], w3
        sub             x1,  x1,  #2
        mov             x3,  x2
        b               put_h264_qpel16_h_lowpass_neon



        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16

        EXPORT ff_put_h264_qpel16_mc30_neon





ff_put_h264_qpel16_mc30_neon
        










        movz            w3, #20, lsl #16
        movk            w3, #5
        mov             v6.s[0], w3
        add             x3,  x1,  #1
        sub             x1,  x1,  #2
        b               put_h264_qpel16_h_lowpass_l2_neon



        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16

        EXPORT ff_put_h264_qpel16_mc01_neon





ff_put_h264_qpel16_mc01_neon
        










        mov             x14, x30
        mov             x12, x1
put_h264_qpel16_mc01
        movz            w3, #20, lsl #16
        movk            w3, #5
        mov             v6.s[0], w3
        mov             x3,  x2
        sub             x1,  x1,  x2, lsl #1
        bl              put_h264_qpel16_v_lowpass_l2_neon
        ret             x14



        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16

        EXPORT ff_put_h264_qpel16_mc11_neon





ff_put_h264_qpel16_mc11_neon
        










        mov             x14, x30
        mov             x8,  x0
        mov             x9,  x1
put_h264_qpel16_mc11
        movz            w3, #20, lsl #16
        movk            w3, #5
        mov             v6.s[0], w3
        mov             x11, sp
        sub             sp,  sp,  #256
        mov             x0,  sp
        sub             x1,  x1,  #2
        mov             x3,  #16
        bl              put_h264_qpel16_h_lowpass_neon
        mov             x0,  x8
        mov             x3,  x2
        mov             x12, sp
        sub             x1,  x9,  x2, lsl #1
        mov             x2,  #16
        bl              put_h264_qpel16_v_lowpass_l2_neon
        mov             sp,  x11
        ret             x14



        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16

        EXPORT ff_put_h264_qpel16_mc21_neon





ff_put_h264_qpel16_mc21_neon
        










        mov             x14, x30
        mov             x8,  x0
        mov             x9,  x1
put_h264_qpel16_mc21
        movz            w3, #20, lsl #16
        movk            w3, #5
        mov             v6.s[0], w3
        mov             x11, sp
        sub             sp,  sp,  #(16*16+16*12)
        sub             x1,  x1,  #2
        mov             x0,  sp
        bl              put_h264_qpel16_h_lowpass_neon_packed
        mov             x4,  x0
        mov             x0,  x8
        sub             x1,  x9,  x2, lsl #1
        sub             x1,  x1,  #2
        mov             x3,  x2
        bl              put_h264_qpel16_hv_lowpass_l2_neon
        mov             sp,  x11
        ret             x14



        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16

        EXPORT ff_put_h264_qpel16_mc31_neon





ff_put_h264_qpel16_mc31_neon
        










        add             x1,  x1,  #1
        mov             x14, x30
        mov             x8,  x0
        mov             x9,  x1
        sub             x1,  x1,  #1
        b               put_h264_qpel16_mc11



        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16

        EXPORT ff_put_h264_qpel16_mc02_neon





ff_put_h264_qpel16_mc02_neon
        










        mov             x14, x30
        movz            w3, #20, lsl #16
        movk            w3, #5
        mov             v6.s[0], w3
        sub             x1,  x1,  x2, lsl #1
        mov             x3,  x2
        bl              put_h264_qpel16_v_lowpass_neon
        ret             x14



        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16

        EXPORT ff_put_h264_qpel16_mc12_neon





ff_put_h264_qpel16_mc12_neon
        










        mov             x14, x30
        mov             x8,  x0
        mov             x9,  x1
put_h264_qpel16_mc12
        movz            w3, #20, lsl #16
        movk            w3, #5
        mov             v6.s[0], w3
        mov             x11, sp
        sub             sp,  sp,  #(16*16+16*12)
        sub             x1,  x1,  x2, lsl #1
        mov             x0,  sp
        mov             x3,  x2
        bl              put_h264_qpel16_v_lowpass_neon_packed
        mov             x4,  x0
        mov             x0,  x8
        sub             x1,  x9,  x3, lsl #1
        sub             x1,  x1,  #2
        mov             x2,  x3
        bl              put_h264_qpel16_hv_lowpass_l2_neon
        mov             sp,  x11
        ret             x14



        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16

        EXPORT ff_put_h264_qpel16_mc22_neon





ff_put_h264_qpel16_mc22_neon
        










        mov             x14, x30
        movz            w3, #20, lsl #16
        movk            w3, #5
        mov             v6.s[0], w3
        mov             x11, sp
        sub             x1,  x1,  x2, lsl #1
        sub             x1,  x1,  #2
        mov             x3,  x2
        bl              put_h264_qpel16_hv_lowpass_neon
        mov             sp,  x11 
        ret             x14



        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16

        EXPORT ff_put_h264_qpel16_mc32_neon





ff_put_h264_qpel16_mc32_neon
        










        mov             x14, x30
        mov             x8,  x0
        mov             x9,  x1
        add             x1,  x1,  #1
        b               put_h264_qpel16_mc12



        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16

        EXPORT ff_put_h264_qpel16_mc03_neon





ff_put_h264_qpel16_mc03_neon
        










        mov             x14, x30
        add             x12, x1,  x2
        b               put_h264_qpel16_mc01



        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16

        EXPORT ff_put_h264_qpel16_mc13_neon





ff_put_h264_qpel16_mc13_neon
        










        mov             x14, x30
        mov             x8,  x0
        mov             x9,  x1
        add             x1,  x1,  x2
        b               put_h264_qpel16_mc11



        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16

        EXPORT ff_put_h264_qpel16_mc23_neon





ff_put_h264_qpel16_mc23_neon
        










        mov             x14, x30
        mov             x8,  x0
        mov             x9,  x1
        add             x1,  x1,  x2
        b               put_h264_qpel16_mc21



        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16

        EXPORT ff_put_h264_qpel16_mc33_neon





ff_put_h264_qpel16_mc33_neon
        










        add             x1,  x1,  #1
        mov             x14, x30
        mov             x8,  x0
        mov             x9,  x1
        add             x1,  x1,  x2
        sub             x1,  x1,  #1
        b               put_h264_qpel16_mc11


        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16

        EXPORT ff_avg_h264_qpel16_mc10_neon





ff_avg_h264_qpel16_mc10_neon
        










        movz            w3, #20, lsl #16
        movk            w3, #5
        mov             v6.s[0], w3
        mov             x3,  x1
        sub             x1,  x1,  #2
        b               avg_h264_qpel16_h_lowpass_l2_neon



        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16

        EXPORT ff_avg_h264_qpel16_mc20_neon





ff_avg_h264_qpel16_mc20_neon
        










        movz            w3, #20, lsl #16
        movk            w3, #5
        mov             v6.s[0], w3
        sub             x1,  x1,  #2
        mov             x3,  x2
        b               avg_h264_qpel16_h_lowpass_neon



        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16

        EXPORT ff_avg_h264_qpel16_mc30_neon





ff_avg_h264_qpel16_mc30_neon
        










        movz            w3, #20, lsl #16
        movk            w3, #5
        mov             v6.s[0], w3
        add             x3,  x1,  #1
        sub             x1,  x1,  #2
        b               avg_h264_qpel16_h_lowpass_l2_neon



        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16

        EXPORT ff_avg_h264_qpel16_mc01_neon





ff_avg_h264_qpel16_mc01_neon
        










        mov             x14, x30
        mov             x12, x1
avg_h264_qpel16_mc01
        movz            w3, #20, lsl #16
        movk            w3, #5
        mov             v6.s[0], w3
        mov             x3,  x2
        sub             x1,  x1,  x2, lsl #1
        bl              avg_h264_qpel16_v_lowpass_l2_neon
        ret             x14



        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16

        EXPORT ff_avg_h264_qpel16_mc11_neon





ff_avg_h264_qpel16_mc11_neon
        










        mov             x14, x30
        mov             x8,  x0
        mov             x9,  x1
avg_h264_qpel16_mc11
        movz            w3, #20, lsl #16
        movk            w3, #5
        mov             v6.s[0], w3
        mov             x11, sp
        sub             sp,  sp,  #256
        mov             x0,  sp
        sub             x1,  x1,  #2
        mov             x3,  #16
        bl              put_h264_qpel16_h_lowpass_neon
        mov             x0,  x8
        mov             x3,  x2
        mov             x12, sp
        sub             x1,  x9,  x2, lsl #1
        mov             x2,  #16
        bl              avg_h264_qpel16_v_lowpass_l2_neon
        mov             sp,  x11
        ret             x14



        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16

        EXPORT ff_avg_h264_qpel16_mc21_neon





ff_avg_h264_qpel16_mc21_neon
        










        mov             x14, x30
        mov             x8,  x0
        mov             x9,  x1
avg_h264_qpel16_mc21
        movz            w3, #20, lsl #16
        movk            w3, #5
        mov             v6.s[0], w3
        mov             x11, sp
        sub             sp,  sp,  #(16*16+16*12)
        sub             x1,  x1,  #2
        mov             x0,  sp
        bl              put_h264_qpel16_h_lowpass_neon_packed
        mov             x4,  x0
        mov             x0,  x8
        sub             x1,  x9,  x2, lsl #1
        sub             x1,  x1,  #2
        mov             x3,  x2
        bl              avg_h264_qpel16_hv_lowpass_l2_neon
        mov             sp,  x11
        ret             x14



        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16

        EXPORT ff_avg_h264_qpel16_mc31_neon





ff_avg_h264_qpel16_mc31_neon
        










        add             x1,  x1,  #1
        mov             x14, x30
        mov             x8,  x0
        mov             x9,  x1
        sub             x1,  x1,  #1
        b               avg_h264_qpel16_mc11



        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16

        EXPORT ff_avg_h264_qpel16_mc02_neon





ff_avg_h264_qpel16_mc02_neon
        










        mov             x14, x30
        movz            w3, #20, lsl #16
        movk            w3, #5
        mov             v6.s[0], w3
        sub             x1,  x1,  x2, lsl #1
        mov             x3,  x2
        bl              avg_h264_qpel16_v_lowpass_neon
        ret             x14



        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16

        EXPORT ff_avg_h264_qpel16_mc12_neon





ff_avg_h264_qpel16_mc12_neon
        










        mov             x14, x30
        mov             x8,  x0
        mov             x9,  x1
avg_h264_qpel16_mc12
        movz            w3, #20, lsl #16
        movk            w3, #5
        mov             v6.s[0], w3
        mov             x11, sp
        sub             sp,  sp,  #(16*16+16*12)
        sub             x1,  x1,  x2, lsl #1
        mov             x0,  sp
        mov             x3,  x2
        bl              put_h264_qpel16_v_lowpass_neon_packed
        mov             x4,  x0
        mov             x0,  x8
        sub             x1,  x9,  x3, lsl #1
        sub             x1,  x1,  #2
        mov             x2,  x3
        bl              avg_h264_qpel16_hv_lowpass_l2_neon
        mov             sp,  x11
        ret             x14



        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16

        EXPORT ff_avg_h264_qpel16_mc22_neon





ff_avg_h264_qpel16_mc22_neon
        










        mov             x14, x30
        movz            w3, #20, lsl #16
        movk            w3, #5
        mov             v6.s[0], w3
        mov             x11, sp
        sub             x1,  x1,  x2, lsl #1
        sub             x1,  x1,  #2
        mov             x3,  x2
        bl              avg_h264_qpel16_hv_lowpass_neon
        mov             sp,  x11 
        ret             x14



        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16

        EXPORT ff_avg_h264_qpel16_mc32_neon





ff_avg_h264_qpel16_mc32_neon
        










        mov             x14, x30
        mov             x8,  x0
        mov             x9,  x1
        add             x1,  x1,  #1
        b               avg_h264_qpel16_mc12



        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16

        EXPORT ff_avg_h264_qpel16_mc03_neon





ff_avg_h264_qpel16_mc03_neon
        










        mov             x14, x30
        add             x12, x1,  x2
        b               avg_h264_qpel16_mc01



        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16

        EXPORT ff_avg_h264_qpel16_mc13_neon





ff_avg_h264_qpel16_mc13_neon
        










        mov             x14, x30
        mov             x8,  x0
        mov             x9,  x1
        add             x1,  x1,  x2
        b               avg_h264_qpel16_mc11



        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16

        EXPORT ff_avg_h264_qpel16_mc23_neon





ff_avg_h264_qpel16_mc23_neon
        










        mov             x14, x30
        mov             x8,  x0
        mov             x9,  x1
        add             x1,  x1,  x2
        b               avg_h264_qpel16_mc21



        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16

        EXPORT ff_avg_h264_qpel16_mc33_neon





ff_avg_h264_qpel16_mc33_neon
        










        add             x1,  x1,  #1
        mov             x14, x30
        mov             x8,  x0
        mov             x9,  x1
        add             x1,  x1,  x2
        sub             x1,  x1,  #1
        b               avg_h264_qpel16_mc11







        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16


put_h264_qpel16_h_lowpass_neon_packed_10
        mov             x4,  x30
        mov             x12, #32
        mov             x3,  #16
        bl              put_h264_qpel8_h_lowpass_neon_10
        sub             x1,  x1,  x2, lsl #4
        add             x1,  x1,  #16
        mov             x12, #32
        mov             x30, x4
        b               put_h264_qpel8_h_lowpass_neon_10




        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16


put_h264_qpel16_h_lowpass_neon_10
        mov             x13, x30
        mov             x12, #32
        bl              put_h264_qpel8_h_lowpass_neon_10
        sub             x0,  x0,  x3, lsl #4
        sub             x1,  x1,  x2, lsl #4
        add             x0,  x0,  #16
        add             x1,  x1,  #16
        mov             x12, #32
        mov             x30, x13



        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16


put_h264_qpel8_h_lowpass_neon_10
temp_label_4
      ld1             {v28.8h, v29.8h}, [x1], x2
        ld1             {v16.8h, v17.8h}, [x1], x2
        subs            x12, x12, #4
        ext8             v2.16b,     v28.16b,  v29.16b, #4
        ext8             v3.16b,     v28.16b,  v29.16b, #6
        add             v2.8h,      v2.8h,       v3.8h
        ext8             v4.16b,     v28.16b,  v29.16b, #2
        ext8             v5.16b,     v28.16b,  v29.16b, #8
        add             v4.8h,      v4.8h,       v5.8h
        ext8             v1.16b,     v28.16b,  v29.16b, #10

        add             v28.8h,  v28.8h,   v1.8h
        ext8             v0.16b,     v16.16b,  v17.16b, #4
        mla             v28.8h,  v2.8h,       v6.h[1]
        ext8             v1.16b,     v16.16b,  v17.16b, #6
        add             v0.8h,      v0.8h,       v1.8h
        ext8             v1.16b,     v16.16b,  v17.16b, #2
        mul             v5.8h,      v4.8h,       v6.h[0]
        uqsub           v28.8h,  v28.8h,   v5.8h
        urshr           v28.8h,  v28.8h,   #5

        ext8             v3.16b,     v16.16b,  v17.16b, #8
        add             v1.8h,      v1.8h,       v3.8h
        ext8             v2.16b,     v16.16b,  v17.16b, #10

        add             v20.8h,  v16.8h,   v2.8h
        mla             v20.8h,  v0.8h,       v6.h[1]
        mul             v5.8h,      v1.8h,       v6.h[0]
        uqsub           v20.8h,  v20.8h,   v5.8h
        mvni            v5.8h,      #0xFC,       lsl #8 
        urshr           v20.8h,  v20.8h,   #5

        umin            v28.8h,  v28.8h,   v5.8h
        umin            v20.8h,  v20.8h,   v5.8h
        st1             {v28.8h},    [x0], x3
        st1             {v20.8h},    [x0], x3
        bne            temp_label_4
        ret


        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16


avg_h264_qpel16_h_lowpass_neon_10
        mov             x13, x30
        mov             x12, #32
        bl              avg_h264_qpel8_h_lowpass_neon_10
        sub             x0,  x0,  x3, lsl #4
        sub             x1,  x1,  x2, lsl #4
        add             x0,  x0,  #16
        add             x1,  x1,  #16
        mov             x12, #32
        mov             x30, x13



        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16


avg_h264_qpel8_h_lowpass_neon_10
temp_label_5
      ld1             {v28.8h, v29.8h}, [x1], x2
        ld1             {v16.8h, v17.8h}, [x1], x2
        subs            x12, x12, #4
        ext8             v2.16b,     v28.16b,  v29.16b, #4
        ext8             v3.16b,     v28.16b,  v29.16b, #6
        add             v2.8h,      v2.8h,       v3.8h
        ext8             v4.16b,     v28.16b,  v29.16b, #2
        ext8             v5.16b,     v28.16b,  v29.16b, #8
        add             v4.8h,      v4.8h,       v5.8h
        ext8             v1.16b,     v28.16b,  v29.16b, #10

        add             v28.8h,  v28.8h,   v1.8h
        ext8             v0.16b,     v16.16b,  v17.16b, #4
        mla             v28.8h,  v2.8h,       v6.h[1]
        ext8             v1.16b,     v16.16b,  v17.16b, #6
        add             v0.8h,      v0.8h,       v1.8h
        ext8             v1.16b,     v16.16b,  v17.16b, #2
        mul             v5.8h,      v4.8h,       v6.h[0]
        uqsub           v28.8h,  v28.8h,   v5.8h
        urshr           v28.8h,  v28.8h,   #5

        ext8             v3.16b,     v16.16b,  v17.16b, #8
        add             v1.8h,      v1.8h,       v3.8h
        ext8             v2.16b,     v16.16b,  v17.16b, #10

        add             v20.8h,  v16.8h,   v2.8h
        mla             v20.8h,  v0.8h,       v6.h[1]
        mul             v5.8h,      v1.8h,       v6.h[0]
        uqsub           v20.8h,  v20.8h,   v5.8h
        mvni            v5.8h,      #0xFC,       lsl #8 
        urshr           v20.8h,  v20.8h,   #5

        umin            v28.8h,  v28.8h,   v5.8h
        umin            v20.8h,  v20.8h,   v5.8h
        ld1             {v2.8h},    [x0], x3
        ld1             {v3.8h},    [x0]
        urhadd          v28.8h, v28.8h, v2.8h
        urhadd          v20.8h, v20.8h, v3.8h
        sub             x0,  x0,  x3
        st1             {v28.8h},    [x0], x3
        st1             {v20.8h},    [x0], x3
        bne            temp_label_5
        ret




        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16


put_h264_qpel16_h_lowpass_l2_neon_10
        mov             x13, x30
        mov             x12, #32
        bl              put_h264_qpel8_h_lowpass_l2_neon_10
        sub             x0,  x0,  x2, lsl #4
        sub             x1,  x1,  x2, lsl #4
        sub             x3,  x3,  x2, lsl #4
        add             x0,  x0,  #16
        add             x1,  x1,  #16
        add             x3,  x3,  #16
        mov             x12, #32
        mov             x30, x13



        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16


put_h264_qpel8_h_lowpass_l2_neon_10
temp_label_6
      ld1             {v26.8h, v27.8h}, [x1], x2
        ld1             {v16.8h, v17.8h}, [x1], x2
        ld1             {v28.8h},     [x3], x2
        ld1             {v29.8h},     [x3], x2
        subs            x12, x12, #4
        ext8             v2.16b,     v26.16b,  v27.16b, #4
        ext8             v3.16b,     v26.16b,  v27.16b, #6
        add             v2.8h,      v2.8h,       v3.8h
        ext8             v4.16b,     v26.16b,  v27.16b, #2
        ext8             v5.16b,     v26.16b,  v27.16b, #8
        add             v4.8h,      v4.8h,       v5.8h
        ext8             v1.16b,     v26.16b,  v27.16b, #10

        add             v26.8h,  v26.8h,   v1.8h
        ext8             v0.16b,     v16.16b,  v17.16b, #4
        mla             v26.8h,  v2.8h,       v6.h[1]
        ext8             v1.16b,     v16.16b,  v17.16b, #6
        add             v0.8h,      v0.8h,       v1.8h
        ext8             v1.16b,     v16.16b,  v17.16b, #2
        mul             v5.8h,      v4.8h,       v6.h[0]
        uqsub           v26.8h,  v26.8h,   v5.8h
        urshr           v26.8h,  v26.8h,   #5

        ext8             v3.16b,     v16.16b,  v17.16b, #8
        add             v1.8h,      v1.8h,       v3.8h
        ext8             v2.16b,     v16.16b,  v17.16b, #10

        add             v27.8h,  v16.8h,   v2.8h
        mla             v27.8h,  v0.8h,       v6.h[1]
        mul             v5.8h,      v1.8h,       v6.h[0]
        uqsub           v27.8h,  v27.8h,   v5.8h
        mvni            v5.8h,      #0xFC,       lsl #8 
        urshr           v27.8h,  v27.8h,   #5

        umin            v26.8h,  v26.8h,   v5.8h
        umin            v27.8h,  v27.8h,   v5.8h
        urhadd          v26.8h, v26.8h, v28.8h
        urhadd          v27.8h, v27.8h, v29.8h
        st1             {v26.8h},     [x0], x2
        st1             {v27.8h},     [x0], x2
        bne            temp_label_6
        ret


        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16


avg_h264_qpel16_h_lowpass_l2_neon_10
        mov             x13, x30
        mov             x12, #32
        bl              avg_h264_qpel8_h_lowpass_l2_neon_10
        sub             x0,  x0,  x2, lsl #4
        sub             x1,  x1,  x2, lsl #4
        sub             x3,  x3,  x2, lsl #4
        add             x0,  x0,  #16
        add             x1,  x1,  #16
        add             x3,  x3,  #16
        mov             x12, #32
        mov             x30, x13



        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16


avg_h264_qpel8_h_lowpass_l2_neon_10
temp_label_7
      ld1             {v26.8h, v27.8h}, [x1], x2
        ld1             {v16.8h, v17.8h}, [x1], x2
        ld1             {v28.8h},     [x3], x2
        ld1             {v29.8h},     [x3], x2
        subs            x12, x12, #4
        ext8             v2.16b,     v26.16b,  v27.16b, #4
        ext8             v3.16b,     v26.16b,  v27.16b, #6
        add             v2.8h,      v2.8h,       v3.8h
        ext8             v4.16b,     v26.16b,  v27.16b, #2
        ext8             v5.16b,     v26.16b,  v27.16b, #8
        add             v4.8h,      v4.8h,       v5.8h
        ext8             v1.16b,     v26.16b,  v27.16b, #10

        add             v26.8h,  v26.8h,   v1.8h
        ext8             v0.16b,     v16.16b,  v17.16b, #4
        mla             v26.8h,  v2.8h,       v6.h[1]
        ext8             v1.16b,     v16.16b,  v17.16b, #6
        add             v0.8h,      v0.8h,       v1.8h
        ext8             v1.16b,     v16.16b,  v17.16b, #2
        mul             v5.8h,      v4.8h,       v6.h[0]
        uqsub           v26.8h,  v26.8h,   v5.8h
        urshr           v26.8h,  v26.8h,   #5

        ext8             v3.16b,     v16.16b,  v17.16b, #8
        add             v1.8h,      v1.8h,       v3.8h
        ext8             v2.16b,     v16.16b,  v17.16b, #10

        add             v27.8h,  v16.8h,   v2.8h
        mla             v27.8h,  v0.8h,       v6.h[1]
        mul             v5.8h,      v1.8h,       v6.h[0]
        uqsub           v27.8h,  v27.8h,   v5.8h
        mvni            v5.8h,      #0xFC,       lsl #8 
        urshr           v27.8h,  v27.8h,   #5

        umin            v26.8h,  v26.8h,   v5.8h
        umin            v27.8h,  v27.8h,   v5.8h
        urhadd          v26.8h, v26.8h, v28.8h
        urhadd          v27.8h, v27.8h, v29.8h
        ld1             {v2.8h},      [x0], x2
        ld1             {v3.8h},      [x0]
        urhadd          v26.8h, v26.8h, v2.8h
        urhadd          v27.8h, v27.8h, v3.8h
        sub             x0,  x0,  x2
        st1             {v26.8h},     [x0], x2
        st1             {v27.8h},     [x0], x2
        bne            temp_label_7
        ret



        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16


put_h264_qpel16_v_lowpass_neon_packed_10
        mov             x4,  x30
        mov             x2,  #8
        bl              put_h264_qpel8_v_lowpass_neon
        sub             x1,  x1,  x3, lsl #2
        bl              put_h264_qpel8_v_lowpass_neon
        sub             x1,  x1,  x3, lsl #4
        sub             x1,  x1,  x3, lsl #2
        add             x1,  x1,  #8
        bl              put_h264_qpel8_v_lowpass_neon
        sub             x1,  x1,  x3, lsl #2
        mov             x30, x4
        b               put_h264_qpel8_v_lowpass_neon




        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16


put_h264_qpel16_v_lowpass_neon_10
        mov             x4,  x30
        bl              put_h264_qpel8_v_lowpass_neon_10
        sub             x1,  x1,  x3, lsl #2
        bl              put_h264_qpel8_v_lowpass_neon_10
        sub             x0,  x0,  x2, lsl #4
        add             x0,  x0,  #16
        sub             x1,  x1,  x3, lsl #4
        sub             x1,  x1,  x3, lsl #2
        add             x1,  x1,  #16
        bl              put_h264_qpel8_v_lowpass_neon_10
        sub             x1,  x1,  x3, lsl #2
        mov             x30, x4



        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16


put_h264_qpel8_v_lowpass_neon_10
        ld1             {v16.8h}, [x1], x3
        ld1             {v17.8h}, [x1], x3
        ld1             {v18.8h}, [x1], x3
        ld1             {v19.8h}, [x1], x3
        ld1             {v20.8h}, [x1], x3
        ld1             {v21.8h}, [x1], x3
        ld1             {v22.8h}, [x1], x3
        ld1             {v23.8h}, [x1], x3
        ld1             {v24.8h}, [x1], x3
        ld1             {v25.8h}, [x1], x3
        ld1             {v26.8h}, [x1], x3
        ld1             {v27.8h}, [x1], x3
        ld1             {v28.8h}, [x1]

        add             v2.8h,      v18.8h,   v19.8h
        add             v0.8h,      v19.8h,   v20.8h
        add             v4.8h,      v17.8h,   v20.8h
        add             v1.8h,      v18.8h,   v21.8h

        add             v16.8h,  v16.8h,   v21.8h
        add             v17.8h,  v17.8h,   v22.8h
        mla             v16.8h,  v2.8h,       v6.h[1]
        mla             v17.8h,  v0.8h,       v6.h[1]
        mul             v2.8h,      v4.8h,       v6.h[0]
        mul             v0.8h,      v1.8h,       v6.h[0]
        uqsub           v16.8h,  v16.8h,   v2.8h
        uqsub           v17.8h,  v17.8h,   v0.8h

        mvni            v0.8h,      #0xFC,       lsl #8 

        urshr           v16.8h,  v16.8h,   #5
        urshr           v17.8h,  v17.8h,   #5

        umin            v16.8h,  v16.8h,   v0.8h
        umin            v17.8h,  v17.8h,   v0.8h
        add             v2.8h,      v20.8h,   v21.8h
        add             v0.8h,      v21.8h,   v22.8h
        add             v4.8h,      v19.8h,   v22.8h
        add             v1.8h,      v20.8h,   v23.8h

        add             v18.8h,  v18.8h,   v23.8h
        add             v19.8h,  v19.8h,   v24.8h
        mla             v18.8h,  v2.8h,       v6.h[1]
        mla             v19.8h,  v0.8h,       v6.h[1]
        mul             v2.8h,      v4.8h,       v6.h[0]
        mul             v0.8h,      v1.8h,       v6.h[0]
        uqsub           v18.8h,  v18.8h,   v2.8h
        uqsub           v19.8h,  v19.8h,   v0.8h

        mvni            v0.8h,      #0xFC,       lsl #8 

        urshr           v18.8h,  v18.8h,   #5
        urshr           v19.8h,  v19.8h,   #5

        umin            v18.8h,  v18.8h,   v0.8h
        umin            v19.8h,  v19.8h,   v0.8h
        add             v2.8h,      v22.8h,   v23.8h
        add             v0.8h,      v23.8h,   v24.8h
        add             v4.8h,      v21.8h,   v24.8h
        add             v1.8h,      v22.8h,   v25.8h

        add             v20.8h,  v20.8h,   v25.8h
        add             v21.8h,  v21.8h,   v26.8h
        mla             v20.8h,  v2.8h,       v6.h[1]
        mla             v21.8h,  v0.8h,       v6.h[1]
        mul             v2.8h,      v4.8h,       v6.h[0]
        mul             v0.8h,      v1.8h,       v6.h[0]
        uqsub           v20.8h,  v20.8h,   v2.8h
        uqsub           v21.8h,  v21.8h,   v0.8h

        mvni            v0.8h,      #0xFC,       lsl #8 

        urshr           v20.8h,  v20.8h,   #5
        urshr           v21.8h,  v21.8h,   #5

        umin            v20.8h,  v20.8h,   v0.8h
        umin            v21.8h,  v21.8h,   v0.8h
        add             v2.8h,      v24.8h,   v25.8h
        add             v0.8h,      v25.8h,   v26.8h
        add             v4.8h,      v23.8h,   v26.8h
        add             v1.8h,      v24.8h,   v27.8h

        add             v22.8h,  v22.8h,   v27.8h
        add             v23.8h,  v23.8h,   v28.8h
        mla             v22.8h,  v2.8h,       v6.h[1]
        mla             v23.8h,  v0.8h,       v6.h[1]
        mul             v2.8h,      v4.8h,       v6.h[0]
        mul             v0.8h,      v1.8h,       v6.h[0]
        uqsub           v22.8h,  v22.8h,   v2.8h
        uqsub           v23.8h,  v23.8h,   v0.8h

        mvni            v0.8h,      #0xFC,       lsl #8 

        urshr           v22.8h,  v22.8h,   #5
        urshr           v23.8h,  v23.8h,   #5

        umin            v22.8h,  v22.8h,   v0.8h
        umin            v23.8h,  v23.8h,   v0.8h


        st1             {v16.8h}, [x0], x2
        st1             {v17.8h}, [x0], x2
        st1             {v18.8h}, [x0], x2
        st1             {v19.8h}, [x0], x2
        st1             {v20.8h}, [x0], x2
        st1             {v21.8h}, [x0], x2
        st1             {v22.8h}, [x0], x2
        st1             {v23.8h}, [x0], x2

        ret


        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16


avg_h264_qpel16_v_lowpass_neon_10
        mov             x4,  x30
        bl              avg_h264_qpel8_v_lowpass_neon_10
        sub             x1,  x1,  x3, lsl #2
        bl              avg_h264_qpel8_v_lowpass_neon_10
        sub             x0,  x0,  x2, lsl #4
        add             x0,  x0,  #16
        sub             x1,  x1,  x3, lsl #4
        sub             x1,  x1,  x3, lsl #2
        add             x1,  x1,  #16
        bl              avg_h264_qpel8_v_lowpass_neon_10
        sub             x1,  x1,  x3, lsl #2
        mov             x30, x4



        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16


avg_h264_qpel8_v_lowpass_neon_10
        ld1             {v16.8h}, [x1], x3
        ld1             {v17.8h}, [x1], x3
        ld1             {v18.8h}, [x1], x3
        ld1             {v19.8h}, [x1], x3
        ld1             {v20.8h}, [x1], x3
        ld1             {v21.8h}, [x1], x3
        ld1             {v22.8h}, [x1], x3
        ld1             {v23.8h}, [x1], x3
        ld1             {v24.8h}, [x1], x3
        ld1             {v25.8h}, [x1], x3
        ld1             {v26.8h}, [x1], x3
        ld1             {v27.8h}, [x1], x3
        ld1             {v28.8h}, [x1]

        add             v2.8h,      v18.8h,   v19.8h
        add             v0.8h,      v19.8h,   v20.8h
        add             v4.8h,      v17.8h,   v20.8h
        add             v1.8h,      v18.8h,   v21.8h

        add             v16.8h,  v16.8h,   v21.8h
        add             v17.8h,  v17.8h,   v22.8h
        mla             v16.8h,  v2.8h,       v6.h[1]
        mla             v17.8h,  v0.8h,       v6.h[1]
        mul             v2.8h,      v4.8h,       v6.h[0]
        mul             v0.8h,      v1.8h,       v6.h[0]
        uqsub           v16.8h,  v16.8h,   v2.8h
        uqsub           v17.8h,  v17.8h,   v0.8h

        mvni            v0.8h,      #0xFC,       lsl #8 

        urshr           v16.8h,  v16.8h,   #5
        urshr           v17.8h,  v17.8h,   #5

        umin            v16.8h,  v16.8h,   v0.8h
        umin            v17.8h,  v17.8h,   v0.8h
        add             v2.8h,      v20.8h,   v21.8h
        add             v0.8h,      v21.8h,   v22.8h
        add             v4.8h,      v19.8h,   v22.8h
        add             v1.8h,      v20.8h,   v23.8h

        add             v18.8h,  v18.8h,   v23.8h
        add             v19.8h,  v19.8h,   v24.8h
        mla             v18.8h,  v2.8h,       v6.h[1]
        mla             v19.8h,  v0.8h,       v6.h[1]
        mul             v2.8h,      v4.8h,       v6.h[0]
        mul             v0.8h,      v1.8h,       v6.h[0]
        uqsub           v18.8h,  v18.8h,   v2.8h
        uqsub           v19.8h,  v19.8h,   v0.8h

        mvni            v0.8h,      #0xFC,       lsl #8 

        urshr           v18.8h,  v18.8h,   #5
        urshr           v19.8h,  v19.8h,   #5

        umin            v18.8h,  v18.8h,   v0.8h
        umin            v19.8h,  v19.8h,   v0.8h
        add             v2.8h,      v22.8h,   v23.8h
        add             v0.8h,      v23.8h,   v24.8h
        add             v4.8h,      v21.8h,   v24.8h
        add             v1.8h,      v22.8h,   v25.8h

        add             v20.8h,  v20.8h,   v25.8h
        add             v21.8h,  v21.8h,   v26.8h
        mla             v20.8h,  v2.8h,       v6.h[1]
        mla             v21.8h,  v0.8h,       v6.h[1]
        mul             v2.8h,      v4.8h,       v6.h[0]
        mul             v0.8h,      v1.8h,       v6.h[0]
        uqsub           v20.8h,  v20.8h,   v2.8h
        uqsub           v21.8h,  v21.8h,   v0.8h

        mvni            v0.8h,      #0xFC,       lsl #8 

        urshr           v20.8h,  v20.8h,   #5
        urshr           v21.8h,  v21.8h,   #5

        umin            v20.8h,  v20.8h,   v0.8h
        umin            v21.8h,  v21.8h,   v0.8h
        add             v2.8h,      v24.8h,   v25.8h
        add             v0.8h,      v25.8h,   v26.8h
        add             v4.8h,      v23.8h,   v26.8h
        add             v1.8h,      v24.8h,   v27.8h

        add             v22.8h,  v22.8h,   v27.8h
        add             v23.8h,  v23.8h,   v28.8h
        mla             v22.8h,  v2.8h,       v6.h[1]
        mla             v23.8h,  v0.8h,       v6.h[1]
        mul             v2.8h,      v4.8h,       v6.h[0]
        mul             v0.8h,      v1.8h,       v6.h[0]
        uqsub           v22.8h,  v22.8h,   v2.8h
        uqsub           v23.8h,  v23.8h,   v0.8h

        mvni            v0.8h,      #0xFC,       lsl #8 

        urshr           v22.8h,  v22.8h,   #5
        urshr           v23.8h,  v23.8h,   #5

        umin            v22.8h,  v22.8h,   v0.8h
        umin            v23.8h,  v23.8h,   v0.8h

        ld1             {v24.8h},  [x0], x2
        ld1             {v25.8h}, [x0], x2
        ld1             {v26.8h}, [x0], x2
        urhadd          v16.8h, v16.8h, v24.8h
        ld1             {v27.8h}, [x0], x2
        urhadd          v17.8h, v17.8h, v25.8h
        ld1             {v28.8h}, [x0], x2
        urhadd          v18.8h, v18.8h, v26.8h
        ld1             {v29.8h}, [x0], x2
        urhadd          v19.8h, v19.8h, v27.8h
        ld1             {v30.8h}, [x0], x2
        urhadd          v20.8h, v20.8h, v28.8h
        ld1             {v31.8h}, [x0], x2
        urhadd          v21.8h, v21.8h, v29.8h
        urhadd          v22.8h, v22.8h, v30.8h
        urhadd          v23.8h, v23.8h, v31.8h
        sub             x0,  x0,  x2,  lsl #3

        st1             {v16.8h}, [x0], x2
        st1             {v17.8h}, [x0], x2
        st1             {v18.8h}, [x0], x2
        st1             {v19.8h}, [x0], x2
        st1             {v20.8h}, [x0], x2
        st1             {v21.8h}, [x0], x2
        st1             {v22.8h}, [x0], x2
        st1             {v23.8h}, [x0], x2

        ret




        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16


put_h264_qpel16_v_lowpass_l2_neon_10
        mov             x4,  x30
        bl              put_h264_qpel8_v_lowpass_l2_neon_10
        sub             x1,  x1,  x3, lsl #2
        bl              put_h264_qpel8_v_lowpass_l2_neon_10
        sub             x0,  x0,  x3, lsl #4
        sub             x12, x12, x2, lsl #4
        add             x0,  x0,  #16
        add             x12, x12, #16
        sub             x1,  x1,  x3, lsl #4
        sub             x1,  x1,  x3, lsl #2
        add             x1,  x1,  #16
        bl              put_h264_qpel8_v_lowpass_l2_neon_10
        sub             x1,  x1,  x3, lsl #2
        mov             x30, x4



        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16


put_h264_qpel8_v_lowpass_l2_neon_10
        ld1             {v16.8h}, [x1], x3
        ld1             {v17.8h}, [x1], x3
        ld1             {v18.8h}, [x1], x3
        ld1             {v19.8h}, [x1], x3
        ld1             {v20.8h}, [x1], x3
        ld1             {v21.8h}, [x1], x3
        ld1             {v22.8h}, [x1], x3
        ld1             {v23.8h}, [x1], x3
        ld1             {v24.8h}, [x1], x3
        ld1             {v25.8h}, [x1], x3
        ld1             {v26.8h}, [x1], x3
        ld1             {v27.8h}, [x1], x3
        ld1             {v28.8h}, [x1]

        add             v2.8h,      v18.8h,   v19.8h
        add             v0.8h,      v19.8h,   v20.8h
        add             v4.8h,      v17.8h,   v20.8h
        add             v1.8h,      v18.8h,   v21.8h

        add             v16.8h,  v16.8h,   v21.8h
        add             v17.8h,  v17.8h,   v22.8h
        mla             v16.8h,  v2.8h,       v6.h[1]
        mla             v17.8h,  v0.8h,       v6.h[1]
        mul             v2.8h,      v4.8h,       v6.h[0]
        mul             v0.8h,      v1.8h,       v6.h[0]
        uqsub           v16.8h,  v16.8h,   v2.8h
        uqsub           v17.8h,  v17.8h,   v0.8h

        mvni            v0.8h,      #0xFC,       lsl #8 

        urshr           v16.8h,  v16.8h,   #5
        urshr           v17.8h,  v17.8h,   #5

        umin            v16.8h,  v16.8h,   v0.8h
        umin            v17.8h,  v17.8h,   v0.8h
        add             v2.8h,      v20.8h,   v21.8h
        add             v0.8h,      v21.8h,   v22.8h
        add             v4.8h,      v19.8h,   v22.8h
        add             v1.8h,      v20.8h,   v23.8h

        add             v18.8h,  v18.8h,   v23.8h
        add             v19.8h,  v19.8h,   v24.8h
        mla             v18.8h,  v2.8h,       v6.h[1]
        mla             v19.8h,  v0.8h,       v6.h[1]
        mul             v2.8h,      v4.8h,       v6.h[0]
        mul             v0.8h,      v1.8h,       v6.h[0]
        uqsub           v18.8h,  v18.8h,   v2.8h
        uqsub           v19.8h,  v19.8h,   v0.8h

        mvni            v0.8h,      #0xFC,       lsl #8 

        urshr           v18.8h,  v18.8h,   #5
        urshr           v19.8h,  v19.8h,   #5

        umin            v18.8h,  v18.8h,   v0.8h
        umin            v19.8h,  v19.8h,   v0.8h
        add             v2.8h,      v22.8h,   v23.8h
        add             v0.8h,      v23.8h,   v24.8h
        add             v4.8h,      v21.8h,   v24.8h
        add             v1.8h,      v22.8h,   v25.8h

        add             v20.8h,  v20.8h,   v25.8h
        add             v21.8h,  v21.8h,   v26.8h
        mla             v20.8h,  v2.8h,       v6.h[1]
        mla             v21.8h,  v0.8h,       v6.h[1]
        mul             v2.8h,      v4.8h,       v6.h[0]
        mul             v0.8h,      v1.8h,       v6.h[0]
        uqsub           v20.8h,  v20.8h,   v2.8h
        uqsub           v21.8h,  v21.8h,   v0.8h

        mvni            v0.8h,      #0xFC,       lsl #8 

        urshr           v20.8h,  v20.8h,   #5
        urshr           v21.8h,  v21.8h,   #5

        umin            v20.8h,  v20.8h,   v0.8h
        umin            v21.8h,  v21.8h,   v0.8h
        add             v2.8h,      v24.8h,   v25.8h
        add             v0.8h,      v25.8h,   v26.8h
        add             v4.8h,      v23.8h,   v26.8h
        add             v1.8h,      v24.8h,   v27.8h

        add             v22.8h,  v22.8h,   v27.8h
        add             v23.8h,  v23.8h,   v28.8h
        mla             v22.8h,  v2.8h,       v6.h[1]
        mla             v23.8h,  v0.8h,       v6.h[1]
        mul             v2.8h,      v4.8h,       v6.h[0]
        mul             v0.8h,      v1.8h,       v6.h[0]
        uqsub           v22.8h,  v22.8h,   v2.8h
        uqsub           v23.8h,  v23.8h,   v0.8h

        mvni            v0.8h,      #0xFC,       lsl #8 

        urshr           v22.8h,  v22.8h,   #5
        urshr           v23.8h,  v23.8h,   #5

        umin            v22.8h,  v22.8h,   v0.8h
        umin            v23.8h,  v23.8h,   v0.8h

        ld1             {v24.8h},  [x12], x2
        ld1             {v25.8h},  [x12], x2
        ld1             {v26.8h},  [x12], x2
        ld1             {v27.8h},  [x12], x2
        ld1             {v28.8h},  [x12], x2
        urhadd          v16.8h, v24.8h, v16.8h
        urhadd          v17.8h, v25.8h, v17.8h
        ld1             {v29.8h},  [x12], x2
        urhadd          v18.8h, v26.8h, v18.8h
        urhadd          v19.8h, v27.8h, v19.8h
        ld1             {v30.8h}, [x12], x2
        urhadd          v20.8h, v28.8h, v20.8h
        urhadd          v21.8h, v29.8h, v21.8h
        ld1             {v31.8h}, [x12], x2
        urhadd          v22.8h, v30.8h, v22.8h
        urhadd          v23.8h, v31.8h, v23.8h


        st1             {v16.8h}, [x0], x3
        st1             {v17.8h}, [x0], x3
        st1             {v18.8h}, [x0], x3
        st1             {v19.8h}, [x0], x3
        st1             {v20.8h}, [x0], x3
        st1             {v21.8h}, [x0], x3
        st1             {v22.8h}, [x0], x3
        st1             {v23.8h}, [x0], x3

        ret


        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16


avg_h264_qpel16_v_lowpass_l2_neon_10
        mov             x4,  x30
        bl              avg_h264_qpel8_v_lowpass_l2_neon_10
        sub             x1,  x1,  x3, lsl #2
        bl              avg_h264_qpel8_v_lowpass_l2_neon_10
        sub             x0,  x0,  x3, lsl #4
        sub             x12, x12, x2, lsl #4
        add             x0,  x0,  #16
        add             x12, x12, #16
        sub             x1,  x1,  x3, lsl #4
        sub             x1,  x1,  x3, lsl #2
        add             x1,  x1,  #16
        bl              avg_h264_qpel8_v_lowpass_l2_neon_10
        sub             x1,  x1,  x3, lsl #2
        mov             x30, x4



        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16


avg_h264_qpel8_v_lowpass_l2_neon_10
        ld1             {v16.8h}, [x1], x3
        ld1             {v17.8h}, [x1], x3
        ld1             {v18.8h}, [x1], x3
        ld1             {v19.8h}, [x1], x3
        ld1             {v20.8h}, [x1], x3
        ld1             {v21.8h}, [x1], x3
        ld1             {v22.8h}, [x1], x3
        ld1             {v23.8h}, [x1], x3
        ld1             {v24.8h}, [x1], x3
        ld1             {v25.8h}, [x1], x3
        ld1             {v26.8h}, [x1], x3
        ld1             {v27.8h}, [x1], x3
        ld1             {v28.8h}, [x1]

        add             v2.8h,      v18.8h,   v19.8h
        add             v0.8h,      v19.8h,   v20.8h
        add             v4.8h,      v17.8h,   v20.8h
        add             v1.8h,      v18.8h,   v21.8h

        add             v16.8h,  v16.8h,   v21.8h
        add             v17.8h,  v17.8h,   v22.8h
        mla             v16.8h,  v2.8h,       v6.h[1]
        mla             v17.8h,  v0.8h,       v6.h[1]
        mul             v2.8h,      v4.8h,       v6.h[0]
        mul             v0.8h,      v1.8h,       v6.h[0]
        uqsub           v16.8h,  v16.8h,   v2.8h
        uqsub           v17.8h,  v17.8h,   v0.8h

        mvni            v0.8h,      #0xFC,       lsl #8 

        urshr           v16.8h,  v16.8h,   #5
        urshr           v17.8h,  v17.8h,   #5

        umin            v16.8h,  v16.8h,   v0.8h
        umin            v17.8h,  v17.8h,   v0.8h
        add             v2.8h,      v20.8h,   v21.8h
        add             v0.8h,      v21.8h,   v22.8h
        add             v4.8h,      v19.8h,   v22.8h
        add             v1.8h,      v20.8h,   v23.8h

        add             v18.8h,  v18.8h,   v23.8h
        add             v19.8h,  v19.8h,   v24.8h
        mla             v18.8h,  v2.8h,       v6.h[1]
        mla             v19.8h,  v0.8h,       v6.h[1]
        mul             v2.8h,      v4.8h,       v6.h[0]
        mul             v0.8h,      v1.8h,       v6.h[0]
        uqsub           v18.8h,  v18.8h,   v2.8h
        uqsub           v19.8h,  v19.8h,   v0.8h

        mvni            v0.8h,      #0xFC,       lsl #8 

        urshr           v18.8h,  v18.8h,   #5
        urshr           v19.8h,  v19.8h,   #5

        umin            v18.8h,  v18.8h,   v0.8h
        umin            v19.8h,  v19.8h,   v0.8h
        add             v2.8h,      v22.8h,   v23.8h
        add             v0.8h,      v23.8h,   v24.8h
        add             v4.8h,      v21.8h,   v24.8h
        add             v1.8h,      v22.8h,   v25.8h

        add             v20.8h,  v20.8h,   v25.8h
        add             v21.8h,  v21.8h,   v26.8h
        mla             v20.8h,  v2.8h,       v6.h[1]
        mla             v21.8h,  v0.8h,       v6.h[1]
        mul             v2.8h,      v4.8h,       v6.h[0]
        mul             v0.8h,      v1.8h,       v6.h[0]
        uqsub           v20.8h,  v20.8h,   v2.8h
        uqsub           v21.8h,  v21.8h,   v0.8h

        mvni            v0.8h,      #0xFC,       lsl #8 

        urshr           v20.8h,  v20.8h,   #5
        urshr           v21.8h,  v21.8h,   #5

        umin            v20.8h,  v20.8h,   v0.8h
        umin            v21.8h,  v21.8h,   v0.8h
        add             v2.8h,      v24.8h,   v25.8h
        add             v0.8h,      v25.8h,   v26.8h
        add             v4.8h,      v23.8h,   v26.8h
        add             v1.8h,      v24.8h,   v27.8h

        add             v22.8h,  v22.8h,   v27.8h
        add             v23.8h,  v23.8h,   v28.8h
        mla             v22.8h,  v2.8h,       v6.h[1]
        mla             v23.8h,  v0.8h,       v6.h[1]
        mul             v2.8h,      v4.8h,       v6.h[0]
        mul             v0.8h,      v1.8h,       v6.h[0]
        uqsub           v22.8h,  v22.8h,   v2.8h
        uqsub           v23.8h,  v23.8h,   v0.8h

        mvni            v0.8h,      #0xFC,       lsl #8 

        urshr           v22.8h,  v22.8h,   #5
        urshr           v23.8h,  v23.8h,   #5

        umin            v22.8h,  v22.8h,   v0.8h
        umin            v23.8h,  v23.8h,   v0.8h

        ld1             {v24.8h},  [x12], x2
        ld1             {v25.8h},  [x12], x2
        ld1             {v26.8h},  [x12], x2
        ld1             {v27.8h},  [x12], x2
        ld1             {v28.8h},  [x12], x2
        urhadd          v16.8h, v24.8h, v16.8h
        urhadd          v17.8h, v25.8h, v17.8h
        ld1             {v29.8h},  [x12], x2
        urhadd          v18.8h, v26.8h, v18.8h
        urhadd          v19.8h, v27.8h, v19.8h
        ld1             {v30.8h}, [x12], x2
        urhadd          v20.8h, v28.8h, v20.8h
        urhadd          v21.8h, v29.8h, v21.8h
        ld1             {v31.8h}, [x12], x2
        urhadd          v22.8h, v30.8h, v22.8h
        urhadd          v23.8h, v31.8h, v23.8h

        ld1             {v24.8h}, [x0], x3
        ld1             {v25.8h}, [x0], x3
        ld1             {v26.8h}, [x0], x3
        urhadd          v16.8h, v16.8h, v24.8h
        ld1             {v27.8h}, [x0], x3
        urhadd          v17.8h, v17.8h, v25.8h
        ld1             {v28.8h}, [x0], x3
        urhadd          v18.8h, v18.8h, v26.8h
        ld1             {v29.8h}, [x0], x3
        urhadd          v19.8h, v19.8h, v27.8h
        ld1             {v30.8h}, [x0], x3
        urhadd          v20.8h, v20.8h, v28.8h
        ld1             {v31.8h}, [x0], x3
        urhadd          v21.8h, v21.8h, v29.8h
        urhadd          v22.8h, v22.8h, v30.8h
        urhadd          v23.8h, v23.8h, v31.8h
        sub             x0,  x0,  x3,  lsl #3

        st1             {v16.8h}, [x0], x3
        st1             {v17.8h}, [x0], x3
        st1             {v18.8h}, [x0], x3
        st1             {v19.8h}, [x0], x3
        st1             {v20.8h}, [x0], x3
        st1             {v21.8h}, [x0], x3
        st1             {v22.8h}, [x0], x3
        st1             {v23.8h}, [x0], x3

        ret




        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16

        EXPORT ff_put_h264_qpel8_mc10_neon_10





ff_put_h264_qpel8_mc10_neon_10
        










        movz            w3, #20, lsl #16
        movk            w3, #5
        mov             v6.s[0], w3
        mov             x3,  x1
        sub             x1,  x1,  #4
        mov             x12, #16
        b               put_h264_qpel8_h_lowpass_l2_neon_10



        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16

        EXPORT ff_put_h264_qpel8_mc20_neon_10





ff_put_h264_qpel8_mc20_neon_10
        










        movz            w3, #20, lsl #16
        movk            w3, #5
        mov             v6.s[0], w3
        sub             x1,  x1,  #4
        mov             x3,  x2
        mov             x12, #16
        b               put_h264_qpel8_h_lowpass_neon_10



        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16

        EXPORT ff_put_h264_qpel8_mc30_neon_10





ff_put_h264_qpel8_mc30_neon_10
        










        movz            w3, #20, lsl #16
        movk            w3, #5
        mov             v6.s[0], w3
        add             x3,  x1,  #2
        sub             x1,  x1,  #4
        mov             x12, #16
        b               put_h264_qpel8_h_lowpass_l2_neon_10



        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16

        EXPORT ff_put_h264_qpel8_mc01_neon_10





ff_put_h264_qpel8_mc01_neon_10
        










        mov             x14, x30
        mov             x12, x1
put_h264_qpel8_mc01_10
        movz            w3, #20, lsl #16
        movk            w3, #5
        mov             v6.s[0], w3
        mov             x3,  x2
        sub             x1,  x1,  x2, lsl #1
        bl              put_h264_qpel8_v_lowpass_l2_neon_10
        ret             x14



        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16

        EXPORT ff_put_h264_qpel8_mc11_neon_10





ff_put_h264_qpel8_mc11_neon_10
        










        mov             x14, x30
        mov             x8,  x0
        mov             x9,  x1
put_h264_qpel8_mc11_10
        movz            w3, #20, lsl #16
        movk            w3, #5
        mov             v6.s[0], w3
        mov             x11, sp
        sub             sp,  sp,  #128
        mov             x0,  sp
        sub             x1,  x1,  #4
        mov             x3,  #16
        mov             x12, #16
        bl              put_h264_qpel8_h_lowpass_neon_10
        mov             x0,  x8
        mov             x3,  x2
        mov             x12, sp
        sub             x1,  x9,  x2, lsl #1
        mov             x2,  #16
        bl              put_h264_qpel8_v_lowpass_l2_neon_10
        mov             sp,  x11
        ret             x14



        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16

        EXPORT ff_put_h264_qpel8_mc31_neon_10





ff_put_h264_qpel8_mc31_neon_10
        










        add             x1,  x1,  #2
        mov             x14, x30
        mov             x8,  x0
        mov             x9,  x1
        sub             x1,  x1,  #2
        b               put_h264_qpel8_mc11_10



        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16

        EXPORT ff_put_h264_qpel8_mc02_neon_10





ff_put_h264_qpel8_mc02_neon_10
        










        mov             x14, x30
        movz            w3, #20, lsl #16
        movk            w3, #5
        mov             v6.s[0], w3
        sub             x1,  x1,  x2, lsl #1
        mov             x3,  x2
        bl              put_h264_qpel8_v_lowpass_neon_10
        ret             x14



        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16

        EXPORT ff_put_h264_qpel8_mc03_neon_10





ff_put_h264_qpel8_mc03_neon_10
        










        mov             x14, x30
        add             x12, x1,  x2
        b               put_h264_qpel8_mc01_10



        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16

        EXPORT ff_put_h264_qpel8_mc13_neon_10





ff_put_h264_qpel8_mc13_neon_10
        










        mov             x14, x30
        mov             x8,  x0
        mov             x9,  x1
        add             x1,  x1,  x2
        b               put_h264_qpel8_mc11_10



        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16

        EXPORT ff_put_h264_qpel8_mc33_neon_10





ff_put_h264_qpel8_mc33_neon_10
        










        add             x1,  x1,  #2
        mov             x14, x30
        mov             x8,  x0
        mov             x9,  x1
        add             x1,  x1,  x2
        sub             x1,  x1,  #2
        b               put_h264_qpel8_mc11_10


        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16

        EXPORT ff_avg_h264_qpel8_mc10_neon_10





ff_avg_h264_qpel8_mc10_neon_10
        










        movz            w3, #20, lsl #16
        movk            w3, #5
        mov             v6.s[0], w3
        mov             x3,  x1
        sub             x1,  x1,  #4
        mov             x12, #16
        b               avg_h264_qpel8_h_lowpass_l2_neon_10



        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16

        EXPORT ff_avg_h264_qpel8_mc20_neon_10





ff_avg_h264_qpel8_mc20_neon_10
        










        movz            w3, #20, lsl #16
        movk            w3, #5
        mov             v6.s[0], w3
        sub             x1,  x1,  #4
        mov             x3,  x2
        mov             x12, #16
        b               avg_h264_qpel8_h_lowpass_neon_10



        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16

        EXPORT ff_avg_h264_qpel8_mc30_neon_10





ff_avg_h264_qpel8_mc30_neon_10
        










        movz            w3, #20, lsl #16
        movk            w3, #5
        mov             v6.s[0], w3
        add             x3,  x1,  #2
        sub             x1,  x1,  #4
        mov             x12, #16
        b               avg_h264_qpel8_h_lowpass_l2_neon_10



        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16

        EXPORT ff_avg_h264_qpel8_mc01_neon_10





ff_avg_h264_qpel8_mc01_neon_10
        










        mov             x14, x30
        mov             x12, x1
avg_h264_qpel8_mc01_10
        movz            w3, #20, lsl #16
        movk            w3, #5
        mov             v6.s[0], w3
        mov             x3,  x2
        sub             x1,  x1,  x2, lsl #1
        bl              avg_h264_qpel8_v_lowpass_l2_neon_10
        ret             x14



        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16

        EXPORT ff_avg_h264_qpel8_mc11_neon_10





ff_avg_h264_qpel8_mc11_neon_10
        










        mov             x14, x30
        mov             x8,  x0
        mov             x9,  x1
avg_h264_qpel8_mc11_10
        movz            w3, #20, lsl #16
        movk            w3, #5
        mov             v6.s[0], w3
        mov             x11, sp
        sub             sp,  sp,  #128
        mov             x0,  sp
        sub             x1,  x1,  #4
        mov             x3,  #16
        mov             x12, #16
        bl              put_h264_qpel8_h_lowpass_neon_10
        mov             x0,  x8
        mov             x3,  x2
        mov             x12, sp
        sub             x1,  x9,  x2, lsl #1
        mov             x2,  #16
        bl              avg_h264_qpel8_v_lowpass_l2_neon_10
        mov             sp,  x11
        ret             x14



        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16

        EXPORT ff_avg_h264_qpel8_mc31_neon_10





ff_avg_h264_qpel8_mc31_neon_10
        










        add             x1,  x1,  #2
        mov             x14, x30
        mov             x8,  x0
        mov             x9,  x1
        sub             x1,  x1,  #2
        b               avg_h264_qpel8_mc11_10



        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16

        EXPORT ff_avg_h264_qpel8_mc02_neon_10





ff_avg_h264_qpel8_mc02_neon_10
        










        mov             x14, x30
        movz            w3, #20, lsl #16
        movk            w3, #5
        mov             v6.s[0], w3
        sub             x1,  x1,  x2, lsl #1
        mov             x3,  x2
        bl              avg_h264_qpel8_v_lowpass_neon_10
        ret             x14



        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16

        EXPORT ff_avg_h264_qpel8_mc03_neon_10





ff_avg_h264_qpel8_mc03_neon_10
        










        mov             x14, x30
        add             x12, x1,  x2
        b               avg_h264_qpel8_mc01_10



        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16

        EXPORT ff_avg_h264_qpel8_mc13_neon_10





ff_avg_h264_qpel8_mc13_neon_10
        










        mov             x14, x30
        mov             x8,  x0
        mov             x9,  x1
        add             x1,  x1,  x2
        b               avg_h264_qpel8_mc11_10



        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16

        EXPORT ff_avg_h264_qpel8_mc33_neon_10





ff_avg_h264_qpel8_mc33_neon_10
        










        add             x1,  x1,  #2
        mov             x14, x30
        mov             x8,  x0
        mov             x9,  x1
        add             x1,  x1,  x2
        sub             x1,  x1,  #2
        b               avg_h264_qpel8_mc11_10




        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16

        EXPORT ff_put_h264_qpel16_mc10_neon_10





ff_put_h264_qpel16_mc10_neon_10
        










        movz            w3, #20, lsl #16
        movk            w3, #5
        mov             v6.s[0], w3
        mov             x3,  x1
        sub             x1,  x1,  #4
        b               put_h264_qpel16_h_lowpass_l2_neon_10



        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16

        EXPORT ff_put_h264_qpel16_mc20_neon_10





ff_put_h264_qpel16_mc20_neon_10
        










        movz            w3, #20, lsl #16
        movk            w3, #5
        mov             v6.s[0], w3
        sub             x1,  x1,  #4
        mov             x3,  x2
        b               put_h264_qpel16_h_lowpass_neon_10



        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16

        EXPORT ff_put_h264_qpel16_mc30_neon_10





ff_put_h264_qpel16_mc30_neon_10
        










        movz            w3, #20, lsl #16
        movk            w3, #5
        mov             v6.s[0], w3
        add             x3,  x1,  #2
        sub             x1,  x1,  #4
        b               put_h264_qpel16_h_lowpass_l2_neon_10



        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16

        EXPORT ff_put_h264_qpel16_mc01_neon_10





ff_put_h264_qpel16_mc01_neon_10
        










        mov             x14, x30
        mov             x12, x1
put_h264_qpel16_mc01_10
        movz            w3, #20, lsl #16
        movk            w3, #5
        mov             v6.s[0], w3
        mov             x3,  x2
        sub             x1,  x1,  x2, lsl #1
        bl              put_h264_qpel16_v_lowpass_l2_neon_10
        ret             x14



        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16

        EXPORT ff_put_h264_qpel16_mc11_neon_10





ff_put_h264_qpel16_mc11_neon_10
        










        mov             x14, x30
        mov             x8,  x0
        mov             x9,  x1
put_h264_qpel16_mc11_10
        movz            w3, #20, lsl #16
        movk            w3, #5
        mov             v6.s[0], w3
        mov             x11, sp
        sub             sp,  sp,  #512
        mov             x0,  sp
        sub             x1,  x1,  #4
        mov             x3,  #32
        bl              put_h264_qpel16_h_lowpass_neon_10
        mov             x0,  x8
        mov             x3,  x2
        mov             x12, sp
        sub             x1,  x9,  x2, lsl #1
        mov             x2,  #32
        bl              put_h264_qpel16_v_lowpass_l2_neon_10
        mov             sp,  x11
        ret             x14



        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16

        EXPORT ff_put_h264_qpel16_mc31_neon_10





ff_put_h264_qpel16_mc31_neon_10
        










        add             x1,  x1,  #2
        mov             x14, x30
        mov             x8,  x0
        mov             x9,  x1
        sub             x1,  x1,  #2
        b               put_h264_qpel16_mc11_10



        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16

        EXPORT ff_put_h264_qpel16_mc02_neon_10





ff_put_h264_qpel16_mc02_neon_10
        










        mov             x14, x30
        movz            w3, #20, lsl #16
        movk            w3, #5
        mov             v6.s[0], w3
        sub             x1,  x1,  x2, lsl #1
        mov             x3,  x2
        bl              put_h264_qpel16_v_lowpass_neon_10
        ret             x14



        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16

        EXPORT ff_put_h264_qpel16_mc03_neon_10





ff_put_h264_qpel16_mc03_neon_10
        










        mov             x14, x30
        add             x12, x1,  x2
        b               put_h264_qpel16_mc01_10



        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16

        EXPORT ff_put_h264_qpel16_mc13_neon_10





ff_put_h264_qpel16_mc13_neon_10
        










        mov             x14, x30
        mov             x8,  x0
        mov             x9,  x1
        add             x1,  x1,  x2
        b               put_h264_qpel16_mc11_10



        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16

        EXPORT ff_put_h264_qpel16_mc33_neon_10





ff_put_h264_qpel16_mc33_neon_10
        










        add             x1,  x1,  #2
        mov             x14, x30
        mov             x8,  x0
        mov             x9,  x1
        add             x1,  x1,  x2
        sub             x1,  x1,  #2
        b               put_h264_qpel16_mc11_10


        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16

        EXPORT ff_avg_h264_qpel16_mc10_neon_10





ff_avg_h264_qpel16_mc10_neon_10
        










        movz            w3, #20, lsl #16
        movk            w3, #5
        mov             v6.s[0], w3
        mov             x3,  x1
        sub             x1,  x1,  #4
        b               avg_h264_qpel16_h_lowpass_l2_neon_10



        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16

        EXPORT ff_avg_h264_qpel16_mc20_neon_10





ff_avg_h264_qpel16_mc20_neon_10
        










        movz            w3, #20, lsl #16
        movk            w3, #5
        mov             v6.s[0], w3
        sub             x1,  x1,  #4
        mov             x3,  x2
        b               avg_h264_qpel16_h_lowpass_neon_10



        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16

        EXPORT ff_avg_h264_qpel16_mc30_neon_10





ff_avg_h264_qpel16_mc30_neon_10
        










        movz            w3, #20, lsl #16
        movk            w3, #5
        mov             v6.s[0], w3
        add             x3,  x1,  #2
        sub             x1,  x1,  #4
        b               avg_h264_qpel16_h_lowpass_l2_neon_10



        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16

        EXPORT ff_avg_h264_qpel16_mc01_neon_10





ff_avg_h264_qpel16_mc01_neon_10
        










        mov             x14, x30
        mov             x12, x1
avg_h264_qpel16_mc01_10
        movz            w3, #20, lsl #16
        movk            w3, #5
        mov             v6.s[0], w3
        mov             x3,  x2
        sub             x1,  x1,  x2, lsl #1
        bl              avg_h264_qpel16_v_lowpass_l2_neon_10
        ret             x14



        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16

        EXPORT ff_avg_h264_qpel16_mc11_neon_10





ff_avg_h264_qpel16_mc11_neon_10
        










        mov             x14, x30
        mov             x8,  x0
        mov             x9,  x1
avg_h264_qpel16_mc11_10
        movz            w3, #20, lsl #16
        movk            w3, #5
        mov             v6.s[0], w3
        mov             x11, sp
        sub             sp,  sp,  #512
        mov             x0,  sp
        sub             x1,  x1,  #4
        mov             x3,  #32
        bl              put_h264_qpel16_h_lowpass_neon_10
        mov             x0,  x8
        mov             x3,  x2
        mov             x12, sp
        sub             x1,  x9,  x2, lsl #1
        mov             x2,  #32
        bl              avg_h264_qpel16_v_lowpass_l2_neon_10
        mov             sp,  x11
        ret             x14



        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16

        EXPORT ff_avg_h264_qpel16_mc31_neon_10





ff_avg_h264_qpel16_mc31_neon_10
        










        add             x1,  x1,  #2
        mov             x14, x30
        mov             x8,  x0
        mov             x9,  x1
        sub             x1,  x1,  #2
        b               avg_h264_qpel16_mc11_10



        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16

        EXPORT ff_avg_h264_qpel16_mc02_neon_10





ff_avg_h264_qpel16_mc02_neon_10
        










        mov             x14, x30
        movz            w3, #20, lsl #16
        movk            w3, #5
        mov             v6.s[0], w3
        sub             x1,  x1,  x2, lsl #1
        mov             x3,  x2
        bl              avg_h264_qpel16_v_lowpass_neon_10
        ret             x14



        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16

        EXPORT ff_avg_h264_qpel16_mc03_neon_10





ff_avg_h264_qpel16_mc03_neon_10
        










        mov             x14, x30
        add             x12, x1,  x2
        b               avg_h264_qpel16_mc01_10



        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16

        EXPORT ff_avg_h264_qpel16_mc13_neon_10





ff_avg_h264_qpel16_mc13_neon_10
        










        mov             x14, x30
        mov             x8,  x0
        mov             x9,  x1
        add             x1,  x1,  x2
        b               avg_h264_qpel16_mc11_10



        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16

        EXPORT ff_avg_h264_qpel16_mc33_neon_10





ff_avg_h264_qpel16_mc33_neon_10
        










        add             x1,  x1,  #2
        mov             x14, x30
        mov             x8,  x0
        mov             x9,  x1
        add             x1,  x1,  x2
        sub             x1,  x1,  #2
        b               avg_h264_qpel16_mc11_10




	END
