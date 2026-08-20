	AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN

























































































































































































































































































































































































































































































































































































































































































































































































































































































        ;.arch           armv8.6-a+crc


































	;.arch_extension nodotprod
	;.arch_extension noi8mm
	;.arch_extension nosve
	;.arch_extension nosve2


































































































































































        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16

        EXPORT ff_h264_v_loop_filter_luma_neon





ff_h264_v_loop_filter_luma_neon
        










        cmp             w2,  #0
        ldr             w6,  [x4]
        ccmpne w3, #0, #0
        mov             v24.s[0], w6
        and             w8,  w6,  w6,  lsl #16
        beq            temp_label_0
        ands            w8,  w8,  w8,  lsl #8
        bge            temp_label_1
temp_label_2
temp_label_0

        ret
temp_label_3
temp_label_1


        ld1             {v0.16b},  [x0], x1
        ld1             {v2.16b},  [x0], x1
        ld1             {v4.16b},  [x0], x1
        sub             x0,  x0,  x1, lsl #2
        sub             x0,  x0,  x1, lsl #1
        ld1             {v20.16b},  [x0], x1
        ld1             {v18.16b},  [x0], x1
        ld1             {v16.16b},  [x0], x1

        dup             v22.16b, w2                     
        ushll v24.8h, v24.8b, #0
        uabd            v21.16b, v16.16b, v0.16b        
        ushll v24.4s, v24.4h, #0
        uabd            v28.16b, v18.16b, v16.16b       
        sli             v24.8h,  v24.8h,  #8
        uabd            v30.16b, v2.16b,  v0.16b        
        sli             v24.4s,  v24.4s,  #16
        cmhi            v21.16b, v22.16b, v21.16b       
        dup             v22.16b, w3                     
        cmlt            v23.16b, v24.16b, #0
        cmhi            v28.16b, v22.16b, v28.16b       
        cmhi            v30.16b, v22.16b, v30.16b       
        bic             v21.16b, v21.16b, v23.16b
        uabd            v17.16b, v20.16b, v16.16b       
        and             v21.16b, v21.16b, v28.16b
        uabd            v19.16b,  v4.16b,  v0.16b       
        and             v21.16b, v21.16b, v30.16b      
        shrn            v30.8b,  v21.8h,  #4
        umov             x7, v30.d[0]
        cmhi            v17.16b, v22.16b, v17.16b       
        cmhi            v19.16b, v22.16b, v19.16b       
        cbz             x7,  temp_label_4
        and             v17.16b, v17.16b, v21.16b
        and             v19.16b, v19.16b, v21.16b
        and             v24.16b, v24.16b, v21.16b
        urhadd          v28.16b, v16.16b,  v0.16b
        sub             v21.16b, v24.16b, v17.16b
        uqadd           v23.16b, v18.16b, v24.16b
        uhadd           v20.16b, v20.16b, v28.16b
        sub             v21.16b, v21.16b, v19.16b
        uhadd           v28.16b,  v4.16b, v28.16b
        umin            v23.16b, v23.16b, v20.16b
        uqsub           v22.16b, v18.16b, v24.16b
        uqadd           v4.16b,   v2.16b, v24.16b
        umax            v23.16b, v23.16b, v22.16b
        uqsub           v22.16b,  v2.16b, v24.16b
        umin            v28.16b,  v4.16b, v28.16b
        ushll v4.8h, v0.8b, #0
        umax            v28.16b, v28.16b, v22.16b
        ushll2 v20.8h, v0.16b, #0
        usubw           v4.8h,    v4.8h,  v16.8b
        usubw2          v20.8h,  v20.8h,  v16.16b
        shl             v4.8h,    v4.8h,  #2
        shl             v20.8h,  v20.8h,  #2
        uaddw           v4.8h,    v4.8h,  v18.8b
        uaddw2          v20.8h,  v20.8h,  v18.16b
        usubw           v4.8h,    v4.8h,   v2.8b
        usubw2          v20.8h,  v20.8h,   v2.16b
        rshrn           v4.8b,    v4.8h,  #3
        rshrn2          v4.16b,  v20.8h,  #3
        bsl             v17.16b, v23.16b, v18.16b
        bsl             v19.16b, v28.16b,  v2.16b
        neg             v23.16b, v21.16b
        ushll v28.8h, v16.8b, #0
        smin            v4.16b,   v4.16b, v21.16b
        ushll2 v21.8h, v16.16b, #0
        smax            v4.16b,   v4.16b, v23.16b
        ushll v22.8h, v0.8b, #0
        ushll2 v24.8h, v0.16b, #0
        saddw           v28.8h,  v28.8h,  v4.8b
        saddw2          v21.8h,  v21.8h,  v4.16b
        ssubw           v22.8h,  v22.8h,  v4.8b
        ssubw2          v24.8h,  v24.8h,  v4.16b
        sqxtun          v16.8b,  v28.8h
        sqxtun2         v16.16b, v21.8h
        sqxtun          v0.8b,   v22.8h
        sqxtun2         v0.16b,  v24.8h

        sub             x0,  x0,  x1, lsl #1
        st1             {v17.16b},  [x0], x1
        st1             {v16.16b}, [x0], x1
        st1             {v0.16b},  [x0], x1
        st1             {v19.16b}, [x0]
temp_label_5
temp_label_4

        ret



        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16

        EXPORT ff_h264_h_loop_filter_luma_neon





ff_h264_h_loop_filter_luma_neon
        










        cmp             w2,  #0
        ldr             w6,  [x4]
        ccmpne w3, #0, #0
        mov             v24.s[0], w6
        and             w8,  w6,  w6,  lsl #16
        beq            temp_label_6
        ands            w8,  w8,  w8,  lsl #8
        bge            temp_label_7
temp_label_8
temp_label_6

        ret
temp_label_9
temp_label_7


        sub             x0,  x0,  #4
        ld1             {v6.8b},  [x0], x1
        ld1             {v20.8b}, [x0], x1
        ld1             {v18.8b}, [x0], x1
        ld1             {v16.8b}, [x0], x1
        ld1             {v0.8b},  [x0], x1
        ld1             {v2.8b},  [x0], x1
        ld1             {v4.8b},  [x0], x1
        ld1             {v26.8b}, [x0], x1
        ld1             {v6.d}[1],  [x0], x1
        ld1             {v20.d}[1], [x0], x1
        ld1             {v18.d}[1], [x0], x1
        ld1             {v16.d}[1], [x0], x1
        ld1             {v0.d}[1],  [x0], x1
        ld1             {v2.d}[1],  [x0], x1
        ld1             {v4.d}[1],  [x0], x1
        ld1             {v26.d}[1], [x0], x1

        trn1            v21.16b, v6.16b, v20.16b
        trn2            v23.16b, v6.16b, v20.16b
        trn1            v20.16b, v18.16b, v16.16b
        trn2            v16.16b, v18.16b, v16.16b
        trn1            v6.16b, v0.16b, v2.16b
        trn2            v2.16b, v0.16b, v2.16b
        trn1            v18.16b, v4.16b, v26.16b
        trn2            v26.16b, v4.16b, v26.16b

        trn1            v0.8h,  v6.8h,  v18.8h
        trn2            v18.8h,  v6.8h,  v18.8h
        trn1            v4.8h,  v2.8h,  v26.8h
        trn2            v26.8h,  v2.8h,  v26.8h
        trn1            v2.8h,  v23.8h,  v16.8h
        trn2            v23.8h,  v23.8h,  v16.8h
        trn1            v16.8h,  v21.8h,  v20.8h
        trn2            v21.8h,  v21.8h,  v20.8h

        trn1            v6.4s,  v16.4s,  v0.4s
        trn2            v0.4s,  v16.4s,  v0.4s

        trn1            v20.4s,  v2.4s,  v4.4s
        trn2            v2.4s,  v2.4s,  v4.4s

        trn2            v4.4s,  v21.4s,  v18.4s
        trn1            v18.4s,  v21.4s,  v18.4s

        trn1            v16.4s,  v23.4s,  v26.4s
        trn2            v26.4s,  v23.4s,  v26.4s

        dup             v22.16b, w2                     
        ushll v24.8h, v24.8b, #0
        uabd            v21.16b, v16.16b, v0.16b        
        ushll v24.4s, v24.4h, #0
        uabd            v28.16b, v18.16b, v16.16b       
        sli             v24.8h,  v24.8h,  #8
        uabd            v30.16b, v2.16b,  v0.16b        
        sli             v24.4s,  v24.4s,  #16
        cmhi            v21.16b, v22.16b, v21.16b       
        dup             v22.16b, w3                     
        cmlt            v23.16b, v24.16b, #0
        cmhi            v28.16b, v22.16b, v28.16b       
        cmhi            v30.16b, v22.16b, v30.16b       
        bic             v21.16b, v21.16b, v23.16b
        uabd            v17.16b, v20.16b, v16.16b       
        and             v21.16b, v21.16b, v28.16b
        uabd            v19.16b,  v4.16b,  v0.16b       
        and             v21.16b, v21.16b, v30.16b      
        shrn            v30.8b,  v21.8h,  #4
        umov             x7, v30.d[0]
        cmhi            v17.16b, v22.16b, v17.16b       
        cmhi            v19.16b, v22.16b, v19.16b       
        cbz             x7,  temp_label_10
        and             v17.16b, v17.16b, v21.16b
        and             v19.16b, v19.16b, v21.16b
        and             v24.16b, v24.16b, v21.16b
        urhadd          v28.16b, v16.16b,  v0.16b
        sub             v21.16b, v24.16b, v17.16b
        uqadd           v23.16b, v18.16b, v24.16b
        uhadd           v20.16b, v20.16b, v28.16b
        sub             v21.16b, v21.16b, v19.16b
        uhadd           v28.16b,  v4.16b, v28.16b
        umin            v23.16b, v23.16b, v20.16b
        uqsub           v22.16b, v18.16b, v24.16b
        uqadd           v4.16b,   v2.16b, v24.16b
        umax            v23.16b, v23.16b, v22.16b
        uqsub           v22.16b,  v2.16b, v24.16b
        umin            v28.16b,  v4.16b, v28.16b
        ushll v4.8h, v0.8b, #0
        umax            v28.16b, v28.16b, v22.16b
        ushll2 v20.8h, v0.16b, #0
        usubw           v4.8h,    v4.8h,  v16.8b
        usubw2          v20.8h,  v20.8h,  v16.16b
        shl             v4.8h,    v4.8h,  #2
        shl             v20.8h,  v20.8h,  #2
        uaddw           v4.8h,    v4.8h,  v18.8b
        uaddw2          v20.8h,  v20.8h,  v18.16b
        usubw           v4.8h,    v4.8h,   v2.8b
        usubw2          v20.8h,  v20.8h,   v2.16b
        rshrn           v4.8b,    v4.8h,  #3
        rshrn2          v4.16b,  v20.8h,  #3
        bsl             v17.16b, v23.16b, v18.16b
        bsl             v19.16b, v28.16b,  v2.16b
        neg             v23.16b, v21.16b
        ushll v28.8h, v16.8b, #0
        smin            v4.16b,   v4.16b, v21.16b
        ushll2 v21.8h, v16.16b, #0
        smax            v4.16b,   v4.16b, v23.16b
        ushll v22.8h, v0.8b, #0
        ushll2 v24.8h, v0.16b, #0
        saddw           v28.8h,  v28.8h,  v4.8b
        saddw2          v21.8h,  v21.8h,  v4.16b
        ssubw           v22.8h,  v22.8h,  v4.8b
        ssubw2          v24.8h,  v24.8h,  v4.16b
        sqxtun          v16.8b,  v28.8h
        sqxtun2         v16.16b, v21.8h
        sqxtun          v0.8b,   v22.8h
        sqxtun2         v0.16b,  v24.8h

        trn1            v21.16b, v17.16b,  v16.16b
        trn2            v23.16b, v17.16b,  v16.16b
        trn1            v25.16b, v0.16b,  v19.16b
        trn2            v27.16b, v0.16b,  v19.16b

        trn1            v17.8h,  v21.8h,  v25.8h
        trn2            v0.8h,  v21.8h,  v25.8h
        trn1            v16.8h,  v23.8h,  v27.8h
        trn2            v19.8h,  v23.8h,  v27.8h

        sub             x0,  x0,  x1, lsl #4
        add             x0,  x0,  #2
        st1             {v17.s}[0],  [x0], x1
        st1             {v16.s}[0], [x0], x1
        st1             {v0.s}[0],  [x0], x1
        st1             {v19.s}[0], [x0], x1
        st1             {v17.s}[1],  [x0], x1
        st1             {v16.s}[1], [x0], x1
        st1             {v0.s}[1],  [x0], x1
        st1             {v19.s}[1], [x0], x1
        st1             {v17.s}[2],  [x0], x1
        st1             {v16.s}[2], [x0], x1
        st1             {v0.s}[2],  [x0], x1
        st1             {v19.s}[2], [x0], x1
        st1             {v17.s}[3],  [x0], x1
        st1             {v16.s}[3], [x0], x1
        st1             {v0.s}[3],  [x0], x1
        st1             {v19.s}[3], [x0], x1
temp_label_11
temp_label_10

        ret






        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16

        EXPORT ff_h264_v_loop_filter_luma_intra_neon





ff_h264_v_loop_filter_luma_intra_neon
        










        orr             w4,  w2,  w3
        cbnz            w4,  temp_label_12
        ret
temp_label_13
temp_label_12

        dup             v30.16b, w2                
        dup             v31.16b, w3                

        ld1             {v0.16b},  [x0], x1 
        ld1             {v1.16b},  [x0], x1 
        ld1             {v2.16b},  [x0], x1 
        ld1             {v3.16b},  [x0], x1 
        sub             x0,  x0,  x1, lsl #3
        ld1             {v4.16b},  [x0], x1 
        ld1             {v5.16b},  [x0], x1 
        ld1             {v6.16b},  [x0], x1 
        ld1             {v7.16b},  [x0]     

        uabd            v16.16b, v7.16b,  v0.16b        
        uabd            v17.16b, v6.16b,  v7.16b        
        uabd            v18.16b, v1.16b,  v0.16b        
        cmhi            v19.16b, v30.16b, v16.16b       
        cmhi            v17.16b, v31.16b, v17.16b       
        cmhi            v18.16b, v31.16b, v18.16b       

        movi            v29.16b, #2
        ushr            v30.16b, v30.16b, #2            
        add             v30.16b, v30.16b, v29.16b       
        cmhi            v16.16b, v30.16b, v16.16b       

        and             v19.16b, v19.16b, v17.16b
        and             v19.16b, v19.16b, v18.16b
        shrn            v20.8b,  v19.8h,  #4
        umov             x4, v20.d[0]
        cbz             x4, temp_label_14

        ushll           v20.8h,  v6.8b,   #1
        ushll           v22.8h,  v1.8b,   #1
        ushll2          v21.8h,  v6.16b,  #1
        ushll2          v23.8h,  v1.16b,  #1
        uaddw           v20.8h,  v20.8h,  v7.8b
        uaddw           v22.8h,  v22.8h,  v0.8b
        uaddw2          v21.8h,  v21.8h,  v7.16b
        uaddw2          v23.8h,  v23.8h,  v0.16b
        uaddw           v20.8h,  v20.8h,  v1.8b
        uaddw           v22.8h,  v22.8h,  v6.8b
        uaddw2          v21.8h,  v21.8h,  v1.16b
        uaddw2          v23.8h,  v23.8h,  v6.16b

        rshrn           v24.8b,  v20.8h,  #2 
        rshrn           v25.8b,  v22.8h,  #2 
        rshrn2          v24.16b, v21.8h,  #2 
        rshrn2          v25.16b, v23.8h,  #2 

        uabd            v17.16b, v5.16b,  v7.16b        
        uabd            v18.16b, v2.16b,  v0.16b        
        cmhi            v17.16b, v31.16b, v17.16b       
        cmhi            v18.16b, v31.16b, v18.16b       

        and             v17.16b, v16.16b, v17.16b  
        and             v18.16b, v16.16b, v18.16b  

        not             v30.16b, v17.16b
        not             v31.16b, v18.16b

        and             v30.16b, v30.16b, v19.16b  
        and             v31.16b, v31.16b, v19.16b  

        and             v17.16b, v19.16b, v17.16b  
        and             v18.16b, v19.16b, v18.16b  

        
        uaddl           v26.8h,  v5.8b,   v7.8b
        uaddl2          v27.8h,  v5.16b,  v7.16b
        uaddw           v26.8h,  v26.8h,  v0.8b
        uaddw2          v27.8h,  v27.8h,  v0.16b
        add             v20.8h,  v20.8h,  v26.8h
        add             v21.8h,  v21.8h,  v27.8h
        uaddw           v20.8h,  v20.8h,  v0.8b
        uaddw2          v21.8h,  v21.8h,  v0.16b
        rshrn           v20.8b,  v20.8h,  #3 
        rshrn2          v20.16b, v21.8h,  #3 
        uaddw           v26.8h,  v26.8h,  v6.8b
        uaddw2          v27.8h,  v27.8h,  v6.16b
        rshrn           v21.8b,  v26.8h,  #2 
        rshrn2          v21.16b, v27.8h,  #2 
        uaddl           v28.8h,  v4.8b,   v5.8b
        uaddl2          v29.8h,  v4.16b,  v5.16b
        shl             v28.8h,  v28.8h,  #1
        shl             v29.8h,  v29.8h,  #1
        add             v28.8h,  v28.8h,  v26.8h
        add             v29.8h,  v29.8h,  v27.8h
        rshrn           v19.8b,  v28.8h,  #3 
        rshrn2          v19.16b, v29.8h,  #3 

        
        uaddl           v26.8h,  v2.8b,   v0.8b
        uaddl2          v27.8h,  v2.16b,  v0.16b
        uaddw           v26.8h,  v26.8h,  v7.8b
        uaddw2          v27.8h,  v27.8h,  v7.16b
        add             v22.8h,  v22.8h,  v26.8h
        add             v23.8h,  v23.8h,  v27.8h
        uaddw           v22.8h,  v22.8h,  v7.8b
        uaddw2          v23.8h,  v23.8h,  v7.16b
        rshrn           v22.8b,  v22.8h,  #3 
        rshrn2          v22.16b, v23.8h,  #3 
        uaddw           v26.8h,  v26.8h,  v1.8b
        uaddw2          v27.8h,  v27.8h,  v1.16b
        rshrn           v23.8b,  v26.8h,  #2 
        rshrn2          v23.16b, v27.8h,  #2 
        uaddl           v28.8h,  v2.8b,   v3.8b
        uaddl2          v29.8h,  v2.16b,  v3.16b
        shl             v28.8h,  v28.8h,  #1
        shl             v29.8h,  v29.8h,  #1
        add             v28.8h,  v28.8h,  v26.8h
        add             v29.8h,  v29.8h,  v27.8h
        rshrn           v26.8b,  v28.8h,  #3 
        rshrn2          v26.16b, v29.8h,  #3 

        bit             v7.16b,  v24.16b, v30.16b  
        bit             v0.16b,  v25.16b, v31.16b  
        bit             v7.16b,  v20.16b, v17.16b  
        bit             v6.16b,  v21.16b, v17.16b  
        bit             v5.16b,  v19.16b, v17.16b  
        bit             v0.16b,  v22.16b, v18.16b  
        bit             v1.16b,  v23.16b, v18.16b  
        bit             v2.16b,  v26.16b, v18.16b  

        sub             x0,  x0,  x1, lsl #1
        st1             {v5.16b}, [x0], x1  
        st1             {v6.16b}, [x0], x1  
        st1             {v7.16b}, [x0], x1  
        st1             {v0.16b}, [x0], x1  
        st1             {v1.16b}, [x0], x1  
        st1             {v2.16b}, [x0]      
temp_label_15
temp_label_14

        ret



        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16

        EXPORT ff_h264_h_loop_filter_luma_intra_neon





ff_h264_h_loop_filter_luma_intra_neon
        










        orr             w4,  w2,  w3
        cbnz            w4,  temp_label_16
        ret
temp_label_17
temp_label_16

        dup             v30.16b, w2                
        dup             v31.16b, w3                

        sub             x0,  x0,  #4
        ld1             {v4.8b},  [x0], x1
        ld1             {v5.8b},  [x0], x1
        ld1             {v6.8b},  [x0], x1
        ld1             {v7.8b},  [x0], x1
        ld1             {v0.8b},  [x0], x1
        ld1             {v1.8b},  [x0], x1
        ld1             {v2.8b},  [x0], x1
        ld1             {v3.8b},  [x0], x1
        ld1             {v4.d}[1],  [x0], x1
        ld1             {v5.d}[1],  [x0], x1
        ld1             {v6.d}[1],  [x0], x1
        ld1             {v7.d}[1],  [x0], x1
        ld1             {v0.d}[1],  [x0], x1
        ld1             {v1.d}[1],  [x0], x1
        ld1             {v2.d}[1],  [x0], x1
        ld1             {v3.d}[1],  [x0], x1

        trn1            v21.16b, v4.16b, v5.16b
        trn2            v23.16b, v4.16b, v5.16b
        trn1            v5.16b, v6.16b, v7.16b
        trn2            v7.16b, v6.16b, v7.16b
        trn1            v4.16b, v0.16b, v1.16b
        trn2            v1.16b, v0.16b, v1.16b
        trn1            v6.16b, v2.16b, v3.16b
        trn2            v3.16b, v2.16b, v3.16b

        trn1            v0.8h,  v4.8h,  v6.8h
        trn2            v6.8h,  v4.8h,  v6.8h
        trn1            v2.8h,  v1.8h,  v3.8h
        trn2            v3.8h,  v1.8h,  v3.8h
        trn1            v1.8h,  v23.8h,  v7.8h
        trn2            v23.8h,  v23.8h,  v7.8h
        trn1            v7.8h,  v21.8h,  v5.8h
        trn2            v21.8h,  v21.8h,  v5.8h

        trn1            v4.4s,  v7.4s,  v0.4s
        trn2            v0.4s,  v7.4s,  v0.4s

        trn1            v5.4s,  v1.4s,  v2.4s
        trn2            v1.4s,  v1.4s,  v2.4s

        trn2            v2.4s,  v21.4s,  v6.4s
        trn1            v6.4s,  v21.4s,  v6.4s

        trn1            v7.4s,  v23.4s,  v3.4s
        trn2            v3.4s,  v23.4s,  v3.4s

        uabd            v16.16b, v7.16b,  v0.16b        
        uabd            v17.16b, v6.16b,  v7.16b        
        uabd            v18.16b, v1.16b,  v0.16b        
        cmhi            v19.16b, v30.16b, v16.16b       
        cmhi            v17.16b, v31.16b, v17.16b       
        cmhi            v18.16b, v31.16b, v18.16b       

        movi            v29.16b, #2
        ushr            v30.16b, v30.16b, #2            
        add             v30.16b, v30.16b, v29.16b       
        cmhi            v16.16b, v30.16b, v16.16b       

        and             v19.16b, v19.16b, v17.16b
        and             v19.16b, v19.16b, v18.16b
        shrn            v20.8b,  v19.8h,  #4
        umov             x4, v20.d[0]
        cbz             x4, temp_label_18

        ushll           v20.8h,  v6.8b,   #1
        ushll           v22.8h,  v1.8b,   #1
        ushll2          v21.8h,  v6.16b,  #1
        ushll2          v23.8h,  v1.16b,  #1
        uaddw           v20.8h,  v20.8h,  v7.8b
        uaddw           v22.8h,  v22.8h,  v0.8b
        uaddw2          v21.8h,  v21.8h,  v7.16b
        uaddw2          v23.8h,  v23.8h,  v0.16b
        uaddw           v20.8h,  v20.8h,  v1.8b
        uaddw           v22.8h,  v22.8h,  v6.8b
        uaddw2          v21.8h,  v21.8h,  v1.16b
        uaddw2          v23.8h,  v23.8h,  v6.16b

        rshrn           v24.8b,  v20.8h,  #2 
        rshrn           v25.8b,  v22.8h,  #2 
        rshrn2          v24.16b, v21.8h,  #2 
        rshrn2          v25.16b, v23.8h,  #2 

        uabd            v17.16b, v5.16b,  v7.16b        
        uabd            v18.16b, v2.16b,  v0.16b        
        cmhi            v17.16b, v31.16b, v17.16b       
        cmhi            v18.16b, v31.16b, v18.16b       

        and             v17.16b, v16.16b, v17.16b  
        and             v18.16b, v16.16b, v18.16b  

        not             v30.16b, v17.16b
        not             v31.16b, v18.16b

        and             v30.16b, v30.16b, v19.16b  
        and             v31.16b, v31.16b, v19.16b  

        and             v17.16b, v19.16b, v17.16b  
        and             v18.16b, v19.16b, v18.16b  

        
        uaddl           v26.8h,  v5.8b,   v7.8b
        uaddl2          v27.8h,  v5.16b,  v7.16b
        uaddw           v26.8h,  v26.8h,  v0.8b
        uaddw2          v27.8h,  v27.8h,  v0.16b
        add             v20.8h,  v20.8h,  v26.8h
        add             v21.8h,  v21.8h,  v27.8h
        uaddw           v20.8h,  v20.8h,  v0.8b
        uaddw2          v21.8h,  v21.8h,  v0.16b
        rshrn           v20.8b,  v20.8h,  #3 
        rshrn2          v20.16b, v21.8h,  #3 
        uaddw           v26.8h,  v26.8h,  v6.8b
        uaddw2          v27.8h,  v27.8h,  v6.16b
        rshrn           v21.8b,  v26.8h,  #2 
        rshrn2          v21.16b, v27.8h,  #2 
        uaddl           v28.8h,  v4.8b,   v5.8b
        uaddl2          v29.8h,  v4.16b,  v5.16b
        shl             v28.8h,  v28.8h,  #1
        shl             v29.8h,  v29.8h,  #1
        add             v28.8h,  v28.8h,  v26.8h
        add             v29.8h,  v29.8h,  v27.8h
        rshrn           v19.8b,  v28.8h,  #3 
        rshrn2          v19.16b, v29.8h,  #3 

        
        uaddl           v26.8h,  v2.8b,   v0.8b
        uaddl2          v27.8h,  v2.16b,  v0.16b
        uaddw           v26.8h,  v26.8h,  v7.8b
        uaddw2          v27.8h,  v27.8h,  v7.16b
        add             v22.8h,  v22.8h,  v26.8h
        add             v23.8h,  v23.8h,  v27.8h
        uaddw           v22.8h,  v22.8h,  v7.8b
        uaddw2          v23.8h,  v23.8h,  v7.16b
        rshrn           v22.8b,  v22.8h,  #3 
        rshrn2          v22.16b, v23.8h,  #3 
        uaddw           v26.8h,  v26.8h,  v1.8b
        uaddw2          v27.8h,  v27.8h,  v1.16b
        rshrn           v23.8b,  v26.8h,  #2 
        rshrn2          v23.16b, v27.8h,  #2 
        uaddl           v28.8h,  v2.8b,   v3.8b
        uaddl2          v29.8h,  v2.16b,  v3.16b
        shl             v28.8h,  v28.8h,  #1
        shl             v29.8h,  v29.8h,  #1
        add             v28.8h,  v28.8h,  v26.8h
        add             v29.8h,  v29.8h,  v27.8h
        rshrn           v26.8b,  v28.8h,  #3 
        rshrn2          v26.16b, v29.8h,  #3 

        bit             v7.16b,  v24.16b, v30.16b  
        bit             v0.16b,  v25.16b, v31.16b  
        bit             v7.16b,  v20.16b, v17.16b  
        bit             v6.16b,  v21.16b, v17.16b  
        bit             v5.16b,  v19.16b, v17.16b  
        bit             v0.16b,  v22.16b, v18.16b  
        bit             v1.16b,  v23.16b, v18.16b  
        bit             v2.16b,  v26.16b, v18.16b  

        trn1            v21.16b, v4.16b, v5.16b
        trn2            v23.16b, v4.16b, v5.16b
        trn1            v5.16b, v6.16b, v7.16b
        trn2            v7.16b, v6.16b, v7.16b
        trn1            v4.16b, v0.16b, v1.16b
        trn2            v1.16b, v0.16b, v1.16b
        trn1            v6.16b, v2.16b, v3.16b
        trn2            v3.16b, v2.16b, v3.16b

        trn1            v0.8h,  v4.8h,  v6.8h
        trn2            v6.8h,  v4.8h,  v6.8h
        trn1            v2.8h,  v1.8h,  v3.8h
        trn2            v3.8h,  v1.8h,  v3.8h
        trn1            v1.8h,  v23.8h,  v7.8h
        trn2            v23.8h,  v23.8h,  v7.8h
        trn1            v7.8h,  v21.8h,  v5.8h
        trn2            v21.8h,  v21.8h,  v5.8h

        trn1            v4.4s,  v7.4s,  v0.4s
        trn2            v0.4s,  v7.4s,  v0.4s

        trn1            v5.4s,  v1.4s,  v2.4s
        trn2            v1.4s,  v1.4s,  v2.4s

        trn2            v2.4s,  v21.4s,  v6.4s
        trn1            v6.4s,  v21.4s,  v6.4s

        trn1            v7.4s,  v23.4s,  v3.4s
        trn2            v3.4s,  v23.4s,  v3.4s

        sub             x0,  x0,  x1, lsl #4
        st1             {v4.8b},  [x0], x1
        st1             {v5.8b},  [x0], x1
        st1             {v6.8b},  [x0], x1
        st1             {v7.8b},  [x0], x1
        st1             {v0.8b},  [x0], x1
        st1             {v1.8b},  [x0], x1
        st1             {v2.8b},  [x0], x1
        st1             {v3.8b},  [x0], x1
        st1             {v4.d}[1],  [x0], x1
        st1             {v5.d}[1],  [x0], x1
        st1             {v6.d}[1],  [x0], x1
        st1             {v7.d}[1],  [x0], x1
        st1             {v0.d}[1],  [x0], x1
        st1             {v1.d}[1],  [x0], x1
        st1             {v2.d}[1],  [x0], x1
        st1             {v3.d}[1],  [x0], x1
temp_label_19
temp_label_18

        ret




        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16

        EXPORT ff_h264_v_loop_filter_chroma_neon





ff_h264_v_loop_filter_chroma_neon
        










        cmp             w2,  #0
        ldr             w6,  [x4]
        ccmpne w3, #0, #0
        mov             v24.s[0], w6
        and             w8,  w6,  w6,  lsl #16
        beq            temp_label_20
        ands            w8,  w8,  w8,  lsl #8
        bge            temp_label_21
temp_label_22
temp_label_20

        ret
temp_label_23
temp_label_21


        sub             x0,  x0,  x1, lsl #1
        ld1             {v18.8b}, [x0], x1
        ld1             {v16.8b}, [x0], x1
        ld1             {v0.8b},  [x0], x1
        ld1             {v2.8b},  [x0]

        dup             v22.8b, w2              
        dup             v23.8b, w3              
        ushll v24.8h, v24.8b, #0
        uabd            v26.8b, v16.8b, v0.8b   
        uabd            v28.8b, v18.8b, v16.8b  
        uabd            v30.8b, v2.8b,  v0.8b   
        cmhi            v26.8b, v22.8b, v26.8b  
        cmhi            v28.8b, v23.8b, v28.8b  
        cmhi            v30.8b, v23.8b, v30.8b  
        ushll v4.8h, v0.8b, #0
        and             v26.8b, v26.8b, v28.8b
        usubw           v4.8h,  v4.8h,  v16.8b
        and             v26.8b, v26.8b, v30.8b
        shl             v4.8h,  v4.8h,  #2
        umov             x8,  v26.d[0]
        sli             v24.8h, v24.8h, #8
        uaddw           v4.8h,  v4.8h,  v18.8b
        cbz             x8,  temp_label_24
        usubw           v4.8h,  v4.8h,  v2.8b
        rshrn           v4.8b,  v4.8h,  #3
        smin            v4.8b,  v4.8b,  v24.8b
        neg             v25.8b, v24.8b
        smax            v4.8b,  v4.8b,  v25.8b
        ushll v22.8h, v0.8b, #0
        and             v4.8b,  v4.8b,  v26.8b
        ushll v28.8h, v16.8b, #0
        saddw           v28.8h, v28.8h, v4.8b
        ssubw           v22.8h, v22.8h, v4.8b
        sqxtun          v16.8b, v28.8h
        sqxtun          v0.8b,  v22.8h

        sub             x0,  x0,  x1, lsl #1
        st1             {v16.8b}, [x0], x1
        st1             {v0.8b},  [x0], x1
temp_label_25
temp_label_24

        ret



        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16

        EXPORT ff_h264_h_loop_filter_chroma_neon





ff_h264_h_loop_filter_chroma_neon
        










        cmp             w2,  #0
        ldr             w6,  [x4]
        ccmpne w3, #0, #0
        mov             v24.s[0], w6
        and             w8,  w6,  w6,  lsl #16
        beq            temp_label_26
        ands            w8,  w8,  w8,  lsl #8
        bge            temp_label_27
temp_label_28
temp_label_26

        ret
temp_label_29
temp_label_27


        sub             x0,  x0,  #2
h_loop_filter_chroma420
        ld1             {v18.s}[0], [x0], x1
        ld1             {v16.s}[0], [x0], x1
        ld1             {v0.s}[0],  [x0], x1
        ld1             {v2.s}[0],  [x0], x1
        ld1             {v18.s}[1], [x0], x1
        ld1             {v16.s}[1], [x0], x1
        ld1             {v0.s}[1],  [x0], x1
        ld1             {v2.s}[1],  [x0], x1

        trn1            v28.8b,  v18.8b,  v16.8b
        trn2            v29.8b,  v18.8b,  v16.8b
        trn1            v30.8b,  v0.8b,  v2.8b
        trn2            v31.8b,  v0.8b,  v2.8b

        trn1            v18.4h,  v28.4h,  v30.4h
        trn2            v0.4h,  v28.4h,  v30.4h
        trn1            v16.4h,  v29.4h,  v31.4h
        trn2            v2.4h,  v29.4h,  v31.4h

        dup             v22.8b, w2              
        dup             v23.8b, w3              
        ushll v24.8h, v24.8b, #0
        uabd            v26.8b, v16.8b, v0.8b   
        uabd            v28.8b, v18.8b, v16.8b  
        uabd            v30.8b, v2.8b,  v0.8b   
        cmhi            v26.8b, v22.8b, v26.8b  
        cmhi            v28.8b, v23.8b, v28.8b  
        cmhi            v30.8b, v23.8b, v30.8b  
        ushll v4.8h, v0.8b, #0
        and             v26.8b, v26.8b, v28.8b
        usubw           v4.8h,  v4.8h,  v16.8b
        and             v26.8b, v26.8b, v30.8b
        shl             v4.8h,  v4.8h,  #2
        umov             x8,  v26.d[0]
        sli             v24.8h, v24.8h, #8
        uaddw           v4.8h,  v4.8h,  v18.8b
        cbz             x8,  temp_label_30
        usubw           v4.8h,  v4.8h,  v2.8b
        rshrn           v4.8b,  v4.8h,  #3
        smin            v4.8b,  v4.8b,  v24.8b
        neg             v25.8b, v24.8b
        smax            v4.8b,  v4.8b,  v25.8b
        ushll v22.8h, v0.8b, #0
        and             v4.8b,  v4.8b,  v26.8b
        ushll v28.8h, v16.8b, #0
        saddw           v28.8h, v28.8h, v4.8b
        ssubw           v22.8h, v22.8h, v4.8b
        sqxtun          v16.8b, v28.8h
        sqxtun          v0.8b,  v22.8h

        trn1            v28.8b,  v18.8b,  v16.8b
        trn2            v29.8b,  v18.8b,  v16.8b
        trn1            v30.8b,  v0.8b,  v2.8b
        trn2            v31.8b,  v0.8b,  v2.8b

        trn1            v18.4h,  v28.4h,  v30.4h
        trn2            v0.4h,  v28.4h,  v30.4h
        trn1            v16.4h,  v29.4h,  v31.4h
        trn2            v2.4h,  v29.4h,  v31.4h

        sub             x0,  x0,  x1, lsl #3
        st1             {v18.s}[0], [x0], x1
        st1             {v16.s}[0], [x0], x1
        st1             {v0.s}[0],  [x0], x1
        st1             {v2.s}[0],  [x0], x1
        st1             {v18.s}[1], [x0], x1
        st1             {v16.s}[1], [x0], x1
        st1             {v0.s}[1],  [x0], x1
        st1             {v2.s}[1],  [x0], x1
temp_label_31
temp_label_30

        ret



        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16

        EXPORT ff_h264_h_loop_filter_chroma422_neon





ff_h264_h_loop_filter_chroma422_neon
        










        cmp             w2,  #0
        ldr             w6,  [x4]
        ccmpne w3, #0, #0
        mov             v24.s[0], w6
        and             w8,  w6,  w6,  lsl #16
        beq            temp_label_32
        ands            w8,  w8,  w8,  lsl #8
        bge            temp_label_33
temp_label_34
temp_label_32

        ret
temp_label_35
temp_label_33

        add             x5,  x0,  x1
        sub             x0,  x0,  #2
        add             x1,  x1,  x1
        mov             x7,  x30
        bl              h_loop_filter_chroma420
        mov             x30, x7
        sub             x0,  x5,  #2
        mov             v24.s[0], w6
        b               h_loop_filter_chroma420




        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16

        EXPORT ff_h264_v_loop_filter_chroma_intra_neon





ff_h264_v_loop_filter_chroma_intra_neon
        










        orr             w4,  w2,  w3
        cbnz            w4,  temp_label_36
        ret
temp_label_37
temp_label_36

        dup             v30.16b, w2                
        dup             v31.16b, w3                

        sub             x0,  x0,  x1, lsl #1
        ld1             {v18.8b}, [x0], x1
        ld1             {v16.8b}, [x0], x1
        ld1             {v17.8b}, [x0], x1
        ld1             {v19.8b}, [x0]

        uabd            v26.8b,  v16.8b,  v17.8b  
        uabd            v27.8b,  v18.8b,  v16.8b  
        uabd            v28.8b,  v19.8b,  v17.8b  
        cmhi            v26.8b,  v30.8b,  v26.8b  
        cmhi            v27.8b,  v31.8b,  v27.8b  
        cmhi            v28.8b,  v31.8b,  v28.8b  
        and             v26.8b,  v26.8b,  v27.8b
        and             v26.8b,  v26.8b,  v28.8b
        umov             x2, v26.d[0]

        ushll           v4.8h,   v18.8b,  #1
        ushll           v6.8h,   v19.8b,  #1
        cbz             x2, temp_label_38
        uaddl           v20.8h,  v16.8b,  v19.8b
        uaddl           v22.8h,  v17.8b,  v18.8b
        add             v20.8h,  v20.8h,  v4.8h
        add             v22.8h,  v22.8h,  v6.8h
        uqrshrn         v24.8b,  v20.8h,  #2
        uqrshrn         v25.8b,  v22.8h,  #2
        bit             v16.8b,  v24.8b,  v26.8b
        bit             v17.8b,  v25.8b,  v26.8b

        sub             x0,  x0,  x1, lsl #1
        st1             {v16.8b}, [x0], x1
        st1             {v17.8b}, [x0], x1

temp_label_39
temp_label_38

        ret



        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16

        EXPORT ff_h264_h_loop_filter_chroma_mbaff_intra_neon





ff_h264_h_loop_filter_chroma_mbaff_intra_neon
        










        orr             w4,  w2,  w3
        cbnz            w4,  temp_label_40
        ret
temp_label_41
temp_label_40

        dup             v30.16b, w2                
        dup             v31.16b, w3                

        sub             x4,  x0,  #2
        sub             x0,  x0,  #1
        ld1             {v18.8b}, [x4], x1
        ld1             {v16.8b}, [x4], x1
        ld1             {v17.8b}, [x4], x1
        ld1             {v19.8b}, [x4], x1

        trn1            v26.8b,  v18.8b,  v16.8b
        trn2            v27.8b,  v18.8b,  v16.8b
        trn1            v28.8b,  v17.8b,  v19.8b
        trn2            v29.8b,  v17.8b,  v19.8b

        trn1            v18.4h,  v26.4h,  v28.4h
        trn2            v17.4h,  v26.4h,  v28.4h
        trn1            v16.4h,  v27.4h,  v29.4h
        trn2            v19.4h,  v27.4h,  v29.4h

        uabd            v26.8b,  v16.8b,  v17.8b  
        uabd            v27.8b,  v18.8b,  v16.8b  
        uabd            v28.8b,  v19.8b,  v17.8b  
        cmhi            v26.8b,  v30.8b,  v26.8b  
        cmhi            v27.8b,  v31.8b,  v27.8b  
        cmhi            v28.8b,  v31.8b,  v28.8b  
        and             v26.8b,  v26.8b,  v27.8b
        and             v26.8b,  v26.8b,  v28.8b
        umov             x2, v26.d[0]

        ushll           v4.8h,   v18.8b,  #1
        ushll           v6.8h,   v19.8b,  #1
        cbz             x2, temp_label_42
        uaddl           v20.8h,  v16.8b,  v19.8b
        uaddl           v22.8h,  v17.8b,  v18.8b
        add             v20.8h,  v20.8h,  v4.8h
        add             v22.8h,  v22.8h,  v6.8h
        uqrshrn         v24.8b,  v20.8h,  #2
        uqrshrn         v25.8b,  v22.8h,  #2
        bit             v16.8b,  v24.8b,  v26.8b
        bit             v17.8b,  v25.8b,  v26.8b

        st2             {v16.b,v17.b}[0], [x0], x1
        st2             {v16.b,v17.b}[1], [x0], x1
        st2             {v16.b,v17.b}[2], [x0], x1
        st2             {v16.b,v17.b}[3], [x0], x1

temp_label_43
temp_label_42

        ret



        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16

        EXPORT ff_h264_h_loop_filter_chroma_intra_neon





ff_h264_h_loop_filter_chroma_intra_neon
        










        orr             w4,  w2,  w3
        cbnz            w4,  temp_label_44
        ret
temp_label_45
temp_label_44

        dup             v30.16b, w2                
        dup             v31.16b, w3                

        sub             x4,  x0,  #2
        sub             x0,  x0,  #1
h_loop_filter_chroma420_intra
        ld1             {v18.8b}, [x4], x1
        ld1             {v16.8b}, [x4], x1
        ld1             {v17.8b}, [x4], x1
        ld1             {v19.8b}, [x4], x1
        ld1             {v18.s}[1], [x4], x1
        ld1             {v16.s}[1], [x4], x1
        ld1             {v17.s}[1], [x4], x1
        ld1             {v19.s}[1], [x4], x1

        trn1            v26.8b,  v18.8b,  v16.8b
        trn2            v27.8b,  v18.8b,  v16.8b
        trn1            v28.8b,  v17.8b,  v19.8b
        trn2            v29.8b,  v17.8b,  v19.8b

        trn1            v18.4h,  v26.4h,  v28.4h
        trn2            v17.4h,  v26.4h,  v28.4h
        trn1            v16.4h,  v27.4h,  v29.4h
        trn2            v19.4h,  v27.4h,  v29.4h

        uabd            v26.8b,  v16.8b,  v17.8b  
        uabd            v27.8b,  v18.8b,  v16.8b  
        uabd            v28.8b,  v19.8b,  v17.8b  
        cmhi            v26.8b,  v30.8b,  v26.8b  
        cmhi            v27.8b,  v31.8b,  v27.8b  
        cmhi            v28.8b,  v31.8b,  v28.8b  
        and             v26.8b,  v26.8b,  v27.8b
        and             v26.8b,  v26.8b,  v28.8b
        umov             x2, v26.d[0]

        ushll           v4.8h,   v18.8b,  #1
        ushll           v6.8h,   v19.8b,  #1
        cbz             x2, temp_label_46
        uaddl           v20.8h,  v16.8b,  v19.8b
        uaddl           v22.8h,  v17.8b,  v18.8b
        add             v20.8h,  v20.8h,  v4.8h
        add             v22.8h,  v22.8h,  v6.8h
        uqrshrn         v24.8b,  v20.8h,  #2
        uqrshrn         v25.8b,  v22.8h,  #2
        bit             v16.8b,  v24.8b,  v26.8b
        bit             v17.8b,  v25.8b,  v26.8b

        st2             {v16.b,v17.b}[0], [x0], x1
        st2             {v16.b,v17.b}[1], [x0], x1
        st2             {v16.b,v17.b}[2], [x0], x1
        st2             {v16.b,v17.b}[3], [x0], x1
        st2             {v16.b,v17.b}[4], [x0], x1
        st2             {v16.b,v17.b}[5], [x0], x1
        st2             {v16.b,v17.b}[6], [x0], x1
        st2             {v16.b,v17.b}[7], [x0], x1

temp_label_47
temp_label_46

        ret



        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16

        EXPORT ff_h264_h_loop_filter_chroma422_intra_neon





ff_h264_h_loop_filter_chroma422_intra_neon
        










        orr             w4,  w2,  w3
        cbnz            w4,  temp_label_48
        ret
temp_label_49
temp_label_48

        dup             v30.16b, w2                
        dup             v31.16b, w3                
        sub             x4,  x0,  #2
        add             x5,  x0,  x1, lsl #3
        sub             x0,  x0,  #1
        mov             x7,  x30
        bl              h_loop_filter_chroma420_intra
        sub             x0,  x5,  #1
        mov             x30, x7
        b               h_loop_filter_chroma420_intra







        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16

        EXPORT ff_biweight_h264_pixels_16_neon





ff_biweight_h264_pixels_16_neon
        










        lsr             w8,  w5,  #31
        add             w7,  w7,  #1
        eor             w8,  w8,  w6,  lsr #30
        orr             w7,  w7,  #1
        dup             v18.8h,   w4
        lsl             w7,  w7,  w4
        not             v18.16b,  v18.16b
        dup             v16.8h,   w7
        mov             x7,  x0
        cbz             w8,  temp_label_50
        subs            w8,  w8,  #1
        beq            temp_label_51
        subs            w8,  w8,  #1
        beq            temp_label_52
        b               temp_label_53
temp_label_54
temp_label_50
        dup             v0.16b,  w5
        dup             v1.16b,  w6
        orr v4.16b, v16.16b, v16.16b
        orr v6.16b, v16.16b, v16.16b
temp_label_55
      subs            w3,  w3,  #2
        ld1             {v20.16b}, [x0], x2
        umlal           v4.8h,   v0.8b,  v20.8b
        umlal2       v6.8H,   v0.16B, v20.16B
        ld1             {v22.16b}, [x1], x2
        umlal           v4.8h,   v1.8b,  v22.8b
        umlal2       v6.8H,   v1.16B, v22.16B
        orr v24.16b, v16.16b, v16.16b
        ld1             {v28.16b}, [x0], x2
        orr v26.16b, v16.16b, v16.16b
        umlal           v24.8h,  v0.8b,  v28.8b
        umlal2       v26.8H,  v0.16B, v28.16B
        ld1             {v30.16b}, [x1], x2
        umlal           v24.8h,  v1.8b,  v30.8b
        umlal2       v26.8H,  v1.16B, v30.16B
        sshl            v4.8h,   v4.8h,  v18.8h
        sshl            v6.8h,   v6.8h,  v18.8h
        sqxtun          v4.8b,   v4.8h
        sqxtun2         v4.16b,  v6.8h
        sshl            v24.8h,  v24.8h, v18.8h
        sshl            v26.8h,  v26.8h, v18.8h
        sqxtun          v24.8b,  v24.8h
        sqxtun2         v24.16b, v26.8h
        orr v6.16b, v16.16b, v16.16b
        st1             {v4.16b},  [x7], x2
        orr v4.16b, v16.16b, v16.16b
        st1             {v24.16b}, [x7], x2
        bne            temp_label_55
        ret
temp_label_56
temp_label_51
     neg             w5, w5
        dup             v0.16b,  w5
        dup             v1.16b,  w6
        orr v4.16b, v16.16b, v16.16b
        orr v6.16b, v16.16b, v16.16b
temp_label_57
      subs            w3,  w3,  #2
        ld1             {v20.16b}, [x0], x2
        umlsl           v4.8h,   v0.8b,  v20.8b
        umlsl2       v6.8H,   v0.16B, v20.16B
        ld1             {v22.16b}, [x1], x2
        umlal           v4.8h,   v1.8b,  v22.8b
        umlal2       v6.8H,   v1.16B, v22.16B
        orr v24.16b, v16.16b, v16.16b
        ld1             {v28.16b}, [x0], x2
        orr v26.16b, v16.16b, v16.16b
        umlsl           v24.8h,  v0.8b,  v28.8b
        umlsl2       v26.8H,  v0.16B, v28.16B
        ld1             {v30.16b}, [x1], x2
        umlal           v24.8h,  v1.8b,  v30.8b
        umlal2       v26.8H,  v1.16B, v30.16B
        sshl            v4.8h,   v4.8h,  v18.8h
        sshl            v6.8h,   v6.8h,  v18.8h
        sqxtun          v4.8b,   v4.8h
        sqxtun2         v4.16b,  v6.8h
        sshl            v24.8h,  v24.8h, v18.8h
        sshl            v26.8h,  v26.8h, v18.8h
        sqxtun          v24.8b,  v24.8h
        sqxtun2         v24.16b, v26.8h
        orr v6.16b, v16.16b, v16.16b
        st1             {v4.16b},  [x7], x2
        orr v4.16b, v16.16b, v16.16b
        st1             {v24.16b}, [x7], x2
        bne            temp_label_57
        ret
temp_label_58
temp_label_52
     neg             w5, w5
        neg             w6, w6
        dup             v0.16b,  w5
        dup             v1.16b,  w6
        orr v4.16b, v16.16b, v16.16b
        orr v6.16b, v16.16b, v16.16b
temp_label_59
      subs            w3,  w3,  #2
        ld1             {v20.16b}, [x0], x2
        umlsl           v4.8h,   v0.8b,  v20.8b
        umlsl2       v6.8H,   v0.16B, v20.16B
        ld1             {v22.16b}, [x1], x2
        umlsl           v4.8h,   v1.8b,  v22.8b
        umlsl2       v6.8H,   v1.16B, v22.16B
        orr v24.16b, v16.16b, v16.16b
        ld1             {v28.16b}, [x0], x2
        orr v26.16b, v16.16b, v16.16b
        umlsl           v24.8h,  v0.8b,  v28.8b
        umlsl2       v26.8H,  v0.16B, v28.16B
        ld1             {v30.16b}, [x1], x2
        umlsl           v24.8h,  v1.8b,  v30.8b
        umlsl2       v26.8H,  v1.16B, v30.16B
        sshl            v4.8h,   v4.8h,  v18.8h
        sshl            v6.8h,   v6.8h,  v18.8h
        sqxtun          v4.8b,   v4.8h
        sqxtun2         v4.16b,  v6.8h
        sshl            v24.8h,  v24.8h, v18.8h
        sshl            v26.8h,  v26.8h, v18.8h
        sqxtun          v24.8b,  v24.8h
        sqxtun2         v24.16b, v26.8h
        orr v6.16b, v16.16b, v16.16b
        st1             {v4.16b},  [x7], x2
        orr v4.16b, v16.16b, v16.16b
        st1             {v24.16b}, [x7], x2
        bne            temp_label_59
        ret
temp_label_60
temp_label_53
     neg             w6, w6
        dup             v0.16b,  w5
        dup             v1.16b,  w6
        orr v4.16b, v16.16b, v16.16b
        orr v6.16b, v16.16b, v16.16b
temp_label_61
      subs            w3,  w3,  #2
        ld1             {v20.16b}, [x0], x2
        umlal           v4.8h,   v0.8b,  v20.8b
        umlal2       v6.8H,   v0.16B, v20.16B
        ld1             {v22.16b}, [x1], x2
        umlsl           v4.8h,   v1.8b,  v22.8b
        umlsl2       v6.8H,   v1.16B, v22.16B
        orr v24.16b, v16.16b, v16.16b
        ld1             {v28.16b}, [x0], x2
        orr v26.16b, v16.16b, v16.16b
        umlal           v24.8h,  v0.8b,  v28.8b
        umlal2       v26.8H,  v0.16B, v28.16B
        ld1             {v30.16b}, [x1], x2
        umlsl           v24.8h,  v1.8b,  v30.8b
        umlsl2       v26.8H,  v1.16B, v30.16B
        sshl            v4.8h,   v4.8h,  v18.8h
        sshl            v6.8h,   v6.8h,  v18.8h
        sqxtun          v4.8b,   v4.8h
        sqxtun2         v4.16b,  v6.8h
        sshl            v24.8h,  v24.8h, v18.8h
        sshl            v26.8h,  v26.8h, v18.8h
        sqxtun          v24.8b,  v24.8h
        sqxtun2         v24.16b, v26.8h
        orr v6.16b, v16.16b, v16.16b
        st1             {v4.16b},  [x7], x2
        orr v4.16b, v16.16b, v16.16b
        st1             {v24.16b}, [x7], x2
        bne            temp_label_61
        ret


        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16

        EXPORT ff_biweight_h264_pixels_8_neon





ff_biweight_h264_pixels_8_neon
        










        lsr             w8,  w5,  #31
        add             w7,  w7,  #1
        eor             w8,  w8,  w6,  lsr #30
        orr             w7,  w7,  #1
        dup             v18.8h,   w4
        lsl             w7,  w7,  w4
        not             v18.16b,  v18.16b
        dup             v16.8h,   w7
        mov             x7,  x0
        cbz             w8,  temp_label_62
        subs            w8,  w8,  #1
        beq            temp_label_63
        subs            w8,  w8,  #1
        beq            temp_label_64
        b               temp_label_65
temp_label_66
temp_label_62
        dup             v0.8b,  w5
        dup             v1.8b,  w6
        orr v2.16b, v16.16b, v16.16b
        orr v20.16b, v16.16b, v16.16b
temp_label_67
      subs            w3,  w3,  #2
        ld1             {v4.8b}, [x0], x2
        umlal           v2.8h,  v0.8b,  v4.8b
        ld1             {v5.8b}, [x1], x2
        umlal           v2.8h,  v1.8b,  v5.8b
        ld1             {v6.8b}, [x0], x2
        umlal           v20.8h, v0.8b,  v6.8b
        ld1             {v7.8b}, [x1], x2
        umlal           v20.8h, v1.8b,  v7.8b
        sshl            v2.8h,  v2.8h,  v18.8h
        sqxtun          v2.8b,  v2.8h
        sshl            v20.8h, v20.8h, v18.8h
        sqxtun          v4.8b,  v20.8h
        orr v20.16b, v16.16b, v16.16b
        st1             {v2.8b}, [x7], x2
        orr v2.16b, v16.16b, v16.16b
        st1             {v4.8b}, [x7], x2
        bne            temp_label_67
        ret
temp_label_68
temp_label_63
     neg             w5, w5
        dup             v0.8b,  w5
        dup             v1.8b,  w6
        orr v2.16b, v16.16b, v16.16b
        orr v20.16b, v16.16b, v16.16b
temp_label_69
      subs            w3,  w3,  #2
        ld1             {v4.8b}, [x0], x2
        umlsl           v2.8h,  v0.8b,  v4.8b
        ld1             {v5.8b}, [x1], x2
        umlal           v2.8h,  v1.8b,  v5.8b
        ld1             {v6.8b}, [x0], x2
        umlsl           v20.8h, v0.8b,  v6.8b
        ld1             {v7.8b}, [x1], x2
        umlal           v20.8h, v1.8b,  v7.8b
        sshl            v2.8h,  v2.8h,  v18.8h
        sqxtun          v2.8b,  v2.8h
        sshl            v20.8h, v20.8h, v18.8h
        sqxtun          v4.8b,  v20.8h
        orr v20.16b, v16.16b, v16.16b
        st1             {v2.8b}, [x7], x2
        orr v2.16b, v16.16b, v16.16b
        st1             {v4.8b}, [x7], x2
        bne            temp_label_69
        ret
temp_label_70
temp_label_64
     neg             w5, w5
        neg             w6, w6
        dup             v0.8b,  w5
        dup             v1.8b,  w6
        orr v2.16b, v16.16b, v16.16b
        orr v20.16b, v16.16b, v16.16b
temp_label_71
      subs            w3,  w3,  #2
        ld1             {v4.8b}, [x0], x2
        umlsl           v2.8h,  v0.8b,  v4.8b
        ld1             {v5.8b}, [x1], x2
        umlsl           v2.8h,  v1.8b,  v5.8b
        ld1             {v6.8b}, [x0], x2
        umlsl           v20.8h, v0.8b,  v6.8b
        ld1             {v7.8b}, [x1], x2
        umlsl           v20.8h, v1.8b,  v7.8b
        sshl            v2.8h,  v2.8h,  v18.8h
        sqxtun          v2.8b,  v2.8h
        sshl            v20.8h, v20.8h, v18.8h
        sqxtun          v4.8b,  v20.8h
        orr v20.16b, v16.16b, v16.16b
        st1             {v2.8b}, [x7], x2
        orr v2.16b, v16.16b, v16.16b
        st1             {v4.8b}, [x7], x2
        bne            temp_label_71
        ret
temp_label_72
temp_label_65
     neg             w6, w6
        dup             v0.8b,  w5
        dup             v1.8b,  w6
        orr v2.16b, v16.16b, v16.16b
        orr v20.16b, v16.16b, v16.16b
temp_label_73
      subs            w3,  w3,  #2
        ld1             {v4.8b}, [x0], x2
        umlal           v2.8h,  v0.8b,  v4.8b
        ld1             {v5.8b}, [x1], x2
        umlsl           v2.8h,  v1.8b,  v5.8b
        ld1             {v6.8b}, [x0], x2
        umlal           v20.8h, v0.8b,  v6.8b
        ld1             {v7.8b}, [x1], x2
        umlsl           v20.8h, v1.8b,  v7.8b
        sshl            v2.8h,  v2.8h,  v18.8h
        sqxtun          v2.8b,  v2.8h
        sshl            v20.8h, v20.8h, v18.8h
        sqxtun          v4.8b,  v20.8h
        orr v20.16b, v16.16b, v16.16b
        st1             {v2.8b}, [x7], x2
        orr v2.16b, v16.16b, v16.16b
        st1             {v4.8b}, [x7], x2
        bne            temp_label_73
        ret


        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16

        EXPORT ff_biweight_h264_pixels_4_neon





ff_biweight_h264_pixels_4_neon
        










        lsr             w8,  w5,  #31
        add             w7,  w7,  #1
        eor             w8,  w8,  w6,  lsr #30
        orr             w7,  w7,  #1
        dup             v18.8h,   w4
        lsl             w7,  w7,  w4
        not             v18.16b,  v18.16b
        dup             v16.8h,   w7
        mov             x7,  x0
        cbz             w8,  temp_label_74
        subs            w8,  w8,  #1
        beq            temp_label_75
        subs            w8,  w8,  #1
        beq            temp_label_76
        b               temp_label_77
temp_label_78
temp_label_74
        dup             v0.8b,  w5
        dup             v1.8b,  w6
        orr v2.16b, v16.16b, v16.16b
        orr v20.16b, v16.16b, v16.16b
temp_label_79
      subs            w3,  w3,  #4
        ld1             {v4.s}[0], [x0], x2
        ld1             {v4.s}[1], [x0], x2
        umlal           v2.8h,  v0.8b,  v4.8b
        ld1             {v5.s}[0], [x1], x2
        ld1             {v5.s}[1], [x1], x2
        umlal           v2.8h,  v1.8b,  v5.8b
        blt            temp_label_80
        ld1             {v6.s}[0], [x0], x2
        ld1             {v6.s}[1], [x0], x2
        umlal           v20.8h, v0.8b,  v6.8b
        ld1             {v7.s}[0], [x1], x2
        ld1             {v7.s}[1], [x1], x2
        umlal           v20.8h, v1.8b,  v7.8b
        sshl            v2.8h,  v2.8h,  v18.8h
        sqxtun          v2.8b,  v2.8h
        sshl            v20.8h, v20.8h, v18.8h
        sqxtun          v4.8b,  v20.8h
        orr v20.16b, v16.16b, v16.16b
        st1             {v2.s}[0], [x7], x2
        st1             {v2.s}[1], [x7], x2
        orr v2.16b, v16.16b, v16.16b
        st1             {v4.s}[0], [x7], x2
        st1             {v4.s}[1], [x7], x2
        bne            temp_label_79
        ret
temp_label_81
temp_label_80
      sshl            v2.8h,  v2.8h,  v18.8h
        sqxtun          v2.8b,  v2.8h
        st1             {v2.s}[0], [x7], x2
        st1             {v2.s}[1], [x7], x2
        ret
temp_label_82
temp_label_75
     neg             w5, w5
        dup             v0.8b,  w5
        dup             v1.8b,  w6
        orr v2.16b, v16.16b, v16.16b
        orr v20.16b, v16.16b, v16.16b
temp_label_83
      subs            w3,  w3,  #4
        ld1             {v4.s}[0], [x0], x2
        ld1             {v4.s}[1], [x0], x2
        umlsl           v2.8h,  v0.8b,  v4.8b
        ld1             {v5.s}[0], [x1], x2
        ld1             {v5.s}[1], [x1], x2
        umlal           v2.8h,  v1.8b,  v5.8b
        blt            temp_label_84
        ld1             {v6.s}[0], [x0], x2
        ld1             {v6.s}[1], [x0], x2
        umlsl           v20.8h, v0.8b,  v6.8b
        ld1             {v7.s}[0], [x1], x2
        ld1             {v7.s}[1], [x1], x2
        umlal           v20.8h, v1.8b,  v7.8b
        sshl            v2.8h,  v2.8h,  v18.8h
        sqxtun          v2.8b,  v2.8h
        sshl            v20.8h, v20.8h, v18.8h
        sqxtun          v4.8b,  v20.8h
        orr v20.16b, v16.16b, v16.16b
        st1             {v2.s}[0], [x7], x2
        st1             {v2.s}[1], [x7], x2
        orr v2.16b, v16.16b, v16.16b
        st1             {v4.s}[0], [x7], x2
        st1             {v4.s}[1], [x7], x2
        bne            temp_label_83
        ret
temp_label_85
temp_label_84
      sshl            v2.8h,  v2.8h,  v18.8h
        sqxtun          v2.8b,  v2.8h
        st1             {v2.s}[0], [x7], x2
        st1             {v2.s}[1], [x7], x2
        ret
temp_label_86
temp_label_76
     neg             w5, w5
        neg             w6, w6
        dup             v0.8b,  w5
        dup             v1.8b,  w6
        orr v2.16b, v16.16b, v16.16b
        orr v20.16b, v16.16b, v16.16b
temp_label_87
      subs            w3,  w3,  #4
        ld1             {v4.s}[0], [x0], x2
        ld1             {v4.s}[1], [x0], x2
        umlsl           v2.8h,  v0.8b,  v4.8b
        ld1             {v5.s}[0], [x1], x2
        ld1             {v5.s}[1], [x1], x2
        umlsl           v2.8h,  v1.8b,  v5.8b
        blt            temp_label_88
        ld1             {v6.s}[0], [x0], x2
        ld1             {v6.s}[1], [x0], x2
        umlsl           v20.8h, v0.8b,  v6.8b
        ld1             {v7.s}[0], [x1], x2
        ld1             {v7.s}[1], [x1], x2
        umlsl           v20.8h, v1.8b,  v7.8b
        sshl            v2.8h,  v2.8h,  v18.8h
        sqxtun          v2.8b,  v2.8h
        sshl            v20.8h, v20.8h, v18.8h
        sqxtun          v4.8b,  v20.8h
        orr v20.16b, v16.16b, v16.16b
        st1             {v2.s}[0], [x7], x2
        st1             {v2.s}[1], [x7], x2
        orr v2.16b, v16.16b, v16.16b
        st1             {v4.s}[0], [x7], x2
        st1             {v4.s}[1], [x7], x2
        bne            temp_label_87
        ret
temp_label_89
temp_label_88
      sshl            v2.8h,  v2.8h,  v18.8h
        sqxtun          v2.8b,  v2.8h
        st1             {v2.s}[0], [x7], x2
        st1             {v2.s}[1], [x7], x2
        ret
temp_label_90
temp_label_77
     neg             w6, w6
        dup             v0.8b,  w5
        dup             v1.8b,  w6
        orr v2.16b, v16.16b, v16.16b
        orr v20.16b, v16.16b, v16.16b
temp_label_91
      subs            w3,  w3,  #4
        ld1             {v4.s}[0], [x0], x2
        ld1             {v4.s}[1], [x0], x2
        umlal           v2.8h,  v0.8b,  v4.8b
        ld1             {v5.s}[0], [x1], x2
        ld1             {v5.s}[1], [x1], x2
        umlsl           v2.8h,  v1.8b,  v5.8b
        blt            temp_label_92
        ld1             {v6.s}[0], [x0], x2
        ld1             {v6.s}[1], [x0], x2
        umlal           v20.8h, v0.8b,  v6.8b
        ld1             {v7.s}[0], [x1], x2
        ld1             {v7.s}[1], [x1], x2
        umlsl           v20.8h, v1.8b,  v7.8b
        sshl            v2.8h,  v2.8h,  v18.8h
        sqxtun          v2.8b,  v2.8h
        sshl            v20.8h, v20.8h, v18.8h
        sqxtun          v4.8b,  v20.8h
        orr v20.16b, v16.16b, v16.16b
        st1             {v2.s}[0], [x7], x2
        st1             {v2.s}[1], [x7], x2
        orr v2.16b, v16.16b, v16.16b
        st1             {v4.s}[0], [x7], x2
        st1             {v4.s}[1], [x7], x2
        bne            temp_label_91
        ret
temp_label_93
temp_label_92
      sshl            v2.8h,  v2.8h,  v18.8h
        sqxtun          v2.8b,  v2.8h
        st1             {v2.s}[0], [x7], x2
        st1             {v2.s}[1], [x7], x2
        ret







        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16

        EXPORT ff_weight_h264_pixels_16_neon





ff_weight_h264_pixels_16_neon
        










        cmp             w3,  #1
        mov             w6,  #1
        lsl             w5,  w5,  w3
        dup             v16.8h,  w5
        mov             x5,  x0
        ble            temp_label_94
        sub             w6,  w6,  w3
        dup             v18.8h,  w6
        cmp             w4, #0
        blt            temp_label_95
        dup             v0.16b,  w4
temp_label_96
      subs            w2,  w2,  #2
        ld1             {v20.16b}, [x0], x1
        umull           v4.8h,   v0.8b,  v20.8b
        umull2          v6.8h,   v0.16b, v20.16b
        ld1             {v28.16b}, [x0], x1
        umull           v24.8h,  v0.8b,  v28.8b
        umull2          v26.8h,  v0.16b, v28.16b
        shadd            v4.8h,   v16.8h, v4.8h
        srshl           v4.8h,   v4.8h,  v18.8h
        shadd            v6.8h,   v16.8h, v6.8h
        srshl           v6.8h,   v6.8h,  v18.8h
        sqxtun          v4.8b,   v4.8h
        sqxtun2         v4.16b,  v6.8h
        shadd            v24.8h,  v16.8h, v24.8h
        srshl           v24.8h,  v24.8h, v18.8h
        shadd            v26.8h,  v16.8h, v26.8h
        srshl           v26.8h,  v26.8h, v18.8h
        sqxtun          v24.8b,  v24.8h
        sqxtun2         v24.16b, v26.8h
        st1             {v4.16b},  [x5], x1
        st1             {v24.16b}, [x5], x1
        bne            temp_label_96
        ret
temp_label_97
temp_label_95
     neg             w4,  w4
        dup             v0.16b,  w4
temp_label_98
      subs            w2,  w2,  #2
        ld1             {v20.16b}, [x0], x1
        umull           v4.8h,   v0.8b,  v20.8b
        umull2          v6.8h,   v0.16b, v20.16b
        ld1             {v28.16b}, [x0], x1
        umull           v24.8h,  v0.8b,  v28.8b
        umull2          v26.8h,  v0.16b, v28.16b
        shsub            v4.8h,   v16.8h, v4.8h
        srshl           v4.8h,   v4.8h,  v18.8h
        shsub            v6.8h,   v16.8h, v6.8h
        srshl           v6.8h,   v6.8h,  v18.8h
        sqxtun          v4.8b,   v4.8h
        sqxtun2         v4.16b,  v6.8h
        shsub            v24.8h,  v16.8h, v24.8h
        srshl           v24.8h,  v24.8h, v18.8h
        shsub            v26.8h,  v16.8h, v26.8h
        srshl           v26.8h,  v26.8h, v18.8h
        sqxtun          v24.8b,  v24.8h
        sqxtun2         v24.16b, v26.8h
        st1             {v4.16b},  [x5], x1
        st1             {v24.16b}, [x5], x1
        bne            temp_label_98
        ret
temp_label_99
temp_label_94
     neg             w6,  w3
        dup             v18.8h,  w6
        cmp             w4,  #0
        blt            temp_label_100
        dup             v0.16b,  w4
temp_label_101
      subs            w2,  w2,  #2
        ld1             {v20.16b}, [x0], x1
        umull           v4.8h,   v0.8b,  v20.8b
        umull2          v6.8h,   v0.16b, v20.16b
        ld1             {v28.16b}, [x0], x1
        umull           v24.8h,  v0.8b,  v28.8b
        umull2          v26.8h,  v0.16b, v28.16b
        add            v4.8h,   v16.8h, v4.8h
        srshl           v4.8h,   v4.8h,  v18.8h
        add            v6.8h,   v16.8h, v6.8h
        srshl           v6.8h,   v6.8h,  v18.8h
        sqxtun          v4.8b,   v4.8h
        sqxtun2         v4.16b,  v6.8h
        add            v24.8h,  v16.8h, v24.8h
        srshl           v24.8h,  v24.8h, v18.8h
        add            v26.8h,  v16.8h, v26.8h
        srshl           v26.8h,  v26.8h, v18.8h
        sqxtun          v24.8b,  v24.8h
        sqxtun2         v24.16b, v26.8h
        st1             {v4.16b},  [x5], x1
        st1             {v24.16b}, [x5], x1
        bne            temp_label_101
        ret
temp_label_102
temp_label_100
     neg             w4,  w4
        dup             v0.16b,  w4
temp_label_103
      subs            w2,  w2,  #2
        ld1             {v20.16b}, [x0], x1
        umull           v4.8h,   v0.8b,  v20.8b
        umull2          v6.8h,   v0.16b, v20.16b
        ld1             {v28.16b}, [x0], x1
        umull           v24.8h,  v0.8b,  v28.8b
        umull2          v26.8h,  v0.16b, v28.16b
        sub            v4.8h,   v16.8h, v4.8h
        srshl           v4.8h,   v4.8h,  v18.8h
        sub            v6.8h,   v16.8h, v6.8h
        srshl           v6.8h,   v6.8h,  v18.8h
        sqxtun          v4.8b,   v4.8h
        sqxtun2         v4.16b,  v6.8h
        sub            v24.8h,  v16.8h, v24.8h
        srshl           v24.8h,  v24.8h, v18.8h
        sub            v26.8h,  v16.8h, v26.8h
        srshl           v26.8h,  v26.8h, v18.8h
        sqxtun          v24.8b,  v24.8h
        sqxtun2         v24.16b, v26.8h
        st1             {v4.16b},  [x5], x1
        st1             {v24.16b}, [x5], x1
        bne            temp_label_103
        ret


        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16

        EXPORT ff_weight_h264_pixels_8_neon





ff_weight_h264_pixels_8_neon
        










        cmp             w3,  #1
        mov             w6,  #1
        lsl             w5,  w5,  w3
        dup             v16.8h,  w5
        mov             x5,  x0
        ble            temp_label_104
        sub             w6,  w6,  w3
        dup             v18.8h,  w6
        cmp             w4, #0
        blt            temp_label_105
        dup             v0.8b,  w4
temp_label_106
      subs            w2,  w2,  #2
        ld1             {v4.8b}, [x0], x1
        umull           v2.8h,  v0.8b,  v4.8b
        ld1             {v6.8b}, [x0], x1
        umull           v20.8h, v0.8b,  v6.8b
        shadd            v2.8h,  v16.8h,  v2.8h
        srshl           v2.8h,  v2.8h,  v18.8h
        sqxtun          v2.8b,  v2.8h
        shadd            v20.8h, v16.8h,  v20.8h
        srshl           v20.8h, v20.8h, v18.8h
        sqxtun          v4.8b,  v20.8h
        st1             {v2.8b}, [x5], x1
        st1             {v4.8b}, [x5], x1
        bne            temp_label_106
        ret
temp_label_107
temp_label_105
     neg             w4,  w4
        dup             v0.8b,  w4
temp_label_108
      subs            w2,  w2,  #2
        ld1             {v4.8b}, [x0], x1
        umull           v2.8h,  v0.8b,  v4.8b
        ld1             {v6.8b}, [x0], x1
        umull           v20.8h, v0.8b,  v6.8b
        shsub            v2.8h,  v16.8h,  v2.8h
        srshl           v2.8h,  v2.8h,  v18.8h
        sqxtun          v2.8b,  v2.8h
        shsub            v20.8h, v16.8h,  v20.8h
        srshl           v20.8h, v20.8h, v18.8h
        sqxtun          v4.8b,  v20.8h
        st1             {v2.8b}, [x5], x1
        st1             {v4.8b}, [x5], x1
        bne            temp_label_108
        ret
temp_label_109
temp_label_104
     neg             w6,  w3
        dup             v18.8h,  w6
        cmp             w4,  #0
        blt            temp_label_110
        dup             v0.8b,  w4
temp_label_111
      subs            w2,  w2,  #2
        ld1             {v4.8b}, [x0], x1
        umull           v2.8h,  v0.8b,  v4.8b
        ld1             {v6.8b}, [x0], x1
        umull           v20.8h, v0.8b,  v6.8b
        add            v2.8h,  v16.8h,  v2.8h
        srshl           v2.8h,  v2.8h,  v18.8h
        sqxtun          v2.8b,  v2.8h
        add            v20.8h, v16.8h,  v20.8h
        srshl           v20.8h, v20.8h, v18.8h
        sqxtun          v4.8b,  v20.8h
        st1             {v2.8b}, [x5], x1
        st1             {v4.8b}, [x5], x1
        bne            temp_label_111
        ret
temp_label_112
temp_label_110
     neg             w4,  w4
        dup             v0.8b,  w4
temp_label_113
      subs            w2,  w2,  #2
        ld1             {v4.8b}, [x0], x1
        umull           v2.8h,  v0.8b,  v4.8b
        ld1             {v6.8b}, [x0], x1
        umull           v20.8h, v0.8b,  v6.8b
        sub            v2.8h,  v16.8h,  v2.8h
        srshl           v2.8h,  v2.8h,  v18.8h
        sqxtun          v2.8b,  v2.8h
        sub            v20.8h, v16.8h,  v20.8h
        srshl           v20.8h, v20.8h, v18.8h
        sqxtun          v4.8b,  v20.8h
        st1             {v2.8b}, [x5], x1
        st1             {v4.8b}, [x5], x1
        bne            temp_label_113
        ret


        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16

        EXPORT ff_weight_h264_pixels_4_neon





ff_weight_h264_pixels_4_neon
        










        cmp             w3,  #1
        mov             w6,  #1
        lsl             w5,  w5,  w3
        dup             v16.8h,  w5
        mov             x5,  x0
        ble            temp_label_114
        sub             w6,  w6,  w3
        dup             v18.8h,  w6
        cmp             w4, #0
        blt            temp_label_115
        dup             v0.8b,  w4
temp_label_116
      subs            w2,  w2,  #4
        ld1             {v4.s}[0], [x0], x1
        ld1             {v4.s}[1], [x0], x1
        umull           v2.8h,  v0.8b,  v4.8b
        blt            temp_label_117
        ld1             {v6.s}[0], [x0], x1
        ld1             {v6.s}[1], [x0], x1
        umull           v20.8h, v0.8b,  v6.8b
        shadd            v2.8h,  v16.8h,  v2.8h
        srshl           v2.8h,  v2.8h,  v18.8h
        sqxtun          v2.8b,  v2.8h
        shadd            v20.8h, v16.8h,  v20.8h
        srshl           v20.8h, v20.8h, v18.8h
        sqxtun          v4.8b,  v20.8h
        st1             {v2.s}[0], [x5], x1
        st1             {v2.s}[1], [x5], x1
        st1             {v4.s}[0], [x5], x1
        st1             {v4.s}[1], [x5], x1
        bne            temp_label_116
        ret
temp_label_118
temp_label_117
      shadd            v2.8h,  v16.8h,  v2.8h
        srshl           v2.8h,  v2.8h,  v18.8h
        sqxtun          v2.8b,  v2.8h
        st1             {v2.s}[0], [x5], x1
        st1             {v2.s}[1], [x5], x1
        ret
temp_label_119
temp_label_115
     neg             w4,  w4
        dup             v0.8b,  w4
temp_label_120
      subs            w2,  w2,  #4
        ld1             {v4.s}[0], [x0], x1
        ld1             {v4.s}[1], [x0], x1
        umull           v2.8h,  v0.8b,  v4.8b
        blt            temp_label_121
        ld1             {v6.s}[0], [x0], x1
        ld1             {v6.s}[1], [x0], x1
        umull           v20.8h, v0.8b,  v6.8b
        shsub            v2.8h,  v16.8h,  v2.8h
        srshl           v2.8h,  v2.8h,  v18.8h
        sqxtun          v2.8b,  v2.8h
        shsub            v20.8h, v16.8h,  v20.8h
        srshl           v20.8h, v20.8h, v18.8h
        sqxtun          v4.8b,  v20.8h
        st1             {v2.s}[0], [x5], x1
        st1             {v2.s}[1], [x5], x1
        st1             {v4.s}[0], [x5], x1
        st1             {v4.s}[1], [x5], x1
        bne            temp_label_120
        ret
temp_label_122
temp_label_121
      shsub            v2.8h,  v16.8h,  v2.8h
        srshl           v2.8h,  v2.8h,  v18.8h
        sqxtun          v2.8b,  v2.8h
        st1             {v2.s}[0], [x5], x1
        st1             {v2.s}[1], [x5], x1
        ret
temp_label_123
temp_label_114
     neg             w6,  w3
        dup             v18.8h,  w6
        cmp             w4,  #0
        blt            temp_label_124
        dup             v0.8b,  w4
temp_label_125
      subs            w2,  w2,  #4
        ld1             {v4.s}[0], [x0], x1
        ld1             {v4.s}[1], [x0], x1
        umull           v2.8h,  v0.8b,  v4.8b
        blt            temp_label_126
        ld1             {v6.s}[0], [x0], x1
        ld1             {v6.s}[1], [x0], x1
        umull           v20.8h, v0.8b,  v6.8b
        add            v2.8h,  v16.8h,  v2.8h
        srshl           v2.8h,  v2.8h,  v18.8h
        sqxtun          v2.8b,  v2.8h
        add            v20.8h, v16.8h,  v20.8h
        srshl           v20.8h, v20.8h, v18.8h
        sqxtun          v4.8b,  v20.8h
        st1             {v2.s}[0], [x5], x1
        st1             {v2.s}[1], [x5], x1
        st1             {v4.s}[0], [x5], x1
        st1             {v4.s}[1], [x5], x1
        bne            temp_label_125
        ret
temp_label_127
temp_label_126
      add            v2.8h,  v16.8h,  v2.8h
        srshl           v2.8h,  v2.8h,  v18.8h
        sqxtun          v2.8b,  v2.8h
        st1             {v2.s}[0], [x5], x1
        st1             {v2.s}[1], [x5], x1
        ret
temp_label_128
temp_label_124
     neg             w4,  w4
        dup             v0.8b,  w4
temp_label_129
      subs            w2,  w2,  #4
        ld1             {v4.s}[0], [x0], x1
        ld1             {v4.s}[1], [x0], x1
        umull           v2.8h,  v0.8b,  v4.8b
        blt            temp_label_130
        ld1             {v6.s}[0], [x0], x1
        ld1             {v6.s}[1], [x0], x1
        umull           v20.8h, v0.8b,  v6.8b
        sub            v2.8h,  v16.8h,  v2.8h
        srshl           v2.8h,  v2.8h,  v18.8h
        sqxtun          v2.8b,  v2.8h
        sub            v20.8h, v16.8h,  v20.8h
        srshl           v20.8h, v20.8h, v18.8h
        sqxtun          v4.8b,  v20.8h
        st1             {v2.s}[0], [x5], x1
        st1             {v2.s}[1], [x5], x1
        st1             {v4.s}[0], [x5], x1
        st1             {v4.s}[1], [x5], x1
        bne            temp_label_129
        ret
temp_label_131
temp_label_130
      sub            v2.8h,  v16.8h,  v2.8h
        srshl           v2.8h,  v2.8h,  v18.8h
        sqxtun          v2.8b,  v2.8h
        st1             {v2.s}[0], [x5], x1
        st1             {v2.s}[1], [x5], x1
        ret






        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16

        EXPORT ff_h264_v_loop_filter_chroma_neon_10





ff_h264_v_loop_filter_chroma_neon_10
        










        cmp             w2,  #0
        ldr             w6,  [x4]
        ccmpne w3, #0, #0
        lsl             w2,  w2,  #2
        mov             v24.s[0], w6
        lsl             w3,  w3,  #2
        and             w8,  w6,  w6,  lsl #16
        beq            temp_label_132
        ands            w8,  w8,  w8,  lsl #8
        bge            temp_label_133
temp_label_134
temp_label_132

        ret
temp_label_135
temp_label_133


        mov             x10,  x0
        sub             x0,  x0,  x1, lsl #1
        ld1             {v18.8h}, [x0 ], x1
        ld1             {v0.8h},  [x10], x1
        ld1             {v16.8h}, [x0 ], x1
        ld1             {v2.8h},  [x10]

        dup             v22.8h,  w2               
        dup             v23.8h,  w3               
        ushll v24.8h, v24.8b, #0

        uabd            v26.8h,  v16.8h,  v0.8h   
        uabd            v28.8h,  v18.8h,  v16.8h  
        uabd            v30.8h,  v2.8h,   v0.8h   
        cmhi            v26.8h,  v22.8h,  v26.8h  
        cmhi            v28.8h,  v23.8h,  v28.8h  
        cmhi            v30.8h,  v23.8h,  v30.8h  

        and             v26.16b, v26.16b, v28.16b
        orr v4.16b, v0.16b, v0.16b
        sub             v4.8h,   v4.8h,   v16.8h
        and             v26.16b, v26.16b, v30.16b
        shl             v4.8h,   v4.8h,   #2
        umov             x8, v26.d[0]
        umov             x9, v26.d[1]
        sli             v24.8h,  v24.8h,  #8
        ushll v24.8h, v24.8b, #0
        add             v4.8h,   v4.8h,   v18.8h
        adds            x8,  x8,  x9
        shl             v24.8h,  v24.8h,  #2

        beq            temp_label_136

        movi v31.8h, #3, lsl #0
        uqsub           v24.8h,  v24.8h,  v31.8h
        sub             v4.8h,   v4.8h,   v2.8h
        srshr           v4.8h,   v4.8h,   #3
        smin            v4.8h,   v4.8h,   v24.8h
        neg             v25.8h,  v24.8h
        smax            v4.8h,   v4.8h,   v25.8h
        and             v4.16b,  v4.16b,  v26.16b
        add             v16.8h,  v16.8h,  v4.8h
        sub             v0.8h,   v0.8h,   v4.8h

        mvni            v4.8h,   #0xFC, lsl #8    
        movi v5.8h, #0, lsl #0
        smin            v0.8h,   v0.8h,   v4.8h
        smin            v16.8h,  v16.8h,  v4.8h
        smax            v0.8h,   v0.8h,   v5.8h
        smax            v16.8h,  v16.8h,  v5.8h

        sub             x0,  x10,  x1, lsl #1
        st1             {v16.8h}, [x0], x1
        st1             {v0.8h},  [x0], x1
temp_label_137
temp_label_136

        ret



        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16

        EXPORT ff_h264_h_loop_filter_chroma_neon_10





ff_h264_h_loop_filter_chroma_neon_10
        










        cmp             w2,  #0
        ldr             w6,  [x4]
        ccmpne w3, #0, #0
        lsl             w2,  w2,  #2
        mov             v24.s[0], w6
        lsl             w3,  w3,  #2
        and             w8,  w6,  w6,  lsl #16
        beq            temp_label_138
        ands            w8,  w8,  w8,  lsl #8
        bge            temp_label_139
temp_label_140
temp_label_138

        ret
temp_label_141
temp_label_139


        sub             x0,  x0,  #4 
h_loop_filter_chroma420_10
        add             x10,  x0,  x1,  lsl #2
        ld1             {v18.d}[0], [x0 ], x1
        ld1             {v18.d}[1], [x10], x1
        ld1             {v16.d}[0], [x0 ], x1
        ld1             {v16.d}[1], [x10], x1
        ld1             {v0.d}[0],  [x0 ], x1
        ld1             {v0.d}[1],  [x10], x1
        ld1             {v2.d}[0],  [x0 ], x1
        ld1             {v2.d}[1],  [x10], x1

        trn1            v28.8h,  v18.8h,  v16.8h
        trn2            v29.8h,  v18.8h,  v16.8h
        trn1            v30.8h,  v0.8h,  v2.8h
        trn2            v31.8h,  v0.8h,  v2.8h

        trn1            v18.4s,  v28.4s,  v30.4s
        trn2            v0.4s,  v28.4s,  v30.4s
        trn1            v16.4s,  v29.4s,  v31.4s
        trn2            v2.4s,  v29.4s,  v31.4s

        dup             v22.8h,  w2               
        dup             v23.8h,  w3               
        ushll v24.8h, v24.8b, #0

        uabd            v26.8h,  v16.8h,  v0.8h   
        uabd            v28.8h,  v18.8h,  v16.8h  
        uabd            v30.8h,  v2.8h,   v0.8h   
        cmhi            v26.8h,  v22.8h,  v26.8h  
        cmhi            v28.8h,  v23.8h,  v28.8h  
        cmhi            v30.8h,  v23.8h,  v30.8h  

        and             v26.16b, v26.16b, v28.16b
        orr v4.16b, v0.16b, v0.16b
        sub             v4.8h,   v4.8h,   v16.8h
        and             v26.16b, v26.16b, v30.16b
        shl             v4.8h,   v4.8h,   #2
        umov             x8, v26.d[0]
        umov             x9, v26.d[1]
        sli             v24.8h,  v24.8h,  #8
        ushll v24.8h, v24.8b, #0
        add             v4.8h,   v4.8h,   v18.8h
        adds            x8,  x8,  x9
        shl             v24.8h,  v24.8h,  #2

        beq            temp_label_142

        movi v31.8h, #3, lsl #0
        uqsub           v24.8h,  v24.8h,  v31.8h
        sub             v4.8h,   v4.8h,   v2.8h
        srshr           v4.8h,   v4.8h,   #3
        smin            v4.8h,   v4.8h,   v24.8h
        neg             v25.8h,  v24.8h
        smax            v4.8h,   v4.8h,   v25.8h
        and             v4.16b,  v4.16b,  v26.16b
        add             v16.8h,  v16.8h,  v4.8h
        sub             v0.8h,   v0.8h,   v4.8h

        mvni            v4.8h,   #0xFC, lsl #8    
        movi v5.8h, #0, lsl #0
        smin            v0.8h,   v0.8h,   v4.8h
        smin            v16.8h,  v16.8h,  v4.8h
        smax            v0.8h,   v0.8h,   v5.8h
        smax            v16.8h,  v16.8h,  v5.8h

        trn1            v28.8h,  v18.8h,  v16.8h
        trn2            v29.8h,  v18.8h,  v16.8h
        trn1            v30.8h,  v0.8h,  v2.8h
        trn2            v31.8h,  v0.8h,  v2.8h

        trn1            v18.4s,  v28.4s,  v30.4s
        trn2            v0.4s,  v28.4s,  v30.4s
        trn1            v16.4s,  v29.4s,  v31.4s
        trn2            v2.4s,  v29.4s,  v31.4s

        sub             x0,  x10,  x1, lsl #3
        st1             {v18.d}[0], [x0], x1
        st1             {v16.d}[0], [x0], x1
        st1             {v0.d}[0],  [x0], x1
        st1             {v2.d}[0],  [x0], x1
        st1             {v18.d}[1], [x0], x1
        st1             {v16.d}[1], [x0], x1
        st1             {v0.d}[1],  [x0], x1
        st1             {v2.d}[1],  [x0], x1
temp_label_143
temp_label_142

        ret



        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16

        EXPORT ff_h264_h_loop_filter_chroma422_neon_10





ff_h264_h_loop_filter_chroma422_neon_10
        










        cmp             w2,  #0
        ldr             w6,  [x4]
        ccmpne w3, #0, #0
        lsl             w2,  w2,  #2
        mov             v24.s[0], w6
        lsl             w3,  w3,  #2
        and             w8,  w6,  w6,  lsl #16
        beq            temp_label_144
        ands            w8,  w8,  w8,  lsl #8
        bge            temp_label_145
temp_label_146
temp_label_144

        ret
temp_label_147
temp_label_145

        add             x5,  x0,  x1
        sub             x0,  x0,  #4
        add             x1,  x1,  x1
        mov             x7,  x30
        bl              h_loop_filter_chroma420_10
        mov             x30, x7
        sub             x0,  x5,  #4
        mov             v24.s[0], w6
        b               h_loop_filter_chroma420_10




        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16

        EXPORT ff_h264_v_loop_filter_chroma_intra_neon_10





ff_h264_v_loop_filter_chroma_intra_neon_10
        










        orr             w4,  w2,  w3
        cbnz            w4,  temp_label_148
        ret
temp_label_149
temp_label_148

        lsl             w2,  w2,  #2
        lsl             w3,  w3,  #2
        dup             v30.8h,   w2              
        dup             v31.8h,   w3              
        mov             x9,  x0
        sub             x0,  x0,  x1, lsl #1
        ld1             {v18.8h}, [x0], x1
        ld1             {v17.8h}, [x9], x1
        ld1             {v16.8h}, [x0], x1
        ld1             {v19.8h}, [x9]

        uabd            v26.8h,  v16.8h,  v17.8h  
        uabd            v27.8h,  v18.8h,  v16.8h  
        uabd            v28.8h,  v19.8h,  v17.8h  
        cmhi            v26.8h,  v30.8h,  v26.8h  
        cmhi            v27.8h,  v31.8h,  v27.8h  
        cmhi            v28.8h,  v31.8h,  v28.8h  
        and             v26.16b, v26.16b, v27.16b
        and             v26.16b, v26.16b, v28.16b
        umov             x2, v26.d[0]
        umov             x3, v26.d[1]

        shl             v4.8h,  v18.8h,  #1
        shl             v6.8h,  v19.8h,  #1

        adds            x2,  x2,  x3
        beq            temp_label_150

        add             v20.8h,  v16.8h,  v19.8h
        add             v22.8h,  v17.8h,  v18.8h
        add             v20.8h,  v20.8h,  v4.8h
        add             v22.8h,  v22.8h,  v6.8h
        urshr           v24.8h,  v20.8h,  #2
        urshr           v25.8h,  v22.8h,  #2
        bit             v16.16b, v24.16b, v26.16b
        bit             v17.16b, v25.16b, v26.16b

        sub             x0,  x9,  x1, lsl #1
        st1             {v16.8h}, [x0], x1
        st1             {v17.8h}, [x0], x1

temp_label_151
temp_label_150

        ret



        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16

        EXPORT ff_h264_h_loop_filter_chroma_mbaff_intra_neon_10





ff_h264_h_loop_filter_chroma_mbaff_intra_neon_10
        










        orr             w4,  w2,  w3
        cbnz            w4,  temp_label_152
        ret
temp_label_153
temp_label_152

        lsl             w2,  w2,  #2
        lsl             w3,  w3,  #2
        dup             v30.8h,   w2              
        dup             v31.8h,   w3              

        sub             x4,  x0,  #4
        sub             x0,  x0,  #2
        add             x9,  x4,  x1, lsl #1
        ld1             {v18.8h}, [x4], x1
        ld1             {v17.8h}, [x9], x1
        ld1             {v16.8h}, [x4], x1
        ld1             {v19.8h}, [x9], x1

        trn1            v26.8h,  v18.8h,  v16.8h
        trn2            v27.8h,  v18.8h,  v16.8h
        trn1            v28.8h,  v17.8h,  v19.8h
        trn2            v29.8h,  v17.8h,  v19.8h

        trn1            v18.4s,  v26.4s,  v28.4s
        trn2            v17.4s,  v26.4s,  v28.4s
        trn1            v16.4s,  v27.4s,  v29.4s
        trn2            v19.4s,  v27.4s,  v29.4s

        uabd            v26.8h,  v16.8h,  v17.8h  
        uabd            v27.8h,  v18.8h,  v16.8h  
        uabd            v28.8h,  v19.8h,  v17.8h  
        cmhi            v26.8h,  v30.8h,  v26.8h  
        cmhi            v27.8h,  v31.8h,  v27.8h  
        cmhi            v28.8h,  v31.8h,  v28.8h  
        and             v26.16b, v26.16b, v27.16b
        and             v26.16b, v26.16b, v28.16b
        umov             x2, v26.d[0]
        umov             x3, v26.d[1]

        shl             v4.8h,  v18.8h,  #1
        shl             v6.8h,  v19.8h,  #1

        adds            x2,  x2,  x3
        beq            temp_label_154

        add             v20.8h,  v16.8h,  v19.8h
        add             v22.8h,  v17.8h,  v18.8h
        add             v20.8h,  v20.8h,  v4.8h
        add             v22.8h,  v22.8h,  v6.8h
        urshr           v24.8h,  v20.8h,  #2
        urshr           v25.8h,  v22.8h,  #2
        bit             v16.16b, v24.16b, v26.16b
        bit             v17.16b, v25.16b, v26.16b

        st2             {v16.h,v17.h}[0], [x0], x1
        st2             {v16.h,v17.h}[1], [x0], x1
        st2             {v16.h,v17.h}[2], [x0], x1
        st2             {v16.h,v17.h}[3], [x0], x1

temp_label_155
temp_label_154

        ret



        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16

        EXPORT ff_h264_h_loop_filter_chroma_intra_neon_10





ff_h264_h_loop_filter_chroma_intra_neon_10
        










        orr             w4,  w2,  w3
        cbnz            w4,  temp_label_156
        ret
temp_label_157
temp_label_156

        lsl             w2,  w2,  #2
        lsl             w3,  w3,  #2
        dup             v30.8h,   w2              
        dup             v31.8h,   w3              
        sub             x4,  x0,  #4
        sub             x0,  x0,  #2
h_loop_filter_chroma420_intra_10
        add             x9,  x4,  x1, lsl #2
        ld1             {v18.4h},   [x4], x1
        ld1             {v18.d}[1], [x9], x1
        ld1             {v16.4h},   [x4], x1
        ld1             {v16.d}[1], [x9], x1
        ld1             {v17.4h},   [x4], x1
        ld1             {v17.d}[1], [x9], x1
        ld1             {v19.4h},   [x4], x1
        ld1             {v19.d}[1], [x9], x1

        trn1            v26.8h,  v18.8h,  v16.8h
        trn2            v27.8h,  v18.8h,  v16.8h
        trn1            v28.8h,  v17.8h,  v19.8h
        trn2            v29.8h,  v17.8h,  v19.8h

        trn1            v18.4s,  v26.4s,  v28.4s
        trn2            v17.4s,  v26.4s,  v28.4s
        trn1            v16.4s,  v27.4s,  v29.4s
        trn2            v19.4s,  v27.4s,  v29.4s

        uabd            v26.8h,  v16.8h,  v17.8h  
        uabd            v27.8h,  v18.8h,  v16.8h  
        uabd            v28.8h,  v19.8h,  v17.8h  
        cmhi            v26.8h,  v30.8h,  v26.8h  
        cmhi            v27.8h,  v31.8h,  v27.8h  
        cmhi            v28.8h,  v31.8h,  v28.8h  
        and             v26.16b, v26.16b, v27.16b
        and             v26.16b, v26.16b, v28.16b
        umov             x2, v26.d[0]
        umov             x3, v26.d[1]

        shl             v4.8h,  v18.8h,  #1
        shl             v6.8h,  v19.8h,  #1

        adds            x2,  x2,  x3
        beq            temp_label_158

        add             v20.8h,  v16.8h,  v19.8h
        add             v22.8h,  v17.8h,  v18.8h
        add             v20.8h,  v20.8h,  v4.8h
        add             v22.8h,  v22.8h,  v6.8h
        urshr           v24.8h,  v20.8h,  #2
        urshr           v25.8h,  v22.8h,  #2
        bit             v16.16b, v24.16b, v26.16b
        bit             v17.16b, v25.16b, v26.16b

        st2             {v16.h,v17.h}[0], [x0], x1
        st2             {v16.h,v17.h}[1], [x0], x1
        st2             {v16.h,v17.h}[2], [x0], x1
        st2             {v16.h,v17.h}[3], [x0], x1
        st2             {v16.h,v17.h}[4], [x0], x1
        st2             {v16.h,v17.h}[5], [x0], x1
        st2             {v16.h,v17.h}[6], [x0], x1
        st2             {v16.h,v17.h}[7], [x0], x1

temp_label_159
temp_label_158

        ret



        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16

        EXPORT ff_h264_h_loop_filter_chroma422_intra_neon_10





ff_h264_h_loop_filter_chroma422_intra_neon_10
        










        orr             w4,  w2,  w3
        cbnz            w4,  temp_label_160
        ret
temp_label_161
temp_label_160

        lsl             w2,  w2,  #2
        lsl             w3,  w3,  #2
        dup             v30.8h,   w2              
        dup             v31.8h,   w3              
        sub             x4,  x0,  #4
        add             x5,  x0,  x1, lsl #3
        sub             x0,  x0,  #2
        mov             x7,  x30
        bl              h_loop_filter_chroma420_intra_10
        mov             x4,  x9
        sub             x0,  x5,  #2
        mov             x30, x7
        b               h_loop_filter_chroma420_intra_10




	END
