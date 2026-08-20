	AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN





















































































































































































































































































































































































































































































































































































































































































































































































































































































        ;.arch           armv8.6-a+crc


































	;.arch_extension nodotprod
	;.arch_extension noi8mm
	;.arch_extension nosve
	;.arch_extension nosve2





































































































































































        AREA |.rdata|, DATA, READONLY, ALIGN=5





        ALIGN 16
subadd
        dcfs -1.0,  1.0, -1.0,  1.0















        AREA |.rdata|, DATA, READONLY, ALIGN=5





        ALIGN 16
tab_8pt
        dcfs 0.707106781186547524401, -0.707106781186547524401, -0.707106781186547524401,  0.707106781186547524401












        AREA |.rdata|, DATA, READONLY, ALIGN=5





        ALIGN 16
tab_16pt
        dcfs   -0.92387950420379638671875,    0.92387950420379638671875,   -0.3826834261417388916015625,    0.3826834261417388916015625    
        dcfs    0.3826834261417388916015625,    0.3826834261417388916015625,    0.92387950420379638671875,    0.92387950420379638671875
        dcfs        1.0,        1.0,  0.707106781186547524401,  0.707106781186547524401





        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16

        EXPORT ff_tx_fft2_float_neon





ff_tx_fft2_float_neon
        










        ld2r            { v0.2d, v1.2d }, [x2]

        fneg            v2.2s, v1.2s
        mov             v2.d[1], v1.d[0]

        fsub            v2.4s, v0.4s, v2.4s

        st1             { v2.4s }, [x1]
        ret




        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16

        EXPORT ff_tx_fft4_fwd_float_neon





ff_tx_fft4_fwd_float_neon
        










        ld1             {v0.4s, v1.4s}, [x2]


        fadd            v16.4s, v0.4s, v1.4s         
        fsub            v0.4s, v0.4s, v1.4s      

        rev64           v18.4s, v0.4s

        zip2            v1.2d, v16.2d, v0.2d
        zip1            v17.2d, v16.2d, v0.2d

        mov             v1.d[1], v18.d[1]

        fadd            v0.4s, v17.4s, v1.4s         
        fsub            v16.4s,    v17.4s, v1.4s         

        orr v1.16b, v16.16b, v16.16b
        mov             v1.s[3], v0.s[3]
        mov             v0.s[3], v16.s[3]


        st1             { v0.4s, v1.4s }, [x1]
        ret


        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16

        EXPORT ff_tx_fft4_inv_float_neon





ff_tx_fft4_inv_float_neon
        










        ld1             {v0.4s, v1.4s}, [x2]

        mov             v2.d[0], v0.d[1]
        mov             v0.d[1], v1.d[1]
        mov             v1.d[1], v2.d[0]

        fadd            v16.4s, v0.4s, v1.4s         
        fsub            v0.4s, v0.4s, v1.4s      

        rev64           v18.4s, v0.4s

        zip2            v1.2d, v16.2d, v0.2d
        zip1            v17.2d, v16.2d, v0.2d

        mov             v1.d[1], v18.d[1]

        fadd            v0.4s, v17.4s, v1.4s         
        fsub            v16.4s,    v17.4s, v1.4s         

        orr v1.16b, v16.16b, v16.16b
        mov             v1.s[3], v0.s[3]
        mov             v0.s[3], v16.s[3]


        st1             { v0.4s, v1.4s }, [x1]
        ret




        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16

        EXPORT ff_tx_fft8_float_neon





ff_tx_fft8_float_neon
        










        ldr             x4, [x0, #8]
        ldp             w10, w11, [x4, #0 ]
        ldp             w12, w13, [x4, #8 ]
        ldp             w14, w15, [x4, #16]
        ldp             w16, w17, [x4, #24]

        add             x4, x4, #32

        ldr             d0, [x2, x10, lsl #3]
        add             x11, x2, x11, lsl #3
        ldr             d1, [x2, x12, lsl #3]
        add             x13, x2, x13, lsl #3
        ldr             d2, [x2, x14, lsl #3]
        add             x15, x2, x15, lsl #3
        ldr             d3, [x2, x16, lsl #3]
        add             x17, x2, x17, lsl #3

        ld1             { v0.d }[1], [x11]
        ld1             { v1.d }[1], [x13]
        ld1             { v2.d }[1], [x15]
        ld1             { v3.d }[1], [x17]











        adrp            x5, subadd
        add             x5, x5, subadd










        ld1             { v31.4s }, [x5]











        adrp            x5, tab_8pt
        add             x5, x5, tab_8pt











        fsub            v17.4s, v1.4s, v3.4s             
        fadd            v3.4s, v1.4s, v3.4s             
        fsub            v16.4s, v0.4s, v2.4s             
        fadd            v2.4s, v0.4s, v2.4s             

        ld1             { v21.4s }, [x5]

        ext8             v20.16b, v3.16b, v3.16b, #12
        rev64           v20.4s,  v20.4s

        ext8             v18.16b, v2.16b, v20.16b, #8      
        mov             v2.d[1], v20.d[1]                    

        fsub            v1.4s, v2.4s, v18.4s             
        fadd            v18.4s, v2.4s, v18.4s             

        rev64           v22.4s, v31.4s                           
        dup             v2.2d, v16.d[0]                      
        dup             v3.2d, v16.d[1]                      

        rev64           v20.4s, v1.4s                        
        rev64           v3.4s, v3.4s                        

        ext8             v22.16b, v31.16b, v22.16b, #8         
        zip1            v19.2d, v18.2d, v1.2d             
        zip2            v18.2d, v18.2d, v20.2d             

        fadd            v0.4s, v19.4s, v18.4s             
        fsub            v1.4s, v19.4s, v18.4s             

        fmul            v17.4s, v17.4s, v21.4s             
        fmls            v2.4s, v3.4s, v22.4s             

        rev64           v20.4s, v17.4s                        
        fmla            v17.4s, v20.4s, v31.4s                

        rev64           v20.4s, v17.4s                        
        ext8             v20.16b, v20.16b, v20.16b, #8      

        fmla            v20.4s, v17.4s, v31.4s                

        fadd            v3.4s, v2.4s, v20.4s             
        fsub            v2.4s, v2.4s, v20.4s             

        zip1            v16.2d, v0.2d, v2.2d
        zip2            v17.2d, v0.2d, v2.2d
        zip1            v18.2d, v1.2d, v3.2d
        zip2            v19.2d, v1.2d, v3.2d
        st1             { v16.4s, v17.4s, v18.4s, v19.4s }, [x1]

        ret


        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16

        EXPORT ff_tx_fft8_ns_float_neon





ff_tx_fft8_ns_float_neon
        










        ld1             { v0.4s, v1.4s, v2.4s, v3.4s }, [x2], #64











        adrp            x5, subadd
        add             x5, x5, subadd










        ld1             { v31.4s }, [x5]











        adrp            x5, tab_8pt
        add             x5, x5, tab_8pt











        fsub            v17.4s, v1.4s, v3.4s             
        fadd            v3.4s, v1.4s, v3.4s             
        fsub            v16.4s, v0.4s, v2.4s             
        fadd            v2.4s, v0.4s, v2.4s             

        ld1             { v21.4s }, [x5]

        ext8             v20.16b, v3.16b, v3.16b, #12
        rev64           v20.4s,  v20.4s

        ext8             v18.16b, v2.16b, v20.16b, #8      
        mov             v2.d[1], v20.d[1]                    

        fsub            v1.4s, v2.4s, v18.4s             
        fadd            v18.4s, v2.4s, v18.4s             

        rev64           v22.4s, v31.4s                           
        dup             v2.2d, v16.d[0]                      
        dup             v3.2d, v16.d[1]                      

        rev64           v20.4s, v1.4s                        
        rev64           v3.4s, v3.4s                        

        ext8             v22.16b, v31.16b, v22.16b, #8         
        zip1            v19.2d, v18.2d, v1.2d             
        zip2            v18.2d, v18.2d, v20.2d             

        fadd            v0.4s, v19.4s, v18.4s             
        fsub            v1.4s, v19.4s, v18.4s             

        fmul            v17.4s, v17.4s, v21.4s             
        fmls            v2.4s, v3.4s, v22.4s             

        rev64           v20.4s, v17.4s                        
        fmla            v17.4s, v20.4s, v31.4s                

        rev64           v20.4s, v17.4s                        
        ext8             v20.16b, v20.16b, v20.16b, #8      

        fmla            v20.4s, v17.4s, v31.4s                

        fadd            v3.4s, v2.4s, v20.4s             
        fsub            v2.4s, v2.4s, v20.4s             

        zip1            v16.2d, v0.2d, v2.2d
        zip2            v17.2d, v0.2d, v2.2d
        zip1            v18.2d, v1.2d, v3.2d
        zip2            v19.2d, v1.2d, v3.2d
        st1             { v16.4s, v17.4s, v18.4s, v19.4s }, [x1]

        ret




        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16

        EXPORT ff_tx_fft16_float_neon





ff_tx_fft16_float_neon
        










        ldr             x4, [x0, #8]
        ldp             w10, w11, [x4, #0 ]
        ldp             w12, w13, [x4, #8 ]
        ldp             w14, w15, [x4, #16]
        ldp             w16, w17, [x4, #24]

        add             x4, x4, #32

        ldr             d0, [x2, x10, lsl #3]
        add             x11, x2, x11, lsl #3
        ldr             d1, [x2, x12, lsl #3]
        add             x13, x2, x13, lsl #3
        ldr             d2, [x2, x14, lsl #3]
        add             x15, x2, x15, lsl #3
        ldr             d3, [x2, x16, lsl #3]
        add             x17, x2, x17, lsl #3

        ld1             { v0.d }[1], [x11]
        ld1             { v1.d }[1], [x13]
        ld1             { v2.d }[1], [x15]
        ld1             { v3.d }[1], [x17]
        ldp             w10, w11, [x4, #0 ]
        ldp             w12, w13, [x4, #8 ]
        ldp             w14, w15, [x4, #16]
        ldp             w16, w17, [x4, #24]

        add             x4, x4, #32

        ldr             d4, [x2, x10, lsl #3]
        add             x11, x2, x11, lsl #3
        ldr             d5, [x2, x12, lsl #3]
        add             x13, x2, x13, lsl #3
        ldr             d6, [x2, x14, lsl #3]
        add             x15, x2, x15, lsl #3
        ldr             d7, [x2, x16, lsl #3]
        add             x17, x2, x17, lsl #3

        ld1             { v4.d }[1], [x11]
        ld1             { v5.d }[1], [x13]
        ld1             { v6.d }[1], [x15]
        ld1             { v7.d }[1], [x17]











        adrp            x5, subadd
        add             x5, x5, subadd










        ld1             { v31.4s }, [x5]












        adrp            x5, tab_8pt
        add             x5, x5, tab_8pt











        fsub            v17.4s, v1.4s, v3.4s             
        fadd            v3.4s, v1.4s, v3.4s             
        fsub            v16.4s, v0.4s, v2.4s             
        fadd            v2.4s, v0.4s, v2.4s             

        ld1             { v21.4s }, [x5]

        ext8             v20.16b, v3.16b, v3.16b, #12
        rev64           v20.4s,  v20.4s

        ext8             v18.16b, v2.16b, v20.16b, #8      
        mov             v2.d[1], v20.d[1]                    

        fsub            v1.4s, v2.4s, v18.4s             
        fadd            v18.4s, v2.4s, v18.4s             

        rev64           v22.4s, v31.4s                           
        dup             v2.2d, v16.d[0]                      
        dup             v3.2d, v16.d[1]                      

        rev64           v20.4s, v1.4s                        
        rev64           v3.4s, v3.4s                        

        ext8             v22.16b, v31.16b, v22.16b, #8         
        zip1            v19.2d, v18.2d, v1.2d             
        zip2            v18.2d, v18.2d, v20.2d             

        fadd            v0.4s, v19.4s, v18.4s             
        fsub            v1.4s, v19.4s, v18.4s             

        fmul            v17.4s, v17.4s, v21.4s             
        fmls            v2.4s, v3.4s, v22.4s             

        rev64           v20.4s, v17.4s                        
        fmla            v17.4s, v20.4s, v31.4s                

        rev64           v20.4s, v17.4s                        
        ext8             v20.16b, v20.16b, v20.16b, #8      

        fmla            v20.4s, v17.4s, v31.4s                

        fadd            v3.4s, v2.4s, v20.4s             
        fsub            v2.4s, v2.4s, v20.4s             

        fadd            v16.4s, v4.4s, v5.4s                     
        fadd            v18.4s, v6.4s, v7.4s                     
        fsub            v4.4s, v4.4s, v5.4s                     
        fsub            v6.4s, v6.4s, v7.4s                     

        rev64           v20.4s, v4.4s
        rev64           v21.4s, v6.4s

        zip2            v5.2d, v16.2d, v20.2d                     
        zip2            v7.2d, v18.2d, v21.2d                     

        fneg            v19.4s, v20.4s
        fneg            v20.4s, v21.4s

        mov             v5.s[3], v19.s[3]
        mov             v7.s[3], v20.s[3]

        zip1            v17.2d, v16.2d, v4.2d                     
        zip1            v19.2d, v18.2d, v6.2d                     

        fsub            v20.4s, v17.4s, v5.4s                     
        fadd            v21.4s, v17.4s, v5.4s                     
        fsub            v18.4s, v19.4s, v7.4s                     
        fadd            v19.4s, v19.4s, v7.4s                     

        zip1            v4.2d, v21.2d, v20.2d                     
        zip2            v5.2d, v21.2d, v20.2d                     
        zip1            v6.2d, v19.2d, v18.2d                     
        zip2            v7.2d, v19.2d, v18.2d                     











        adrp            x5, tab_16pt
        add             x5, x5, tab_16pt











        rev64           v16.4s, v4.4s                     
        rev64           v17.4s, v6.4s                     

        ins             v16.d[0], xzr
        ins             v17.d[0], xzr

        ld1             { v20.4s, v21.4s, v22.4s }, [x5]
        

        fmla            v6.4s, v17.4s, v31.4s             
        fmls            v4.4s, v16.4s, v31.4s             

        fmul            v18.4s, v5.4s, v20.4s
        fmul            v19.4s, v7.4s, v20.4s

        rev64           v7.4s, v7.4s
        rev64           v5.4s, v5.4s

        fmla            v19.4s, v7.4s, v21.4s          
        fmls            v18.4s, v5.4s, v21.4s          

        fmul            v17.4s, v6.4s, v22.4s          
        fmul            v16.4s, v4.4s, v22.4s          

        orr v5.16b, v19.16b, v19.16b
        orr v6.16b, v17.16b, v17.16b

        fsub            v19.4s, v19.4s, v18.4s          
        fsub            v17.4s, v17.4s, v16.4s          

        fadd            v18.4s, v18.4s, v5.4s          
        rev64           v19.4s, v19.4s
        fadd            v16.4s, v16.4s, v6.4s          
        rev64           v17.4s, v17.4s

        fmul            v18.4s, v18.4s, v31.4s
        fmul            v17.4s, v17.4s, v31.4s

        fadd            v7.4s, v3.4s, v19.4s
        fsub            v6.4s, v3.4s, v19.4s
        fsub            v5.4s, v2.4s, v18.4s
        fadd            v4.4s, v2.4s, v18.4s

        fsub            v2.4s, v0.4s, v16.4s
        fadd            v0.4s, v0.4s, v16.4s
        fsub            v3.4s, v1.4s, v17.4s
        fadd            v1.4s, v1.4s, v17.4s

        zip1            v20.2d, v0.2d, v4.2d
        zip2            v21.2d, v0.2d, v4.2d
        zip1            v22.2d, v1.2d, v6.2d
        zip2            v23.2d, v1.2d, v6.2d
        st1             { v20.4s, v21.4s, v22.4s, v23.4s }, [x1], #64

        zip1            v24.2d, v2.2d, v5.2d
        zip2            v25.2d, v2.2d, v5.2d
        zip1            v26.2d, v3.2d, v7.2d
        zip2            v27.2d, v3.2d, v7.2d
        st1             { v24.4s, v25.4s, v26.4s, v27.4s }, [x1]

        ret


        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16

        EXPORT ff_tx_fft16_ns_float_neon





ff_tx_fft16_ns_float_neon
        










        ld1             { v0.4s, v1.4s, v2.4s, v3.4s }, [x2], #64
        ld1             { v4.4s, v5.4s, v6.4s, v7.4s }, [x2], #64











        adrp            x5, subadd
        add             x5, x5, subadd










        ld1             { v31.4s }, [x5]












        adrp            x5, tab_8pt
        add             x5, x5, tab_8pt











        fsub            v17.4s, v1.4s, v3.4s             
        fadd            v3.4s, v1.4s, v3.4s             
        fsub            v16.4s, v0.4s, v2.4s             
        fadd            v2.4s, v0.4s, v2.4s             

        ld1             { v21.4s }, [x5]

        ext8             v20.16b, v3.16b, v3.16b, #12
        rev64           v20.4s,  v20.4s

        ext8             v18.16b, v2.16b, v20.16b, #8      
        mov             v2.d[1], v20.d[1]                    

        fsub            v1.4s, v2.4s, v18.4s             
        fadd            v18.4s, v2.4s, v18.4s             

        rev64           v22.4s, v31.4s                           
        dup             v2.2d, v16.d[0]                      
        dup             v3.2d, v16.d[1]                      

        rev64           v20.4s, v1.4s                        
        rev64           v3.4s, v3.4s                        

        ext8             v22.16b, v31.16b, v22.16b, #8         
        zip1            v19.2d, v18.2d, v1.2d             
        zip2            v18.2d, v18.2d, v20.2d             

        fadd            v0.4s, v19.4s, v18.4s             
        fsub            v1.4s, v19.4s, v18.4s             

        fmul            v17.4s, v17.4s, v21.4s             
        fmls            v2.4s, v3.4s, v22.4s             

        rev64           v20.4s, v17.4s                        
        fmla            v17.4s, v20.4s, v31.4s                

        rev64           v20.4s, v17.4s                        
        ext8             v20.16b, v20.16b, v20.16b, #8      

        fmla            v20.4s, v17.4s, v31.4s                

        fadd            v3.4s, v2.4s, v20.4s             
        fsub            v2.4s, v2.4s, v20.4s             

        fadd            v16.4s, v4.4s, v5.4s                     
        fadd            v18.4s, v6.4s, v7.4s                     
        fsub            v4.4s, v4.4s, v5.4s                     
        fsub            v6.4s, v6.4s, v7.4s                     

        rev64           v20.4s, v4.4s
        rev64           v21.4s, v6.4s

        zip2            v5.2d, v16.2d, v20.2d                     
        zip2            v7.2d, v18.2d, v21.2d                     

        fneg            v19.4s, v20.4s
        fneg            v20.4s, v21.4s

        mov             v5.s[3], v19.s[3]
        mov             v7.s[3], v20.s[3]

        zip1            v17.2d, v16.2d, v4.2d                     
        zip1            v19.2d, v18.2d, v6.2d                     

        fsub            v20.4s, v17.4s, v5.4s                     
        fadd            v21.4s, v17.4s, v5.4s                     
        fsub            v18.4s, v19.4s, v7.4s                     
        fadd            v19.4s, v19.4s, v7.4s                     

        zip1            v4.2d, v21.2d, v20.2d                     
        zip2            v5.2d, v21.2d, v20.2d                     
        zip1            v6.2d, v19.2d, v18.2d                     
        zip2            v7.2d, v19.2d, v18.2d                     











        adrp            x5, tab_16pt
        add             x5, x5, tab_16pt











        rev64           v16.4s, v4.4s                     
        rev64           v17.4s, v6.4s                     

        ins             v16.d[0], xzr
        ins             v17.d[0], xzr

        ld1             { v20.4s, v21.4s, v22.4s }, [x5]
        

        fmla            v6.4s, v17.4s, v31.4s             
        fmls            v4.4s, v16.4s, v31.4s             

        fmul            v18.4s, v5.4s, v20.4s
        fmul            v19.4s, v7.4s, v20.4s

        rev64           v7.4s, v7.4s
        rev64           v5.4s, v5.4s

        fmla            v19.4s, v7.4s, v21.4s          
        fmls            v18.4s, v5.4s, v21.4s          

        fmul            v17.4s, v6.4s, v22.4s          
        fmul            v16.4s, v4.4s, v22.4s          

        orr v5.16b, v19.16b, v19.16b
        orr v6.16b, v17.16b, v17.16b

        fsub            v19.4s, v19.4s, v18.4s          
        fsub            v17.4s, v17.4s, v16.4s          

        fadd            v18.4s, v18.4s, v5.4s          
        rev64           v19.4s, v19.4s
        fadd            v16.4s, v16.4s, v6.4s          
        rev64           v17.4s, v17.4s

        fmul            v18.4s, v18.4s, v31.4s
        fmul            v17.4s, v17.4s, v31.4s

        fadd            v7.4s, v3.4s, v19.4s
        fsub            v6.4s, v3.4s, v19.4s
        fsub            v5.4s, v2.4s, v18.4s
        fadd            v4.4s, v2.4s, v18.4s

        fsub            v2.4s, v0.4s, v16.4s
        fadd            v0.4s, v0.4s, v16.4s
        fsub            v3.4s, v1.4s, v17.4s
        fadd            v1.4s, v1.4s, v17.4s

        zip1            v20.2d, v0.2d, v4.2d
        zip2            v21.2d, v0.2d, v4.2d
        zip1            v22.2d, v1.2d, v6.2d
        zip2            v23.2d, v1.2d, v6.2d
        st1             { v20.4s, v21.4s, v22.4s, v23.4s }, [x1], #64

        zip1            v24.2d, v2.2d, v5.2d
        zip2            v25.2d, v2.2d, v5.2d
        zip1            v26.2d, v3.2d, v7.2d
        zip2            v27.2d, v3.2d, v7.2d
        st1             { v24.4s, v25.4s, v26.4s, v27.4s }, [x1]

        ret














        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16

        EXPORT ff_tx_fft32_float_neon





ff_tx_fft32_float_neon
        










        stp             d14, d15, [sp, #-16*4]!
        stp             d8,  d9,  [sp, #16*3]
        stp             d10, d11, [sp, #16*2]
        stp             d12, d13, [sp, #16]











        adrp            x5, subadd
        add             x5, x5, subadd










        ld1             { v31.4s }, [x5]
        ldr             w5, =(32 - 4*7)










        adrp            x7, ff_tx_tab_32_float
        add             x7, x7, ff_tx_tab_32_float










        add             x8, x7, x5
        mov             x9, #-32


        ldr             x4, [x0, #8]
        ldp             w10, w11, [x4, #0 ]
        ldp             w12, w13, [x4, #8 ]
        ldp             w14, w15, [x4, #16]
        ldp             w16, w17, [x4, #24]

        add             x4, x4, #32

        ldr             d0, [x2, x10, lsl #3]
        add             x11, x2, x11, lsl #3
        ldr             d1, [x2, x12, lsl #3]
        add             x13, x2, x13, lsl #3
        ldr             d2, [x2, x14, lsl #3]
        add             x15, x2, x15, lsl #3
        ldr             d3, [x2, x16, lsl #3]
        add             x17, x2, x17, lsl #3

        ld1             { v0.d }[1], [x11]
        ld1             { v1.d }[1], [x13]
        ld1             { v2.d }[1], [x15]
        ld1             { v3.d }[1], [x17]
        ldp             w10, w11, [x4, #0 ]
        ldp             w12, w13, [x4, #8 ]
        ldp             w14, w15, [x4, #16]
        ldp             w16, w17, [x4, #24]

        add             x4, x4, #32

        ldr             d4, [x2, x10, lsl #3]
        add             x11, x2, x11, lsl #3
        ldr             d5, [x2, x12, lsl #3]
        add             x13, x2, x13, lsl #3
        ldr             d6, [x2, x14, lsl #3]
        add             x15, x2, x15, lsl #3
        ldr             d7, [x2, x16, lsl #3]
        add             x17, x2, x17, lsl #3

        ld1             { v4.d }[1], [x11]
        ld1             { v5.d }[1], [x13]
        ld1             { v6.d }[1], [x15]
        ld1             { v7.d }[1], [x17]
        ldp             w10, w11, [x4, #0 ]
        ldp             w12, w13, [x4, #8 ]
        ldp             w14, w15, [x4, #16]
        ldp             w16, w17, [x4, #24]

        add             x4, x4, #32

        ldr             d8, [x2, x10, lsl #3]
        add             x11, x2, x11, lsl #3
        ldr             d9, [x2, x12, lsl #3]
        add             x13, x2, x13, lsl #3
        ldr             d10, [x2, x14, lsl #3]
        add             x15, x2, x15, lsl #3
        ldr             d11, [x2, x16, lsl #3]
        add             x17, x2, x17, lsl #3

        ld1             { v8.d }[1], [x11]
        ld1             { v9.d }[1], [x13]
        ld1             { v10.d }[1], [x15]
        ld1             { v11.d }[1], [x17]
        ldp             w10, w11, [x4, #0 ]
        ldp             w12, w13, [x4, #8 ]
        ldp             w14, w15, [x4, #16]
        ldp             w16, w17, [x4, #24]

        add             x4, x4, #32

        ldr             d12, [x2, x10, lsl #3]
        add             x11, x2, x11, lsl #3
        ldr             d13, [x2, x12, lsl #3]
        add             x13, x2, x13, lsl #3
        ldr             d14, [x2, x14, lsl #3]
        add             x15, x2, x15, lsl #3
        ldr             d15, [x2, x16, lsl #3]
        add             x17, x2, x17, lsl #3

        ld1             { v12.d }[1], [x11]
        ld1             { v13.d }[1], [x13]
        ld1             { v14.d }[1], [x15]
        ld1             { v15.d }[1], [x17]












        adrp            x5, tab_8pt
        add             x5, x5, tab_8pt











        fadd            v19.4s, v13.4s, v15.4s             
        fadd            v17.4s, v9.4s, v11.4s             
        fadd            v18.4s, v12.4s, v14.4s             
        fadd            v16.4s, v8.4s, v10.4s             

        ld1             { v23.4s }, [x5]

        ext8             v22.16b, v19.16b, v19.16b, #12
        ext8             v21.16b, v17.16b, v17.16b, #12

        rev64           v22.4s,  v22.4s
        rev64           v21.4s,  v21.4s

        ext8             v19.16b, v18.16b, v22.16b, #8
        ext8             v17.16b, v16.16b, v21.16b, #8

        mov             v18.d[1], v22.d[1]
        mov             v21.d[0], v16.d[0]

        fadd            v22.4s, v18.4s, v19.4s                   
        fsub            v19.4s, v18.4s, v19.4s                   
        fsub            v18.4s, v21.4s, v17.4s                   
        fadd            v16.4s, v21.4s, v17.4s                   

        fsub            v8.4s, v8.4s, v10.4s          
        fsub            v20.4s, v9.4s, v11.4s             
        fsub            v12.4s, v12.4s, v14.4s          
        fsub            v21.4s, v13.4s, v15.4s             

        rev64           v24.4s, v31.4s                           
        zip1            v17.2d, v16.2d, v18.2d                   
        zip1            v9.2d, v22.2d, v19.2d                

        rev64           v18.4s, v18.4s                           
        rev64           v19.4s, v19.4s                           

        zip2            v16.2d, v16.2d, v18.2d                   
        zip2            v13.2d, v22.2d, v19.2d                

        dup             v10.2d, v8.d[0]                   
        dup             v11.2d, v8.d[1]                   
        dup             v14.2d, v12.d[0]                   
        dup             v15.2d, v12.d[1]                   

        fadd            v12.4s, v9.4s, v13.4s          
        fsub            v13.4s, v9.4s, v13.4s          
        fadd            v8.4s, v17.4s, v16.4s                
        fsub            v9.4s, v17.4s, v16.4s                

        ext8             v24.16b, v31.16b, v24.16b, #8            
        rev64           v11.4s, v11.4s                     
        rev64           v15.4s, v15.4s                     

        fmul            v19.4s, v20.4s, v23.4s                   
        fmul            v21.4s, v21.4s, v23.4s                   

        rev64           v20.4s, v19.4s                           
        rev64           v18.4s, v21.4s                           

        fmls            v10.4s, v11.4s, v24.4s             
        fmls            v14.4s, v15.4s, v24.4s             

        fmla            v19.4s, v20.4s, v31.4s                   
        fmla            v21.4s, v18.4s, v31.4s                   

        rev64           v20.4s, v19.4s                           
        rev64           v18.4s, v21.4s                           
        ext8             v20.16b, v20.16b, v20.16b, #8            
        ext8             v18.16b, v18.16b, v18.16b, #8            

        fmla            v20.4s, v19.4s, v31.4s                   
        fmla            v18.4s, v21.4s, v31.4s                   

        fadd            v11.4s, v10.4s, v20.4s             
        fadd            v15.4s, v14.4s, v18.4s             
        fsub            v10.4s, v10.4s, v20.4s             
        fsub            v14.4s, v14.4s, v18.4s             












        adrp            x5, tab_8pt
        add             x5, x5, tab_8pt











        fsub            v17.4s, v1.4s, v3.4s             
        fadd            v3.4s, v1.4s, v3.4s             
        fsub            v16.4s, v0.4s, v2.4s             
        fadd            v2.4s, v0.4s, v2.4s             

        ld1             { v21.4s }, [x5]

        ext8             v20.16b, v3.16b, v3.16b, #12
        rev64           v20.4s,  v20.4s

        ext8             v18.16b, v2.16b, v20.16b, #8      
        mov             v2.d[1], v20.d[1]                    

        fsub            v1.4s, v2.4s, v18.4s             
        fadd            v18.4s, v2.4s, v18.4s             

        rev64           v22.4s, v31.4s                           
        dup             v2.2d, v16.d[0]                      
        dup             v3.2d, v16.d[1]                      

        rev64           v20.4s, v1.4s                        
        rev64           v3.4s, v3.4s                        

        ext8             v22.16b, v31.16b, v22.16b, #8         
        zip1            v19.2d, v18.2d, v1.2d             
        zip2            v18.2d, v18.2d, v20.2d             

        fadd            v0.4s, v19.4s, v18.4s             
        fsub            v1.4s, v19.4s, v18.4s             

        fmul            v17.4s, v17.4s, v21.4s             
        fmls            v2.4s, v3.4s, v22.4s             

        rev64           v20.4s, v17.4s                        
        fmla            v17.4s, v20.4s, v31.4s                

        rev64           v20.4s, v17.4s                        
        ext8             v20.16b, v20.16b, v20.16b, #8      

        fmla            v20.4s, v17.4s, v31.4s                

        fadd            v3.4s, v2.4s, v20.4s             
        fsub            v2.4s, v2.4s, v20.4s             

        fadd            v16.4s, v4.4s, v5.4s                     
        fadd            v18.4s, v6.4s, v7.4s                     
        fsub            v4.4s, v4.4s, v5.4s                     
        fsub            v6.4s, v6.4s, v7.4s                     

        rev64           v20.4s, v4.4s
        rev64           v21.4s, v6.4s

        zip2            v5.2d, v16.2d, v20.2d                     
        zip2            v7.2d, v18.2d, v21.2d                     

        fneg            v19.4s, v20.4s
        fneg            v20.4s, v21.4s

        mov             v5.s[3], v19.s[3]
        mov             v7.s[3], v20.s[3]

        zip1            v17.2d, v16.2d, v4.2d                     
        zip1            v19.2d, v18.2d, v6.2d                     

        fsub            v20.4s, v17.4s, v5.4s                     
        fadd            v21.4s, v17.4s, v5.4s                     
        fsub            v18.4s, v19.4s, v7.4s                     
        fadd            v19.4s, v19.4s, v7.4s                     

        zip1            v4.2d, v21.2d, v20.2d                     
        zip2            v5.2d, v21.2d, v20.2d                     
        zip1            v6.2d, v19.2d, v18.2d                     
        zip2            v7.2d, v19.2d, v18.2d                     











        adrp            x5, tab_16pt
        add             x5, x5, tab_16pt











        rev64           v16.4s, v4.4s                     
        rev64           v17.4s, v6.4s                     

        ins             v16.d[0], xzr
        ins             v17.d[0], xzr

        ld1             { v20.4s, v21.4s, v22.4s }, [x5]
        

        fmla            v6.4s, v17.4s, v31.4s             
        fmls            v4.4s, v16.4s, v31.4s             

        fmul            v18.4s, v5.4s, v20.4s
        fmul            v19.4s, v7.4s, v20.4s

        rev64           v7.4s, v7.4s
        rev64           v5.4s, v5.4s

        fmla            v19.4s, v7.4s, v21.4s          
        fmls            v18.4s, v5.4s, v21.4s          

        fmul            v17.4s, v6.4s, v22.4s          
        fmul            v16.4s, v4.4s, v22.4s          

        orr v5.16b, v19.16b, v19.16b
        orr v6.16b, v17.16b, v17.16b

        fsub            v19.4s, v19.4s, v18.4s          
        fsub            v17.4s, v17.4s, v16.4s          

        fadd            v18.4s, v18.4s, v5.4s          
        rev64           v19.4s, v19.4s
        fadd            v16.4s, v16.4s, v6.4s          
        rev64           v17.4s, v17.4s

        fmul            v18.4s, v18.4s, v31.4s
        fmul            v17.4s, v17.4s, v31.4s

        fadd            v7.4s, v3.4s, v19.4s
        fsub            v6.4s, v3.4s, v19.4s
        fsub            v5.4s, v2.4s, v18.4s
        fadd            v4.4s, v2.4s, v18.4s

        fsub            v2.4s, v0.4s, v16.4s
        fadd            v0.4s, v0.4s, v16.4s
        fsub            v3.4s, v1.4s, v17.4s
        fadd            v1.4s, v1.4s, v17.4s


        ld1             { v24.4s, v25.4s }, [x8], x9
        ld1             { v16.4s, v17.4s }, [x7], #32

        ext8             v18.16b, v24.16b, v24.16b, #8
        ext8             v19.16b, v25.16b, v25.16b, #8

        trn1            v20.4s, v16.4s, v16.4s      
        trn2            v16.4s, v16.4s, v16.4s      
        trn1            v21.4s, v17.4s, v17.4s      
        trn2            v17.4s, v17.4s, v17.4s      

        rev64           v22.4s, v8.4s                 
        rev64           v23.4s, v10.4s                 
        rev64           v24.4s, v12.4s                 
        rev64           v25.4s, v14.4s                 

        fmul            v22.4s, v22.4s, v20.4s      
        fmul            v23.4s, v23.4s, v16.4s      
        fmul            v24.4s, v24.4s, v20.4s      
        fmul            v25.4s, v25.4s, v16.4s      

        rev64           v26.4s, v9.4s                 
        rev64           v27.4s, v11.4s                 
        rev64           v20.4s, v13.4s                 
        rev64           v16.4s, v15.4s                 

        fmul            v26.4s, v26.4s, v21.4s      
        fmul            v27.4s, v27.4s, v17.4s      
        fmul            v20.4s, v20.4s, v21.4s      
        fmul            v16.4s, v16.4s, v17.4s      

        trn1            v21.4s, v19.4s, v19.4s      
        trn2            v19.4s, v19.4s, v19.4s      
        trn1            v17.4s, v18.4s, v18.4s      
        trn2            v18.4s, v18.4s, v18.4s      

        fmul            v21.4s, v21.4s, v31.4s
        fmul            v19.4s, v19.4s, v31.4s
        fmul            v17.4s, v17.4s, v31.4s
        fmul            v18.4s, v18.4s, v31.4s

        fmla            v23.4s, v10.4s, v21.4s      
        fmls            v25.4s, v14.4s, v21.4s      
        fmla            v22.4s, v8.4s, v19.4s      
        fmls            v24.4s, v12.4s, v19.4s      

        fmla            v26.4s, v9.4s, v18.4s      
        fmla            v27.4s, v11.4s, v17.4s      
        fmls            v20.4s, v13.4s, v18.4s      
        fmls            v16.4s, v15.4s, v17.4s      

        fsub            v18.4s, v23.4s, v25.4s
        fsub            v17.4s, v24.4s, v22.4s
        fsub            v19.4s, v20.4s, v26.4s
        fsub            v21.4s, v16.4s, v27.4s

        fadd            v22.4s, v24.4s, v22.4s
        fadd            v23.4s, v25.4s, v23.4s
        fadd            v24.4s, v20.4s, v26.4s
        fadd            v25.4s, v16.4s, v27.4s

        fmul            v17.4s, v17.4s, v31.4s
        fmul            v18.4s, v18.4s, v31.4s
        fmul            v19.4s, v19.4s, v31.4s
        fmul            v21.4s, v21.4s, v31.4s

        rev64           v22.4s, v22.4s
        rev64           v24.4s, v24.4s
        rev64           v23.4s, v23.4s
        rev64           v25.4s, v25.4s

        fsub            v8.4s, v0.4s, v22.4s
        fsub            v9.4s, v1.4s, v24.4s
        fsub            v10.4s, v2.4s, v17.4s
        fsub            v11.4s, v3.4s, v19.4s

        fsub            v12.4s, v4.4s, v23.4s
        fsub            v13.4s, v6.4s, v25.4s
        fadd            v14.4s, v5.4s, v18.4s
        fsub            v15.4s, v7.4s, v21.4s

        fadd            v0.4s, v0.4s, v22.4s
        fadd            v1.4s, v1.4s, v24.4s
        fadd            v2.4s, v2.4s, v17.4s
        fadd            v3.4s, v3.4s, v19.4s

        fadd            v4.4s, v4.4s, v23.4s
        fsub            v5.4s, v5.4s, v18.4s      
        fadd            v6.4s, v6.4s, v25.4s      
        fadd            v7.4s, v7.4s, v21.4s

        zip1            v16.2d, v0.2d, v4.2d
        zip2            v17.2d, v0.2d, v4.2d
        zip1            v18.2d, v1.2d, v6.2d
        zip2            v19.2d, v1.2d, v6.2d
        st1             { v16.4s, v17.4s, v18.4s, v19.4s }, [x1], #64

        zip1            v20.2d, v2.2d, v5.2d
        zip2            v21.2d, v2.2d, v5.2d
        zip1            v22.2d, v3.2d, v7.2d
        zip2            v23.2d, v3.2d, v7.2d
        st1             { v20.4s, v21.4s, v22.4s, v23.4s }, [x1], #64

        zip1            v24.2d, v8.2d, v12.2d
        zip2            v25.2d, v8.2d, v12.2d
        zip1            v26.2d, v9.2d, v13.2d
        zip2            v27.2d, v9.2d, v13.2d
        st1             { v24.4s, v25.4s, v26.4s, v27.4s }, [x1], #64

        zip1            v28.2d, v10.2d, v14.2d
        zip2            v29.2d, v10.2d, v14.2d
        zip1            v30.2d, v11.2d, v15.2d
        zip2            v31.2d, v11.2d, v15.2d
        st1             { v28.4s, v29.4s, v30.4s, v31.4s }, [x1]

        ldp             d12, d13, [sp, #16]
        ldp             d10, d11, [sp, #16*2]
        ldp             d8,  d9,  [sp, #16*3]
        ldp             d14, d15, [sp], #16*4

        ret


        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16

        EXPORT ff_tx_fft32_ns_float_neon





ff_tx_fft32_ns_float_neon
        










        stp             d14, d15, [sp, #-16*4]!
        stp             d8,  d9,  [sp, #16*3]
        stp             d10, d11, [sp, #16*2]
        stp             d12, d13, [sp, #16]











        adrp            x5, subadd
        add             x5, x5, subadd










        ld1             { v31.4s }, [x5]
        ldr             w5, =(32 - 4*7)










        adrp            x7, ff_tx_tab_32_float
        add             x7, x7, ff_tx_tab_32_float










        add             x8, x7, x5
        mov             x9, #-32


        ld1             { v0.4s, v1.4s, v2.4s, v3.4s }, [x2], #64
        ld1             { v4.4s, v5.4s, v6.4s, v7.4s }, [x2], #64
        ld1             { v8.4s, v9.4s, v10.4s, v11.4s }, [x2], #64
        ld1             { v12.4s, v13.4s, v14.4s, v15.4s }, [x2], #64












        adrp            x5, tab_8pt
        add             x5, x5, tab_8pt











        fadd            v19.4s, v13.4s, v15.4s             
        fadd            v17.4s, v9.4s, v11.4s             
        fadd            v18.4s, v12.4s, v14.4s             
        fadd            v16.4s, v8.4s, v10.4s             

        ld1             { v23.4s }, [x5]

        ext8             v22.16b, v19.16b, v19.16b, #12
        ext8             v21.16b, v17.16b, v17.16b, #12

        rev64           v22.4s,  v22.4s
        rev64           v21.4s,  v21.4s

        ext8             v19.16b, v18.16b, v22.16b, #8
        ext8             v17.16b, v16.16b, v21.16b, #8

        mov             v18.d[1], v22.d[1]
        mov             v21.d[0], v16.d[0]

        fadd            v22.4s, v18.4s, v19.4s                   
        fsub            v19.4s, v18.4s, v19.4s                   
        fsub            v18.4s, v21.4s, v17.4s                   
        fadd            v16.4s, v21.4s, v17.4s                   

        fsub            v8.4s, v8.4s, v10.4s          
        fsub            v20.4s, v9.4s, v11.4s             
        fsub            v12.4s, v12.4s, v14.4s          
        fsub            v21.4s, v13.4s, v15.4s             

        rev64           v24.4s, v31.4s                           
        zip1            v17.2d, v16.2d, v18.2d                   
        zip1            v9.2d, v22.2d, v19.2d                

        rev64           v18.4s, v18.4s                           
        rev64           v19.4s, v19.4s                           

        zip2            v16.2d, v16.2d, v18.2d                   
        zip2            v13.2d, v22.2d, v19.2d                

        dup             v10.2d, v8.d[0]                   
        dup             v11.2d, v8.d[1]                   
        dup             v14.2d, v12.d[0]                   
        dup             v15.2d, v12.d[1]                   

        fadd            v12.4s, v9.4s, v13.4s          
        fsub            v13.4s, v9.4s, v13.4s          
        fadd            v8.4s, v17.4s, v16.4s                
        fsub            v9.4s, v17.4s, v16.4s                

        ext8             v24.16b, v31.16b, v24.16b, #8            
        rev64           v11.4s, v11.4s                     
        rev64           v15.4s, v15.4s                     

        fmul            v19.4s, v20.4s, v23.4s                   
        fmul            v21.4s, v21.4s, v23.4s                   

        rev64           v20.4s, v19.4s                           
        rev64           v18.4s, v21.4s                           

        fmls            v10.4s, v11.4s, v24.4s             
        fmls            v14.4s, v15.4s, v24.4s             

        fmla            v19.4s, v20.4s, v31.4s                   
        fmla            v21.4s, v18.4s, v31.4s                   

        rev64           v20.4s, v19.4s                           
        rev64           v18.4s, v21.4s                           
        ext8             v20.16b, v20.16b, v20.16b, #8            
        ext8             v18.16b, v18.16b, v18.16b, #8            

        fmla            v20.4s, v19.4s, v31.4s                   
        fmla            v18.4s, v21.4s, v31.4s                   

        fadd            v11.4s, v10.4s, v20.4s             
        fadd            v15.4s, v14.4s, v18.4s             
        fsub            v10.4s, v10.4s, v20.4s             
        fsub            v14.4s, v14.4s, v18.4s             












        adrp            x5, tab_8pt
        add             x5, x5, tab_8pt











        fsub            v17.4s, v1.4s, v3.4s             
        fadd            v3.4s, v1.4s, v3.4s             
        fsub            v16.4s, v0.4s, v2.4s             
        fadd            v2.4s, v0.4s, v2.4s             

        ld1             { v21.4s }, [x5]

        ext8             v20.16b, v3.16b, v3.16b, #12
        rev64           v20.4s,  v20.4s

        ext8             v18.16b, v2.16b, v20.16b, #8      
        mov             v2.d[1], v20.d[1]                    

        fsub            v1.4s, v2.4s, v18.4s             
        fadd            v18.4s, v2.4s, v18.4s             

        rev64           v22.4s, v31.4s                           
        dup             v2.2d, v16.d[0]                      
        dup             v3.2d, v16.d[1]                      

        rev64           v20.4s, v1.4s                        
        rev64           v3.4s, v3.4s                        

        ext8             v22.16b, v31.16b, v22.16b, #8         
        zip1            v19.2d, v18.2d, v1.2d             
        zip2            v18.2d, v18.2d, v20.2d             

        fadd            v0.4s, v19.4s, v18.4s             
        fsub            v1.4s, v19.4s, v18.4s             

        fmul            v17.4s, v17.4s, v21.4s             
        fmls            v2.4s, v3.4s, v22.4s             

        rev64           v20.4s, v17.4s                        
        fmla            v17.4s, v20.4s, v31.4s                

        rev64           v20.4s, v17.4s                        
        ext8             v20.16b, v20.16b, v20.16b, #8      

        fmla            v20.4s, v17.4s, v31.4s                

        fadd            v3.4s, v2.4s, v20.4s             
        fsub            v2.4s, v2.4s, v20.4s             

        fadd            v16.4s, v4.4s, v5.4s                     
        fadd            v18.4s, v6.4s, v7.4s                     
        fsub            v4.4s, v4.4s, v5.4s                     
        fsub            v6.4s, v6.4s, v7.4s                     

        rev64           v20.4s, v4.4s
        rev64           v21.4s, v6.4s

        zip2            v5.2d, v16.2d, v20.2d                     
        zip2            v7.2d, v18.2d, v21.2d                     

        fneg            v19.4s, v20.4s
        fneg            v20.4s, v21.4s

        mov             v5.s[3], v19.s[3]
        mov             v7.s[3], v20.s[3]

        zip1            v17.2d, v16.2d, v4.2d                     
        zip1            v19.2d, v18.2d, v6.2d                     

        fsub            v20.4s, v17.4s, v5.4s                     
        fadd            v21.4s, v17.4s, v5.4s                     
        fsub            v18.4s, v19.4s, v7.4s                     
        fadd            v19.4s, v19.4s, v7.4s                     

        zip1            v4.2d, v21.2d, v20.2d                     
        zip2            v5.2d, v21.2d, v20.2d                     
        zip1            v6.2d, v19.2d, v18.2d                     
        zip2            v7.2d, v19.2d, v18.2d                     











        adrp            x5, tab_16pt
        add             x5, x5, tab_16pt











        rev64           v16.4s, v4.4s                     
        rev64           v17.4s, v6.4s                     

        ins             v16.d[0], xzr
        ins             v17.d[0], xzr

        ld1             { v20.4s, v21.4s, v22.4s }, [x5]
        

        fmla            v6.4s, v17.4s, v31.4s             
        fmls            v4.4s, v16.4s, v31.4s             

        fmul            v18.4s, v5.4s, v20.4s
        fmul            v19.4s, v7.4s, v20.4s

        rev64           v7.4s, v7.4s
        rev64           v5.4s, v5.4s

        fmla            v19.4s, v7.4s, v21.4s          
        fmls            v18.4s, v5.4s, v21.4s          

        fmul            v17.4s, v6.4s, v22.4s          
        fmul            v16.4s, v4.4s, v22.4s          

        orr v5.16b, v19.16b, v19.16b
        orr v6.16b, v17.16b, v17.16b

        fsub            v19.4s, v19.4s, v18.4s          
        fsub            v17.4s, v17.4s, v16.4s          

        fadd            v18.4s, v18.4s, v5.4s          
        rev64           v19.4s, v19.4s
        fadd            v16.4s, v16.4s, v6.4s          
        rev64           v17.4s, v17.4s

        fmul            v18.4s, v18.4s, v31.4s
        fmul            v17.4s, v17.4s, v31.4s

        fadd            v7.4s, v3.4s, v19.4s
        fsub            v6.4s, v3.4s, v19.4s
        fsub            v5.4s, v2.4s, v18.4s
        fadd            v4.4s, v2.4s, v18.4s

        fsub            v2.4s, v0.4s, v16.4s
        fadd            v0.4s, v0.4s, v16.4s
        fsub            v3.4s, v1.4s, v17.4s
        fadd            v1.4s, v1.4s, v17.4s


        ld1             { v24.4s, v25.4s }, [x8], x9
        ld1             { v16.4s, v17.4s }, [x7], #32

        ext8             v18.16b, v24.16b, v24.16b, #8
        ext8             v19.16b, v25.16b, v25.16b, #8

        trn1            v20.4s, v16.4s, v16.4s      
        trn2            v16.4s, v16.4s, v16.4s      
        trn1            v21.4s, v17.4s, v17.4s      
        trn2            v17.4s, v17.4s, v17.4s      

        rev64           v22.4s, v8.4s                 
        rev64           v23.4s, v10.4s                 
        rev64           v24.4s, v12.4s                 
        rev64           v25.4s, v14.4s                 

        fmul            v22.4s, v22.4s, v20.4s      
        fmul            v23.4s, v23.4s, v16.4s      
        fmul            v24.4s, v24.4s, v20.4s      
        fmul            v25.4s, v25.4s, v16.4s      

        rev64           v26.4s, v9.4s                 
        rev64           v27.4s, v11.4s                 
        rev64           v20.4s, v13.4s                 
        rev64           v16.4s, v15.4s                 

        fmul            v26.4s, v26.4s, v21.4s      
        fmul            v27.4s, v27.4s, v17.4s      
        fmul            v20.4s, v20.4s, v21.4s      
        fmul            v16.4s, v16.4s, v17.4s      

        trn1            v21.4s, v19.4s, v19.4s      
        trn2            v19.4s, v19.4s, v19.4s      
        trn1            v17.4s, v18.4s, v18.4s      
        trn2            v18.4s, v18.4s, v18.4s      

        fmul            v21.4s, v21.4s, v31.4s
        fmul            v19.4s, v19.4s, v31.4s
        fmul            v17.4s, v17.4s, v31.4s
        fmul            v18.4s, v18.4s, v31.4s

        fmla            v23.4s, v10.4s, v21.4s      
        fmls            v25.4s, v14.4s, v21.4s      
        fmla            v22.4s, v8.4s, v19.4s      
        fmls            v24.4s, v12.4s, v19.4s      

        fmla            v26.4s, v9.4s, v18.4s      
        fmla            v27.4s, v11.4s, v17.4s      
        fmls            v20.4s, v13.4s, v18.4s      
        fmls            v16.4s, v15.4s, v17.4s      

        fsub            v18.4s, v23.4s, v25.4s
        fsub            v17.4s, v24.4s, v22.4s
        fsub            v19.4s, v20.4s, v26.4s
        fsub            v21.4s, v16.4s, v27.4s

        fadd            v22.4s, v24.4s, v22.4s
        fadd            v23.4s, v25.4s, v23.4s
        fadd            v24.4s, v20.4s, v26.4s
        fadd            v25.4s, v16.4s, v27.4s

        fmul            v17.4s, v17.4s, v31.4s
        fmul            v18.4s, v18.4s, v31.4s
        fmul            v19.4s, v19.4s, v31.4s
        fmul            v21.4s, v21.4s, v31.4s

        rev64           v22.4s, v22.4s
        rev64           v24.4s, v24.4s
        rev64           v23.4s, v23.4s
        rev64           v25.4s, v25.4s

        fsub            v8.4s, v0.4s, v22.4s
        fsub            v9.4s, v1.4s, v24.4s
        fsub            v10.4s, v2.4s, v17.4s
        fsub            v11.4s, v3.4s, v19.4s

        fsub            v12.4s, v4.4s, v23.4s
        fsub            v13.4s, v6.4s, v25.4s
        fadd            v14.4s, v5.4s, v18.4s
        fsub            v15.4s, v7.4s, v21.4s

        fadd            v0.4s, v0.4s, v22.4s
        fadd            v1.4s, v1.4s, v24.4s
        fadd            v2.4s, v2.4s, v17.4s
        fadd            v3.4s, v3.4s, v19.4s

        fadd            v4.4s, v4.4s, v23.4s
        fsub            v5.4s, v5.4s, v18.4s      
        fadd            v6.4s, v6.4s, v25.4s      
        fadd            v7.4s, v7.4s, v21.4s

        zip1            v16.2d, v0.2d, v4.2d
        zip2            v17.2d, v0.2d, v4.2d
        zip1            v18.2d, v1.2d, v6.2d
        zip2            v19.2d, v1.2d, v6.2d
        st1             { v16.4s, v17.4s, v18.4s, v19.4s }, [x1], #64

        zip1            v20.2d, v2.2d, v5.2d
        zip2            v21.2d, v2.2d, v5.2d
        zip1            v22.2d, v3.2d, v7.2d
        zip2            v23.2d, v3.2d, v7.2d
        st1             { v20.4s, v21.4s, v22.4s, v23.4s }, [x1], #64

        zip1            v24.2d, v8.2d, v12.2d
        zip2            v25.2d, v8.2d, v12.2d
        zip1            v26.2d, v9.2d, v13.2d
        zip2            v27.2d, v9.2d, v13.2d
        st1             { v24.4s, v25.4s, v26.4s, v27.4s }, [x1], #64

        zip1            v28.2d, v10.2d, v14.2d
        zip2            v29.2d, v10.2d, v14.2d
        zip1            v30.2d, v11.2d, v15.2d
        zip2            v31.2d, v11.2d, v15.2d
        st1             { v28.4s, v29.4s, v30.4s, v31.4s }, [x1]

        ldp             d12, d13, [sp, #16]
        ldp             d10, d11, [sp, #16*2]
        ldp             d8,  d9,  [sp, #16*3]
        ldp             d14, d15, [sp], #16*4

        ret






        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16

        EXPORT ff_tx_fft_sr_float_neon





ff_tx_fft_sr_float_neon
        










        stp             x21, x22, [sp, #-16*6]!
        stp             d8,  d9,  [sp, #16*5]
        stp             d10, d11, [sp, #16*4]
        stp             d12, d13, [sp, #16*3]
        stp             d14, d15, [sp, #16*2]
        stp             x19, x20, [sp, #16]

        ldr             w19, [x0, #0] 
        mov             w20, w19      











        adrp            x5, subadd
        add             x5, x5, subadd










        ld1             { v31.4s }, [x5]
        ldr             x4, [x0, #8]

temp_label_0

        ldr             w5, =(32 - 4*7)










        adrp            x7, ff_tx_tab_32_float
        add             x7, x7, ff_tx_tab_32_float










        add             x8, x7, x5
        mov             x9, #-32


        ldp             w10, w11, [x4, #0 ]
        ldp             w12, w13, [x4, #8 ]
        ldp             w14, w15, [x4, #16]
        ldp             w16, w17, [x4, #24]

        add             x4, x4, #32

        ldr             d0, [x2, x10, lsl #3]
        add             x11, x2, x11, lsl #3
        ldr             d1, [x2, x12, lsl #3]
        add             x13, x2, x13, lsl #3
        ldr             d2, [x2, x14, lsl #3]
        add             x15, x2, x15, lsl #3
        ldr             d3, [x2, x16, lsl #3]
        add             x17, x2, x17, lsl #3

        ld1             { v0.d }[1], [x11]
        ld1             { v1.d }[1], [x13]
        ld1             { v2.d }[1], [x15]
        ld1             { v3.d }[1], [x17]
        ldp             w10, w11, [x4, #0 ]
        ldp             w12, w13, [x4, #8 ]
        ldp             w14, w15, [x4, #16]
        ldp             w16, w17, [x4, #24]

        add             x4, x4, #32

        ldr             d4, [x2, x10, lsl #3]
        add             x11, x2, x11, lsl #3
        ldr             d6, [x2, x12, lsl #3]
        add             x13, x2, x13, lsl #3
        ldr             d5, [x2, x14, lsl #3]
        add             x15, x2, x15, lsl #3
        ldr             d7, [x2, x16, lsl #3]
        add             x17, x2, x17, lsl #3

        ld1             { v4.d }[1], [x11]
        ld1             { v6.d }[1], [x13]
        ld1             { v5.d }[1], [x15]
        ld1             { v7.d }[1], [x17]
        ldp             w10, w11, [x4, #0 ]
        ldp             w12, w13, [x4, #8 ]
        ldp             w14, w15, [x4, #16]
        ldp             w16, w17, [x4, #24]

        add             x4, x4, #32

        ldr             d8, [x2, x10, lsl #3]
        add             x11, x2, x11, lsl #3
        ldr             d9, [x2, x12, lsl #3]
        add             x13, x2, x13, lsl #3
        ldr             d10, [x2, x14, lsl #3]
        add             x15, x2, x15, lsl #3
        ldr             d11, [x2, x16, lsl #3]
        add             x17, x2, x17, lsl #3

        ld1             { v8.d }[1], [x11]
        ld1             { v9.d }[1], [x13]
        ld1             { v10.d }[1], [x15]
        ld1             { v11.d }[1], [x17]
        ldp             w10, w11, [x4, #0 ]
        ldp             w12, w13, [x4, #8 ]
        ldp             w14, w15, [x4, #16]
        ldp             w16, w17, [x4, #24]

        add             x4, x4, #32

        ldr             d12, [x2, x10, lsl #3]
        add             x11, x2, x11, lsl #3
        ldr             d13, [x2, x12, lsl #3]
        add             x13, x2, x13, lsl #3
        ldr             d14, [x2, x14, lsl #3]
        add             x15, x2, x15, lsl #3
        ldr             d15, [x2, x16, lsl #3]
        add             x17, x2, x17, lsl #3

        ld1             { v12.d }[1], [x11]
        ld1             { v13.d }[1], [x13]
        ld1             { v14.d }[1], [x15]
        ld1             { v15.d }[1], [x17]












        adrp            x5, tab_8pt
        add             x5, x5, tab_8pt











        fadd            v19.4s, v13.4s, v15.4s             
        fadd            v17.4s, v9.4s, v11.4s             
        fadd            v18.4s, v12.4s, v14.4s             
        fadd            v16.4s, v8.4s, v10.4s             

        ld1             { v23.4s }, [x5]

        ext8             v22.16b, v19.16b, v19.16b, #12
        ext8             v21.16b, v17.16b, v17.16b, #12

        rev64           v22.4s,  v22.4s
        rev64           v21.4s,  v21.4s

        ext8             v19.16b, v18.16b, v22.16b, #8
        ext8             v17.16b, v16.16b, v21.16b, #8

        mov             v18.d[1], v22.d[1]
        mov             v21.d[0], v16.d[0]

        fadd            v22.4s, v18.4s, v19.4s                   
        fsub            v19.4s, v18.4s, v19.4s                   
        fsub            v18.4s, v21.4s, v17.4s                   
        fadd            v16.4s, v21.4s, v17.4s                   

        fsub            v8.4s, v8.4s, v10.4s          
        fsub            v20.4s, v9.4s, v11.4s             
        fsub            v12.4s, v12.4s, v14.4s          
        fsub            v21.4s, v13.4s, v15.4s             

        rev64           v24.4s, v31.4s                           
        zip1            v17.2d, v16.2d, v18.2d                   
        zip1            v9.2d, v22.2d, v19.2d                

        rev64           v18.4s, v18.4s                           
        rev64           v19.4s, v19.4s                           

        zip2            v16.2d, v16.2d, v18.2d                   
        zip2            v13.2d, v22.2d, v19.2d                

        dup             v10.2d, v8.d[0]                   
        dup             v11.2d, v8.d[1]                   
        dup             v14.2d, v12.d[0]                   
        dup             v15.2d, v12.d[1]                   

        fadd            v12.4s, v9.4s, v13.4s          
        fsub            v13.4s, v9.4s, v13.4s          
        fadd            v8.4s, v17.4s, v16.4s                
        fsub            v9.4s, v17.4s, v16.4s                

        ext8             v24.16b, v31.16b, v24.16b, #8            
        rev64           v11.4s, v11.4s                     
        rev64           v15.4s, v15.4s                     

        fmul            v19.4s, v20.4s, v23.4s                   
        fmul            v21.4s, v21.4s, v23.4s                   

        rev64           v20.4s, v19.4s                           
        rev64           v18.4s, v21.4s                           

        fmls            v10.4s, v11.4s, v24.4s             
        fmls            v14.4s, v15.4s, v24.4s             

        fmla            v19.4s, v20.4s, v31.4s                   
        fmla            v21.4s, v18.4s, v31.4s                   

        rev64           v20.4s, v19.4s                           
        rev64           v18.4s, v21.4s                           
        ext8             v20.16b, v20.16b, v20.16b, #8            
        ext8             v18.16b, v18.16b, v18.16b, #8            

        fmla            v20.4s, v19.4s, v31.4s                   
        fmla            v18.4s, v21.4s, v31.4s                   

        fadd            v11.4s, v10.4s, v20.4s             
        fadd            v15.4s, v14.4s, v18.4s             
        fsub            v10.4s, v10.4s, v20.4s             
        fsub            v14.4s, v14.4s, v18.4s             












        adrp            x5, tab_8pt
        add             x5, x5, tab_8pt











        fsub            v17.4s, v1.4s, v3.4s             
        fadd            v3.4s, v1.4s, v3.4s             
        fsub            v16.4s, v0.4s, v2.4s             
        fadd            v2.4s, v0.4s, v2.4s             

        ld1             { v21.4s }, [x5]

        ext8             v20.16b, v3.16b, v3.16b, #12
        rev64           v20.4s,  v20.4s

        ext8             v18.16b, v2.16b, v20.16b, #8      
        mov             v2.d[1], v20.d[1]                    

        fsub            v1.4s, v2.4s, v18.4s             
        fadd            v18.4s, v2.4s, v18.4s             

        rev64           v22.4s, v31.4s                           
        dup             v2.2d, v16.d[0]                      
        dup             v3.2d, v16.d[1]                      

        rev64           v20.4s, v1.4s                        
        rev64           v3.4s, v3.4s                        

        ext8             v22.16b, v31.16b, v22.16b, #8         
        zip1            v19.2d, v18.2d, v1.2d             
        zip2            v18.2d, v18.2d, v20.2d             

        fadd            v0.4s, v19.4s, v18.4s             
        fsub            v1.4s, v19.4s, v18.4s             

        fmul            v17.4s, v17.4s, v21.4s             
        fmls            v2.4s, v3.4s, v22.4s             

        rev64           v20.4s, v17.4s                        
        fmla            v17.4s, v20.4s, v31.4s                

        rev64           v20.4s, v17.4s                        
        ext8             v20.16b, v20.16b, v20.16b, #8      

        fmla            v20.4s, v17.4s, v31.4s                

        fadd            v3.4s, v2.4s, v20.4s             
        fsub            v2.4s, v2.4s, v20.4s             

        fadd            v16.4s, v4.4s, v6.4s                     
        fadd            v18.4s, v5.4s, v7.4s                     
        fsub            v4.4s, v4.4s, v6.4s                     
        fsub            v5.4s, v5.4s, v7.4s                     

        rev64           v20.4s, v4.4s
        rev64           v21.4s, v5.4s

        zip2            v6.2d, v16.2d, v20.2d                     
        zip2            v7.2d, v18.2d, v21.2d                     

        fneg            v19.4s, v20.4s
        fneg            v20.4s, v21.4s

        mov             v6.s[3], v19.s[3]
        mov             v7.s[3], v20.s[3]

        zip1            v17.2d, v16.2d, v4.2d                     
        zip1            v19.2d, v18.2d, v5.2d                     

        fsub            v20.4s, v17.4s, v6.4s                     
        fadd            v21.4s, v17.4s, v6.4s                     
        fsub            v18.4s, v19.4s, v7.4s                     
        fadd            v19.4s, v19.4s, v7.4s                     

        zip1            v4.2d, v21.2d, v20.2d                     
        zip2            v6.2d, v21.2d, v20.2d                     
        zip1            v5.2d, v19.2d, v18.2d                     
        zip2            v7.2d, v19.2d, v18.2d                     











        adrp            x5, tab_16pt
        add             x5, x5, tab_16pt











        rev64           v16.4s, v4.4s                     
        rev64           v17.4s, v5.4s                     

        ins             v16.d[0], xzr
        ins             v17.d[0], xzr

        ld1             { v20.4s, v21.4s, v22.4s }, [x5]
        

        fmla            v5.4s, v17.4s, v31.4s             
        fmls            v4.4s, v16.4s, v31.4s             

        fmul            v18.4s, v6.4s, v20.4s
        fmul            v19.4s, v7.4s, v20.4s

        rev64           v7.4s, v7.4s
        rev64           v6.4s, v6.4s

        fmla            v19.4s, v7.4s, v21.4s          
        fmls            v18.4s, v6.4s, v21.4s          

        fmul            v17.4s, v5.4s, v22.4s          
        fmul            v16.4s, v4.4s, v22.4s          

        orr v6.16b, v19.16b, v19.16b
        orr v5.16b, v17.16b, v17.16b

        fsub            v19.4s, v19.4s, v18.4s          
        fsub            v17.4s, v17.4s, v16.4s          

        fadd            v18.4s, v18.4s, v6.4s          
        rev64           v19.4s, v19.4s
        fadd            v16.4s, v16.4s, v5.4s          
        rev64           v17.4s, v17.4s

        fmul            v18.4s, v18.4s, v31.4s
        fmul            v17.4s, v17.4s, v31.4s

        fadd            v7.4s, v3.4s, v19.4s
        fsub            v5.4s, v3.4s, v19.4s
        fsub            v6.4s, v2.4s, v18.4s
        fadd            v4.4s, v2.4s, v18.4s

        fsub            v2.4s, v0.4s, v16.4s
        fadd            v0.4s, v0.4s, v16.4s
        fsub            v3.4s, v1.4s, v17.4s
        fadd            v1.4s, v1.4s, v17.4s


        ld1             { v24.4s, v25.4s }, [x8], x9
        ld1             { v16.4s, v17.4s }, [x7], #32

        ext8             v18.16b, v24.16b, v24.16b, #8
        ext8             v19.16b, v25.16b, v25.16b, #8

        trn1            v20.4s, v16.4s, v16.4s      
        trn2            v16.4s, v16.4s, v16.4s      
        trn1            v21.4s, v17.4s, v17.4s      
        trn2            v17.4s, v17.4s, v17.4s      

        rev64           v22.4s, v8.4s                 
        rev64           v23.4s, v10.4s                 
        rev64           v24.4s, v12.4s                 
        rev64           v25.4s, v14.4s                 

        fmul            v22.4s, v22.4s, v20.4s      
        fmul            v23.4s, v23.4s, v16.4s      
        fmul            v24.4s, v24.4s, v20.4s      
        fmul            v25.4s, v25.4s, v16.4s      

        rev64           v26.4s, v9.4s                 
        rev64           v27.4s, v11.4s                 
        rev64           v20.4s, v13.4s                 
        rev64           v16.4s, v15.4s                 

        fmul            v26.4s, v26.4s, v21.4s      
        fmul            v27.4s, v27.4s, v17.4s      
        fmul            v20.4s, v20.4s, v21.4s      
        fmul            v16.4s, v16.4s, v17.4s      

        trn1            v21.4s, v19.4s, v19.4s      
        trn2            v19.4s, v19.4s, v19.4s      
        trn1            v17.4s, v18.4s, v18.4s      
        trn2            v18.4s, v18.4s, v18.4s      

        fmul            v21.4s, v21.4s, v31.4s
        fmul            v19.4s, v19.4s, v31.4s
        fmul            v17.4s, v17.4s, v31.4s
        fmul            v18.4s, v18.4s, v31.4s

        fmla            v23.4s, v10.4s, v21.4s      
        fmls            v25.4s, v14.4s, v21.4s      
        fmla            v22.4s, v8.4s, v19.4s      
        fmls            v24.4s, v12.4s, v19.4s      

        fmla            v26.4s, v9.4s, v18.4s      
        fmla            v27.4s, v11.4s, v17.4s      
        fmls            v20.4s, v13.4s, v18.4s      
        fmls            v16.4s, v15.4s, v17.4s      

        fsub            v18.4s, v23.4s, v25.4s
        fsub            v17.4s, v24.4s, v22.4s
        fsub            v19.4s, v20.4s, v26.4s
        fsub            v21.4s, v16.4s, v27.4s

        fadd            v22.4s, v24.4s, v22.4s
        fadd            v23.4s, v25.4s, v23.4s
        fadd            v24.4s, v20.4s, v26.4s
        fadd            v25.4s, v16.4s, v27.4s

        fmul            v17.4s, v17.4s, v31.4s
        fmul            v18.4s, v18.4s, v31.4s
        fmul            v19.4s, v19.4s, v31.4s
        fmul            v21.4s, v21.4s, v31.4s

        rev64           v22.4s, v22.4s
        rev64           v24.4s, v24.4s
        rev64           v23.4s, v23.4s
        rev64           v25.4s, v25.4s

        fsub            v8.4s, v0.4s, v22.4s
        fsub            v9.4s, v1.4s, v24.4s
        fsub            v10.4s, v2.4s, v17.4s
        fsub            v11.4s, v3.4s, v19.4s

        fsub            v12.4s, v4.4s, v23.4s
        fsub            v13.4s, v5.4s, v25.4s
        fadd            v14.4s, v6.4s, v18.4s
        fsub            v15.4s, v7.4s, v21.4s

        fadd            v0.4s, v0.4s, v22.4s
        fadd            v1.4s, v1.4s, v24.4s
        fadd            v2.4s, v2.4s, v17.4s
        fadd            v3.4s, v3.4s, v19.4s

        fadd            v4.4s, v4.4s, v23.4s
        fsub            v6.4s, v6.4s, v18.4s      
        fadd            v5.4s, v5.4s, v25.4s      
        fadd            v7.4s, v7.4s, v21.4s

        stp             q2,  q3,  [x1, #32*1]
        stp             q6,  q7,  [x1, #32*3]
        stp             q10, q11, [x1, #32*5]
        stp             q14, q15, [x1, #32*7]

        cmp             w20, #32
        bgt            temp_label_1

        stp             q0,  q1,  [x1, #32*0]
        stp             q4,  q5,  [x1, #32*2]
        stp             q8,  q9,  [x1, #32*4]
        stp             q12, q13, [x1, #32*6]

        ret
temp_label_2
temp_label_1

        ldr             w5, =(64 - 4*7)










        adrp            x7, ff_tx_tab_64_float
        add             x7, x7, ff_tx_tab_64_float










        add             x8, x7, x5
        mov             x9, #-32

        mov             x21, #2*64
        add             x22, x21, x21, lsl #1

        ldp             w10, w11, [x4, #0 ]
        ldp             w12, w13, [x4, #8 ]
        ldp             w14, w15, [x4, #16]
        ldp             w16, w17, [x4, #24]

        add             x4, x4, #32

        ldr             d2, [x2, x10, lsl #3]
        add             x11, x2, x11, lsl #3
        ldr             d3, [x2, x12, lsl #3]
        add             x13, x2, x13, lsl #3
        ldr             d10, [x2, x14, lsl #3]
        add             x15, x2, x15, lsl #3
        ldr             d11, [x2, x16, lsl #3]
        add             x17, x2, x17, lsl #3

        ld1             { v2.d }[1], [x11]
        ld1             { v3.d }[1], [x13]
        ld1             { v10.d }[1], [x15]
        ld1             { v11.d }[1], [x17]
        ldp             w10, w11, [x4, #0 ]
        ldp             w12, w13, [x4, #8 ]
        ldp             w14, w15, [x4, #16]
        ldp             w16, w17, [x4, #24]

        add             x4, x4, #32

        ldr             d6, [x2, x10, lsl #3]
        add             x11, x2, x11, lsl #3
        ldr             d14, [x2, x12, lsl #3]
        add             x13, x2, x13, lsl #3
        ldr             d7, [x2, x14, lsl #3]
        add             x15, x2, x15, lsl #3
        ldr             d15, [x2, x16, lsl #3]
        add             x17, x2, x17, lsl #3

        ld1             { v6.d }[1], [x11]
        ld1             { v14.d }[1], [x13]
        ld1             { v7.d }[1], [x15]
        ld1             { v15.d }[1], [x17]













        adrp            x5, tab_8pt
        add             x5, x5, tab_8pt











        fsub            v17.4s, v3.4s, v11.4s             
        fadd            v11.4s, v3.4s, v11.4s             
        fsub            v16.4s, v2.4s, v10.4s             
        fadd            v10.4s, v2.4s, v10.4s             

        ld1             { v21.4s }, [x5]

        ext8             v20.16b, v11.16b, v11.16b, #12
        rev64           v20.4s,  v20.4s

        ext8             v18.16b, v10.16b, v20.16b, #8      
        mov             v10.d[1], v20.d[1]                    

        fsub            v3.4s, v10.4s, v18.4s             
        fadd            v18.4s, v10.4s, v18.4s             

        rev64           v22.4s, v31.4s                           
        dup             v10.2d, v16.d[0]                      
        dup             v11.2d, v16.d[1]                      

        rev64           v20.4s, v3.4s                        
        rev64           v11.4s, v11.4s                        

        ext8             v22.16b, v31.16b, v22.16b, #8         
        zip1            v19.2d, v18.2d, v3.2d             
        zip2            v18.2d, v18.2d, v20.2d             

        fadd            v2.4s, v19.4s, v18.4s             
        fsub            v3.4s, v19.4s, v18.4s             

        fmul            v17.4s, v17.4s, v21.4s             
        fmls            v10.4s, v11.4s, v22.4s             

        rev64           v20.4s, v17.4s                        
        fmla            v17.4s, v20.4s, v31.4s                

        rev64           v20.4s, v17.4s                        
        ext8             v20.16b, v20.16b, v20.16b, #8      

        fmla            v20.4s, v17.4s, v31.4s                

        fadd            v11.4s, v10.4s, v20.4s             
        fsub            v10.4s, v10.4s, v20.4s             

        fadd            v16.4s, v6.4s, v14.4s                     
        fadd            v18.4s, v7.4s, v15.4s                     
        fsub            v6.4s, v6.4s, v14.4s                     
        fsub            v7.4s, v7.4s, v15.4s                     

        rev64           v20.4s, v6.4s
        rev64           v21.4s, v7.4s

        zip2            v14.2d, v16.2d, v20.2d                     
        zip2            v15.2d, v18.2d, v21.2d                     

        fneg            v19.4s, v20.4s
        fneg            v20.4s, v21.4s

        mov             v14.s[3], v19.s[3]
        mov             v15.s[3], v20.s[3]

        zip1            v17.2d, v16.2d, v6.2d                     
        zip1            v19.2d, v18.2d, v7.2d                     

        fsub            v20.4s, v17.4s, v14.4s                     
        fadd            v21.4s, v17.4s, v14.4s                     
        fsub            v18.4s, v19.4s, v15.4s                     
        fadd            v19.4s, v19.4s, v15.4s                     

        zip1            v6.2d, v21.2d, v20.2d                     
        zip2            v14.2d, v21.2d, v20.2d                     
        zip1            v7.2d, v19.2d, v18.2d                     
        zip2            v15.2d, v19.2d, v18.2d                     











        adrp            x5, tab_16pt
        add             x5, x5, tab_16pt











        rev64           v16.4s, v6.4s                     
        rev64           v17.4s, v7.4s                     

        ins             v16.d[0], xzr
        ins             v17.d[0], xzr

        ld1             { v20.4s, v21.4s, v22.4s }, [x5]
        

        fmla            v7.4s, v17.4s, v31.4s             
        fmls            v6.4s, v16.4s, v31.4s             

        fmul            v18.4s, v14.4s, v20.4s
        fmul            v19.4s, v15.4s, v20.4s

        rev64           v15.4s, v15.4s
        rev64           v14.4s, v14.4s

        fmla            v19.4s, v15.4s, v21.4s          
        fmls            v18.4s, v14.4s, v21.4s          

        fmul            v17.4s, v7.4s, v22.4s          
        fmul            v16.4s, v6.4s, v22.4s          

        orr v14.16b, v19.16b, v19.16b
        orr v7.16b, v17.16b, v17.16b

        fsub            v19.4s, v19.4s, v18.4s          
        fsub            v17.4s, v17.4s, v16.4s          

        fadd            v18.4s, v18.4s, v14.4s          
        rev64           v19.4s, v19.4s
        fadd            v16.4s, v16.4s, v7.4s          
        rev64           v17.4s, v17.4s

        fmul            v18.4s, v18.4s, v31.4s
        fmul            v17.4s, v17.4s, v31.4s

        fadd            v15.4s, v11.4s, v19.4s
        fsub            v7.4s, v11.4s, v19.4s
        fsub            v14.4s, v10.4s, v18.4s
        fadd            v6.4s, v10.4s, v18.4s

        fsub            v10.4s, v2.4s, v16.4s
        fadd            v2.4s, v2.4s, v16.4s
        fsub            v11.4s, v3.4s, v17.4s
        fadd            v3.4s, v3.4s, v17.4s

        ldp             w10, w11, [x4, #0 ]
        ldp             w12, w13, [x4, #8 ]
        ldp             w14, w15, [x4, #16]
        ldp             w16, w17, [x4, #24]

        add             x4, x4, #32

        ldr             d16, [x2, x10, lsl #3]
        add             x11, x2, x11, lsl #3
        ldr             d17, [x2, x12, lsl #3]
        add             x13, x2, x13, lsl #3
        ldr             d18, [x2, x14, lsl #3]
        add             x15, x2, x15, lsl #3
        ldr             d19, [x2, x16, lsl #3]
        add             x17, x2, x17, lsl #3

        ld1             { v16.d }[1], [x11]
        ld1             { v17.d }[1], [x13]
        ld1             { v18.d }[1], [x15]
        ld1             { v19.d }[1], [x17]
        ldp             w10, w11, [x4, #0 ]
        ldp             w12, w13, [x4, #8 ]
        ldp             w14, w15, [x4, #16]
        ldp             w16, w17, [x4, #24]

        add             x4, x4, #32

        ldr             d20, [x2, x10, lsl #3]
        add             x11, x2, x11, lsl #3
        ldr             d22, [x2, x12, lsl #3]
        add             x13, x2, x13, lsl #3
        ldr             d21, [x2, x14, lsl #3]
        add             x15, x2, x15, lsl #3
        ldr             d23, [x2, x16, lsl #3]
        add             x17, x2, x17, lsl #3

        ld1             { v20.d }[1], [x11]
        ld1             { v22.d }[1], [x13]
        ld1             { v21.d }[1], [x15]
        ld1             { v23.d }[1], [x17]













        adrp            x5, tab_8pt
        add             x5, x5, tab_8pt











        fsub            v25.4s, v17.4s, v19.4s             
        fadd            v19.4s, v17.4s, v19.4s             
        fsub            v24.4s, v16.4s, v18.4s             
        fadd            v18.4s, v16.4s, v18.4s             

        ld1             { v29.4s }, [x5]

        ext8             v28.16b, v19.16b, v19.16b, #12
        rev64           v28.4s,  v28.4s

        ext8             v26.16b, v18.16b, v28.16b, #8      
        mov             v18.d[1], v28.d[1]                    

        fsub            v17.4s, v18.4s, v26.4s             
        fadd            v26.4s, v18.4s, v26.4s             

        rev64           v30.4s, v31.4s                           
        dup             v18.2d, v24.d[0]                      
        dup             v19.2d, v24.d[1]                      

        rev64           v28.4s, v17.4s                        
        rev64           v19.4s, v19.4s                        

        ext8             v30.16b, v31.16b, v30.16b, #8         
        zip1            v27.2d, v26.2d, v17.2d             
        zip2            v26.2d, v26.2d, v28.2d             

        fadd            v16.4s, v27.4s, v26.4s             
        fsub            v17.4s, v27.4s, v26.4s             

        fmul            v25.4s, v25.4s, v29.4s             
        fmls            v18.4s, v19.4s, v30.4s             

        rev64           v28.4s, v25.4s                        
        fmla            v25.4s, v28.4s, v31.4s                

        rev64           v28.4s, v25.4s                        
        ext8             v28.16b, v28.16b, v28.16b, #8      

        fmla            v28.4s, v25.4s, v31.4s                

        fadd            v19.4s, v18.4s, v28.4s             
        fsub            v18.4s, v18.4s, v28.4s             

        fadd            v24.4s, v20.4s, v22.4s                     
        fadd            v26.4s, v21.4s, v23.4s                     
        fsub            v20.4s, v20.4s, v22.4s                     
        fsub            v21.4s, v21.4s, v23.4s                     

        rev64           v28.4s, v20.4s
        rev64           v29.4s, v21.4s

        zip2            v22.2d, v24.2d, v28.2d                     
        zip2            v23.2d, v26.2d, v29.2d                     

        fneg            v27.4s, v28.4s
        fneg            v28.4s, v29.4s

        mov             v22.s[3], v27.s[3]
        mov             v23.s[3], v28.s[3]

        zip1            v25.2d, v24.2d, v20.2d                     
        zip1            v27.2d, v26.2d, v21.2d                     

        fsub            v28.4s, v25.4s, v22.4s                     
        fadd            v29.4s, v25.4s, v22.4s                     
        fsub            v26.4s, v27.4s, v23.4s                     
        fadd            v27.4s, v27.4s, v23.4s                     

        zip1            v20.2d, v29.2d, v28.2d                     
        zip2            v22.2d, v29.2d, v28.2d                     
        zip1            v21.2d, v27.2d, v26.2d                     
        zip2            v23.2d, v27.2d, v26.2d                     











        adrp            x5, tab_16pt
        add             x5, x5, tab_16pt











        rev64           v24.4s, v20.4s                     
        rev64           v25.4s, v21.4s                     

        ins             v24.d[0], xzr
        ins             v25.d[0], xzr

        ld1             { v28.4s, v29.4s, v30.4s }, [x5]
        

        fmla            v21.4s, v25.4s, v31.4s             
        fmls            v20.4s, v24.4s, v31.4s             

        fmul            v26.4s, v22.4s, v28.4s
        fmul            v27.4s, v23.4s, v28.4s

        rev64           v23.4s, v23.4s
        rev64           v22.4s, v22.4s

        fmla            v27.4s, v23.4s, v29.4s          
        fmls            v26.4s, v22.4s, v29.4s          

        fmul            v25.4s, v21.4s, v30.4s          
        fmul            v24.4s, v20.4s, v30.4s          

        orr v22.16b, v27.16b, v27.16b
        orr v21.16b, v25.16b, v25.16b

        fsub            v27.4s, v27.4s, v26.4s          
        fsub            v25.4s, v25.4s, v24.4s          

        fadd            v26.4s, v26.4s, v22.4s          
        rev64           v27.4s, v27.4s
        fadd            v24.4s, v24.4s, v21.4s          
        rev64           v25.4s, v25.4s

        fmul            v26.4s, v26.4s, v31.4s
        fmul            v25.4s, v25.4s, v31.4s

        fadd            v23.4s, v19.4s, v27.4s
        fsub            v21.4s, v19.4s, v27.4s
        fsub            v22.4s, v18.4s, v26.4s
        fadd            v20.4s, v18.4s, v26.4s

        fsub            v18.4s, v16.4s, v24.4s
        fadd            v16.4s, v16.4s, v24.4s
        fsub            v19.4s, v17.4s, v25.4s
        fadd            v17.4s, v17.4s, v25.4s

        ld1             { v26.4s, v27.4s }, [x8], x9
        ldp             q24, q25, [x7], #32

        ext8             v26.16b, v26.16b, v26.16b, #8
        ext8             v27.16b, v27.16b, v27.16b, #8

        cmp             w19, #64
        beq            temp_label_3 

        
        

        rev64           v28.4s, v2.4s              
        rev64           v29.4s, v16.4s              
        trn2            v30.4s, v27.4s, v27.4s   
        fmul            v30.4s, v30.4s, v31.4s
        fmul            v16.4s, v16.4s, v30.4s
        fmul            v2.4s, v2.4s, v30.4s   
        trn1            v30.4s, v24.4s, v24.4s   
        fmul            v29.4s, v29.4s, v30.4s   
        fmla            v2.4s, v28.4s, v30.4s   
        fsub            v29.4s, v29.4s, v16.4s   

        rev64           v30.4s, v3.4s              
        rev64           v16.4s, v17.4s              

        trn2            v28.4s, v26.4s, v26.4s   
        fmul            v28.4s, v28.4s, v31.4s

        fmul            v3.4s, v3.4s, v28.4s   
        fmul            v17.4s, v17.4s, v28.4s

        trn1            v28.4s, v25.4s, v25.4s   
        fmul            v16.4s, v16.4s, v28.4s   
        fmla            v3.4s, v30.4s, v28.4s   
        fsub            v16.4s, v16.4s, v17.4s   

        fsub            v28.4s, v29.4s, v2.4s
        fadd            v2.4s, v29.4s, v2.4s
        fadd            v30.4s, v16.4s, v3.4s
        fsub            v29.4s, v16.4s, v3.4s

        fmul            v28.4s, v28.4s, v31.4s
        fmul            v29.4s, v29.4s, v31.4s

        rev64           v30.4s, v30.4s
        rev64           v2.4s, v2.4s

        fsub            v3.4s, v1.4s, v30.4s
        fsub            v16.4s, v8.4s, v28.4s
        fsub            v17.4s, v9.4s, v29.4s

        fadd            v1.4s, v1.4s, v30.4s
        fadd            v8.4s, v8.4s, v28.4s
        fadd            v9.4s, v9.4s, v29.4s

        orr v29.16b, v2.16b, v2.16b

        fsub            v2.4s, v0.4s, v29.4s
        fadd            v0.4s, v0.4s, v29.4s

        stp             q0,  q1,  [x1, #32* 0]
        stp             q8,  q9,  [x1, #32* 4]
        stp             q2,  q3,  [x1, #32* 8]
        stp             q16, q17, [x1, #32*12]


        trn2            v1.4s, v24.4s, v24.4s   
        trn2            v25.4s, v25.4s, v25.4s   
        trn1            v8.4s, v26.4s, v26.4s   
        trn1            v27.4s, v27.4s, v27.4s   

        fmul            v8.4s, v8.4s, v31.4s
        fmul            v27.4s, v27.4s, v31.4s

        rev64           v28.4s, v6.4s              
        rev64           v29.4s, v20.4s              
        rev64           v30.4s, v7.4s              
        rev64           v0.4s, v21.4s              

        fmul            v6.4s, v6.4s, v27.4s   
        fmul            v7.4s, v7.4s, v8.4s   
        fmla            v6.4s, v28.4s, v1.4s   
        fmla            v7.4s, v30.4s, v25.4s   

        fmul            v29.4s, v29.4s, v1.4s   
        fmul            v0.4s, v0.4s, v25.4s   
        fmls            v29.4s, v20.4s, v27.4s   
        fmls            v0.4s, v21.4s, v8.4s   

        fsub            v28.4s, v29.4s, v6.4s
        fadd            v29.4s, v29.4s, v6.4s
        fadd            v30.4s, v0.4s, v7.4s
        fsub            v0.4s, v0.4s, v7.4s

        fmul            v28.4s, v28.4s, v31.4s
        fmul            v0.4s, v0.4s, v31.4s

        rev64           v29.4s, v29.4s
        rev64           v30.4s, v30.4s

        fsub            v6.4s, v4.4s, v29.4s
        fadd            v20.4s, v5.4s, v30.4s
        fsub            v7.4s, v12.4s, v28.4s
        fadd            v21.4s, v13.4s, v0.4s

        fadd            v4.4s, v4.4s, v29.4s
        fsub            v5.4s, v5.4s, v30.4s   
        fadd            v12.4s, v12.4s, v28.4s   
        fsub            v13.4s, v13.4s, v0.4s

        stp             q4,  q20, [x1, #32* 2]
        stp             q12, q21, [x1, #32* 6]
        stp             q6,  q5,  [x1, #32*10]
        stp             q7,  q13, [x1, #32*14]

        ldp             q2,  q3,  [x1, #32*1]
        ldp             q6,  q7,  [x1, #32*3]
        ldp             q12, q13, [x1, #32*5]
        ldp             q16, q17, [x1, #32*7]


        ld1             { v0.4s, v1.4s }, [x8], x9
        ld1             { v24.4s, v25.4s }, [x7], #32

        ext8             v26.16b, v0.16b, v0.16b, #8
        ext8             v27.16b, v1.16b, v1.16b, #8

        trn1            v28.4s, v24.4s, v24.4s      
        trn2            v24.4s, v24.4s, v24.4s      
        trn1            v29.4s, v25.4s, v25.4s      
        trn2            v25.4s, v25.4s, v25.4s      

        rev64           v30.4s, v10.4s                 
        rev64           v8.4s, v14.4s                 
        rev64           v0.4s, v18.4s                 
        rev64           v1.4s, v22.4s                 

        fmul            v30.4s, v30.4s, v28.4s      
        fmul            v8.4s, v8.4s, v24.4s      
        fmul            v0.4s, v0.4s, v28.4s      
        fmul            v1.4s, v1.4s, v24.4s      

        rev64           v4.4s, v11.4s                 
        rev64           v5.4s, v15.4s                 
        rev64           v28.4s, v19.4s                 
        rev64           v24.4s, v23.4s                 

        fmul            v4.4s, v4.4s, v29.4s      
        fmul            v5.4s, v5.4s, v25.4s      
        fmul            v28.4s, v28.4s, v29.4s      
        fmul            v24.4s, v24.4s, v25.4s      

        trn1            v29.4s, v27.4s, v27.4s      
        trn2            v27.4s, v27.4s, v27.4s      
        trn1            v25.4s, v26.4s, v26.4s      
        trn2            v26.4s, v26.4s, v26.4s      

        fmul            v29.4s, v29.4s, v31.4s
        fmul            v27.4s, v27.4s, v31.4s
        fmul            v25.4s, v25.4s, v31.4s
        fmul            v26.4s, v26.4s, v31.4s

        fmla            v8.4s, v14.4s, v29.4s      
        fmls            v1.4s, v22.4s, v29.4s      
        fmla            v30.4s, v10.4s, v27.4s      
        fmls            v0.4s, v18.4s, v27.4s      

        fmla            v4.4s, v11.4s, v26.4s      
        fmla            v5.4s, v15.4s, v25.4s      
        fmls            v28.4s, v19.4s, v26.4s      
        fmls            v24.4s, v23.4s, v25.4s      

        fsub            v26.4s, v8.4s, v1.4s
        fsub            v25.4s, v0.4s, v30.4s
        fsub            v27.4s, v28.4s, v4.4s
        fsub            v29.4s, v24.4s, v5.4s

        fadd            v30.4s, v0.4s, v30.4s
        fadd            v8.4s, v1.4s, v8.4s
        fadd            v0.4s, v28.4s, v4.4s
        fadd            v1.4s, v24.4s, v5.4s

        fmul            v25.4s, v25.4s, v31.4s
        fmul            v26.4s, v26.4s, v31.4s
        fmul            v27.4s, v27.4s, v31.4s
        fmul            v29.4s, v29.4s, v31.4s

        rev64           v30.4s, v30.4s
        rev64           v0.4s, v0.4s
        rev64           v8.4s, v8.4s
        rev64           v1.4s, v1.4s

        fsub            v10.4s, v2.4s, v30.4s
        fsub            v11.4s, v3.4s, v0.4s
        fsub            v14.4s, v12.4s, v25.4s
        fsub            v15.4s, v13.4s, v27.4s

        fsub            v18.4s, v6.4s, v8.4s
        fsub            v19.4s, v7.4s, v1.4s
        fadd            v22.4s, v16.4s, v26.4s
        fsub            v23.4s, v17.4s, v29.4s

        fadd            v2.4s, v2.4s, v30.4s
        fadd            v3.4s, v3.4s, v0.4s
        fadd            v12.4s, v12.4s, v25.4s
        fadd            v13.4s, v13.4s, v27.4s

        fadd            v6.4s, v6.4s, v8.4s
        fsub            v16.4s, v16.4s, v26.4s      
        fadd            v7.4s, v7.4s, v1.4s      
        fadd            v17.4s, v17.4s, v29.4s

        stp             q2,  q3,  [x1, #32* 1]
        stp             q6,  q7,  [x1, #32* 3]
        stp             q12, q13, [x1, #32* 5]
        stp             q16, q17, [x1, #32* 7]

        stp             q10, q11, [x1, #32* 9]
        stp             q18, q19, [x1, #32*11]
        stp             q14, q15, [x1, #32*13]
        stp             q22, q23, [x1, #32*15]

        cmp             w20, #64
        bgt            temp_label_4
        ret
temp_label_5
temp_label_4

        stp             x20, x30, [sp, #-16]!
        mov             w20, #32
        add             x1, x1, #16*32
        bl              temp_label_0
        add             x1, x1, #8*32
        bl              temp_label_0
        ldp             x20, x30, [sp], #16
        sub             x1, x1, #24*32

        ldr             w5, =(128 - 4*7)










        adrp            x7, ff_tx_tab_128_float
        add             x7, x7, ff_tx_tab_128_float










        add             x8, x7, x5
        mov             x9, #-32

        mov             x21, #2*128
        add             x22, x21, x21, lsl #1

        cmp             w19, #128
        beq            temp_label_6

        add             x10, x1, x21
        add             x11, x1, x21, lsl #1
        add             x12, x1, x22

        add             x10, x1, x21
        add             x11, x1, x21, lsl #1
        add             x12, x1, x22

        ldp             q0,  q1,  [x1,  #((0 + 0)*32 + 0)]
        ldp             q4,  q5,  [x1,  #((2 + 0)*32 + 0)]
        ldp             q2,  q3,  [x10, #((0 + 0)*32 + 0)]
        ldp             q6,  q7,  [x10, #((2 + 0)*32 + 0)]

        ldp             q8,  q9,  [x11, #((0 + 0)*32 + 0)]
        ldp             q10, q11, [x11, #((2 + 0)*32 + 0)]
        ldp             q12, q13, [x12, #((0 + 0)*32 + 0)]
        ldp             q14, q15, [x12, #((2 + 0)*32 + 0)]


        ld1             { v24.4s, v25.4s }, [x8], x9
        ld1             { v16.4s, v17.4s }, [x7], #32

        ext8             v18.16b, v24.16b, v24.16b, #8
        ext8             v19.16b, v25.16b, v25.16b, #8

        trn1            v20.4s, v16.4s, v16.4s      
        trn2            v16.4s, v16.4s, v16.4s      
        trn1            v21.4s, v17.4s, v17.4s      
        trn2            v17.4s, v17.4s, v17.4s      

        rev64           v22.4s, v8.4s                 
        rev64           v23.4s, v10.4s                 
        rev64           v24.4s, v12.4s                 
        rev64           v25.4s, v14.4s                 

        fmul            v22.4s, v22.4s, v20.4s      
        fmul            v23.4s, v23.4s, v16.4s      
        fmul            v24.4s, v24.4s, v20.4s      
        fmul            v25.4s, v25.4s, v16.4s      

        rev64           v26.4s, v9.4s                 
        rev64           v27.4s, v11.4s                 
        rev64           v20.4s, v13.4s                 
        rev64           v16.4s, v15.4s                 

        fmul            v26.4s, v26.4s, v21.4s      
        fmul            v27.4s, v27.4s, v17.4s      
        fmul            v20.4s, v20.4s, v21.4s      
        fmul            v16.4s, v16.4s, v17.4s      

        trn1            v21.4s, v19.4s, v19.4s      
        trn2            v19.4s, v19.4s, v19.4s      
        trn1            v17.4s, v18.4s, v18.4s      
        trn2            v18.4s, v18.4s, v18.4s      

        fmul            v21.4s, v21.4s, v31.4s
        fmul            v19.4s, v19.4s, v31.4s
        fmul            v17.4s, v17.4s, v31.4s
        fmul            v18.4s, v18.4s, v31.4s

        fmla            v23.4s, v10.4s, v21.4s      
        fmls            v25.4s, v14.4s, v21.4s      
        fmla            v22.4s, v8.4s, v19.4s      
        fmls            v24.4s, v12.4s, v19.4s      

        fmla            v26.4s, v9.4s, v18.4s      
        fmla            v27.4s, v11.4s, v17.4s      
        fmls            v20.4s, v13.4s, v18.4s      
        fmls            v16.4s, v15.4s, v17.4s      

        fsub            v18.4s, v23.4s, v25.4s
        fsub            v17.4s, v24.4s, v22.4s
        fsub            v19.4s, v20.4s, v26.4s
        fsub            v21.4s, v16.4s, v27.4s

        fadd            v22.4s, v24.4s, v22.4s
        fadd            v23.4s, v25.4s, v23.4s
        fadd            v24.4s, v20.4s, v26.4s
        fadd            v25.4s, v16.4s, v27.4s

        fmul            v17.4s, v17.4s, v31.4s
        fmul            v18.4s, v18.4s, v31.4s
        fmul            v19.4s, v19.4s, v31.4s
        fmul            v21.4s, v21.4s, v31.4s

        rev64           v22.4s, v22.4s
        rev64           v24.4s, v24.4s
        rev64           v23.4s, v23.4s
        rev64           v25.4s, v25.4s

        fsub            v8.4s, v0.4s, v22.4s
        fsub            v9.4s, v1.4s, v24.4s
        fsub            v10.4s, v2.4s, v17.4s
        fsub            v11.4s, v3.4s, v19.4s

        fsub            v12.4s, v4.4s, v23.4s
        fsub            v13.4s, v5.4s, v25.4s
        fadd            v14.4s, v6.4s, v18.4s
        fsub            v15.4s, v7.4s, v21.4s

        fadd            v0.4s, v0.4s, v22.4s
        fadd            v1.4s, v1.4s, v24.4s
        fadd            v2.4s, v2.4s, v17.4s
        fadd            v3.4s, v3.4s, v19.4s

        fadd            v4.4s, v4.4s, v23.4s
        fsub            v6.4s, v6.4s, v18.4s      
        fadd            v5.4s, v5.4s, v25.4s      
        fadd            v7.4s, v7.4s, v21.4s

        stp             q0,  q1,  [x1,  #((0 + 0)*32 + 0)]
        stp             q4,  q5,  [x1,  #((2 + 0)*32 + 0)]
        stp             q2,  q3,  [x10, #((0 + 0)*32 + 0)]
        stp             q6,  q7,  [x10, #((2 + 0)*32 + 0)]

        stp             q8,  q9,  [x11, #((0 + 0)*32 + 0)]
        stp             q12, q13, [x11, #((2 + 0)*32 + 0)]
        stp             q10, q11, [x12, #((0 + 0)*32 + 0)]
        stp             q14, q15, [x12, #((2 + 0)*32 + 0)]
        add             x10, x1, x21
        add             x11, x1, x21, lsl #1
        add             x12, x1, x22

        ldp             q0,  q1,  [x1,  #((0 + 1)*32 + 0)]
        ldp             q4,  q5,  [x1,  #((2 + 1)*32 + 0)]
        ldp             q2,  q3,  [x10, #((0 + 1)*32 + 0)]
        ldp             q6,  q7,  [x10, #((2 + 1)*32 + 0)]

        ldp             q8,  q9,  [x11, #((0 + 1)*32 + 0)]
        ldp             q10, q11, [x11, #((2 + 1)*32 + 0)]
        ldp             q12, q13, [x12, #((0 + 1)*32 + 0)]
        ldp             q14, q15, [x12, #((2 + 1)*32 + 0)]


        ld1             { v24.4s, v25.4s }, [x8], x9
        ld1             { v16.4s, v17.4s }, [x7], #32

        ext8             v18.16b, v24.16b, v24.16b, #8
        ext8             v19.16b, v25.16b, v25.16b, #8

        trn1            v20.4s, v16.4s, v16.4s      
        trn2            v16.4s, v16.4s, v16.4s      
        trn1            v21.4s, v17.4s, v17.4s      
        trn2            v17.4s, v17.4s, v17.4s      

        rev64           v22.4s, v8.4s                 
        rev64           v23.4s, v10.4s                 
        rev64           v24.4s, v12.4s                 
        rev64           v25.4s, v14.4s                 

        fmul            v22.4s, v22.4s, v20.4s      
        fmul            v23.4s, v23.4s, v16.4s      
        fmul            v24.4s, v24.4s, v20.4s      
        fmul            v25.4s, v25.4s, v16.4s      

        rev64           v26.4s, v9.4s                 
        rev64           v27.4s, v11.4s                 
        rev64           v20.4s, v13.4s                 
        rev64           v16.4s, v15.4s                 

        fmul            v26.4s, v26.4s, v21.4s      
        fmul            v27.4s, v27.4s, v17.4s      
        fmul            v20.4s, v20.4s, v21.4s      
        fmul            v16.4s, v16.4s, v17.4s      

        trn1            v21.4s, v19.4s, v19.4s      
        trn2            v19.4s, v19.4s, v19.4s      
        trn1            v17.4s, v18.4s, v18.4s      
        trn2            v18.4s, v18.4s, v18.4s      

        fmul            v21.4s, v21.4s, v31.4s
        fmul            v19.4s, v19.4s, v31.4s
        fmul            v17.4s, v17.4s, v31.4s
        fmul            v18.4s, v18.4s, v31.4s

        fmla            v23.4s, v10.4s, v21.4s      
        fmls            v25.4s, v14.4s, v21.4s      
        fmla            v22.4s, v8.4s, v19.4s      
        fmls            v24.4s, v12.4s, v19.4s      

        fmla            v26.4s, v9.4s, v18.4s      
        fmla            v27.4s, v11.4s, v17.4s      
        fmls            v20.4s, v13.4s, v18.4s      
        fmls            v16.4s, v15.4s, v17.4s      

        fsub            v18.4s, v23.4s, v25.4s
        fsub            v17.4s, v24.4s, v22.4s
        fsub            v19.4s, v20.4s, v26.4s
        fsub            v21.4s, v16.4s, v27.4s

        fadd            v22.4s, v24.4s, v22.4s
        fadd            v23.4s, v25.4s, v23.4s
        fadd            v24.4s, v20.4s, v26.4s
        fadd            v25.4s, v16.4s, v27.4s

        fmul            v17.4s, v17.4s, v31.4s
        fmul            v18.4s, v18.4s, v31.4s
        fmul            v19.4s, v19.4s, v31.4s
        fmul            v21.4s, v21.4s, v31.4s

        rev64           v22.4s, v22.4s
        rev64           v24.4s, v24.4s
        rev64           v23.4s, v23.4s
        rev64           v25.4s, v25.4s

        fsub            v8.4s, v0.4s, v22.4s
        fsub            v9.4s, v1.4s, v24.4s
        fsub            v10.4s, v2.4s, v17.4s
        fsub            v11.4s, v3.4s, v19.4s

        fsub            v12.4s, v4.4s, v23.4s
        fsub            v13.4s, v5.4s, v25.4s
        fadd            v14.4s, v6.4s, v18.4s
        fsub            v15.4s, v7.4s, v21.4s

        fadd            v0.4s, v0.4s, v22.4s
        fadd            v1.4s, v1.4s, v24.4s
        fadd            v2.4s, v2.4s, v17.4s
        fadd            v3.4s, v3.4s, v19.4s

        fadd            v4.4s, v4.4s, v23.4s
        fsub            v6.4s, v6.4s, v18.4s      
        fadd            v5.4s, v5.4s, v25.4s      
        fadd            v7.4s, v7.4s, v21.4s

        stp             q0,  q1,  [x1,  #((0 + 1)*32 + 0)]
        stp             q4,  q5,  [x1,  #((2 + 1)*32 + 0)]
        stp             q2,  q3,  [x10, #((0 + 1)*32 + 0)]
        stp             q6,  q7,  [x10, #((2 + 1)*32 + 0)]

        stp             q8,  q9,  [x11, #((0 + 1)*32 + 0)]
        stp             q12, q13, [x11, #((2 + 1)*32 + 0)]
        stp             q10, q11, [x12, #((0 + 1)*32 + 0)]
        stp             q14, q15, [x12, #((2 + 1)*32 + 0)]
        add             x10, x1, x21
        add             x11, x1, x21, lsl #1
        add             x12, x1, x22

        ldp             q0,  q1,  [x1,  #((0 + 4)*32 + 0)]
        ldp             q4,  q5,  [x1,  #((2 + 4)*32 + 0)]
        ldp             q2,  q3,  [x10, #((0 + 4)*32 + 0)]
        ldp             q6,  q7,  [x10, #((2 + 4)*32 + 0)]

        ldp             q8,  q9,  [x11, #((0 + 4)*32 + 0)]
        ldp             q10, q11, [x11, #((2 + 4)*32 + 0)]
        ldp             q12, q13, [x12, #((0 + 4)*32 + 0)]
        ldp             q14, q15, [x12, #((2 + 4)*32 + 0)]


        ld1             { v24.4s, v25.4s }, [x8], x9
        ld1             { v16.4s, v17.4s }, [x7], #32

        ext8             v18.16b, v24.16b, v24.16b, #8
        ext8             v19.16b, v25.16b, v25.16b, #8

        trn1            v20.4s, v16.4s, v16.4s      
        trn2            v16.4s, v16.4s, v16.4s      
        trn1            v21.4s, v17.4s, v17.4s      
        trn2            v17.4s, v17.4s, v17.4s      

        rev64           v22.4s, v8.4s                 
        rev64           v23.4s, v10.4s                 
        rev64           v24.4s, v12.4s                 
        rev64           v25.4s, v14.4s                 

        fmul            v22.4s, v22.4s, v20.4s      
        fmul            v23.4s, v23.4s, v16.4s      
        fmul            v24.4s, v24.4s, v20.4s      
        fmul            v25.4s, v25.4s, v16.4s      

        rev64           v26.4s, v9.4s                 
        rev64           v27.4s, v11.4s                 
        rev64           v20.4s, v13.4s                 
        rev64           v16.4s, v15.4s                 

        fmul            v26.4s, v26.4s, v21.4s      
        fmul            v27.4s, v27.4s, v17.4s      
        fmul            v20.4s, v20.4s, v21.4s      
        fmul            v16.4s, v16.4s, v17.4s      

        trn1            v21.4s, v19.4s, v19.4s      
        trn2            v19.4s, v19.4s, v19.4s      
        trn1            v17.4s, v18.4s, v18.4s      
        trn2            v18.4s, v18.4s, v18.4s      

        fmul            v21.4s, v21.4s, v31.4s
        fmul            v19.4s, v19.4s, v31.4s
        fmul            v17.4s, v17.4s, v31.4s
        fmul            v18.4s, v18.4s, v31.4s

        fmla            v23.4s, v10.4s, v21.4s      
        fmls            v25.4s, v14.4s, v21.4s      
        fmla            v22.4s, v8.4s, v19.4s      
        fmls            v24.4s, v12.4s, v19.4s      

        fmla            v26.4s, v9.4s, v18.4s      
        fmla            v27.4s, v11.4s, v17.4s      
        fmls            v20.4s, v13.4s, v18.4s      
        fmls            v16.4s, v15.4s, v17.4s      

        fsub            v18.4s, v23.4s, v25.4s
        fsub            v17.4s, v24.4s, v22.4s
        fsub            v19.4s, v20.4s, v26.4s
        fsub            v21.4s, v16.4s, v27.4s

        fadd            v22.4s, v24.4s, v22.4s
        fadd            v23.4s, v25.4s, v23.4s
        fadd            v24.4s, v20.4s, v26.4s
        fadd            v25.4s, v16.4s, v27.4s

        fmul            v17.4s, v17.4s, v31.4s
        fmul            v18.4s, v18.4s, v31.4s
        fmul            v19.4s, v19.4s, v31.4s
        fmul            v21.4s, v21.4s, v31.4s

        rev64           v22.4s, v22.4s
        rev64           v24.4s, v24.4s
        rev64           v23.4s, v23.4s
        rev64           v25.4s, v25.4s

        fsub            v8.4s, v0.4s, v22.4s
        fsub            v9.4s, v1.4s, v24.4s
        fsub            v10.4s, v2.4s, v17.4s
        fsub            v11.4s, v3.4s, v19.4s

        fsub            v12.4s, v4.4s, v23.4s
        fsub            v13.4s, v5.4s, v25.4s
        fadd            v14.4s, v6.4s, v18.4s
        fsub            v15.4s, v7.4s, v21.4s

        fadd            v0.4s, v0.4s, v22.4s
        fadd            v1.4s, v1.4s, v24.4s
        fadd            v2.4s, v2.4s, v17.4s
        fadd            v3.4s, v3.4s, v19.4s

        fadd            v4.4s, v4.4s, v23.4s
        fsub            v6.4s, v6.4s, v18.4s      
        fadd            v5.4s, v5.4s, v25.4s      
        fadd            v7.4s, v7.4s, v21.4s

        stp             q0,  q1,  [x1,  #((0 + 4)*32 + 0)]
        stp             q4,  q5,  [x1,  #((2 + 4)*32 + 0)]
        stp             q2,  q3,  [x10, #((0 + 4)*32 + 0)]
        stp             q6,  q7,  [x10, #((2 + 4)*32 + 0)]

        stp             q8,  q9,  [x11, #((0 + 4)*32 + 0)]
        stp             q12, q13, [x11, #((2 + 4)*32 + 0)]
        stp             q10, q11, [x12, #((0 + 4)*32 + 0)]
        stp             q14, q15, [x12, #((2 + 4)*32 + 0)]
        add             x10, x1, x21
        add             x11, x1, x21, lsl #1
        add             x12, x1, x22

        ldp             q0,  q1,  [x1,  #((0 + 5)*32 + 0)]
        ldp             q4,  q5,  [x1,  #((2 + 5)*32 + 0)]
        ldp             q2,  q3,  [x10, #((0 + 5)*32 + 0)]
        ldp             q6,  q7,  [x10, #((2 + 5)*32 + 0)]

        ldp             q8,  q9,  [x11, #((0 + 5)*32 + 0)]
        ldp             q10, q11, [x11, #((2 + 5)*32 + 0)]
        ldp             q12, q13, [x12, #((0 + 5)*32 + 0)]
        ldp             q14, q15, [x12, #((2 + 5)*32 + 0)]


        ld1             { v24.4s, v25.4s }, [x8], x9
        ld1             { v16.4s, v17.4s }, [x7], #32

        ext8             v18.16b, v24.16b, v24.16b, #8
        ext8             v19.16b, v25.16b, v25.16b, #8

        trn1            v20.4s, v16.4s, v16.4s      
        trn2            v16.4s, v16.4s, v16.4s      
        trn1            v21.4s, v17.4s, v17.4s      
        trn2            v17.4s, v17.4s, v17.4s      

        rev64           v22.4s, v8.4s                 
        rev64           v23.4s, v10.4s                 
        rev64           v24.4s, v12.4s                 
        rev64           v25.4s, v14.4s                 

        fmul            v22.4s, v22.4s, v20.4s      
        fmul            v23.4s, v23.4s, v16.4s      
        fmul            v24.4s, v24.4s, v20.4s      
        fmul            v25.4s, v25.4s, v16.4s      

        rev64           v26.4s, v9.4s                 
        rev64           v27.4s, v11.4s                 
        rev64           v20.4s, v13.4s                 
        rev64           v16.4s, v15.4s                 

        fmul            v26.4s, v26.4s, v21.4s      
        fmul            v27.4s, v27.4s, v17.4s      
        fmul            v20.4s, v20.4s, v21.4s      
        fmul            v16.4s, v16.4s, v17.4s      

        trn1            v21.4s, v19.4s, v19.4s      
        trn2            v19.4s, v19.4s, v19.4s      
        trn1            v17.4s, v18.4s, v18.4s      
        trn2            v18.4s, v18.4s, v18.4s      

        fmul            v21.4s, v21.4s, v31.4s
        fmul            v19.4s, v19.4s, v31.4s
        fmul            v17.4s, v17.4s, v31.4s
        fmul            v18.4s, v18.4s, v31.4s

        fmla            v23.4s, v10.4s, v21.4s      
        fmls            v25.4s, v14.4s, v21.4s      
        fmla            v22.4s, v8.4s, v19.4s      
        fmls            v24.4s, v12.4s, v19.4s      

        fmla            v26.4s, v9.4s, v18.4s      
        fmla            v27.4s, v11.4s, v17.4s      
        fmls            v20.4s, v13.4s, v18.4s      
        fmls            v16.4s, v15.4s, v17.4s      

        fsub            v18.4s, v23.4s, v25.4s
        fsub            v17.4s, v24.4s, v22.4s
        fsub            v19.4s, v20.4s, v26.4s
        fsub            v21.4s, v16.4s, v27.4s

        fadd            v22.4s, v24.4s, v22.4s
        fadd            v23.4s, v25.4s, v23.4s
        fadd            v24.4s, v20.4s, v26.4s
        fadd            v25.4s, v16.4s, v27.4s

        fmul            v17.4s, v17.4s, v31.4s
        fmul            v18.4s, v18.4s, v31.4s
        fmul            v19.4s, v19.4s, v31.4s
        fmul            v21.4s, v21.4s, v31.4s

        rev64           v22.4s, v22.4s
        rev64           v24.4s, v24.4s
        rev64           v23.4s, v23.4s
        rev64           v25.4s, v25.4s

        fsub            v8.4s, v0.4s, v22.4s
        fsub            v9.4s, v1.4s, v24.4s
        fsub            v10.4s, v2.4s, v17.4s
        fsub            v11.4s, v3.4s, v19.4s

        fsub            v12.4s, v4.4s, v23.4s
        fsub            v13.4s, v5.4s, v25.4s
        fadd            v14.4s, v6.4s, v18.4s
        fsub            v15.4s, v7.4s, v21.4s

        fadd            v0.4s, v0.4s, v22.4s
        fadd            v1.4s, v1.4s, v24.4s
        fadd            v2.4s, v2.4s, v17.4s
        fadd            v3.4s, v3.4s, v19.4s

        fadd            v4.4s, v4.4s, v23.4s
        fsub            v6.4s, v6.4s, v18.4s      
        fadd            v5.4s, v5.4s, v25.4s      
        fadd            v7.4s, v7.4s, v21.4s

        stp             q0,  q1,  [x1,  #((0 + 5)*32 + 0)]
        stp             q4,  q5,  [x1,  #((2 + 5)*32 + 0)]
        stp             q2,  q3,  [x10, #((0 + 5)*32 + 0)]
        stp             q6,  q7,  [x10, #((2 + 5)*32 + 0)]

        stp             q8,  q9,  [x11, #((0 + 5)*32 + 0)]
        stp             q12, q13, [x11, #((2 + 5)*32 + 0)]
        stp             q10, q11, [x12, #((0 + 5)*32 + 0)]
        stp             q14, q15, [x12, #((2 + 5)*32 + 0)]

        cmp             w20, #128
        bgt            temp_label_7
        ret
temp_label_8
temp_label_7

        stp             x20, x30, [sp, #-16]!
        mov             w20, #64
        add             x1, x1, #32*32
        bl              temp_label_0
        add             x1, x1, #16*32
        bl              temp_label_0
        ldp             x20, x30, [sp], #16
        sub             x1, x1, #48*32

        ldr             w5, =(256 - 4*7)










        adrp            x7, ff_tx_tab_256_float
        add             x7, x7, ff_tx_tab_256_float










        add             x8, x7, x5
        mov             x9, #-32

        mov             x21, #2*256
        add             x22, x21, x21, lsl #1

        cmp             w19, #256
        beq            temp_label_9

        add             x10, x1, x21
        add             x11, x1, x21, lsl #1
        add             x12, x1, x22

        add             x10, x1, x21
        add             x11, x1, x21, lsl #1
        add             x12, x1, x22

        ldp             q0,  q1,  [x1,  #((0 + 0)*32 + 0)]
        ldp             q4,  q5,  [x1,  #((2 + 0)*32 + 0)]
        ldp             q2,  q3,  [x10, #((0 + 0)*32 + 0)]
        ldp             q6,  q7,  [x10, #((2 + 0)*32 + 0)]

        ldp             q8,  q9,  [x11, #((0 + 0)*32 + 0)]
        ldp             q10, q11, [x11, #((2 + 0)*32 + 0)]
        ldp             q12, q13, [x12, #((0 + 0)*32 + 0)]
        ldp             q14, q15, [x12, #((2 + 0)*32 + 0)]


        ld1             { v24.4s, v25.4s }, [x8], x9
        ld1             { v16.4s, v17.4s }, [x7], #32

        ext8             v18.16b, v24.16b, v24.16b, #8
        ext8             v19.16b, v25.16b, v25.16b, #8

        trn1            v20.4s, v16.4s, v16.4s      
        trn2            v16.4s, v16.4s, v16.4s      
        trn1            v21.4s, v17.4s, v17.4s      
        trn2            v17.4s, v17.4s, v17.4s      

        rev64           v22.4s, v8.4s                 
        rev64           v23.4s, v10.4s                 
        rev64           v24.4s, v12.4s                 
        rev64           v25.4s, v14.4s                 

        fmul            v22.4s, v22.4s, v20.4s      
        fmul            v23.4s, v23.4s, v16.4s      
        fmul            v24.4s, v24.4s, v20.4s      
        fmul            v25.4s, v25.4s, v16.4s      

        rev64           v26.4s, v9.4s                 
        rev64           v27.4s, v11.4s                 
        rev64           v20.4s, v13.4s                 
        rev64           v16.4s, v15.4s                 

        fmul            v26.4s, v26.4s, v21.4s      
        fmul            v27.4s, v27.4s, v17.4s      
        fmul            v20.4s, v20.4s, v21.4s      
        fmul            v16.4s, v16.4s, v17.4s      

        trn1            v21.4s, v19.4s, v19.4s      
        trn2            v19.4s, v19.4s, v19.4s      
        trn1            v17.4s, v18.4s, v18.4s      
        trn2            v18.4s, v18.4s, v18.4s      

        fmul            v21.4s, v21.4s, v31.4s
        fmul            v19.4s, v19.4s, v31.4s
        fmul            v17.4s, v17.4s, v31.4s
        fmul            v18.4s, v18.4s, v31.4s

        fmla            v23.4s, v10.4s, v21.4s      
        fmls            v25.4s, v14.4s, v21.4s      
        fmla            v22.4s, v8.4s, v19.4s      
        fmls            v24.4s, v12.4s, v19.4s      

        fmla            v26.4s, v9.4s, v18.4s      
        fmla            v27.4s, v11.4s, v17.4s      
        fmls            v20.4s, v13.4s, v18.4s      
        fmls            v16.4s, v15.4s, v17.4s      

        fsub            v18.4s, v23.4s, v25.4s
        fsub            v17.4s, v24.4s, v22.4s
        fsub            v19.4s, v20.4s, v26.4s
        fsub            v21.4s, v16.4s, v27.4s

        fadd            v22.4s, v24.4s, v22.4s
        fadd            v23.4s, v25.4s, v23.4s
        fadd            v24.4s, v20.4s, v26.4s
        fadd            v25.4s, v16.4s, v27.4s

        fmul            v17.4s, v17.4s, v31.4s
        fmul            v18.4s, v18.4s, v31.4s
        fmul            v19.4s, v19.4s, v31.4s
        fmul            v21.4s, v21.4s, v31.4s

        rev64           v22.4s, v22.4s
        rev64           v24.4s, v24.4s
        rev64           v23.4s, v23.4s
        rev64           v25.4s, v25.4s

        fsub            v8.4s, v0.4s, v22.4s
        fsub            v9.4s, v1.4s, v24.4s
        fsub            v10.4s, v2.4s, v17.4s
        fsub            v11.4s, v3.4s, v19.4s

        fsub            v12.4s, v4.4s, v23.4s
        fsub            v13.4s, v5.4s, v25.4s
        fadd            v14.4s, v6.4s, v18.4s
        fsub            v15.4s, v7.4s, v21.4s

        fadd            v0.4s, v0.4s, v22.4s
        fadd            v1.4s, v1.4s, v24.4s
        fadd            v2.4s, v2.4s, v17.4s
        fadd            v3.4s, v3.4s, v19.4s

        fadd            v4.4s, v4.4s, v23.4s
        fsub            v6.4s, v6.4s, v18.4s      
        fadd            v5.4s, v5.4s, v25.4s      
        fadd            v7.4s, v7.4s, v21.4s

        stp             q0,  q1,  [x1,  #((0 + 0)*32 + 0)]
        stp             q4,  q5,  [x1,  #((2 + 0)*32 + 0)]
        stp             q2,  q3,  [x10, #((0 + 0)*32 + 0)]
        stp             q6,  q7,  [x10, #((2 + 0)*32 + 0)]

        stp             q8,  q9,  [x11, #((0 + 0)*32 + 0)]
        stp             q12, q13, [x11, #((2 + 0)*32 + 0)]
        stp             q10, q11, [x12, #((0 + 0)*32 + 0)]
        stp             q14, q15, [x12, #((2 + 0)*32 + 0)]
        add             x10, x1, x21
        add             x11, x1, x21, lsl #1
        add             x12, x1, x22

        ldp             q0,  q1,  [x1,  #((0 + 1)*32 + 0)]
        ldp             q4,  q5,  [x1,  #((2 + 1)*32 + 0)]
        ldp             q2,  q3,  [x10, #((0 + 1)*32 + 0)]
        ldp             q6,  q7,  [x10, #((2 + 1)*32 + 0)]

        ldp             q8,  q9,  [x11, #((0 + 1)*32 + 0)]
        ldp             q10, q11, [x11, #((2 + 1)*32 + 0)]
        ldp             q12, q13, [x12, #((0 + 1)*32 + 0)]
        ldp             q14, q15, [x12, #((2 + 1)*32 + 0)]


        ld1             { v24.4s, v25.4s }, [x8], x9
        ld1             { v16.4s, v17.4s }, [x7], #32

        ext8             v18.16b, v24.16b, v24.16b, #8
        ext8             v19.16b, v25.16b, v25.16b, #8

        trn1            v20.4s, v16.4s, v16.4s      
        trn2            v16.4s, v16.4s, v16.4s      
        trn1            v21.4s, v17.4s, v17.4s      
        trn2            v17.4s, v17.4s, v17.4s      

        rev64           v22.4s, v8.4s                 
        rev64           v23.4s, v10.4s                 
        rev64           v24.4s, v12.4s                 
        rev64           v25.4s, v14.4s                 

        fmul            v22.4s, v22.4s, v20.4s      
        fmul            v23.4s, v23.4s, v16.4s      
        fmul            v24.4s, v24.4s, v20.4s      
        fmul            v25.4s, v25.4s, v16.4s      

        rev64           v26.4s, v9.4s                 
        rev64           v27.4s, v11.4s                 
        rev64           v20.4s, v13.4s                 
        rev64           v16.4s, v15.4s                 

        fmul            v26.4s, v26.4s, v21.4s      
        fmul            v27.4s, v27.4s, v17.4s      
        fmul            v20.4s, v20.4s, v21.4s      
        fmul            v16.4s, v16.4s, v17.4s      

        trn1            v21.4s, v19.4s, v19.4s      
        trn2            v19.4s, v19.4s, v19.4s      
        trn1            v17.4s, v18.4s, v18.4s      
        trn2            v18.4s, v18.4s, v18.4s      

        fmul            v21.4s, v21.4s, v31.4s
        fmul            v19.4s, v19.4s, v31.4s
        fmul            v17.4s, v17.4s, v31.4s
        fmul            v18.4s, v18.4s, v31.4s

        fmla            v23.4s, v10.4s, v21.4s      
        fmls            v25.4s, v14.4s, v21.4s      
        fmla            v22.4s, v8.4s, v19.4s      
        fmls            v24.4s, v12.4s, v19.4s      

        fmla            v26.4s, v9.4s, v18.4s      
        fmla            v27.4s, v11.4s, v17.4s      
        fmls            v20.4s, v13.4s, v18.4s      
        fmls            v16.4s, v15.4s, v17.4s      

        fsub            v18.4s, v23.4s, v25.4s
        fsub            v17.4s, v24.4s, v22.4s
        fsub            v19.4s, v20.4s, v26.4s
        fsub            v21.4s, v16.4s, v27.4s

        fadd            v22.4s, v24.4s, v22.4s
        fadd            v23.4s, v25.4s, v23.4s
        fadd            v24.4s, v20.4s, v26.4s
        fadd            v25.4s, v16.4s, v27.4s

        fmul            v17.4s, v17.4s, v31.4s
        fmul            v18.4s, v18.4s, v31.4s
        fmul            v19.4s, v19.4s, v31.4s
        fmul            v21.4s, v21.4s, v31.4s

        rev64           v22.4s, v22.4s
        rev64           v24.4s, v24.4s
        rev64           v23.4s, v23.4s
        rev64           v25.4s, v25.4s

        fsub            v8.4s, v0.4s, v22.4s
        fsub            v9.4s, v1.4s, v24.4s
        fsub            v10.4s, v2.4s, v17.4s
        fsub            v11.4s, v3.4s, v19.4s

        fsub            v12.4s, v4.4s, v23.4s
        fsub            v13.4s, v5.4s, v25.4s
        fadd            v14.4s, v6.4s, v18.4s
        fsub            v15.4s, v7.4s, v21.4s

        fadd            v0.4s, v0.4s, v22.4s
        fadd            v1.4s, v1.4s, v24.4s
        fadd            v2.4s, v2.4s, v17.4s
        fadd            v3.4s, v3.4s, v19.4s

        fadd            v4.4s, v4.4s, v23.4s
        fsub            v6.4s, v6.4s, v18.4s      
        fadd            v5.4s, v5.4s, v25.4s      
        fadd            v7.4s, v7.4s, v21.4s

        stp             q0,  q1,  [x1,  #((0 + 1)*32 + 0)]
        stp             q4,  q5,  [x1,  #((2 + 1)*32 + 0)]
        stp             q2,  q3,  [x10, #((0 + 1)*32 + 0)]
        stp             q6,  q7,  [x10, #((2 + 1)*32 + 0)]

        stp             q8,  q9,  [x11, #((0 + 1)*32 + 0)]
        stp             q12, q13, [x11, #((2 + 1)*32 + 0)]
        stp             q10, q11, [x12, #((0 + 1)*32 + 0)]
        stp             q14, q15, [x12, #((2 + 1)*32 + 0)]
        add             x10, x1, x21
        add             x11, x1, x21, lsl #1
        add             x12, x1, x22

        ldp             q0,  q1,  [x1,  #((0 + 4)*32 + 0)]
        ldp             q4,  q5,  [x1,  #((2 + 4)*32 + 0)]
        ldp             q2,  q3,  [x10, #((0 + 4)*32 + 0)]
        ldp             q6,  q7,  [x10, #((2 + 4)*32 + 0)]

        ldp             q8,  q9,  [x11, #((0 + 4)*32 + 0)]
        ldp             q10, q11, [x11, #((2 + 4)*32 + 0)]
        ldp             q12, q13, [x12, #((0 + 4)*32 + 0)]
        ldp             q14, q15, [x12, #((2 + 4)*32 + 0)]


        ld1             { v24.4s, v25.4s }, [x8], x9
        ld1             { v16.4s, v17.4s }, [x7], #32

        ext8             v18.16b, v24.16b, v24.16b, #8
        ext8             v19.16b, v25.16b, v25.16b, #8

        trn1            v20.4s, v16.4s, v16.4s      
        trn2            v16.4s, v16.4s, v16.4s      
        trn1            v21.4s, v17.4s, v17.4s      
        trn2            v17.4s, v17.4s, v17.4s      

        rev64           v22.4s, v8.4s                 
        rev64           v23.4s, v10.4s                 
        rev64           v24.4s, v12.4s                 
        rev64           v25.4s, v14.4s                 

        fmul            v22.4s, v22.4s, v20.4s      
        fmul            v23.4s, v23.4s, v16.4s      
        fmul            v24.4s, v24.4s, v20.4s      
        fmul            v25.4s, v25.4s, v16.4s      

        rev64           v26.4s, v9.4s                 
        rev64           v27.4s, v11.4s                 
        rev64           v20.4s, v13.4s                 
        rev64           v16.4s, v15.4s                 

        fmul            v26.4s, v26.4s, v21.4s      
        fmul            v27.4s, v27.4s, v17.4s      
        fmul            v20.4s, v20.4s, v21.4s      
        fmul            v16.4s, v16.4s, v17.4s      

        trn1            v21.4s, v19.4s, v19.4s      
        trn2            v19.4s, v19.4s, v19.4s      
        trn1            v17.4s, v18.4s, v18.4s      
        trn2            v18.4s, v18.4s, v18.4s      

        fmul            v21.4s, v21.4s, v31.4s
        fmul            v19.4s, v19.4s, v31.4s
        fmul            v17.4s, v17.4s, v31.4s
        fmul            v18.4s, v18.4s, v31.4s

        fmla            v23.4s, v10.4s, v21.4s      
        fmls            v25.4s, v14.4s, v21.4s      
        fmla            v22.4s, v8.4s, v19.4s      
        fmls            v24.4s, v12.4s, v19.4s      

        fmla            v26.4s, v9.4s, v18.4s      
        fmla            v27.4s, v11.4s, v17.4s      
        fmls            v20.4s, v13.4s, v18.4s      
        fmls            v16.4s, v15.4s, v17.4s      

        fsub            v18.4s, v23.4s, v25.4s
        fsub            v17.4s, v24.4s, v22.4s
        fsub            v19.4s, v20.4s, v26.4s
        fsub            v21.4s, v16.4s, v27.4s

        fadd            v22.4s, v24.4s, v22.4s
        fadd            v23.4s, v25.4s, v23.4s
        fadd            v24.4s, v20.4s, v26.4s
        fadd            v25.4s, v16.4s, v27.4s

        fmul            v17.4s, v17.4s, v31.4s
        fmul            v18.4s, v18.4s, v31.4s
        fmul            v19.4s, v19.4s, v31.4s
        fmul            v21.4s, v21.4s, v31.4s

        rev64           v22.4s, v22.4s
        rev64           v24.4s, v24.4s
        rev64           v23.4s, v23.4s
        rev64           v25.4s, v25.4s

        fsub            v8.4s, v0.4s, v22.4s
        fsub            v9.4s, v1.4s, v24.4s
        fsub            v10.4s, v2.4s, v17.4s
        fsub            v11.4s, v3.4s, v19.4s

        fsub            v12.4s, v4.4s, v23.4s
        fsub            v13.4s, v5.4s, v25.4s
        fadd            v14.4s, v6.4s, v18.4s
        fsub            v15.4s, v7.4s, v21.4s

        fadd            v0.4s, v0.4s, v22.4s
        fadd            v1.4s, v1.4s, v24.4s
        fadd            v2.4s, v2.4s, v17.4s
        fadd            v3.4s, v3.4s, v19.4s

        fadd            v4.4s, v4.4s, v23.4s
        fsub            v6.4s, v6.4s, v18.4s      
        fadd            v5.4s, v5.4s, v25.4s      
        fadd            v7.4s, v7.4s, v21.4s

        stp             q0,  q1,  [x1,  #((0 + 4)*32 + 0)]
        stp             q4,  q5,  [x1,  #((2 + 4)*32 + 0)]
        stp             q2,  q3,  [x10, #((0 + 4)*32 + 0)]
        stp             q6,  q7,  [x10, #((2 + 4)*32 + 0)]

        stp             q8,  q9,  [x11, #((0 + 4)*32 + 0)]
        stp             q12, q13, [x11, #((2 + 4)*32 + 0)]
        stp             q10, q11, [x12, #((0 + 4)*32 + 0)]
        stp             q14, q15, [x12, #((2 + 4)*32 + 0)]
        add             x10, x1, x21
        add             x11, x1, x21, lsl #1
        add             x12, x1, x22

        ldp             q0,  q1,  [x1,  #((0 + 5)*32 + 0)]
        ldp             q4,  q5,  [x1,  #((2 + 5)*32 + 0)]
        ldp             q2,  q3,  [x10, #((0 + 5)*32 + 0)]
        ldp             q6,  q7,  [x10, #((2 + 5)*32 + 0)]

        ldp             q8,  q9,  [x11, #((0 + 5)*32 + 0)]
        ldp             q10, q11, [x11, #((2 + 5)*32 + 0)]
        ldp             q12, q13, [x12, #((0 + 5)*32 + 0)]
        ldp             q14, q15, [x12, #((2 + 5)*32 + 0)]


        ld1             { v24.4s, v25.4s }, [x8], x9
        ld1             { v16.4s, v17.4s }, [x7], #32

        ext8             v18.16b, v24.16b, v24.16b, #8
        ext8             v19.16b, v25.16b, v25.16b, #8

        trn1            v20.4s, v16.4s, v16.4s      
        trn2            v16.4s, v16.4s, v16.4s      
        trn1            v21.4s, v17.4s, v17.4s      
        trn2            v17.4s, v17.4s, v17.4s      

        rev64           v22.4s, v8.4s                 
        rev64           v23.4s, v10.4s                 
        rev64           v24.4s, v12.4s                 
        rev64           v25.4s, v14.4s                 

        fmul            v22.4s, v22.4s, v20.4s      
        fmul            v23.4s, v23.4s, v16.4s      
        fmul            v24.4s, v24.4s, v20.4s      
        fmul            v25.4s, v25.4s, v16.4s      

        rev64           v26.4s, v9.4s                 
        rev64           v27.4s, v11.4s                 
        rev64           v20.4s, v13.4s                 
        rev64           v16.4s, v15.4s                 

        fmul            v26.4s, v26.4s, v21.4s      
        fmul            v27.4s, v27.4s, v17.4s      
        fmul            v20.4s, v20.4s, v21.4s      
        fmul            v16.4s, v16.4s, v17.4s      

        trn1            v21.4s, v19.4s, v19.4s      
        trn2            v19.4s, v19.4s, v19.4s      
        trn1            v17.4s, v18.4s, v18.4s      
        trn2            v18.4s, v18.4s, v18.4s      

        fmul            v21.4s, v21.4s, v31.4s
        fmul            v19.4s, v19.4s, v31.4s
        fmul            v17.4s, v17.4s, v31.4s
        fmul            v18.4s, v18.4s, v31.4s

        fmla            v23.4s, v10.4s, v21.4s      
        fmls            v25.4s, v14.4s, v21.4s      
        fmla            v22.4s, v8.4s, v19.4s      
        fmls            v24.4s, v12.4s, v19.4s      

        fmla            v26.4s, v9.4s, v18.4s      
        fmla            v27.4s, v11.4s, v17.4s      
        fmls            v20.4s, v13.4s, v18.4s      
        fmls            v16.4s, v15.4s, v17.4s      

        fsub            v18.4s, v23.4s, v25.4s
        fsub            v17.4s, v24.4s, v22.4s
        fsub            v19.4s, v20.4s, v26.4s
        fsub            v21.4s, v16.4s, v27.4s

        fadd            v22.4s, v24.4s, v22.4s
        fadd            v23.4s, v25.4s, v23.4s
        fadd            v24.4s, v20.4s, v26.4s
        fadd            v25.4s, v16.4s, v27.4s

        fmul            v17.4s, v17.4s, v31.4s
        fmul            v18.4s, v18.4s, v31.4s
        fmul            v19.4s, v19.4s, v31.4s
        fmul            v21.4s, v21.4s, v31.4s

        rev64           v22.4s, v22.4s
        rev64           v24.4s, v24.4s
        rev64           v23.4s, v23.4s
        rev64           v25.4s, v25.4s

        fsub            v8.4s, v0.4s, v22.4s
        fsub            v9.4s, v1.4s, v24.4s
        fsub            v10.4s, v2.4s, v17.4s
        fsub            v11.4s, v3.4s, v19.4s

        fsub            v12.4s, v4.4s, v23.4s
        fsub            v13.4s, v5.4s, v25.4s
        fadd            v14.4s, v6.4s, v18.4s
        fsub            v15.4s, v7.4s, v21.4s

        fadd            v0.4s, v0.4s, v22.4s
        fadd            v1.4s, v1.4s, v24.4s
        fadd            v2.4s, v2.4s, v17.4s
        fadd            v3.4s, v3.4s, v19.4s

        fadd            v4.4s, v4.4s, v23.4s
        fsub            v6.4s, v6.4s, v18.4s      
        fadd            v5.4s, v5.4s, v25.4s      
        fadd            v7.4s, v7.4s, v21.4s

        stp             q0,  q1,  [x1,  #((0 + 5)*32 + 0)]
        stp             q4,  q5,  [x1,  #((2 + 5)*32 + 0)]
        stp             q2,  q3,  [x10, #((0 + 5)*32 + 0)]
        stp             q6,  q7,  [x10, #((2 + 5)*32 + 0)]

        stp             q8,  q9,  [x11, #((0 + 5)*32 + 0)]
        stp             q12, q13, [x11, #((2 + 5)*32 + 0)]
        stp             q10, q11, [x12, #((0 + 5)*32 + 0)]
        stp             q14, q15, [x12, #((2 + 5)*32 + 0)]
        add             x10, x1, x21
        add             x11, x1, x21, lsl #1
        add             x12, x1, x22

        add             x10, x1, x21
        add             x11, x1, x21, lsl #1
        add             x12, x1, x22

        ldp             q0,  q1,  [x1,  #((0 + 0)*32 + 8*32)]
        ldp             q4,  q5,  [x1,  #((2 + 0)*32 + 8*32)]
        ldp             q2,  q3,  [x10, #((0 + 0)*32 + 8*32)]
        ldp             q6,  q7,  [x10, #((2 + 0)*32 + 8*32)]

        ldp             q8,  q9,  [x11, #((0 + 0)*32 + 8*32)]
        ldp             q10, q11, [x11, #((2 + 0)*32 + 8*32)]
        ldp             q12, q13, [x12, #((0 + 0)*32 + 8*32)]
        ldp             q14, q15, [x12, #((2 + 0)*32 + 8*32)]


        ld1             { v24.4s, v25.4s }, [x8], x9
        ld1             { v16.4s, v17.4s }, [x7], #32

        ext8             v18.16b, v24.16b, v24.16b, #8
        ext8             v19.16b, v25.16b, v25.16b, #8

        trn1            v20.4s, v16.4s, v16.4s      
        trn2            v16.4s, v16.4s, v16.4s      
        trn1            v21.4s, v17.4s, v17.4s      
        trn2            v17.4s, v17.4s, v17.4s      

        rev64           v22.4s, v8.4s                 
        rev64           v23.4s, v10.4s                 
        rev64           v24.4s, v12.4s                 
        rev64           v25.4s, v14.4s                 

        fmul            v22.4s, v22.4s, v20.4s      
        fmul            v23.4s, v23.4s, v16.4s      
        fmul            v24.4s, v24.4s, v20.4s      
        fmul            v25.4s, v25.4s, v16.4s      

        rev64           v26.4s, v9.4s                 
        rev64           v27.4s, v11.4s                 
        rev64           v20.4s, v13.4s                 
        rev64           v16.4s, v15.4s                 

        fmul            v26.4s, v26.4s, v21.4s      
        fmul            v27.4s, v27.4s, v17.4s      
        fmul            v20.4s, v20.4s, v21.4s      
        fmul            v16.4s, v16.4s, v17.4s      

        trn1            v21.4s, v19.4s, v19.4s      
        trn2            v19.4s, v19.4s, v19.4s      
        trn1            v17.4s, v18.4s, v18.4s      
        trn2            v18.4s, v18.4s, v18.4s      

        fmul            v21.4s, v21.4s, v31.4s
        fmul            v19.4s, v19.4s, v31.4s
        fmul            v17.4s, v17.4s, v31.4s
        fmul            v18.4s, v18.4s, v31.4s

        fmla            v23.4s, v10.4s, v21.4s      
        fmls            v25.4s, v14.4s, v21.4s      
        fmla            v22.4s, v8.4s, v19.4s      
        fmls            v24.4s, v12.4s, v19.4s      

        fmla            v26.4s, v9.4s, v18.4s      
        fmla            v27.4s, v11.4s, v17.4s      
        fmls            v20.4s, v13.4s, v18.4s      
        fmls            v16.4s, v15.4s, v17.4s      

        fsub            v18.4s, v23.4s, v25.4s
        fsub            v17.4s, v24.4s, v22.4s
        fsub            v19.4s, v20.4s, v26.4s
        fsub            v21.4s, v16.4s, v27.4s

        fadd            v22.4s, v24.4s, v22.4s
        fadd            v23.4s, v25.4s, v23.4s
        fadd            v24.4s, v20.4s, v26.4s
        fadd            v25.4s, v16.4s, v27.4s

        fmul            v17.4s, v17.4s, v31.4s
        fmul            v18.4s, v18.4s, v31.4s
        fmul            v19.4s, v19.4s, v31.4s
        fmul            v21.4s, v21.4s, v31.4s

        rev64           v22.4s, v22.4s
        rev64           v24.4s, v24.4s
        rev64           v23.4s, v23.4s
        rev64           v25.4s, v25.4s

        fsub            v8.4s, v0.4s, v22.4s
        fsub            v9.4s, v1.4s, v24.4s
        fsub            v10.4s, v2.4s, v17.4s
        fsub            v11.4s, v3.4s, v19.4s

        fsub            v12.4s, v4.4s, v23.4s
        fsub            v13.4s, v5.4s, v25.4s
        fadd            v14.4s, v6.4s, v18.4s
        fsub            v15.4s, v7.4s, v21.4s

        fadd            v0.4s, v0.4s, v22.4s
        fadd            v1.4s, v1.4s, v24.4s
        fadd            v2.4s, v2.4s, v17.4s
        fadd            v3.4s, v3.4s, v19.4s

        fadd            v4.4s, v4.4s, v23.4s
        fsub            v6.4s, v6.4s, v18.4s      
        fadd            v5.4s, v5.4s, v25.4s      
        fadd            v7.4s, v7.4s, v21.4s

        stp             q0,  q1,  [x1,  #((0 + 0)*32 + 8*32)]
        stp             q4,  q5,  [x1,  #((2 + 0)*32 + 8*32)]
        stp             q2,  q3,  [x10, #((0 + 0)*32 + 8*32)]
        stp             q6,  q7,  [x10, #((2 + 0)*32 + 8*32)]

        stp             q8,  q9,  [x11, #((0 + 0)*32 + 8*32)]
        stp             q12, q13, [x11, #((2 + 0)*32 + 8*32)]
        stp             q10, q11, [x12, #((0 + 0)*32 + 8*32)]
        stp             q14, q15, [x12, #((2 + 0)*32 + 8*32)]
        add             x10, x1, x21
        add             x11, x1, x21, lsl #1
        add             x12, x1, x22

        ldp             q0,  q1,  [x1,  #((0 + 1)*32 + 8*32)]
        ldp             q4,  q5,  [x1,  #((2 + 1)*32 + 8*32)]
        ldp             q2,  q3,  [x10, #((0 + 1)*32 + 8*32)]
        ldp             q6,  q7,  [x10, #((2 + 1)*32 + 8*32)]

        ldp             q8,  q9,  [x11, #((0 + 1)*32 + 8*32)]
        ldp             q10, q11, [x11, #((2 + 1)*32 + 8*32)]
        ldp             q12, q13, [x12, #((0 + 1)*32 + 8*32)]
        ldp             q14, q15, [x12, #((2 + 1)*32 + 8*32)]


        ld1             { v24.4s, v25.4s }, [x8], x9
        ld1             { v16.4s, v17.4s }, [x7], #32

        ext8             v18.16b, v24.16b, v24.16b, #8
        ext8             v19.16b, v25.16b, v25.16b, #8

        trn1            v20.4s, v16.4s, v16.4s      
        trn2            v16.4s, v16.4s, v16.4s      
        trn1            v21.4s, v17.4s, v17.4s      
        trn2            v17.4s, v17.4s, v17.4s      

        rev64           v22.4s, v8.4s                 
        rev64           v23.4s, v10.4s                 
        rev64           v24.4s, v12.4s                 
        rev64           v25.4s, v14.4s                 

        fmul            v22.4s, v22.4s, v20.4s      
        fmul            v23.4s, v23.4s, v16.4s      
        fmul            v24.4s, v24.4s, v20.4s      
        fmul            v25.4s, v25.4s, v16.4s      

        rev64           v26.4s, v9.4s                 
        rev64           v27.4s, v11.4s                 
        rev64           v20.4s, v13.4s                 
        rev64           v16.4s, v15.4s                 

        fmul            v26.4s, v26.4s, v21.4s      
        fmul            v27.4s, v27.4s, v17.4s      
        fmul            v20.4s, v20.4s, v21.4s      
        fmul            v16.4s, v16.4s, v17.4s      

        trn1            v21.4s, v19.4s, v19.4s      
        trn2            v19.4s, v19.4s, v19.4s      
        trn1            v17.4s, v18.4s, v18.4s      
        trn2            v18.4s, v18.4s, v18.4s      

        fmul            v21.4s, v21.4s, v31.4s
        fmul            v19.4s, v19.4s, v31.4s
        fmul            v17.4s, v17.4s, v31.4s
        fmul            v18.4s, v18.4s, v31.4s

        fmla            v23.4s, v10.4s, v21.4s      
        fmls            v25.4s, v14.4s, v21.4s      
        fmla            v22.4s, v8.4s, v19.4s      
        fmls            v24.4s, v12.4s, v19.4s      

        fmla            v26.4s, v9.4s, v18.4s      
        fmla            v27.4s, v11.4s, v17.4s      
        fmls            v20.4s, v13.4s, v18.4s      
        fmls            v16.4s, v15.4s, v17.4s      

        fsub            v18.4s, v23.4s, v25.4s
        fsub            v17.4s, v24.4s, v22.4s
        fsub            v19.4s, v20.4s, v26.4s
        fsub            v21.4s, v16.4s, v27.4s

        fadd            v22.4s, v24.4s, v22.4s
        fadd            v23.4s, v25.4s, v23.4s
        fadd            v24.4s, v20.4s, v26.4s
        fadd            v25.4s, v16.4s, v27.4s

        fmul            v17.4s, v17.4s, v31.4s
        fmul            v18.4s, v18.4s, v31.4s
        fmul            v19.4s, v19.4s, v31.4s
        fmul            v21.4s, v21.4s, v31.4s

        rev64           v22.4s, v22.4s
        rev64           v24.4s, v24.4s
        rev64           v23.4s, v23.4s
        rev64           v25.4s, v25.4s

        fsub            v8.4s, v0.4s, v22.4s
        fsub            v9.4s, v1.4s, v24.4s
        fsub            v10.4s, v2.4s, v17.4s
        fsub            v11.4s, v3.4s, v19.4s

        fsub            v12.4s, v4.4s, v23.4s
        fsub            v13.4s, v5.4s, v25.4s
        fadd            v14.4s, v6.4s, v18.4s
        fsub            v15.4s, v7.4s, v21.4s

        fadd            v0.4s, v0.4s, v22.4s
        fadd            v1.4s, v1.4s, v24.4s
        fadd            v2.4s, v2.4s, v17.4s
        fadd            v3.4s, v3.4s, v19.4s

        fadd            v4.4s, v4.4s, v23.4s
        fsub            v6.4s, v6.4s, v18.4s      
        fadd            v5.4s, v5.4s, v25.4s      
        fadd            v7.4s, v7.4s, v21.4s

        stp             q0,  q1,  [x1,  #((0 + 1)*32 + 8*32)]
        stp             q4,  q5,  [x1,  #((2 + 1)*32 + 8*32)]
        stp             q2,  q3,  [x10, #((0 + 1)*32 + 8*32)]
        stp             q6,  q7,  [x10, #((2 + 1)*32 + 8*32)]

        stp             q8,  q9,  [x11, #((0 + 1)*32 + 8*32)]
        stp             q12, q13, [x11, #((2 + 1)*32 + 8*32)]
        stp             q10, q11, [x12, #((0 + 1)*32 + 8*32)]
        stp             q14, q15, [x12, #((2 + 1)*32 + 8*32)]
        add             x10, x1, x21
        add             x11, x1, x21, lsl #1
        add             x12, x1, x22

        ldp             q0,  q1,  [x1,  #((0 + 4)*32 + 8*32)]
        ldp             q4,  q5,  [x1,  #((2 + 4)*32 + 8*32)]
        ldp             q2,  q3,  [x10, #((0 + 4)*32 + 8*32)]
        ldp             q6,  q7,  [x10, #((2 + 4)*32 + 8*32)]

        ldp             q8,  q9,  [x11, #((0 + 4)*32 + 8*32)]
        ldp             q10, q11, [x11, #((2 + 4)*32 + 8*32)]
        ldp             q12, q13, [x12, #((0 + 4)*32 + 8*32)]
        ldp             q14, q15, [x12, #((2 + 4)*32 + 8*32)]


        ld1             { v24.4s, v25.4s }, [x8], x9
        ld1             { v16.4s, v17.4s }, [x7], #32

        ext8             v18.16b, v24.16b, v24.16b, #8
        ext8             v19.16b, v25.16b, v25.16b, #8

        trn1            v20.4s, v16.4s, v16.4s      
        trn2            v16.4s, v16.4s, v16.4s      
        trn1            v21.4s, v17.4s, v17.4s      
        trn2            v17.4s, v17.4s, v17.4s      

        rev64           v22.4s, v8.4s                 
        rev64           v23.4s, v10.4s                 
        rev64           v24.4s, v12.4s                 
        rev64           v25.4s, v14.4s                 

        fmul            v22.4s, v22.4s, v20.4s      
        fmul            v23.4s, v23.4s, v16.4s      
        fmul            v24.4s, v24.4s, v20.4s      
        fmul            v25.4s, v25.4s, v16.4s      

        rev64           v26.4s, v9.4s                 
        rev64           v27.4s, v11.4s                 
        rev64           v20.4s, v13.4s                 
        rev64           v16.4s, v15.4s                 

        fmul            v26.4s, v26.4s, v21.4s      
        fmul            v27.4s, v27.4s, v17.4s      
        fmul            v20.4s, v20.4s, v21.4s      
        fmul            v16.4s, v16.4s, v17.4s      

        trn1            v21.4s, v19.4s, v19.4s      
        trn2            v19.4s, v19.4s, v19.4s      
        trn1            v17.4s, v18.4s, v18.4s      
        trn2            v18.4s, v18.4s, v18.4s      

        fmul            v21.4s, v21.4s, v31.4s
        fmul            v19.4s, v19.4s, v31.4s
        fmul            v17.4s, v17.4s, v31.4s
        fmul            v18.4s, v18.4s, v31.4s

        fmla            v23.4s, v10.4s, v21.4s      
        fmls            v25.4s, v14.4s, v21.4s      
        fmla            v22.4s, v8.4s, v19.4s      
        fmls            v24.4s, v12.4s, v19.4s      

        fmla            v26.4s, v9.4s, v18.4s      
        fmla            v27.4s, v11.4s, v17.4s      
        fmls            v20.4s, v13.4s, v18.4s      
        fmls            v16.4s, v15.4s, v17.4s      

        fsub            v18.4s, v23.4s, v25.4s
        fsub            v17.4s, v24.4s, v22.4s
        fsub            v19.4s, v20.4s, v26.4s
        fsub            v21.4s, v16.4s, v27.4s

        fadd            v22.4s, v24.4s, v22.4s
        fadd            v23.4s, v25.4s, v23.4s
        fadd            v24.4s, v20.4s, v26.4s
        fadd            v25.4s, v16.4s, v27.4s

        fmul            v17.4s, v17.4s, v31.4s
        fmul            v18.4s, v18.4s, v31.4s
        fmul            v19.4s, v19.4s, v31.4s
        fmul            v21.4s, v21.4s, v31.4s

        rev64           v22.4s, v22.4s
        rev64           v24.4s, v24.4s
        rev64           v23.4s, v23.4s
        rev64           v25.4s, v25.4s

        fsub            v8.4s, v0.4s, v22.4s
        fsub            v9.4s, v1.4s, v24.4s
        fsub            v10.4s, v2.4s, v17.4s
        fsub            v11.4s, v3.4s, v19.4s

        fsub            v12.4s, v4.4s, v23.4s
        fsub            v13.4s, v5.4s, v25.4s
        fadd            v14.4s, v6.4s, v18.4s
        fsub            v15.4s, v7.4s, v21.4s

        fadd            v0.4s, v0.4s, v22.4s
        fadd            v1.4s, v1.4s, v24.4s
        fadd            v2.4s, v2.4s, v17.4s
        fadd            v3.4s, v3.4s, v19.4s

        fadd            v4.4s, v4.4s, v23.4s
        fsub            v6.4s, v6.4s, v18.4s      
        fadd            v5.4s, v5.4s, v25.4s      
        fadd            v7.4s, v7.4s, v21.4s

        stp             q0,  q1,  [x1,  #((0 + 4)*32 + 8*32)]
        stp             q4,  q5,  [x1,  #((2 + 4)*32 + 8*32)]
        stp             q2,  q3,  [x10, #((0 + 4)*32 + 8*32)]
        stp             q6,  q7,  [x10, #((2 + 4)*32 + 8*32)]

        stp             q8,  q9,  [x11, #((0 + 4)*32 + 8*32)]
        stp             q12, q13, [x11, #((2 + 4)*32 + 8*32)]
        stp             q10, q11, [x12, #((0 + 4)*32 + 8*32)]
        stp             q14, q15, [x12, #((2 + 4)*32 + 8*32)]
        add             x10, x1, x21
        add             x11, x1, x21, lsl #1
        add             x12, x1, x22

        ldp             q0,  q1,  [x1,  #((0 + 5)*32 + 8*32)]
        ldp             q4,  q5,  [x1,  #((2 + 5)*32 + 8*32)]
        ldp             q2,  q3,  [x10, #((0 + 5)*32 + 8*32)]
        ldp             q6,  q7,  [x10, #((2 + 5)*32 + 8*32)]

        ldp             q8,  q9,  [x11, #((0 + 5)*32 + 8*32)]
        ldp             q10, q11, [x11, #((2 + 5)*32 + 8*32)]
        ldp             q12, q13, [x12, #((0 + 5)*32 + 8*32)]
        ldp             q14, q15, [x12, #((2 + 5)*32 + 8*32)]


        ld1             { v24.4s, v25.4s }, [x8], x9
        ld1             { v16.4s, v17.4s }, [x7], #32

        ext8             v18.16b, v24.16b, v24.16b, #8
        ext8             v19.16b, v25.16b, v25.16b, #8

        trn1            v20.4s, v16.4s, v16.4s      
        trn2            v16.4s, v16.4s, v16.4s      
        trn1            v21.4s, v17.4s, v17.4s      
        trn2            v17.4s, v17.4s, v17.4s      

        rev64           v22.4s, v8.4s                 
        rev64           v23.4s, v10.4s                 
        rev64           v24.4s, v12.4s                 
        rev64           v25.4s, v14.4s                 

        fmul            v22.4s, v22.4s, v20.4s      
        fmul            v23.4s, v23.4s, v16.4s      
        fmul            v24.4s, v24.4s, v20.4s      
        fmul            v25.4s, v25.4s, v16.4s      

        rev64           v26.4s, v9.4s                 
        rev64           v27.4s, v11.4s                 
        rev64           v20.4s, v13.4s                 
        rev64           v16.4s, v15.4s                 

        fmul            v26.4s, v26.4s, v21.4s      
        fmul            v27.4s, v27.4s, v17.4s      
        fmul            v20.4s, v20.4s, v21.4s      
        fmul            v16.4s, v16.4s, v17.4s      

        trn1            v21.4s, v19.4s, v19.4s      
        trn2            v19.4s, v19.4s, v19.4s      
        trn1            v17.4s, v18.4s, v18.4s      
        trn2            v18.4s, v18.4s, v18.4s      

        fmul            v21.4s, v21.4s, v31.4s
        fmul            v19.4s, v19.4s, v31.4s
        fmul            v17.4s, v17.4s, v31.4s
        fmul            v18.4s, v18.4s, v31.4s

        fmla            v23.4s, v10.4s, v21.4s      
        fmls            v25.4s, v14.4s, v21.4s      
        fmla            v22.4s, v8.4s, v19.4s      
        fmls            v24.4s, v12.4s, v19.4s      

        fmla            v26.4s, v9.4s, v18.4s      
        fmla            v27.4s, v11.4s, v17.4s      
        fmls            v20.4s, v13.4s, v18.4s      
        fmls            v16.4s, v15.4s, v17.4s      

        fsub            v18.4s, v23.4s, v25.4s
        fsub            v17.4s, v24.4s, v22.4s
        fsub            v19.4s, v20.4s, v26.4s
        fsub            v21.4s, v16.4s, v27.4s

        fadd            v22.4s, v24.4s, v22.4s
        fadd            v23.4s, v25.4s, v23.4s
        fadd            v24.4s, v20.4s, v26.4s
        fadd            v25.4s, v16.4s, v27.4s

        fmul            v17.4s, v17.4s, v31.4s
        fmul            v18.4s, v18.4s, v31.4s
        fmul            v19.4s, v19.4s, v31.4s
        fmul            v21.4s, v21.4s, v31.4s

        rev64           v22.4s, v22.4s
        rev64           v24.4s, v24.4s
        rev64           v23.4s, v23.4s
        rev64           v25.4s, v25.4s

        fsub            v8.4s, v0.4s, v22.4s
        fsub            v9.4s, v1.4s, v24.4s
        fsub            v10.4s, v2.4s, v17.4s
        fsub            v11.4s, v3.4s, v19.4s

        fsub            v12.4s, v4.4s, v23.4s
        fsub            v13.4s, v5.4s, v25.4s
        fadd            v14.4s, v6.4s, v18.4s
        fsub            v15.4s, v7.4s, v21.4s

        fadd            v0.4s, v0.4s, v22.4s
        fadd            v1.4s, v1.4s, v24.4s
        fadd            v2.4s, v2.4s, v17.4s
        fadd            v3.4s, v3.4s, v19.4s

        fadd            v4.4s, v4.4s, v23.4s
        fsub            v6.4s, v6.4s, v18.4s      
        fadd            v5.4s, v5.4s, v25.4s      
        fadd            v7.4s, v7.4s, v21.4s

        stp             q0,  q1,  [x1,  #((0 + 5)*32 + 8*32)]
        stp             q4,  q5,  [x1,  #((2 + 5)*32 + 8*32)]
        stp             q2,  q3,  [x10, #((0 + 5)*32 + 8*32)]
        stp             q6,  q7,  [x10, #((2 + 5)*32 + 8*32)]

        stp             q8,  q9,  [x11, #((0 + 5)*32 + 8*32)]
        stp             q12, q13, [x11, #((2 + 5)*32 + 8*32)]
        stp             q10, q11, [x12, #((0 + 5)*32 + 8*32)]
        stp             q14, q15, [x12, #((2 + 5)*32 + 8*32)]

        cmp             w20, #256
        bgt            temp_label_10
        ret
temp_label_11
temp_label_10

        stp             x20, x30, [sp, #-16]!
        mov             w20, #128
        add             x1, x1, #64*32
        bl              temp_label_0
        add             x1, x1, #32*32
        bl              temp_label_0
        ldp             x20, x30, [sp], #16
        sub             x1, x1, #96*32

        ldr             w5, =(512 - 4*7)










        adrp            x7, ff_tx_tab_512_float
        add             x7, x7, ff_tx_tab_512_float










        add             x8, x7, x5
        mov             x9, #-32

        mov             x21, #2*512
        add             x22, x21, x21, lsl #1

        cmp             w19, #512
        beq            temp_label_12

        mov             x5, 4
temp_label_13

        add             x10, x1, x21
        add             x11, x1, x21, lsl #1
        add             x12, x1, x22

        add             x10, x1, x21
        add             x11, x1, x21, lsl #1
        add             x12, x1, x22

        ldp             q0,  q1,  [x1,  #((0 + 0)*32 + 0)]
        ldp             q4,  q5,  [x1,  #((2 + 0)*32 + 0)]
        ldp             q2,  q3,  [x10, #((0 + 0)*32 + 0)]
        ldp             q6,  q7,  [x10, #((2 + 0)*32 + 0)]

        ldp             q8,  q9,  [x11, #((0 + 0)*32 + 0)]
        ldp             q10, q11, [x11, #((2 + 0)*32 + 0)]
        ldp             q12, q13, [x12, #((0 + 0)*32 + 0)]
        ldp             q14, q15, [x12, #((2 + 0)*32 + 0)]


        ld1             { v24.4s, v25.4s }, [x8], x9
        ld1             { v16.4s, v17.4s }, [x7], #32

        ext8             v18.16b, v24.16b, v24.16b, #8
        ext8             v19.16b, v25.16b, v25.16b, #8

        trn1            v20.4s, v16.4s, v16.4s      
        trn2            v16.4s, v16.4s, v16.4s      
        trn1            v21.4s, v17.4s, v17.4s      
        trn2            v17.4s, v17.4s, v17.4s      

        rev64           v22.4s, v8.4s                 
        rev64           v23.4s, v10.4s                 
        rev64           v24.4s, v12.4s                 
        rev64           v25.4s, v14.4s                 

        fmul            v22.4s, v22.4s, v20.4s      
        fmul            v23.4s, v23.4s, v16.4s      
        fmul            v24.4s, v24.4s, v20.4s      
        fmul            v25.4s, v25.4s, v16.4s      

        rev64           v26.4s, v9.4s                 
        rev64           v27.4s, v11.4s                 
        rev64           v20.4s, v13.4s                 
        rev64           v16.4s, v15.4s                 

        fmul            v26.4s, v26.4s, v21.4s      
        fmul            v27.4s, v27.4s, v17.4s      
        fmul            v20.4s, v20.4s, v21.4s      
        fmul            v16.4s, v16.4s, v17.4s      

        trn1            v21.4s, v19.4s, v19.4s      
        trn2            v19.4s, v19.4s, v19.4s      
        trn1            v17.4s, v18.4s, v18.4s      
        trn2            v18.4s, v18.4s, v18.4s      

        fmul            v21.4s, v21.4s, v31.4s
        fmul            v19.4s, v19.4s, v31.4s
        fmul            v17.4s, v17.4s, v31.4s
        fmul            v18.4s, v18.4s, v31.4s

        fmla            v23.4s, v10.4s, v21.4s      
        fmls            v25.4s, v14.4s, v21.4s      
        fmla            v22.4s, v8.4s, v19.4s      
        fmls            v24.4s, v12.4s, v19.4s      

        fmla            v26.4s, v9.4s, v18.4s      
        fmla            v27.4s, v11.4s, v17.4s      
        fmls            v20.4s, v13.4s, v18.4s      
        fmls            v16.4s, v15.4s, v17.4s      

        fsub            v18.4s, v23.4s, v25.4s
        fsub            v17.4s, v24.4s, v22.4s
        fsub            v19.4s, v20.4s, v26.4s
        fsub            v21.4s, v16.4s, v27.4s

        fadd            v22.4s, v24.4s, v22.4s
        fadd            v23.4s, v25.4s, v23.4s
        fadd            v24.4s, v20.4s, v26.4s
        fadd            v25.4s, v16.4s, v27.4s

        fmul            v17.4s, v17.4s, v31.4s
        fmul            v18.4s, v18.4s, v31.4s
        fmul            v19.4s, v19.4s, v31.4s
        fmul            v21.4s, v21.4s, v31.4s

        rev64           v22.4s, v22.4s
        rev64           v24.4s, v24.4s
        rev64           v23.4s, v23.4s
        rev64           v25.4s, v25.4s

        fsub            v8.4s, v0.4s, v22.4s
        fsub            v9.4s, v1.4s, v24.4s
        fsub            v10.4s, v2.4s, v17.4s
        fsub            v11.4s, v3.4s, v19.4s

        fsub            v12.4s, v4.4s, v23.4s
        fsub            v13.4s, v5.4s, v25.4s
        fadd            v14.4s, v6.4s, v18.4s
        fsub            v15.4s, v7.4s, v21.4s

        fadd            v0.4s, v0.4s, v22.4s
        fadd            v1.4s, v1.4s, v24.4s
        fadd            v2.4s, v2.4s, v17.4s
        fadd            v3.4s, v3.4s, v19.4s

        fadd            v4.4s, v4.4s, v23.4s
        fsub            v6.4s, v6.4s, v18.4s      
        fadd            v5.4s, v5.4s, v25.4s      
        fadd            v7.4s, v7.4s, v21.4s

        stp             q0,  q1,  [x1,  #((0 + 0)*32 + 0)]
        stp             q4,  q5,  [x1,  #((2 + 0)*32 + 0)]
        stp             q2,  q3,  [x10, #((0 + 0)*32 + 0)]
        stp             q6,  q7,  [x10, #((2 + 0)*32 + 0)]

        stp             q8,  q9,  [x11, #((0 + 0)*32 + 0)]
        stp             q12, q13, [x11, #((2 + 0)*32 + 0)]
        stp             q10, q11, [x12, #((0 + 0)*32 + 0)]
        stp             q14, q15, [x12, #((2 + 0)*32 + 0)]
        add             x10, x1, x21
        add             x11, x1, x21, lsl #1
        add             x12, x1, x22

        ldp             q0,  q1,  [x1,  #((0 + 1)*32 + 0)]
        ldp             q4,  q5,  [x1,  #((2 + 1)*32 + 0)]
        ldp             q2,  q3,  [x10, #((0 + 1)*32 + 0)]
        ldp             q6,  q7,  [x10, #((2 + 1)*32 + 0)]

        ldp             q8,  q9,  [x11, #((0 + 1)*32 + 0)]
        ldp             q10, q11, [x11, #((2 + 1)*32 + 0)]
        ldp             q12, q13, [x12, #((0 + 1)*32 + 0)]
        ldp             q14, q15, [x12, #((2 + 1)*32 + 0)]


        ld1             { v24.4s, v25.4s }, [x8], x9
        ld1             { v16.4s, v17.4s }, [x7], #32

        ext8             v18.16b, v24.16b, v24.16b, #8
        ext8             v19.16b, v25.16b, v25.16b, #8

        trn1            v20.4s, v16.4s, v16.4s      
        trn2            v16.4s, v16.4s, v16.4s      
        trn1            v21.4s, v17.4s, v17.4s      
        trn2            v17.4s, v17.4s, v17.4s      

        rev64           v22.4s, v8.4s                 
        rev64           v23.4s, v10.4s                 
        rev64           v24.4s, v12.4s                 
        rev64           v25.4s, v14.4s                 

        fmul            v22.4s, v22.4s, v20.4s      
        fmul            v23.4s, v23.4s, v16.4s      
        fmul            v24.4s, v24.4s, v20.4s      
        fmul            v25.4s, v25.4s, v16.4s      

        rev64           v26.4s, v9.4s                 
        rev64           v27.4s, v11.4s                 
        rev64           v20.4s, v13.4s                 
        rev64           v16.4s, v15.4s                 

        fmul            v26.4s, v26.4s, v21.4s      
        fmul            v27.4s, v27.4s, v17.4s      
        fmul            v20.4s, v20.4s, v21.4s      
        fmul            v16.4s, v16.4s, v17.4s      

        trn1            v21.4s, v19.4s, v19.4s      
        trn2            v19.4s, v19.4s, v19.4s      
        trn1            v17.4s, v18.4s, v18.4s      
        trn2            v18.4s, v18.4s, v18.4s      

        fmul            v21.4s, v21.4s, v31.4s
        fmul            v19.4s, v19.4s, v31.4s
        fmul            v17.4s, v17.4s, v31.4s
        fmul            v18.4s, v18.4s, v31.4s

        fmla            v23.4s, v10.4s, v21.4s      
        fmls            v25.4s, v14.4s, v21.4s      
        fmla            v22.4s, v8.4s, v19.4s      
        fmls            v24.4s, v12.4s, v19.4s      

        fmla            v26.4s, v9.4s, v18.4s      
        fmla            v27.4s, v11.4s, v17.4s      
        fmls            v20.4s, v13.4s, v18.4s      
        fmls            v16.4s, v15.4s, v17.4s      

        fsub            v18.4s, v23.4s, v25.4s
        fsub            v17.4s, v24.4s, v22.4s
        fsub            v19.4s, v20.4s, v26.4s
        fsub            v21.4s, v16.4s, v27.4s

        fadd            v22.4s, v24.4s, v22.4s
        fadd            v23.4s, v25.4s, v23.4s
        fadd            v24.4s, v20.4s, v26.4s
        fadd            v25.4s, v16.4s, v27.4s

        fmul            v17.4s, v17.4s, v31.4s
        fmul            v18.4s, v18.4s, v31.4s
        fmul            v19.4s, v19.4s, v31.4s
        fmul            v21.4s, v21.4s, v31.4s

        rev64           v22.4s, v22.4s
        rev64           v24.4s, v24.4s
        rev64           v23.4s, v23.4s
        rev64           v25.4s, v25.4s

        fsub            v8.4s, v0.4s, v22.4s
        fsub            v9.4s, v1.4s, v24.4s
        fsub            v10.4s, v2.4s, v17.4s
        fsub            v11.4s, v3.4s, v19.4s

        fsub            v12.4s, v4.4s, v23.4s
        fsub            v13.4s, v5.4s, v25.4s
        fadd            v14.4s, v6.4s, v18.4s
        fsub            v15.4s, v7.4s, v21.4s

        fadd            v0.4s, v0.4s, v22.4s
        fadd            v1.4s, v1.4s, v24.4s
        fadd            v2.4s, v2.4s, v17.4s
        fadd            v3.4s, v3.4s, v19.4s

        fadd            v4.4s, v4.4s, v23.4s
        fsub            v6.4s, v6.4s, v18.4s      
        fadd            v5.4s, v5.4s, v25.4s      
        fadd            v7.4s, v7.4s, v21.4s

        stp             q0,  q1,  [x1,  #((0 + 1)*32 + 0)]
        stp             q4,  q5,  [x1,  #((2 + 1)*32 + 0)]
        stp             q2,  q3,  [x10, #((0 + 1)*32 + 0)]
        stp             q6,  q7,  [x10, #((2 + 1)*32 + 0)]

        stp             q8,  q9,  [x11, #((0 + 1)*32 + 0)]
        stp             q12, q13, [x11, #((2 + 1)*32 + 0)]
        stp             q10, q11, [x12, #((0 + 1)*32 + 0)]
        stp             q14, q15, [x12, #((2 + 1)*32 + 0)]
        add             x10, x1, x21
        add             x11, x1, x21, lsl #1
        add             x12, x1, x22

        ldp             q0,  q1,  [x1,  #((0 + 4)*32 + 0)]
        ldp             q4,  q5,  [x1,  #((2 + 4)*32 + 0)]
        ldp             q2,  q3,  [x10, #((0 + 4)*32 + 0)]
        ldp             q6,  q7,  [x10, #((2 + 4)*32 + 0)]

        ldp             q8,  q9,  [x11, #((0 + 4)*32 + 0)]
        ldp             q10, q11, [x11, #((2 + 4)*32 + 0)]
        ldp             q12, q13, [x12, #((0 + 4)*32 + 0)]
        ldp             q14, q15, [x12, #((2 + 4)*32 + 0)]


        ld1             { v24.4s, v25.4s }, [x8], x9
        ld1             { v16.4s, v17.4s }, [x7], #32

        ext8             v18.16b, v24.16b, v24.16b, #8
        ext8             v19.16b, v25.16b, v25.16b, #8

        trn1            v20.4s, v16.4s, v16.4s      
        trn2            v16.4s, v16.4s, v16.4s      
        trn1            v21.4s, v17.4s, v17.4s      
        trn2            v17.4s, v17.4s, v17.4s      

        rev64           v22.4s, v8.4s                 
        rev64           v23.4s, v10.4s                 
        rev64           v24.4s, v12.4s                 
        rev64           v25.4s, v14.4s                 

        fmul            v22.4s, v22.4s, v20.4s      
        fmul            v23.4s, v23.4s, v16.4s      
        fmul            v24.4s, v24.4s, v20.4s      
        fmul            v25.4s, v25.4s, v16.4s      

        rev64           v26.4s, v9.4s                 
        rev64           v27.4s, v11.4s                 
        rev64           v20.4s, v13.4s                 
        rev64           v16.4s, v15.4s                 

        fmul            v26.4s, v26.4s, v21.4s      
        fmul            v27.4s, v27.4s, v17.4s      
        fmul            v20.4s, v20.4s, v21.4s      
        fmul            v16.4s, v16.4s, v17.4s      

        trn1            v21.4s, v19.4s, v19.4s      
        trn2            v19.4s, v19.4s, v19.4s      
        trn1            v17.4s, v18.4s, v18.4s      
        trn2            v18.4s, v18.4s, v18.4s      

        fmul            v21.4s, v21.4s, v31.4s
        fmul            v19.4s, v19.4s, v31.4s
        fmul            v17.4s, v17.4s, v31.4s
        fmul            v18.4s, v18.4s, v31.4s

        fmla            v23.4s, v10.4s, v21.4s      
        fmls            v25.4s, v14.4s, v21.4s      
        fmla            v22.4s, v8.4s, v19.4s      
        fmls            v24.4s, v12.4s, v19.4s      

        fmla            v26.4s, v9.4s, v18.4s      
        fmla            v27.4s, v11.4s, v17.4s      
        fmls            v20.4s, v13.4s, v18.4s      
        fmls            v16.4s, v15.4s, v17.4s      

        fsub            v18.4s, v23.4s, v25.4s
        fsub            v17.4s, v24.4s, v22.4s
        fsub            v19.4s, v20.4s, v26.4s
        fsub            v21.4s, v16.4s, v27.4s

        fadd            v22.4s, v24.4s, v22.4s
        fadd            v23.4s, v25.4s, v23.4s
        fadd            v24.4s, v20.4s, v26.4s
        fadd            v25.4s, v16.4s, v27.4s

        fmul            v17.4s, v17.4s, v31.4s
        fmul            v18.4s, v18.4s, v31.4s
        fmul            v19.4s, v19.4s, v31.4s
        fmul            v21.4s, v21.4s, v31.4s

        rev64           v22.4s, v22.4s
        rev64           v24.4s, v24.4s
        rev64           v23.4s, v23.4s
        rev64           v25.4s, v25.4s

        fsub            v8.4s, v0.4s, v22.4s
        fsub            v9.4s, v1.4s, v24.4s
        fsub            v10.4s, v2.4s, v17.4s
        fsub            v11.4s, v3.4s, v19.4s

        fsub            v12.4s, v4.4s, v23.4s
        fsub            v13.4s, v5.4s, v25.4s
        fadd            v14.4s, v6.4s, v18.4s
        fsub            v15.4s, v7.4s, v21.4s

        fadd            v0.4s, v0.4s, v22.4s
        fadd            v1.4s, v1.4s, v24.4s
        fadd            v2.4s, v2.4s, v17.4s
        fadd            v3.4s, v3.4s, v19.4s

        fadd            v4.4s, v4.4s, v23.4s
        fsub            v6.4s, v6.4s, v18.4s      
        fadd            v5.4s, v5.4s, v25.4s      
        fadd            v7.4s, v7.4s, v21.4s

        stp             q0,  q1,  [x1,  #((0 + 4)*32 + 0)]
        stp             q4,  q5,  [x1,  #((2 + 4)*32 + 0)]
        stp             q2,  q3,  [x10, #((0 + 4)*32 + 0)]
        stp             q6,  q7,  [x10, #((2 + 4)*32 + 0)]

        stp             q8,  q9,  [x11, #((0 + 4)*32 + 0)]
        stp             q12, q13, [x11, #((2 + 4)*32 + 0)]
        stp             q10, q11, [x12, #((0 + 4)*32 + 0)]
        stp             q14, q15, [x12, #((2 + 4)*32 + 0)]
        add             x10, x1, x21
        add             x11, x1, x21, lsl #1
        add             x12, x1, x22

        ldp             q0,  q1,  [x1,  #((0 + 5)*32 + 0)]
        ldp             q4,  q5,  [x1,  #((2 + 5)*32 + 0)]
        ldp             q2,  q3,  [x10, #((0 + 5)*32 + 0)]
        ldp             q6,  q7,  [x10, #((2 + 5)*32 + 0)]

        ldp             q8,  q9,  [x11, #((0 + 5)*32 + 0)]
        ldp             q10, q11, [x11, #((2 + 5)*32 + 0)]
        ldp             q12, q13, [x12, #((0 + 5)*32 + 0)]
        ldp             q14, q15, [x12, #((2 + 5)*32 + 0)]


        ld1             { v24.4s, v25.4s }, [x8], x9
        ld1             { v16.4s, v17.4s }, [x7], #32

        ext8             v18.16b, v24.16b, v24.16b, #8
        ext8             v19.16b, v25.16b, v25.16b, #8

        trn1            v20.4s, v16.4s, v16.4s      
        trn2            v16.4s, v16.4s, v16.4s      
        trn1            v21.4s, v17.4s, v17.4s      
        trn2            v17.4s, v17.4s, v17.4s      

        rev64           v22.4s, v8.4s                 
        rev64           v23.4s, v10.4s                 
        rev64           v24.4s, v12.4s                 
        rev64           v25.4s, v14.4s                 

        fmul            v22.4s, v22.4s, v20.4s      
        fmul            v23.4s, v23.4s, v16.4s      
        fmul            v24.4s, v24.4s, v20.4s      
        fmul            v25.4s, v25.4s, v16.4s      

        rev64           v26.4s, v9.4s                 
        rev64           v27.4s, v11.4s                 
        rev64           v20.4s, v13.4s                 
        rev64           v16.4s, v15.4s                 

        fmul            v26.4s, v26.4s, v21.4s      
        fmul            v27.4s, v27.4s, v17.4s      
        fmul            v20.4s, v20.4s, v21.4s      
        fmul            v16.4s, v16.4s, v17.4s      

        trn1            v21.4s, v19.4s, v19.4s      
        trn2            v19.4s, v19.4s, v19.4s      
        trn1            v17.4s, v18.4s, v18.4s      
        trn2            v18.4s, v18.4s, v18.4s      

        fmul            v21.4s, v21.4s, v31.4s
        fmul            v19.4s, v19.4s, v31.4s
        fmul            v17.4s, v17.4s, v31.4s
        fmul            v18.4s, v18.4s, v31.4s

        fmla            v23.4s, v10.4s, v21.4s      
        fmls            v25.4s, v14.4s, v21.4s      
        fmla            v22.4s, v8.4s, v19.4s      
        fmls            v24.4s, v12.4s, v19.4s      

        fmla            v26.4s, v9.4s, v18.4s      
        fmla            v27.4s, v11.4s, v17.4s      
        fmls            v20.4s, v13.4s, v18.4s      
        fmls            v16.4s, v15.4s, v17.4s      

        fsub            v18.4s, v23.4s, v25.4s
        fsub            v17.4s, v24.4s, v22.4s
        fsub            v19.4s, v20.4s, v26.4s
        fsub            v21.4s, v16.4s, v27.4s

        fadd            v22.4s, v24.4s, v22.4s
        fadd            v23.4s, v25.4s, v23.4s
        fadd            v24.4s, v20.4s, v26.4s
        fadd            v25.4s, v16.4s, v27.4s

        fmul            v17.4s, v17.4s, v31.4s
        fmul            v18.4s, v18.4s, v31.4s
        fmul            v19.4s, v19.4s, v31.4s
        fmul            v21.4s, v21.4s, v31.4s

        rev64           v22.4s, v22.4s
        rev64           v24.4s, v24.4s
        rev64           v23.4s, v23.4s
        rev64           v25.4s, v25.4s

        fsub            v8.4s, v0.4s, v22.4s
        fsub            v9.4s, v1.4s, v24.4s
        fsub            v10.4s, v2.4s, v17.4s
        fsub            v11.4s, v3.4s, v19.4s

        fsub            v12.4s, v4.4s, v23.4s
        fsub            v13.4s, v5.4s, v25.4s
        fadd            v14.4s, v6.4s, v18.4s
        fsub            v15.4s, v7.4s, v21.4s

        fadd            v0.4s, v0.4s, v22.4s
        fadd            v1.4s, v1.4s, v24.4s
        fadd            v2.4s, v2.4s, v17.4s
        fadd            v3.4s, v3.4s, v19.4s

        fadd            v4.4s, v4.4s, v23.4s
        fsub            v6.4s, v6.4s, v18.4s      
        fadd            v5.4s, v5.4s, v25.4s      
        fadd            v7.4s, v7.4s, v21.4s

        stp             q0,  q1,  [x1,  #((0 + 5)*32 + 0)]
        stp             q4,  q5,  [x1,  #((2 + 5)*32 + 0)]
        stp             q2,  q3,  [x10, #((0 + 5)*32 + 0)]
        stp             q6,  q7,  [x10, #((2 + 5)*32 + 0)]

        stp             q8,  q9,  [x11, #((0 + 5)*32 + 0)]
        stp             q12, q13, [x11, #((2 + 5)*32 + 0)]
        stp             q10, q11, [x12, #((0 + 5)*32 + 0)]
        stp             q14, q15, [x12, #((2 + 5)*32 + 0)]
        add             x1, x1, 8*32
        subs            w5, w5, 1
        bgt            temp_label_13

        cmp             w20, #512
        bgt            temp_label_14

        ret
temp_label_15
temp_label_14

        stp             x20, x30, [sp, #-16]!
        mov             w20, #256
        add             x1, x1, #96*32
        bl              temp_label_0
        add             x1, x1, #64*32
        bl              temp_label_0
        ldp             x20, x30, [sp], #16
        mov             x5, #192*32
        sub             x1, x1, x5

        ldr             w5, =(1024 - 4*7)










        adrp            x7, ff_tx_tab_1024_float
        add             x7, x7, ff_tx_tab_1024_float










        add             x8, x7, x5
        mov             x9, #-32

        mov             x21, #2*1024
        add             x22, x21, x21, lsl #1

        cmp             w19, #1024
        beq            temp_label_16

        mov             w5, 8
temp_label_17

        add             x10, x1, x21
        add             x11, x1, x21, lsl #1
        add             x12, x1, x22

        add             x10, x1, x21
        add             x11, x1, x21, lsl #1
        add             x12, x1, x22

        ldp             q0,  q1,  [x1,  #((0 + 0)*32 + 0)]
        ldp             q4,  q5,  [x1,  #((2 + 0)*32 + 0)]
        ldp             q2,  q3,  [x10, #((0 + 0)*32 + 0)]
        ldp             q6,  q7,  [x10, #((2 + 0)*32 + 0)]

        ldp             q8,  q9,  [x11, #((0 + 0)*32 + 0)]
        ldp             q10, q11, [x11, #((2 + 0)*32 + 0)]
        ldp             q12, q13, [x12, #((0 + 0)*32 + 0)]
        ldp             q14, q15, [x12, #((2 + 0)*32 + 0)]


        ld1             { v24.4s, v25.4s }, [x8], x9
        ld1             { v16.4s, v17.4s }, [x7], #32

        ext8             v18.16b, v24.16b, v24.16b, #8
        ext8             v19.16b, v25.16b, v25.16b, #8

        trn1            v20.4s, v16.4s, v16.4s      
        trn2            v16.4s, v16.4s, v16.4s      
        trn1            v21.4s, v17.4s, v17.4s      
        trn2            v17.4s, v17.4s, v17.4s      

        rev64           v22.4s, v8.4s                 
        rev64           v23.4s, v10.4s                 
        rev64           v24.4s, v12.4s                 
        rev64           v25.4s, v14.4s                 

        fmul            v22.4s, v22.4s, v20.4s      
        fmul            v23.4s, v23.4s, v16.4s      
        fmul            v24.4s, v24.4s, v20.4s      
        fmul            v25.4s, v25.4s, v16.4s      

        rev64           v26.4s, v9.4s                 
        rev64           v27.4s, v11.4s                 
        rev64           v20.4s, v13.4s                 
        rev64           v16.4s, v15.4s                 

        fmul            v26.4s, v26.4s, v21.4s      
        fmul            v27.4s, v27.4s, v17.4s      
        fmul            v20.4s, v20.4s, v21.4s      
        fmul            v16.4s, v16.4s, v17.4s      

        trn1            v21.4s, v19.4s, v19.4s      
        trn2            v19.4s, v19.4s, v19.4s      
        trn1            v17.4s, v18.4s, v18.4s      
        trn2            v18.4s, v18.4s, v18.4s      

        fmul            v21.4s, v21.4s, v31.4s
        fmul            v19.4s, v19.4s, v31.4s
        fmul            v17.4s, v17.4s, v31.4s
        fmul            v18.4s, v18.4s, v31.4s

        fmla            v23.4s, v10.4s, v21.4s      
        fmls            v25.4s, v14.4s, v21.4s      
        fmla            v22.4s, v8.4s, v19.4s      
        fmls            v24.4s, v12.4s, v19.4s      

        fmla            v26.4s, v9.4s, v18.4s      
        fmla            v27.4s, v11.4s, v17.4s      
        fmls            v20.4s, v13.4s, v18.4s      
        fmls            v16.4s, v15.4s, v17.4s      

        fsub            v18.4s, v23.4s, v25.4s
        fsub            v17.4s, v24.4s, v22.4s
        fsub            v19.4s, v20.4s, v26.4s
        fsub            v21.4s, v16.4s, v27.4s

        fadd            v22.4s, v24.4s, v22.4s
        fadd            v23.4s, v25.4s, v23.4s
        fadd            v24.4s, v20.4s, v26.4s
        fadd            v25.4s, v16.4s, v27.4s

        fmul            v17.4s, v17.4s, v31.4s
        fmul            v18.4s, v18.4s, v31.4s
        fmul            v19.4s, v19.4s, v31.4s
        fmul            v21.4s, v21.4s, v31.4s

        rev64           v22.4s, v22.4s
        rev64           v24.4s, v24.4s
        rev64           v23.4s, v23.4s
        rev64           v25.4s, v25.4s

        fsub            v8.4s, v0.4s, v22.4s
        fsub            v9.4s, v1.4s, v24.4s
        fsub            v10.4s, v2.4s, v17.4s
        fsub            v11.4s, v3.4s, v19.4s

        fsub            v12.4s, v4.4s, v23.4s
        fsub            v13.4s, v5.4s, v25.4s
        fadd            v14.4s, v6.4s, v18.4s
        fsub            v15.4s, v7.4s, v21.4s

        fadd            v0.4s, v0.4s, v22.4s
        fadd            v1.4s, v1.4s, v24.4s
        fadd            v2.4s, v2.4s, v17.4s
        fadd            v3.4s, v3.4s, v19.4s

        fadd            v4.4s, v4.4s, v23.4s
        fsub            v6.4s, v6.4s, v18.4s      
        fadd            v5.4s, v5.4s, v25.4s      
        fadd            v7.4s, v7.4s, v21.4s

        stp             q0,  q1,  [x1,  #((0 + 0)*32 + 0)]
        stp             q4,  q5,  [x1,  #((2 + 0)*32 + 0)]
        stp             q2,  q3,  [x10, #((0 + 0)*32 + 0)]
        stp             q6,  q7,  [x10, #((2 + 0)*32 + 0)]

        stp             q8,  q9,  [x11, #((0 + 0)*32 + 0)]
        stp             q12, q13, [x11, #((2 + 0)*32 + 0)]
        stp             q10, q11, [x12, #((0 + 0)*32 + 0)]
        stp             q14, q15, [x12, #((2 + 0)*32 + 0)]
        add             x10, x1, x21
        add             x11, x1, x21, lsl #1
        add             x12, x1, x22

        ldp             q0,  q1,  [x1,  #((0 + 1)*32 + 0)]
        ldp             q4,  q5,  [x1,  #((2 + 1)*32 + 0)]
        ldp             q2,  q3,  [x10, #((0 + 1)*32 + 0)]
        ldp             q6,  q7,  [x10, #((2 + 1)*32 + 0)]

        ldp             q8,  q9,  [x11, #((0 + 1)*32 + 0)]
        ldp             q10, q11, [x11, #((2 + 1)*32 + 0)]
        ldp             q12, q13, [x12, #((0 + 1)*32 + 0)]
        ldp             q14, q15, [x12, #((2 + 1)*32 + 0)]


        ld1             { v24.4s, v25.4s }, [x8], x9
        ld1             { v16.4s, v17.4s }, [x7], #32

        ext8             v18.16b, v24.16b, v24.16b, #8
        ext8             v19.16b, v25.16b, v25.16b, #8

        trn1            v20.4s, v16.4s, v16.4s      
        trn2            v16.4s, v16.4s, v16.4s      
        trn1            v21.4s, v17.4s, v17.4s      
        trn2            v17.4s, v17.4s, v17.4s      

        rev64           v22.4s, v8.4s                 
        rev64           v23.4s, v10.4s                 
        rev64           v24.4s, v12.4s                 
        rev64           v25.4s, v14.4s                 

        fmul            v22.4s, v22.4s, v20.4s      
        fmul            v23.4s, v23.4s, v16.4s      
        fmul            v24.4s, v24.4s, v20.4s      
        fmul            v25.4s, v25.4s, v16.4s      

        rev64           v26.4s, v9.4s                 
        rev64           v27.4s, v11.4s                 
        rev64           v20.4s, v13.4s                 
        rev64           v16.4s, v15.4s                 

        fmul            v26.4s, v26.4s, v21.4s      
        fmul            v27.4s, v27.4s, v17.4s      
        fmul            v20.4s, v20.4s, v21.4s      
        fmul            v16.4s, v16.4s, v17.4s      

        trn1            v21.4s, v19.4s, v19.4s      
        trn2            v19.4s, v19.4s, v19.4s      
        trn1            v17.4s, v18.4s, v18.4s      
        trn2            v18.4s, v18.4s, v18.4s      

        fmul            v21.4s, v21.4s, v31.4s
        fmul            v19.4s, v19.4s, v31.4s
        fmul            v17.4s, v17.4s, v31.4s
        fmul            v18.4s, v18.4s, v31.4s

        fmla            v23.4s, v10.4s, v21.4s      
        fmls            v25.4s, v14.4s, v21.4s      
        fmla            v22.4s, v8.4s, v19.4s      
        fmls            v24.4s, v12.4s, v19.4s      

        fmla            v26.4s, v9.4s, v18.4s      
        fmla            v27.4s, v11.4s, v17.4s      
        fmls            v20.4s, v13.4s, v18.4s      
        fmls            v16.4s, v15.4s, v17.4s      

        fsub            v18.4s, v23.4s, v25.4s
        fsub            v17.4s, v24.4s, v22.4s
        fsub            v19.4s, v20.4s, v26.4s
        fsub            v21.4s, v16.4s, v27.4s

        fadd            v22.4s, v24.4s, v22.4s
        fadd            v23.4s, v25.4s, v23.4s
        fadd            v24.4s, v20.4s, v26.4s
        fadd            v25.4s, v16.4s, v27.4s

        fmul            v17.4s, v17.4s, v31.4s
        fmul            v18.4s, v18.4s, v31.4s
        fmul            v19.4s, v19.4s, v31.4s
        fmul            v21.4s, v21.4s, v31.4s

        rev64           v22.4s, v22.4s
        rev64           v24.4s, v24.4s
        rev64           v23.4s, v23.4s
        rev64           v25.4s, v25.4s

        fsub            v8.4s, v0.4s, v22.4s
        fsub            v9.4s, v1.4s, v24.4s
        fsub            v10.4s, v2.4s, v17.4s
        fsub            v11.4s, v3.4s, v19.4s

        fsub            v12.4s, v4.4s, v23.4s
        fsub            v13.4s, v5.4s, v25.4s
        fadd            v14.4s, v6.4s, v18.4s
        fsub            v15.4s, v7.4s, v21.4s

        fadd            v0.4s, v0.4s, v22.4s
        fadd            v1.4s, v1.4s, v24.4s
        fadd            v2.4s, v2.4s, v17.4s
        fadd            v3.4s, v3.4s, v19.4s

        fadd            v4.4s, v4.4s, v23.4s
        fsub            v6.4s, v6.4s, v18.4s      
        fadd            v5.4s, v5.4s, v25.4s      
        fadd            v7.4s, v7.4s, v21.4s

        stp             q0,  q1,  [x1,  #((0 + 1)*32 + 0)]
        stp             q4,  q5,  [x1,  #((2 + 1)*32 + 0)]
        stp             q2,  q3,  [x10, #((0 + 1)*32 + 0)]
        stp             q6,  q7,  [x10, #((2 + 1)*32 + 0)]

        stp             q8,  q9,  [x11, #((0 + 1)*32 + 0)]
        stp             q12, q13, [x11, #((2 + 1)*32 + 0)]
        stp             q10, q11, [x12, #((0 + 1)*32 + 0)]
        stp             q14, q15, [x12, #((2 + 1)*32 + 0)]
        add             x10, x1, x21
        add             x11, x1, x21, lsl #1
        add             x12, x1, x22

        ldp             q0,  q1,  [x1,  #((0 + 4)*32 + 0)]
        ldp             q4,  q5,  [x1,  #((2 + 4)*32 + 0)]
        ldp             q2,  q3,  [x10, #((0 + 4)*32 + 0)]
        ldp             q6,  q7,  [x10, #((2 + 4)*32 + 0)]

        ldp             q8,  q9,  [x11, #((0 + 4)*32 + 0)]
        ldp             q10, q11, [x11, #((2 + 4)*32 + 0)]
        ldp             q12, q13, [x12, #((0 + 4)*32 + 0)]
        ldp             q14, q15, [x12, #((2 + 4)*32 + 0)]


        ld1             { v24.4s, v25.4s }, [x8], x9
        ld1             { v16.4s, v17.4s }, [x7], #32

        ext8             v18.16b, v24.16b, v24.16b, #8
        ext8             v19.16b, v25.16b, v25.16b, #8

        trn1            v20.4s, v16.4s, v16.4s      
        trn2            v16.4s, v16.4s, v16.4s      
        trn1            v21.4s, v17.4s, v17.4s      
        trn2            v17.4s, v17.4s, v17.4s      

        rev64           v22.4s, v8.4s                 
        rev64           v23.4s, v10.4s                 
        rev64           v24.4s, v12.4s                 
        rev64           v25.4s, v14.4s                 

        fmul            v22.4s, v22.4s, v20.4s      
        fmul            v23.4s, v23.4s, v16.4s      
        fmul            v24.4s, v24.4s, v20.4s      
        fmul            v25.4s, v25.4s, v16.4s      

        rev64           v26.4s, v9.4s                 
        rev64           v27.4s, v11.4s                 
        rev64           v20.4s, v13.4s                 
        rev64           v16.4s, v15.4s                 

        fmul            v26.4s, v26.4s, v21.4s      
        fmul            v27.4s, v27.4s, v17.4s      
        fmul            v20.4s, v20.4s, v21.4s      
        fmul            v16.4s, v16.4s, v17.4s      

        trn1            v21.4s, v19.4s, v19.4s      
        trn2            v19.4s, v19.4s, v19.4s      
        trn1            v17.4s, v18.4s, v18.4s      
        trn2            v18.4s, v18.4s, v18.4s      

        fmul            v21.4s, v21.4s, v31.4s
        fmul            v19.4s, v19.4s, v31.4s
        fmul            v17.4s, v17.4s, v31.4s
        fmul            v18.4s, v18.4s, v31.4s

        fmla            v23.4s, v10.4s, v21.4s      
        fmls            v25.4s, v14.4s, v21.4s      
        fmla            v22.4s, v8.4s, v19.4s      
        fmls            v24.4s, v12.4s, v19.4s      

        fmla            v26.4s, v9.4s, v18.4s      
        fmla            v27.4s, v11.4s, v17.4s      
        fmls            v20.4s, v13.4s, v18.4s      
        fmls            v16.4s, v15.4s, v17.4s      

        fsub            v18.4s, v23.4s, v25.4s
        fsub            v17.4s, v24.4s, v22.4s
        fsub            v19.4s, v20.4s, v26.4s
        fsub            v21.4s, v16.4s, v27.4s

        fadd            v22.4s, v24.4s, v22.4s
        fadd            v23.4s, v25.4s, v23.4s
        fadd            v24.4s, v20.4s, v26.4s
        fadd            v25.4s, v16.4s, v27.4s

        fmul            v17.4s, v17.4s, v31.4s
        fmul            v18.4s, v18.4s, v31.4s
        fmul            v19.4s, v19.4s, v31.4s
        fmul            v21.4s, v21.4s, v31.4s

        rev64           v22.4s, v22.4s
        rev64           v24.4s, v24.4s
        rev64           v23.4s, v23.4s
        rev64           v25.4s, v25.4s

        fsub            v8.4s, v0.4s, v22.4s
        fsub            v9.4s, v1.4s, v24.4s
        fsub            v10.4s, v2.4s, v17.4s
        fsub            v11.4s, v3.4s, v19.4s

        fsub            v12.4s, v4.4s, v23.4s
        fsub            v13.4s, v5.4s, v25.4s
        fadd            v14.4s, v6.4s, v18.4s
        fsub            v15.4s, v7.4s, v21.4s

        fadd            v0.4s, v0.4s, v22.4s
        fadd            v1.4s, v1.4s, v24.4s
        fadd            v2.4s, v2.4s, v17.4s
        fadd            v3.4s, v3.4s, v19.4s

        fadd            v4.4s, v4.4s, v23.4s
        fsub            v6.4s, v6.4s, v18.4s      
        fadd            v5.4s, v5.4s, v25.4s      
        fadd            v7.4s, v7.4s, v21.4s

        stp             q0,  q1,  [x1,  #((0 + 4)*32 + 0)]
        stp             q4,  q5,  [x1,  #((2 + 4)*32 + 0)]
        stp             q2,  q3,  [x10, #((0 + 4)*32 + 0)]
        stp             q6,  q7,  [x10, #((2 + 4)*32 + 0)]

        stp             q8,  q9,  [x11, #((0 + 4)*32 + 0)]
        stp             q12, q13, [x11, #((2 + 4)*32 + 0)]
        stp             q10, q11, [x12, #((0 + 4)*32 + 0)]
        stp             q14, q15, [x12, #((2 + 4)*32 + 0)]
        add             x10, x1, x21
        add             x11, x1, x21, lsl #1
        add             x12, x1, x22

        ldp             q0,  q1,  [x1,  #((0 + 5)*32 + 0)]
        ldp             q4,  q5,  [x1,  #((2 + 5)*32 + 0)]
        ldp             q2,  q3,  [x10, #((0 + 5)*32 + 0)]
        ldp             q6,  q7,  [x10, #((2 + 5)*32 + 0)]

        ldp             q8,  q9,  [x11, #((0 + 5)*32 + 0)]
        ldp             q10, q11, [x11, #((2 + 5)*32 + 0)]
        ldp             q12, q13, [x12, #((0 + 5)*32 + 0)]
        ldp             q14, q15, [x12, #((2 + 5)*32 + 0)]


        ld1             { v24.4s, v25.4s }, [x8], x9
        ld1             { v16.4s, v17.4s }, [x7], #32

        ext8             v18.16b, v24.16b, v24.16b, #8
        ext8             v19.16b, v25.16b, v25.16b, #8

        trn1            v20.4s, v16.4s, v16.4s      
        trn2            v16.4s, v16.4s, v16.4s      
        trn1            v21.4s, v17.4s, v17.4s      
        trn2            v17.4s, v17.4s, v17.4s      

        rev64           v22.4s, v8.4s                 
        rev64           v23.4s, v10.4s                 
        rev64           v24.4s, v12.4s                 
        rev64           v25.4s, v14.4s                 

        fmul            v22.4s, v22.4s, v20.4s      
        fmul            v23.4s, v23.4s, v16.4s      
        fmul            v24.4s, v24.4s, v20.4s      
        fmul            v25.4s, v25.4s, v16.4s      

        rev64           v26.4s, v9.4s                 
        rev64           v27.4s, v11.4s                 
        rev64           v20.4s, v13.4s                 
        rev64           v16.4s, v15.4s                 

        fmul            v26.4s, v26.4s, v21.4s      
        fmul            v27.4s, v27.4s, v17.4s      
        fmul            v20.4s, v20.4s, v21.4s      
        fmul            v16.4s, v16.4s, v17.4s      

        trn1            v21.4s, v19.4s, v19.4s      
        trn2            v19.4s, v19.4s, v19.4s      
        trn1            v17.4s, v18.4s, v18.4s      
        trn2            v18.4s, v18.4s, v18.4s      

        fmul            v21.4s, v21.4s, v31.4s
        fmul            v19.4s, v19.4s, v31.4s
        fmul            v17.4s, v17.4s, v31.4s
        fmul            v18.4s, v18.4s, v31.4s

        fmla            v23.4s, v10.4s, v21.4s      
        fmls            v25.4s, v14.4s, v21.4s      
        fmla            v22.4s, v8.4s, v19.4s      
        fmls            v24.4s, v12.4s, v19.4s      

        fmla            v26.4s, v9.4s, v18.4s      
        fmla            v27.4s, v11.4s, v17.4s      
        fmls            v20.4s, v13.4s, v18.4s      
        fmls            v16.4s, v15.4s, v17.4s      

        fsub            v18.4s, v23.4s, v25.4s
        fsub            v17.4s, v24.4s, v22.4s
        fsub            v19.4s, v20.4s, v26.4s
        fsub            v21.4s, v16.4s, v27.4s

        fadd            v22.4s, v24.4s, v22.4s
        fadd            v23.4s, v25.4s, v23.4s
        fadd            v24.4s, v20.4s, v26.4s
        fadd            v25.4s, v16.4s, v27.4s

        fmul            v17.4s, v17.4s, v31.4s
        fmul            v18.4s, v18.4s, v31.4s
        fmul            v19.4s, v19.4s, v31.4s
        fmul            v21.4s, v21.4s, v31.4s

        rev64           v22.4s, v22.4s
        rev64           v24.4s, v24.4s
        rev64           v23.4s, v23.4s
        rev64           v25.4s, v25.4s

        fsub            v8.4s, v0.4s, v22.4s
        fsub            v9.4s, v1.4s, v24.4s
        fsub            v10.4s, v2.4s, v17.4s
        fsub            v11.4s, v3.4s, v19.4s

        fsub            v12.4s, v4.4s, v23.4s
        fsub            v13.4s, v5.4s, v25.4s
        fadd            v14.4s, v6.4s, v18.4s
        fsub            v15.4s, v7.4s, v21.4s

        fadd            v0.4s, v0.4s, v22.4s
        fadd            v1.4s, v1.4s, v24.4s
        fadd            v2.4s, v2.4s, v17.4s
        fadd            v3.4s, v3.4s, v19.4s

        fadd            v4.4s, v4.4s, v23.4s
        fsub            v6.4s, v6.4s, v18.4s      
        fadd            v5.4s, v5.4s, v25.4s      
        fadd            v7.4s, v7.4s, v21.4s

        stp             q0,  q1,  [x1,  #((0 + 5)*32 + 0)]
        stp             q4,  q5,  [x1,  #((2 + 5)*32 + 0)]
        stp             q2,  q3,  [x10, #((0 + 5)*32 + 0)]
        stp             q6,  q7,  [x10, #((2 + 5)*32 + 0)]

        stp             q8,  q9,  [x11, #((0 + 5)*32 + 0)]
        stp             q12, q13, [x11, #((2 + 5)*32 + 0)]
        stp             q10, q11, [x12, #((0 + 5)*32 + 0)]
        stp             q14, q15, [x12, #((2 + 5)*32 + 0)]
        add             x1, x1, 8*32
        subs            w5, w5, 1
        bgt            temp_label_17

        cmp             w20, #1024
        bgt            temp_label_18

        ret

temp_label_19
temp_label_18

        stp             x20, x30, [sp, #-16]!
        mov             w20, #(2048/4)
        mov             x5, #((2048*4) - (2048/1))
        add             x1, x1, x5
        bl              temp_label_0
        mov             x5, #((2048*2) - (2048/2))
        add             x1, x1, x5
        bl              temp_label_0
        ldp             x20, x30, [sp], #16
        ldr             w5, =(2048*6 + 2048/2)
        sub             x1, x1, x5

        ldr             w5, =(2048 - 4*7)










        adrp            x7, ff_tx_tab_2048_float
        add             x7, x7, ff_tx_tab_2048_float










        add             x8, x7, x5
        mov             x9, #-32

        mov             x21, #2*2048
        add             x22, x21, x21, lsl #1

        cmp             w19, #(2048)
        beq            temp_label_20

        mov             w5, #(2048/128)
temp_label_21

        add             x10, x1, x21
        add             x11, x1, x21, lsl #1
        add             x12, x1, x22

        add             x10, x1, x21
        add             x11, x1, x21, lsl #1
        add             x12, x1, x22

        ldp             q0,  q1,  [x1,  #((0 + 0)*32 + 0)]
        ldp             q4,  q5,  [x1,  #((2 + 0)*32 + 0)]
        ldp             q2,  q3,  [x10, #((0 + 0)*32 + 0)]
        ldp             q6,  q7,  [x10, #((2 + 0)*32 + 0)]

        ldp             q8,  q9,  [x11, #((0 + 0)*32 + 0)]
        ldp             q10, q11, [x11, #((2 + 0)*32 + 0)]
        ldp             q12, q13, [x12, #((0 + 0)*32 + 0)]
        ldp             q14, q15, [x12, #((2 + 0)*32 + 0)]


        ld1             { v24.4s, v25.4s }, [x8], x9
        ld1             { v16.4s, v17.4s }, [x7], #32

        ext8             v18.16b, v24.16b, v24.16b, #8
        ext8             v19.16b, v25.16b, v25.16b, #8

        trn1            v20.4s, v16.4s, v16.4s      
        trn2            v16.4s, v16.4s, v16.4s      
        trn1            v21.4s, v17.4s, v17.4s      
        trn2            v17.4s, v17.4s, v17.4s      

        rev64           v22.4s, v8.4s                 
        rev64           v23.4s, v10.4s                 
        rev64           v24.4s, v12.4s                 
        rev64           v25.4s, v14.4s                 

        fmul            v22.4s, v22.4s, v20.4s      
        fmul            v23.4s, v23.4s, v16.4s      
        fmul            v24.4s, v24.4s, v20.4s      
        fmul            v25.4s, v25.4s, v16.4s      

        rev64           v26.4s, v9.4s                 
        rev64           v27.4s, v11.4s                 
        rev64           v20.4s, v13.4s                 
        rev64           v16.4s, v15.4s                 

        fmul            v26.4s, v26.4s, v21.4s      
        fmul            v27.4s, v27.4s, v17.4s      
        fmul            v20.4s, v20.4s, v21.4s      
        fmul            v16.4s, v16.4s, v17.4s      

        trn1            v21.4s, v19.4s, v19.4s      
        trn2            v19.4s, v19.4s, v19.4s      
        trn1            v17.4s, v18.4s, v18.4s      
        trn2            v18.4s, v18.4s, v18.4s      

        fmul            v21.4s, v21.4s, v31.4s
        fmul            v19.4s, v19.4s, v31.4s
        fmul            v17.4s, v17.4s, v31.4s
        fmul            v18.4s, v18.4s, v31.4s

        fmla            v23.4s, v10.4s, v21.4s      
        fmls            v25.4s, v14.4s, v21.4s      
        fmla            v22.4s, v8.4s, v19.4s      
        fmls            v24.4s, v12.4s, v19.4s      

        fmla            v26.4s, v9.4s, v18.4s      
        fmla            v27.4s, v11.4s, v17.4s      
        fmls            v20.4s, v13.4s, v18.4s      
        fmls            v16.4s, v15.4s, v17.4s      

        fsub            v18.4s, v23.4s, v25.4s
        fsub            v17.4s, v24.4s, v22.4s
        fsub            v19.4s, v20.4s, v26.4s
        fsub            v21.4s, v16.4s, v27.4s

        fadd            v22.4s, v24.4s, v22.4s
        fadd            v23.4s, v25.4s, v23.4s
        fadd            v24.4s, v20.4s, v26.4s
        fadd            v25.4s, v16.4s, v27.4s

        fmul            v17.4s, v17.4s, v31.4s
        fmul            v18.4s, v18.4s, v31.4s
        fmul            v19.4s, v19.4s, v31.4s
        fmul            v21.4s, v21.4s, v31.4s

        rev64           v22.4s, v22.4s
        rev64           v24.4s, v24.4s
        rev64           v23.4s, v23.4s
        rev64           v25.4s, v25.4s

        fsub            v8.4s, v0.4s, v22.4s
        fsub            v9.4s, v1.4s, v24.4s
        fsub            v10.4s, v2.4s, v17.4s
        fsub            v11.4s, v3.4s, v19.4s

        fsub            v12.4s, v4.4s, v23.4s
        fsub            v13.4s, v5.4s, v25.4s
        fadd            v14.4s, v6.4s, v18.4s
        fsub            v15.4s, v7.4s, v21.4s

        fadd            v0.4s, v0.4s, v22.4s
        fadd            v1.4s, v1.4s, v24.4s
        fadd            v2.4s, v2.4s, v17.4s
        fadd            v3.4s, v3.4s, v19.4s

        fadd            v4.4s, v4.4s, v23.4s
        fsub            v6.4s, v6.4s, v18.4s      
        fadd            v5.4s, v5.4s, v25.4s      
        fadd            v7.4s, v7.4s, v21.4s

        stp             q0,  q1,  [x1,  #((0 + 0)*32 + 0)]
        stp             q4,  q5,  [x1,  #((2 + 0)*32 + 0)]
        stp             q2,  q3,  [x10, #((0 + 0)*32 + 0)]
        stp             q6,  q7,  [x10, #((2 + 0)*32 + 0)]

        stp             q8,  q9,  [x11, #((0 + 0)*32 + 0)]
        stp             q12, q13, [x11, #((2 + 0)*32 + 0)]
        stp             q10, q11, [x12, #((0 + 0)*32 + 0)]
        stp             q14, q15, [x12, #((2 + 0)*32 + 0)]
        add             x10, x1, x21
        add             x11, x1, x21, lsl #1
        add             x12, x1, x22

        ldp             q0,  q1,  [x1,  #((0 + 1)*32 + 0)]
        ldp             q4,  q5,  [x1,  #((2 + 1)*32 + 0)]
        ldp             q2,  q3,  [x10, #((0 + 1)*32 + 0)]
        ldp             q6,  q7,  [x10, #((2 + 1)*32 + 0)]

        ldp             q8,  q9,  [x11, #((0 + 1)*32 + 0)]
        ldp             q10, q11, [x11, #((2 + 1)*32 + 0)]
        ldp             q12, q13, [x12, #((0 + 1)*32 + 0)]
        ldp             q14, q15, [x12, #((2 + 1)*32 + 0)]


        ld1             { v24.4s, v25.4s }, [x8], x9
        ld1             { v16.4s, v17.4s }, [x7], #32

        ext8             v18.16b, v24.16b, v24.16b, #8
        ext8             v19.16b, v25.16b, v25.16b, #8

        trn1            v20.4s, v16.4s, v16.4s      
        trn2            v16.4s, v16.4s, v16.4s      
        trn1            v21.4s, v17.4s, v17.4s      
        trn2            v17.4s, v17.4s, v17.4s      

        rev64           v22.4s, v8.4s                 
        rev64           v23.4s, v10.4s                 
        rev64           v24.4s, v12.4s                 
        rev64           v25.4s, v14.4s                 

        fmul            v22.4s, v22.4s, v20.4s      
        fmul            v23.4s, v23.4s, v16.4s      
        fmul            v24.4s, v24.4s, v20.4s      
        fmul            v25.4s, v25.4s, v16.4s      

        rev64           v26.4s, v9.4s                 
        rev64           v27.4s, v11.4s                 
        rev64           v20.4s, v13.4s                 
        rev64           v16.4s, v15.4s                 

        fmul            v26.4s, v26.4s, v21.4s      
        fmul            v27.4s, v27.4s, v17.4s      
        fmul            v20.4s, v20.4s, v21.4s      
        fmul            v16.4s, v16.4s, v17.4s      

        trn1            v21.4s, v19.4s, v19.4s      
        trn2            v19.4s, v19.4s, v19.4s      
        trn1            v17.4s, v18.4s, v18.4s      
        trn2            v18.4s, v18.4s, v18.4s      

        fmul            v21.4s, v21.4s, v31.4s
        fmul            v19.4s, v19.4s, v31.4s
        fmul            v17.4s, v17.4s, v31.4s
        fmul            v18.4s, v18.4s, v31.4s

        fmla            v23.4s, v10.4s, v21.4s      
        fmls            v25.4s, v14.4s, v21.4s      
        fmla            v22.4s, v8.4s, v19.4s      
        fmls            v24.4s, v12.4s, v19.4s      

        fmla            v26.4s, v9.4s, v18.4s      
        fmla            v27.4s, v11.4s, v17.4s      
        fmls            v20.4s, v13.4s, v18.4s      
        fmls            v16.4s, v15.4s, v17.4s      

        fsub            v18.4s, v23.4s, v25.4s
        fsub            v17.4s, v24.4s, v22.4s
        fsub            v19.4s, v20.4s, v26.4s
        fsub            v21.4s, v16.4s, v27.4s

        fadd            v22.4s, v24.4s, v22.4s
        fadd            v23.4s, v25.4s, v23.4s
        fadd            v24.4s, v20.4s, v26.4s
        fadd            v25.4s, v16.4s, v27.4s

        fmul            v17.4s, v17.4s, v31.4s
        fmul            v18.4s, v18.4s, v31.4s
        fmul            v19.4s, v19.4s, v31.4s
        fmul            v21.4s, v21.4s, v31.4s

        rev64           v22.4s, v22.4s
        rev64           v24.4s, v24.4s
        rev64           v23.4s, v23.4s
        rev64           v25.4s, v25.4s

        fsub            v8.4s, v0.4s, v22.4s
        fsub            v9.4s, v1.4s, v24.4s
        fsub            v10.4s, v2.4s, v17.4s
        fsub            v11.4s, v3.4s, v19.4s

        fsub            v12.4s, v4.4s, v23.4s
        fsub            v13.4s, v5.4s, v25.4s
        fadd            v14.4s, v6.4s, v18.4s
        fsub            v15.4s, v7.4s, v21.4s

        fadd            v0.4s, v0.4s, v22.4s
        fadd            v1.4s, v1.4s, v24.4s
        fadd            v2.4s, v2.4s, v17.4s
        fadd            v3.4s, v3.4s, v19.4s

        fadd            v4.4s, v4.4s, v23.4s
        fsub            v6.4s, v6.4s, v18.4s      
        fadd            v5.4s, v5.4s, v25.4s      
        fadd            v7.4s, v7.4s, v21.4s

        stp             q0,  q1,  [x1,  #((0 + 1)*32 + 0)]
        stp             q4,  q5,  [x1,  #((2 + 1)*32 + 0)]
        stp             q2,  q3,  [x10, #((0 + 1)*32 + 0)]
        stp             q6,  q7,  [x10, #((2 + 1)*32 + 0)]

        stp             q8,  q9,  [x11, #((0 + 1)*32 + 0)]
        stp             q12, q13, [x11, #((2 + 1)*32 + 0)]
        stp             q10, q11, [x12, #((0 + 1)*32 + 0)]
        stp             q14, q15, [x12, #((2 + 1)*32 + 0)]
        add             x10, x1, x21
        add             x11, x1, x21, lsl #1
        add             x12, x1, x22

        ldp             q0,  q1,  [x1,  #((0 + 4)*32 + 0)]
        ldp             q4,  q5,  [x1,  #((2 + 4)*32 + 0)]
        ldp             q2,  q3,  [x10, #((0 + 4)*32 + 0)]
        ldp             q6,  q7,  [x10, #((2 + 4)*32 + 0)]

        ldp             q8,  q9,  [x11, #((0 + 4)*32 + 0)]
        ldp             q10, q11, [x11, #((2 + 4)*32 + 0)]
        ldp             q12, q13, [x12, #((0 + 4)*32 + 0)]
        ldp             q14, q15, [x12, #((2 + 4)*32 + 0)]


        ld1             { v24.4s, v25.4s }, [x8], x9
        ld1             { v16.4s, v17.4s }, [x7], #32

        ext8             v18.16b, v24.16b, v24.16b, #8
        ext8             v19.16b, v25.16b, v25.16b, #8

        trn1            v20.4s, v16.4s, v16.4s      
        trn2            v16.4s, v16.4s, v16.4s      
        trn1            v21.4s, v17.4s, v17.4s      
        trn2            v17.4s, v17.4s, v17.4s      

        rev64           v22.4s, v8.4s                 
        rev64           v23.4s, v10.4s                 
        rev64           v24.4s, v12.4s                 
        rev64           v25.4s, v14.4s                 

        fmul            v22.4s, v22.4s, v20.4s      
        fmul            v23.4s, v23.4s, v16.4s      
        fmul            v24.4s, v24.4s, v20.4s      
        fmul            v25.4s, v25.4s, v16.4s      

        rev64           v26.4s, v9.4s                 
        rev64           v27.4s, v11.4s                 
        rev64           v20.4s, v13.4s                 
        rev64           v16.4s, v15.4s                 

        fmul            v26.4s, v26.4s, v21.4s      
        fmul            v27.4s, v27.4s, v17.4s      
        fmul            v20.4s, v20.4s, v21.4s      
        fmul            v16.4s, v16.4s, v17.4s      

        trn1            v21.4s, v19.4s, v19.4s      
        trn2            v19.4s, v19.4s, v19.4s      
        trn1            v17.4s, v18.4s, v18.4s      
        trn2            v18.4s, v18.4s, v18.4s      

        fmul            v21.4s, v21.4s, v31.4s
        fmul            v19.4s, v19.4s, v31.4s
        fmul            v17.4s, v17.4s, v31.4s
        fmul            v18.4s, v18.4s, v31.4s

        fmla            v23.4s, v10.4s, v21.4s      
        fmls            v25.4s, v14.4s, v21.4s      
        fmla            v22.4s, v8.4s, v19.4s      
        fmls            v24.4s, v12.4s, v19.4s      

        fmla            v26.4s, v9.4s, v18.4s      
        fmla            v27.4s, v11.4s, v17.4s      
        fmls            v20.4s, v13.4s, v18.4s      
        fmls            v16.4s, v15.4s, v17.4s      

        fsub            v18.4s, v23.4s, v25.4s
        fsub            v17.4s, v24.4s, v22.4s
        fsub            v19.4s, v20.4s, v26.4s
        fsub            v21.4s, v16.4s, v27.4s

        fadd            v22.4s, v24.4s, v22.4s
        fadd            v23.4s, v25.4s, v23.4s
        fadd            v24.4s, v20.4s, v26.4s
        fadd            v25.4s, v16.4s, v27.4s

        fmul            v17.4s, v17.4s, v31.4s
        fmul            v18.4s, v18.4s, v31.4s
        fmul            v19.4s, v19.4s, v31.4s
        fmul            v21.4s, v21.4s, v31.4s

        rev64           v22.4s, v22.4s
        rev64           v24.4s, v24.4s
        rev64           v23.4s, v23.4s
        rev64           v25.4s, v25.4s

        fsub            v8.4s, v0.4s, v22.4s
        fsub            v9.4s, v1.4s, v24.4s
        fsub            v10.4s, v2.4s, v17.4s
        fsub            v11.4s, v3.4s, v19.4s

        fsub            v12.4s, v4.4s, v23.4s
        fsub            v13.4s, v5.4s, v25.4s
        fadd            v14.4s, v6.4s, v18.4s
        fsub            v15.4s, v7.4s, v21.4s

        fadd            v0.4s, v0.4s, v22.4s
        fadd            v1.4s, v1.4s, v24.4s
        fadd            v2.4s, v2.4s, v17.4s
        fadd            v3.4s, v3.4s, v19.4s

        fadd            v4.4s, v4.4s, v23.4s
        fsub            v6.4s, v6.4s, v18.4s      
        fadd            v5.4s, v5.4s, v25.4s      
        fadd            v7.4s, v7.4s, v21.4s

        stp             q0,  q1,  [x1,  #((0 + 4)*32 + 0)]
        stp             q4,  q5,  [x1,  #((2 + 4)*32 + 0)]
        stp             q2,  q3,  [x10, #((0 + 4)*32 + 0)]
        stp             q6,  q7,  [x10, #((2 + 4)*32 + 0)]

        stp             q8,  q9,  [x11, #((0 + 4)*32 + 0)]
        stp             q12, q13, [x11, #((2 + 4)*32 + 0)]
        stp             q10, q11, [x12, #((0 + 4)*32 + 0)]
        stp             q14, q15, [x12, #((2 + 4)*32 + 0)]
        add             x10, x1, x21
        add             x11, x1, x21, lsl #1
        add             x12, x1, x22

        ldp             q0,  q1,  [x1,  #((0 + 5)*32 + 0)]
        ldp             q4,  q5,  [x1,  #((2 + 5)*32 + 0)]
        ldp             q2,  q3,  [x10, #((0 + 5)*32 + 0)]
        ldp             q6,  q7,  [x10, #((2 + 5)*32 + 0)]

        ldp             q8,  q9,  [x11, #((0 + 5)*32 + 0)]
        ldp             q10, q11, [x11, #((2 + 5)*32 + 0)]
        ldp             q12, q13, [x12, #((0 + 5)*32 + 0)]
        ldp             q14, q15, [x12, #((2 + 5)*32 + 0)]


        ld1             { v24.4s, v25.4s }, [x8], x9
        ld1             { v16.4s, v17.4s }, [x7], #32

        ext8             v18.16b, v24.16b, v24.16b, #8
        ext8             v19.16b, v25.16b, v25.16b, #8

        trn1            v20.4s, v16.4s, v16.4s      
        trn2            v16.4s, v16.4s, v16.4s      
        trn1            v21.4s, v17.4s, v17.4s      
        trn2            v17.4s, v17.4s, v17.4s      

        rev64           v22.4s, v8.4s                 
        rev64           v23.4s, v10.4s                 
        rev64           v24.4s, v12.4s                 
        rev64           v25.4s, v14.4s                 

        fmul            v22.4s, v22.4s, v20.4s      
        fmul            v23.4s, v23.4s, v16.4s      
        fmul            v24.4s, v24.4s, v20.4s      
        fmul            v25.4s, v25.4s, v16.4s      

        rev64           v26.4s, v9.4s                 
        rev64           v27.4s, v11.4s                 
        rev64           v20.4s, v13.4s                 
        rev64           v16.4s, v15.4s                 

        fmul            v26.4s, v26.4s, v21.4s      
        fmul            v27.4s, v27.4s, v17.4s      
        fmul            v20.4s, v20.4s, v21.4s      
        fmul            v16.4s, v16.4s, v17.4s      

        trn1            v21.4s, v19.4s, v19.4s      
        trn2            v19.4s, v19.4s, v19.4s      
        trn1            v17.4s, v18.4s, v18.4s      
        trn2            v18.4s, v18.4s, v18.4s      

        fmul            v21.4s, v21.4s, v31.4s
        fmul            v19.4s, v19.4s, v31.4s
        fmul            v17.4s, v17.4s, v31.4s
        fmul            v18.4s, v18.4s, v31.4s

        fmla            v23.4s, v10.4s, v21.4s      
        fmls            v25.4s, v14.4s, v21.4s      
        fmla            v22.4s, v8.4s, v19.4s      
        fmls            v24.4s, v12.4s, v19.4s      

        fmla            v26.4s, v9.4s, v18.4s      
        fmla            v27.4s, v11.4s, v17.4s      
        fmls            v20.4s, v13.4s, v18.4s      
        fmls            v16.4s, v15.4s, v17.4s      

        fsub            v18.4s, v23.4s, v25.4s
        fsub            v17.4s, v24.4s, v22.4s
        fsub            v19.4s, v20.4s, v26.4s
        fsub            v21.4s, v16.4s, v27.4s

        fadd            v22.4s, v24.4s, v22.4s
        fadd            v23.4s, v25.4s, v23.4s
        fadd            v24.4s, v20.4s, v26.4s
        fadd            v25.4s, v16.4s, v27.4s

        fmul            v17.4s, v17.4s, v31.4s
        fmul            v18.4s, v18.4s, v31.4s
        fmul            v19.4s, v19.4s, v31.4s
        fmul            v21.4s, v21.4s, v31.4s

        rev64           v22.4s, v22.4s
        rev64           v24.4s, v24.4s
        rev64           v23.4s, v23.4s
        rev64           v25.4s, v25.4s

        fsub            v8.4s, v0.4s, v22.4s
        fsub            v9.4s, v1.4s, v24.4s
        fsub            v10.4s, v2.4s, v17.4s
        fsub            v11.4s, v3.4s, v19.4s

        fsub            v12.4s, v4.4s, v23.4s
        fsub            v13.4s, v5.4s, v25.4s
        fadd            v14.4s, v6.4s, v18.4s
        fsub            v15.4s, v7.4s, v21.4s

        fadd            v0.4s, v0.4s, v22.4s
        fadd            v1.4s, v1.4s, v24.4s
        fadd            v2.4s, v2.4s, v17.4s
        fadd            v3.4s, v3.4s, v19.4s

        fadd            v4.4s, v4.4s, v23.4s
        fsub            v6.4s, v6.4s, v18.4s      
        fadd            v5.4s, v5.4s, v25.4s      
        fadd            v7.4s, v7.4s, v21.4s

        stp             q0,  q1,  [x1,  #((0 + 5)*32 + 0)]
        stp             q4,  q5,  [x1,  #((2 + 5)*32 + 0)]
        stp             q2,  q3,  [x10, #((0 + 5)*32 + 0)]
        stp             q6,  q7,  [x10, #((2 + 5)*32 + 0)]

        stp             q8,  q9,  [x11, #((0 + 5)*32 + 0)]
        stp             q12, q13, [x11, #((2 + 5)*32 + 0)]
        stp             q10, q11, [x12, #((0 + 5)*32 + 0)]
        stp             q14, q15, [x12, #((2 + 5)*32 + 0)]
        add             x1, x1, 8*32
        subs            w5, w5, 1
        bgt            temp_label_21

        cmp             w20, #(2048)
        bgt            temp_label_22
        ret
temp_label_23
temp_label_22

        stp             x20, x30, [sp, #-16]!
        mov             w20, #(4096/4)
        mov             x5, #((4096*4) - (4096/1))
        add             x1, x1, x5
        bl              temp_label_0
        mov             x5, #((4096*2) - (4096/2))
        add             x1, x1, x5
        bl              temp_label_0
        ldp             x20, x30, [sp], #16
        ldr             w5, =(4096*6 + 4096/2)
        sub             x1, x1, x5

        ldr             w5, =(4096 - 4*7)










        adrp            x7, ff_tx_tab_4096_float
        add             x7, x7, ff_tx_tab_4096_float










        add             x8, x7, x5
        mov             x9, #-32

        mov             x21, #2*4096
        add             x22, x21, x21, lsl #1

        cmp             w19, #((4096)/4096), lsl #12
        beq            temp_label_24

        mov             w5, #(4096/128)
temp_label_25

        add             x10, x1, x21
        add             x11, x1, x21, lsl #1
        add             x12, x1, x22

        add             x10, x1, x21
        add             x11, x1, x21, lsl #1
        add             x12, x1, x22

        ldp             q0,  q1,  [x1,  #((0 + 0)*32 + 0)]
        ldp             q4,  q5,  [x1,  #((2 + 0)*32 + 0)]
        ldp             q2,  q3,  [x10, #((0 + 0)*32 + 0)]
        ldp             q6,  q7,  [x10, #((2 + 0)*32 + 0)]

        ldp             q8,  q9,  [x11, #((0 + 0)*32 + 0)]
        ldp             q10, q11, [x11, #((2 + 0)*32 + 0)]
        ldp             q12, q13, [x12, #((0 + 0)*32 + 0)]
        ldp             q14, q15, [x12, #((2 + 0)*32 + 0)]


        ld1             { v24.4s, v25.4s }, [x8], x9
        ld1             { v16.4s, v17.4s }, [x7], #32

        ext8             v18.16b, v24.16b, v24.16b, #8
        ext8             v19.16b, v25.16b, v25.16b, #8

        trn1            v20.4s, v16.4s, v16.4s      
        trn2            v16.4s, v16.4s, v16.4s      
        trn1            v21.4s, v17.4s, v17.4s      
        trn2            v17.4s, v17.4s, v17.4s      

        rev64           v22.4s, v8.4s                 
        rev64           v23.4s, v10.4s                 
        rev64           v24.4s, v12.4s                 
        rev64           v25.4s, v14.4s                 

        fmul            v22.4s, v22.4s, v20.4s      
        fmul            v23.4s, v23.4s, v16.4s      
        fmul            v24.4s, v24.4s, v20.4s      
        fmul            v25.4s, v25.4s, v16.4s      

        rev64           v26.4s, v9.4s                 
        rev64           v27.4s, v11.4s                 
        rev64           v20.4s, v13.4s                 
        rev64           v16.4s, v15.4s                 

        fmul            v26.4s, v26.4s, v21.4s      
        fmul            v27.4s, v27.4s, v17.4s      
        fmul            v20.4s, v20.4s, v21.4s      
        fmul            v16.4s, v16.4s, v17.4s      

        trn1            v21.4s, v19.4s, v19.4s      
        trn2            v19.4s, v19.4s, v19.4s      
        trn1            v17.4s, v18.4s, v18.4s      
        trn2            v18.4s, v18.4s, v18.4s      

        fmul            v21.4s, v21.4s, v31.4s
        fmul            v19.4s, v19.4s, v31.4s
        fmul            v17.4s, v17.4s, v31.4s
        fmul            v18.4s, v18.4s, v31.4s

        fmla            v23.4s, v10.4s, v21.4s      
        fmls            v25.4s, v14.4s, v21.4s      
        fmla            v22.4s, v8.4s, v19.4s      
        fmls            v24.4s, v12.4s, v19.4s      

        fmla            v26.4s, v9.4s, v18.4s      
        fmla            v27.4s, v11.4s, v17.4s      
        fmls            v20.4s, v13.4s, v18.4s      
        fmls            v16.4s, v15.4s, v17.4s      

        fsub            v18.4s, v23.4s, v25.4s
        fsub            v17.4s, v24.4s, v22.4s
        fsub            v19.4s, v20.4s, v26.4s
        fsub            v21.4s, v16.4s, v27.4s

        fadd            v22.4s, v24.4s, v22.4s
        fadd            v23.4s, v25.4s, v23.4s
        fadd            v24.4s, v20.4s, v26.4s
        fadd            v25.4s, v16.4s, v27.4s

        fmul            v17.4s, v17.4s, v31.4s
        fmul            v18.4s, v18.4s, v31.4s
        fmul            v19.4s, v19.4s, v31.4s
        fmul            v21.4s, v21.4s, v31.4s

        rev64           v22.4s, v22.4s
        rev64           v24.4s, v24.4s
        rev64           v23.4s, v23.4s
        rev64           v25.4s, v25.4s

        fsub            v8.4s, v0.4s, v22.4s
        fsub            v9.4s, v1.4s, v24.4s
        fsub            v10.4s, v2.4s, v17.4s
        fsub            v11.4s, v3.4s, v19.4s

        fsub            v12.4s, v4.4s, v23.4s
        fsub            v13.4s, v5.4s, v25.4s
        fadd            v14.4s, v6.4s, v18.4s
        fsub            v15.4s, v7.4s, v21.4s

        fadd            v0.4s, v0.4s, v22.4s
        fadd            v1.4s, v1.4s, v24.4s
        fadd            v2.4s, v2.4s, v17.4s
        fadd            v3.4s, v3.4s, v19.4s

        fadd            v4.4s, v4.4s, v23.4s
        fsub            v6.4s, v6.4s, v18.4s      
        fadd            v5.4s, v5.4s, v25.4s      
        fadd            v7.4s, v7.4s, v21.4s

        stp             q0,  q1,  [x1,  #((0 + 0)*32 + 0)]
        stp             q4,  q5,  [x1,  #((2 + 0)*32 + 0)]
        stp             q2,  q3,  [x10, #((0 + 0)*32 + 0)]
        stp             q6,  q7,  [x10, #((2 + 0)*32 + 0)]

        stp             q8,  q9,  [x11, #((0 + 0)*32 + 0)]
        stp             q12, q13, [x11, #((2 + 0)*32 + 0)]
        stp             q10, q11, [x12, #((0 + 0)*32 + 0)]
        stp             q14, q15, [x12, #((2 + 0)*32 + 0)]
        add             x10, x1, x21
        add             x11, x1, x21, lsl #1
        add             x12, x1, x22

        ldp             q0,  q1,  [x1,  #((0 + 1)*32 + 0)]
        ldp             q4,  q5,  [x1,  #((2 + 1)*32 + 0)]
        ldp             q2,  q3,  [x10, #((0 + 1)*32 + 0)]
        ldp             q6,  q7,  [x10, #((2 + 1)*32 + 0)]

        ldp             q8,  q9,  [x11, #((0 + 1)*32 + 0)]
        ldp             q10, q11, [x11, #((2 + 1)*32 + 0)]
        ldp             q12, q13, [x12, #((0 + 1)*32 + 0)]
        ldp             q14, q15, [x12, #((2 + 1)*32 + 0)]


        ld1             { v24.4s, v25.4s }, [x8], x9
        ld1             { v16.4s, v17.4s }, [x7], #32

        ext8             v18.16b, v24.16b, v24.16b, #8
        ext8             v19.16b, v25.16b, v25.16b, #8

        trn1            v20.4s, v16.4s, v16.4s      
        trn2            v16.4s, v16.4s, v16.4s      
        trn1            v21.4s, v17.4s, v17.4s      
        trn2            v17.4s, v17.4s, v17.4s      

        rev64           v22.4s, v8.4s                 
        rev64           v23.4s, v10.4s                 
        rev64           v24.4s, v12.4s                 
        rev64           v25.4s, v14.4s                 

        fmul            v22.4s, v22.4s, v20.4s      
        fmul            v23.4s, v23.4s, v16.4s      
        fmul            v24.4s, v24.4s, v20.4s      
        fmul            v25.4s, v25.4s, v16.4s      

        rev64           v26.4s, v9.4s                 
        rev64           v27.4s, v11.4s                 
        rev64           v20.4s, v13.4s                 
        rev64           v16.4s, v15.4s                 

        fmul            v26.4s, v26.4s, v21.4s      
        fmul            v27.4s, v27.4s, v17.4s      
        fmul            v20.4s, v20.4s, v21.4s      
        fmul            v16.4s, v16.4s, v17.4s      

        trn1            v21.4s, v19.4s, v19.4s      
        trn2            v19.4s, v19.4s, v19.4s      
        trn1            v17.4s, v18.4s, v18.4s      
        trn2            v18.4s, v18.4s, v18.4s      

        fmul            v21.4s, v21.4s, v31.4s
        fmul            v19.4s, v19.4s, v31.4s
        fmul            v17.4s, v17.4s, v31.4s
        fmul            v18.4s, v18.4s, v31.4s

        fmla            v23.4s, v10.4s, v21.4s      
        fmls            v25.4s, v14.4s, v21.4s      
        fmla            v22.4s, v8.4s, v19.4s      
        fmls            v24.4s, v12.4s, v19.4s      

        fmla            v26.4s, v9.4s, v18.4s      
        fmla            v27.4s, v11.4s, v17.4s      
        fmls            v20.4s, v13.4s, v18.4s      
        fmls            v16.4s, v15.4s, v17.4s      

        fsub            v18.4s, v23.4s, v25.4s
        fsub            v17.4s, v24.4s, v22.4s
        fsub            v19.4s, v20.4s, v26.4s
        fsub            v21.4s, v16.4s, v27.4s

        fadd            v22.4s, v24.4s, v22.4s
        fadd            v23.4s, v25.4s, v23.4s
        fadd            v24.4s, v20.4s, v26.4s
        fadd            v25.4s, v16.4s, v27.4s

        fmul            v17.4s, v17.4s, v31.4s
        fmul            v18.4s, v18.4s, v31.4s
        fmul            v19.4s, v19.4s, v31.4s
        fmul            v21.4s, v21.4s, v31.4s

        rev64           v22.4s, v22.4s
        rev64           v24.4s, v24.4s
        rev64           v23.4s, v23.4s
        rev64           v25.4s, v25.4s

        fsub            v8.4s, v0.4s, v22.4s
        fsub            v9.4s, v1.4s, v24.4s
        fsub            v10.4s, v2.4s, v17.4s
        fsub            v11.4s, v3.4s, v19.4s

        fsub            v12.4s, v4.4s, v23.4s
        fsub            v13.4s, v5.4s, v25.4s
        fadd            v14.4s, v6.4s, v18.4s
        fsub            v15.4s, v7.4s, v21.4s

        fadd            v0.4s, v0.4s, v22.4s
        fadd            v1.4s, v1.4s, v24.4s
        fadd            v2.4s, v2.4s, v17.4s
        fadd            v3.4s, v3.4s, v19.4s

        fadd            v4.4s, v4.4s, v23.4s
        fsub            v6.4s, v6.4s, v18.4s      
        fadd            v5.4s, v5.4s, v25.4s      
        fadd            v7.4s, v7.4s, v21.4s

        stp             q0,  q1,  [x1,  #((0 + 1)*32 + 0)]
        stp             q4,  q5,  [x1,  #((2 + 1)*32 + 0)]
        stp             q2,  q3,  [x10, #((0 + 1)*32 + 0)]
        stp             q6,  q7,  [x10, #((2 + 1)*32 + 0)]

        stp             q8,  q9,  [x11, #((0 + 1)*32 + 0)]
        stp             q12, q13, [x11, #((2 + 1)*32 + 0)]
        stp             q10, q11, [x12, #((0 + 1)*32 + 0)]
        stp             q14, q15, [x12, #((2 + 1)*32 + 0)]
        add             x10, x1, x21
        add             x11, x1, x21, lsl #1
        add             x12, x1, x22

        ldp             q0,  q1,  [x1,  #((0 + 4)*32 + 0)]
        ldp             q4,  q5,  [x1,  #((2 + 4)*32 + 0)]
        ldp             q2,  q3,  [x10, #((0 + 4)*32 + 0)]
        ldp             q6,  q7,  [x10, #((2 + 4)*32 + 0)]

        ldp             q8,  q9,  [x11, #((0 + 4)*32 + 0)]
        ldp             q10, q11, [x11, #((2 + 4)*32 + 0)]
        ldp             q12, q13, [x12, #((0 + 4)*32 + 0)]
        ldp             q14, q15, [x12, #((2 + 4)*32 + 0)]


        ld1             { v24.4s, v25.4s }, [x8], x9
        ld1             { v16.4s, v17.4s }, [x7], #32

        ext8             v18.16b, v24.16b, v24.16b, #8
        ext8             v19.16b, v25.16b, v25.16b, #8

        trn1            v20.4s, v16.4s, v16.4s      
        trn2            v16.4s, v16.4s, v16.4s      
        trn1            v21.4s, v17.4s, v17.4s      
        trn2            v17.4s, v17.4s, v17.4s      

        rev64           v22.4s, v8.4s                 
        rev64           v23.4s, v10.4s                 
        rev64           v24.4s, v12.4s                 
        rev64           v25.4s, v14.4s                 

        fmul            v22.4s, v22.4s, v20.4s      
        fmul            v23.4s, v23.4s, v16.4s      
        fmul            v24.4s, v24.4s, v20.4s      
        fmul            v25.4s, v25.4s, v16.4s      

        rev64           v26.4s, v9.4s                 
        rev64           v27.4s, v11.4s                 
        rev64           v20.4s, v13.4s                 
        rev64           v16.4s, v15.4s                 

        fmul            v26.4s, v26.4s, v21.4s      
        fmul            v27.4s, v27.4s, v17.4s      
        fmul            v20.4s, v20.4s, v21.4s      
        fmul            v16.4s, v16.4s, v17.4s      

        trn1            v21.4s, v19.4s, v19.4s      
        trn2            v19.4s, v19.4s, v19.4s      
        trn1            v17.4s, v18.4s, v18.4s      
        trn2            v18.4s, v18.4s, v18.4s      

        fmul            v21.4s, v21.4s, v31.4s
        fmul            v19.4s, v19.4s, v31.4s
        fmul            v17.4s, v17.4s, v31.4s
        fmul            v18.4s, v18.4s, v31.4s

        fmla            v23.4s, v10.4s, v21.4s      
        fmls            v25.4s, v14.4s, v21.4s      
        fmla            v22.4s, v8.4s, v19.4s      
        fmls            v24.4s, v12.4s, v19.4s      

        fmla            v26.4s, v9.4s, v18.4s      
        fmla            v27.4s, v11.4s, v17.4s      
        fmls            v20.4s, v13.4s, v18.4s      
        fmls            v16.4s, v15.4s, v17.4s      

        fsub            v18.4s, v23.4s, v25.4s
        fsub            v17.4s, v24.4s, v22.4s
        fsub            v19.4s, v20.4s, v26.4s
        fsub            v21.4s, v16.4s, v27.4s

        fadd            v22.4s, v24.4s, v22.4s
        fadd            v23.4s, v25.4s, v23.4s
        fadd            v24.4s, v20.4s, v26.4s
        fadd            v25.4s, v16.4s, v27.4s

        fmul            v17.4s, v17.4s, v31.4s
        fmul            v18.4s, v18.4s, v31.4s
        fmul            v19.4s, v19.4s, v31.4s
        fmul            v21.4s, v21.4s, v31.4s

        rev64           v22.4s, v22.4s
        rev64           v24.4s, v24.4s
        rev64           v23.4s, v23.4s
        rev64           v25.4s, v25.4s

        fsub            v8.4s, v0.4s, v22.4s
        fsub            v9.4s, v1.4s, v24.4s
        fsub            v10.4s, v2.4s, v17.4s
        fsub            v11.4s, v3.4s, v19.4s

        fsub            v12.4s, v4.4s, v23.4s
        fsub            v13.4s, v5.4s, v25.4s
        fadd            v14.4s, v6.4s, v18.4s
        fsub            v15.4s, v7.4s, v21.4s

        fadd            v0.4s, v0.4s, v22.4s
        fadd            v1.4s, v1.4s, v24.4s
        fadd            v2.4s, v2.4s, v17.4s
        fadd            v3.4s, v3.4s, v19.4s

        fadd            v4.4s, v4.4s, v23.4s
        fsub            v6.4s, v6.4s, v18.4s      
        fadd            v5.4s, v5.4s, v25.4s      
        fadd            v7.4s, v7.4s, v21.4s

        stp             q0,  q1,  [x1,  #((0 + 4)*32 + 0)]
        stp             q4,  q5,  [x1,  #((2 + 4)*32 + 0)]
        stp             q2,  q3,  [x10, #((0 + 4)*32 + 0)]
        stp             q6,  q7,  [x10, #((2 + 4)*32 + 0)]

        stp             q8,  q9,  [x11, #((0 + 4)*32 + 0)]
        stp             q12, q13, [x11, #((2 + 4)*32 + 0)]
        stp             q10, q11, [x12, #((0 + 4)*32 + 0)]
        stp             q14, q15, [x12, #((2 + 4)*32 + 0)]
        add             x10, x1, x21
        add             x11, x1, x21, lsl #1
        add             x12, x1, x22

        ldp             q0,  q1,  [x1,  #((0 + 5)*32 + 0)]
        ldp             q4,  q5,  [x1,  #((2 + 5)*32 + 0)]
        ldp             q2,  q3,  [x10, #((0 + 5)*32 + 0)]
        ldp             q6,  q7,  [x10, #((2 + 5)*32 + 0)]

        ldp             q8,  q9,  [x11, #((0 + 5)*32 + 0)]
        ldp             q10, q11, [x11, #((2 + 5)*32 + 0)]
        ldp             q12, q13, [x12, #((0 + 5)*32 + 0)]
        ldp             q14, q15, [x12, #((2 + 5)*32 + 0)]


        ld1             { v24.4s, v25.4s }, [x8], x9
        ld1             { v16.4s, v17.4s }, [x7], #32

        ext8             v18.16b, v24.16b, v24.16b, #8
        ext8             v19.16b, v25.16b, v25.16b, #8

        trn1            v20.4s, v16.4s, v16.4s      
        trn2            v16.4s, v16.4s, v16.4s      
        trn1            v21.4s, v17.4s, v17.4s      
        trn2            v17.4s, v17.4s, v17.4s      

        rev64           v22.4s, v8.4s                 
        rev64           v23.4s, v10.4s                 
        rev64           v24.4s, v12.4s                 
        rev64           v25.4s, v14.4s                 

        fmul            v22.4s, v22.4s, v20.4s      
        fmul            v23.4s, v23.4s, v16.4s      
        fmul            v24.4s, v24.4s, v20.4s      
        fmul            v25.4s, v25.4s, v16.4s      

        rev64           v26.4s, v9.4s                 
        rev64           v27.4s, v11.4s                 
        rev64           v20.4s, v13.4s                 
        rev64           v16.4s, v15.4s                 

        fmul            v26.4s, v26.4s, v21.4s      
        fmul            v27.4s, v27.4s, v17.4s      
        fmul            v20.4s, v20.4s, v21.4s      
        fmul            v16.4s, v16.4s, v17.4s      

        trn1            v21.4s, v19.4s, v19.4s      
        trn2            v19.4s, v19.4s, v19.4s      
        trn1            v17.4s, v18.4s, v18.4s      
        trn2            v18.4s, v18.4s, v18.4s      

        fmul            v21.4s, v21.4s, v31.4s
        fmul            v19.4s, v19.4s, v31.4s
        fmul            v17.4s, v17.4s, v31.4s
        fmul            v18.4s, v18.4s, v31.4s

        fmla            v23.4s, v10.4s, v21.4s      
        fmls            v25.4s, v14.4s, v21.4s      
        fmla            v22.4s, v8.4s, v19.4s      
        fmls            v24.4s, v12.4s, v19.4s      

        fmla            v26.4s, v9.4s, v18.4s      
        fmla            v27.4s, v11.4s, v17.4s      
        fmls            v20.4s, v13.4s, v18.4s      
        fmls            v16.4s, v15.4s, v17.4s      

        fsub            v18.4s, v23.4s, v25.4s
        fsub            v17.4s, v24.4s, v22.4s
        fsub            v19.4s, v20.4s, v26.4s
        fsub            v21.4s, v16.4s, v27.4s

        fadd            v22.4s, v24.4s, v22.4s
        fadd            v23.4s, v25.4s, v23.4s
        fadd            v24.4s, v20.4s, v26.4s
        fadd            v25.4s, v16.4s, v27.4s

        fmul            v17.4s, v17.4s, v31.4s
        fmul            v18.4s, v18.4s, v31.4s
        fmul            v19.4s, v19.4s, v31.4s
        fmul            v21.4s, v21.4s, v31.4s

        rev64           v22.4s, v22.4s
        rev64           v24.4s, v24.4s
        rev64           v23.4s, v23.4s
        rev64           v25.4s, v25.4s

        fsub            v8.4s, v0.4s, v22.4s
        fsub            v9.4s, v1.4s, v24.4s
        fsub            v10.4s, v2.4s, v17.4s
        fsub            v11.4s, v3.4s, v19.4s

        fsub            v12.4s, v4.4s, v23.4s
        fsub            v13.4s, v5.4s, v25.4s
        fadd            v14.4s, v6.4s, v18.4s
        fsub            v15.4s, v7.4s, v21.4s

        fadd            v0.4s, v0.4s, v22.4s
        fadd            v1.4s, v1.4s, v24.4s
        fadd            v2.4s, v2.4s, v17.4s
        fadd            v3.4s, v3.4s, v19.4s

        fadd            v4.4s, v4.4s, v23.4s
        fsub            v6.4s, v6.4s, v18.4s      
        fadd            v5.4s, v5.4s, v25.4s      
        fadd            v7.4s, v7.4s, v21.4s

        stp             q0,  q1,  [x1,  #((0 + 5)*32 + 0)]
        stp             q4,  q5,  [x1,  #((2 + 5)*32 + 0)]
        stp             q2,  q3,  [x10, #((0 + 5)*32 + 0)]
        stp             q6,  q7,  [x10, #((2 + 5)*32 + 0)]

        stp             q8,  q9,  [x11, #((0 + 5)*32 + 0)]
        stp             q12, q13, [x11, #((2 + 5)*32 + 0)]
        stp             q10, q11, [x12, #((0 + 5)*32 + 0)]
        stp             q14, q15, [x12, #((2 + 5)*32 + 0)]
        add             x1, x1, 8*32
        subs            w5, w5, 1
        bgt            temp_label_25

        cmp             w20, #((4096)/4096), lsl #12
        bgt            temp_label_26
        ret
temp_label_27
temp_label_26

        stp             x20, x30, [sp, #-16]!
        mov             w20, #(8192/4)
        mov             x5, #((8192*4) - (8192/1))
        add             x1, x1, x5
        bl              temp_label_0
        mov             x5, #((8192*2) - (8192/2))
        add             x1, x1, x5
        bl              temp_label_0
        ldp             x20, x30, [sp], #16
        ldr             w5, =(8192*6 + 8192/2)
        sub             x1, x1, x5

        ldr             w5, =(8192 - 4*7)










        adrp            x7, ff_tx_tab_8192_float
        add             x7, x7, ff_tx_tab_8192_float










        add             x8, x7, x5
        mov             x9, #-32

        mov             x21, #2*8192
        add             x22, x21, x21, lsl #1

        cmp             w19, #((8192)/4096), lsl #12
        beq            temp_label_28

        mov             w5, #(8192/128)
temp_label_29

        add             x10, x1, x21
        add             x11, x1, x21, lsl #1
        add             x12, x1, x22

        add             x10, x1, x21
        add             x11, x1, x21, lsl #1
        add             x12, x1, x22

        ldp             q0,  q1,  [x1,  #((0 + 0)*32 + 0)]
        ldp             q4,  q5,  [x1,  #((2 + 0)*32 + 0)]
        ldp             q2,  q3,  [x10, #((0 + 0)*32 + 0)]
        ldp             q6,  q7,  [x10, #((2 + 0)*32 + 0)]

        ldp             q8,  q9,  [x11, #((0 + 0)*32 + 0)]
        ldp             q10, q11, [x11, #((2 + 0)*32 + 0)]
        ldp             q12, q13, [x12, #((0 + 0)*32 + 0)]
        ldp             q14, q15, [x12, #((2 + 0)*32 + 0)]


        ld1             { v24.4s, v25.4s }, [x8], x9
        ld1             { v16.4s, v17.4s }, [x7], #32

        ext8             v18.16b, v24.16b, v24.16b, #8
        ext8             v19.16b, v25.16b, v25.16b, #8

        trn1            v20.4s, v16.4s, v16.4s      
        trn2            v16.4s, v16.4s, v16.4s      
        trn1            v21.4s, v17.4s, v17.4s      
        trn2            v17.4s, v17.4s, v17.4s      

        rev64           v22.4s, v8.4s                 
        rev64           v23.4s, v10.4s                 
        rev64           v24.4s, v12.4s                 
        rev64           v25.4s, v14.4s                 

        fmul            v22.4s, v22.4s, v20.4s      
        fmul            v23.4s, v23.4s, v16.4s      
        fmul            v24.4s, v24.4s, v20.4s      
        fmul            v25.4s, v25.4s, v16.4s      

        rev64           v26.4s, v9.4s                 
        rev64           v27.4s, v11.4s                 
        rev64           v20.4s, v13.4s                 
        rev64           v16.4s, v15.4s                 

        fmul            v26.4s, v26.4s, v21.4s      
        fmul            v27.4s, v27.4s, v17.4s      
        fmul            v20.4s, v20.4s, v21.4s      
        fmul            v16.4s, v16.4s, v17.4s      

        trn1            v21.4s, v19.4s, v19.4s      
        trn2            v19.4s, v19.4s, v19.4s      
        trn1            v17.4s, v18.4s, v18.4s      
        trn2            v18.4s, v18.4s, v18.4s      

        fmul            v21.4s, v21.4s, v31.4s
        fmul            v19.4s, v19.4s, v31.4s
        fmul            v17.4s, v17.4s, v31.4s
        fmul            v18.4s, v18.4s, v31.4s

        fmla            v23.4s, v10.4s, v21.4s      
        fmls            v25.4s, v14.4s, v21.4s      
        fmla            v22.4s, v8.4s, v19.4s      
        fmls            v24.4s, v12.4s, v19.4s      

        fmla            v26.4s, v9.4s, v18.4s      
        fmla            v27.4s, v11.4s, v17.4s      
        fmls            v20.4s, v13.4s, v18.4s      
        fmls            v16.4s, v15.4s, v17.4s      

        fsub            v18.4s, v23.4s, v25.4s
        fsub            v17.4s, v24.4s, v22.4s
        fsub            v19.4s, v20.4s, v26.4s
        fsub            v21.4s, v16.4s, v27.4s

        fadd            v22.4s, v24.4s, v22.4s
        fadd            v23.4s, v25.4s, v23.4s
        fadd            v24.4s, v20.4s, v26.4s
        fadd            v25.4s, v16.4s, v27.4s

        fmul            v17.4s, v17.4s, v31.4s
        fmul            v18.4s, v18.4s, v31.4s
        fmul            v19.4s, v19.4s, v31.4s
        fmul            v21.4s, v21.4s, v31.4s

        rev64           v22.4s, v22.4s
        rev64           v24.4s, v24.4s
        rev64           v23.4s, v23.4s
        rev64           v25.4s, v25.4s

        fsub            v8.4s, v0.4s, v22.4s
        fsub            v9.4s, v1.4s, v24.4s
        fsub            v10.4s, v2.4s, v17.4s
        fsub            v11.4s, v3.4s, v19.4s

        fsub            v12.4s, v4.4s, v23.4s
        fsub            v13.4s, v5.4s, v25.4s
        fadd            v14.4s, v6.4s, v18.4s
        fsub            v15.4s, v7.4s, v21.4s

        fadd            v0.4s, v0.4s, v22.4s
        fadd            v1.4s, v1.4s, v24.4s
        fadd            v2.4s, v2.4s, v17.4s
        fadd            v3.4s, v3.4s, v19.4s

        fadd            v4.4s, v4.4s, v23.4s
        fsub            v6.4s, v6.4s, v18.4s      
        fadd            v5.4s, v5.4s, v25.4s      
        fadd            v7.4s, v7.4s, v21.4s

        stp             q0,  q1,  [x1,  #((0 + 0)*32 + 0)]
        stp             q4,  q5,  [x1,  #((2 + 0)*32 + 0)]
        stp             q2,  q3,  [x10, #((0 + 0)*32 + 0)]
        stp             q6,  q7,  [x10, #((2 + 0)*32 + 0)]

        stp             q8,  q9,  [x11, #((0 + 0)*32 + 0)]
        stp             q12, q13, [x11, #((2 + 0)*32 + 0)]
        stp             q10, q11, [x12, #((0 + 0)*32 + 0)]
        stp             q14, q15, [x12, #((2 + 0)*32 + 0)]
        add             x10, x1, x21
        add             x11, x1, x21, lsl #1
        add             x12, x1, x22

        ldp             q0,  q1,  [x1,  #((0 + 1)*32 + 0)]
        ldp             q4,  q5,  [x1,  #((2 + 1)*32 + 0)]
        ldp             q2,  q3,  [x10, #((0 + 1)*32 + 0)]
        ldp             q6,  q7,  [x10, #((2 + 1)*32 + 0)]

        ldp             q8,  q9,  [x11, #((0 + 1)*32 + 0)]
        ldp             q10, q11, [x11, #((2 + 1)*32 + 0)]
        ldp             q12, q13, [x12, #((0 + 1)*32 + 0)]
        ldp             q14, q15, [x12, #((2 + 1)*32 + 0)]


        ld1             { v24.4s, v25.4s }, [x8], x9
        ld1             { v16.4s, v17.4s }, [x7], #32

        ext8             v18.16b, v24.16b, v24.16b, #8
        ext8             v19.16b, v25.16b, v25.16b, #8

        trn1            v20.4s, v16.4s, v16.4s      
        trn2            v16.4s, v16.4s, v16.4s      
        trn1            v21.4s, v17.4s, v17.4s      
        trn2            v17.4s, v17.4s, v17.4s      

        rev64           v22.4s, v8.4s                 
        rev64           v23.4s, v10.4s                 
        rev64           v24.4s, v12.4s                 
        rev64           v25.4s, v14.4s                 

        fmul            v22.4s, v22.4s, v20.4s      
        fmul            v23.4s, v23.4s, v16.4s      
        fmul            v24.4s, v24.4s, v20.4s      
        fmul            v25.4s, v25.4s, v16.4s      

        rev64           v26.4s, v9.4s                 
        rev64           v27.4s, v11.4s                 
        rev64           v20.4s, v13.4s                 
        rev64           v16.4s, v15.4s                 

        fmul            v26.4s, v26.4s, v21.4s      
        fmul            v27.4s, v27.4s, v17.4s      
        fmul            v20.4s, v20.4s, v21.4s      
        fmul            v16.4s, v16.4s, v17.4s      

        trn1            v21.4s, v19.4s, v19.4s      
        trn2            v19.4s, v19.4s, v19.4s      
        trn1            v17.4s, v18.4s, v18.4s      
        trn2            v18.4s, v18.4s, v18.4s      

        fmul            v21.4s, v21.4s, v31.4s
        fmul            v19.4s, v19.4s, v31.4s
        fmul            v17.4s, v17.4s, v31.4s
        fmul            v18.4s, v18.4s, v31.4s

        fmla            v23.4s, v10.4s, v21.4s      
        fmls            v25.4s, v14.4s, v21.4s      
        fmla            v22.4s, v8.4s, v19.4s      
        fmls            v24.4s, v12.4s, v19.4s      

        fmla            v26.4s, v9.4s, v18.4s      
        fmla            v27.4s, v11.4s, v17.4s      
        fmls            v20.4s, v13.4s, v18.4s      
        fmls            v16.4s, v15.4s, v17.4s      

        fsub            v18.4s, v23.4s, v25.4s
        fsub            v17.4s, v24.4s, v22.4s
        fsub            v19.4s, v20.4s, v26.4s
        fsub            v21.4s, v16.4s, v27.4s

        fadd            v22.4s, v24.4s, v22.4s
        fadd            v23.4s, v25.4s, v23.4s
        fadd            v24.4s, v20.4s, v26.4s
        fadd            v25.4s, v16.4s, v27.4s

        fmul            v17.4s, v17.4s, v31.4s
        fmul            v18.4s, v18.4s, v31.4s
        fmul            v19.4s, v19.4s, v31.4s
        fmul            v21.4s, v21.4s, v31.4s

        rev64           v22.4s, v22.4s
        rev64           v24.4s, v24.4s
        rev64           v23.4s, v23.4s
        rev64           v25.4s, v25.4s

        fsub            v8.4s, v0.4s, v22.4s
        fsub            v9.4s, v1.4s, v24.4s
        fsub            v10.4s, v2.4s, v17.4s
        fsub            v11.4s, v3.4s, v19.4s

        fsub            v12.4s, v4.4s, v23.4s
        fsub            v13.4s, v5.4s, v25.4s
        fadd            v14.4s, v6.4s, v18.4s
        fsub            v15.4s, v7.4s, v21.4s

        fadd            v0.4s, v0.4s, v22.4s
        fadd            v1.4s, v1.4s, v24.4s
        fadd            v2.4s, v2.4s, v17.4s
        fadd            v3.4s, v3.4s, v19.4s

        fadd            v4.4s, v4.4s, v23.4s
        fsub            v6.4s, v6.4s, v18.4s      
        fadd            v5.4s, v5.4s, v25.4s      
        fadd            v7.4s, v7.4s, v21.4s

        stp             q0,  q1,  [x1,  #((0 + 1)*32 + 0)]
        stp             q4,  q5,  [x1,  #((2 + 1)*32 + 0)]
        stp             q2,  q3,  [x10, #((0 + 1)*32 + 0)]
        stp             q6,  q7,  [x10, #((2 + 1)*32 + 0)]

        stp             q8,  q9,  [x11, #((0 + 1)*32 + 0)]
        stp             q12, q13, [x11, #((2 + 1)*32 + 0)]
        stp             q10, q11, [x12, #((0 + 1)*32 + 0)]
        stp             q14, q15, [x12, #((2 + 1)*32 + 0)]
        add             x10, x1, x21
        add             x11, x1, x21, lsl #1
        add             x12, x1, x22

        ldp             q0,  q1,  [x1,  #((0 + 4)*32 + 0)]
        ldp             q4,  q5,  [x1,  #((2 + 4)*32 + 0)]
        ldp             q2,  q3,  [x10, #((0 + 4)*32 + 0)]
        ldp             q6,  q7,  [x10, #((2 + 4)*32 + 0)]

        ldp             q8,  q9,  [x11, #((0 + 4)*32 + 0)]
        ldp             q10, q11, [x11, #((2 + 4)*32 + 0)]
        ldp             q12, q13, [x12, #((0 + 4)*32 + 0)]
        ldp             q14, q15, [x12, #((2 + 4)*32 + 0)]


        ld1             { v24.4s, v25.4s }, [x8], x9
        ld1             { v16.4s, v17.4s }, [x7], #32

        ext8             v18.16b, v24.16b, v24.16b, #8
        ext8             v19.16b, v25.16b, v25.16b, #8

        trn1            v20.4s, v16.4s, v16.4s      
        trn2            v16.4s, v16.4s, v16.4s      
        trn1            v21.4s, v17.4s, v17.4s      
        trn2            v17.4s, v17.4s, v17.4s      

        rev64           v22.4s, v8.4s                 
        rev64           v23.4s, v10.4s                 
        rev64           v24.4s, v12.4s                 
        rev64           v25.4s, v14.4s                 

        fmul            v22.4s, v22.4s, v20.4s      
        fmul            v23.4s, v23.4s, v16.4s      
        fmul            v24.4s, v24.4s, v20.4s      
        fmul            v25.4s, v25.4s, v16.4s      

        rev64           v26.4s, v9.4s                 
        rev64           v27.4s, v11.4s                 
        rev64           v20.4s, v13.4s                 
        rev64           v16.4s, v15.4s                 

        fmul            v26.4s, v26.4s, v21.4s      
        fmul            v27.4s, v27.4s, v17.4s      
        fmul            v20.4s, v20.4s, v21.4s      
        fmul            v16.4s, v16.4s, v17.4s      

        trn1            v21.4s, v19.4s, v19.4s      
        trn2            v19.4s, v19.4s, v19.4s      
        trn1            v17.4s, v18.4s, v18.4s      
        trn2            v18.4s, v18.4s, v18.4s      

        fmul            v21.4s, v21.4s, v31.4s
        fmul            v19.4s, v19.4s, v31.4s
        fmul            v17.4s, v17.4s, v31.4s
        fmul            v18.4s, v18.4s, v31.4s

        fmla            v23.4s, v10.4s, v21.4s      
        fmls            v25.4s, v14.4s, v21.4s      
        fmla            v22.4s, v8.4s, v19.4s      
        fmls            v24.4s, v12.4s, v19.4s      

        fmla            v26.4s, v9.4s, v18.4s      
        fmla            v27.4s, v11.4s, v17.4s      
        fmls            v20.4s, v13.4s, v18.4s      
        fmls            v16.4s, v15.4s, v17.4s      

        fsub            v18.4s, v23.4s, v25.4s
        fsub            v17.4s, v24.4s, v22.4s
        fsub            v19.4s, v20.4s, v26.4s
        fsub            v21.4s, v16.4s, v27.4s

        fadd            v22.4s, v24.4s, v22.4s
        fadd            v23.4s, v25.4s, v23.4s
        fadd            v24.4s, v20.4s, v26.4s
        fadd            v25.4s, v16.4s, v27.4s

        fmul            v17.4s, v17.4s, v31.4s
        fmul            v18.4s, v18.4s, v31.4s
        fmul            v19.4s, v19.4s, v31.4s
        fmul            v21.4s, v21.4s, v31.4s

        rev64           v22.4s, v22.4s
        rev64           v24.4s, v24.4s
        rev64           v23.4s, v23.4s
        rev64           v25.4s, v25.4s

        fsub            v8.4s, v0.4s, v22.4s
        fsub            v9.4s, v1.4s, v24.4s
        fsub            v10.4s, v2.4s, v17.4s
        fsub            v11.4s, v3.4s, v19.4s

        fsub            v12.4s, v4.4s, v23.4s
        fsub            v13.4s, v5.4s, v25.4s
        fadd            v14.4s, v6.4s, v18.4s
        fsub            v15.4s, v7.4s, v21.4s

        fadd            v0.4s, v0.4s, v22.4s
        fadd            v1.4s, v1.4s, v24.4s
        fadd            v2.4s, v2.4s, v17.4s
        fadd            v3.4s, v3.4s, v19.4s

        fadd            v4.4s, v4.4s, v23.4s
        fsub            v6.4s, v6.4s, v18.4s      
        fadd            v5.4s, v5.4s, v25.4s      
        fadd            v7.4s, v7.4s, v21.4s

        stp             q0,  q1,  [x1,  #((0 + 4)*32 + 0)]
        stp             q4,  q5,  [x1,  #((2 + 4)*32 + 0)]
        stp             q2,  q3,  [x10, #((0 + 4)*32 + 0)]
        stp             q6,  q7,  [x10, #((2 + 4)*32 + 0)]

        stp             q8,  q9,  [x11, #((0 + 4)*32 + 0)]
        stp             q12, q13, [x11, #((2 + 4)*32 + 0)]
        stp             q10, q11, [x12, #((0 + 4)*32 + 0)]
        stp             q14, q15, [x12, #((2 + 4)*32 + 0)]
        add             x10, x1, x21
        add             x11, x1, x21, lsl #1
        add             x12, x1, x22

        ldp             q0,  q1,  [x1,  #((0 + 5)*32 + 0)]
        ldp             q4,  q5,  [x1,  #((2 + 5)*32 + 0)]
        ldp             q2,  q3,  [x10, #((0 + 5)*32 + 0)]
        ldp             q6,  q7,  [x10, #((2 + 5)*32 + 0)]

        ldp             q8,  q9,  [x11, #((0 + 5)*32 + 0)]
        ldp             q10, q11, [x11, #((2 + 5)*32 + 0)]
        ldp             q12, q13, [x12, #((0 + 5)*32 + 0)]
        ldp             q14, q15, [x12, #((2 + 5)*32 + 0)]


        ld1             { v24.4s, v25.4s }, [x8], x9
        ld1             { v16.4s, v17.4s }, [x7], #32

        ext8             v18.16b, v24.16b, v24.16b, #8
        ext8             v19.16b, v25.16b, v25.16b, #8

        trn1            v20.4s, v16.4s, v16.4s      
        trn2            v16.4s, v16.4s, v16.4s      
        trn1            v21.4s, v17.4s, v17.4s      
        trn2            v17.4s, v17.4s, v17.4s      

        rev64           v22.4s, v8.4s                 
        rev64           v23.4s, v10.4s                 
        rev64           v24.4s, v12.4s                 
        rev64           v25.4s, v14.4s                 

        fmul            v22.4s, v22.4s, v20.4s      
        fmul            v23.4s, v23.4s, v16.4s      
        fmul            v24.4s, v24.4s, v20.4s      
        fmul            v25.4s, v25.4s, v16.4s      

        rev64           v26.4s, v9.4s                 
        rev64           v27.4s, v11.4s                 
        rev64           v20.4s, v13.4s                 
        rev64           v16.4s, v15.4s                 

        fmul            v26.4s, v26.4s, v21.4s      
        fmul            v27.4s, v27.4s, v17.4s      
        fmul            v20.4s, v20.4s, v21.4s      
        fmul            v16.4s, v16.4s, v17.4s      

        trn1            v21.4s, v19.4s, v19.4s      
        trn2            v19.4s, v19.4s, v19.4s      
        trn1            v17.4s, v18.4s, v18.4s      
        trn2            v18.4s, v18.4s, v18.4s      

        fmul            v21.4s, v21.4s, v31.4s
        fmul            v19.4s, v19.4s, v31.4s
        fmul            v17.4s, v17.4s, v31.4s
        fmul            v18.4s, v18.4s, v31.4s

        fmla            v23.4s, v10.4s, v21.4s      
        fmls            v25.4s, v14.4s, v21.4s      
        fmla            v22.4s, v8.4s, v19.4s      
        fmls            v24.4s, v12.4s, v19.4s      

        fmla            v26.4s, v9.4s, v18.4s      
        fmla            v27.4s, v11.4s, v17.4s      
        fmls            v20.4s, v13.4s, v18.4s      
        fmls            v16.4s, v15.4s, v17.4s      

        fsub            v18.4s, v23.4s, v25.4s
        fsub            v17.4s, v24.4s, v22.4s
        fsub            v19.4s, v20.4s, v26.4s
        fsub            v21.4s, v16.4s, v27.4s

        fadd            v22.4s, v24.4s, v22.4s
        fadd            v23.4s, v25.4s, v23.4s
        fadd            v24.4s, v20.4s, v26.4s
        fadd            v25.4s, v16.4s, v27.4s

        fmul            v17.4s, v17.4s, v31.4s
        fmul            v18.4s, v18.4s, v31.4s
        fmul            v19.4s, v19.4s, v31.4s
        fmul            v21.4s, v21.4s, v31.4s

        rev64           v22.4s, v22.4s
        rev64           v24.4s, v24.4s
        rev64           v23.4s, v23.4s
        rev64           v25.4s, v25.4s

        fsub            v8.4s, v0.4s, v22.4s
        fsub            v9.4s, v1.4s, v24.4s
        fsub            v10.4s, v2.4s, v17.4s
        fsub            v11.4s, v3.4s, v19.4s

        fsub            v12.4s, v4.4s, v23.4s
        fsub            v13.4s, v5.4s, v25.4s
        fadd            v14.4s, v6.4s, v18.4s
        fsub            v15.4s, v7.4s, v21.4s

        fadd            v0.4s, v0.4s, v22.4s
        fadd            v1.4s, v1.4s, v24.4s
        fadd            v2.4s, v2.4s, v17.4s
        fadd            v3.4s, v3.4s, v19.4s

        fadd            v4.4s, v4.4s, v23.4s
        fsub            v6.4s, v6.4s, v18.4s      
        fadd            v5.4s, v5.4s, v25.4s      
        fadd            v7.4s, v7.4s, v21.4s

        stp             q0,  q1,  [x1,  #((0 + 5)*32 + 0)]
        stp             q4,  q5,  [x1,  #((2 + 5)*32 + 0)]
        stp             q2,  q3,  [x10, #((0 + 5)*32 + 0)]
        stp             q6,  q7,  [x10, #((2 + 5)*32 + 0)]

        stp             q8,  q9,  [x11, #((0 + 5)*32 + 0)]
        stp             q12, q13, [x11, #((2 + 5)*32 + 0)]
        stp             q10, q11, [x12, #((0 + 5)*32 + 0)]
        stp             q14, q15, [x12, #((2 + 5)*32 + 0)]
        add             x1, x1, 8*32
        subs            w5, w5, 1
        bgt            temp_label_29

        cmp             w20, #((8192)/4096), lsl #12
        bgt            temp_label_30
        ret
temp_label_31
temp_label_30

        stp             x20, x30, [sp, #-16]!
        mov             w20, #(16384/4)
        mov             x5, #((16384*4) - (16384/1))
        add             x1, x1, x5
        bl              temp_label_0
        mov             x5, #((16384*2) - (16384/2))
        add             x1, x1, x5
        bl              temp_label_0
        ldp             x20, x30, [sp], #16
        ldr             w5, =(16384*6 + 16384/2)
        sub             x1, x1, x5

        ldr             w5, =(16384 - 4*7)










        adrp            x7, ff_tx_tab_16384_float
        add             x7, x7, ff_tx_tab_16384_float










        add             x8, x7, x5
        mov             x9, #-32

        mov             x21, #2*16384
        add             x22, x21, x21, lsl #1

        cmp             w19, #((16384)/4096), lsl #12
        beq            temp_label_32

        mov             w5, #(16384/128)
temp_label_33

        add             x10, x1, x21
        add             x11, x1, x21, lsl #1
        add             x12, x1, x22

        add             x10, x1, x21
        add             x11, x1, x21, lsl #1
        add             x12, x1, x22

        ldp             q0,  q1,  [x1,  #((0 + 0)*32 + 0)]
        ldp             q4,  q5,  [x1,  #((2 + 0)*32 + 0)]
        ldp             q2,  q3,  [x10, #((0 + 0)*32 + 0)]
        ldp             q6,  q7,  [x10, #((2 + 0)*32 + 0)]

        ldp             q8,  q9,  [x11, #((0 + 0)*32 + 0)]
        ldp             q10, q11, [x11, #((2 + 0)*32 + 0)]
        ldp             q12, q13, [x12, #((0 + 0)*32 + 0)]
        ldp             q14, q15, [x12, #((2 + 0)*32 + 0)]


        ld1             { v24.4s, v25.4s }, [x8], x9
        ld1             { v16.4s, v17.4s }, [x7], #32

        ext8             v18.16b, v24.16b, v24.16b, #8
        ext8             v19.16b, v25.16b, v25.16b, #8

        trn1            v20.4s, v16.4s, v16.4s      
        trn2            v16.4s, v16.4s, v16.4s      
        trn1            v21.4s, v17.4s, v17.4s      
        trn2            v17.4s, v17.4s, v17.4s      

        rev64           v22.4s, v8.4s                 
        rev64           v23.4s, v10.4s                 
        rev64           v24.4s, v12.4s                 
        rev64           v25.4s, v14.4s                 

        fmul            v22.4s, v22.4s, v20.4s      
        fmul            v23.4s, v23.4s, v16.4s      
        fmul            v24.4s, v24.4s, v20.4s      
        fmul            v25.4s, v25.4s, v16.4s      

        rev64           v26.4s, v9.4s                 
        rev64           v27.4s, v11.4s                 
        rev64           v20.4s, v13.4s                 
        rev64           v16.4s, v15.4s                 

        fmul            v26.4s, v26.4s, v21.4s      
        fmul            v27.4s, v27.4s, v17.4s      
        fmul            v20.4s, v20.4s, v21.4s      
        fmul            v16.4s, v16.4s, v17.4s      

        trn1            v21.4s, v19.4s, v19.4s      
        trn2            v19.4s, v19.4s, v19.4s      
        trn1            v17.4s, v18.4s, v18.4s      
        trn2            v18.4s, v18.4s, v18.4s      

        fmul            v21.4s, v21.4s, v31.4s
        fmul            v19.4s, v19.4s, v31.4s
        fmul            v17.4s, v17.4s, v31.4s
        fmul            v18.4s, v18.4s, v31.4s

        fmla            v23.4s, v10.4s, v21.4s      
        fmls            v25.4s, v14.4s, v21.4s      
        fmla            v22.4s, v8.4s, v19.4s      
        fmls            v24.4s, v12.4s, v19.4s      

        fmla            v26.4s, v9.4s, v18.4s      
        fmla            v27.4s, v11.4s, v17.4s      
        fmls            v20.4s, v13.4s, v18.4s      
        fmls            v16.4s, v15.4s, v17.4s      

        fsub            v18.4s, v23.4s, v25.4s
        fsub            v17.4s, v24.4s, v22.4s
        fsub            v19.4s, v20.4s, v26.4s
        fsub            v21.4s, v16.4s, v27.4s

        fadd            v22.4s, v24.4s, v22.4s
        fadd            v23.4s, v25.4s, v23.4s
        fadd            v24.4s, v20.4s, v26.4s
        fadd            v25.4s, v16.4s, v27.4s

        fmul            v17.4s, v17.4s, v31.4s
        fmul            v18.4s, v18.4s, v31.4s
        fmul            v19.4s, v19.4s, v31.4s
        fmul            v21.4s, v21.4s, v31.4s

        rev64           v22.4s, v22.4s
        rev64           v24.4s, v24.4s
        rev64           v23.4s, v23.4s
        rev64           v25.4s, v25.4s

        fsub            v8.4s, v0.4s, v22.4s
        fsub            v9.4s, v1.4s, v24.4s
        fsub            v10.4s, v2.4s, v17.4s
        fsub            v11.4s, v3.4s, v19.4s

        fsub            v12.4s, v4.4s, v23.4s
        fsub            v13.4s, v5.4s, v25.4s
        fadd            v14.4s, v6.4s, v18.4s
        fsub            v15.4s, v7.4s, v21.4s

        fadd            v0.4s, v0.4s, v22.4s
        fadd            v1.4s, v1.4s, v24.4s
        fadd            v2.4s, v2.4s, v17.4s
        fadd            v3.4s, v3.4s, v19.4s

        fadd            v4.4s, v4.4s, v23.4s
        fsub            v6.4s, v6.4s, v18.4s      
        fadd            v5.4s, v5.4s, v25.4s      
        fadd            v7.4s, v7.4s, v21.4s

        stp             q0,  q1,  [x1,  #((0 + 0)*32 + 0)]
        stp             q4,  q5,  [x1,  #((2 + 0)*32 + 0)]
        stp             q2,  q3,  [x10, #((0 + 0)*32 + 0)]
        stp             q6,  q7,  [x10, #((2 + 0)*32 + 0)]

        stp             q8,  q9,  [x11, #((0 + 0)*32 + 0)]
        stp             q12, q13, [x11, #((2 + 0)*32 + 0)]
        stp             q10, q11, [x12, #((0 + 0)*32 + 0)]
        stp             q14, q15, [x12, #((2 + 0)*32 + 0)]
        add             x10, x1, x21
        add             x11, x1, x21, lsl #1
        add             x12, x1, x22

        ldp             q0,  q1,  [x1,  #((0 + 1)*32 + 0)]
        ldp             q4,  q5,  [x1,  #((2 + 1)*32 + 0)]
        ldp             q2,  q3,  [x10, #((0 + 1)*32 + 0)]
        ldp             q6,  q7,  [x10, #((2 + 1)*32 + 0)]

        ldp             q8,  q9,  [x11, #((0 + 1)*32 + 0)]
        ldp             q10, q11, [x11, #((2 + 1)*32 + 0)]
        ldp             q12, q13, [x12, #((0 + 1)*32 + 0)]
        ldp             q14, q15, [x12, #((2 + 1)*32 + 0)]


        ld1             { v24.4s, v25.4s }, [x8], x9
        ld1             { v16.4s, v17.4s }, [x7], #32

        ext8             v18.16b, v24.16b, v24.16b, #8
        ext8             v19.16b, v25.16b, v25.16b, #8

        trn1            v20.4s, v16.4s, v16.4s      
        trn2            v16.4s, v16.4s, v16.4s      
        trn1            v21.4s, v17.4s, v17.4s      
        trn2            v17.4s, v17.4s, v17.4s      

        rev64           v22.4s, v8.4s                 
        rev64           v23.4s, v10.4s                 
        rev64           v24.4s, v12.4s                 
        rev64           v25.4s, v14.4s                 

        fmul            v22.4s, v22.4s, v20.4s      
        fmul            v23.4s, v23.4s, v16.4s      
        fmul            v24.4s, v24.4s, v20.4s      
        fmul            v25.4s, v25.4s, v16.4s      

        rev64           v26.4s, v9.4s                 
        rev64           v27.4s, v11.4s                 
        rev64           v20.4s, v13.4s                 
        rev64           v16.4s, v15.4s                 

        fmul            v26.4s, v26.4s, v21.4s      
        fmul            v27.4s, v27.4s, v17.4s      
        fmul            v20.4s, v20.4s, v21.4s      
        fmul            v16.4s, v16.4s, v17.4s      

        trn1            v21.4s, v19.4s, v19.4s      
        trn2            v19.4s, v19.4s, v19.4s      
        trn1            v17.4s, v18.4s, v18.4s      
        trn2            v18.4s, v18.4s, v18.4s      

        fmul            v21.4s, v21.4s, v31.4s
        fmul            v19.4s, v19.4s, v31.4s
        fmul            v17.4s, v17.4s, v31.4s
        fmul            v18.4s, v18.4s, v31.4s

        fmla            v23.4s, v10.4s, v21.4s      
        fmls            v25.4s, v14.4s, v21.4s      
        fmla            v22.4s, v8.4s, v19.4s      
        fmls            v24.4s, v12.4s, v19.4s      

        fmla            v26.4s, v9.4s, v18.4s      
        fmla            v27.4s, v11.4s, v17.4s      
        fmls            v20.4s, v13.4s, v18.4s      
        fmls            v16.4s, v15.4s, v17.4s      

        fsub            v18.4s, v23.4s, v25.4s
        fsub            v17.4s, v24.4s, v22.4s
        fsub            v19.4s, v20.4s, v26.4s
        fsub            v21.4s, v16.4s, v27.4s

        fadd            v22.4s, v24.4s, v22.4s
        fadd            v23.4s, v25.4s, v23.4s
        fadd            v24.4s, v20.4s, v26.4s
        fadd            v25.4s, v16.4s, v27.4s

        fmul            v17.4s, v17.4s, v31.4s
        fmul            v18.4s, v18.4s, v31.4s
        fmul            v19.4s, v19.4s, v31.4s
        fmul            v21.4s, v21.4s, v31.4s

        rev64           v22.4s, v22.4s
        rev64           v24.4s, v24.4s
        rev64           v23.4s, v23.4s
        rev64           v25.4s, v25.4s

        fsub            v8.4s, v0.4s, v22.4s
        fsub            v9.4s, v1.4s, v24.4s
        fsub            v10.4s, v2.4s, v17.4s
        fsub            v11.4s, v3.4s, v19.4s

        fsub            v12.4s, v4.4s, v23.4s
        fsub            v13.4s, v5.4s, v25.4s
        fadd            v14.4s, v6.4s, v18.4s
        fsub            v15.4s, v7.4s, v21.4s

        fadd            v0.4s, v0.4s, v22.4s
        fadd            v1.4s, v1.4s, v24.4s
        fadd            v2.4s, v2.4s, v17.4s
        fadd            v3.4s, v3.4s, v19.4s

        fadd            v4.4s, v4.4s, v23.4s
        fsub            v6.4s, v6.4s, v18.4s      
        fadd            v5.4s, v5.4s, v25.4s      
        fadd            v7.4s, v7.4s, v21.4s

        stp             q0,  q1,  [x1,  #((0 + 1)*32 + 0)]
        stp             q4,  q5,  [x1,  #((2 + 1)*32 + 0)]
        stp             q2,  q3,  [x10, #((0 + 1)*32 + 0)]
        stp             q6,  q7,  [x10, #((2 + 1)*32 + 0)]

        stp             q8,  q9,  [x11, #((0 + 1)*32 + 0)]
        stp             q12, q13, [x11, #((2 + 1)*32 + 0)]
        stp             q10, q11, [x12, #((0 + 1)*32 + 0)]
        stp             q14, q15, [x12, #((2 + 1)*32 + 0)]
        add             x10, x1, x21
        add             x11, x1, x21, lsl #1
        add             x12, x1, x22

        ldp             q0,  q1,  [x1,  #((0 + 4)*32 + 0)]
        ldp             q4,  q5,  [x1,  #((2 + 4)*32 + 0)]
        ldp             q2,  q3,  [x10, #((0 + 4)*32 + 0)]
        ldp             q6,  q7,  [x10, #((2 + 4)*32 + 0)]

        ldp             q8,  q9,  [x11, #((0 + 4)*32 + 0)]
        ldp             q10, q11, [x11, #((2 + 4)*32 + 0)]
        ldp             q12, q13, [x12, #((0 + 4)*32 + 0)]
        ldp             q14, q15, [x12, #((2 + 4)*32 + 0)]


        ld1             { v24.4s, v25.4s }, [x8], x9
        ld1             { v16.4s, v17.4s }, [x7], #32

        ext8             v18.16b, v24.16b, v24.16b, #8
        ext8             v19.16b, v25.16b, v25.16b, #8

        trn1            v20.4s, v16.4s, v16.4s      
        trn2            v16.4s, v16.4s, v16.4s      
        trn1            v21.4s, v17.4s, v17.4s      
        trn2            v17.4s, v17.4s, v17.4s      

        rev64           v22.4s, v8.4s                 
        rev64           v23.4s, v10.4s                 
        rev64           v24.4s, v12.4s                 
        rev64           v25.4s, v14.4s                 

        fmul            v22.4s, v22.4s, v20.4s      
        fmul            v23.4s, v23.4s, v16.4s      
        fmul            v24.4s, v24.4s, v20.4s      
        fmul            v25.4s, v25.4s, v16.4s      

        rev64           v26.4s, v9.4s                 
        rev64           v27.4s, v11.4s                 
        rev64           v20.4s, v13.4s                 
        rev64           v16.4s, v15.4s                 

        fmul            v26.4s, v26.4s, v21.4s      
        fmul            v27.4s, v27.4s, v17.4s      
        fmul            v20.4s, v20.4s, v21.4s      
        fmul            v16.4s, v16.4s, v17.4s      

        trn1            v21.4s, v19.4s, v19.4s      
        trn2            v19.4s, v19.4s, v19.4s      
        trn1            v17.4s, v18.4s, v18.4s      
        trn2            v18.4s, v18.4s, v18.4s      

        fmul            v21.4s, v21.4s, v31.4s
        fmul            v19.4s, v19.4s, v31.4s
        fmul            v17.4s, v17.4s, v31.4s
        fmul            v18.4s, v18.4s, v31.4s

        fmla            v23.4s, v10.4s, v21.4s      
        fmls            v25.4s, v14.4s, v21.4s      
        fmla            v22.4s, v8.4s, v19.4s      
        fmls            v24.4s, v12.4s, v19.4s      

        fmla            v26.4s, v9.4s, v18.4s      
        fmla            v27.4s, v11.4s, v17.4s      
        fmls            v20.4s, v13.4s, v18.4s      
        fmls            v16.4s, v15.4s, v17.4s      

        fsub            v18.4s, v23.4s, v25.4s
        fsub            v17.4s, v24.4s, v22.4s
        fsub            v19.4s, v20.4s, v26.4s
        fsub            v21.4s, v16.4s, v27.4s

        fadd            v22.4s, v24.4s, v22.4s
        fadd            v23.4s, v25.4s, v23.4s
        fadd            v24.4s, v20.4s, v26.4s
        fadd            v25.4s, v16.4s, v27.4s

        fmul            v17.4s, v17.4s, v31.4s
        fmul            v18.4s, v18.4s, v31.4s
        fmul            v19.4s, v19.4s, v31.4s
        fmul            v21.4s, v21.4s, v31.4s

        rev64           v22.4s, v22.4s
        rev64           v24.4s, v24.4s
        rev64           v23.4s, v23.4s
        rev64           v25.4s, v25.4s

        fsub            v8.4s, v0.4s, v22.4s
        fsub            v9.4s, v1.4s, v24.4s
        fsub            v10.4s, v2.4s, v17.4s
        fsub            v11.4s, v3.4s, v19.4s

        fsub            v12.4s, v4.4s, v23.4s
        fsub            v13.4s, v5.4s, v25.4s
        fadd            v14.4s, v6.4s, v18.4s
        fsub            v15.4s, v7.4s, v21.4s

        fadd            v0.4s, v0.4s, v22.4s
        fadd            v1.4s, v1.4s, v24.4s
        fadd            v2.4s, v2.4s, v17.4s
        fadd            v3.4s, v3.4s, v19.4s

        fadd            v4.4s, v4.4s, v23.4s
        fsub            v6.4s, v6.4s, v18.4s      
        fadd            v5.4s, v5.4s, v25.4s      
        fadd            v7.4s, v7.4s, v21.4s

        stp             q0,  q1,  [x1,  #((0 + 4)*32 + 0)]
        stp             q4,  q5,  [x1,  #((2 + 4)*32 + 0)]
        stp             q2,  q3,  [x10, #((0 + 4)*32 + 0)]
        stp             q6,  q7,  [x10, #((2 + 4)*32 + 0)]

        stp             q8,  q9,  [x11, #((0 + 4)*32 + 0)]
        stp             q12, q13, [x11, #((2 + 4)*32 + 0)]
        stp             q10, q11, [x12, #((0 + 4)*32 + 0)]
        stp             q14, q15, [x12, #((2 + 4)*32 + 0)]
        add             x10, x1, x21
        add             x11, x1, x21, lsl #1
        add             x12, x1, x22

        ldp             q0,  q1,  [x1,  #((0 + 5)*32 + 0)]
        ldp             q4,  q5,  [x1,  #((2 + 5)*32 + 0)]
        ldp             q2,  q3,  [x10, #((0 + 5)*32 + 0)]
        ldp             q6,  q7,  [x10, #((2 + 5)*32 + 0)]

        ldp             q8,  q9,  [x11, #((0 + 5)*32 + 0)]
        ldp             q10, q11, [x11, #((2 + 5)*32 + 0)]
        ldp             q12, q13, [x12, #((0 + 5)*32 + 0)]
        ldp             q14, q15, [x12, #((2 + 5)*32 + 0)]


        ld1             { v24.4s, v25.4s }, [x8], x9
        ld1             { v16.4s, v17.4s }, [x7], #32

        ext8             v18.16b, v24.16b, v24.16b, #8
        ext8             v19.16b, v25.16b, v25.16b, #8

        trn1            v20.4s, v16.4s, v16.4s      
        trn2            v16.4s, v16.4s, v16.4s      
        trn1            v21.4s, v17.4s, v17.4s      
        trn2            v17.4s, v17.4s, v17.4s      

        rev64           v22.4s, v8.4s                 
        rev64           v23.4s, v10.4s                 
        rev64           v24.4s, v12.4s                 
        rev64           v25.4s, v14.4s                 

        fmul            v22.4s, v22.4s, v20.4s      
        fmul            v23.4s, v23.4s, v16.4s      
        fmul            v24.4s, v24.4s, v20.4s      
        fmul            v25.4s, v25.4s, v16.4s      

        rev64           v26.4s, v9.4s                 
        rev64           v27.4s, v11.4s                 
        rev64           v20.4s, v13.4s                 
        rev64           v16.4s, v15.4s                 

        fmul            v26.4s, v26.4s, v21.4s      
        fmul            v27.4s, v27.4s, v17.4s      
        fmul            v20.4s, v20.4s, v21.4s      
        fmul            v16.4s, v16.4s, v17.4s      

        trn1            v21.4s, v19.4s, v19.4s      
        trn2            v19.4s, v19.4s, v19.4s      
        trn1            v17.4s, v18.4s, v18.4s      
        trn2            v18.4s, v18.4s, v18.4s      

        fmul            v21.4s, v21.4s, v31.4s
        fmul            v19.4s, v19.4s, v31.4s
        fmul            v17.4s, v17.4s, v31.4s
        fmul            v18.4s, v18.4s, v31.4s

        fmla            v23.4s, v10.4s, v21.4s      
        fmls            v25.4s, v14.4s, v21.4s      
        fmla            v22.4s, v8.4s, v19.4s      
        fmls            v24.4s, v12.4s, v19.4s      

        fmla            v26.4s, v9.4s, v18.4s      
        fmla            v27.4s, v11.4s, v17.4s      
        fmls            v20.4s, v13.4s, v18.4s      
        fmls            v16.4s, v15.4s, v17.4s      

        fsub            v18.4s, v23.4s, v25.4s
        fsub            v17.4s, v24.4s, v22.4s
        fsub            v19.4s, v20.4s, v26.4s
        fsub            v21.4s, v16.4s, v27.4s

        fadd            v22.4s, v24.4s, v22.4s
        fadd            v23.4s, v25.4s, v23.4s
        fadd            v24.4s, v20.4s, v26.4s
        fadd            v25.4s, v16.4s, v27.4s

        fmul            v17.4s, v17.4s, v31.4s
        fmul            v18.4s, v18.4s, v31.4s
        fmul            v19.4s, v19.4s, v31.4s
        fmul            v21.4s, v21.4s, v31.4s

        rev64           v22.4s, v22.4s
        rev64           v24.4s, v24.4s
        rev64           v23.4s, v23.4s
        rev64           v25.4s, v25.4s

        fsub            v8.4s, v0.4s, v22.4s
        fsub            v9.4s, v1.4s, v24.4s
        fsub            v10.4s, v2.4s, v17.4s
        fsub            v11.4s, v3.4s, v19.4s

        fsub            v12.4s, v4.4s, v23.4s
        fsub            v13.4s, v5.4s, v25.4s
        fadd            v14.4s, v6.4s, v18.4s
        fsub            v15.4s, v7.4s, v21.4s

        fadd            v0.4s, v0.4s, v22.4s
        fadd            v1.4s, v1.4s, v24.4s
        fadd            v2.4s, v2.4s, v17.4s
        fadd            v3.4s, v3.4s, v19.4s

        fadd            v4.4s, v4.4s, v23.4s
        fsub            v6.4s, v6.4s, v18.4s      
        fadd            v5.4s, v5.4s, v25.4s      
        fadd            v7.4s, v7.4s, v21.4s

        stp             q0,  q1,  [x1,  #((0 + 5)*32 + 0)]
        stp             q4,  q5,  [x1,  #((2 + 5)*32 + 0)]
        stp             q2,  q3,  [x10, #((0 + 5)*32 + 0)]
        stp             q6,  q7,  [x10, #((2 + 5)*32 + 0)]

        stp             q8,  q9,  [x11, #((0 + 5)*32 + 0)]
        stp             q12, q13, [x11, #((2 + 5)*32 + 0)]
        stp             q10, q11, [x12, #((0 + 5)*32 + 0)]
        stp             q14, q15, [x12, #((2 + 5)*32 + 0)]
        add             x1, x1, 8*32
        subs            w5, w5, 1
        bgt            temp_label_33

        cmp             w20, #((16384)/4096), lsl #12
        bgt            temp_label_34
        ret
temp_label_35
temp_label_34

        stp             x20, x30, [sp, #-16]!
        mov             w20, #(32768/4)
        mov             x5, #((32768*4) - (32768/1))
        add             x1, x1, x5
        bl              temp_label_0
        mov             x5, #((32768*2) - (32768/2))
        add             x1, x1, x5
        bl              temp_label_0
        ldp             x20, x30, [sp], #16
        ldr             w5, =(32768*6 + 32768/2)
        sub             x1, x1, x5

        ldr             w5, =(32768 - 4*7)










        adrp            x7, ff_tx_tab_32768_float
        add             x7, x7, ff_tx_tab_32768_float










        add             x8, x7, x5
        mov             x9, #-32

        mov             x21, #2*32768
        add             x22, x21, x21, lsl #1

        cmp             w19, #((32768)/4096), lsl #12
        beq            temp_label_36

        mov             w5, #(32768/128)
temp_label_37

        add             x10, x1, x21
        add             x11, x1, x21, lsl #1
        add             x12, x1, x22

        add             x10, x1, x21
        add             x11, x1, x21, lsl #1
        add             x12, x1, x22

        ldp             q0,  q1,  [x1,  #((0 + 0)*32 + 0)]
        ldp             q4,  q5,  [x1,  #((2 + 0)*32 + 0)]
        ldp             q2,  q3,  [x10, #((0 + 0)*32 + 0)]
        ldp             q6,  q7,  [x10, #((2 + 0)*32 + 0)]

        ldp             q8,  q9,  [x11, #((0 + 0)*32 + 0)]
        ldp             q10, q11, [x11, #((2 + 0)*32 + 0)]
        ldp             q12, q13, [x12, #((0 + 0)*32 + 0)]
        ldp             q14, q15, [x12, #((2 + 0)*32 + 0)]


        ld1             { v24.4s, v25.4s }, [x8], x9
        ld1             { v16.4s, v17.4s }, [x7], #32

        ext8             v18.16b, v24.16b, v24.16b, #8
        ext8             v19.16b, v25.16b, v25.16b, #8

        trn1            v20.4s, v16.4s, v16.4s      
        trn2            v16.4s, v16.4s, v16.4s      
        trn1            v21.4s, v17.4s, v17.4s      
        trn2            v17.4s, v17.4s, v17.4s      

        rev64           v22.4s, v8.4s                 
        rev64           v23.4s, v10.4s                 
        rev64           v24.4s, v12.4s                 
        rev64           v25.4s, v14.4s                 

        fmul            v22.4s, v22.4s, v20.4s      
        fmul            v23.4s, v23.4s, v16.4s      
        fmul            v24.4s, v24.4s, v20.4s      
        fmul            v25.4s, v25.4s, v16.4s      

        rev64           v26.4s, v9.4s                 
        rev64           v27.4s, v11.4s                 
        rev64           v20.4s, v13.4s                 
        rev64           v16.4s, v15.4s                 

        fmul            v26.4s, v26.4s, v21.4s      
        fmul            v27.4s, v27.4s, v17.4s      
        fmul            v20.4s, v20.4s, v21.4s      
        fmul            v16.4s, v16.4s, v17.4s      

        trn1            v21.4s, v19.4s, v19.4s      
        trn2            v19.4s, v19.4s, v19.4s      
        trn1            v17.4s, v18.4s, v18.4s      
        trn2            v18.4s, v18.4s, v18.4s      

        fmul            v21.4s, v21.4s, v31.4s
        fmul            v19.4s, v19.4s, v31.4s
        fmul            v17.4s, v17.4s, v31.4s
        fmul            v18.4s, v18.4s, v31.4s

        fmla            v23.4s, v10.4s, v21.4s      
        fmls            v25.4s, v14.4s, v21.4s      
        fmla            v22.4s, v8.4s, v19.4s      
        fmls            v24.4s, v12.4s, v19.4s      

        fmla            v26.4s, v9.4s, v18.4s      
        fmla            v27.4s, v11.4s, v17.4s      
        fmls            v20.4s, v13.4s, v18.4s      
        fmls            v16.4s, v15.4s, v17.4s      

        fsub            v18.4s, v23.4s, v25.4s
        fsub            v17.4s, v24.4s, v22.4s
        fsub            v19.4s, v20.4s, v26.4s
        fsub            v21.4s, v16.4s, v27.4s

        fadd            v22.4s, v24.4s, v22.4s
        fadd            v23.4s, v25.4s, v23.4s
        fadd            v24.4s, v20.4s, v26.4s
        fadd            v25.4s, v16.4s, v27.4s

        fmul            v17.4s, v17.4s, v31.4s
        fmul            v18.4s, v18.4s, v31.4s
        fmul            v19.4s, v19.4s, v31.4s
        fmul            v21.4s, v21.4s, v31.4s

        rev64           v22.4s, v22.4s
        rev64           v24.4s, v24.4s
        rev64           v23.4s, v23.4s
        rev64           v25.4s, v25.4s

        fsub            v8.4s, v0.4s, v22.4s
        fsub            v9.4s, v1.4s, v24.4s
        fsub            v10.4s, v2.4s, v17.4s
        fsub            v11.4s, v3.4s, v19.4s

        fsub            v12.4s, v4.4s, v23.4s
        fsub            v13.4s, v5.4s, v25.4s
        fadd            v14.4s, v6.4s, v18.4s
        fsub            v15.4s, v7.4s, v21.4s

        fadd            v0.4s, v0.4s, v22.4s
        fadd            v1.4s, v1.4s, v24.4s
        fadd            v2.4s, v2.4s, v17.4s
        fadd            v3.4s, v3.4s, v19.4s

        fadd            v4.4s, v4.4s, v23.4s
        fsub            v6.4s, v6.4s, v18.4s      
        fadd            v5.4s, v5.4s, v25.4s      
        fadd            v7.4s, v7.4s, v21.4s

        stp             q0,  q1,  [x1,  #((0 + 0)*32 + 0)]
        stp             q4,  q5,  [x1,  #((2 + 0)*32 + 0)]
        stp             q2,  q3,  [x10, #((0 + 0)*32 + 0)]
        stp             q6,  q7,  [x10, #((2 + 0)*32 + 0)]

        stp             q8,  q9,  [x11, #((0 + 0)*32 + 0)]
        stp             q12, q13, [x11, #((2 + 0)*32 + 0)]
        stp             q10, q11, [x12, #((0 + 0)*32 + 0)]
        stp             q14, q15, [x12, #((2 + 0)*32 + 0)]
        add             x10, x1, x21
        add             x11, x1, x21, lsl #1
        add             x12, x1, x22

        ldp             q0,  q1,  [x1,  #((0 + 1)*32 + 0)]
        ldp             q4,  q5,  [x1,  #((2 + 1)*32 + 0)]
        ldp             q2,  q3,  [x10, #((0 + 1)*32 + 0)]
        ldp             q6,  q7,  [x10, #((2 + 1)*32 + 0)]

        ldp             q8,  q9,  [x11, #((0 + 1)*32 + 0)]
        ldp             q10, q11, [x11, #((2 + 1)*32 + 0)]
        ldp             q12, q13, [x12, #((0 + 1)*32 + 0)]
        ldp             q14, q15, [x12, #((2 + 1)*32 + 0)]


        ld1             { v24.4s, v25.4s }, [x8], x9
        ld1             { v16.4s, v17.4s }, [x7], #32

        ext8             v18.16b, v24.16b, v24.16b, #8
        ext8             v19.16b, v25.16b, v25.16b, #8

        trn1            v20.4s, v16.4s, v16.4s      
        trn2            v16.4s, v16.4s, v16.4s      
        trn1            v21.4s, v17.4s, v17.4s      
        trn2            v17.4s, v17.4s, v17.4s      

        rev64           v22.4s, v8.4s                 
        rev64           v23.4s, v10.4s                 
        rev64           v24.4s, v12.4s                 
        rev64           v25.4s, v14.4s                 

        fmul            v22.4s, v22.4s, v20.4s      
        fmul            v23.4s, v23.4s, v16.4s      
        fmul            v24.4s, v24.4s, v20.4s      
        fmul            v25.4s, v25.4s, v16.4s      

        rev64           v26.4s, v9.4s                 
        rev64           v27.4s, v11.4s                 
        rev64           v20.4s, v13.4s                 
        rev64           v16.4s, v15.4s                 

        fmul            v26.4s, v26.4s, v21.4s      
        fmul            v27.4s, v27.4s, v17.4s      
        fmul            v20.4s, v20.4s, v21.4s      
        fmul            v16.4s, v16.4s, v17.4s      

        trn1            v21.4s, v19.4s, v19.4s      
        trn2            v19.4s, v19.4s, v19.4s      
        trn1            v17.4s, v18.4s, v18.4s      
        trn2            v18.4s, v18.4s, v18.4s      

        fmul            v21.4s, v21.4s, v31.4s
        fmul            v19.4s, v19.4s, v31.4s
        fmul            v17.4s, v17.4s, v31.4s
        fmul            v18.4s, v18.4s, v31.4s

        fmla            v23.4s, v10.4s, v21.4s      
        fmls            v25.4s, v14.4s, v21.4s      
        fmla            v22.4s, v8.4s, v19.4s      
        fmls            v24.4s, v12.4s, v19.4s      

        fmla            v26.4s, v9.4s, v18.4s      
        fmla            v27.4s, v11.4s, v17.4s      
        fmls            v20.4s, v13.4s, v18.4s      
        fmls            v16.4s, v15.4s, v17.4s      

        fsub            v18.4s, v23.4s, v25.4s
        fsub            v17.4s, v24.4s, v22.4s
        fsub            v19.4s, v20.4s, v26.4s
        fsub            v21.4s, v16.4s, v27.4s

        fadd            v22.4s, v24.4s, v22.4s
        fadd            v23.4s, v25.4s, v23.4s
        fadd            v24.4s, v20.4s, v26.4s
        fadd            v25.4s, v16.4s, v27.4s

        fmul            v17.4s, v17.4s, v31.4s
        fmul            v18.4s, v18.4s, v31.4s
        fmul            v19.4s, v19.4s, v31.4s
        fmul            v21.4s, v21.4s, v31.4s

        rev64           v22.4s, v22.4s
        rev64           v24.4s, v24.4s
        rev64           v23.4s, v23.4s
        rev64           v25.4s, v25.4s

        fsub            v8.4s, v0.4s, v22.4s
        fsub            v9.4s, v1.4s, v24.4s
        fsub            v10.4s, v2.4s, v17.4s
        fsub            v11.4s, v3.4s, v19.4s

        fsub            v12.4s, v4.4s, v23.4s
        fsub            v13.4s, v5.4s, v25.4s
        fadd            v14.4s, v6.4s, v18.4s
        fsub            v15.4s, v7.4s, v21.4s

        fadd            v0.4s, v0.4s, v22.4s
        fadd            v1.4s, v1.4s, v24.4s
        fadd            v2.4s, v2.4s, v17.4s
        fadd            v3.4s, v3.4s, v19.4s

        fadd            v4.4s, v4.4s, v23.4s
        fsub            v6.4s, v6.4s, v18.4s      
        fadd            v5.4s, v5.4s, v25.4s      
        fadd            v7.4s, v7.4s, v21.4s

        stp             q0,  q1,  [x1,  #((0 + 1)*32 + 0)]
        stp             q4,  q5,  [x1,  #((2 + 1)*32 + 0)]
        stp             q2,  q3,  [x10, #((0 + 1)*32 + 0)]
        stp             q6,  q7,  [x10, #((2 + 1)*32 + 0)]

        stp             q8,  q9,  [x11, #((0 + 1)*32 + 0)]
        stp             q12, q13, [x11, #((2 + 1)*32 + 0)]
        stp             q10, q11, [x12, #((0 + 1)*32 + 0)]
        stp             q14, q15, [x12, #((2 + 1)*32 + 0)]
        add             x10, x1, x21
        add             x11, x1, x21, lsl #1
        add             x12, x1, x22

        ldp             q0,  q1,  [x1,  #((0 + 4)*32 + 0)]
        ldp             q4,  q5,  [x1,  #((2 + 4)*32 + 0)]
        ldp             q2,  q3,  [x10, #((0 + 4)*32 + 0)]
        ldp             q6,  q7,  [x10, #((2 + 4)*32 + 0)]

        ldp             q8,  q9,  [x11, #((0 + 4)*32 + 0)]
        ldp             q10, q11, [x11, #((2 + 4)*32 + 0)]
        ldp             q12, q13, [x12, #((0 + 4)*32 + 0)]
        ldp             q14, q15, [x12, #((2 + 4)*32 + 0)]


        ld1             { v24.4s, v25.4s }, [x8], x9
        ld1             { v16.4s, v17.4s }, [x7], #32

        ext8             v18.16b, v24.16b, v24.16b, #8
        ext8             v19.16b, v25.16b, v25.16b, #8

        trn1            v20.4s, v16.4s, v16.4s      
        trn2            v16.4s, v16.4s, v16.4s      
        trn1            v21.4s, v17.4s, v17.4s      
        trn2            v17.4s, v17.4s, v17.4s      

        rev64           v22.4s, v8.4s                 
        rev64           v23.4s, v10.4s                 
        rev64           v24.4s, v12.4s                 
        rev64           v25.4s, v14.4s                 

        fmul            v22.4s, v22.4s, v20.4s      
        fmul            v23.4s, v23.4s, v16.4s      
        fmul            v24.4s, v24.4s, v20.4s      
        fmul            v25.4s, v25.4s, v16.4s      

        rev64           v26.4s, v9.4s                 
        rev64           v27.4s, v11.4s                 
        rev64           v20.4s, v13.4s                 
        rev64           v16.4s, v15.4s                 

        fmul            v26.4s, v26.4s, v21.4s      
        fmul            v27.4s, v27.4s, v17.4s      
        fmul            v20.4s, v20.4s, v21.4s      
        fmul            v16.4s, v16.4s, v17.4s      

        trn1            v21.4s, v19.4s, v19.4s      
        trn2            v19.4s, v19.4s, v19.4s      
        trn1            v17.4s, v18.4s, v18.4s      
        trn2            v18.4s, v18.4s, v18.4s      

        fmul            v21.4s, v21.4s, v31.4s
        fmul            v19.4s, v19.4s, v31.4s
        fmul            v17.4s, v17.4s, v31.4s
        fmul            v18.4s, v18.4s, v31.4s

        fmla            v23.4s, v10.4s, v21.4s      
        fmls            v25.4s, v14.4s, v21.4s      
        fmla            v22.4s, v8.4s, v19.4s      
        fmls            v24.4s, v12.4s, v19.4s      

        fmla            v26.4s, v9.4s, v18.4s      
        fmla            v27.4s, v11.4s, v17.4s      
        fmls            v20.4s, v13.4s, v18.4s      
        fmls            v16.4s, v15.4s, v17.4s      

        fsub            v18.4s, v23.4s, v25.4s
        fsub            v17.4s, v24.4s, v22.4s
        fsub            v19.4s, v20.4s, v26.4s
        fsub            v21.4s, v16.4s, v27.4s

        fadd            v22.4s, v24.4s, v22.4s
        fadd            v23.4s, v25.4s, v23.4s
        fadd            v24.4s, v20.4s, v26.4s
        fadd            v25.4s, v16.4s, v27.4s

        fmul            v17.4s, v17.4s, v31.4s
        fmul            v18.4s, v18.4s, v31.4s
        fmul            v19.4s, v19.4s, v31.4s
        fmul            v21.4s, v21.4s, v31.4s

        rev64           v22.4s, v22.4s
        rev64           v24.4s, v24.4s
        rev64           v23.4s, v23.4s
        rev64           v25.4s, v25.4s

        fsub            v8.4s, v0.4s, v22.4s
        fsub            v9.4s, v1.4s, v24.4s
        fsub            v10.4s, v2.4s, v17.4s
        fsub            v11.4s, v3.4s, v19.4s

        fsub            v12.4s, v4.4s, v23.4s
        fsub            v13.4s, v5.4s, v25.4s
        fadd            v14.4s, v6.4s, v18.4s
        fsub            v15.4s, v7.4s, v21.4s

        fadd            v0.4s, v0.4s, v22.4s
        fadd            v1.4s, v1.4s, v24.4s
        fadd            v2.4s, v2.4s, v17.4s
        fadd            v3.4s, v3.4s, v19.4s

        fadd            v4.4s, v4.4s, v23.4s
        fsub            v6.4s, v6.4s, v18.4s      
        fadd            v5.4s, v5.4s, v25.4s      
        fadd            v7.4s, v7.4s, v21.4s

        stp             q0,  q1,  [x1,  #((0 + 4)*32 + 0)]
        stp             q4,  q5,  [x1,  #((2 + 4)*32 + 0)]
        stp             q2,  q3,  [x10, #((0 + 4)*32 + 0)]
        stp             q6,  q7,  [x10, #((2 + 4)*32 + 0)]

        stp             q8,  q9,  [x11, #((0 + 4)*32 + 0)]
        stp             q12, q13, [x11, #((2 + 4)*32 + 0)]
        stp             q10, q11, [x12, #((0 + 4)*32 + 0)]
        stp             q14, q15, [x12, #((2 + 4)*32 + 0)]
        add             x10, x1, x21
        add             x11, x1, x21, lsl #1
        add             x12, x1, x22

        ldp             q0,  q1,  [x1,  #((0 + 5)*32 + 0)]
        ldp             q4,  q5,  [x1,  #((2 + 5)*32 + 0)]
        ldp             q2,  q3,  [x10, #((0 + 5)*32 + 0)]
        ldp             q6,  q7,  [x10, #((2 + 5)*32 + 0)]

        ldp             q8,  q9,  [x11, #((0 + 5)*32 + 0)]
        ldp             q10, q11, [x11, #((2 + 5)*32 + 0)]
        ldp             q12, q13, [x12, #((0 + 5)*32 + 0)]
        ldp             q14, q15, [x12, #((2 + 5)*32 + 0)]


        ld1             { v24.4s, v25.4s }, [x8], x9
        ld1             { v16.4s, v17.4s }, [x7], #32

        ext8             v18.16b, v24.16b, v24.16b, #8
        ext8             v19.16b, v25.16b, v25.16b, #8

        trn1            v20.4s, v16.4s, v16.4s      
        trn2            v16.4s, v16.4s, v16.4s      
        trn1            v21.4s, v17.4s, v17.4s      
        trn2            v17.4s, v17.4s, v17.4s      

        rev64           v22.4s, v8.4s                 
        rev64           v23.4s, v10.4s                 
        rev64           v24.4s, v12.4s                 
        rev64           v25.4s, v14.4s                 

        fmul            v22.4s, v22.4s, v20.4s      
        fmul            v23.4s, v23.4s, v16.4s      
        fmul            v24.4s, v24.4s, v20.4s      
        fmul            v25.4s, v25.4s, v16.4s      

        rev64           v26.4s, v9.4s                 
        rev64           v27.4s, v11.4s                 
        rev64           v20.4s, v13.4s                 
        rev64           v16.4s, v15.4s                 

        fmul            v26.4s, v26.4s, v21.4s      
        fmul            v27.4s, v27.4s, v17.4s      
        fmul            v20.4s, v20.4s, v21.4s      
        fmul            v16.4s, v16.4s, v17.4s      

        trn1            v21.4s, v19.4s, v19.4s      
        trn2            v19.4s, v19.4s, v19.4s      
        trn1            v17.4s, v18.4s, v18.4s      
        trn2            v18.4s, v18.4s, v18.4s      

        fmul            v21.4s, v21.4s, v31.4s
        fmul            v19.4s, v19.4s, v31.4s
        fmul            v17.4s, v17.4s, v31.4s
        fmul            v18.4s, v18.4s, v31.4s

        fmla            v23.4s, v10.4s, v21.4s      
        fmls            v25.4s, v14.4s, v21.4s      
        fmla            v22.4s, v8.4s, v19.4s      
        fmls            v24.4s, v12.4s, v19.4s      

        fmla            v26.4s, v9.4s, v18.4s      
        fmla            v27.4s, v11.4s, v17.4s      
        fmls            v20.4s, v13.4s, v18.4s      
        fmls            v16.4s, v15.4s, v17.4s      

        fsub            v18.4s, v23.4s, v25.4s
        fsub            v17.4s, v24.4s, v22.4s
        fsub            v19.4s, v20.4s, v26.4s
        fsub            v21.4s, v16.4s, v27.4s

        fadd            v22.4s, v24.4s, v22.4s
        fadd            v23.4s, v25.4s, v23.4s
        fadd            v24.4s, v20.4s, v26.4s
        fadd            v25.4s, v16.4s, v27.4s

        fmul            v17.4s, v17.4s, v31.4s
        fmul            v18.4s, v18.4s, v31.4s
        fmul            v19.4s, v19.4s, v31.4s
        fmul            v21.4s, v21.4s, v31.4s

        rev64           v22.4s, v22.4s
        rev64           v24.4s, v24.4s
        rev64           v23.4s, v23.4s
        rev64           v25.4s, v25.4s

        fsub            v8.4s, v0.4s, v22.4s
        fsub            v9.4s, v1.4s, v24.4s
        fsub            v10.4s, v2.4s, v17.4s
        fsub            v11.4s, v3.4s, v19.4s

        fsub            v12.4s, v4.4s, v23.4s
        fsub            v13.4s, v5.4s, v25.4s
        fadd            v14.4s, v6.4s, v18.4s
        fsub            v15.4s, v7.4s, v21.4s

        fadd            v0.4s, v0.4s, v22.4s
        fadd            v1.4s, v1.4s, v24.4s
        fadd            v2.4s, v2.4s, v17.4s
        fadd            v3.4s, v3.4s, v19.4s

        fadd            v4.4s, v4.4s, v23.4s
        fsub            v6.4s, v6.4s, v18.4s      
        fadd            v5.4s, v5.4s, v25.4s      
        fadd            v7.4s, v7.4s, v21.4s

        stp             q0,  q1,  [x1,  #((0 + 5)*32 + 0)]
        stp             q4,  q5,  [x1,  #((2 + 5)*32 + 0)]
        stp             q2,  q3,  [x10, #((0 + 5)*32 + 0)]
        stp             q6,  q7,  [x10, #((2 + 5)*32 + 0)]

        stp             q8,  q9,  [x11, #((0 + 5)*32 + 0)]
        stp             q12, q13, [x11, #((2 + 5)*32 + 0)]
        stp             q10, q11, [x12, #((0 + 5)*32 + 0)]
        stp             q14, q15, [x12, #((2 + 5)*32 + 0)]
        add             x1, x1, 8*32
        subs            w5, w5, 1
        bgt            temp_label_37

        cmp             w20, #((32768)/4096), lsl #12
        bgt            temp_label_38
        ret
temp_label_39
temp_label_38

        stp             x20, x30, [sp, #-16]!
        mov             w20, #(65536/4)
        mov             x5, #((65536*4) - (65536/1))
        add             x1, x1, x5
        bl              temp_label_0
        mov             x5, #((65536*2) - (65536/2))
        add             x1, x1, x5
        bl              temp_label_0
        ldp             x20, x30, [sp], #16
        ldr             w5, =(65536*6 + 65536/2)
        sub             x1, x1, x5

        ldr             w5, =(65536 - 4*7)










        adrp            x7, ff_tx_tab_65536_float
        add             x7, x7, ff_tx_tab_65536_float










        add             x8, x7, x5
        mov             x9, #-32

        mov             x21, #2*65536
        add             x22, x21, x21, lsl #1

        cmp             w19, #((65536)/4096), lsl #12
        beq            temp_label_40

        mov             w5, #(65536/128)
temp_label_41

        add             x10, x1, x21
        add             x11, x1, x21, lsl #1
        add             x12, x1, x22

        add             x10, x1, x21
        add             x11, x1, x21, lsl #1
        add             x12, x1, x22

        ldp             q0,  q1,  [x1,  #((0 + 0)*32 + 0)]
        ldp             q4,  q5,  [x1,  #((2 + 0)*32 + 0)]
        ldp             q2,  q3,  [x10, #((0 + 0)*32 + 0)]
        ldp             q6,  q7,  [x10, #((2 + 0)*32 + 0)]

        ldp             q8,  q9,  [x11, #((0 + 0)*32 + 0)]
        ldp             q10, q11, [x11, #((2 + 0)*32 + 0)]
        ldp             q12, q13, [x12, #((0 + 0)*32 + 0)]
        ldp             q14, q15, [x12, #((2 + 0)*32 + 0)]


        ld1             { v24.4s, v25.4s }, [x8], x9
        ld1             { v16.4s, v17.4s }, [x7], #32

        ext8             v18.16b, v24.16b, v24.16b, #8
        ext8             v19.16b, v25.16b, v25.16b, #8

        trn1            v20.4s, v16.4s, v16.4s      
        trn2            v16.4s, v16.4s, v16.4s      
        trn1            v21.4s, v17.4s, v17.4s      
        trn2            v17.4s, v17.4s, v17.4s      

        rev64           v22.4s, v8.4s                 
        rev64           v23.4s, v10.4s                 
        rev64           v24.4s, v12.4s                 
        rev64           v25.4s, v14.4s                 

        fmul            v22.4s, v22.4s, v20.4s      
        fmul            v23.4s, v23.4s, v16.4s      
        fmul            v24.4s, v24.4s, v20.4s      
        fmul            v25.4s, v25.4s, v16.4s      

        rev64           v26.4s, v9.4s                 
        rev64           v27.4s, v11.4s                 
        rev64           v20.4s, v13.4s                 
        rev64           v16.4s, v15.4s                 

        fmul            v26.4s, v26.4s, v21.4s      
        fmul            v27.4s, v27.4s, v17.4s      
        fmul            v20.4s, v20.4s, v21.4s      
        fmul            v16.4s, v16.4s, v17.4s      

        trn1            v21.4s, v19.4s, v19.4s      
        trn2            v19.4s, v19.4s, v19.4s      
        trn1            v17.4s, v18.4s, v18.4s      
        trn2            v18.4s, v18.4s, v18.4s      

        fmul            v21.4s, v21.4s, v31.4s
        fmul            v19.4s, v19.4s, v31.4s
        fmul            v17.4s, v17.4s, v31.4s
        fmul            v18.4s, v18.4s, v31.4s

        fmla            v23.4s, v10.4s, v21.4s      
        fmls            v25.4s, v14.4s, v21.4s      
        fmla            v22.4s, v8.4s, v19.4s      
        fmls            v24.4s, v12.4s, v19.4s      

        fmla            v26.4s, v9.4s, v18.4s      
        fmla            v27.4s, v11.4s, v17.4s      
        fmls            v20.4s, v13.4s, v18.4s      
        fmls            v16.4s, v15.4s, v17.4s      

        fsub            v18.4s, v23.4s, v25.4s
        fsub            v17.4s, v24.4s, v22.4s
        fsub            v19.4s, v20.4s, v26.4s
        fsub            v21.4s, v16.4s, v27.4s

        fadd            v22.4s, v24.4s, v22.4s
        fadd            v23.4s, v25.4s, v23.4s
        fadd            v24.4s, v20.4s, v26.4s
        fadd            v25.4s, v16.4s, v27.4s

        fmul            v17.4s, v17.4s, v31.4s
        fmul            v18.4s, v18.4s, v31.4s
        fmul            v19.4s, v19.4s, v31.4s
        fmul            v21.4s, v21.4s, v31.4s

        rev64           v22.4s, v22.4s
        rev64           v24.4s, v24.4s
        rev64           v23.4s, v23.4s
        rev64           v25.4s, v25.4s

        fsub            v8.4s, v0.4s, v22.4s
        fsub            v9.4s, v1.4s, v24.4s
        fsub            v10.4s, v2.4s, v17.4s
        fsub            v11.4s, v3.4s, v19.4s

        fsub            v12.4s, v4.4s, v23.4s
        fsub            v13.4s, v5.4s, v25.4s
        fadd            v14.4s, v6.4s, v18.4s
        fsub            v15.4s, v7.4s, v21.4s

        fadd            v0.4s, v0.4s, v22.4s
        fadd            v1.4s, v1.4s, v24.4s
        fadd            v2.4s, v2.4s, v17.4s
        fadd            v3.4s, v3.4s, v19.4s

        fadd            v4.4s, v4.4s, v23.4s
        fsub            v6.4s, v6.4s, v18.4s      
        fadd            v5.4s, v5.4s, v25.4s      
        fadd            v7.4s, v7.4s, v21.4s

        stp             q0,  q1,  [x1,  #((0 + 0)*32 + 0)]
        stp             q4,  q5,  [x1,  #((2 + 0)*32 + 0)]
        stp             q2,  q3,  [x10, #((0 + 0)*32 + 0)]
        stp             q6,  q7,  [x10, #((2 + 0)*32 + 0)]

        stp             q8,  q9,  [x11, #((0 + 0)*32 + 0)]
        stp             q12, q13, [x11, #((2 + 0)*32 + 0)]
        stp             q10, q11, [x12, #((0 + 0)*32 + 0)]
        stp             q14, q15, [x12, #((2 + 0)*32 + 0)]
        add             x10, x1, x21
        add             x11, x1, x21, lsl #1
        add             x12, x1, x22

        ldp             q0,  q1,  [x1,  #((0 + 1)*32 + 0)]
        ldp             q4,  q5,  [x1,  #((2 + 1)*32 + 0)]
        ldp             q2,  q3,  [x10, #((0 + 1)*32 + 0)]
        ldp             q6,  q7,  [x10, #((2 + 1)*32 + 0)]

        ldp             q8,  q9,  [x11, #((0 + 1)*32 + 0)]
        ldp             q10, q11, [x11, #((2 + 1)*32 + 0)]
        ldp             q12, q13, [x12, #((0 + 1)*32 + 0)]
        ldp             q14, q15, [x12, #((2 + 1)*32 + 0)]


        ld1             { v24.4s, v25.4s }, [x8], x9
        ld1             { v16.4s, v17.4s }, [x7], #32

        ext8             v18.16b, v24.16b, v24.16b, #8
        ext8             v19.16b, v25.16b, v25.16b, #8

        trn1            v20.4s, v16.4s, v16.4s      
        trn2            v16.4s, v16.4s, v16.4s      
        trn1            v21.4s, v17.4s, v17.4s      
        trn2            v17.4s, v17.4s, v17.4s      

        rev64           v22.4s, v8.4s                 
        rev64           v23.4s, v10.4s                 
        rev64           v24.4s, v12.4s                 
        rev64           v25.4s, v14.4s                 

        fmul            v22.4s, v22.4s, v20.4s      
        fmul            v23.4s, v23.4s, v16.4s      
        fmul            v24.4s, v24.4s, v20.4s      
        fmul            v25.4s, v25.4s, v16.4s      

        rev64           v26.4s, v9.4s                 
        rev64           v27.4s, v11.4s                 
        rev64           v20.4s, v13.4s                 
        rev64           v16.4s, v15.4s                 

        fmul            v26.4s, v26.4s, v21.4s      
        fmul            v27.4s, v27.4s, v17.4s      
        fmul            v20.4s, v20.4s, v21.4s      
        fmul            v16.4s, v16.4s, v17.4s      

        trn1            v21.4s, v19.4s, v19.4s      
        trn2            v19.4s, v19.4s, v19.4s      
        trn1            v17.4s, v18.4s, v18.4s      
        trn2            v18.4s, v18.4s, v18.4s      

        fmul            v21.4s, v21.4s, v31.4s
        fmul            v19.4s, v19.4s, v31.4s
        fmul            v17.4s, v17.4s, v31.4s
        fmul            v18.4s, v18.4s, v31.4s

        fmla            v23.4s, v10.4s, v21.4s      
        fmls            v25.4s, v14.4s, v21.4s      
        fmla            v22.4s, v8.4s, v19.4s      
        fmls            v24.4s, v12.4s, v19.4s      

        fmla            v26.4s, v9.4s, v18.4s      
        fmla            v27.4s, v11.4s, v17.4s      
        fmls            v20.4s, v13.4s, v18.4s      
        fmls            v16.4s, v15.4s, v17.4s      

        fsub            v18.4s, v23.4s, v25.4s
        fsub            v17.4s, v24.4s, v22.4s
        fsub            v19.4s, v20.4s, v26.4s
        fsub            v21.4s, v16.4s, v27.4s

        fadd            v22.4s, v24.4s, v22.4s
        fadd            v23.4s, v25.4s, v23.4s
        fadd            v24.4s, v20.4s, v26.4s
        fadd            v25.4s, v16.4s, v27.4s

        fmul            v17.4s, v17.4s, v31.4s
        fmul            v18.4s, v18.4s, v31.4s
        fmul            v19.4s, v19.4s, v31.4s
        fmul            v21.4s, v21.4s, v31.4s

        rev64           v22.4s, v22.4s
        rev64           v24.4s, v24.4s
        rev64           v23.4s, v23.4s
        rev64           v25.4s, v25.4s

        fsub            v8.4s, v0.4s, v22.4s
        fsub            v9.4s, v1.4s, v24.4s
        fsub            v10.4s, v2.4s, v17.4s
        fsub            v11.4s, v3.4s, v19.4s

        fsub            v12.4s, v4.4s, v23.4s
        fsub            v13.4s, v5.4s, v25.4s
        fadd            v14.4s, v6.4s, v18.4s
        fsub            v15.4s, v7.4s, v21.4s

        fadd            v0.4s, v0.4s, v22.4s
        fadd            v1.4s, v1.4s, v24.4s
        fadd            v2.4s, v2.4s, v17.4s
        fadd            v3.4s, v3.4s, v19.4s

        fadd            v4.4s, v4.4s, v23.4s
        fsub            v6.4s, v6.4s, v18.4s      
        fadd            v5.4s, v5.4s, v25.4s      
        fadd            v7.4s, v7.4s, v21.4s

        stp             q0,  q1,  [x1,  #((0 + 1)*32 + 0)]
        stp             q4,  q5,  [x1,  #((2 + 1)*32 + 0)]
        stp             q2,  q3,  [x10, #((0 + 1)*32 + 0)]
        stp             q6,  q7,  [x10, #((2 + 1)*32 + 0)]

        stp             q8,  q9,  [x11, #((0 + 1)*32 + 0)]
        stp             q12, q13, [x11, #((2 + 1)*32 + 0)]
        stp             q10, q11, [x12, #((0 + 1)*32 + 0)]
        stp             q14, q15, [x12, #((2 + 1)*32 + 0)]
        add             x10, x1, x21
        add             x11, x1, x21, lsl #1
        add             x12, x1, x22

        ldp             q0,  q1,  [x1,  #((0 + 4)*32 + 0)]
        ldp             q4,  q5,  [x1,  #((2 + 4)*32 + 0)]
        ldp             q2,  q3,  [x10, #((0 + 4)*32 + 0)]
        ldp             q6,  q7,  [x10, #((2 + 4)*32 + 0)]

        ldp             q8,  q9,  [x11, #((0 + 4)*32 + 0)]
        ldp             q10, q11, [x11, #((2 + 4)*32 + 0)]
        ldp             q12, q13, [x12, #((0 + 4)*32 + 0)]
        ldp             q14, q15, [x12, #((2 + 4)*32 + 0)]


        ld1             { v24.4s, v25.4s }, [x8], x9
        ld1             { v16.4s, v17.4s }, [x7], #32

        ext8             v18.16b, v24.16b, v24.16b, #8
        ext8             v19.16b, v25.16b, v25.16b, #8

        trn1            v20.4s, v16.4s, v16.4s      
        trn2            v16.4s, v16.4s, v16.4s      
        trn1            v21.4s, v17.4s, v17.4s      
        trn2            v17.4s, v17.4s, v17.4s      

        rev64           v22.4s, v8.4s                 
        rev64           v23.4s, v10.4s                 
        rev64           v24.4s, v12.4s                 
        rev64           v25.4s, v14.4s                 

        fmul            v22.4s, v22.4s, v20.4s      
        fmul            v23.4s, v23.4s, v16.4s      
        fmul            v24.4s, v24.4s, v20.4s      
        fmul            v25.4s, v25.4s, v16.4s      

        rev64           v26.4s, v9.4s                 
        rev64           v27.4s, v11.4s                 
        rev64           v20.4s, v13.4s                 
        rev64           v16.4s, v15.4s                 

        fmul            v26.4s, v26.4s, v21.4s      
        fmul            v27.4s, v27.4s, v17.4s      
        fmul            v20.4s, v20.4s, v21.4s      
        fmul            v16.4s, v16.4s, v17.4s      

        trn1            v21.4s, v19.4s, v19.4s      
        trn2            v19.4s, v19.4s, v19.4s      
        trn1            v17.4s, v18.4s, v18.4s      
        trn2            v18.4s, v18.4s, v18.4s      

        fmul            v21.4s, v21.4s, v31.4s
        fmul            v19.4s, v19.4s, v31.4s
        fmul            v17.4s, v17.4s, v31.4s
        fmul            v18.4s, v18.4s, v31.4s

        fmla            v23.4s, v10.4s, v21.4s      
        fmls            v25.4s, v14.4s, v21.4s      
        fmla            v22.4s, v8.4s, v19.4s      
        fmls            v24.4s, v12.4s, v19.4s      

        fmla            v26.4s, v9.4s, v18.4s      
        fmla            v27.4s, v11.4s, v17.4s      
        fmls            v20.4s, v13.4s, v18.4s      
        fmls            v16.4s, v15.4s, v17.4s      

        fsub            v18.4s, v23.4s, v25.4s
        fsub            v17.4s, v24.4s, v22.4s
        fsub            v19.4s, v20.4s, v26.4s
        fsub            v21.4s, v16.4s, v27.4s

        fadd            v22.4s, v24.4s, v22.4s
        fadd            v23.4s, v25.4s, v23.4s
        fadd            v24.4s, v20.4s, v26.4s
        fadd            v25.4s, v16.4s, v27.4s

        fmul            v17.4s, v17.4s, v31.4s
        fmul            v18.4s, v18.4s, v31.4s
        fmul            v19.4s, v19.4s, v31.4s
        fmul            v21.4s, v21.4s, v31.4s

        rev64           v22.4s, v22.4s
        rev64           v24.4s, v24.4s
        rev64           v23.4s, v23.4s
        rev64           v25.4s, v25.4s

        fsub            v8.4s, v0.4s, v22.4s
        fsub            v9.4s, v1.4s, v24.4s
        fsub            v10.4s, v2.4s, v17.4s
        fsub            v11.4s, v3.4s, v19.4s

        fsub            v12.4s, v4.4s, v23.4s
        fsub            v13.4s, v5.4s, v25.4s
        fadd            v14.4s, v6.4s, v18.4s
        fsub            v15.4s, v7.4s, v21.4s

        fadd            v0.4s, v0.4s, v22.4s
        fadd            v1.4s, v1.4s, v24.4s
        fadd            v2.4s, v2.4s, v17.4s
        fadd            v3.4s, v3.4s, v19.4s

        fadd            v4.4s, v4.4s, v23.4s
        fsub            v6.4s, v6.4s, v18.4s      
        fadd            v5.4s, v5.4s, v25.4s      
        fadd            v7.4s, v7.4s, v21.4s

        stp             q0,  q1,  [x1,  #((0 + 4)*32 + 0)]
        stp             q4,  q5,  [x1,  #((2 + 4)*32 + 0)]
        stp             q2,  q3,  [x10, #((0 + 4)*32 + 0)]
        stp             q6,  q7,  [x10, #((2 + 4)*32 + 0)]

        stp             q8,  q9,  [x11, #((0 + 4)*32 + 0)]
        stp             q12, q13, [x11, #((2 + 4)*32 + 0)]
        stp             q10, q11, [x12, #((0 + 4)*32 + 0)]
        stp             q14, q15, [x12, #((2 + 4)*32 + 0)]
        add             x10, x1, x21
        add             x11, x1, x21, lsl #1
        add             x12, x1, x22

        ldp             q0,  q1,  [x1,  #((0 + 5)*32 + 0)]
        ldp             q4,  q5,  [x1,  #((2 + 5)*32 + 0)]
        ldp             q2,  q3,  [x10, #((0 + 5)*32 + 0)]
        ldp             q6,  q7,  [x10, #((2 + 5)*32 + 0)]

        ldp             q8,  q9,  [x11, #((0 + 5)*32 + 0)]
        ldp             q10, q11, [x11, #((2 + 5)*32 + 0)]
        ldp             q12, q13, [x12, #((0 + 5)*32 + 0)]
        ldp             q14, q15, [x12, #((2 + 5)*32 + 0)]


        ld1             { v24.4s, v25.4s }, [x8], x9
        ld1             { v16.4s, v17.4s }, [x7], #32

        ext8             v18.16b, v24.16b, v24.16b, #8
        ext8             v19.16b, v25.16b, v25.16b, #8

        trn1            v20.4s, v16.4s, v16.4s      
        trn2            v16.4s, v16.4s, v16.4s      
        trn1            v21.4s, v17.4s, v17.4s      
        trn2            v17.4s, v17.4s, v17.4s      

        rev64           v22.4s, v8.4s                 
        rev64           v23.4s, v10.4s                 
        rev64           v24.4s, v12.4s                 
        rev64           v25.4s, v14.4s                 

        fmul            v22.4s, v22.4s, v20.4s      
        fmul            v23.4s, v23.4s, v16.4s      
        fmul            v24.4s, v24.4s, v20.4s      
        fmul            v25.4s, v25.4s, v16.4s      

        rev64           v26.4s, v9.4s                 
        rev64           v27.4s, v11.4s                 
        rev64           v20.4s, v13.4s                 
        rev64           v16.4s, v15.4s                 

        fmul            v26.4s, v26.4s, v21.4s      
        fmul            v27.4s, v27.4s, v17.4s      
        fmul            v20.4s, v20.4s, v21.4s      
        fmul            v16.4s, v16.4s, v17.4s      

        trn1            v21.4s, v19.4s, v19.4s      
        trn2            v19.4s, v19.4s, v19.4s      
        trn1            v17.4s, v18.4s, v18.4s      
        trn2            v18.4s, v18.4s, v18.4s      

        fmul            v21.4s, v21.4s, v31.4s
        fmul            v19.4s, v19.4s, v31.4s
        fmul            v17.4s, v17.4s, v31.4s
        fmul            v18.4s, v18.4s, v31.4s

        fmla            v23.4s, v10.4s, v21.4s      
        fmls            v25.4s, v14.4s, v21.4s      
        fmla            v22.4s, v8.4s, v19.4s      
        fmls            v24.4s, v12.4s, v19.4s      

        fmla            v26.4s, v9.4s, v18.4s      
        fmla            v27.4s, v11.4s, v17.4s      
        fmls            v20.4s, v13.4s, v18.4s      
        fmls            v16.4s, v15.4s, v17.4s      

        fsub            v18.4s, v23.4s, v25.4s
        fsub            v17.4s, v24.4s, v22.4s
        fsub            v19.4s, v20.4s, v26.4s
        fsub            v21.4s, v16.4s, v27.4s

        fadd            v22.4s, v24.4s, v22.4s
        fadd            v23.4s, v25.4s, v23.4s
        fadd            v24.4s, v20.4s, v26.4s
        fadd            v25.4s, v16.4s, v27.4s

        fmul            v17.4s, v17.4s, v31.4s
        fmul            v18.4s, v18.4s, v31.4s
        fmul            v19.4s, v19.4s, v31.4s
        fmul            v21.4s, v21.4s, v31.4s

        rev64           v22.4s, v22.4s
        rev64           v24.4s, v24.4s
        rev64           v23.4s, v23.4s
        rev64           v25.4s, v25.4s

        fsub            v8.4s, v0.4s, v22.4s
        fsub            v9.4s, v1.4s, v24.4s
        fsub            v10.4s, v2.4s, v17.4s
        fsub            v11.4s, v3.4s, v19.4s

        fsub            v12.4s, v4.4s, v23.4s
        fsub            v13.4s, v5.4s, v25.4s
        fadd            v14.4s, v6.4s, v18.4s
        fsub            v15.4s, v7.4s, v21.4s

        fadd            v0.4s, v0.4s, v22.4s
        fadd            v1.4s, v1.4s, v24.4s
        fadd            v2.4s, v2.4s, v17.4s
        fadd            v3.4s, v3.4s, v19.4s

        fadd            v4.4s, v4.4s, v23.4s
        fsub            v6.4s, v6.4s, v18.4s      
        fadd            v5.4s, v5.4s, v25.4s      
        fadd            v7.4s, v7.4s, v21.4s

        stp             q0,  q1,  [x1,  #((0 + 5)*32 + 0)]
        stp             q4,  q5,  [x1,  #((2 + 5)*32 + 0)]
        stp             q2,  q3,  [x10, #((0 + 5)*32 + 0)]
        stp             q6,  q7,  [x10, #((2 + 5)*32 + 0)]

        stp             q8,  q9,  [x11, #((0 + 5)*32 + 0)]
        stp             q12, q13, [x11, #((2 + 5)*32 + 0)]
        stp             q10, q11, [x12, #((0 + 5)*32 + 0)]
        stp             q14, q15, [x12, #((2 + 5)*32 + 0)]
        add             x1, x1, 8*32
        subs            w5, w5, 1
        bgt            temp_label_41

        cmp             w20, #((65536)/4096), lsl #12
        bgt            temp_label_42
        ret
temp_label_43
temp_label_42

        stp             x20, x30, [sp, #-16]!
        mov             w20, #(131072/4)
        mov             x5, #((131072*4) - (131072/1))
        add             x1, x1, x5
        bl              temp_label_0
        mov             x5, #((131072*2) - (131072/2))
        add             x1, x1, x5
        bl              temp_label_0
        ldp             x20, x30, [sp], #16
        ldr             w5, =(131072*6 + 131072/2)
        sub             x1, x1, x5

        ldr             w5, =(131072 - 4*7)










        adrp            x7, ff_tx_tab_131072_float
        add             x7, x7, ff_tx_tab_131072_float










        add             x8, x7, x5
        mov             x9, #-32

        mov             x21, #2*131072
        add             x22, x21, x21, lsl #1


temp_label_44
temp_label_40
temp_label_36
temp_label_32
temp_label_28
temp_label_24
temp_label_20
temp_label_16
temp_label_12
temp_label_9
temp_label_6
 
        add             x14, x1, x21
        add             x15, x1, x21, lsl #1
        add             x16, x1, x22

        add             x10,  x1, #((0)*32 + 0)
        add             x11, x14, #((0)*32 + 0)
        add             x12, x15, #((0)*32 + 0)
        add             x13, x16, #((0)*32 + 0)

        ldp             q0,  q1,  [x10]
        ldp             q4,  q5,  [x10, #(2*32)]
        ldp             q2,  q3,  [x11]
        ldp             q6,  q7,  [x11, #(2*32)]

        ldp             q8,  q9,  [x12]
        ldp             q10, q11, [x12, #(2*32)]
        ldp             q12, q13, [x13]
        ldp             q14, q15, [x13, #(2*32)]


        ld1             { v24.4s, v25.4s }, [x8], x9
        ld1             { v16.4s, v17.4s }, [x7], #32

        ext8             v18.16b, v24.16b, v24.16b, #8
        ext8             v19.16b, v25.16b, v25.16b, #8

        trn1            v20.4s, v16.4s, v16.4s      
        trn2            v16.4s, v16.4s, v16.4s      
        trn1            v21.4s, v17.4s, v17.4s      
        trn2            v17.4s, v17.4s, v17.4s      

        rev64           v22.4s, v8.4s                 
        rev64           v23.4s, v10.4s                 
        rev64           v24.4s, v12.4s                 
        rev64           v25.4s, v14.4s                 

        fmul            v22.4s, v22.4s, v20.4s      
        fmul            v23.4s, v23.4s, v16.4s      
        fmul            v24.4s, v24.4s, v20.4s      
        fmul            v25.4s, v25.4s, v16.4s      

        rev64           v26.4s, v9.4s                 
        rev64           v27.4s, v11.4s                 
        rev64           v20.4s, v13.4s                 
        rev64           v16.4s, v15.4s                 

        fmul            v26.4s, v26.4s, v21.4s      
        fmul            v27.4s, v27.4s, v17.4s      
        fmul            v20.4s, v20.4s, v21.4s      
        fmul            v16.4s, v16.4s, v17.4s      

        trn1            v21.4s, v19.4s, v19.4s      
        trn2            v19.4s, v19.4s, v19.4s      
        trn1            v17.4s, v18.4s, v18.4s      
        trn2            v18.4s, v18.4s, v18.4s      

        fmul            v21.4s, v21.4s, v31.4s
        fmul            v19.4s, v19.4s, v31.4s
        fmul            v17.4s, v17.4s, v31.4s
        fmul            v18.4s, v18.4s, v31.4s

        fmla            v23.4s, v10.4s, v21.4s      
        fmls            v25.4s, v14.4s, v21.4s      
        fmla            v22.4s, v8.4s, v19.4s      
        fmls            v24.4s, v12.4s, v19.4s      

        fmla            v26.4s, v9.4s, v18.4s      
        fmla            v27.4s, v11.4s, v17.4s      
        fmls            v20.4s, v13.4s, v18.4s      
        fmls            v16.4s, v15.4s, v17.4s      

        fsub            v18.4s, v23.4s, v25.4s
        fsub            v17.4s, v24.4s, v22.4s
        fsub            v19.4s, v20.4s, v26.4s
        fsub            v21.4s, v16.4s, v27.4s

        fadd            v22.4s, v24.4s, v22.4s
        fadd            v23.4s, v25.4s, v23.4s
        fadd            v24.4s, v20.4s, v26.4s
        fadd            v25.4s, v16.4s, v27.4s

        fmul            v17.4s, v17.4s, v31.4s
        fmul            v18.4s, v18.4s, v31.4s
        fmul            v19.4s, v19.4s, v31.4s
        fmul            v21.4s, v21.4s, v31.4s

        rev64           v22.4s, v22.4s
        rev64           v24.4s, v24.4s
        rev64           v23.4s, v23.4s
        rev64           v25.4s, v25.4s

        fsub            v8.4s, v0.4s, v22.4s
        fsub            v9.4s, v1.4s, v24.4s
        fsub            v10.4s, v2.4s, v17.4s
        fsub            v11.4s, v3.4s, v19.4s

        fsub            v12.4s, v4.4s, v23.4s
        fsub            v13.4s, v5.4s, v25.4s
        fadd            v14.4s, v6.4s, v18.4s
        fsub            v15.4s, v7.4s, v21.4s

        fadd            v0.4s, v0.4s, v22.4s
        fadd            v1.4s, v1.4s, v24.4s
        fadd            v2.4s, v2.4s, v17.4s
        fadd            v3.4s, v3.4s, v19.4s

        fadd            v4.4s, v4.4s, v23.4s
        fsub            v6.4s, v6.4s, v18.4s      
        fadd            v5.4s, v5.4s, v25.4s      
        fadd            v7.4s, v7.4s, v21.4s

        zip1            v16.2d, v0.2d, v4.2d
        zip2            v17.2d, v0.2d, v4.2d
        zip1            v18.2d, v1.2d, v5.2d
        zip2            v19.2d, v1.2d, v5.2d

        zip1            v20.2d, v2.2d, v6.2d
        zip2            v21.2d, v2.2d, v6.2d
        zip1            v22.2d, v3.2d, v7.2d
        zip2            v23.2d, v3.2d, v7.2d

        ldp             q0,  q1,  [x10, #(1*32)]
        ldp             q4,  q5,  [x10, #(3*32)]
        ldp             q2,  q3,  [x11, #(1*32)]
        ldp             q6,  q7,  [x11, #(3*32)]

        st1             { v16.4s, v17.4s, v18.4s, v19.4s }, [x10], #64
        st1             { v20.4s, v21.4s, v22.4s, v23.4s }, [x11], #64

        zip1            v20.2d, v8.2d, v12.2d
        zip2            v21.2d, v8.2d, v12.2d
        zip1            v22.2d, v9.2d, v13.2d
        zip2            v23.2d, v9.2d, v13.2d
        zip1            v24.2d, v10.2d, v14.2d
        zip2            v25.2d, v10.2d, v14.2d
        zip1            v26.2d, v11.2d, v15.2d
        zip2            v27.2d, v11.2d, v15.2d

        ldp             q8,  q9,  [x12, #(1*32)]
        ldp             q10, q11, [x12, #(3*32)]
        ldp             q12, q13, [x13, #(1*32)]
        ldp             q14, q15, [x13, #(3*32)]

        st1             { v20.4s, v21.4s, v22.4s, v23.4s }, [x12], #64
        st1             { v24.4s, v25.4s, v26.4s, v27.4s }, [x13], #64


        ld1             { v24.4s, v25.4s }, [x8], x9
        ld1             { v16.4s, v17.4s }, [x7], #32

        ext8             v18.16b, v24.16b, v24.16b, #8
        ext8             v19.16b, v25.16b, v25.16b, #8

        trn1            v20.4s, v16.4s, v16.4s      
        trn2            v16.4s, v16.4s, v16.4s      
        trn1            v21.4s, v17.4s, v17.4s      
        trn2            v17.4s, v17.4s, v17.4s      

        rev64           v22.4s, v8.4s                 
        rev64           v23.4s, v10.4s                 
        rev64           v24.4s, v12.4s                 
        rev64           v25.4s, v14.4s                 

        fmul            v22.4s, v22.4s, v20.4s      
        fmul            v23.4s, v23.4s, v16.4s      
        fmul            v24.4s, v24.4s, v20.4s      
        fmul            v25.4s, v25.4s, v16.4s      

        rev64           v26.4s, v9.4s                 
        rev64           v27.4s, v11.4s                 
        rev64           v20.4s, v13.4s                 
        rev64           v16.4s, v15.4s                 

        fmul            v26.4s, v26.4s, v21.4s      
        fmul            v27.4s, v27.4s, v17.4s      
        fmul            v20.4s, v20.4s, v21.4s      
        fmul            v16.4s, v16.4s, v17.4s      

        trn1            v21.4s, v19.4s, v19.4s      
        trn2            v19.4s, v19.4s, v19.4s      
        trn1            v17.4s, v18.4s, v18.4s      
        trn2            v18.4s, v18.4s, v18.4s      

        fmul            v21.4s, v21.4s, v31.4s
        fmul            v19.4s, v19.4s, v31.4s
        fmul            v17.4s, v17.4s, v31.4s
        fmul            v18.4s, v18.4s, v31.4s

        fmla            v23.4s, v10.4s, v21.4s      
        fmls            v25.4s, v14.4s, v21.4s      
        fmla            v22.4s, v8.4s, v19.4s      
        fmls            v24.4s, v12.4s, v19.4s      

        fmla            v26.4s, v9.4s, v18.4s      
        fmla            v27.4s, v11.4s, v17.4s      
        fmls            v20.4s, v13.4s, v18.4s      
        fmls            v16.4s, v15.4s, v17.4s      

        fsub            v18.4s, v23.4s, v25.4s
        fsub            v17.4s, v24.4s, v22.4s
        fsub            v19.4s, v20.4s, v26.4s
        fsub            v21.4s, v16.4s, v27.4s

        fadd            v22.4s, v24.4s, v22.4s
        fadd            v23.4s, v25.4s, v23.4s
        fadd            v24.4s, v20.4s, v26.4s
        fadd            v25.4s, v16.4s, v27.4s

        fmul            v17.4s, v17.4s, v31.4s
        fmul            v18.4s, v18.4s, v31.4s
        fmul            v19.4s, v19.4s, v31.4s
        fmul            v21.4s, v21.4s, v31.4s

        rev64           v22.4s, v22.4s
        rev64           v24.4s, v24.4s
        rev64           v23.4s, v23.4s
        rev64           v25.4s, v25.4s

        fsub            v8.4s, v0.4s, v22.4s
        fsub            v9.4s, v1.4s, v24.4s
        fsub            v10.4s, v2.4s, v17.4s
        fsub            v11.4s, v3.4s, v19.4s

        fsub            v12.4s, v4.4s, v23.4s
        fsub            v13.4s, v5.4s, v25.4s
        fadd            v14.4s, v6.4s, v18.4s
        fsub            v15.4s, v7.4s, v21.4s

        fadd            v0.4s, v0.4s, v22.4s
        fadd            v1.4s, v1.4s, v24.4s
        fadd            v2.4s, v2.4s, v17.4s
        fadd            v3.4s, v3.4s, v19.4s

        fadd            v4.4s, v4.4s, v23.4s
        fsub            v6.4s, v6.4s, v18.4s      
        fadd            v5.4s, v5.4s, v25.4s      
        fadd            v7.4s, v7.4s, v21.4s

        zip1            v16.2d, v0.2d, v4.2d
        zip2            v17.2d, v0.2d, v4.2d
        zip1            v18.2d, v1.2d, v5.2d
        zip2            v19.2d, v1.2d, v5.2d
        st1             { v16.4s, v17.4s, v18.4s, v19.4s }, [x10]

        zip1            v16.2d, v2.2d, v6.2d
        zip2            v17.2d, v2.2d, v6.2d
        zip1            v18.2d, v3.2d, v7.2d
        zip2            v19.2d, v3.2d, v7.2d
        st1             { v16.4s, v17.4s, v18.4s, v19.4s }, [x11]

        zip1            v20.2d, v8.2d, v12.2d
        zip2            v21.2d, v8.2d, v12.2d
        zip1            v22.2d, v9.2d, v13.2d
        zip2            v23.2d, v9.2d, v13.2d
        st1             { v20.4s, v21.4s, v22.4s, v23.4s }, [x12]

        zip1            v24.2d, v10.2d, v14.2d
        zip2            v25.2d, v10.2d, v14.2d
        zip1            v26.2d, v11.2d, v15.2d
        zip2            v27.2d, v11.2d, v15.2d
        st1             { v24.4s, v25.4s, v26.4s, v27.4s }, [x13]
        add             x10,  x1, #((4)*32 + 0)
        add             x11, x14, #((4)*32 + 0)
        add             x12, x15, #((4)*32 + 0)
        add             x13, x16, #((4)*32 + 0)

        ldp             q0,  q1,  [x10]
        ldp             q4,  q5,  [x10, #(2*32)]
        ldp             q2,  q3,  [x11]
        ldp             q6,  q7,  [x11, #(2*32)]

        ldp             q8,  q9,  [x12]
        ldp             q10, q11, [x12, #(2*32)]
        ldp             q12, q13, [x13]
        ldp             q14, q15, [x13, #(2*32)]


        ld1             { v24.4s, v25.4s }, [x8], x9
        ld1             { v16.4s, v17.4s }, [x7], #32

        ext8             v18.16b, v24.16b, v24.16b, #8
        ext8             v19.16b, v25.16b, v25.16b, #8

        trn1            v20.4s, v16.4s, v16.4s      
        trn2            v16.4s, v16.4s, v16.4s      
        trn1            v21.4s, v17.4s, v17.4s      
        trn2            v17.4s, v17.4s, v17.4s      

        rev64           v22.4s, v8.4s                 
        rev64           v23.4s, v10.4s                 
        rev64           v24.4s, v12.4s                 
        rev64           v25.4s, v14.4s                 

        fmul            v22.4s, v22.4s, v20.4s      
        fmul            v23.4s, v23.4s, v16.4s      
        fmul            v24.4s, v24.4s, v20.4s      
        fmul            v25.4s, v25.4s, v16.4s      

        rev64           v26.4s, v9.4s                 
        rev64           v27.4s, v11.4s                 
        rev64           v20.4s, v13.4s                 
        rev64           v16.4s, v15.4s                 

        fmul            v26.4s, v26.4s, v21.4s      
        fmul            v27.4s, v27.4s, v17.4s      
        fmul            v20.4s, v20.4s, v21.4s      
        fmul            v16.4s, v16.4s, v17.4s      

        trn1            v21.4s, v19.4s, v19.4s      
        trn2            v19.4s, v19.4s, v19.4s      
        trn1            v17.4s, v18.4s, v18.4s      
        trn2            v18.4s, v18.4s, v18.4s      

        fmul            v21.4s, v21.4s, v31.4s
        fmul            v19.4s, v19.4s, v31.4s
        fmul            v17.4s, v17.4s, v31.4s
        fmul            v18.4s, v18.4s, v31.4s

        fmla            v23.4s, v10.4s, v21.4s      
        fmls            v25.4s, v14.4s, v21.4s      
        fmla            v22.4s, v8.4s, v19.4s      
        fmls            v24.4s, v12.4s, v19.4s      

        fmla            v26.4s, v9.4s, v18.4s      
        fmla            v27.4s, v11.4s, v17.4s      
        fmls            v20.4s, v13.4s, v18.4s      
        fmls            v16.4s, v15.4s, v17.4s      

        fsub            v18.4s, v23.4s, v25.4s
        fsub            v17.4s, v24.4s, v22.4s
        fsub            v19.4s, v20.4s, v26.4s
        fsub            v21.4s, v16.4s, v27.4s

        fadd            v22.4s, v24.4s, v22.4s
        fadd            v23.4s, v25.4s, v23.4s
        fadd            v24.4s, v20.4s, v26.4s
        fadd            v25.4s, v16.4s, v27.4s

        fmul            v17.4s, v17.4s, v31.4s
        fmul            v18.4s, v18.4s, v31.4s
        fmul            v19.4s, v19.4s, v31.4s
        fmul            v21.4s, v21.4s, v31.4s

        rev64           v22.4s, v22.4s
        rev64           v24.4s, v24.4s
        rev64           v23.4s, v23.4s
        rev64           v25.4s, v25.4s

        fsub            v8.4s, v0.4s, v22.4s
        fsub            v9.4s, v1.4s, v24.4s
        fsub            v10.4s, v2.4s, v17.4s
        fsub            v11.4s, v3.4s, v19.4s

        fsub            v12.4s, v4.4s, v23.4s
        fsub            v13.4s, v5.4s, v25.4s
        fadd            v14.4s, v6.4s, v18.4s
        fsub            v15.4s, v7.4s, v21.4s

        fadd            v0.4s, v0.4s, v22.4s
        fadd            v1.4s, v1.4s, v24.4s
        fadd            v2.4s, v2.4s, v17.4s
        fadd            v3.4s, v3.4s, v19.4s

        fadd            v4.4s, v4.4s, v23.4s
        fsub            v6.4s, v6.4s, v18.4s      
        fadd            v5.4s, v5.4s, v25.4s      
        fadd            v7.4s, v7.4s, v21.4s

        zip1            v16.2d, v0.2d, v4.2d
        zip2            v17.2d, v0.2d, v4.2d
        zip1            v18.2d, v1.2d, v5.2d
        zip2            v19.2d, v1.2d, v5.2d

        zip1            v20.2d, v2.2d, v6.2d
        zip2            v21.2d, v2.2d, v6.2d
        zip1            v22.2d, v3.2d, v7.2d
        zip2            v23.2d, v3.2d, v7.2d

        ldp             q0,  q1,  [x10, #(1*32)]
        ldp             q4,  q5,  [x10, #(3*32)]
        ldp             q2,  q3,  [x11, #(1*32)]
        ldp             q6,  q7,  [x11, #(3*32)]

        st1             { v16.4s, v17.4s, v18.4s, v19.4s }, [x10], #64
        st1             { v20.4s, v21.4s, v22.4s, v23.4s }, [x11], #64

        zip1            v20.2d, v8.2d, v12.2d
        zip2            v21.2d, v8.2d, v12.2d
        zip1            v22.2d, v9.2d, v13.2d
        zip2            v23.2d, v9.2d, v13.2d
        zip1            v24.2d, v10.2d, v14.2d
        zip2            v25.2d, v10.2d, v14.2d
        zip1            v26.2d, v11.2d, v15.2d
        zip2            v27.2d, v11.2d, v15.2d

        ldp             q8,  q9,  [x12, #(1*32)]
        ldp             q10, q11, [x12, #(3*32)]
        ldp             q12, q13, [x13, #(1*32)]
        ldp             q14, q15, [x13, #(3*32)]

        st1             { v20.4s, v21.4s, v22.4s, v23.4s }, [x12], #64
        st1             { v24.4s, v25.4s, v26.4s, v27.4s }, [x13], #64


        ld1             { v24.4s, v25.4s }, [x8], x9
        ld1             { v16.4s, v17.4s }, [x7], #32

        ext8             v18.16b, v24.16b, v24.16b, #8
        ext8             v19.16b, v25.16b, v25.16b, #8

        trn1            v20.4s, v16.4s, v16.4s      
        trn2            v16.4s, v16.4s, v16.4s      
        trn1            v21.4s, v17.4s, v17.4s      
        trn2            v17.4s, v17.4s, v17.4s      

        rev64           v22.4s, v8.4s                 
        rev64           v23.4s, v10.4s                 
        rev64           v24.4s, v12.4s                 
        rev64           v25.4s, v14.4s                 

        fmul            v22.4s, v22.4s, v20.4s      
        fmul            v23.4s, v23.4s, v16.4s      
        fmul            v24.4s, v24.4s, v20.4s      
        fmul            v25.4s, v25.4s, v16.4s      

        rev64           v26.4s, v9.4s                 
        rev64           v27.4s, v11.4s                 
        rev64           v20.4s, v13.4s                 
        rev64           v16.4s, v15.4s                 

        fmul            v26.4s, v26.4s, v21.4s      
        fmul            v27.4s, v27.4s, v17.4s      
        fmul            v20.4s, v20.4s, v21.4s      
        fmul            v16.4s, v16.4s, v17.4s      

        trn1            v21.4s, v19.4s, v19.4s      
        trn2            v19.4s, v19.4s, v19.4s      
        trn1            v17.4s, v18.4s, v18.4s      
        trn2            v18.4s, v18.4s, v18.4s      

        fmul            v21.4s, v21.4s, v31.4s
        fmul            v19.4s, v19.4s, v31.4s
        fmul            v17.4s, v17.4s, v31.4s
        fmul            v18.4s, v18.4s, v31.4s

        fmla            v23.4s, v10.4s, v21.4s      
        fmls            v25.4s, v14.4s, v21.4s      
        fmla            v22.4s, v8.4s, v19.4s      
        fmls            v24.4s, v12.4s, v19.4s      

        fmla            v26.4s, v9.4s, v18.4s      
        fmla            v27.4s, v11.4s, v17.4s      
        fmls            v20.4s, v13.4s, v18.4s      
        fmls            v16.4s, v15.4s, v17.4s      

        fsub            v18.4s, v23.4s, v25.4s
        fsub            v17.4s, v24.4s, v22.4s
        fsub            v19.4s, v20.4s, v26.4s
        fsub            v21.4s, v16.4s, v27.4s

        fadd            v22.4s, v24.4s, v22.4s
        fadd            v23.4s, v25.4s, v23.4s
        fadd            v24.4s, v20.4s, v26.4s
        fadd            v25.4s, v16.4s, v27.4s

        fmul            v17.4s, v17.4s, v31.4s
        fmul            v18.4s, v18.4s, v31.4s
        fmul            v19.4s, v19.4s, v31.4s
        fmul            v21.4s, v21.4s, v31.4s

        rev64           v22.4s, v22.4s
        rev64           v24.4s, v24.4s
        rev64           v23.4s, v23.4s
        rev64           v25.4s, v25.4s

        fsub            v8.4s, v0.4s, v22.4s
        fsub            v9.4s, v1.4s, v24.4s
        fsub            v10.4s, v2.4s, v17.4s
        fsub            v11.4s, v3.4s, v19.4s

        fsub            v12.4s, v4.4s, v23.4s
        fsub            v13.4s, v5.4s, v25.4s
        fadd            v14.4s, v6.4s, v18.4s
        fsub            v15.4s, v7.4s, v21.4s

        fadd            v0.4s, v0.4s, v22.4s
        fadd            v1.4s, v1.4s, v24.4s
        fadd            v2.4s, v2.4s, v17.4s
        fadd            v3.4s, v3.4s, v19.4s

        fadd            v4.4s, v4.4s, v23.4s
        fsub            v6.4s, v6.4s, v18.4s      
        fadd            v5.4s, v5.4s, v25.4s      
        fadd            v7.4s, v7.4s, v21.4s

        zip1            v16.2d, v0.2d, v4.2d
        zip2            v17.2d, v0.2d, v4.2d
        zip1            v18.2d, v1.2d, v5.2d
        zip2            v19.2d, v1.2d, v5.2d
        st1             { v16.4s, v17.4s, v18.4s, v19.4s }, [x10]

        zip1            v16.2d, v2.2d, v6.2d
        zip2            v17.2d, v2.2d, v6.2d
        zip1            v18.2d, v3.2d, v7.2d
        zip2            v19.2d, v3.2d, v7.2d
        st1             { v16.4s, v17.4s, v18.4s, v19.4s }, [x11]

        zip1            v20.2d, v8.2d, v12.2d
        zip2            v21.2d, v8.2d, v12.2d
        zip1            v22.2d, v9.2d, v13.2d
        zip2            v23.2d, v9.2d, v13.2d
        st1             { v20.4s, v21.4s, v22.4s, v23.4s }, [x12]

        zip1            v24.2d, v10.2d, v14.2d
        zip2            v25.2d, v10.2d, v14.2d
        zip1            v26.2d, v11.2d, v15.2d
        zip2            v27.2d, v11.2d, v15.2d
        st1             { v24.4s, v25.4s, v26.4s, v27.4s }, [x13]
        add             x1, x1, #32*8
        subs            w19, w19, #32*4
        bgt            temp_label_44

        ldp             x19, x20, [sp, #16]
        ldp             d14, d15, [sp, #16*2]
        ldp             d12, d13, [sp, #16*3]
        ldp             d10, d11, [sp, #16*4]
        ldp             d8,  d9,  [sp, #16*5]
        ldp             x21, x22, [sp], #16*6

        ret

temp_label_45
temp_label_3
 
        umov             x10, v23.d[0]
        umov             x11, v23.d[1]


        rev64           v28.4s, v2.4s              
        rev64           v29.4s, v16.4s              
        trn2            v30.4s, v27.4s, v27.4s   
        fmul            v30.4s, v30.4s, v31.4s
        fmul            v16.4s, v16.4s, v30.4s
        fmul            v2.4s, v2.4s, v30.4s   
        trn1            v30.4s, v24.4s, v24.4s   
        fmul            v29.4s, v29.4s, v30.4s   
        fmla            v2.4s, v28.4s, v30.4s   
        fsub            v29.4s, v29.4s, v16.4s   

        rev64           v30.4s, v3.4s              
        rev64           v16.4s, v17.4s              

        trn2            v28.4s, v26.4s, v26.4s   
        fmul            v28.4s, v28.4s, v31.4s

        fmul            v3.4s, v3.4s, v28.4s   
        fmul            v17.4s, v17.4s, v28.4s

        trn1            v28.4s, v25.4s, v25.4s   
        fmul            v16.4s, v16.4s, v28.4s   
        fmla            v3.4s, v30.4s, v28.4s   
        fsub            v16.4s, v16.4s, v17.4s   

        fsub            v28.4s, v29.4s, v2.4s
        fadd            v2.4s, v29.4s, v2.4s
        fadd            v30.4s, v16.4s, v3.4s
        fsub            v29.4s, v16.4s, v3.4s

        fmul            v28.4s, v28.4s, v31.4s
        fmul            v29.4s, v29.4s, v31.4s

        rev64           v30.4s, v30.4s
        rev64           v2.4s, v2.4s

        fsub            v3.4s, v1.4s, v30.4s
        fsub            v16.4s, v8.4s, v28.4s
        fsub            v17.4s, v9.4s, v29.4s

        fadd            v1.4s, v1.4s, v30.4s
        fadd            v8.4s, v8.4s, v28.4s
        fadd            v9.4s, v9.4s, v29.4s

        orr v29.16b, v2.16b, v2.16b

        fsub            v2.4s, v0.4s, v29.4s
        fadd            v0.4s, v0.4s, v29.4s


        trn2            v24.4s, v24.4s, v24.4s   
        trn2            v25.4s, v25.4s, v25.4s   
        trn1            v26.4s, v26.4s, v26.4s   
        trn1            v27.4s, v27.4s, v27.4s   

        fmul            v26.4s, v26.4s, v31.4s
        fmul            v27.4s, v27.4s, v31.4s

        rev64           v28.4s, v6.4s              
        rev64           v29.4s, v20.4s              
        rev64           v30.4s, v7.4s              
        rev64           v23.4s, v21.4s              

        fmul            v6.4s, v6.4s, v27.4s   
        fmul            v7.4s, v7.4s, v26.4s   
        fmla            v6.4s, v28.4s, v24.4s   
        fmla            v7.4s, v30.4s, v25.4s   

        fmul            v29.4s, v29.4s, v24.4s   
        fmul            v23.4s, v23.4s, v25.4s   
        fmls            v29.4s, v20.4s, v27.4s   
        fmls            v23.4s, v21.4s, v26.4s   

        fsub            v28.4s, v29.4s, v6.4s
        fadd            v29.4s, v29.4s, v6.4s
        fadd            v30.4s, v23.4s, v7.4s
        fsub            v23.4s, v23.4s, v7.4s

        fmul            v28.4s, v28.4s, v31.4s
        fmul            v23.4s, v23.4s, v31.4s

        rev64           v29.4s, v29.4s
        rev64           v30.4s, v30.4s

        fsub            v6.4s, v4.4s, v29.4s
        fadd            v20.4s, v5.4s, v30.4s
        fsub            v7.4s, v12.4s, v28.4s
        fadd            v21.4s, v13.4s, v23.4s

        fadd            v4.4s, v4.4s, v29.4s
        fsub            v5.4s, v5.4s, v30.4s   
        fadd            v12.4s, v12.4s, v28.4s   
        fsub            v13.4s, v13.4s, v23.4s

        zip1            v23.2d, v0.2d, v4.2d
        zip2            v24.2d, v0.2d, v4.2d
        zip1            v25.2d, v1.2d, v20.2d
        zip2            v26.2d, v1.2d, v20.2d

        zip1            v27.2d, v8.2d, v12.2d
        zip2            v28.2d, v8.2d, v12.2d
        zip1            v29.2d, v9.2d, v21.2d
        zip2            v30.2d, v9.2d, v21.2d

        orr v20.16b, v5.16b, v5.16b
        orr v21.16b, v7.16b, v7.16b
        mov             x12, x1
        add             x13, x1, #32* 4
        add             x14, x1, #32* 8
        add             x15, x1, #32*12

        zip1            v4.2d,  v2.2d, v6.2d
        zip2            v5.2d,  v2.2d, v6.2d
        zip1            v6.2d,  v3.2d, v20.2d
        zip2            v7.2d,  v3.2d, v20.2d

        zip1            v0.2d, v16.2d, v21.2d
        zip2            v1.2d, v16.2d, v21.2d
        zip1            v2.2d, v17.2d, v13.2d
        zip2            v3.2d, v17.2d, v13.2d

        
        ldp             q8,  q9, [x1, #32*1]
        ldp             q12, q13, [x1, #32*5]

        st1             { v23.4s, v24.4s, v25.4s, v26.4s }, [x12], #64  
        st1             { v27.4s, v28.4s, v29.4s, v30.4s }, [x13], #64  
        st1             {  v4.4s,  v5.4s,  v6.4s,  v7.4s }, [x14], #64  
        st1             {  v0.4s,  v1.4s,  v2.4s,  v3.4s }, [x15], #64  

        mov             v23.d[0], x10
        mov             v23.d[1], x11

        ldp             q6,  q7, [x1, #32*3]
        ldp             q16, q17, [x1, #32*7]


        ld1             { v0.4s, v1.4s }, [x8], x9
        ld1             { v24.4s, v25.4s }, [x7], #32

        ext8             v26.16b, v0.16b, v0.16b, #8
        ext8             v27.16b, v1.16b, v1.16b, #8

        trn1            v28.4s, v24.4s, v24.4s      
        trn2            v24.4s, v24.4s, v24.4s      
        trn1            v29.4s, v25.4s, v25.4s      
        trn2            v25.4s, v25.4s, v25.4s      

        rev64           v30.4s, v10.4s                 
        rev64           v4.4s, v14.4s                 
        rev64           v0.4s, v18.4s                 
        rev64           v1.4s, v22.4s                 

        fmul            v30.4s, v30.4s, v28.4s      
        fmul            v4.4s, v4.4s, v24.4s      
        fmul            v0.4s, v0.4s, v28.4s      
        fmul            v1.4s, v1.4s, v24.4s      

        rev64           v5.4s, v11.4s                 
        rev64           v20.4s, v15.4s                 
        rev64           v28.4s, v19.4s                 
        rev64           v24.4s, v23.4s                 

        fmul            v5.4s, v5.4s, v29.4s      
        fmul            v20.4s, v20.4s, v25.4s      
        fmul            v28.4s, v28.4s, v29.4s      
        fmul            v24.4s, v24.4s, v25.4s      

        trn1            v29.4s, v27.4s, v27.4s      
        trn2            v27.4s, v27.4s, v27.4s      
        trn1            v25.4s, v26.4s, v26.4s      
        trn2            v26.4s, v26.4s, v26.4s      

        fmul            v29.4s, v29.4s, v31.4s
        fmul            v27.4s, v27.4s, v31.4s
        fmul            v25.4s, v25.4s, v31.4s
        fmul            v26.4s, v26.4s, v31.4s

        fmla            v4.4s, v14.4s, v29.4s      
        fmls            v1.4s, v22.4s, v29.4s      
        fmla            v30.4s, v10.4s, v27.4s      
        fmls            v0.4s, v18.4s, v27.4s      

        fmla            v5.4s, v11.4s, v26.4s      
        fmla            v20.4s, v15.4s, v25.4s      
        fmls            v28.4s, v19.4s, v26.4s      
        fmls            v24.4s, v23.4s, v25.4s      

        fsub            v26.4s, v4.4s, v1.4s
        fsub            v25.4s, v0.4s, v30.4s
        fsub            v27.4s, v28.4s, v5.4s
        fsub            v29.4s, v24.4s, v20.4s

        fadd            v30.4s, v0.4s, v30.4s
        fadd            v4.4s, v1.4s, v4.4s
        fadd            v0.4s, v28.4s, v5.4s
        fadd            v1.4s, v24.4s, v20.4s

        fmul            v25.4s, v25.4s, v31.4s
        fmul            v26.4s, v26.4s, v31.4s
        fmul            v27.4s, v27.4s, v31.4s
        fmul            v29.4s, v29.4s, v31.4s

        rev64           v30.4s, v30.4s
        rev64           v0.4s, v0.4s
        rev64           v4.4s, v4.4s
        rev64           v1.4s, v1.4s

        fsub            v10.4s, v8.4s, v30.4s
        fsub            v11.4s, v9.4s, v0.4s
        fsub            v14.4s, v12.4s, v25.4s
        fsub            v15.4s, v13.4s, v27.4s

        fsub            v18.4s, v6.4s, v4.4s
        fsub            v19.4s, v7.4s, v1.4s
        fadd            v22.4s, v16.4s, v26.4s
        fsub            v23.4s, v17.4s, v29.4s

        fadd            v8.4s, v8.4s, v30.4s
        fadd            v9.4s, v9.4s, v0.4s
        fadd            v12.4s, v12.4s, v25.4s
        fadd            v13.4s, v13.4s, v27.4s

        fadd            v6.4s, v6.4s, v4.4s
        fsub            v16.4s, v16.4s, v26.4s      
        fadd            v7.4s, v7.4s, v1.4s      
        fadd            v17.4s, v17.4s, v29.4s

        zip1            v0.2d, v8.2d, v6.2d
        zip2            v1.2d, v8.2d, v6.2d
        zip1            v2.2d, v9.2d, v7.2d
        zip2            v3.2d, v9.2d, v7.2d
        st1             {  v0.4s,  v1.4s,  v2.4s,  v3.4s }, [x12]

        zip1            v4.2d, v12.2d, v16.2d
        zip2            v5.2d, v12.2d, v16.2d
        zip1            v6.2d, v13.2d, v17.2d
        zip2            v7.2d, v13.2d, v17.2d
        st1             {  v4.4s,  v5.4s,  v6.4s,  v7.4s }, [x13]

        zip1            v0.2d, v10.2d, v18.2d
        zip2            v1.2d, v10.2d, v18.2d
        zip1            v2.2d, v11.2d, v19.2d
        zip2            v3.2d, v11.2d, v19.2d
        st1             {  v0.4s,  v1.4s,  v2.4s,  v3.4s }, [x14]

        zip1            v4.2d, v14.2d, v22.2d
        zip2            v5.2d, v14.2d, v22.2d
        zip1            v6.2d, v15.2d, v23.2d
        zip2            v7.2d, v15.2d, v23.2d
        st1             {  v4.4s,  v5.4s,  v6.4s,  v7.4s }, [x15]

        ldp             x19, x20, [sp, #16]
        ldp             d14, d15, [sp, #16*2]
        ldp             d12, d13, [sp, #16*3]
        ldp             d10, d11, [sp, #16*4]
        ldp             d8,  d9,  [sp, #16*5]
        ldp             x21, x22, [sp], #16*6

        ret


        AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN
        ALIGN 16

        EXPORT ff_tx_fft_sr_ns_float_neon





ff_tx_fft_sr_ns_float_neon
        










        stp             x21, x22, [sp, #-16*6]!
        stp             d8,  d9,  [sp, #16*5]
        stp             d10, d11, [sp, #16*4]
        stp             d12, d13, [sp, #16*3]
        stp             d14, d15, [sp, #16*2]
        stp             x19, x20, [sp, #16]

        ldr             w19, [x0, #0] 
        mov             w20, w19      











        adrp            x5, subadd
        add             x5, x5, subadd










        ld1             { v31.4s }, [x5]

temp_label_46

        ldr             w5, =(32 - 4*7)










        adrp            x7, ff_tx_tab_32_float
        add             x7, x7, ff_tx_tab_32_float










        add             x8, x7, x5
        mov             x9, #-32


        ld1             { v0.4s, v1.4s, v2.4s, v3.4s }, [x2], #64
        ldp             q4, q6, [x2]
        ldp             q5, q7, [x2, #32]
        add             x2, x2, #64
        ld1             { v8.4s, v9.4s, v10.4s, v11.4s }, [x2], #64
        ld1             { v12.4s, v13.4s, v14.4s, v15.4s }, [x2], #64












        adrp            x5, tab_8pt
        add             x5, x5, tab_8pt











        fadd            v19.4s, v13.4s, v15.4s             
        fadd            v17.4s, v9.4s, v11.4s             
        fadd            v18.4s, v12.4s, v14.4s             
        fadd            v16.4s, v8.4s, v10.4s             

        ld1             { v23.4s }, [x5]

        ext8             v22.16b, v19.16b, v19.16b, #12
        ext8             v21.16b, v17.16b, v17.16b, #12

        rev64           v22.4s,  v22.4s
        rev64           v21.4s,  v21.4s

        ext8             v19.16b, v18.16b, v22.16b, #8
        ext8             v17.16b, v16.16b, v21.16b, #8

        mov             v18.d[1], v22.d[1]
        mov             v21.d[0], v16.d[0]

        fadd            v22.4s, v18.4s, v19.4s                   
        fsub            v19.4s, v18.4s, v19.4s                   
        fsub            v18.4s, v21.4s, v17.4s                   
        fadd            v16.4s, v21.4s, v17.4s                   

        fsub            v8.4s, v8.4s, v10.4s          
        fsub            v20.4s, v9.4s, v11.4s             
        fsub            v12.4s, v12.4s, v14.4s          
        fsub            v21.4s, v13.4s, v15.4s             

        rev64           v24.4s, v31.4s                           
        zip1            v17.2d, v16.2d, v18.2d                   
        zip1            v9.2d, v22.2d, v19.2d                

        rev64           v18.4s, v18.4s                           
        rev64           v19.4s, v19.4s                           

        zip2            v16.2d, v16.2d, v18.2d                   
        zip2            v13.2d, v22.2d, v19.2d                

        dup             v10.2d, v8.d[0]                   
        dup             v11.2d, v8.d[1]                   
        dup             v14.2d, v12.d[0]                   
        dup             v15.2d, v12.d[1]                   

        fadd            v12.4s, v9.4s, v13.4s          
        fsub            v13.4s, v9.4s, v13.4s          
        fadd            v8.4s, v17.4s, v16.4s                
        fsub            v9.4s, v17.4s, v16.4s                

        ext8             v24.16b, v31.16b, v24.16b, #8            
        rev64           v11.4s, v11.4s                     
        rev64           v15.4s, v15.4s                     

        fmul            v19.4s, v20.4s, v23.4s                   
        fmul            v21.4s, v21.4s, v23.4s                   

        rev64           v20.4s, v19.4s                           
        rev64           v18.4s, v21.4s                           

        fmls            v10.4s, v11.4s, v24.4s             
        fmls            v14.4s, v15.4s, v24.4s             

        fmla            v19.4s, v20.4s, v31.4s                   
        fmla            v21.4s, v18.4s, v31.4s                   

        rev64           v20.4s, v19.4s                           
        rev64           v18.4s, v21.4s                           
        ext8             v20.16b, v20.16b, v20.16b, #8            
        ext8             v18.16b, v18.16b, v18.16b, #8            

        fmla            v20.4s, v19.4s, v31.4s                   
        fmla            v18.4s, v21.4s, v31.4s                   

        fadd            v11.4s, v10.4s, v20.4s             
        fadd            v15.4s, v14.4s, v18.4s             
        fsub            v10.4s, v10.4s, v20.4s             
        fsub            v14.4s, v14.4s, v18.4s             












        adrp            x5, tab_8pt
        add             x5, x5, tab_8pt











        fsub            v17.4s, v1.4s, v3.4s             
        fadd            v3.4s, v1.4s, v3.4s             
        fsub            v16.4s, v0.4s, v2.4s             
        fadd            v2.4s, v0.4s, v2.4s             

        ld1             { v21.4s }, [x5]

        ext8             v20.16b, v3.16b, v3.16b, #12
        rev64           v20.4s,  v20.4s

        ext8             v18.16b, v2.16b, v20.16b, #8      
        mov             v2.d[1], v20.d[1]                    

        fsub            v1.4s, v2.4s, v18.4s             
        fadd            v18.4s, v2.4s, v18.4s             

        rev64           v22.4s, v31.4s                           
        dup             v2.2d, v16.d[0]                      
        dup             v3.2d, v16.d[1]                      

        rev64           v20.4s, v1.4s                        
        rev64           v3.4s, v3.4s                        

        ext8             v22.16b, v31.16b, v22.16b, #8         
        zip1            v19.2d, v18.2d, v1.2d             
        zip2            v18.2d, v18.2d, v20.2d             

        fadd            v0.4s, v19.4s, v18.4s             
        fsub            v1.4s, v19.4s, v18.4s             

        fmul            v17.4s, v17.4s, v21.4s             
        fmls            v2.4s, v3.4s, v22.4s             

        rev64           v20.4s, v17.4s                        
        fmla            v17.4s, v20.4s, v31.4s                

        rev64           v20.4s, v17.4s                        
        ext8             v20.16b, v20.16b, v20.16b, #8      

        fmla            v20.4s, v17.4s, v31.4s                

        fadd            v3.4s, v2.4s, v20.4s             
        fsub            v2.4s, v2.4s, v20.4s             

        fadd            v16.4s, v4.4s, v6.4s                     
        fadd            v18.4s, v5.4s, v7.4s                     
        fsub            v4.4s, v4.4s, v6.4s                     
        fsub            v5.4s, v5.4s, v7.4s                     

        rev64           v20.4s, v4.4s
        rev64           v21.4s, v5.4s

        zip2            v6.2d, v16.2d, v20.2d                     
        zip2            v7.2d, v18.2d, v21.2d                     

        fneg            v19.4s, v20.4s
        fneg            v20.4s, v21.4s

        mov             v6.s[3], v19.s[3]
        mov             v7.s[3], v20.s[3]

        zip1            v17.2d, v16.2d, v4.2d                     
        zip1            v19.2d, v18.2d, v5.2d                     

        fsub            v20.4s, v17.4s, v6.4s                     
        fadd            v21.4s, v17.4s, v6.4s                     
        fsub            v18.4s, v19.4s, v7.4s                     
        fadd            v19.4s, v19.4s, v7.4s                     

        zip1            v4.2d, v21.2d, v20.2d                     
        zip2            v6.2d, v21.2d, v20.2d                     
        zip1            v5.2d, v19.2d, v18.2d                     
        zip2            v7.2d, v19.2d, v18.2d                     











        adrp            x5, tab_16pt
        add             x5, x5, tab_16pt











        rev64           v16.4s, v4.4s                     
        rev64           v17.4s, v5.4s                     

        ins             v16.d[0], xzr
        ins             v17.d[0], xzr

        ld1             { v20.4s, v21.4s, v22.4s }, [x5]
        

        fmla            v5.4s, v17.4s, v31.4s             
        fmls            v4.4s, v16.4s, v31.4s             

        fmul            v18.4s, v6.4s, v20.4s
        fmul            v19.4s, v7.4s, v20.4s

        rev64           v7.4s, v7.4s
        rev64           v6.4s, v6.4s

        fmla            v19.4s, v7.4s, v21.4s          
        fmls            v18.4s, v6.4s, v21.4s          

        fmul            v17.4s, v5.4s, v22.4s          
        fmul            v16.4s, v4.4s, v22.4s          

        orr v6.16b, v19.16b, v19.16b
        orr v5.16b, v17.16b, v17.16b

        fsub            v19.4s, v19.4s, v18.4s          
        fsub            v17.4s, v17.4s, v16.4s          

        fadd            v18.4s, v18.4s, v6.4s          
        rev64           v19.4s, v19.4s
        fadd            v16.4s, v16.4s, v5.4s          
        rev64           v17.4s, v17.4s

        fmul            v18.4s, v18.4s, v31.4s
        fmul            v17.4s, v17.4s, v31.4s

        fadd            v7.4s, v3.4s, v19.4s
        fsub            v5.4s, v3.4s, v19.4s
        fsub            v6.4s, v2.4s, v18.4s
        fadd            v4.4s, v2.4s, v18.4s

        fsub            v2.4s, v0.4s, v16.4s
        fadd            v0.4s, v0.4s, v16.4s
        fsub            v3.4s, v1.4s, v17.4s
        fadd            v1.4s, v1.4s, v17.4s


        ld1             { v24.4s, v25.4s }, [x8], x9
        ld1             { v16.4s, v17.4s }, [x7], #32

        ext8             v18.16b, v24.16b, v24.16b, #8
        ext8             v19.16b, v25.16b, v25.16b, #8

        trn1            v20.4s, v16.4s, v16.4s      
        trn2            v16.4s, v16.4s, v16.4s      
        trn1            v21.4s, v17.4s, v17.4s      
        trn2            v17.4s, v17.4s, v17.4s      

        rev64           v22.4s, v8.4s                 
        rev64           v23.4s, v10.4s                 
        rev64           v24.4s, v12.4s                 
        rev64           v25.4s, v14.4s                 

        fmul            v22.4s, v22.4s, v20.4s      
        fmul            v23.4s, v23.4s, v16.4s      
        fmul            v24.4s, v24.4s, v20.4s      
        fmul            v25.4s, v25.4s, v16.4s      

        rev64           v26.4s, v9.4s                 
        rev64           v27.4s, v11.4s                 
        rev64           v20.4s, v13.4s                 
        rev64           v16.4s, v15.4s                 

        fmul            v26.4s, v26.4s, v21.4s      
        fmul            v27.4s, v27.4s, v17.4s      
        fmul            v20.4s, v20.4s, v21.4s      
        fmul            v16.4s, v16.4s, v17.4s      

        trn1            v21.4s, v19.4s, v19.4s      
        trn2            v19.4s, v19.4s, v19.4s      
        trn1            v17.4s, v18.4s, v18.4s      
        trn2            v18.4s, v18.4s, v18.4s      

        fmul            v21.4s, v21.4s, v31.4s
        fmul            v19.4s, v19.4s, v31.4s
        fmul            v17.4s, v17.4s, v31.4s
        fmul            v18.4s, v18.4s, v31.4s

        fmla            v23.4s, v10.4s, v21.4s      
        fmls            v25.4s, v14.4s, v21.4s      
        fmla            v22.4s, v8.4s, v19.4s      
        fmls            v24.4s, v12.4s, v19.4s      

        fmla            v26.4s, v9.4s, v18.4s      
        fmla            v27.4s, v11.4s, v17.4s      
        fmls            v20.4s, v13.4s, v18.4s      
        fmls            v16.4s, v15.4s, v17.4s      

        fsub            v18.4s, v23.4s, v25.4s
        fsub            v17.4s, v24.4s, v22.4s
        fsub            v19.4s, v20.4s, v26.4s
        fsub            v21.4s, v16.4s, v27.4s

        fadd            v22.4s, v24.4s, v22.4s
        fadd            v23.4s, v25.4s, v23.4s
        fadd            v24.4s, v20.4s, v26.4s
        fadd            v25.4s, v16.4s, v27.4s

        fmul            v17.4s, v17.4s, v31.4s
        fmul            v18.4s, v18.4s, v31.4s
        fmul            v19.4s, v19.4s, v31.4s
        fmul            v21.4s, v21.4s, v31.4s

        rev64           v22.4s, v22.4s
        rev64           v24.4s, v24.4s
        rev64           v23.4s, v23.4s
        rev64           v25.4s, v25.4s

        fsub            v8.4s, v0.4s, v22.4s
        fsub            v9.4s, v1.4s, v24.4s
        fsub            v10.4s, v2.4s, v17.4s
        fsub            v11.4s, v3.4s, v19.4s

        fsub            v12.4s, v4.4s, v23.4s
        fsub            v13.4s, v5.4s, v25.4s
        fadd            v14.4s, v6.4s, v18.4s
        fsub            v15.4s, v7.4s, v21.4s

        fadd            v0.4s, v0.4s, v22.4s
        fadd            v1.4s, v1.4s, v24.4s
        fadd            v2.4s, v2.4s, v17.4s
        fadd            v3.4s, v3.4s, v19.4s

        fadd            v4.4s, v4.4s, v23.4s
        fsub            v6.4s, v6.4s, v18.4s      
        fadd            v5.4s, v5.4s, v25.4s      
        fadd            v7.4s, v7.4s, v21.4s

        stp             q2,  q3,  [x1, #32*1]
        stp             q6,  q7,  [x1, #32*3]
        stp             q10, q11, [x1, #32*5]
        stp             q14, q15, [x1, #32*7]

        cmp             w20, #32
        bgt            temp_label_47

        stp             q0,  q1,  [x1, #32*0]
        stp             q4,  q5,  [x1, #32*2]
        stp             q8,  q9,  [x1, #32*4]
        stp             q12, q13, [x1, #32*6]

        ret
temp_label_48
temp_label_47

        ldr             w5, =(64 - 4*7)










        adrp            x7, ff_tx_tab_64_float
        add             x7, x7, ff_tx_tab_64_float










        add             x8, x7, x5
        mov             x9, #-32

        mov             x21, #2*64
        add             x22, x21, x21, lsl #1

        ldp             q2, q3, [x2]
        ldp             q10, q11, [x2, #32]
        add             x2, x2, #64
        ldp             q6, q14, [x2]
        ldp             q7, q15, [x2, #32]
        add             x2, x2, #64













        adrp            x5, tab_8pt
        add             x5, x5, tab_8pt











        fsub            v17.4s, v3.4s, v11.4s             
        fadd            v11.4s, v3.4s, v11.4s             
        fsub            v16.4s, v2.4s, v10.4s             
        fadd            v10.4s, v2.4s, v10.4s             

        ld1             { v21.4s }, [x5]

        ext8             v20.16b, v11.16b, v11.16b, #12
        rev64           v20.4s,  v20.4s

        ext8             v18.16b, v10.16b, v20.16b, #8      
        mov             v10.d[1], v20.d[1]                    

        fsub            v3.4s, v10.4s, v18.4s             
        fadd            v18.4s, v10.4s, v18.4s             

        rev64           v22.4s, v31.4s                           
        dup             v10.2d, v16.d[0]                      
        dup             v11.2d, v16.d[1]                      

        rev64           v20.4s, v3.4s                        
        rev64           v11.4s, v11.4s                        

        ext8             v22.16b, v31.16b, v22.16b, #8         
        zip1            v19.2d, v18.2d, v3.2d             
        zip2            v18.2d, v18.2d, v20.2d             

        fadd            v2.4s, v19.4s, v18.4s             
        fsub            v3.4s, v19.4s, v18.4s             

        fmul            v17.4s, v17.4s, v21.4s             
        fmls            v10.4s, v11.4s, v22.4s             

        rev64           v20.4s, v17.4s                        
        fmla            v17.4s, v20.4s, v31.4s                

        rev64           v20.4s, v17.4s                        
        ext8             v20.16b, v20.16b, v20.16b, #8      

        fmla            v20.4s, v17.4s, v31.4s                

        fadd            v11.4s, v10.4s, v20.4s             
        fsub            v10.4s, v10.4s, v20.4s             

        fadd            v16.4s, v6.4s, v14.4s                     
        fadd            v18.4s, v7.4s, v15.4s                     
        fsub            v6.4s, v6.4s, v14.4s                     
        fsub            v7.4s, v7.4s, v15.4s                     

        rev64           v20.4s, v6.4s
        rev64           v21.4s, v7.4s

        zip2            v14.2d, v16.2d, v20.2d                     
        zip2            v15.2d, v18.2d, v21.2d                     

        fneg            v19.4s, v20.4s
        fneg            v20.4s, v21.4s

        mov             v14.s[3], v19.s[3]
        mov             v15.s[3], v20.s[3]

        zip1            v17.2d, v16.2d, v6.2d                     
        zip1            v19.2d, v18.2d, v7.2d                     

        fsub            v20.4s, v17.4s, v14.4s                     
        fadd            v21.4s, v17.4s, v14.4s                     
        fsub            v18.4s, v19.4s, v15.4s                     
        fadd            v19.4s, v19.4s, v15.4s                     

        zip1            v6.2d, v21.2d, v20.2d                     
        zip2            v14.2d, v21.2d, v20.2d                     
        zip1            v7.2d, v19.2d, v18.2d                     
        zip2            v15.2d, v19.2d, v18.2d                     











        adrp            x5, tab_16pt
        add             x5, x5, tab_16pt











        rev64           v16.4s, v6.4s                     
        rev64           v17.4s, v7.4s                     

        ins             v16.d[0], xzr
        ins             v17.d[0], xzr

        ld1             { v20.4s, v21.4s, v22.4s }, [x5]
        

        fmla            v7.4s, v17.4s, v31.4s             
        fmls            v6.4s, v16.4s, v31.4s             

        fmul            v18.4s, v14.4s, v20.4s
        fmul            v19.4s, v15.4s, v20.4s

        rev64           v15.4s, v15.4s
        rev64           v14.4s, v14.4s

        fmla            v19.4s, v15.4s, v21.4s          
        fmls            v18.4s, v14.4s, v21.4s          

        fmul            v17.4s, v7.4s, v22.4s          
        fmul            v16.4s, v6.4s, v22.4s          

        orr v14.16b, v19.16b, v19.16b
        orr v7.16b, v17.16b, v17.16b

        fsub            v19.4s, v19.4s, v18.4s          
        fsub            v17.4s, v17.4s, v16.4s          

        fadd            v18.4s, v18.4s, v14.4s          
        rev64           v19.4s, v19.4s
        fadd            v16.4s, v16.4s, v7.4s          
        rev64           v17.4s, v17.4s

        fmul            v18.4s, v18.4s, v31.4s
        fmul            v17.4s, v17.4s, v31.4s

        fadd            v15.4s, v11.4s, v19.4s
        fsub            v7.4s, v11.4s, v19.4s
        fsub            v14.4s, v10.4s, v18.4s
        fadd            v6.4s, v10.4s, v18.4s

        fsub            v10.4s, v2.4s, v16.4s
        fadd            v2.4s, v2.4s, v16.4s
        fsub            v11.4s, v3.4s, v17.4s
        fadd            v3.4s, v3.4s, v17.4s

        ld1             { v16.4s, v17.4s, v18.4s, v19.4s }, [x2], #64
        ldp             q20, q22, [x2]
        ldp             q21, q23, [x2, #32]
        add             x2, x2, #64













        adrp            x5, tab_8pt
        add             x5, x5, tab_8pt











        fsub            v25.4s, v17.4s, v19.4s             
        fadd            v19.4s, v17.4s, v19.4s             
        fsub            v24.4s, v16.4s, v18.4s             
        fadd            v18.4s, v16.4s, v18.4s             

        ld1             { v29.4s }, [x5]

        ext8             v28.16b, v19.16b, v19.16b, #12
        rev64           v28.4s,  v28.4s

        ext8             v26.16b, v18.16b, v28.16b, #8      
        mov             v18.d[1], v28.d[1]                    

        fsub            v17.4s, v18.4s, v26.4s             
        fadd            v26.4s, v18.4s, v26.4s             

        rev64           v30.4s, v31.4s                           
        dup             v18.2d, v24.d[0]                      
        dup             v19.2d, v24.d[1]                      

        rev64           v28.4s, v17.4s                        
        rev64           v19.4s, v19.4s                        

        ext8             v30.16b, v31.16b, v30.16b, #8         
        zip1            v27.2d, v26.2d, v17.2d             
        zip2            v26.2d, v26.2d, v28.2d             

        fadd            v16.4s, v27.4s, v26.4s             
        fsub            v17.4s, v27.4s, v26.4s             

        fmul            v25.4s, v25.4s, v29.4s             
        fmls            v18.4s, v19.4s, v30.4s             

        rev64           v28.4s, v25.4s                        
        fmla            v25.4s, v28.4s, v31.4s                

        rev64           v28.4s, v25.4s                        
        ext8             v28.16b, v28.16b, v28.16b, #8      

        fmla            v28.4s, v25.4s, v31.4s                

        fadd            v19.4s, v18.4s, v28.4s             
        fsub            v18.4s, v18.4s, v28.4s             

        fadd            v24.4s, v20.4s, v22.4s                     
        fadd            v26.4s, v21.4s, v23.4s                     
        fsub            v20.4s, v20.4s, v22.4s                     
        fsub            v21.4s, v21.4s, v23.4s                     

        rev64           v28.4s, v20.4s
        rev64           v29.4s, v21.4s

        zip2            v22.2d, v24.2d, v28.2d                     
        zip2            v23.2d, v26.2d, v29.2d                     

        fneg            v27.4s, v28.4s
        fneg            v28.4s, v29.4s

        mov             v22.s[3], v27.s[3]
        mov             v23.s[3], v28.s[3]

        zip1            v25.2d, v24.2d, v20.2d                     
        zip1            v27.2d, v26.2d, v21.2d                     

        fsub            v28.4s, v25.4s, v22.4s                     
        fadd            v29.4s, v25.4s, v22.4s                     
        fsub            v26.4s, v27.4s, v23.4s                     
        fadd            v27.4s, v27.4s, v23.4s                     

        zip1            v20.2d, v29.2d, v28.2d                     
        zip2            v22.2d, v29.2d, v28.2d                     
        zip1            v21.2d, v27.2d, v26.2d                     
        zip2            v23.2d, v27.2d, v26.2d                     











        adrp            x5, tab_16pt
        add             x5, x5, tab_16pt











        rev64           v24.4s, v20.4s                     
        rev64           v25.4s, v21.4s                     

        ins             v24.d[0], xzr
        ins             v25.d[0], xzr

        ld1             { v28.4s, v29.4s, v30.4s }, [x5]
        

        fmla            v21.4s, v25.4s, v31.4s             
        fmls            v20.4s, v24.4s, v31.4s             

        fmul            v26.4s, v22.4s, v28.4s
        fmul            v27.4s, v23.4s, v28.4s

        rev64           v23.4s, v23.4s
        rev64           v22.4s, v22.4s

        fmla            v27.4s, v23.4s, v29.4s          
        fmls            v26.4s, v22.4s, v29.4s          

        fmul            v25.4s, v21.4s, v30.4s          
        fmul            v24.4s, v20.4s, v30.4s          

        orr v22.16b, v27.16b, v27.16b
        orr v21.16b, v25.16b, v25.16b

        fsub            v27.4s, v27.4s, v26.4s          
        fsub            v25.4s, v25.4s, v24.4s          

        fadd            v26.4s, v26.4s, v22.4s          
        rev64           v27.4s, v27.4s
        fadd            v24.4s, v24.4s, v21.4s          
        rev64           v25.4s, v25.4s

        fmul            v26.4s, v26.4s, v31.4s
        fmul            v25.4s, v25.4s, v31.4s

        fadd            v23.4s, v19.4s, v27.4s
        fsub            v21.4s, v19.4s, v27.4s
        fsub            v22.4s, v18.4s, v26.4s
        fadd            v20.4s, v18.4s, v26.4s

        fsub            v18.4s, v16.4s, v24.4s
        fadd            v16.4s, v16.4s, v24.4s
        fsub            v19.4s, v17.4s, v25.4s
        fadd            v17.4s, v17.4s, v25.4s

        ld1             { v26.4s, v27.4s }, [x8], x9
        ldp             q24, q25, [x7], #32

        ext8             v26.16b, v26.16b, v26.16b, #8
        ext8             v27.16b, v27.16b, v27.16b, #8

        cmp             w19, #64
        beq            temp_label_49 

        
        

        rev64           v28.4s, v2.4s              
        rev64           v29.4s, v16.4s              
        trn2            v30.4s, v27.4s, v27.4s   
        fmul            v30.4s, v30.4s, v31.4s
        fmul            v16.4s, v16.4s, v30.4s
        fmul            v2.4s, v2.4s, v30.4s   
        trn1            v30.4s, v24.4s, v24.4s   
        fmul            v29.4s, v29.4s, v30.4s   
        fmla            v2.4s, v28.4s, v30.4s   
        fsub            v29.4s, v29.4s, v16.4s   

        rev64           v30.4s, v3.4s              
        rev64           v16.4s, v17.4s              

        trn2            v28.4s, v26.4s, v26.4s   
        fmul            v28.4s, v28.4s, v31.4s

        fmul            v3.4s, v3.4s, v28.4s   
        fmul            v17.4s, v17.4s, v28.4s

        trn1            v28.4s, v25.4s, v25.4s   
        fmul            v16.4s, v16.4s, v28.4s   
        fmla            v3.4s, v30.4s, v28.4s   
        fsub            v16.4s, v16.4s, v17.4s   

        fsub            v28.4s, v29.4s, v2.4s
        fadd            v2.4s, v29.4s, v2.4s
        fadd            v30.4s, v16.4s, v3.4s
        fsub            v29.4s, v16.4s, v3.4s

        fmul            v28.4s, v28.4s, v31.4s
        fmul            v29.4s, v29.4s, v31.4s

        rev64           v30.4s, v30.4s
        rev64           v2.4s, v2.4s

        fsub            v3.4s, v1.4s, v30.4s
        fsub            v16.4s, v8.4s, v28.4s
        fsub            v17.4s, v9.4s, v29.4s

        fadd            v1.4s, v1.4s, v30.4s
        fadd            v8.4s, v8.4s, v28.4s
        fadd            v9.4s, v9.4s, v29.4s

        orr v29.16b, v2.16b, v2.16b

        fsub            v2.4s, v0.4s, v29.4s
        fadd            v0.4s, v0.4s, v29.4s

        stp             q0,  q1,  [x1, #32* 0]
        stp             q8,  q9,  [x1, #32* 4]
        stp             q2,  q3,  [x1, #32* 8]
        stp             q16, q17, [x1, #32*12]


        trn2            v1.4s, v24.4s, v24.4s   
        trn2            v25.4s, v25.4s, v25.4s   
        trn1            v8.4s, v26.4s, v26.4s   
        trn1            v27.4s, v27.4s, v27.4s   

        fmul            v8.4s, v8.4s, v31.4s
        fmul            v27.4s, v27.4s, v31.4s

        rev64           v28.4s, v6.4s              
        rev64           v29.4s, v20.4s              
        rev64           v30.4s, v7.4s              
        rev64           v0.4s, v21.4s              

        fmul            v6.4s, v6.4s, v27.4s   
        fmul            v7.4s, v7.4s, v8.4s   
        fmla            v6.4s, v28.4s, v1.4s   
        fmla            v7.4s, v30.4s, v25.4s   

        fmul            v29.4s, v29.4s, v1.4s   
        fmul            v0.4s, v0.4s, v25.4s   
        fmls            v29.4s, v20.4s, v27.4s   
        fmls            v0.4s, v21.4s, v8.4s   

        fsub            v28.4s, v29.4s, v6.4s
        fadd            v29.4s, v29.4s, v6.4s
        fadd            v30.4s, v0.4s, v7.4s
        fsub            v0.4s, v0.4s, v7.4s

        fmul            v28.4s, v28.4s, v31.4s
        fmul            v0.4s, v0.4s, v31.4s

        rev64           v29.4s, v29.4s
        rev64           v30.4s, v30.4s

        fsub            v6.4s, v4.4s, v29.4s
        fadd            v20.4s, v5.4s, v30.4s
        fsub            v7.4s, v12.4s, v28.4s
        fadd            v21.4s, v13.4s, v0.4s

        fadd            v4.4s, v4.4s, v29.4s
        fsub            v5.4s, v5.4s, v30.4s   
        fadd            v12.4s, v12.4s, v28.4s   
        fsub            v13.4s, v13.4s, v0.4s

        stp             q4,  q20, [x1, #32* 2]
        stp             q12, q21, [x1, #32* 6]
        stp             q6,  q5,  [x1, #32*10]
        stp             q7,  q13, [x1, #32*14]

        ldp             q2,  q3,  [x1, #32*1]
        ldp             q6,  q7,  [x1, #32*3]
        ldp             q12, q13, [x1, #32*5]
        ldp             q16, q17, [x1, #32*7]


        ld1             { v0.4s, v1.4s }, [x8], x9
        ld1             { v24.4s, v25.4s }, [x7], #32

        ext8             v26.16b, v0.16b, v0.16b, #8
        ext8             v27.16b, v1.16b, v1.16b, #8

        trn1            v28.4s, v24.4s, v24.4s      
        trn2            v24.4s, v24.4s, v24.4s      
        trn1            v29.4s, v25.4s, v25.4s      
        trn2            v25.4s, v25.4s, v25.4s      

        rev64           v30.4s, v10.4s                 
        rev64           v8.4s, v14.4s                 
        rev64           v0.4s, v18.4s                 
        rev64           v1.4s, v22.4s                 

        fmul            v30.4s, v30.4s, v28.4s      
        fmul            v8.4s, v8.4s, v24.4s      
        fmul            v0.4s, v0.4s, v28.4s      
        fmul            v1.4s, v1.4s, v24.4s      

        rev64           v4.4s, v11.4s                 
        rev64           v5.4s, v15.4s                 
        rev64           v28.4s, v19.4s                 
        rev64           v24.4s, v23.4s                 

        fmul            v4.4s, v4.4s, v29.4s      
        fmul            v5.4s, v5.4s, v25.4s      
        fmul            v28.4s, v28.4s, v29.4s      
        fmul            v24.4s, v24.4s, v25.4s      

        trn1            v29.4s, v27.4s, v27.4s      
        trn2            v27.4s, v27.4s, v27.4s      
        trn1            v25.4s, v26.4s, v26.4s      
        trn2            v26.4s, v26.4s, v26.4s      

        fmul            v29.4s, v29.4s, v31.4s
        fmul            v27.4s, v27.4s, v31.4s
        fmul            v25.4s, v25.4s, v31.4s
        fmul            v26.4s, v26.4s, v31.4s

        fmla            v8.4s, v14.4s, v29.4s      
        fmls            v1.4s, v22.4s, v29.4s      
        fmla            v30.4s, v10.4s, v27.4s      
        fmls            v0.4s, v18.4s, v27.4s      

        fmla            v4.4s, v11.4s, v26.4s      
        fmla            v5.4s, v15.4s, v25.4s      
        fmls            v28.4s, v19.4s, v26.4s      
        fmls            v24.4s, v23.4s, v25.4s      

        fsub            v26.4s, v8.4s, v1.4s
        fsub            v25.4s, v0.4s, v30.4s
        fsub            v27.4s, v28.4s, v4.4s
        fsub            v29.4s, v24.4s, v5.4s

        fadd            v30.4s, v0.4s, v30.4s
        fadd            v8.4s, v1.4s, v8.4s
        fadd            v0.4s, v28.4s, v4.4s
        fadd            v1.4s, v24.4s, v5.4s

        fmul            v25.4s, v25.4s, v31.4s
        fmul            v26.4s, v26.4s, v31.4s
        fmul            v27.4s, v27.4s, v31.4s
        fmul            v29.4s, v29.4s, v31.4s

        rev64           v30.4s, v30.4s
        rev64           v0.4s, v0.4s
        rev64           v8.4s, v8.4s
        rev64           v1.4s, v1.4s

        fsub            v10.4s, v2.4s, v30.4s
        fsub            v11.4s, v3.4s, v0.4s
        fsub            v14.4s, v12.4s, v25.4s
        fsub            v15.4s, v13.4s, v27.4s

        fsub            v18.4s, v6.4s, v8.4s
        fsub            v19.4s, v7.4s, v1.4s
        fadd            v22.4s, v16.4s, v26.4s
        fsub            v23.4s, v17.4s, v29.4s

        fadd            v2.4s, v2.4s, v30.4s
        fadd            v3.4s, v3.4s, v0.4s
        fadd            v12.4s, v12.4s, v25.4s
        fadd            v13.4s, v13.4s, v27.4s

        fadd            v6.4s, v6.4s, v8.4s
        fsub            v16.4s, v16.4s, v26.4s      
        fadd            v7.4s, v7.4s, v1.4s      
        fadd            v17.4s, v17.4s, v29.4s

        stp             q2,  q3,  [x1, #32* 1]
        stp             q6,  q7,  [x1, #32* 3]
        stp             q12, q13, [x1, #32* 5]
        stp             q16, q17, [x1, #32* 7]

        stp             q10, q11, [x1, #32* 9]
        stp             q18, q19, [x1, #32*11]
        stp             q14, q15, [x1, #32*13]
        stp             q22, q23, [x1, #32*15]

        cmp             w20, #64
        bgt            temp_label_50
        ret
temp_label_51
temp_label_50

        stp             x20, x30, [sp, #-16]!
        mov             w20, #32
        add             x1, x1, #16*32
        bl              temp_label_46
        add             x1, x1, #8*32
        bl              temp_label_46
        ldp             x20, x30, [sp], #16
        sub             x1, x1, #24*32

        ldr             w5, =(128 - 4*7)










        adrp            x7, ff_tx_tab_128_float
        add             x7, x7, ff_tx_tab_128_float










        add             x8, x7, x5
        mov             x9, #-32

        mov             x21, #2*128
        add             x22, x21, x21, lsl #1

        cmp             w19, #128
        beq            temp_label_52

        add             x10, x1, x21
        add             x11, x1, x21, lsl #1
        add             x12, x1, x22

        add             x10, x1, x21
        add             x11, x1, x21, lsl #1
        add             x12, x1, x22

        ldp             q0,  q1,  [x1,  #((0 + 0)*32 + 0)]
        ldp             q4,  q5,  [x1,  #((2 + 0)*32 + 0)]
        ldp             q2,  q3,  [x10, #((0 + 0)*32 + 0)]
        ldp             q6,  q7,  [x10, #((2 + 0)*32 + 0)]

        ldp             q8,  q9,  [x11, #((0 + 0)*32 + 0)]
        ldp             q10, q11, [x11, #((2 + 0)*32 + 0)]
        ldp             q12, q13, [x12, #((0 + 0)*32 + 0)]
        ldp             q14, q15, [x12, #((2 + 0)*32 + 0)]


        ld1             { v24.4s, v25.4s }, [x8], x9
        ld1             { v16.4s, v17.4s }, [x7], #32

        ext8             v18.16b, v24.16b, v24.16b, #8
        ext8             v19.16b, v25.16b, v25.16b, #8

        trn1            v20.4s, v16.4s, v16.4s      
        trn2            v16.4s, v16.4s, v16.4s      
        trn1            v21.4s, v17.4s, v17.4s      
        trn2            v17.4s, v17.4s, v17.4s      

        rev64           v22.4s, v8.4s                 
        rev64           v23.4s, v10.4s                 
        rev64           v24.4s, v12.4s                 
        rev64           v25.4s, v14.4s                 

        fmul            v22.4s, v22.4s, v20.4s      
        fmul            v23.4s, v23.4s, v16.4s      
        fmul            v24.4s, v24.4s, v20.4s      
        fmul            v25.4s, v25.4s, v16.4s      

        rev64           v26.4s, v9.4s                 
        rev64           v27.4s, v11.4s                 
        rev64           v20.4s, v13.4s                 
        rev64           v16.4s, v15.4s                 

        fmul            v26.4s, v26.4s, v21.4s      
        fmul            v27.4s, v27.4s, v17.4s      
        fmul            v20.4s, v20.4s, v21.4s      
        fmul            v16.4s, v16.4s, v17.4s      

        trn1            v21.4s, v19.4s, v19.4s      
        trn2            v19.4s, v19.4s, v19.4s      
        trn1            v17.4s, v18.4s, v18.4s      
        trn2            v18.4s, v18.4s, v18.4s      

        fmul            v21.4s, v21.4s, v31.4s
        fmul            v19.4s, v19.4s, v31.4s
        fmul            v17.4s, v17.4s, v31.4s
        fmul            v18.4s, v18.4s, v31.4s

        fmla            v23.4s, v10.4s, v21.4s      
        fmls            v25.4s, v14.4s, v21.4s      
        fmla            v22.4s, v8.4s, v19.4s      
        fmls            v24.4s, v12.4s, v19.4s      

        fmla            v26.4s, v9.4s, v18.4s      
        fmla            v27.4s, v11.4s, v17.4s      
        fmls            v20.4s, v13.4s, v18.4s      
        fmls            v16.4s, v15.4s, v17.4s      

        fsub            v18.4s, v23.4s, v25.4s
        fsub            v17.4s, v24.4s, v22.4s
        fsub            v19.4s, v20.4s, v26.4s
        fsub            v21.4s, v16.4s, v27.4s

        fadd            v22.4s, v24.4s, v22.4s
        fadd            v23.4s, v25.4s, v23.4s
        fadd            v24.4s, v20.4s, v26.4s
        fadd            v25.4s, v16.4s, v27.4s

        fmul            v17.4s, v17.4s, v31.4s
        fmul            v18.4s, v18.4s, v31.4s
        fmul            v19.4s, v19.4s, v31.4s
        fmul            v21.4s, v21.4s, v31.4s

        rev64           v22.4s, v22.4s
        rev64           v24.4s, v24.4s
        rev64           v23.4s, v23.4s
        rev64           v25.4s, v25.4s

        fsub            v8.4s, v0.4s, v22.4s
        fsub            v9.4s, v1.4s, v24.4s
        fsub            v10.4s, v2.4s, v17.4s
        fsub            v11.4s, v3.4s, v19.4s

        fsub            v12.4s, v4.4s, v23.4s
        fsub            v13.4s, v5.4s, v25.4s
        fadd            v14.4s, v6.4s, v18.4s
        fsub            v15.4s, v7.4s, v21.4s

        fadd            v0.4s, v0.4s, v22.4s
        fadd            v1.4s, v1.4s, v24.4s
        fadd            v2.4s, v2.4s, v17.4s
        fadd            v3.4s, v3.4s, v19.4s

        fadd            v4.4s, v4.4s, v23.4s
        fsub            v6.4s, v6.4s, v18.4s      
        fadd            v5.4s, v5.4s, v25.4s      
        fadd            v7.4s, v7.4s, v21.4s

        stp             q0,  q1,  [x1,  #((0 + 0)*32 + 0)]
        stp             q4,  q5,  [x1,  #((2 + 0)*32 + 0)]
        stp             q2,  q3,  [x10, #((0 + 0)*32 + 0)]
        stp             q6,  q7,  [x10, #((2 + 0)*32 + 0)]

        stp             q8,  q9,  [x11, #((0 + 0)*32 + 0)]
        stp             q12, q13, [x11, #((2 + 0)*32 + 0)]
        stp             q10, q11, [x12, #((0 + 0)*32 + 0)]
        stp             q14, q15, [x12, #((2 + 0)*32 + 0)]
        add             x10, x1, x21
        add             x11, x1, x21, lsl #1
        add             x12, x1, x22

        ldp             q0,  q1,  [x1,  #((0 + 1)*32 + 0)]
        ldp             q4,  q5,  [x1,  #((2 + 1)*32 + 0)]
        ldp             q2,  q3,  [x10, #((0 + 1)*32 + 0)]
        ldp             q6,  q7,  [x10, #((2 + 1)*32 + 0)]

        ldp             q8,  q9,  [x11, #((0 + 1)*32 + 0)]
        ldp             q10, q11, [x11, #((2 + 1)*32 + 0)]
        ldp             q12, q13, [x12, #((0 + 1)*32 + 0)]
        ldp             q14, q15, [x12, #((2 + 1)*32 + 0)]


        ld1             { v24.4s, v25.4s }, [x8], x9
        ld1             { v16.4s, v17.4s }, [x7], #32

        ext8             v18.16b, v24.16b, v24.16b, #8
        ext8             v19.16b, v25.16b, v25.16b, #8

        trn1            v20.4s, v16.4s, v16.4s      
        trn2            v16.4s, v16.4s, v16.4s      
        trn1            v21.4s, v17.4s, v17.4s      
        trn2            v17.4s, v17.4s, v17.4s      

        rev64           v22.4s, v8.4s                 
        rev64           v23.4s, v10.4s                 
        rev64           v24.4s, v12.4s                 
        rev64           v25.4s, v14.4s                 

        fmul            v22.4s, v22.4s, v20.4s      
        fmul            v23.4s, v23.4s, v16.4s      
        fmul            v24.4s, v24.4s, v20.4s      
        fmul            v25.4s, v25.4s, v16.4s      

        rev64           v26.4s, v9.4s                 
        rev64           v27.4s, v11.4s                 
        rev64           v20.4s, v13.4s                 
        rev64           v16.4s, v15.4s                 

        fmul            v26.4s, v26.4s, v21.4s      
        fmul            v27.4s, v27.4s, v17.4s      
        fmul            v20.4s, v20.4s, v21.4s      
        fmul            v16.4s, v16.4s, v17.4s      

        trn1            v21.4s, v19.4s, v19.4s      
        trn2            v19.4s, v19.4s, v19.4s      
        trn1            v17.4s, v18.4s, v18.4s      
        trn2            v18.4s, v18.4s, v18.4s      

        fmul            v21.4s, v21.4s, v31.4s
        fmul            v19.4s, v19.4s, v31.4s
        fmul            v17.4s, v17.4s, v31.4s
        fmul            v18.4s, v18.4s, v31.4s

        fmla            v23.4s, v10.4s, v21.4s      
        fmls            v25.4s, v14.4s, v21.4s      
        fmla            v22.4s, v8.4s, v19.4s      
        fmls            v24.4s, v12.4s, v19.4s      

        fmla            v26.4s, v9.4s, v18.4s      
        fmla            v27.4s, v11.4s, v17.4s      
        fmls            v20.4s, v13.4s, v18.4s      
        fmls            v16.4s, v15.4s, v17.4s      

        fsub            v18.4s, v23.4s, v25.4s
        fsub            v17.4s, v24.4s, v22.4s
        fsub            v19.4s, v20.4s, v26.4s
        fsub            v21.4s, v16.4s, v27.4s

        fadd            v22.4s, v24.4s, v22.4s
        fadd            v23.4s, v25.4s, v23.4s
        fadd            v24.4s, v20.4s, v26.4s
        fadd            v25.4s, v16.4s, v27.4s

        fmul            v17.4s, v17.4s, v31.4s
        fmul            v18.4s, v18.4s, v31.4s
        fmul            v19.4s, v19.4s, v31.4s
        fmul            v21.4s, v21.4s, v31.4s

        rev64           v22.4s, v22.4s
        rev64           v24.4s, v24.4s
        rev64           v23.4s, v23.4s
        rev64           v25.4s, v25.4s

        fsub            v8.4s, v0.4s, v22.4s
        fsub            v9.4s, v1.4s, v24.4s
        fsub            v10.4s, v2.4s, v17.4s
        fsub            v11.4s, v3.4s, v19.4s

        fsub            v12.4s, v4.4s, v23.4s
        fsub            v13.4s, v5.4s, v25.4s
        fadd            v14.4s, v6.4s, v18.4s
        fsub            v15.4s, v7.4s, v21.4s

        fadd            v0.4s, v0.4s, v22.4s
        fadd            v1.4s, v1.4s, v24.4s
        fadd            v2.4s, v2.4s, v17.4s
        fadd            v3.4s, v3.4s, v19.4s

        fadd            v4.4s, v4.4s, v23.4s
        fsub            v6.4s, v6.4s, v18.4s      
        fadd            v5.4s, v5.4s, v25.4s      
        fadd            v7.4s, v7.4s, v21.4s

        stp             q0,  q1,  [x1,  #((0 + 1)*32 + 0)]
        stp             q4,  q5,  [x1,  #((2 + 1)*32 + 0)]
        stp             q2,  q3,  [x10, #((0 + 1)*32 + 0)]
        stp             q6,  q7,  [x10, #((2 + 1)*32 + 0)]

        stp             q8,  q9,  [x11, #((0 + 1)*32 + 0)]
        stp             q12, q13, [x11, #((2 + 1)*32 + 0)]
        stp             q10, q11, [x12, #((0 + 1)*32 + 0)]
        stp             q14, q15, [x12, #((2 + 1)*32 + 0)]
        add             x10, x1, x21
        add             x11, x1, x21, lsl #1
        add             x12, x1, x22

        ldp             q0,  q1,  [x1,  #((0 + 4)*32 + 0)]
        ldp             q4,  q5,  [x1,  #((2 + 4)*32 + 0)]
        ldp             q2,  q3,  [x10, #((0 + 4)*32 + 0)]
        ldp             q6,  q7,  [x10, #((2 + 4)*32 + 0)]

        ldp             q8,  q9,  [x11, #((0 + 4)*32 + 0)]
        ldp             q10, q11, [x11, #((2 + 4)*32 + 0)]
        ldp             q12, q13, [x12, #((0 + 4)*32 + 0)]
        ldp             q14, q15, [x12, #((2 + 4)*32 + 0)]


        ld1             { v24.4s, v25.4s }, [x8], x9
        ld1             { v16.4s, v17.4s }, [x7], #32

        ext8             v18.16b, v24.16b, v24.16b, #8
        ext8             v19.16b, v25.16b, v25.16b, #8

        trn1            v20.4s, v16.4s, v16.4s      
        trn2            v16.4s, v16.4s, v16.4s      
        trn1            v21.4s, v17.4s, v17.4s      
        trn2            v17.4s, v17.4s, v17.4s      

        rev64           v22.4s, v8.4s                 
        rev64           v23.4s, v10.4s                 
        rev64           v24.4s, v12.4s                 
        rev64           v25.4s, v14.4s                 

        fmul            v22.4s, v22.4s, v20.4s      
        fmul            v23.4s, v23.4s, v16.4s      
        fmul            v24.4s, v24.4s, v20.4s      
        fmul            v25.4s, v25.4s, v16.4s      

        rev64           v26.4s, v9.4s                 
        rev64           v27.4s, v11.4s                 
        rev64           v20.4s, v13.4s                 
        rev64           v16.4s, v15.4s                 

        fmul            v26.4s, v26.4s, v21.4s      
        fmul            v27.4s, v27.4s, v17.4s      
        fmul            v20.4s, v20.4s, v21.4s      
        fmul            v16.4s, v16.4s, v17.4s      

        trn1            v21.4s, v19.4s, v19.4s      
        trn2            v19.4s, v19.4s, v19.4s      
        trn1            v17.4s, v18.4s, v18.4s      
        trn2            v18.4s, v18.4s, v18.4s      

        fmul            v21.4s, v21.4s, v31.4s
        fmul            v19.4s, v19.4s, v31.4s
        fmul            v17.4s, v17.4s, v31.4s
        fmul            v18.4s, v18.4s, v31.4s

        fmla            v23.4s, v10.4s, v21.4s      
        fmls            v25.4s, v14.4s, v21.4s      
        fmla            v22.4s, v8.4s, v19.4s      
        fmls            v24.4s, v12.4s, v19.4s      

        fmla            v26.4s, v9.4s, v18.4s      
        fmla            v27.4s, v11.4s, v17.4s      
        fmls            v20.4s, v13.4s, v18.4s      
        fmls            v16.4s, v15.4s, v17.4s      

        fsub            v18.4s, v23.4s, v25.4s
        fsub            v17.4s, v24.4s, v22.4s
        fsub            v19.4s, v20.4s, v26.4s
        fsub            v21.4s, v16.4s, v27.4s

        fadd            v22.4s, v24.4s, v22.4s
        fadd            v23.4s, v25.4s, v23.4s
        fadd            v24.4s, v20.4s, v26.4s
        fadd            v25.4s, v16.4s, v27.4s

        fmul            v17.4s, v17.4s, v31.4s
        fmul            v18.4s, v18.4s, v31.4s
        fmul            v19.4s, v19.4s, v31.4s
        fmul            v21.4s, v21.4s, v31.4s

        rev64           v22.4s, v22.4s
        rev64           v24.4s, v24.4s
        rev64           v23.4s, v23.4s
        rev64           v25.4s, v25.4s

        fsub            v8.4s, v0.4s, v22.4s
        fsub            v9.4s, v1.4s, v24.4s
        fsub            v10.4s, v2.4s, v17.4s
        fsub            v11.4s, v3.4s, v19.4s

        fsub            v12.4s, v4.4s, v23.4s
        fsub            v13.4s, v5.4s, v25.4s
        fadd            v14.4s, v6.4s, v18.4s
        fsub            v15.4s, v7.4s, v21.4s

        fadd            v0.4s, v0.4s, v22.4s
        fadd            v1.4s, v1.4s, v24.4s
        fadd            v2.4s, v2.4s, v17.4s
        fadd            v3.4s, v3.4s, v19.4s

        fadd            v4.4s, v4.4s, v23.4s
        fsub            v6.4s, v6.4s, v18.4s      
        fadd            v5.4s, v5.4s, v25.4s      
        fadd            v7.4s, v7.4s, v21.4s

        stp             q0,  q1,  [x1,  #((0 + 4)*32 + 0)]
        stp             q4,  q5,  [x1,  #((2 + 4)*32 + 0)]
        stp             q2,  q3,  [x10, #((0 + 4)*32 + 0)]
        stp             q6,  q7,  [x10, #((2 + 4)*32 + 0)]

        stp             q8,  q9,  [x11, #((0 + 4)*32 + 0)]
        stp             q12, q13, [x11, #((2 + 4)*32 + 0)]
        stp             q10, q11, [x12, #((0 + 4)*32 + 0)]
        stp             q14, q15, [x12, #((2 + 4)*32 + 0)]
        add             x10, x1, x21
        add             x11, x1, x21, lsl #1
        add             x12, x1, x22

        ldp             q0,  q1,  [x1,  #((0 + 5)*32 + 0)]
        ldp             q4,  q5,  [x1,  #((2 + 5)*32 + 0)]
        ldp             q2,  q3,  [x10, #((0 + 5)*32 + 0)]
        ldp             q6,  q7,  [x10, #((2 + 5)*32 + 0)]

        ldp             q8,  q9,  [x11, #((0 + 5)*32 + 0)]
        ldp             q10, q11, [x11, #((2 + 5)*32 + 0)]
        ldp             q12, q13, [x12, #((0 + 5)*32 + 0)]
        ldp             q14, q15, [x12, #((2 + 5)*32 + 0)]


        ld1             { v24.4s, v25.4s }, [x8], x9
        ld1             { v16.4s, v17.4s }, [x7], #32

        ext8             v18.16b, v24.16b, v24.16b, #8
        ext8             v19.16b, v25.16b, v25.16b, #8

        trn1            v20.4s, v16.4s, v16.4s      
        trn2            v16.4s, v16.4s, v16.4s      
        trn1            v21.4s, v17.4s, v17.4s      
        trn2            v17.4s, v17.4s, v17.4s      

        rev64           v22.4s, v8.4s                 
        rev64           v23.4s, v10.4s                 
        rev64           v24.4s, v12.4s                 
        rev64           v25.4s, v14.4s                 

        fmul            v22.4s, v22.4s, v20.4s      
        fmul            v23.4s, v23.4s, v16.4s      
        fmul            v24.4s, v24.4s, v20.4s      
        fmul            v25.4s, v25.4s, v16.4s      

        rev64           v26.4s, v9.4s                 
        rev64           v27.4s, v11.4s                 
        rev64           v20.4s, v13.4s                 
        rev64           v16.4s, v15.4s                 

        fmul            v26.4s, v26.4s, v21.4s      
        fmul            v27.4s, v27.4s, v17.4s      
        fmul            v20.4s, v20.4s, v21.4s      
        fmul            v16.4s, v16.4s, v17.4s      

        trn1            v21.4s, v19.4s, v19.4s      
        trn2            v19.4s, v19.4s, v19.4s      
        trn1            v17.4s, v18.4s, v18.4s      
        trn2            v18.4s, v18.4s, v18.4s      

        fmul            v21.4s, v21.4s, v31.4s
        fmul            v19.4s, v19.4s, v31.4s
        fmul            v17.4s, v17.4s, v31.4s
        fmul            v18.4s, v18.4s, v31.4s

        fmla            v23.4s, v10.4s, v21.4s      
        fmls            v25.4s, v14.4s, v21.4s      
        fmla            v22.4s, v8.4s, v19.4s      
        fmls            v24.4s, v12.4s, v19.4s      

        fmla            v26.4s, v9.4s, v18.4s      
        fmla            v27.4s, v11.4s, v17.4s      
        fmls            v20.4s, v13.4s, v18.4s      
        fmls            v16.4s, v15.4s, v17.4s      

        fsub            v18.4s, v23.4s, v25.4s
        fsub            v17.4s, v24.4s, v22.4s
        fsub            v19.4s, v20.4s, v26.4s
        fsub            v21.4s, v16.4s, v27.4s

        fadd            v22.4s, v24.4s, v22.4s
        fadd            v23.4s, v25.4s, v23.4s
        fadd            v24.4s, v20.4s, v26.4s
        fadd            v25.4s, v16.4s, v27.4s

        fmul            v17.4s, v17.4s, v31.4s
        fmul            v18.4s, v18.4s, v31.4s
        fmul            v19.4s, v19.4s, v31.4s
        fmul            v21.4s, v21.4s, v31.4s

        rev64           v22.4s, v22.4s
        rev64           v24.4s, v24.4s
        rev64           v23.4s, v23.4s
        rev64           v25.4s, v25.4s

        fsub            v8.4s, v0.4s, v22.4s
        fsub            v9.4s, v1.4s, v24.4s
        fsub            v10.4s, v2.4s, v17.4s
        fsub            v11.4s, v3.4s, v19.4s

        fsub            v12.4s, v4.4s, v23.4s
        fsub            v13.4s, v5.4s, v25.4s
        fadd            v14.4s, v6.4s, v18.4s
        fsub            v15.4s, v7.4s, v21.4s

        fadd            v0.4s, v0.4s, v22.4s
        fadd            v1.4s, v1.4s, v24.4s
        fadd            v2.4s, v2.4s, v17.4s
        fadd            v3.4s, v3.4s, v19.4s

        fadd            v4.4s, v4.4s, v23.4s
        fsub            v6.4s, v6.4s, v18.4s      
        fadd            v5.4s, v5.4s, v25.4s      
        fadd            v7.4s, v7.4s, v21.4s

        stp             q0,  q1,  [x1,  #((0 + 5)*32 + 0)]
        stp             q4,  q5,  [x1,  #((2 + 5)*32 + 0)]
        stp             q2,  q3,  [x10, #((0 + 5)*32 + 0)]
        stp             q6,  q7,  [x10, #((2 + 5)*32 + 0)]

        stp             q8,  q9,  [x11, #((0 + 5)*32 + 0)]
        stp             q12, q13, [x11, #((2 + 5)*32 + 0)]
        stp             q10, q11, [x12, #((0 + 5)*32 + 0)]
        stp             q14, q15, [x12, #((2 + 5)*32 + 0)]

        cmp             w20, #128
        bgt            temp_label_53
        ret
temp_label_54
temp_label_53

        stp             x20, x30, [sp, #-16]!
        mov             w20, #64
        add             x1, x1, #32*32
        bl              temp_label_46
        add             x1, x1, #16*32
        bl              temp_label_46
        ldp             x20, x30, [sp], #16
        sub             x1, x1, #48*32

        ldr             w5, =(256 - 4*7)










        adrp            x7, ff_tx_tab_256_float
        add             x7, x7, ff_tx_tab_256_float










        add             x8, x7, x5
        mov             x9, #-32

        mov             x21, #2*256
        add             x22, x21, x21, lsl #1

        cmp             w19, #256
        beq            temp_label_55

        add             x10, x1, x21
        add             x11, x1, x21, lsl #1
        add             x12, x1, x22

        add             x10, x1, x21
        add             x11, x1, x21, lsl #1
        add             x12, x1, x22

        ldp             q0,  q1,  [x1,  #((0 + 0)*32 + 0)]
        ldp             q4,  q5,  [x1,  #((2 + 0)*32 + 0)]
        ldp             q2,  q3,  [x10, #((0 + 0)*32 + 0)]
        ldp             q6,  q7,  [x10, #((2 + 0)*32 + 0)]

        ldp             q8,  q9,  [x11, #((0 + 0)*32 + 0)]
        ldp             q10, q11, [x11, #((2 + 0)*32 + 0)]
        ldp             q12, q13, [x12, #((0 + 0)*32 + 0)]
        ldp             q14, q15, [x12, #((2 + 0)*32 + 0)]


        ld1             { v24.4s, v25.4s }, [x8], x9
        ld1             { v16.4s, v17.4s }, [x7], #32

        ext8             v18.16b, v24.16b, v24.16b, #8
        ext8             v19.16b, v25.16b, v25.16b, #8

        trn1            v20.4s, v16.4s, v16.4s      
        trn2            v16.4s, v16.4s, v16.4s      
        trn1            v21.4s, v17.4s, v17.4s      
        trn2            v17.4s, v17.4s, v17.4s      

        rev64           v22.4s, v8.4s                 
        rev64           v23.4s, v10.4s                 
        rev64           v24.4s, v12.4s                 
        rev64           v25.4s, v14.4s                 

        fmul            v22.4s, v22.4s, v20.4s      
        fmul            v23.4s, v23.4s, v16.4s      
        fmul            v24.4s, v24.4s, v20.4s      
        fmul            v25.4s, v25.4s, v16.4s      

        rev64           v26.4s, v9.4s                 
        rev64           v27.4s, v11.4s                 
        rev64           v20.4s, v13.4s                 
        rev64           v16.4s, v15.4s                 

        fmul            v26.4s, v26.4s, v21.4s      
        fmul            v27.4s, v27.4s, v17.4s      
        fmul            v20.4s, v20.4s, v21.4s      
        fmul            v16.4s, v16.4s, v17.4s      

        trn1            v21.4s, v19.4s, v19.4s      
        trn2            v19.4s, v19.4s, v19.4s      
        trn1            v17.4s, v18.4s, v18.4s      
        trn2            v18.4s, v18.4s, v18.4s      

        fmul            v21.4s, v21.4s, v31.4s
        fmul            v19.4s, v19.4s, v31.4s
        fmul            v17.4s, v17.4s, v31.4s
        fmul            v18.4s, v18.4s, v31.4s

        fmla            v23.4s, v10.4s, v21.4s      
        fmls            v25.4s, v14.4s, v21.4s      
        fmla            v22.4s, v8.4s, v19.4s      
        fmls            v24.4s, v12.4s, v19.4s      

        fmla            v26.4s, v9.4s, v18.4s      
        fmla            v27.4s, v11.4s, v17.4s      
        fmls            v20.4s, v13.4s, v18.4s      
        fmls            v16.4s, v15.4s, v17.4s      

        fsub            v18.4s, v23.4s, v25.4s
        fsub            v17.4s, v24.4s, v22.4s
        fsub            v19.4s, v20.4s, v26.4s
        fsub            v21.4s, v16.4s, v27.4s

        fadd            v22.4s, v24.4s, v22.4s
        fadd            v23.4s, v25.4s, v23.4s
        fadd            v24.4s, v20.4s, v26.4s
        fadd            v25.4s, v16.4s, v27.4s

        fmul            v17.4s, v17.4s, v31.4s
        fmul            v18.4s, v18.4s, v31.4s
        fmul            v19.4s, v19.4s, v31.4s
        fmul            v21.4s, v21.4s, v31.4s

        rev64           v22.4s, v22.4s
        rev64           v24.4s, v24.4s
        rev64           v23.4s, v23.4s
        rev64           v25.4s, v25.4s

        fsub            v8.4s, v0.4s, v22.4s
        fsub            v9.4s, v1.4s, v24.4s
        fsub            v10.4s, v2.4s, v17.4s
        fsub            v11.4s, v3.4s, v19.4s

        fsub            v12.4s, v4.4s, v23.4s
        fsub            v13.4s, v5.4s, v25.4s
        fadd            v14.4s, v6.4s, v18.4s
        fsub            v15.4s, v7.4s, v21.4s

        fadd            v0.4s, v0.4s, v22.4s
        fadd            v1.4s, v1.4s, v24.4s
        fadd            v2.4s, v2.4s, v17.4s
        fadd            v3.4s, v3.4s, v19.4s

        fadd            v4.4s, v4.4s, v23.4s
        fsub            v6.4s, v6.4s, v18.4s      
        fadd            v5.4s, v5.4s, v25.4s      
        fadd            v7.4s, v7.4s, v21.4s

        stp             q0,  q1,  [x1,  #((0 + 0)*32 + 0)]
        stp             q4,  q5,  [x1,  #((2 + 0)*32 + 0)]
        stp             q2,  q3,  [x10, #((0 + 0)*32 + 0)]
        stp             q6,  q7,  [x10, #((2 + 0)*32 + 0)]

        stp             q8,  q9,  [x11, #((0 + 0)*32 + 0)]
        stp             q12, q13, [x11, #((2 + 0)*32 + 0)]
        stp             q10, q11, [x12, #((0 + 0)*32 + 0)]
        stp             q14, q15, [x12, #((2 + 0)*32 + 0)]
        add             x10, x1, x21
        add             x11, x1, x21, lsl #1
        add             x12, x1, x22

        ldp             q0,  q1,  [x1,  #((0 + 1)*32 + 0)]
        ldp             q4,  q5,  [x1,  #((2 + 1)*32 + 0)]
        ldp             q2,  q3,  [x10, #((0 + 1)*32 + 0)]
        ldp             q6,  q7,  [x10, #((2 + 1)*32 + 0)]

        ldp             q8,  q9,  [x11, #((0 + 1)*32 + 0)]
        ldp             q10, q11, [x11, #((2 + 1)*32 + 0)]
        ldp             q12, q13, [x12, #((0 + 1)*32 + 0)]
        ldp             q14, q15, [x12, #((2 + 1)*32 + 0)]


        ld1             { v24.4s, v25.4s }, [x8], x9
        ld1             { v16.4s, v17.4s }, [x7], #32

        ext8             v18.16b, v24.16b, v24.16b, #8
        ext8             v19.16b, v25.16b, v25.16b, #8

        trn1            v20.4s, v16.4s, v16.4s      
        trn2            v16.4s, v16.4s, v16.4s      
        trn1            v21.4s, v17.4s, v17.4s      
        trn2            v17.4s, v17.4s, v17.4s      

        rev64           v22.4s, v8.4s                 
        rev64           v23.4s, v10.4s                 
        rev64           v24.4s, v12.4s                 
        rev64           v25.4s, v14.4s                 

        fmul            v22.4s, v22.4s, v20.4s      
        fmul            v23.4s, v23.4s, v16.4s      
        fmul            v24.4s, v24.4s, v20.4s      
        fmul            v25.4s, v25.4s, v16.4s      

        rev64           v26.4s, v9.4s                 
        rev64           v27.4s, v11.4s                 
        rev64           v20.4s, v13.4s                 
        rev64           v16.4s, v15.4s                 

        fmul            v26.4s, v26.4s, v21.4s      
        fmul            v27.4s, v27.4s, v17.4s      
        fmul            v20.4s, v20.4s, v21.4s      
        fmul            v16.4s, v16.4s, v17.4s      

        trn1            v21.4s, v19.4s, v19.4s      
        trn2            v19.4s, v19.4s, v19.4s      
        trn1            v17.4s, v18.4s, v18.4s      
        trn2            v18.4s, v18.4s, v18.4s      

        fmul            v21.4s, v21.4s, v31.4s
        fmul            v19.4s, v19.4s, v31.4s
        fmul            v17.4s, v17.4s, v31.4s
        fmul            v18.4s, v18.4s, v31.4s

        fmla            v23.4s, v10.4s, v21.4s      
        fmls            v25.4s, v14.4s, v21.4s      
        fmla            v22.4s, v8.4s, v19.4s      
        fmls            v24.4s, v12.4s, v19.4s      

        fmla            v26.4s, v9.4s, v18.4s      
        fmla            v27.4s, v11.4s, v17.4s      
        fmls            v20.4s, v13.4s, v18.4s      
        fmls            v16.4s, v15.4s, v17.4s      

        fsub            v18.4s, v23.4s, v25.4s
        fsub            v17.4s, v24.4s, v22.4s
        fsub            v19.4s, v20.4s, v26.4s
        fsub            v21.4s, v16.4s, v27.4s

        fadd            v22.4s, v24.4s, v22.4s
        fadd            v23.4s, v25.4s, v23.4s
        fadd            v24.4s, v20.4s, v26.4s
        fadd            v25.4s, v16.4s, v27.4s

        fmul            v17.4s, v17.4s, v31.4s
        fmul            v18.4s, v18.4s, v31.4s
        fmul            v19.4s, v19.4s, v31.4s
        fmul            v21.4s, v21.4s, v31.4s

        rev64           v22.4s, v22.4s
        rev64           v24.4s, v24.4s
        rev64           v23.4s, v23.4s
        rev64           v25.4s, v25.4s

        fsub            v8.4s, v0.4s, v22.4s
        fsub            v9.4s, v1.4s, v24.4s
        fsub            v10.4s, v2.4s, v17.4s
        fsub            v11.4s, v3.4s, v19.4s

        fsub            v12.4s, v4.4s, v23.4s
        fsub            v13.4s, v5.4s, v25.4s
        fadd            v14.4s, v6.4s, v18.4s
        fsub            v15.4s, v7.4s, v21.4s

        fadd            v0.4s, v0.4s, v22.4s
        fadd            v1.4s, v1.4s, v24.4s
        fadd            v2.4s, v2.4s, v17.4s
        fadd            v3.4s, v3.4s, v19.4s

        fadd            v4.4s, v4.4s, v23.4s
        fsub            v6.4s, v6.4s, v18.4s      
        fadd            v5.4s, v5.4s, v25.4s      
        fadd            v7.4s, v7.4s, v21.4s

        stp             q0,  q1,  [x1,  #((0 + 1)*32 + 0)]
        stp             q4,  q5,  [x1,  #((2 + 1)*32 + 0)]
        stp             q2,  q3,  [x10, #((0 + 1)*32 + 0)]
        stp             q6,  q7,  [x10, #((2 + 1)*32 + 0)]

        stp             q8,  q9,  [x11, #((0 + 1)*32 + 0)]
        stp             q12, q13, [x11, #((2 + 1)*32 + 0)]
        stp             q10, q11, [x12, #((0 + 1)*32 + 0)]
        stp             q14, q15, [x12, #((2 + 1)*32 + 0)]
        add             x10, x1, x21
        add             x11, x1, x21, lsl #1
        add             x12, x1, x22

        ldp             q0,  q1,  [x1,  #((0 + 4)*32 + 0)]
        ldp             q4,  q5,  [x1,  #((2 + 4)*32 + 0)]
        ldp             q2,  q3,  [x10, #((0 + 4)*32 + 0)]
        ldp             q6,  q7,  [x10, #((2 + 4)*32 + 0)]

        ldp             q8,  q9,  [x11, #((0 + 4)*32 + 0)]
        ldp             q10, q11, [x11, #((2 + 4)*32 + 0)]
        ldp             q12, q13, [x12, #((0 + 4)*32 + 0)]
        ldp             q14, q15, [x12, #((2 + 4)*32 + 0)]


        ld1             { v24.4s, v25.4s }, [x8], x9
        ld1             { v16.4s, v17.4s }, [x7], #32

        ext8             v18.16b, v24.16b, v24.16b, #8
        ext8             v19.16b, v25.16b, v25.16b, #8

        trn1            v20.4s, v16.4s, v16.4s      
        trn2            v16.4s, v16.4s, v16.4s      
        trn1            v21.4s, v17.4s, v17.4s      
        trn2            v17.4s, v17.4s, v17.4s      

        rev64           v22.4s, v8.4s                 
        rev64           v23.4s, v10.4s                 
        rev64           v24.4s, v12.4s                 
        rev64           v25.4s, v14.4s                 

        fmul            v22.4s, v22.4s, v20.4s      
        fmul            v23.4s, v23.4s, v16.4s      
        fmul            v24.4s, v24.4s, v20.4s      
        fmul            v25.4s, v25.4s, v16.4s      

        rev64           v26.4s, v9.4s                 
        rev64           v27.4s, v11.4s                 
        rev64           v20.4s, v13.4s                 
        rev64           v16.4s, v15.4s                 

        fmul            v26.4s, v26.4s, v21.4s      
        fmul            v27.4s, v27.4s, v17.4s      
        fmul            v20.4s, v20.4s, v21.4s      
        fmul            v16.4s, v16.4s, v17.4s      

        trn1            v21.4s, v19.4s, v19.4s      
        trn2            v19.4s, v19.4s, v19.4s      
        trn1            v17.4s, v18.4s, v18.4s      
        trn2            v18.4s, v18.4s, v18.4s      

        fmul            v21.4s, v21.4s, v31.4s
        fmul            v19.4s, v19.4s, v31.4s
        fmul            v17.4s, v17.4s, v31.4s
        fmul            v18.4s, v18.4s, v31.4s

        fmla            v23.4s, v10.4s, v21.4s      
        fmls            v25.4s, v14.4s, v21.4s      
        fmla            v22.4s, v8.4s, v19.4s      
        fmls            v24.4s, v12.4s, v19.4s      

        fmla            v26.4s, v9.4s, v18.4s      
        fmla            v27.4s, v11.4s, v17.4s      
        fmls            v20.4s, v13.4s, v18.4s      
        fmls            v16.4s, v15.4s, v17.4s      

        fsub            v18.4s, v23.4s, v25.4s
        fsub            v17.4s, v24.4s, v22.4s
        fsub            v19.4s, v20.4s, v26.4s
        fsub            v21.4s, v16.4s, v27.4s

        fadd            v22.4s, v24.4s, v22.4s
        fadd            v23.4s, v25.4s, v23.4s
        fadd            v24.4s, v20.4s, v26.4s
        fadd            v25.4s, v16.4s, v27.4s

        fmul            v17.4s, v17.4s, v31.4s
        fmul            v18.4s, v18.4s, v31.4s
        fmul            v19.4s, v19.4s, v31.4s
        fmul            v21.4s, v21.4s, v31.4s

        rev64           v22.4s, v22.4s
        rev64           v24.4s, v24.4s
        rev64           v23.4s, v23.4s
        rev64           v25.4s, v25.4s

        fsub            v8.4s, v0.4s, v22.4s
        fsub            v9.4s, v1.4s, v24.4s
        fsub            v10.4s, v2.4s, v17.4s
        fsub            v11.4s, v3.4s, v19.4s

        fsub            v12.4s, v4.4s, v23.4s
        fsub            v13.4s, v5.4s, v25.4s
        fadd            v14.4s, v6.4s, v18.4s
        fsub            v15.4s, v7.4s, v21.4s

        fadd            v0.4s, v0.4s, v22.4s
        fadd            v1.4s, v1.4s, v24.4s
        fadd            v2.4s, v2.4s, v17.4s
        fadd            v3.4s, v3.4s, v19.4s

        fadd            v4.4s, v4.4s, v23.4s
        fsub            v6.4s, v6.4s, v18.4s      
        fadd            v5.4s, v5.4s, v25.4s      
        fadd            v7.4s, v7.4s, v21.4s

        stp             q0,  q1,  [x1,  #((0 + 4)*32 + 0)]
        stp             q4,  q5,  [x1,  #((2 + 4)*32 + 0)]
        stp             q2,  q3,  [x10, #((0 + 4)*32 + 0)]
        stp             q6,  q7,  [x10, #((2 + 4)*32 + 0)]

        stp             q8,  q9,  [x11, #((0 + 4)*32 + 0)]
        stp             q12, q13, [x11, #((2 + 4)*32 + 0)]
        stp             q10, q11, [x12, #((0 + 4)*32 + 0)]
        stp             q14, q15, [x12, #((2 + 4)*32 + 0)]
        add             x10, x1, x21
        add             x11, x1, x21, lsl #1
        add             x12, x1, x22

        ldp             q0,  q1,  [x1,  #((0 + 5)*32 + 0)]
        ldp             q4,  q5,  [x1,  #((2 + 5)*32 + 0)]
        ldp             q2,  q3,  [x10, #((0 + 5)*32 + 0)]
        ldp             q6,  q7,  [x10, #((2 + 5)*32 + 0)]

        ldp             q8,  q9,  [x11, #((0 + 5)*32 + 0)]
        ldp             q10, q11, [x11, #((2 + 5)*32 + 0)]
        ldp             q12, q13, [x12, #((0 + 5)*32 + 0)]
        ldp             q14, q15, [x12, #((2 + 5)*32 + 0)]


        ld1             { v24.4s, v25.4s }, [x8], x9
        ld1             { v16.4s, v17.4s }, [x7], #32

        ext8             v18.16b, v24.16b, v24.16b, #8
        ext8             v19.16b, v25.16b, v25.16b, #8

        trn1            v20.4s, v16.4s, v16.4s      
        trn2            v16.4s, v16.4s, v16.4s      
        trn1            v21.4s, v17.4s, v17.4s      
        trn2            v17.4s, v17.4s, v17.4s      

        rev64           v22.4s, v8.4s                 
        rev64           v23.4s, v10.4s                 
        rev64           v24.4s, v12.4s                 
        rev64           v25.4s, v14.4s                 

        fmul            v22.4s, v22.4s, v20.4s      
        fmul            v23.4s, v23.4s, v16.4s      
        fmul            v24.4s, v24.4s, v20.4s      
        fmul            v25.4s, v25.4s, v16.4s      

        rev64           v26.4s, v9.4s                 
        rev64           v27.4s, v11.4s                 
        rev64           v20.4s, v13.4s                 
        rev64           v16.4s, v15.4s                 

        fmul            v26.4s, v26.4s, v21.4s      
        fmul            v27.4s, v27.4s, v17.4s      
        fmul            v20.4s, v20.4s, v21.4s      
        fmul            v16.4s, v16.4s, v17.4s      

        trn1            v21.4s, v19.4s, v19.4s      
        trn2            v19.4s, v19.4s, v19.4s      
        trn1            v17.4s, v18.4s, v18.4s      
        trn2            v18.4s, v18.4s, v18.4s      

        fmul            v21.4s, v21.4s, v31.4s
        fmul            v19.4s, v19.4s, v31.4s
        fmul            v17.4s, v17.4s, v31.4s
        fmul            v18.4s, v18.4s, v31.4s

        fmla            v23.4s, v10.4s, v21.4s      
        fmls            v25.4s, v14.4s, v21.4s      
        fmla            v22.4s, v8.4s, v19.4s      
        fmls            v24.4s, v12.4s, v19.4s      

        fmla            v26.4s, v9.4s, v18.4s      
        fmla            v27.4s, v11.4s, v17.4s      
        fmls            v20.4s, v13.4s, v18.4s      
        fmls            v16.4s, v15.4s, v17.4s      

        fsub            v18.4s, v23.4s, v25.4s
        fsub            v17.4s, v24.4s, v22.4s
        fsub            v19.4s, v20.4s, v26.4s
        fsub            v21.4s, v16.4s, v27.4s

        fadd            v22.4s, v24.4s, v22.4s
        fadd            v23.4s, v25.4s, v23.4s
        fadd            v24.4s, v20.4s, v26.4s
        fadd            v25.4s, v16.4s, v27.4s

        fmul            v17.4s, v17.4s, v31.4s
        fmul            v18.4s, v18.4s, v31.4s
        fmul            v19.4s, v19.4s, v31.4s
        fmul            v21.4s, v21.4s, v31.4s

        rev64           v22.4s, v22.4s
        rev64           v24.4s, v24.4s
        rev64           v23.4s, v23.4s
        rev64           v25.4s, v25.4s

        fsub            v8.4s, v0.4s, v22.4s
        fsub            v9.4s, v1.4s, v24.4s
        fsub            v10.4s, v2.4s, v17.4s
        fsub            v11.4s, v3.4s, v19.4s

        fsub            v12.4s, v4.4s, v23.4s
        fsub            v13.4s, v5.4s, v25.4s
        fadd            v14.4s, v6.4s, v18.4s
        fsub            v15.4s, v7.4s, v21.4s

        fadd            v0.4s, v0.4s, v22.4s
        fadd            v1.4s, v1.4s, v24.4s
        fadd            v2.4s, v2.4s, v17.4s
        fadd            v3.4s, v3.4s, v19.4s

        fadd            v4.4s, v4.4s, v23.4s
        fsub            v6.4s, v6.4s, v18.4s      
        fadd            v5.4s, v5.4s, v25.4s      
        fadd            v7.4s, v7.4s, v21.4s

        stp             q0,  q1,  [x1,  #((0 + 5)*32 + 0)]
        stp             q4,  q5,  [x1,  #((2 + 5)*32 + 0)]
        stp             q2,  q3,  [x10, #((0 + 5)*32 + 0)]
        stp             q6,  q7,  [x10, #((2 + 5)*32 + 0)]

        stp             q8,  q9,  [x11, #((0 + 5)*32 + 0)]
        stp             q12, q13, [x11, #((2 + 5)*32 + 0)]
        stp             q10, q11, [x12, #((0 + 5)*32 + 0)]
        stp             q14, q15, [x12, #((2 + 5)*32 + 0)]
        add             x10, x1, x21
        add             x11, x1, x21, lsl #1
        add             x12, x1, x22

        add             x10, x1, x21
        add             x11, x1, x21, lsl #1
        add             x12, x1, x22

        ldp             q0,  q1,  [x1,  #((0 + 0)*32 + 8*32)]
        ldp             q4,  q5,  [x1,  #((2 + 0)*32 + 8*32)]
        ldp             q2,  q3,  [x10, #((0 + 0)*32 + 8*32)]
        ldp             q6,  q7,  [x10, #((2 + 0)*32 + 8*32)]

        ldp             q8,  q9,  [x11, #((0 + 0)*32 + 8*32)]
        ldp             q10, q11, [x11, #((2 + 0)*32 + 8*32)]
        ldp             q12, q13, [x12, #((0 + 0)*32 + 8*32)]
        ldp             q14, q15, [x12, #((2 + 0)*32 + 8*32)]


        ld1             { v24.4s, v25.4s }, [x8], x9
        ld1             { v16.4s, v17.4s }, [x7], #32

        ext8             v18.16b, v24.16b, v24.16b, #8
        ext8             v19.16b, v25.16b, v25.16b, #8

        trn1            v20.4s, v16.4s, v16.4s      
        trn2            v16.4s, v16.4s, v16.4s      
        trn1            v21.4s, v17.4s, v17.4s      
        trn2            v17.4s, v17.4s, v17.4s      

        rev64           v22.4s, v8.4s                 
        rev64           v23.4s, v10.4s                 
        rev64           v24.4s, v12.4s                 
        rev64           v25.4s, v14.4s                 

        fmul            v22.4s, v22.4s, v20.4s      
        fmul            v23.4s, v23.4s, v16.4s      
        fmul            v24.4s, v24.4s, v20.4s      
        fmul            v25.4s, v25.4s, v16.4s      

        rev64           v26.4s, v9.4s                 
        rev64           v27.4s, v11.4s                 
        rev64           v20.4s, v13.4s                 
        rev64           v16.4s, v15.4s                 

        fmul            v26.4s, v26.4s, v21.4s      
        fmul            v27.4s, v27.4s, v17.4s      
        fmul            v20.4s, v20.4s, v21.4s      
        fmul            v16.4s, v16.4s, v17.4s      

        trn1            v21.4s, v19.4s, v19.4s      
        trn2            v19.4s, v19.4s, v19.4s      
        trn1            v17.4s, v18.4s, v18.4s      
        trn2            v18.4s, v18.4s, v18.4s      

        fmul            v21.4s, v21.4s, v31.4s
        fmul            v19.4s, v19.4s, v31.4s
        fmul            v17.4s, v17.4s, v31.4s
        fmul            v18.4s, v18.4s, v31.4s

        fmla            v23.4s, v10.4s, v21.4s      
        fmls            v25.4s, v14.4s, v21.4s      
        fmla            v22.4s, v8.4s, v19.4s      
        fmls            v24.4s, v12.4s, v19.4s      

        fmla            v26.4s, v9.4s, v18.4s      
        fmla            v27.4s, v11.4s, v17.4s      
        fmls            v20.4s, v13.4s, v18.4s      
        fmls            v16.4s, v15.4s, v17.4s      

        fsub            v18.4s, v23.4s, v25.4s
        fsub            v17.4s, v24.4s, v22.4s
        fsub            v19.4s, v20.4s, v26.4s
        fsub            v21.4s, v16.4s, v27.4s

        fadd            v22.4s, v24.4s, v22.4s
        fadd            v23.4s, v25.4s, v23.4s
        fadd            v24.4s, v20.4s, v26.4s
        fadd            v25.4s, v16.4s, v27.4s

        fmul            v17.4s, v17.4s, v31.4s
        fmul            v18.4s, v18.4s, v31.4s
        fmul            v19.4s, v19.4s, v31.4s
        fmul            v21.4s, v21.4s, v31.4s

        rev64           v22.4s, v22.4s
        rev64           v24.4s, v24.4s
        rev64           v23.4s, v23.4s
        rev64           v25.4s, v25.4s

        fsub            v8.4s, v0.4s, v22.4s
        fsub            v9.4s, v1.4s, v24.4s
        fsub            v10.4s, v2.4s, v17.4s
        fsub            v11.4s, v3.4s, v19.4s

        fsub            v12.4s, v4.4s, v23.4s
        fsub            v13.4s, v5.4s, v25.4s
        fadd            v14.4s, v6.4s, v18.4s
        fsub            v15.4s, v7.4s, v21.4s

        fadd            v0.4s, v0.4s, v22.4s
        fadd            v1.4s, v1.4s, v24.4s
        fadd            v2.4s, v2.4s, v17.4s
        fadd            v3.4s, v3.4s, v19.4s

        fadd            v4.4s, v4.4s, v23.4s
        fsub            v6.4s, v6.4s, v18.4s      
        fadd            v5.4s, v5.4s, v25.4s      
        fadd            v7.4s, v7.4s, v21.4s

        stp             q0,  q1,  [x1,  #((0 + 0)*32 + 8*32)]
        stp             q4,  q5,  [x1,  #((2 + 0)*32 + 8*32)]
        stp             q2,  q3,  [x10, #((0 + 0)*32 + 8*32)]
        stp             q6,  q7,  [x10, #((2 + 0)*32 + 8*32)]

        stp             q8,  q9,  [x11, #((0 + 0)*32 + 8*32)]
        stp             q12, q13, [x11, #((2 + 0)*32 + 8*32)]
        stp             q10, q11, [x12, #((0 + 0)*32 + 8*32)]
        stp             q14, q15, [x12, #((2 + 0)*32 + 8*32)]
        add             x10, x1, x21
        add             x11, x1, x21, lsl #1
        add             x12, x1, x22

        ldp             q0,  q1,  [x1,  #((0 + 1)*32 + 8*32)]
        ldp             q4,  q5,  [x1,  #((2 + 1)*32 + 8*32)]
        ldp             q2,  q3,  [x10, #((0 + 1)*32 + 8*32)]
        ldp             q6,  q7,  [x10, #((2 + 1)*32 + 8*32)]

        ldp             q8,  q9,  [x11, #((0 + 1)*32 + 8*32)]
        ldp             q10, q11, [x11, #((2 + 1)*32 + 8*32)]
        ldp             q12, q13, [x12, #((0 + 1)*32 + 8*32)]
        ldp             q14, q15, [x12, #((2 + 1)*32 + 8*32)]


        ld1             { v24.4s, v25.4s }, [x8], x9
        ld1             { v16.4s, v17.4s }, [x7], #32

        ext8             v18.16b, v24.16b, v24.16b, #8
        ext8             v19.16b, v25.16b, v25.16b, #8

        trn1            v20.4s, v16.4s, v16.4s      
        trn2            v16.4s, v16.4s, v16.4s      
        trn1            v21.4s, v17.4s, v17.4s      
        trn2            v17.4s, v17.4s, v17.4s      

        rev64           v22.4s, v8.4s                 
        rev64           v23.4s, v10.4s                 
        rev64           v24.4s, v12.4s                 
        rev64           v25.4s, v14.4s                 

        fmul            v22.4s, v22.4s, v20.4s      
        fmul            v23.4s, v23.4s, v16.4s      
        fmul            v24.4s, v24.4s, v20.4s      
        fmul            v25.4s, v25.4s, v16.4s      

        rev64           v26.4s, v9.4s                 
        rev64           v27.4s, v11.4s                 
        rev64           v20.4s, v13.4s                 
        rev64           v16.4s, v15.4s                 

        fmul            v26.4s, v26.4s, v21.4s      
        fmul            v27.4s, v27.4s, v17.4s      
        fmul            v20.4s, v20.4s, v21.4s      
        fmul            v16.4s, v16.4s, v17.4s      

        trn1            v21.4s, v19.4s, v19.4s      
        trn2            v19.4s, v19.4s, v19.4s      
        trn1            v17.4s, v18.4s, v18.4s      
        trn2            v18.4s, v18.4s, v18.4s      

        fmul            v21.4s, v21.4s, v31.4s
        fmul            v19.4s, v19.4s, v31.4s
        fmul            v17.4s, v17.4s, v31.4s
        fmul            v18.4s, v18.4s, v31.4s

        fmla            v23.4s, v10.4s, v21.4s      
        fmls            v25.4s, v14.4s, v21.4s      
        fmla            v22.4s, v8.4s, v19.4s      
        fmls            v24.4s, v12.4s, v19.4s      

        fmla            v26.4s, v9.4s, v18.4s      
        fmla            v27.4s, v11.4s, v17.4s      
        fmls            v20.4s, v13.4s, v18.4s      
        fmls            v16.4s, v15.4s, v17.4s      

        fsub            v18.4s, v23.4s, v25.4s
        fsub            v17.4s, v24.4s, v22.4s
        fsub            v19.4s, v20.4s, v26.4s
        fsub            v21.4s, v16.4s, v27.4s

        fadd            v22.4s, v24.4s, v22.4s
        fadd            v23.4s, v25.4s, v23.4s
        fadd            v24.4s, v20.4s, v26.4s
        fadd            v25.4s, v16.4s, v27.4s

        fmul            v17.4s, v17.4s, v31.4s
        fmul            v18.4s, v18.4s, v31.4s
        fmul            v19.4s, v19.4s, v31.4s
        fmul            v21.4s, v21.4s, v31.4s

        rev64           v22.4s, v22.4s
        rev64           v24.4s, v24.4s
        rev64           v23.4s, v23.4s
        rev64           v25.4s, v25.4s

        fsub            v8.4s, v0.4s, v22.4s
        fsub            v9.4s, v1.4s, v24.4s
        fsub            v10.4s, v2.4s, v17.4s
        fsub            v11.4s, v3.4s, v19.4s

        fsub            v12.4s, v4.4s, v23.4s
        fsub            v13.4s, v5.4s, v25.4s
        fadd            v14.4s, v6.4s, v18.4s
        fsub            v15.4s, v7.4s, v21.4s

        fadd            v0.4s, v0.4s, v22.4s
        fadd            v1.4s, v1.4s, v24.4s
        fadd            v2.4s, v2.4s, v17.4s
        fadd            v3.4s, v3.4s, v19.4s

        fadd            v4.4s, v4.4s, v23.4s
        fsub            v6.4s, v6.4s, v18.4s      
        fadd            v5.4s, v5.4s, v25.4s      
        fadd            v7.4s, v7.4s, v21.4s

        stp             q0,  q1,  [x1,  #((0 + 1)*32 + 8*32)]
        stp             q4,  q5,  [x1,  #((2 + 1)*32 + 8*32)]
        stp             q2,  q3,  [x10, #((0 + 1)*32 + 8*32)]
        stp             q6,  q7,  [x10, #((2 + 1)*32 + 8*32)]

        stp             q8,  q9,  [x11, #((0 + 1)*32 + 8*32)]
        stp             q12, q13, [x11, #((2 + 1)*32 + 8*32)]
        stp             q10, q11, [x12, #((0 + 1)*32 + 8*32)]
        stp             q14, q15, [x12, #((2 + 1)*32 + 8*32)]
        add             x10, x1, x21
        add             x11, x1, x21, lsl #1
        add             x12, x1, x22

        ldp             q0,  q1,  [x1,  #((0 + 4)*32 + 8*32)]
        ldp             q4,  q5,  [x1,  #((2 + 4)*32 + 8*32)]
        ldp             q2,  q3,  [x10, #((0 + 4)*32 + 8*32)]
        ldp             q6,  q7,  [x10, #((2 + 4)*32 + 8*32)]

        ldp             q8,  q9,  [x11, #((0 + 4)*32 + 8*32)]
        ldp             q10, q11, [x11, #((2 + 4)*32 + 8*32)]
        ldp             q12, q13, [x12, #((0 + 4)*32 + 8*32)]
        ldp             q14, q15, [x12, #((2 + 4)*32 + 8*32)]


        ld1             { v24.4s, v25.4s }, [x8], x9
        ld1             { v16.4s, v17.4s }, [x7], #32

        ext8             v18.16b, v24.16b, v24.16b, #8
        ext8             v19.16b, v25.16b, v25.16b, #8

        trn1            v20.4s, v16.4s, v16.4s      
        trn2            v16.4s, v16.4s, v16.4s      
        trn1            v21.4s, v17.4s, v17.4s      
        trn2            v17.4s, v17.4s, v17.4s      

        rev64           v22.4s, v8.4s                 
        rev64           v23.4s, v10.4s                 
        rev64           v24.4s, v12.4s                 
        rev64           v25.4s, v14.4s                 

        fmul            v22.4s, v22.4s, v20.4s      
        fmul            v23.4s, v23.4s, v16.4s      
        fmul            v24.4s, v24.4s, v20.4s      
        fmul            v25.4s, v25.4s, v16.4s      

        rev64           v26.4s, v9.4s                 
        rev64           v27.4s, v11.4s                 
        rev64           v20.4s, v13.4s                 
        rev64           v16.4s, v15.4s                 

        fmul            v26.4s, v26.4s, v21.4s      
        fmul            v27.4s, v27.4s, v17.4s      
        fmul            v20.4s, v20.4s, v21.4s      
        fmul            v16.4s, v16.4s, v17.4s      

        trn1            v21.4s, v19.4s, v19.4s      
        trn2            v19.4s, v19.4s, v19.4s      
        trn1            v17.4s, v18.4s, v18.4s      
        trn2            v18.4s, v18.4s, v18.4s      

        fmul            v21.4s, v21.4s, v31.4s
        fmul            v19.4s, v19.4s, v31.4s
        fmul            v17.4s, v17.4s, v31.4s
        fmul            v18.4s, v18.4s, v31.4s

        fmla            v23.4s, v10.4s, v21.4s      
        fmls            v25.4s, v14.4s, v21.4s      
        fmla            v22.4s, v8.4s, v19.4s      
        fmls            v24.4s, v12.4s, v19.4s      

        fmla            v26.4s, v9.4s, v18.4s      
        fmla            v27.4s, v11.4s, v17.4s      
        fmls            v20.4s, v13.4s, v18.4s      
        fmls            v16.4s, v15.4s, v17.4s      

        fsub            v18.4s, v23.4s, v25.4s
        fsub            v17.4s, v24.4s, v22.4s
        fsub            v19.4s, v20.4s, v26.4s
        fsub            v21.4s, v16.4s, v27.4s

        fadd            v22.4s, v24.4s, v22.4s
        fadd            v23.4s, v25.4s, v23.4s
        fadd            v24.4s, v20.4s, v26.4s
        fadd            v25.4s, v16.4s, v27.4s

        fmul            v17.4s, v17.4s, v31.4s
        fmul            v18.4s, v18.4s, v31.4s
        fmul            v19.4s, v19.4s, v31.4s
        fmul            v21.4s, v21.4s, v31.4s

        rev64           v22.4s, v22.4s
        rev64           v24.4s, v24.4s
        rev64           v23.4s, v23.4s
        rev64           v25.4s, v25.4s

        fsub            v8.4s, v0.4s, v22.4s
        fsub            v9.4s, v1.4s, v24.4s
        fsub            v10.4s, v2.4s, v17.4s
        fsub            v11.4s, v3.4s, v19.4s

        fsub            v12.4s, v4.4s, v23.4s
        fsub            v13.4s, v5.4s, v25.4s
        fadd            v14.4s, v6.4s, v18.4s
        fsub            v15.4s, v7.4s, v21.4s

        fadd            v0.4s, v0.4s, v22.4s
        fadd            v1.4s, v1.4s, v24.4s
        fadd            v2.4s, v2.4s, v17.4s
        fadd            v3.4s, v3.4s, v19.4s

        fadd            v4.4s, v4.4s, v23.4s
        fsub            v6.4s, v6.4s, v18.4s      
        fadd            v5.4s, v5.4s, v25.4s      
        fadd            v7.4s, v7.4s, v21.4s

        stp             q0,  q1,  [x1,  #((0 + 4)*32 + 8*32)]
        stp             q4,  q5,  [x1,  #((2 + 4)*32 + 8*32)]
        stp             q2,  q3,  [x10, #((0 + 4)*32 + 8*32)]
        stp             q6,  q7,  [x10, #((2 + 4)*32 + 8*32)]

        stp             q8,  q9,  [x11, #((0 + 4)*32 + 8*32)]
        stp             q12, q13, [x11, #((2 + 4)*32 + 8*32)]
        stp             q10, q11, [x12, #((0 + 4)*32 + 8*32)]
        stp             q14, q15, [x12, #((2 + 4)*32 + 8*32)]
        add             x10, x1, x21
        add             x11, x1, x21, lsl #1
        add             x12, x1, x22

        ldp             q0,  q1,  [x1,  #((0 + 5)*32 + 8*32)]
        ldp             q4,  q5,  [x1,  #((2 + 5)*32 + 8*32)]
        ldp             q2,  q3,  [x10, #((0 + 5)*32 + 8*32)]
        ldp             q6,  q7,  [x10, #((2 + 5)*32 + 8*32)]

        ldp             q8,  q9,  [x11, #((0 + 5)*32 + 8*32)]
        ldp             q10, q11, [x11, #((2 + 5)*32 + 8*32)]
        ldp             q12, q13, [x12, #((0 + 5)*32 + 8*32)]
        ldp             q14, q15, [x12, #((2 + 5)*32 + 8*32)]


        ld1             { v24.4s, v25.4s }, [x8], x9
        ld1             { v16.4s, v17.4s }, [x7], #32

        ext8             v18.16b, v24.16b, v24.16b, #8
        ext8             v19.16b, v25.16b, v25.16b, #8

        trn1            v20.4s, v16.4s, v16.4s      
        trn2            v16.4s, v16.4s, v16.4s      
        trn1            v21.4s, v17.4s, v17.4s      
        trn2            v17.4s, v17.4s, v17.4s      

        rev64           v22.4s, v8.4s                 
        rev64           v23.4s, v10.4s                 
        rev64           v24.4s, v12.4s                 
        rev64           v25.4s, v14.4s                 

        fmul            v22.4s, v22.4s, v20.4s      
        fmul            v23.4s, v23.4s, v16.4s      
        fmul            v24.4s, v24.4s, v20.4s      
        fmul            v25.4s, v25.4s, v16.4s      

        rev64           v26.4s, v9.4s                 
        rev64           v27.4s, v11.4s                 
        rev64           v20.4s, v13.4s                 
        rev64           v16.4s, v15.4s                 

        fmul            v26.4s, v26.4s, v21.4s      
        fmul            v27.4s, v27.4s, v17.4s      
        fmul            v20.4s, v20.4s, v21.4s      
        fmul            v16.4s, v16.4s, v17.4s      

        trn1            v21.4s, v19.4s, v19.4s      
        trn2            v19.4s, v19.4s, v19.4s      
        trn1            v17.4s, v18.4s, v18.4s      
        trn2            v18.4s, v18.4s, v18.4s      

        fmul            v21.4s, v21.4s, v31.4s
        fmul            v19.4s, v19.4s, v31.4s
        fmul            v17.4s, v17.4s, v31.4s
        fmul            v18.4s, v18.4s, v31.4s

        fmla            v23.4s, v10.4s, v21.4s      
        fmls            v25.4s, v14.4s, v21.4s      
        fmla            v22.4s, v8.4s, v19.4s      
        fmls            v24.4s, v12.4s, v19.4s      

        fmla            v26.4s, v9.4s, v18.4s      
        fmla            v27.4s, v11.4s, v17.4s      
        fmls            v20.4s, v13.4s, v18.4s      
        fmls            v16.4s, v15.4s, v17.4s      

        fsub            v18.4s, v23.4s, v25.4s
        fsub            v17.4s, v24.4s, v22.4s
        fsub            v19.4s, v20.4s, v26.4s
        fsub            v21.4s, v16.4s, v27.4s

        fadd            v22.4s, v24.4s, v22.4s
        fadd            v23.4s, v25.4s, v23.4s
        fadd            v24.4s, v20.4s, v26.4s
        fadd            v25.4s, v16.4s, v27.4s

        fmul            v17.4s, v17.4s, v31.4s
        fmul            v18.4s, v18.4s, v31.4s
        fmul            v19.4s, v19.4s, v31.4s
        fmul            v21.4s, v21.4s, v31.4s

        rev64           v22.4s, v22.4s
        rev64           v24.4s, v24.4s
        rev64           v23.4s, v23.4s
        rev64           v25.4s, v25.4s

        fsub            v8.4s, v0.4s, v22.4s
        fsub            v9.4s, v1.4s, v24.4s
        fsub            v10.4s, v2.4s, v17.4s
        fsub            v11.4s, v3.4s, v19.4s

        fsub            v12.4s, v4.4s, v23.4s
        fsub            v13.4s, v5.4s, v25.4s
        fadd            v14.4s, v6.4s, v18.4s
        fsub            v15.4s, v7.4s, v21.4s

        fadd            v0.4s, v0.4s, v22.4s
        fadd            v1.4s, v1.4s, v24.4s
        fadd            v2.4s, v2.4s, v17.4s
        fadd            v3.4s, v3.4s, v19.4s

        fadd            v4.4s, v4.4s, v23.4s
        fsub            v6.4s, v6.4s, v18.4s      
        fadd            v5.4s, v5.4s, v25.4s      
        fadd            v7.4s, v7.4s, v21.4s

        stp             q0,  q1,  [x1,  #((0 + 5)*32 + 8*32)]
        stp             q4,  q5,  [x1,  #((2 + 5)*32 + 8*32)]
        stp             q2,  q3,  [x10, #((0 + 5)*32 + 8*32)]
        stp             q6,  q7,  [x10, #((2 + 5)*32 + 8*32)]

        stp             q8,  q9,  [x11, #((0 + 5)*32 + 8*32)]
        stp             q12, q13, [x11, #((2 + 5)*32 + 8*32)]
        stp             q10, q11, [x12, #((0 + 5)*32 + 8*32)]
        stp             q14, q15, [x12, #((2 + 5)*32 + 8*32)]

        cmp             w20, #256
        bgt            temp_label_56
        ret
temp_label_57
temp_label_56

        stp             x20, x30, [sp, #-16]!
        mov             w20, #128
        add             x1, x1, #64*32
        bl              temp_label_46
        add             x1, x1, #32*32
        bl              temp_label_46
        ldp             x20, x30, [sp], #16
        sub             x1, x1, #96*32

        ldr             w5, =(512 - 4*7)










        adrp            x7, ff_tx_tab_512_float
        add             x7, x7, ff_tx_tab_512_float










        add             x8, x7, x5
        mov             x9, #-32

        mov             x21, #2*512
        add             x22, x21, x21, lsl #1

        cmp             w19, #512
        beq            temp_label_58

        mov             x5, 4
temp_label_59

        add             x10, x1, x21
        add             x11, x1, x21, lsl #1
        add             x12, x1, x22

        add             x10, x1, x21
        add             x11, x1, x21, lsl #1
        add             x12, x1, x22

        ldp             q0,  q1,  [x1,  #((0 + 0)*32 + 0)]
        ldp             q4,  q5,  [x1,  #((2 + 0)*32 + 0)]
        ldp             q2,  q3,  [x10, #((0 + 0)*32 + 0)]
        ldp             q6,  q7,  [x10, #((2 + 0)*32 + 0)]

        ldp             q8,  q9,  [x11, #((0 + 0)*32 + 0)]
        ldp             q10, q11, [x11, #((2 + 0)*32 + 0)]
        ldp             q12, q13, [x12, #((0 + 0)*32 + 0)]
        ldp             q14, q15, [x12, #((2 + 0)*32 + 0)]


        ld1             { v24.4s, v25.4s }, [x8], x9
        ld1             { v16.4s, v17.4s }, [x7], #32

        ext8             v18.16b, v24.16b, v24.16b, #8
        ext8             v19.16b, v25.16b, v25.16b, #8

        trn1            v20.4s, v16.4s, v16.4s      
        trn2            v16.4s, v16.4s, v16.4s      
        trn1            v21.4s, v17.4s, v17.4s      
        trn2            v17.4s, v17.4s, v17.4s      

        rev64           v22.4s, v8.4s                 
        rev64           v23.4s, v10.4s                 
        rev64           v24.4s, v12.4s                 
        rev64           v25.4s, v14.4s                 

        fmul            v22.4s, v22.4s, v20.4s      
        fmul            v23.4s, v23.4s, v16.4s      
        fmul            v24.4s, v24.4s, v20.4s      
        fmul            v25.4s, v25.4s, v16.4s      

        rev64           v26.4s, v9.4s                 
        rev64           v27.4s, v11.4s                 
        rev64           v20.4s, v13.4s                 
        rev64           v16.4s, v15.4s                 

        fmul            v26.4s, v26.4s, v21.4s      
        fmul            v27.4s, v27.4s, v17.4s      
        fmul            v20.4s, v20.4s, v21.4s      
        fmul            v16.4s, v16.4s, v17.4s      

        trn1            v21.4s, v19.4s, v19.4s      
        trn2            v19.4s, v19.4s, v19.4s      
        trn1            v17.4s, v18.4s, v18.4s      
        trn2            v18.4s, v18.4s, v18.4s      

        fmul            v21.4s, v21.4s, v31.4s
        fmul            v19.4s, v19.4s, v31.4s
        fmul            v17.4s, v17.4s, v31.4s
        fmul            v18.4s, v18.4s, v31.4s

        fmla            v23.4s, v10.4s, v21.4s      
        fmls            v25.4s, v14.4s, v21.4s      
        fmla            v22.4s, v8.4s, v19.4s      
        fmls            v24.4s, v12.4s, v19.4s      

        fmla            v26.4s, v9.4s, v18.4s      
        fmla            v27.4s, v11.4s, v17.4s      
        fmls            v20.4s, v13.4s, v18.4s      
        fmls            v16.4s, v15.4s, v17.4s      

        fsub            v18.4s, v23.4s, v25.4s
        fsub            v17.4s, v24.4s, v22.4s
        fsub            v19.4s, v20.4s, v26.4s
        fsub            v21.4s, v16.4s, v27.4s

        fadd            v22.4s, v24.4s, v22.4s
        fadd            v23.4s, v25.4s, v23.4s
        fadd            v24.4s, v20.4s, v26.4s
        fadd            v25.4s, v16.4s, v27.4s

        fmul            v17.4s, v17.4s, v31.4s
        fmul            v18.4s, v18.4s, v31.4s
        fmul            v19.4s, v19.4s, v31.4s
        fmul            v21.4s, v21.4s, v31.4s

        rev64           v22.4s, v22.4s
        rev64           v24.4s, v24.4s
        rev64           v23.4s, v23.4s
        rev64           v25.4s, v25.4s

        fsub            v8.4s, v0.4s, v22.4s
        fsub            v9.4s, v1.4s, v24.4s
        fsub            v10.4s, v2.4s, v17.4s
        fsub            v11.4s, v3.4s, v19.4s

        fsub            v12.4s, v4.4s, v23.4s
        fsub            v13.4s, v5.4s, v25.4s
        fadd            v14.4s, v6.4s, v18.4s
        fsub            v15.4s, v7.4s, v21.4s

        fadd            v0.4s, v0.4s, v22.4s
        fadd            v1.4s, v1.4s, v24.4s
        fadd            v2.4s, v2.4s, v17.4s
        fadd            v3.4s, v3.4s, v19.4s

        fadd            v4.4s, v4.4s, v23.4s
        fsub            v6.4s, v6.4s, v18.4s      
        fadd            v5.4s, v5.4s, v25.4s      
        fadd            v7.4s, v7.4s, v21.4s

        stp             q0,  q1,  [x1,  #((0 + 0)*32 + 0)]
        stp             q4,  q5,  [x1,  #((2 + 0)*32 + 0)]
        stp             q2,  q3,  [x10, #((0 + 0)*32 + 0)]
        stp             q6,  q7,  [x10, #((2 + 0)*32 + 0)]

        stp             q8,  q9,  [x11, #((0 + 0)*32 + 0)]
        stp             q12, q13, [x11, #((2 + 0)*32 + 0)]
        stp             q10, q11, [x12, #((0 + 0)*32 + 0)]
        stp             q14, q15, [x12, #((2 + 0)*32 + 0)]
        add             x10, x1, x21
        add             x11, x1, x21, lsl #1
        add             x12, x1, x22

        ldp             q0,  q1,  [x1,  #((0 + 1)*32 + 0)]
        ldp             q4,  q5,  [x1,  #((2 + 1)*32 + 0)]
        ldp             q2,  q3,  [x10, #((0 + 1)*32 + 0)]
        ldp             q6,  q7,  [x10, #((2 + 1)*32 + 0)]

        ldp             q8,  q9,  [x11, #((0 + 1)*32 + 0)]
        ldp             q10, q11, [x11, #((2 + 1)*32 + 0)]
        ldp             q12, q13, [x12, #((0 + 1)*32 + 0)]
        ldp             q14, q15, [x12, #((2 + 1)*32 + 0)]


        ld1             { v24.4s, v25.4s }, [x8], x9
        ld1             { v16.4s, v17.4s }, [x7], #32

        ext8             v18.16b, v24.16b, v24.16b, #8
        ext8             v19.16b, v25.16b, v25.16b, #8

        trn1            v20.4s, v16.4s, v16.4s      
        trn2            v16.4s, v16.4s, v16.4s      
        trn1            v21.4s, v17.4s, v17.4s      
        trn2            v17.4s, v17.4s, v17.4s      

        rev64           v22.4s, v8.4s                 
        rev64           v23.4s, v10.4s                 
        rev64           v24.4s, v12.4s                 
        rev64           v25.4s, v14.4s                 

        fmul            v22.4s, v22.4s, v20.4s      
        fmul            v23.4s, v23.4s, v16.4s      
        fmul            v24.4s, v24.4s, v20.4s      
        fmul            v25.4s, v25.4s, v16.4s      

        rev64           v26.4s, v9.4s                 
        rev64           v27.4s, v11.4s                 
        rev64           v20.4s, v13.4s                 
        rev64           v16.4s, v15.4s                 

        fmul            v26.4s, v26.4s, v21.4s      
        fmul            v27.4s, v27.4s, v17.4s      
        fmul            v20.4s, v20.4s, v21.4s      
        fmul            v16.4s, v16.4s, v17.4s      

        trn1            v21.4s, v19.4s, v19.4s      
        trn2            v19.4s, v19.4s, v19.4s      
        trn1            v17.4s, v18.4s, v18.4s      
        trn2            v18.4s, v18.4s, v18.4s      

        fmul            v21.4s, v21.4s, v31.4s
        fmul            v19.4s, v19.4s, v31.4s
        fmul            v17.4s, v17.4s, v31.4s
        fmul            v18.4s, v18.4s, v31.4s

        fmla            v23.4s, v10.4s, v21.4s      
        fmls            v25.4s, v14.4s, v21.4s      
        fmla            v22.4s, v8.4s, v19.4s      
        fmls            v24.4s, v12.4s, v19.4s      

        fmla            v26.4s, v9.4s, v18.4s      
        fmla            v27.4s, v11.4s, v17.4s      
        fmls            v20.4s, v13.4s, v18.4s      
        fmls            v16.4s, v15.4s, v17.4s      

        fsub            v18.4s, v23.4s, v25.4s
        fsub            v17.4s, v24.4s, v22.4s
        fsub            v19.4s, v20.4s, v26.4s
        fsub            v21.4s, v16.4s, v27.4s

        fadd            v22.4s, v24.4s, v22.4s
        fadd            v23.4s, v25.4s, v23.4s
        fadd            v24.4s, v20.4s, v26.4s
        fadd            v25.4s, v16.4s, v27.4s

        fmul            v17.4s, v17.4s, v31.4s
        fmul            v18.4s, v18.4s, v31.4s
        fmul            v19.4s, v19.4s, v31.4s
        fmul            v21.4s, v21.4s, v31.4s

        rev64           v22.4s, v22.4s
        rev64           v24.4s, v24.4s
        rev64           v23.4s, v23.4s
        rev64           v25.4s, v25.4s

        fsub            v8.4s, v0.4s, v22.4s
        fsub            v9.4s, v1.4s, v24.4s
        fsub            v10.4s, v2.4s, v17.4s
        fsub            v11.4s, v3.4s, v19.4s

        fsub            v12.4s, v4.4s, v23.4s
        fsub            v13.4s, v5.4s, v25.4s
        fadd            v14.4s, v6.4s, v18.4s
        fsub            v15.4s, v7.4s, v21.4s

        fadd            v0.4s, v0.4s, v22.4s
        fadd            v1.4s, v1.4s, v24.4s
        fadd            v2.4s, v2.4s, v17.4s
        fadd            v3.4s, v3.4s, v19.4s

        fadd            v4.4s, v4.4s, v23.4s
        fsub            v6.4s, v6.4s, v18.4s      
        fadd            v5.4s, v5.4s, v25.4s      
        fadd            v7.4s, v7.4s, v21.4s

        stp             q0,  q1,  [x1,  #((0 + 1)*32 + 0)]
        stp             q4,  q5,  [x1,  #((2 + 1)*32 + 0)]
        stp             q2,  q3,  [x10, #((0 + 1)*32 + 0)]
        stp             q6,  q7,  [x10, #((2 + 1)*32 + 0)]

        stp             q8,  q9,  [x11, #((0 + 1)*32 + 0)]
        stp             q12, q13, [x11, #((2 + 1)*32 + 0)]
        stp             q10, q11, [x12, #((0 + 1)*32 + 0)]
        stp             q14, q15, [x12, #((2 + 1)*32 + 0)]
        add             x10, x1, x21
        add             x11, x1, x21, lsl #1
        add             x12, x1, x22

        ldp             q0,  q1,  [x1,  #((0 + 4)*32 + 0)]
        ldp             q4,  q5,  [x1,  #((2 + 4)*32 + 0)]
        ldp             q2,  q3,  [x10, #((0 + 4)*32 + 0)]
        ldp             q6,  q7,  [x10, #((2 + 4)*32 + 0)]

        ldp             q8,  q9,  [x11, #((0 + 4)*32 + 0)]
        ldp             q10, q11, [x11, #((2 + 4)*32 + 0)]
        ldp             q12, q13, [x12, #((0 + 4)*32 + 0)]
        ldp             q14, q15, [x12, #((2 + 4)*32 + 0)]


        ld1             { v24.4s, v25.4s }, [x8], x9
        ld1             { v16.4s, v17.4s }, [x7], #32

        ext8             v18.16b, v24.16b, v24.16b, #8
        ext8             v19.16b, v25.16b, v25.16b, #8

        trn1            v20.4s, v16.4s, v16.4s      
        trn2            v16.4s, v16.4s, v16.4s      
        trn1            v21.4s, v17.4s, v17.4s      
        trn2            v17.4s, v17.4s, v17.4s      

        rev64           v22.4s, v8.4s                 
        rev64           v23.4s, v10.4s                 
        rev64           v24.4s, v12.4s                 
        rev64           v25.4s, v14.4s                 

        fmul            v22.4s, v22.4s, v20.4s      
        fmul            v23.4s, v23.4s, v16.4s      
        fmul            v24.4s, v24.4s, v20.4s      
        fmul            v25.4s, v25.4s, v16.4s      

        rev64           v26.4s, v9.4s                 
        rev64           v27.4s, v11.4s                 
        rev64           v20.4s, v13.4s                 
        rev64           v16.4s, v15.4s                 

        fmul            v26.4s, v26.4s, v21.4s      
        fmul            v27.4s, v27.4s, v17.4s      
        fmul            v20.4s, v20.4s, v21.4s      
        fmul            v16.4s, v16.4s, v17.4s      

        trn1            v21.4s, v19.4s, v19.4s      
        trn2            v19.4s, v19.4s, v19.4s      
        trn1            v17.4s, v18.4s, v18.4s      
        trn2            v18.4s, v18.4s, v18.4s      

        fmul            v21.4s, v21.4s, v31.4s
        fmul            v19.4s, v19.4s, v31.4s
        fmul            v17.4s, v17.4s, v31.4s
        fmul            v18.4s, v18.4s, v31.4s

        fmla            v23.4s, v10.4s, v21.4s      
        fmls            v25.4s, v14.4s, v21.4s      
        fmla            v22.4s, v8.4s, v19.4s      
        fmls            v24.4s, v12.4s, v19.4s      

        fmla            v26.4s, v9.4s, v18.4s      
        fmla            v27.4s, v11.4s, v17.4s      
        fmls            v20.4s, v13.4s, v18.4s      
        fmls            v16.4s, v15.4s, v17.4s      

        fsub            v18.4s, v23.4s, v25.4s
        fsub            v17.4s, v24.4s, v22.4s
        fsub            v19.4s, v20.4s, v26.4s
        fsub            v21.4s, v16.4s, v27.4s

        fadd            v22.4s, v24.4s, v22.4s
        fadd            v23.4s, v25.4s, v23.4s
        fadd            v24.4s, v20.4s, v26.4s
        fadd            v25.4s, v16.4s, v27.4s

        fmul            v17.4s, v17.4s, v31.4s
        fmul            v18.4s, v18.4s, v31.4s
        fmul            v19.4s, v19.4s, v31.4s
        fmul            v21.4s, v21.4s, v31.4s

        rev64           v22.4s, v22.4s
        rev64           v24.4s, v24.4s
        rev64           v23.4s, v23.4s
        rev64           v25.4s, v25.4s

        fsub            v8.4s, v0.4s, v22.4s
        fsub            v9.4s, v1.4s, v24.4s
        fsub            v10.4s, v2.4s, v17.4s
        fsub            v11.4s, v3.4s, v19.4s

        fsub            v12.4s, v4.4s, v23.4s
        fsub            v13.4s, v5.4s, v25.4s
        fadd            v14.4s, v6.4s, v18.4s
        fsub            v15.4s, v7.4s, v21.4s

        fadd            v0.4s, v0.4s, v22.4s
        fadd            v1.4s, v1.4s, v24.4s
        fadd            v2.4s, v2.4s, v17.4s
        fadd            v3.4s, v3.4s, v19.4s

        fadd            v4.4s, v4.4s, v23.4s
        fsub            v6.4s, v6.4s, v18.4s      
        fadd            v5.4s, v5.4s, v25.4s      
        fadd            v7.4s, v7.4s, v21.4s

        stp             q0,  q1,  [x1,  #((0 + 4)*32 + 0)]
        stp             q4,  q5,  [x1,  #((2 + 4)*32 + 0)]
        stp             q2,  q3,  [x10, #((0 + 4)*32 + 0)]
        stp             q6,  q7,  [x10, #((2 + 4)*32 + 0)]

        stp             q8,  q9,  [x11, #((0 + 4)*32 + 0)]
        stp             q12, q13, [x11, #((2 + 4)*32 + 0)]
        stp             q10, q11, [x12, #((0 + 4)*32 + 0)]
        stp             q14, q15, [x12, #((2 + 4)*32 + 0)]
        add             x10, x1, x21
        add             x11, x1, x21, lsl #1
        add             x12, x1, x22

        ldp             q0,  q1,  [x1,  #((0 + 5)*32 + 0)]
        ldp             q4,  q5,  [x1,  #((2 + 5)*32 + 0)]
        ldp             q2,  q3,  [x10, #((0 + 5)*32 + 0)]
        ldp             q6,  q7,  [x10, #((2 + 5)*32 + 0)]

        ldp             q8,  q9,  [x11, #((0 + 5)*32 + 0)]
        ldp             q10, q11, [x11, #((2 + 5)*32 + 0)]
        ldp             q12, q13, [x12, #((0 + 5)*32 + 0)]
        ldp             q14, q15, [x12, #((2 + 5)*32 + 0)]


        ld1             { v24.4s, v25.4s }, [x8], x9
        ld1             { v16.4s, v17.4s }, [x7], #32

        ext8             v18.16b, v24.16b, v24.16b, #8
        ext8             v19.16b, v25.16b, v25.16b, #8

        trn1            v20.4s, v16.4s, v16.4s      
        trn2            v16.4s, v16.4s, v16.4s      
        trn1            v21.4s, v17.4s, v17.4s      
        trn2            v17.4s, v17.4s, v17.4s      

        rev64           v22.4s, v8.4s                 
        rev64           v23.4s, v10.4s                 
        rev64           v24.4s, v12.4s                 
        rev64           v25.4s, v14.4s                 

        fmul            v22.4s, v22.4s, v20.4s      
        fmul            v23.4s, v23.4s, v16.4s      
        fmul            v24.4s, v24.4s, v20.4s      
        fmul            v25.4s, v25.4s, v16.4s      

        rev64           v26.4s, v9.4s                 
        rev64           v27.4s, v11.4s                 
        rev64           v20.4s, v13.4s                 
        rev64           v16.4s, v15.4s                 

        fmul            v26.4s, v26.4s, v21.4s      
        fmul            v27.4s, v27.4s, v17.4s      
        fmul            v20.4s, v20.4s, v21.4s      
        fmul            v16.4s, v16.4s, v17.4s      

        trn1            v21.4s, v19.4s, v19.4s      
        trn2            v19.4s, v19.4s, v19.4s      
        trn1            v17.4s, v18.4s, v18.4s      
        trn2            v18.4s, v18.4s, v18.4s      

        fmul            v21.4s, v21.4s, v31.4s
        fmul            v19.4s, v19.4s, v31.4s
        fmul            v17.4s, v17.4s, v31.4s
        fmul            v18.4s, v18.4s, v31.4s

        fmla            v23.4s, v10.4s, v21.4s      
        fmls            v25.4s, v14.4s, v21.4s      
        fmla            v22.4s, v8.4s, v19.4s      
        fmls            v24.4s, v12.4s, v19.4s      

        fmla            v26.4s, v9.4s, v18.4s      
        fmla            v27.4s, v11.4s, v17.4s      
        fmls            v20.4s, v13.4s, v18.4s      
        fmls            v16.4s, v15.4s, v17.4s      

        fsub            v18.4s, v23.4s, v25.4s
        fsub            v17.4s, v24.4s, v22.4s
        fsub            v19.4s, v20.4s, v26.4s
        fsub            v21.4s, v16.4s, v27.4s

        fadd            v22.4s, v24.4s, v22.4s
        fadd            v23.4s, v25.4s, v23.4s
        fadd            v24.4s, v20.4s, v26.4s
        fadd            v25.4s, v16.4s, v27.4s

        fmul            v17.4s, v17.4s, v31.4s
        fmul            v18.4s, v18.4s, v31.4s
        fmul            v19.4s, v19.4s, v31.4s
        fmul            v21.4s, v21.4s, v31.4s

        rev64           v22.4s, v22.4s
        rev64           v24.4s, v24.4s
        rev64           v23.4s, v23.4s
        rev64           v25.4s, v25.4s

        fsub            v8.4s, v0.4s, v22.4s
        fsub            v9.4s, v1.4s, v24.4s
        fsub            v10.4s, v2.4s, v17.4s
        fsub            v11.4s, v3.4s, v19.4s

        fsub            v12.4s, v4.4s, v23.4s
        fsub            v13.4s, v5.4s, v25.4s
        fadd            v14.4s, v6.4s, v18.4s
        fsub            v15.4s, v7.4s, v21.4s

        fadd            v0.4s, v0.4s, v22.4s
        fadd            v1.4s, v1.4s, v24.4s
        fadd            v2.4s, v2.4s, v17.4s
        fadd            v3.4s, v3.4s, v19.4s

        fadd            v4.4s, v4.4s, v23.4s
        fsub            v6.4s, v6.4s, v18.4s      
        fadd            v5.4s, v5.4s, v25.4s      
        fadd            v7.4s, v7.4s, v21.4s

        stp             q0,  q1,  [x1,  #((0 + 5)*32 + 0)]
        stp             q4,  q5,  [x1,  #((2 + 5)*32 + 0)]
        stp             q2,  q3,  [x10, #((0 + 5)*32 + 0)]
        stp             q6,  q7,  [x10, #((2 + 5)*32 + 0)]

        stp             q8,  q9,  [x11, #((0 + 5)*32 + 0)]
        stp             q12, q13, [x11, #((2 + 5)*32 + 0)]
        stp             q10, q11, [x12, #((0 + 5)*32 + 0)]
        stp             q14, q15, [x12, #((2 + 5)*32 + 0)]
        add             x1, x1, 8*32
        subs            w5, w5, 1
        bgt            temp_label_59

        cmp             w20, #512
        bgt            temp_label_60

        ret
temp_label_61
temp_label_60

        stp             x20, x30, [sp, #-16]!
        mov             w20, #256
        add             x1, x1, #96*32
        bl              temp_label_46
        add             x1, x1, #64*32
        bl              temp_label_46
        ldp             x20, x30, [sp], #16
        mov             x5, #192*32
        sub             x1, x1, x5

        ldr             w5, =(1024 - 4*7)










        adrp            x7, ff_tx_tab_1024_float
        add             x7, x7, ff_tx_tab_1024_float










        add             x8, x7, x5
        mov             x9, #-32

        mov             x21, #2*1024
        add             x22, x21, x21, lsl #1

        cmp             w19, #1024
        beq            temp_label_62

        mov             w5, 8
temp_label_63

        add             x10, x1, x21
        add             x11, x1, x21, lsl #1
        add             x12, x1, x22

        add             x10, x1, x21
        add             x11, x1, x21, lsl #1
        add             x12, x1, x22

        ldp             q0,  q1,  [x1,  #((0 + 0)*32 + 0)]
        ldp             q4,  q5,  [x1,  #((2 + 0)*32 + 0)]
        ldp             q2,  q3,  [x10, #((0 + 0)*32 + 0)]
        ldp             q6,  q7,  [x10, #((2 + 0)*32 + 0)]

        ldp             q8,  q9,  [x11, #((0 + 0)*32 + 0)]
        ldp             q10, q11, [x11, #((2 + 0)*32 + 0)]
        ldp             q12, q13, [x12, #((0 + 0)*32 + 0)]
        ldp             q14, q15, [x12, #((2 + 0)*32 + 0)]


        ld1             { v24.4s, v25.4s }, [x8], x9
        ld1             { v16.4s, v17.4s }, [x7], #32

        ext8             v18.16b, v24.16b, v24.16b, #8
        ext8             v19.16b, v25.16b, v25.16b, #8

        trn1            v20.4s, v16.4s, v16.4s      
        trn2            v16.4s, v16.4s, v16.4s      
        trn1            v21.4s, v17.4s, v17.4s      
        trn2            v17.4s, v17.4s, v17.4s      

        rev64           v22.4s, v8.4s                 
        rev64           v23.4s, v10.4s                 
        rev64           v24.4s, v12.4s                 
        rev64           v25.4s, v14.4s                 

        fmul            v22.4s, v22.4s, v20.4s      
        fmul            v23.4s, v23.4s, v16.4s      
        fmul            v24.4s, v24.4s, v20.4s      
        fmul            v25.4s, v25.4s, v16.4s      

        rev64           v26.4s, v9.4s                 
        rev64           v27.4s, v11.4s                 
        rev64           v20.4s, v13.4s                 
        rev64           v16.4s, v15.4s                 

        fmul            v26.4s, v26.4s, v21.4s      
        fmul            v27.4s, v27.4s, v17.4s      
        fmul            v20.4s, v20.4s, v21.4s      
        fmul            v16.4s, v16.4s, v17.4s      

        trn1            v21.4s, v19.4s, v19.4s      
        trn2            v19.4s, v19.4s, v19.4s      
        trn1            v17.4s, v18.4s, v18.4s      
        trn2            v18.4s, v18.4s, v18.4s      

        fmul            v21.4s, v21.4s, v31.4s
        fmul            v19.4s, v19.4s, v31.4s
        fmul            v17.4s, v17.4s, v31.4s
        fmul            v18.4s, v18.4s, v31.4s

        fmla            v23.4s, v10.4s, v21.4s      
        fmls            v25.4s, v14.4s, v21.4s      
        fmla            v22.4s, v8.4s, v19.4s      
        fmls            v24.4s, v12.4s, v19.4s      

        fmla            v26.4s, v9.4s, v18.4s      
        fmla            v27.4s, v11.4s, v17.4s      
        fmls            v20.4s, v13.4s, v18.4s      
        fmls            v16.4s, v15.4s, v17.4s      

        fsub            v18.4s, v23.4s, v25.4s
        fsub            v17.4s, v24.4s, v22.4s
        fsub            v19.4s, v20.4s, v26.4s
        fsub            v21.4s, v16.4s, v27.4s

        fadd            v22.4s, v24.4s, v22.4s
        fadd            v23.4s, v25.4s, v23.4s
        fadd            v24.4s, v20.4s, v26.4s
        fadd            v25.4s, v16.4s, v27.4s

        fmul            v17.4s, v17.4s, v31.4s
        fmul            v18.4s, v18.4s, v31.4s
        fmul            v19.4s, v19.4s, v31.4s
        fmul            v21.4s, v21.4s, v31.4s

        rev64           v22.4s, v22.4s
        rev64           v24.4s, v24.4s
        rev64           v23.4s, v23.4s
        rev64           v25.4s, v25.4s

        fsub            v8.4s, v0.4s, v22.4s
        fsub            v9.4s, v1.4s, v24.4s
        fsub            v10.4s, v2.4s, v17.4s
        fsub            v11.4s, v3.4s, v19.4s

        fsub            v12.4s, v4.4s, v23.4s
        fsub            v13.4s, v5.4s, v25.4s
        fadd            v14.4s, v6.4s, v18.4s
        fsub            v15.4s, v7.4s, v21.4s

        fadd            v0.4s, v0.4s, v22.4s
        fadd            v1.4s, v1.4s, v24.4s
        fadd            v2.4s, v2.4s, v17.4s
        fadd            v3.4s, v3.4s, v19.4s

        fadd            v4.4s, v4.4s, v23.4s
        fsub            v6.4s, v6.4s, v18.4s      
        fadd            v5.4s, v5.4s, v25.4s      
        fadd            v7.4s, v7.4s, v21.4s

        stp             q0,  q1,  [x1,  #((0 + 0)*32 + 0)]
        stp             q4,  q5,  [x1,  #((2 + 0)*32 + 0)]
        stp             q2,  q3,  [x10, #((0 + 0)*32 + 0)]
        stp             q6,  q7,  [x10, #((2 + 0)*32 + 0)]

        stp             q8,  q9,  [x11, #((0 + 0)*32 + 0)]
        stp             q12, q13, [x11, #((2 + 0)*32 + 0)]
        stp             q10, q11, [x12, #((0 + 0)*32 + 0)]
        stp             q14, q15, [x12, #((2 + 0)*32 + 0)]
        add             x10, x1, x21
        add             x11, x1, x21, lsl #1
        add             x12, x1, x22

        ldp             q0,  q1,  [x1,  #((0 + 1)*32 + 0)]
        ldp             q4,  q5,  [x1,  #((2 + 1)*32 + 0)]
        ldp             q2,  q3,  [x10, #((0 + 1)*32 + 0)]
        ldp             q6,  q7,  [x10, #((2 + 1)*32 + 0)]

        ldp             q8,  q9,  [x11, #((0 + 1)*32 + 0)]
        ldp             q10, q11, [x11, #((2 + 1)*32 + 0)]
        ldp             q12, q13, [x12, #((0 + 1)*32 + 0)]
        ldp             q14, q15, [x12, #((2 + 1)*32 + 0)]


        ld1             { v24.4s, v25.4s }, [x8], x9
        ld1             { v16.4s, v17.4s }, [x7], #32

        ext8             v18.16b, v24.16b, v24.16b, #8
        ext8             v19.16b, v25.16b, v25.16b, #8

        trn1            v20.4s, v16.4s, v16.4s      
        trn2            v16.4s, v16.4s, v16.4s      
        trn1            v21.4s, v17.4s, v17.4s      
        trn2            v17.4s, v17.4s, v17.4s      

        rev64           v22.4s, v8.4s                 
        rev64           v23.4s, v10.4s                 
        rev64           v24.4s, v12.4s                 
        rev64           v25.4s, v14.4s                 

        fmul            v22.4s, v22.4s, v20.4s      
        fmul            v23.4s, v23.4s, v16.4s      
        fmul            v24.4s, v24.4s, v20.4s      
        fmul            v25.4s, v25.4s, v16.4s      

        rev64           v26.4s, v9.4s                 
        rev64           v27.4s, v11.4s                 
        rev64           v20.4s, v13.4s                 
        rev64           v16.4s, v15.4s                 

        fmul            v26.4s, v26.4s, v21.4s      
        fmul            v27.4s, v27.4s, v17.4s      
        fmul            v20.4s, v20.4s, v21.4s      
        fmul            v16.4s, v16.4s, v17.4s      

        trn1            v21.4s, v19.4s, v19.4s      
        trn2            v19.4s, v19.4s, v19.4s      
        trn1            v17.4s, v18.4s, v18.4s      
        trn2            v18.4s, v18.4s, v18.4s      

        fmul            v21.4s, v21.4s, v31.4s
        fmul            v19.4s, v19.4s, v31.4s
        fmul            v17.4s, v17.4s, v31.4s
        fmul            v18.4s, v18.4s, v31.4s

        fmla            v23.4s, v10.4s, v21.4s      
        fmls            v25.4s, v14.4s, v21.4s      
        fmla            v22.4s, v8.4s, v19.4s      
        fmls            v24.4s, v12.4s, v19.4s      

        fmla            v26.4s, v9.4s, v18.4s      
        fmla            v27.4s, v11.4s, v17.4s      
        fmls            v20.4s, v13.4s, v18.4s      
        fmls            v16.4s, v15.4s, v17.4s      

        fsub            v18.4s, v23.4s, v25.4s
        fsub            v17.4s, v24.4s, v22.4s
        fsub            v19.4s, v20.4s, v26.4s
        fsub            v21.4s, v16.4s, v27.4s

        fadd            v22.4s, v24.4s, v22.4s
        fadd            v23.4s, v25.4s, v23.4s
        fadd            v24.4s, v20.4s, v26.4s
        fadd            v25.4s, v16.4s, v27.4s

        fmul            v17.4s, v17.4s, v31.4s
        fmul            v18.4s, v18.4s, v31.4s
        fmul            v19.4s, v19.4s, v31.4s
        fmul            v21.4s, v21.4s, v31.4s

        rev64           v22.4s, v22.4s
        rev64           v24.4s, v24.4s
        rev64           v23.4s, v23.4s
        rev64           v25.4s, v25.4s

        fsub            v8.4s, v0.4s, v22.4s
        fsub            v9.4s, v1.4s, v24.4s
        fsub            v10.4s, v2.4s, v17.4s
        fsub            v11.4s, v3.4s, v19.4s

        fsub            v12.4s, v4.4s, v23.4s
        fsub            v13.4s, v5.4s, v25.4s
        fadd            v14.4s, v6.4s, v18.4s
        fsub            v15.4s, v7.4s, v21.4s

        fadd            v0.4s, v0.4s, v22.4s
        fadd            v1.4s, v1.4s, v24.4s
        fadd            v2.4s, v2.4s, v17.4s
        fadd            v3.4s, v3.4s, v19.4s

        fadd            v4.4s, v4.4s, v23.4s
        fsub            v6.4s, v6.4s, v18.4s      
        fadd            v5.4s, v5.4s, v25.4s      
        fadd            v7.4s, v7.4s, v21.4s

        stp             q0,  q1,  [x1,  #((0 + 1)*32 + 0)]
        stp             q4,  q5,  [x1,  #((2 + 1)*32 + 0)]
        stp             q2,  q3,  [x10, #((0 + 1)*32 + 0)]
        stp             q6,  q7,  [x10, #((2 + 1)*32 + 0)]

        stp             q8,  q9,  [x11, #((0 + 1)*32 + 0)]
        stp             q12, q13, [x11, #((2 + 1)*32 + 0)]
        stp             q10, q11, [x12, #((0 + 1)*32 + 0)]
        stp             q14, q15, [x12, #((2 + 1)*32 + 0)]
        add             x10, x1, x21
        add             x11, x1, x21, lsl #1
        add             x12, x1, x22

        ldp             q0,  q1,  [x1,  #((0 + 4)*32 + 0)]
        ldp             q4,  q5,  [x1,  #((2 + 4)*32 + 0)]
        ldp             q2,  q3,  [x10, #((0 + 4)*32 + 0)]
        ldp             q6,  q7,  [x10, #((2 + 4)*32 + 0)]

        ldp             q8,  q9,  [x11, #((0 + 4)*32 + 0)]
        ldp             q10, q11, [x11, #((2 + 4)*32 + 0)]
        ldp             q12, q13, [x12, #((0 + 4)*32 + 0)]
        ldp             q14, q15, [x12, #((2 + 4)*32 + 0)]


        ld1             { v24.4s, v25.4s }, [x8], x9
        ld1             { v16.4s, v17.4s }, [x7], #32

        ext8             v18.16b, v24.16b, v24.16b, #8
        ext8             v19.16b, v25.16b, v25.16b, #8

        trn1            v20.4s, v16.4s, v16.4s      
        trn2            v16.4s, v16.4s, v16.4s      
        trn1            v21.4s, v17.4s, v17.4s      
        trn2            v17.4s, v17.4s, v17.4s      

        rev64           v22.4s, v8.4s                 
        rev64           v23.4s, v10.4s                 
        rev64           v24.4s, v12.4s                 
        rev64           v25.4s, v14.4s                 

        fmul            v22.4s, v22.4s, v20.4s      
        fmul            v23.4s, v23.4s, v16.4s      
        fmul            v24.4s, v24.4s, v20.4s      
        fmul            v25.4s, v25.4s, v16.4s      

        rev64           v26.4s, v9.4s                 
        rev64           v27.4s, v11.4s                 
        rev64           v20.4s, v13.4s                 
        rev64           v16.4s, v15.4s                 

        fmul            v26.4s, v26.4s, v21.4s      
        fmul            v27.4s, v27.4s, v17.4s      
        fmul            v20.4s, v20.4s, v21.4s      
        fmul            v16.4s, v16.4s, v17.4s      

        trn1            v21.4s, v19.4s, v19.4s      
        trn2            v19.4s, v19.4s, v19.4s      
        trn1            v17.4s, v18.4s, v18.4s      
        trn2            v18.4s, v18.4s, v18.4s      

        fmul            v21.4s, v21.4s, v31.4s
        fmul            v19.4s, v19.4s, v31.4s
        fmul            v17.4s, v17.4s, v31.4s
        fmul            v18.4s, v18.4s, v31.4s

        fmla            v23.4s, v10.4s, v21.4s      
        fmls            v25.4s, v14.4s, v21.4s      
        fmla            v22.4s, v8.4s, v19.4s      
        fmls            v24.4s, v12.4s, v19.4s      

        fmla            v26.4s, v9.4s, v18.4s      
        fmla            v27.4s, v11.4s, v17.4s      
        fmls            v20.4s, v13.4s, v18.4s      
        fmls            v16.4s, v15.4s, v17.4s      

        fsub            v18.4s, v23.4s, v25.4s
        fsub            v17.4s, v24.4s, v22.4s
        fsub            v19.4s, v20.4s, v26.4s
        fsub            v21.4s, v16.4s, v27.4s

        fadd            v22.4s, v24.4s, v22.4s
        fadd            v23.4s, v25.4s, v23.4s
        fadd            v24.4s, v20.4s, v26.4s
        fadd            v25.4s, v16.4s, v27.4s

        fmul            v17.4s, v17.4s, v31.4s
        fmul            v18.4s, v18.4s, v31.4s
        fmul            v19.4s, v19.4s, v31.4s
        fmul            v21.4s, v21.4s, v31.4s

        rev64           v22.4s, v22.4s
        rev64           v24.4s, v24.4s
        rev64           v23.4s, v23.4s
        rev64           v25.4s, v25.4s

        fsub            v8.4s, v0.4s, v22.4s
        fsub            v9.4s, v1.4s, v24.4s
        fsub            v10.4s, v2.4s, v17.4s
        fsub            v11.4s, v3.4s, v19.4s

        fsub            v12.4s, v4.4s, v23.4s
        fsub            v13.4s, v5.4s, v25.4s
        fadd            v14.4s, v6.4s, v18.4s
        fsub            v15.4s, v7.4s, v21.4s

        fadd            v0.4s, v0.4s, v22.4s
        fadd            v1.4s, v1.4s, v24.4s
        fadd            v2.4s, v2.4s, v17.4s
        fadd            v3.4s, v3.4s, v19.4s

        fadd            v4.4s, v4.4s, v23.4s
        fsub            v6.4s, v6.4s, v18.4s      
        fadd            v5.4s, v5.4s, v25.4s      
        fadd            v7.4s, v7.4s, v21.4s

        stp             q0,  q1,  [x1,  #((0 + 4)*32 + 0)]
        stp             q4,  q5,  [x1,  #((2 + 4)*32 + 0)]
        stp             q2,  q3,  [x10, #((0 + 4)*32 + 0)]
        stp             q6,  q7,  [x10, #((2 + 4)*32 + 0)]

        stp             q8,  q9,  [x11, #((0 + 4)*32 + 0)]
        stp             q12, q13, [x11, #((2 + 4)*32 + 0)]
        stp             q10, q11, [x12, #((0 + 4)*32 + 0)]
        stp             q14, q15, [x12, #((2 + 4)*32 + 0)]
        add             x10, x1, x21
        add             x11, x1, x21, lsl #1
        add             x12, x1, x22

        ldp             q0,  q1,  [x1,  #((0 + 5)*32 + 0)]
        ldp             q4,  q5,  [x1,  #((2 + 5)*32 + 0)]
        ldp             q2,  q3,  [x10, #((0 + 5)*32 + 0)]
        ldp             q6,  q7,  [x10, #((2 + 5)*32 + 0)]

        ldp             q8,  q9,  [x11, #((0 + 5)*32 + 0)]
        ldp             q10, q11, [x11, #((2 + 5)*32 + 0)]
        ldp             q12, q13, [x12, #((0 + 5)*32 + 0)]
        ldp             q14, q15, [x12, #((2 + 5)*32 + 0)]


        ld1             { v24.4s, v25.4s }, [x8], x9
        ld1             { v16.4s, v17.4s }, [x7], #32

        ext8             v18.16b, v24.16b, v24.16b, #8
        ext8             v19.16b, v25.16b, v25.16b, #8

        trn1            v20.4s, v16.4s, v16.4s      
        trn2            v16.4s, v16.4s, v16.4s      
        trn1            v21.4s, v17.4s, v17.4s      
        trn2            v17.4s, v17.4s, v17.4s      

        rev64           v22.4s, v8.4s                 
        rev64           v23.4s, v10.4s                 
        rev64           v24.4s, v12.4s                 
        rev64           v25.4s, v14.4s                 

        fmul            v22.4s, v22.4s, v20.4s      
        fmul            v23.4s, v23.4s, v16.4s      
        fmul            v24.4s, v24.4s, v20.4s      
        fmul            v25.4s, v25.4s, v16.4s      

        rev64           v26.4s, v9.4s                 
        rev64           v27.4s, v11.4s                 
        rev64           v20.4s, v13.4s                 
        rev64           v16.4s, v15.4s                 

        fmul            v26.4s, v26.4s, v21.4s      
        fmul            v27.4s, v27.4s, v17.4s      
        fmul            v20.4s, v20.4s, v21.4s      
        fmul            v16.4s, v16.4s, v17.4s      

        trn1            v21.4s, v19.4s, v19.4s      
        trn2            v19.4s, v19.4s, v19.4s      
        trn1            v17.4s, v18.4s, v18.4s      
        trn2            v18.4s, v18.4s, v18.4s      

        fmul            v21.4s, v21.4s, v31.4s
        fmul            v19.4s, v19.4s, v31.4s
        fmul            v17.4s, v17.4s, v31.4s
        fmul            v18.4s, v18.4s, v31.4s

        fmla            v23.4s, v10.4s, v21.4s      
        fmls            v25.4s, v14.4s, v21.4s      
        fmla            v22.4s, v8.4s, v19.4s      
        fmls            v24.4s, v12.4s, v19.4s      

        fmla            v26.4s, v9.4s, v18.4s      
        fmla            v27.4s, v11.4s, v17.4s      
        fmls            v20.4s, v13.4s, v18.4s      
        fmls            v16.4s, v15.4s, v17.4s      

        fsub            v18.4s, v23.4s, v25.4s
        fsub            v17.4s, v24.4s, v22.4s
        fsub            v19.4s, v20.4s, v26.4s
        fsub            v21.4s, v16.4s, v27.4s

        fadd            v22.4s, v24.4s, v22.4s
        fadd            v23.4s, v25.4s, v23.4s
        fadd            v24.4s, v20.4s, v26.4s
        fadd            v25.4s, v16.4s, v27.4s

        fmul            v17.4s, v17.4s, v31.4s
        fmul            v18.4s, v18.4s, v31.4s
        fmul            v19.4s, v19.4s, v31.4s
        fmul            v21.4s, v21.4s, v31.4s

        rev64           v22.4s, v22.4s
        rev64           v24.4s, v24.4s
        rev64           v23.4s, v23.4s
        rev64           v25.4s, v25.4s

        fsub            v8.4s, v0.4s, v22.4s
        fsub            v9.4s, v1.4s, v24.4s
        fsub            v10.4s, v2.4s, v17.4s
        fsub            v11.4s, v3.4s, v19.4s

        fsub            v12.4s, v4.4s, v23.4s
        fsub            v13.4s, v5.4s, v25.4s
        fadd            v14.4s, v6.4s, v18.4s
        fsub            v15.4s, v7.4s, v21.4s

        fadd            v0.4s, v0.4s, v22.4s
        fadd            v1.4s, v1.4s, v24.4s
        fadd            v2.4s, v2.4s, v17.4s
        fadd            v3.4s, v3.4s, v19.4s

        fadd            v4.4s, v4.4s, v23.4s
        fsub            v6.4s, v6.4s, v18.4s      
        fadd            v5.4s, v5.4s, v25.4s      
        fadd            v7.4s, v7.4s, v21.4s

        stp             q0,  q1,  [x1,  #((0 + 5)*32 + 0)]
        stp             q4,  q5,  [x1,  #((2 + 5)*32 + 0)]
        stp             q2,  q3,  [x10, #((0 + 5)*32 + 0)]
        stp             q6,  q7,  [x10, #((2 + 5)*32 + 0)]

        stp             q8,  q9,  [x11, #((0 + 5)*32 + 0)]
        stp             q12, q13, [x11, #((2 + 5)*32 + 0)]
        stp             q10, q11, [x12, #((0 + 5)*32 + 0)]
        stp             q14, q15, [x12, #((2 + 5)*32 + 0)]
        add             x1, x1, 8*32
        subs            w5, w5, 1
        bgt            temp_label_63

        cmp             w20, #1024
        bgt            temp_label_64

        ret

temp_label_65
temp_label_64

        stp             x20, x30, [sp, #-16]!
        mov             w20, #(2048/4)
        mov             x5, #((2048*4) - (2048/1))
        add             x1, x1, x5
        bl              temp_label_46
        mov             x5, #((2048*2) - (2048/2))
        add             x1, x1, x5
        bl              temp_label_46
        ldp             x20, x30, [sp], #16
        ldr             w5, =(2048*6 + 2048/2)
        sub             x1, x1, x5

        ldr             w5, =(2048 - 4*7)










        adrp            x7, ff_tx_tab_2048_float
        add             x7, x7, ff_tx_tab_2048_float










        add             x8, x7, x5
        mov             x9, #-32

        mov             x21, #2*2048
        add             x22, x21, x21, lsl #1

        cmp             w19, #(2048)
        beq            temp_label_66

        mov             w5, #(2048/128)
temp_label_67

        add             x10, x1, x21
        add             x11, x1, x21, lsl #1
        add             x12, x1, x22

        add             x10, x1, x21
        add             x11, x1, x21, lsl #1
        add             x12, x1, x22

        ldp             q0,  q1,  [x1,  #((0 + 0)*32 + 0)]
        ldp             q4,  q5,  [x1,  #((2 + 0)*32 + 0)]
        ldp             q2,  q3,  [x10, #((0 + 0)*32 + 0)]
        ldp             q6,  q7,  [x10, #((2 + 0)*32 + 0)]

        ldp             q8,  q9,  [x11, #((0 + 0)*32 + 0)]
        ldp             q10, q11, [x11, #((2 + 0)*32 + 0)]
        ldp             q12, q13, [x12, #((0 + 0)*32 + 0)]
        ldp             q14, q15, [x12, #((2 + 0)*32 + 0)]


        ld1             { v24.4s, v25.4s }, [x8], x9
        ld1             { v16.4s, v17.4s }, [x7], #32

        ext8             v18.16b, v24.16b, v24.16b, #8
        ext8             v19.16b, v25.16b, v25.16b, #8

        trn1            v20.4s, v16.4s, v16.4s      
        trn2            v16.4s, v16.4s, v16.4s      
        trn1            v21.4s, v17.4s, v17.4s      
        trn2            v17.4s, v17.4s, v17.4s      

        rev64           v22.4s, v8.4s                 
        rev64           v23.4s, v10.4s                 
        rev64           v24.4s, v12.4s                 
        rev64           v25.4s, v14.4s                 

        fmul            v22.4s, v22.4s, v20.4s      
        fmul            v23.4s, v23.4s, v16.4s      
        fmul            v24.4s, v24.4s, v20.4s      
        fmul            v25.4s, v25.4s, v16.4s      

        rev64           v26.4s, v9.4s                 
        rev64           v27.4s, v11.4s                 
        rev64           v20.4s, v13.4s                 
        rev64           v16.4s, v15.4s                 

        fmul            v26.4s, v26.4s, v21.4s      
        fmul            v27.4s, v27.4s, v17.4s      
        fmul            v20.4s, v20.4s, v21.4s      
        fmul            v16.4s, v16.4s, v17.4s      

        trn1            v21.4s, v19.4s, v19.4s      
        trn2            v19.4s, v19.4s, v19.4s      
        trn1            v17.4s, v18.4s, v18.4s      
        trn2            v18.4s, v18.4s, v18.4s      

        fmul            v21.4s, v21.4s, v31.4s
        fmul            v19.4s, v19.4s, v31.4s
        fmul            v17.4s, v17.4s, v31.4s
        fmul            v18.4s, v18.4s, v31.4s

        fmla            v23.4s, v10.4s, v21.4s      
        fmls            v25.4s, v14.4s, v21.4s      
        fmla            v22.4s, v8.4s, v19.4s      
        fmls            v24.4s, v12.4s, v19.4s      

        fmla            v26.4s, v9.4s, v18.4s      
        fmla            v27.4s, v11.4s, v17.4s      
        fmls            v20.4s, v13.4s, v18.4s      
        fmls            v16.4s, v15.4s, v17.4s      

        fsub            v18.4s, v23.4s, v25.4s
        fsub            v17.4s, v24.4s, v22.4s
        fsub            v19.4s, v20.4s, v26.4s
        fsub            v21.4s, v16.4s, v27.4s

        fadd            v22.4s, v24.4s, v22.4s
        fadd            v23.4s, v25.4s, v23.4s
        fadd            v24.4s, v20.4s, v26.4s
        fadd            v25.4s, v16.4s, v27.4s

        fmul            v17.4s, v17.4s, v31.4s
        fmul            v18.4s, v18.4s, v31.4s
        fmul            v19.4s, v19.4s, v31.4s
        fmul            v21.4s, v21.4s, v31.4s

        rev64           v22.4s, v22.4s
        rev64           v24.4s, v24.4s
        rev64           v23.4s, v23.4s
        rev64           v25.4s, v25.4s

        fsub            v8.4s, v0.4s, v22.4s
        fsub            v9.4s, v1.4s, v24.4s
        fsub            v10.4s, v2.4s, v17.4s
        fsub            v11.4s, v3.4s, v19.4s

        fsub            v12.4s, v4.4s, v23.4s
        fsub            v13.4s, v5.4s, v25.4s
        fadd            v14.4s, v6.4s, v18.4s
        fsub            v15.4s, v7.4s, v21.4s

        fadd            v0.4s, v0.4s, v22.4s
        fadd            v1.4s, v1.4s, v24.4s
        fadd            v2.4s, v2.4s, v17.4s
        fadd            v3.4s, v3.4s, v19.4s

        fadd            v4.4s, v4.4s, v23.4s
        fsub            v6.4s, v6.4s, v18.4s      
        fadd            v5.4s, v5.4s, v25.4s      
        fadd            v7.4s, v7.4s, v21.4s

        stp             q0,  q1,  [x1,  #((0 + 0)*32 + 0)]
        stp             q4,  q5,  [x1,  #((2 + 0)*32 + 0)]
        stp             q2,  q3,  [x10, #((0 + 0)*32 + 0)]
        stp             q6,  q7,  [x10, #((2 + 0)*32 + 0)]

        stp             q8,  q9,  [x11, #((0 + 0)*32 + 0)]
        stp             q12, q13, [x11, #((2 + 0)*32 + 0)]
        stp             q10, q11, [x12, #((0 + 0)*32 + 0)]
        stp             q14, q15, [x12, #((2 + 0)*32 + 0)]
        add             x10, x1, x21
        add             x11, x1, x21, lsl #1
        add             x12, x1, x22

        ldp             q0,  q1,  [x1,  #((0 + 1)*32 + 0)]
        ldp             q4,  q5,  [x1,  #((2 + 1)*32 + 0)]
        ldp             q2,  q3,  [x10, #((0 + 1)*32 + 0)]
        ldp             q6,  q7,  [x10, #((2 + 1)*32 + 0)]

        ldp             q8,  q9,  [x11, #((0 + 1)*32 + 0)]
        ldp             q10, q11, [x11, #((2 + 1)*32 + 0)]
        ldp             q12, q13, [x12, #((0 + 1)*32 + 0)]
        ldp             q14, q15, [x12, #((2 + 1)*32 + 0)]


        ld1             { v24.4s, v25.4s }, [x8], x9
        ld1             { v16.4s, v17.4s }, [x7], #32

        ext8             v18.16b, v24.16b, v24.16b, #8
        ext8             v19.16b, v25.16b, v25.16b, #8

        trn1            v20.4s, v16.4s, v16.4s      
        trn2            v16.4s, v16.4s, v16.4s      
        trn1            v21.4s, v17.4s, v17.4s      
        trn2            v17.4s, v17.4s, v17.4s      

        rev64           v22.4s, v8.4s                 
        rev64           v23.4s, v10.4s                 
        rev64           v24.4s, v12.4s                 
        rev64           v25.4s, v14.4s                 

        fmul            v22.4s, v22.4s, v20.4s      
        fmul            v23.4s, v23.4s, v16.4s      
        fmul            v24.4s, v24.4s, v20.4s      
        fmul            v25.4s, v25.4s, v16.4s      

        rev64           v26.4s, v9.4s                 
        rev64           v27.4s, v11.4s                 
        rev64           v20.4s, v13.4s                 
        rev64           v16.4s, v15.4s                 

        fmul            v26.4s, v26.4s, v21.4s      
        fmul            v27.4s, v27.4s, v17.4s      
        fmul            v20.4s, v20.4s, v21.4s      
        fmul            v16.4s, v16.4s, v17.4s      

        trn1            v21.4s, v19.4s, v19.4s      
        trn2            v19.4s, v19.4s, v19.4s      
        trn1            v17.4s, v18.4s, v18.4s      
        trn2            v18.4s, v18.4s, v18.4s      

        fmul            v21.4s, v21.4s, v31.4s
        fmul            v19.4s, v19.4s, v31.4s
        fmul            v17.4s, v17.4s, v31.4s
        fmul            v18.4s, v18.4s, v31.4s

        fmla            v23.4s, v10.4s, v21.4s      
        fmls            v25.4s, v14.4s, v21.4s      
        fmla            v22.4s, v8.4s, v19.4s      
        fmls            v24.4s, v12.4s, v19.4s      

        fmla            v26.4s, v9.4s, v18.4s      
        fmla            v27.4s, v11.4s, v17.4s      
        fmls            v20.4s, v13.4s, v18.4s      
        fmls            v16.4s, v15.4s, v17.4s      

        fsub            v18.4s, v23.4s, v25.4s
        fsub            v17.4s, v24.4s, v22.4s
        fsub            v19.4s, v20.4s, v26.4s
        fsub            v21.4s, v16.4s, v27.4s

        fadd            v22.4s, v24.4s, v22.4s
        fadd            v23.4s, v25.4s, v23.4s
        fadd            v24.4s, v20.4s, v26.4s
        fadd            v25.4s, v16.4s, v27.4s

        fmul            v17.4s, v17.4s, v31.4s
        fmul            v18.4s, v18.4s, v31.4s
        fmul            v19.4s, v19.4s, v31.4s
        fmul            v21.4s, v21.4s, v31.4s

        rev64           v22.4s, v22.4s
        rev64           v24.4s, v24.4s
        rev64           v23.4s, v23.4s
        rev64           v25.4s, v25.4s

        fsub            v8.4s, v0.4s, v22.4s
        fsub            v9.4s, v1.4s, v24.4s
        fsub            v10.4s, v2.4s, v17.4s
        fsub            v11.4s, v3.4s, v19.4s

        fsub            v12.4s, v4.4s, v23.4s
        fsub            v13.4s, v5.4s, v25.4s
        fadd            v14.4s, v6.4s, v18.4s
        fsub            v15.4s, v7.4s, v21.4s

        fadd            v0.4s, v0.4s, v22.4s
        fadd            v1.4s, v1.4s, v24.4s
        fadd            v2.4s, v2.4s, v17.4s
        fadd            v3.4s, v3.4s, v19.4s

        fadd            v4.4s, v4.4s, v23.4s
        fsub            v6.4s, v6.4s, v18.4s      
        fadd            v5.4s, v5.4s, v25.4s      
        fadd            v7.4s, v7.4s, v21.4s

        stp             q0,  q1,  [x1,  #((0 + 1)*32 + 0)]
        stp             q4,  q5,  [x1,  #((2 + 1)*32 + 0)]
        stp             q2,  q3,  [x10, #((0 + 1)*32 + 0)]
        stp             q6,  q7,  [x10, #((2 + 1)*32 + 0)]

        stp             q8,  q9,  [x11, #((0 + 1)*32 + 0)]
        stp             q12, q13, [x11, #((2 + 1)*32 + 0)]
        stp             q10, q11, [x12, #((0 + 1)*32 + 0)]
        stp             q14, q15, [x12, #((2 + 1)*32 + 0)]
        add             x10, x1, x21
        add             x11, x1, x21, lsl #1
        add             x12, x1, x22

        ldp             q0,  q1,  [x1,  #((0 + 4)*32 + 0)]
        ldp             q4,  q5,  [x1,  #((2 + 4)*32 + 0)]
        ldp             q2,  q3,  [x10, #((0 + 4)*32 + 0)]
        ldp             q6,  q7,  [x10, #((2 + 4)*32 + 0)]

        ldp             q8,  q9,  [x11, #((0 + 4)*32 + 0)]
        ldp             q10, q11, [x11, #((2 + 4)*32 + 0)]
        ldp             q12, q13, [x12, #((0 + 4)*32 + 0)]
        ldp             q14, q15, [x12, #((2 + 4)*32 + 0)]


        ld1             { v24.4s, v25.4s }, [x8], x9
        ld1             { v16.4s, v17.4s }, [x7], #32

        ext8             v18.16b, v24.16b, v24.16b, #8
        ext8             v19.16b, v25.16b, v25.16b, #8

        trn1            v20.4s, v16.4s, v16.4s      
        trn2            v16.4s, v16.4s, v16.4s      
        trn1            v21.4s, v17.4s, v17.4s      
        trn2            v17.4s, v17.4s, v17.4s      

        rev64           v22.4s, v8.4s                 
        rev64           v23.4s, v10.4s                 
        rev64           v24.4s, v12.4s                 
        rev64           v25.4s, v14.4s                 

        fmul            v22.4s, v22.4s, v20.4s      
        fmul            v23.4s, v23.4s, v16.4s      
        fmul            v24.4s, v24.4s, v20.4s      
        fmul            v25.4s, v25.4s, v16.4s      

        rev64           v26.4s, v9.4s                 
        rev64           v27.4s, v11.4s                 
        rev64           v20.4s, v13.4s                 
        rev64           v16.4s, v15.4s                 

        fmul            v26.4s, v26.4s, v21.4s      
        fmul            v27.4s, v27.4s, v17.4s      
        fmul            v20.4s, v20.4s, v21.4s      
        fmul            v16.4s, v16.4s, v17.4s      

        trn1            v21.4s, v19.4s, v19.4s      
        trn2            v19.4s, v19.4s, v19.4s      
        trn1            v17.4s, v18.4s, v18.4s      
        trn2            v18.4s, v18.4s, v18.4s      

        fmul            v21.4s, v21.4s, v31.4s
        fmul            v19.4s, v19.4s, v31.4s
        fmul            v17.4s, v17.4s, v31.4s
        fmul            v18.4s, v18.4s, v31.4s

        fmla            v23.4s, v10.4s, v21.4s      
        fmls            v25.4s, v14.4s, v21.4s      
        fmla            v22.4s, v8.4s, v19.4s      
        fmls            v24.4s, v12.4s, v19.4s      

        fmla            v26.4s, v9.4s, v18.4s      
        fmla            v27.4s, v11.4s, v17.4s      
        fmls            v20.4s, v13.4s, v18.4s      
        fmls            v16.4s, v15.4s, v17.4s      

        fsub            v18.4s, v23.4s, v25.4s
        fsub            v17.4s, v24.4s, v22.4s
        fsub            v19.4s, v20.4s, v26.4s
        fsub            v21.4s, v16.4s, v27.4s

        fadd            v22.4s, v24.4s, v22.4s
        fadd            v23.4s, v25.4s, v23.4s
        fadd            v24.4s, v20.4s, v26.4s
        fadd            v25.4s, v16.4s, v27.4s

        fmul            v17.4s, v17.4s, v31.4s
        fmul            v18.4s, v18.4s, v31.4s
        fmul            v19.4s, v19.4s, v31.4s
        fmul            v21.4s, v21.4s, v31.4s

        rev64           v22.4s, v22.4s
        rev64           v24.4s, v24.4s
        rev64           v23.4s, v23.4s
        rev64           v25.4s, v25.4s

        fsub            v8.4s, v0.4s, v22.4s
        fsub            v9.4s, v1.4s, v24.4s
        fsub            v10.4s, v2.4s, v17.4s
        fsub            v11.4s, v3.4s, v19.4s

        fsub            v12.4s, v4.4s, v23.4s
        fsub            v13.4s, v5.4s, v25.4s
        fadd            v14.4s, v6.4s, v18.4s
        fsub            v15.4s, v7.4s, v21.4s

        fadd            v0.4s, v0.4s, v22.4s
        fadd            v1.4s, v1.4s, v24.4s
        fadd            v2.4s, v2.4s, v17.4s
        fadd            v3.4s, v3.4s, v19.4s

        fadd            v4.4s, v4.4s, v23.4s
        fsub            v6.4s, v6.4s, v18.4s      
        fadd            v5.4s, v5.4s, v25.4s      
        fadd            v7.4s, v7.4s, v21.4s

        stp             q0,  q1,  [x1,  #((0 + 4)*32 + 0)]
        stp             q4,  q5,  [x1,  #((2 + 4)*32 + 0)]
        stp             q2,  q3,  [x10, #((0 + 4)*32 + 0)]
        stp             q6,  q7,  [x10, #((2 + 4)*32 + 0)]

        stp             q8,  q9,  [x11, #((0 + 4)*32 + 0)]
        stp             q12, q13, [x11, #((2 + 4)*32 + 0)]
        stp             q10, q11, [x12, #((0 + 4)*32 + 0)]
        stp             q14, q15, [x12, #((2 + 4)*32 + 0)]
        add             x10, x1, x21
        add             x11, x1, x21, lsl #1
        add             x12, x1, x22

        ldp             q0,  q1,  [x1,  #((0 + 5)*32 + 0)]
        ldp             q4,  q5,  [x1,  #((2 + 5)*32 + 0)]
        ldp             q2,  q3,  [x10, #((0 + 5)*32 + 0)]
        ldp             q6,  q7,  [x10, #((2 + 5)*32 + 0)]

        ldp             q8,  q9,  [x11, #((0 + 5)*32 + 0)]
        ldp             q10, q11, [x11, #((2 + 5)*32 + 0)]
        ldp             q12, q13, [x12, #((0 + 5)*32 + 0)]
        ldp             q14, q15, [x12, #((2 + 5)*32 + 0)]


        ld1             { v24.4s, v25.4s }, [x8], x9
        ld1             { v16.4s, v17.4s }, [x7], #32

        ext8             v18.16b, v24.16b, v24.16b, #8
        ext8             v19.16b, v25.16b, v25.16b, #8

        trn1            v20.4s, v16.4s, v16.4s      
        trn2            v16.4s, v16.4s, v16.4s      
        trn1            v21.4s, v17.4s, v17.4s      
        trn2            v17.4s, v17.4s, v17.4s      

        rev64           v22.4s, v8.4s                 
        rev64           v23.4s, v10.4s                 
        rev64           v24.4s, v12.4s                 
        rev64           v25.4s, v14.4s                 

        fmul            v22.4s, v22.4s, v20.4s      
        fmul            v23.4s, v23.4s, v16.4s      
        fmul            v24.4s, v24.4s, v20.4s      
        fmul            v25.4s, v25.4s, v16.4s      

        rev64           v26.4s, v9.4s                 
        rev64           v27.4s, v11.4s                 
        rev64           v20.4s, v13.4s                 
        rev64           v16.4s, v15.4s                 

        fmul            v26.4s, v26.4s, v21.4s      
        fmul            v27.4s, v27.4s, v17.4s      
        fmul            v20.4s, v20.4s, v21.4s      
        fmul            v16.4s, v16.4s, v17.4s      

        trn1            v21.4s, v19.4s, v19.4s      
        trn2            v19.4s, v19.4s, v19.4s      
        trn1            v17.4s, v18.4s, v18.4s      
        trn2            v18.4s, v18.4s, v18.4s      

        fmul            v21.4s, v21.4s, v31.4s
        fmul            v19.4s, v19.4s, v31.4s
        fmul            v17.4s, v17.4s, v31.4s
        fmul            v18.4s, v18.4s, v31.4s

        fmla            v23.4s, v10.4s, v21.4s      
        fmls            v25.4s, v14.4s, v21.4s      
        fmla            v22.4s, v8.4s, v19.4s      
        fmls            v24.4s, v12.4s, v19.4s      

        fmla            v26.4s, v9.4s, v18.4s      
        fmla            v27.4s, v11.4s, v17.4s      
        fmls            v20.4s, v13.4s, v18.4s      
        fmls            v16.4s, v15.4s, v17.4s      

        fsub            v18.4s, v23.4s, v25.4s
        fsub            v17.4s, v24.4s, v22.4s
        fsub            v19.4s, v20.4s, v26.4s
        fsub            v21.4s, v16.4s, v27.4s

        fadd            v22.4s, v24.4s, v22.4s
        fadd            v23.4s, v25.4s, v23.4s
        fadd            v24.4s, v20.4s, v26.4s
        fadd            v25.4s, v16.4s, v27.4s

        fmul            v17.4s, v17.4s, v31.4s
        fmul            v18.4s, v18.4s, v31.4s
        fmul            v19.4s, v19.4s, v31.4s
        fmul            v21.4s, v21.4s, v31.4s

        rev64           v22.4s, v22.4s
        rev64           v24.4s, v24.4s
        rev64           v23.4s, v23.4s
        rev64           v25.4s, v25.4s

        fsub            v8.4s, v0.4s, v22.4s
        fsub            v9.4s, v1.4s, v24.4s
        fsub            v10.4s, v2.4s, v17.4s
        fsub            v11.4s, v3.4s, v19.4s

        fsub            v12.4s, v4.4s, v23.4s
        fsub            v13.4s, v5.4s, v25.4s
        fadd            v14.4s, v6.4s, v18.4s
        fsub            v15.4s, v7.4s, v21.4s

        fadd            v0.4s, v0.4s, v22.4s
        fadd            v1.4s, v1.4s, v24.4s
        fadd            v2.4s, v2.4s, v17.4s
        fadd            v3.4s, v3.4s, v19.4s

        fadd            v4.4s, v4.4s, v23.4s
        fsub            v6.4s, v6.4s, v18.4s      
        fadd            v5.4s, v5.4s, v25.4s      
        fadd            v7.4s, v7.4s, v21.4s

        stp             q0,  q1,  [x1,  #((0 + 5)*32 + 0)]
        stp             q4,  q5,  [x1,  #((2 + 5)*32 + 0)]
        stp             q2,  q3,  [x10, #((0 + 5)*32 + 0)]
        stp             q6,  q7,  [x10, #((2 + 5)*32 + 0)]

        stp             q8,  q9,  [x11, #((0 + 5)*32 + 0)]
        stp             q12, q13, [x11, #((2 + 5)*32 + 0)]
        stp             q10, q11, [x12, #((0 + 5)*32 + 0)]
        stp             q14, q15, [x12, #((2 + 5)*32 + 0)]
        add             x1, x1, 8*32
        subs            w5, w5, 1
        bgt            temp_label_67

        cmp             w20, #(2048)
        bgt            temp_label_68
        ret
temp_label_69
temp_label_68

        stp             x20, x30, [sp, #-16]!
        mov             w20, #(4096/4)
        mov             x5, #((4096*4) - (4096/1))
        add             x1, x1, x5
        bl              temp_label_46
        mov             x5, #((4096*2) - (4096/2))
        add             x1, x1, x5
        bl              temp_label_46
        ldp             x20, x30, [sp], #16
        ldr             w5, =(4096*6 + 4096/2)
        sub             x1, x1, x5

        ldr             w5, =(4096 - 4*7)










        adrp            x7, ff_tx_tab_4096_float
        add             x7, x7, ff_tx_tab_4096_float










        add             x8, x7, x5
        mov             x9, #-32

        mov             x21, #2*4096
        add             x22, x21, x21, lsl #1

        cmp             w19, #((4096)/4096), lsl #12
        beq            temp_label_70

        mov             w5, #(4096/128)
temp_label_71

        add             x10, x1, x21
        add             x11, x1, x21, lsl #1
        add             x12, x1, x22

        add             x10, x1, x21
        add             x11, x1, x21, lsl #1
        add             x12, x1, x22

        ldp             q0,  q1,  [x1,  #((0 + 0)*32 + 0)]
        ldp             q4,  q5,  [x1,  #((2 + 0)*32 + 0)]
        ldp             q2,  q3,  [x10, #((0 + 0)*32 + 0)]
        ldp             q6,  q7,  [x10, #((2 + 0)*32 + 0)]

        ldp             q8,  q9,  [x11, #((0 + 0)*32 + 0)]
        ldp             q10, q11, [x11, #((2 + 0)*32 + 0)]
        ldp             q12, q13, [x12, #((0 + 0)*32 + 0)]
        ldp             q14, q15, [x12, #((2 + 0)*32 + 0)]


        ld1             { v24.4s, v25.4s }, [x8], x9
        ld1             { v16.4s, v17.4s }, [x7], #32

        ext8             v18.16b, v24.16b, v24.16b, #8
        ext8             v19.16b, v25.16b, v25.16b, #8

        trn1            v20.4s, v16.4s, v16.4s      
        trn2            v16.4s, v16.4s, v16.4s      
        trn1            v21.4s, v17.4s, v17.4s      
        trn2            v17.4s, v17.4s, v17.4s      

        rev64           v22.4s, v8.4s                 
        rev64           v23.4s, v10.4s                 
        rev64           v24.4s, v12.4s                 
        rev64           v25.4s, v14.4s                 

        fmul            v22.4s, v22.4s, v20.4s      
        fmul            v23.4s, v23.4s, v16.4s      
        fmul            v24.4s, v24.4s, v20.4s      
        fmul            v25.4s, v25.4s, v16.4s      

        rev64           v26.4s, v9.4s                 
        rev64           v27.4s, v11.4s                 
        rev64           v20.4s, v13.4s                 
        rev64           v16.4s, v15.4s                 

        fmul            v26.4s, v26.4s, v21.4s      
        fmul            v27.4s, v27.4s, v17.4s      
        fmul            v20.4s, v20.4s, v21.4s      
        fmul            v16.4s, v16.4s, v17.4s      

        trn1            v21.4s, v19.4s, v19.4s      
        trn2            v19.4s, v19.4s, v19.4s      
        trn1            v17.4s, v18.4s, v18.4s      
        trn2            v18.4s, v18.4s, v18.4s      

        fmul            v21.4s, v21.4s, v31.4s
        fmul            v19.4s, v19.4s, v31.4s
        fmul            v17.4s, v17.4s, v31.4s
        fmul            v18.4s, v18.4s, v31.4s

        fmla            v23.4s, v10.4s, v21.4s      
        fmls            v25.4s, v14.4s, v21.4s      
        fmla            v22.4s, v8.4s, v19.4s      
        fmls            v24.4s, v12.4s, v19.4s      

        fmla            v26.4s, v9.4s, v18.4s      
        fmla            v27.4s, v11.4s, v17.4s      
        fmls            v20.4s, v13.4s, v18.4s      
        fmls            v16.4s, v15.4s, v17.4s      

        fsub            v18.4s, v23.4s, v25.4s
        fsub            v17.4s, v24.4s, v22.4s
        fsub            v19.4s, v20.4s, v26.4s
        fsub            v21.4s, v16.4s, v27.4s

        fadd            v22.4s, v24.4s, v22.4s
        fadd            v23.4s, v25.4s, v23.4s
        fadd            v24.4s, v20.4s, v26.4s
        fadd            v25.4s, v16.4s, v27.4s

        fmul            v17.4s, v17.4s, v31.4s
        fmul            v18.4s, v18.4s, v31.4s
        fmul            v19.4s, v19.4s, v31.4s
        fmul            v21.4s, v21.4s, v31.4s

        rev64           v22.4s, v22.4s
        rev64           v24.4s, v24.4s
        rev64           v23.4s, v23.4s
        rev64           v25.4s, v25.4s

        fsub            v8.4s, v0.4s, v22.4s
        fsub            v9.4s, v1.4s, v24.4s
        fsub            v10.4s, v2.4s, v17.4s
        fsub            v11.4s, v3.4s, v19.4s

        fsub            v12.4s, v4.4s, v23.4s
        fsub            v13.4s, v5.4s, v25.4s
        fadd            v14.4s, v6.4s, v18.4s
        fsub            v15.4s, v7.4s, v21.4s

        fadd            v0.4s, v0.4s, v22.4s
        fadd            v1.4s, v1.4s, v24.4s
        fadd            v2.4s, v2.4s, v17.4s
        fadd            v3.4s, v3.4s, v19.4s

        fadd            v4.4s, v4.4s, v23.4s
        fsub            v6.4s, v6.4s, v18.4s      
        fadd            v5.4s, v5.4s, v25.4s      
        fadd            v7.4s, v7.4s, v21.4s

        stp             q0,  q1,  [x1,  #((0 + 0)*32 + 0)]
        stp             q4,  q5,  [x1,  #((2 + 0)*32 + 0)]
        stp             q2,  q3,  [x10, #((0 + 0)*32 + 0)]
        stp             q6,  q7,  [x10, #((2 + 0)*32 + 0)]

        stp             q8,  q9,  [x11, #((0 + 0)*32 + 0)]
        stp             q12, q13, [x11, #((2 + 0)*32 + 0)]
        stp             q10, q11, [x12, #((0 + 0)*32 + 0)]
        stp             q14, q15, [x12, #((2 + 0)*32 + 0)]
        add             x10, x1, x21
        add             x11, x1, x21, lsl #1
        add             x12, x1, x22

        ldp             q0,  q1,  [x1,  #((0 + 1)*32 + 0)]
        ldp             q4,  q5,  [x1,  #((2 + 1)*32 + 0)]
        ldp             q2,  q3,  [x10, #((0 + 1)*32 + 0)]
        ldp             q6,  q7,  [x10, #((2 + 1)*32 + 0)]

        ldp             q8,  q9,  [x11, #((0 + 1)*32 + 0)]
        ldp             q10, q11, [x11, #((2 + 1)*32 + 0)]
        ldp             q12, q13, [x12, #((0 + 1)*32 + 0)]
        ldp             q14, q15, [x12, #((2 + 1)*32 + 0)]


        ld1             { v24.4s, v25.4s }, [x8], x9
        ld1             { v16.4s, v17.4s }, [x7], #32

        ext8             v18.16b, v24.16b, v24.16b, #8
        ext8             v19.16b, v25.16b, v25.16b, #8

        trn1            v20.4s, v16.4s, v16.4s      
        trn2            v16.4s, v16.4s, v16.4s      
        trn1            v21.4s, v17.4s, v17.4s      
        trn2            v17.4s, v17.4s, v17.4s      

        rev64           v22.4s, v8.4s                 
        rev64           v23.4s, v10.4s                 
        rev64           v24.4s, v12.4s                 
        rev64           v25.4s, v14.4s                 

        fmul            v22.4s, v22.4s, v20.4s      
        fmul            v23.4s, v23.4s, v16.4s      
        fmul            v24.4s, v24.4s, v20.4s      
        fmul            v25.4s, v25.4s, v16.4s      

        rev64           v26.4s, v9.4s                 
        rev64           v27.4s, v11.4s                 
        rev64           v20.4s, v13.4s                 
        rev64           v16.4s, v15.4s                 

        fmul            v26.4s, v26.4s, v21.4s      
        fmul            v27.4s, v27.4s, v17.4s      
        fmul            v20.4s, v20.4s, v21.4s      
        fmul            v16.4s, v16.4s, v17.4s      

        trn1            v21.4s, v19.4s, v19.4s      
        trn2            v19.4s, v19.4s, v19.4s      
        trn1            v17.4s, v18.4s, v18.4s      
        trn2            v18.4s, v18.4s, v18.4s      

        fmul            v21.4s, v21.4s, v31.4s
        fmul            v19.4s, v19.4s, v31.4s
        fmul            v17.4s, v17.4s, v31.4s
        fmul            v18.4s, v18.4s, v31.4s

        fmla            v23.4s, v10.4s, v21.4s      
        fmls            v25.4s, v14.4s, v21.4s      
        fmla            v22.4s, v8.4s, v19.4s      
        fmls            v24.4s, v12.4s, v19.4s      

        fmla            v26.4s, v9.4s, v18.4s      
        fmla            v27.4s, v11.4s, v17.4s      
        fmls            v20.4s, v13.4s, v18.4s      
        fmls            v16.4s, v15.4s, v17.4s      

        fsub            v18.4s, v23.4s, v25.4s
        fsub            v17.4s, v24.4s, v22.4s
        fsub            v19.4s, v20.4s, v26.4s
        fsub            v21.4s, v16.4s, v27.4s

        fadd            v22.4s, v24.4s, v22.4s
        fadd            v23.4s, v25.4s, v23.4s
        fadd            v24.4s, v20.4s, v26.4s
        fadd            v25.4s, v16.4s, v27.4s

        fmul            v17.4s, v17.4s, v31.4s
        fmul            v18.4s, v18.4s, v31.4s
        fmul            v19.4s, v19.4s, v31.4s
        fmul            v21.4s, v21.4s, v31.4s

        rev64           v22.4s, v22.4s
        rev64           v24.4s, v24.4s
        rev64           v23.4s, v23.4s
        rev64           v25.4s, v25.4s

        fsub            v8.4s, v0.4s, v22.4s
        fsub            v9.4s, v1.4s, v24.4s
        fsub            v10.4s, v2.4s, v17.4s
        fsub            v11.4s, v3.4s, v19.4s

        fsub            v12.4s, v4.4s, v23.4s
        fsub            v13.4s, v5.4s, v25.4s
        fadd            v14.4s, v6.4s, v18.4s
        fsub            v15.4s, v7.4s, v21.4s

        fadd            v0.4s, v0.4s, v22.4s
        fadd            v1.4s, v1.4s, v24.4s
        fadd            v2.4s, v2.4s, v17.4s
        fadd            v3.4s, v3.4s, v19.4s

        fadd            v4.4s, v4.4s, v23.4s
        fsub            v6.4s, v6.4s, v18.4s      
        fadd            v5.4s, v5.4s, v25.4s      
        fadd            v7.4s, v7.4s, v21.4s

        stp             q0,  q1,  [x1,  #((0 + 1)*32 + 0)]
        stp             q4,  q5,  [x1,  #((2 + 1)*32 + 0)]
        stp             q2,  q3,  [x10, #((0 + 1)*32 + 0)]
        stp             q6,  q7,  [x10, #((2 + 1)*32 + 0)]

        stp             q8,  q9,  [x11, #((0 + 1)*32 + 0)]
        stp             q12, q13, [x11, #((2 + 1)*32 + 0)]
        stp             q10, q11, [x12, #((0 + 1)*32 + 0)]
        stp             q14, q15, [x12, #((2 + 1)*32 + 0)]
        add             x10, x1, x21
        add             x11, x1, x21, lsl #1
        add             x12, x1, x22

        ldp             q0,  q1,  [x1,  #((0 + 4)*32 + 0)]
        ldp             q4,  q5,  [x1,  #((2 + 4)*32 + 0)]
        ldp             q2,  q3,  [x10, #((0 + 4)*32 + 0)]
        ldp             q6,  q7,  [x10, #((2 + 4)*32 + 0)]

        ldp             q8,  q9,  [x11, #((0 + 4)*32 + 0)]
        ldp             q10, q11, [x11, #((2 + 4)*32 + 0)]
        ldp             q12, q13, [x12, #((0 + 4)*32 + 0)]
        ldp             q14, q15, [x12, #((2 + 4)*32 + 0)]


        ld1             { v24.4s, v25.4s }, [x8], x9
        ld1             { v16.4s, v17.4s }, [x7], #32

        ext8             v18.16b, v24.16b, v24.16b, #8
        ext8             v19.16b, v25.16b, v25.16b, #8

        trn1            v20.4s, v16.4s, v16.4s      
        trn2            v16.4s, v16.4s, v16.4s      
        trn1            v21.4s, v17.4s, v17.4s      
        trn2            v17.4s, v17.4s, v17.4s      

        rev64           v22.4s, v8.4s                 
        rev64           v23.4s, v10.4s                 
        rev64           v24.4s, v12.4s                 
        rev64           v25.4s, v14.4s                 

        fmul            v22.4s, v22.4s, v20.4s      
        fmul            v23.4s, v23.4s, v16.4s      
        fmul            v24.4s, v24.4s, v20.4s      
        fmul            v25.4s, v25.4s, v16.4s      

        rev64           v26.4s, v9.4s                 
        rev64           v27.4s, v11.4s                 
        rev64           v20.4s, v13.4s                 
        rev64           v16.4s, v15.4s                 

        fmul            v26.4s, v26.4s, v21.4s      
        fmul            v27.4s, v27.4s, v17.4s      
        fmul            v20.4s, v20.4s, v21.4s      
        fmul            v16.4s, v16.4s, v17.4s      

        trn1            v21.4s, v19.4s, v19.4s      
        trn2            v19.4s, v19.4s, v19.4s      
        trn1            v17.4s, v18.4s, v18.4s      
        trn2            v18.4s, v18.4s, v18.4s      

        fmul            v21.4s, v21.4s, v31.4s
        fmul            v19.4s, v19.4s, v31.4s
        fmul            v17.4s, v17.4s, v31.4s
        fmul            v18.4s, v18.4s, v31.4s

        fmla            v23.4s, v10.4s, v21.4s      
        fmls            v25.4s, v14.4s, v21.4s      
        fmla            v22.4s, v8.4s, v19.4s      
        fmls            v24.4s, v12.4s, v19.4s      

        fmla            v26.4s, v9.4s, v18.4s      
        fmla            v27.4s, v11.4s, v17.4s      
        fmls            v20.4s, v13.4s, v18.4s      
        fmls            v16.4s, v15.4s, v17.4s      

        fsub            v18.4s, v23.4s, v25.4s
        fsub            v17.4s, v24.4s, v22.4s
        fsub            v19.4s, v20.4s, v26.4s
        fsub            v21.4s, v16.4s, v27.4s

        fadd            v22.4s, v24.4s, v22.4s
        fadd            v23.4s, v25.4s, v23.4s
        fadd            v24.4s, v20.4s, v26.4s
        fadd            v25.4s, v16.4s, v27.4s

        fmul            v17.4s, v17.4s, v31.4s
        fmul            v18.4s, v18.4s, v31.4s
        fmul            v19.4s, v19.4s, v31.4s
        fmul            v21.4s, v21.4s, v31.4s

        rev64           v22.4s, v22.4s
        rev64           v24.4s, v24.4s
        rev64           v23.4s, v23.4s
        rev64           v25.4s, v25.4s

        fsub            v8.4s, v0.4s, v22.4s
        fsub            v9.4s, v1.4s, v24.4s
        fsub            v10.4s, v2.4s, v17.4s
        fsub            v11.4s, v3.4s, v19.4s

        fsub            v12.4s, v4.4s, v23.4s
        fsub            v13.4s, v5.4s, v25.4s
        fadd            v14.4s, v6.4s, v18.4s
        fsub            v15.4s, v7.4s, v21.4s

        fadd            v0.4s, v0.4s, v22.4s
        fadd            v1.4s, v1.4s, v24.4s
        fadd            v2.4s, v2.4s, v17.4s
        fadd            v3.4s, v3.4s, v19.4s

        fadd            v4.4s, v4.4s, v23.4s
        fsub            v6.4s, v6.4s, v18.4s      
        fadd            v5.4s, v5.4s, v25.4s      
        fadd            v7.4s, v7.4s, v21.4s

        stp             q0,  q1,  [x1,  #((0 + 4)*32 + 0)]
        stp             q4,  q5,  [x1,  #((2 + 4)*32 + 0)]
        stp             q2,  q3,  [x10, #((0 + 4)*32 + 0)]
        stp             q6,  q7,  [x10, #((2 + 4)*32 + 0)]

        stp             q8,  q9,  [x11, #((0 + 4)*32 + 0)]
        stp             q12, q13, [x11, #((2 + 4)*32 + 0)]
        stp             q10, q11, [x12, #((0 + 4)*32 + 0)]
        stp             q14, q15, [x12, #((2 + 4)*32 + 0)]
        add             x10, x1, x21
        add             x11, x1, x21, lsl #1
        add             x12, x1, x22

        ldp             q0,  q1,  [x1,  #((0 + 5)*32 + 0)]
        ldp             q4,  q5,  [x1,  #((2 + 5)*32 + 0)]
        ldp             q2,  q3,  [x10, #((0 + 5)*32 + 0)]
        ldp             q6,  q7,  [x10, #((2 + 5)*32 + 0)]

        ldp             q8,  q9,  [x11, #((0 + 5)*32 + 0)]
        ldp             q10, q11, [x11, #((2 + 5)*32 + 0)]
        ldp             q12, q13, [x12, #((0 + 5)*32 + 0)]
        ldp             q14, q15, [x12, #((2 + 5)*32 + 0)]


        ld1             { v24.4s, v25.4s }, [x8], x9
        ld1             { v16.4s, v17.4s }, [x7], #32

        ext8             v18.16b, v24.16b, v24.16b, #8
        ext8             v19.16b, v25.16b, v25.16b, #8

        trn1            v20.4s, v16.4s, v16.4s      
        trn2            v16.4s, v16.4s, v16.4s      
        trn1            v21.4s, v17.4s, v17.4s      
        trn2            v17.4s, v17.4s, v17.4s      

        rev64           v22.4s, v8.4s                 
        rev64           v23.4s, v10.4s                 
        rev64           v24.4s, v12.4s                 
        rev64           v25.4s, v14.4s                 

        fmul            v22.4s, v22.4s, v20.4s      
        fmul            v23.4s, v23.4s, v16.4s      
        fmul            v24.4s, v24.4s, v20.4s      
        fmul            v25.4s, v25.4s, v16.4s      

        rev64           v26.4s, v9.4s                 
        rev64           v27.4s, v11.4s                 
        rev64           v20.4s, v13.4s                 
        rev64           v16.4s, v15.4s                 

        fmul            v26.4s, v26.4s, v21.4s      
        fmul            v27.4s, v27.4s, v17.4s      
        fmul            v20.4s, v20.4s, v21.4s      
        fmul            v16.4s, v16.4s, v17.4s      

        trn1            v21.4s, v19.4s, v19.4s      
        trn2            v19.4s, v19.4s, v19.4s      
        trn1            v17.4s, v18.4s, v18.4s      
        trn2            v18.4s, v18.4s, v18.4s      

        fmul            v21.4s, v21.4s, v31.4s
        fmul            v19.4s, v19.4s, v31.4s
        fmul            v17.4s, v17.4s, v31.4s
        fmul            v18.4s, v18.4s, v31.4s

        fmla            v23.4s, v10.4s, v21.4s      
        fmls            v25.4s, v14.4s, v21.4s      
        fmla            v22.4s, v8.4s, v19.4s      
        fmls            v24.4s, v12.4s, v19.4s      

        fmla            v26.4s, v9.4s, v18.4s      
        fmla            v27.4s, v11.4s, v17.4s      
        fmls            v20.4s, v13.4s, v18.4s      
        fmls            v16.4s, v15.4s, v17.4s      

        fsub            v18.4s, v23.4s, v25.4s
        fsub            v17.4s, v24.4s, v22.4s
        fsub            v19.4s, v20.4s, v26.4s
        fsub            v21.4s, v16.4s, v27.4s

        fadd            v22.4s, v24.4s, v22.4s
        fadd            v23.4s, v25.4s, v23.4s
        fadd            v24.4s, v20.4s, v26.4s
        fadd            v25.4s, v16.4s, v27.4s

        fmul            v17.4s, v17.4s, v31.4s
        fmul            v18.4s, v18.4s, v31.4s
        fmul            v19.4s, v19.4s, v31.4s
        fmul            v21.4s, v21.4s, v31.4s

        rev64           v22.4s, v22.4s
        rev64           v24.4s, v24.4s
        rev64           v23.4s, v23.4s
        rev64           v25.4s, v25.4s

        fsub            v8.4s, v0.4s, v22.4s
        fsub            v9.4s, v1.4s, v24.4s
        fsub            v10.4s, v2.4s, v17.4s
        fsub            v11.4s, v3.4s, v19.4s

        fsub            v12.4s, v4.4s, v23.4s
        fsub            v13.4s, v5.4s, v25.4s
        fadd            v14.4s, v6.4s, v18.4s
        fsub            v15.4s, v7.4s, v21.4s

        fadd            v0.4s, v0.4s, v22.4s
        fadd            v1.4s, v1.4s, v24.4s
        fadd            v2.4s, v2.4s, v17.4s
        fadd            v3.4s, v3.4s, v19.4s

        fadd            v4.4s, v4.4s, v23.4s
        fsub            v6.4s, v6.4s, v18.4s      
        fadd            v5.4s, v5.4s, v25.4s      
        fadd            v7.4s, v7.4s, v21.4s

        stp             q0,  q1,  [x1,  #((0 + 5)*32 + 0)]
        stp             q4,  q5,  [x1,  #((2 + 5)*32 + 0)]
        stp             q2,  q3,  [x10, #((0 + 5)*32 + 0)]
        stp             q6,  q7,  [x10, #((2 + 5)*32 + 0)]

        stp             q8,  q9,  [x11, #((0 + 5)*32 + 0)]
        stp             q12, q13, [x11, #((2 + 5)*32 + 0)]
        stp             q10, q11, [x12, #((0 + 5)*32 + 0)]
        stp             q14, q15, [x12, #((2 + 5)*32 + 0)]
        add             x1, x1, 8*32
        subs            w5, w5, 1
        bgt            temp_label_71

        cmp             w20, #((4096)/4096), lsl #12
        bgt            temp_label_72
        ret
temp_label_73
temp_label_72

        stp             x20, x30, [sp, #-16]!
        mov             w20, #(8192/4)
        mov             x5, #((8192*4) - (8192/1))
        add             x1, x1, x5
        bl              temp_label_46
        mov             x5, #((8192*2) - (8192/2))
        add             x1, x1, x5
        bl              temp_label_46
        ldp             x20, x30, [sp], #16
        ldr             w5, =(8192*6 + 8192/2)
        sub             x1, x1, x5

        ldr             w5, =(8192 - 4*7)










        adrp            x7, ff_tx_tab_8192_float
        add             x7, x7, ff_tx_tab_8192_float










        add             x8, x7, x5
        mov             x9, #-32

        mov             x21, #2*8192
        add             x22, x21, x21, lsl #1

        cmp             w19, #((8192)/4096), lsl #12
        beq            temp_label_74

        mov             w5, #(8192/128)
temp_label_75

        add             x10, x1, x21
        add             x11, x1, x21, lsl #1
        add             x12, x1, x22

        add             x10, x1, x21
        add             x11, x1, x21, lsl #1
        add             x12, x1, x22

        ldp             q0,  q1,  [x1,  #((0 + 0)*32 + 0)]
        ldp             q4,  q5,  [x1,  #((2 + 0)*32 + 0)]
        ldp             q2,  q3,  [x10, #((0 + 0)*32 + 0)]
        ldp             q6,  q7,  [x10, #((2 + 0)*32 + 0)]

        ldp             q8,  q9,  [x11, #((0 + 0)*32 + 0)]
        ldp             q10, q11, [x11, #((2 + 0)*32 + 0)]
        ldp             q12, q13, [x12, #((0 + 0)*32 + 0)]
        ldp             q14, q15, [x12, #((2 + 0)*32 + 0)]


        ld1             { v24.4s, v25.4s }, [x8], x9
        ld1             { v16.4s, v17.4s }, [x7], #32

        ext8             v18.16b, v24.16b, v24.16b, #8
        ext8             v19.16b, v25.16b, v25.16b, #8

        trn1            v20.4s, v16.4s, v16.4s      
        trn2            v16.4s, v16.4s, v16.4s      
        trn1            v21.4s, v17.4s, v17.4s      
        trn2            v17.4s, v17.4s, v17.4s      

        rev64           v22.4s, v8.4s                 
        rev64           v23.4s, v10.4s                 
        rev64           v24.4s, v12.4s                 
        rev64           v25.4s, v14.4s                 

        fmul            v22.4s, v22.4s, v20.4s      
        fmul            v23.4s, v23.4s, v16.4s      
        fmul            v24.4s, v24.4s, v20.4s      
        fmul            v25.4s, v25.4s, v16.4s      

        rev64           v26.4s, v9.4s                 
        rev64           v27.4s, v11.4s                 
        rev64           v20.4s, v13.4s                 
        rev64           v16.4s, v15.4s                 

        fmul            v26.4s, v26.4s, v21.4s      
        fmul            v27.4s, v27.4s, v17.4s      
        fmul            v20.4s, v20.4s, v21.4s      
        fmul            v16.4s, v16.4s, v17.4s      

        trn1            v21.4s, v19.4s, v19.4s      
        trn2            v19.4s, v19.4s, v19.4s      
        trn1            v17.4s, v18.4s, v18.4s      
        trn2            v18.4s, v18.4s, v18.4s      

        fmul            v21.4s, v21.4s, v31.4s
        fmul            v19.4s, v19.4s, v31.4s
        fmul            v17.4s, v17.4s, v31.4s
        fmul            v18.4s, v18.4s, v31.4s

        fmla            v23.4s, v10.4s, v21.4s      
        fmls            v25.4s, v14.4s, v21.4s      
        fmla            v22.4s, v8.4s, v19.4s      
        fmls            v24.4s, v12.4s, v19.4s      

        fmla            v26.4s, v9.4s, v18.4s      
        fmla            v27.4s, v11.4s, v17.4s      
        fmls            v20.4s, v13.4s, v18.4s      
        fmls            v16.4s, v15.4s, v17.4s      

        fsub            v18.4s, v23.4s, v25.4s
        fsub            v17.4s, v24.4s, v22.4s
        fsub            v19.4s, v20.4s, v26.4s
        fsub            v21.4s, v16.4s, v27.4s

        fadd            v22.4s, v24.4s, v22.4s
        fadd            v23.4s, v25.4s, v23.4s
        fadd            v24.4s, v20.4s, v26.4s
        fadd            v25.4s, v16.4s, v27.4s

        fmul            v17.4s, v17.4s, v31.4s
        fmul            v18.4s, v18.4s, v31.4s
        fmul            v19.4s, v19.4s, v31.4s
        fmul            v21.4s, v21.4s, v31.4s

        rev64           v22.4s, v22.4s
        rev64           v24.4s, v24.4s
        rev64           v23.4s, v23.4s
        rev64           v25.4s, v25.4s

        fsub            v8.4s, v0.4s, v22.4s
        fsub            v9.4s, v1.4s, v24.4s
        fsub            v10.4s, v2.4s, v17.4s
        fsub            v11.4s, v3.4s, v19.4s

        fsub            v12.4s, v4.4s, v23.4s
        fsub            v13.4s, v5.4s, v25.4s
        fadd            v14.4s, v6.4s, v18.4s
        fsub            v15.4s, v7.4s, v21.4s

        fadd            v0.4s, v0.4s, v22.4s
        fadd            v1.4s, v1.4s, v24.4s
        fadd            v2.4s, v2.4s, v17.4s
        fadd            v3.4s, v3.4s, v19.4s

        fadd            v4.4s, v4.4s, v23.4s
        fsub            v6.4s, v6.4s, v18.4s      
        fadd            v5.4s, v5.4s, v25.4s      
        fadd            v7.4s, v7.4s, v21.4s

        stp             q0,  q1,  [x1,  #((0 + 0)*32 + 0)]
        stp             q4,  q5,  [x1,  #((2 + 0)*32 + 0)]
        stp             q2,  q3,  [x10, #((0 + 0)*32 + 0)]
        stp             q6,  q7,  [x10, #((2 + 0)*32 + 0)]

        stp             q8,  q9,  [x11, #((0 + 0)*32 + 0)]
        stp             q12, q13, [x11, #((2 + 0)*32 + 0)]
        stp             q10, q11, [x12, #((0 + 0)*32 + 0)]
        stp             q14, q15, [x12, #((2 + 0)*32 + 0)]
        add             x10, x1, x21
        add             x11, x1, x21, lsl #1
        add             x12, x1, x22

        ldp             q0,  q1,  [x1,  #((0 + 1)*32 + 0)]
        ldp             q4,  q5,  [x1,  #((2 + 1)*32 + 0)]
        ldp             q2,  q3,  [x10, #((0 + 1)*32 + 0)]
        ldp             q6,  q7,  [x10, #((2 + 1)*32 + 0)]

        ldp             q8,  q9,  [x11, #((0 + 1)*32 + 0)]
        ldp             q10, q11, [x11, #((2 + 1)*32 + 0)]
        ldp             q12, q13, [x12, #((0 + 1)*32 + 0)]
        ldp             q14, q15, [x12, #((2 + 1)*32 + 0)]


        ld1             { v24.4s, v25.4s }, [x8], x9
        ld1             { v16.4s, v17.4s }, [x7], #32

        ext8             v18.16b, v24.16b, v24.16b, #8
        ext8             v19.16b, v25.16b, v25.16b, #8

        trn1            v20.4s, v16.4s, v16.4s      
        trn2            v16.4s, v16.4s, v16.4s      
        trn1            v21.4s, v17.4s, v17.4s      
        trn2            v17.4s, v17.4s, v17.4s      

        rev64           v22.4s, v8.4s                 
        rev64           v23.4s, v10.4s                 
        rev64           v24.4s, v12.4s                 
        rev64           v25.4s, v14.4s                 

        fmul            v22.4s, v22.4s, v20.4s      
        fmul            v23.4s, v23.4s, v16.4s      
        fmul            v24.4s, v24.4s, v20.4s      
        fmul            v25.4s, v25.4s, v16.4s      

        rev64           v26.4s, v9.4s                 
        rev64           v27.4s, v11.4s                 
        rev64           v20.4s, v13.4s                 
        rev64           v16.4s, v15.4s                 

        fmul            v26.4s, v26.4s, v21.4s      
        fmul            v27.4s, v27.4s, v17.4s      
        fmul            v20.4s, v20.4s, v21.4s      
        fmul            v16.4s, v16.4s, v17.4s      

        trn1            v21.4s, v19.4s, v19.4s      
        trn2            v19.4s, v19.4s, v19.4s      
        trn1            v17.4s, v18.4s, v18.4s      
        trn2            v18.4s, v18.4s, v18.4s      

        fmul            v21.4s, v21.4s, v31.4s
        fmul            v19.4s, v19.4s, v31.4s
        fmul            v17.4s, v17.4s, v31.4s
        fmul            v18.4s, v18.4s, v31.4s

        fmla            v23.4s, v10.4s, v21.4s      
        fmls            v25.4s, v14.4s, v21.4s      
        fmla            v22.4s, v8.4s, v19.4s      
        fmls            v24.4s, v12.4s, v19.4s      

        fmla            v26.4s, v9.4s, v18.4s      
        fmla            v27.4s, v11.4s, v17.4s      
        fmls            v20.4s, v13.4s, v18.4s      
        fmls            v16.4s, v15.4s, v17.4s      

        fsub            v18.4s, v23.4s, v25.4s
        fsub            v17.4s, v24.4s, v22.4s
        fsub            v19.4s, v20.4s, v26.4s
        fsub            v21.4s, v16.4s, v27.4s

        fadd            v22.4s, v24.4s, v22.4s
        fadd            v23.4s, v25.4s, v23.4s
        fadd            v24.4s, v20.4s, v26.4s
        fadd            v25.4s, v16.4s, v27.4s

        fmul            v17.4s, v17.4s, v31.4s
        fmul            v18.4s, v18.4s, v31.4s
        fmul            v19.4s, v19.4s, v31.4s
        fmul            v21.4s, v21.4s, v31.4s

        rev64           v22.4s, v22.4s
        rev64           v24.4s, v24.4s
        rev64           v23.4s, v23.4s
        rev64           v25.4s, v25.4s

        fsub            v8.4s, v0.4s, v22.4s
        fsub            v9.4s, v1.4s, v24.4s
        fsub            v10.4s, v2.4s, v17.4s
        fsub            v11.4s, v3.4s, v19.4s

        fsub            v12.4s, v4.4s, v23.4s
        fsub            v13.4s, v5.4s, v25.4s
        fadd            v14.4s, v6.4s, v18.4s
        fsub            v15.4s, v7.4s, v21.4s

        fadd            v0.4s, v0.4s, v22.4s
        fadd            v1.4s, v1.4s, v24.4s
        fadd            v2.4s, v2.4s, v17.4s
        fadd            v3.4s, v3.4s, v19.4s

        fadd            v4.4s, v4.4s, v23.4s
        fsub            v6.4s, v6.4s, v18.4s      
        fadd            v5.4s, v5.4s, v25.4s      
        fadd            v7.4s, v7.4s, v21.4s

        stp             q0,  q1,  [x1,  #((0 + 1)*32 + 0)]
        stp             q4,  q5,  [x1,  #((2 + 1)*32 + 0)]
        stp             q2,  q3,  [x10, #((0 + 1)*32 + 0)]
        stp             q6,  q7,  [x10, #((2 + 1)*32 + 0)]

        stp             q8,  q9,  [x11, #((0 + 1)*32 + 0)]
        stp             q12, q13, [x11, #((2 + 1)*32 + 0)]
        stp             q10, q11, [x12, #((0 + 1)*32 + 0)]
        stp             q14, q15, [x12, #((2 + 1)*32 + 0)]
        add             x10, x1, x21
        add             x11, x1, x21, lsl #1
        add             x12, x1, x22

        ldp             q0,  q1,  [x1,  #((0 + 4)*32 + 0)]
        ldp             q4,  q5,  [x1,  #((2 + 4)*32 + 0)]
        ldp             q2,  q3,  [x10, #((0 + 4)*32 + 0)]
        ldp             q6,  q7,  [x10, #((2 + 4)*32 + 0)]

        ldp             q8,  q9,  [x11, #((0 + 4)*32 + 0)]
        ldp             q10, q11, [x11, #((2 + 4)*32 + 0)]
        ldp             q12, q13, [x12, #((0 + 4)*32 + 0)]
        ldp             q14, q15, [x12, #((2 + 4)*32 + 0)]


        ld1             { v24.4s, v25.4s }, [x8], x9
        ld1             { v16.4s, v17.4s }, [x7], #32

        ext8             v18.16b, v24.16b, v24.16b, #8
        ext8             v19.16b, v25.16b, v25.16b, #8

        trn1            v20.4s, v16.4s, v16.4s      
        trn2            v16.4s, v16.4s, v16.4s      
        trn1            v21.4s, v17.4s, v17.4s      
        trn2            v17.4s, v17.4s, v17.4s      

        rev64           v22.4s, v8.4s                 
        rev64           v23.4s, v10.4s                 
        rev64           v24.4s, v12.4s                 
        rev64           v25.4s, v14.4s                 

        fmul            v22.4s, v22.4s, v20.4s      
        fmul            v23.4s, v23.4s, v16.4s      
        fmul            v24.4s, v24.4s, v20.4s      
        fmul            v25.4s, v25.4s, v16.4s      

        rev64           v26.4s, v9.4s                 
        rev64           v27.4s, v11.4s                 
        rev64           v20.4s, v13.4s                 
        rev64           v16.4s, v15.4s                 

        fmul            v26.4s, v26.4s, v21.4s      
        fmul            v27.4s, v27.4s, v17.4s      
        fmul            v20.4s, v20.4s, v21.4s      
        fmul            v16.4s, v16.4s, v17.4s      

        trn1            v21.4s, v19.4s, v19.4s      
        trn2            v19.4s, v19.4s, v19.4s      
        trn1            v17.4s, v18.4s, v18.4s      
        trn2            v18.4s, v18.4s, v18.4s      

        fmul            v21.4s, v21.4s, v31.4s
        fmul            v19.4s, v19.4s, v31.4s
        fmul            v17.4s, v17.4s, v31.4s
        fmul            v18.4s, v18.4s, v31.4s

        fmla            v23.4s, v10.4s, v21.4s      
        fmls            v25.4s, v14.4s, v21.4s      
        fmla            v22.4s, v8.4s, v19.4s      
        fmls            v24.4s, v12.4s, v19.4s      

        fmla            v26.4s, v9.4s, v18.4s      
        fmla            v27.4s, v11.4s, v17.4s      
        fmls            v20.4s, v13.4s, v18.4s      
        fmls            v16.4s, v15.4s, v17.4s      

        fsub            v18.4s, v23.4s, v25.4s
        fsub            v17.4s, v24.4s, v22.4s
        fsub            v19.4s, v20.4s, v26.4s
        fsub            v21.4s, v16.4s, v27.4s

        fadd            v22.4s, v24.4s, v22.4s
        fadd            v23.4s, v25.4s, v23.4s
        fadd            v24.4s, v20.4s, v26.4s
        fadd            v25.4s, v16.4s, v27.4s

        fmul            v17.4s, v17.4s, v31.4s
        fmul            v18.4s, v18.4s, v31.4s
        fmul            v19.4s, v19.4s, v31.4s
        fmul            v21.4s, v21.4s, v31.4s

        rev64           v22.4s, v22.4s
        rev64           v24.4s, v24.4s
        rev64           v23.4s, v23.4s
        rev64           v25.4s, v25.4s

        fsub            v8.4s, v0.4s, v22.4s
        fsub            v9.4s, v1.4s, v24.4s
        fsub            v10.4s, v2.4s, v17.4s
        fsub            v11.4s, v3.4s, v19.4s

        fsub            v12.4s, v4.4s, v23.4s
        fsub            v13.4s, v5.4s, v25.4s
        fadd            v14.4s, v6.4s, v18.4s
        fsub            v15.4s, v7.4s, v21.4s

        fadd            v0.4s, v0.4s, v22.4s
        fadd            v1.4s, v1.4s, v24.4s
        fadd            v2.4s, v2.4s, v17.4s
        fadd            v3.4s, v3.4s, v19.4s

        fadd            v4.4s, v4.4s, v23.4s
        fsub            v6.4s, v6.4s, v18.4s      
        fadd            v5.4s, v5.4s, v25.4s      
        fadd            v7.4s, v7.4s, v21.4s

        stp             q0,  q1,  [x1,  #((0 + 4)*32 + 0)]
        stp             q4,  q5,  [x1,  #((2 + 4)*32 + 0)]
        stp             q2,  q3,  [x10, #((0 + 4)*32 + 0)]
        stp             q6,  q7,  [x10, #((2 + 4)*32 + 0)]

        stp             q8,  q9,  [x11, #((0 + 4)*32 + 0)]
        stp             q12, q13, [x11, #((2 + 4)*32 + 0)]
        stp             q10, q11, [x12, #((0 + 4)*32 + 0)]
        stp             q14, q15, [x12, #((2 + 4)*32 + 0)]
        add             x10, x1, x21
        add             x11, x1, x21, lsl #1
        add             x12, x1, x22

        ldp             q0,  q1,  [x1,  #((0 + 5)*32 + 0)]
        ldp             q4,  q5,  [x1,  #((2 + 5)*32 + 0)]
        ldp             q2,  q3,  [x10, #((0 + 5)*32 + 0)]
        ldp             q6,  q7,  [x10, #((2 + 5)*32 + 0)]

        ldp             q8,  q9,  [x11, #((0 + 5)*32 + 0)]
        ldp             q10, q11, [x11, #((2 + 5)*32 + 0)]
        ldp             q12, q13, [x12, #((0 + 5)*32 + 0)]
        ldp             q14, q15, [x12, #((2 + 5)*32 + 0)]


        ld1             { v24.4s, v25.4s }, [x8], x9
        ld1             { v16.4s, v17.4s }, [x7], #32

        ext8             v18.16b, v24.16b, v24.16b, #8
        ext8             v19.16b, v25.16b, v25.16b, #8

        trn1            v20.4s, v16.4s, v16.4s      
        trn2            v16.4s, v16.4s, v16.4s      
        trn1            v21.4s, v17.4s, v17.4s      
        trn2            v17.4s, v17.4s, v17.4s      

        rev64           v22.4s, v8.4s                 
        rev64           v23.4s, v10.4s                 
        rev64           v24.4s, v12.4s                 
        rev64           v25.4s, v14.4s                 

        fmul            v22.4s, v22.4s, v20.4s      
        fmul            v23.4s, v23.4s, v16.4s      
        fmul            v24.4s, v24.4s, v20.4s      
        fmul            v25.4s, v25.4s, v16.4s      

        rev64           v26.4s, v9.4s                 
        rev64           v27.4s, v11.4s                 
        rev64           v20.4s, v13.4s                 
        rev64           v16.4s, v15.4s                 

        fmul            v26.4s, v26.4s, v21.4s      
        fmul            v27.4s, v27.4s, v17.4s      
        fmul            v20.4s, v20.4s, v21.4s      
        fmul            v16.4s, v16.4s, v17.4s      

        trn1            v21.4s, v19.4s, v19.4s      
        trn2            v19.4s, v19.4s, v19.4s      
        trn1            v17.4s, v18.4s, v18.4s      
        trn2            v18.4s, v18.4s, v18.4s      

        fmul            v21.4s, v21.4s, v31.4s
        fmul            v19.4s, v19.4s, v31.4s
        fmul            v17.4s, v17.4s, v31.4s
        fmul            v18.4s, v18.4s, v31.4s

        fmla            v23.4s, v10.4s, v21.4s      
        fmls            v25.4s, v14.4s, v21.4s      
        fmla            v22.4s, v8.4s, v19.4s      
        fmls            v24.4s, v12.4s, v19.4s      

        fmla            v26.4s, v9.4s, v18.4s      
        fmla            v27.4s, v11.4s, v17.4s      
        fmls            v20.4s, v13.4s, v18.4s      
        fmls            v16.4s, v15.4s, v17.4s      

        fsub            v18.4s, v23.4s, v25.4s
        fsub            v17.4s, v24.4s, v22.4s
        fsub            v19.4s, v20.4s, v26.4s
        fsub            v21.4s, v16.4s, v27.4s

        fadd            v22.4s, v24.4s, v22.4s
        fadd            v23.4s, v25.4s, v23.4s
        fadd            v24.4s, v20.4s, v26.4s
        fadd            v25.4s, v16.4s, v27.4s

        fmul            v17.4s, v17.4s, v31.4s
        fmul            v18.4s, v18.4s, v31.4s
        fmul            v19.4s, v19.4s, v31.4s
        fmul            v21.4s, v21.4s, v31.4s

        rev64           v22.4s, v22.4s
        rev64           v24.4s, v24.4s
        rev64           v23.4s, v23.4s
        rev64           v25.4s, v25.4s

        fsub            v8.4s, v0.4s, v22.4s
        fsub            v9.4s, v1.4s, v24.4s
        fsub            v10.4s, v2.4s, v17.4s
        fsub            v11.4s, v3.4s, v19.4s

        fsub            v12.4s, v4.4s, v23.4s
        fsub            v13.4s, v5.4s, v25.4s
        fadd            v14.4s, v6.4s, v18.4s
        fsub            v15.4s, v7.4s, v21.4s

        fadd            v0.4s, v0.4s, v22.4s
        fadd            v1.4s, v1.4s, v24.4s
        fadd            v2.4s, v2.4s, v17.4s
        fadd            v3.4s, v3.4s, v19.4s

        fadd            v4.4s, v4.4s, v23.4s
        fsub            v6.4s, v6.4s, v18.4s      
        fadd            v5.4s, v5.4s, v25.4s      
        fadd            v7.4s, v7.4s, v21.4s

        stp             q0,  q1,  [x1,  #((0 + 5)*32 + 0)]
        stp             q4,  q5,  [x1,  #((2 + 5)*32 + 0)]
        stp             q2,  q3,  [x10, #((0 + 5)*32 + 0)]
        stp             q6,  q7,  [x10, #((2 + 5)*32 + 0)]

        stp             q8,  q9,  [x11, #((0 + 5)*32 + 0)]
        stp             q12, q13, [x11, #((2 + 5)*32 + 0)]
        stp             q10, q11, [x12, #((0 + 5)*32 + 0)]
        stp             q14, q15, [x12, #((2 + 5)*32 + 0)]
        add             x1, x1, 8*32
        subs            w5, w5, 1
        bgt            temp_label_75

        cmp             w20, #((8192)/4096), lsl #12
        bgt            temp_label_76
        ret
temp_label_77
temp_label_76

        stp             x20, x30, [sp, #-16]!
        mov             w20, #(16384/4)
        mov             x5, #((16384*4) - (16384/1))
        add             x1, x1, x5
        bl              temp_label_46
        mov             x5, #((16384*2) - (16384/2))
        add             x1, x1, x5
        bl              temp_label_46
        ldp             x20, x30, [sp], #16
        ldr             w5, =(16384*6 + 16384/2)
        sub             x1, x1, x5

        ldr             w5, =(16384 - 4*7)










        adrp            x7, ff_tx_tab_16384_float
        add             x7, x7, ff_tx_tab_16384_float










        add             x8, x7, x5
        mov             x9, #-32

        mov             x21, #2*16384
        add             x22, x21, x21, lsl #1

        cmp             w19, #((16384)/4096), lsl #12
        beq            temp_label_78

        mov             w5, #(16384/128)
temp_label_79

        add             x10, x1, x21
        add             x11, x1, x21, lsl #1
        add             x12, x1, x22

        add             x10, x1, x21
        add             x11, x1, x21, lsl #1
        add             x12, x1, x22

        ldp             q0,  q1,  [x1,  #((0 + 0)*32 + 0)]
        ldp             q4,  q5,  [x1,  #((2 + 0)*32 + 0)]
        ldp             q2,  q3,  [x10, #((0 + 0)*32 + 0)]
        ldp             q6,  q7,  [x10, #((2 + 0)*32 + 0)]

        ldp             q8,  q9,  [x11, #((0 + 0)*32 + 0)]
        ldp             q10, q11, [x11, #((2 + 0)*32 + 0)]
        ldp             q12, q13, [x12, #((0 + 0)*32 + 0)]
        ldp             q14, q15, [x12, #((2 + 0)*32 + 0)]


        ld1             { v24.4s, v25.4s }, [x8], x9
        ld1             { v16.4s, v17.4s }, [x7], #32

        ext8             v18.16b, v24.16b, v24.16b, #8
        ext8             v19.16b, v25.16b, v25.16b, #8

        trn1            v20.4s, v16.4s, v16.4s      
        trn2            v16.4s, v16.4s, v16.4s      
        trn1            v21.4s, v17.4s, v17.4s      
        trn2            v17.4s, v17.4s, v17.4s      

        rev64           v22.4s, v8.4s                 
        rev64           v23.4s, v10.4s                 
        rev64           v24.4s, v12.4s                 
        rev64           v25.4s, v14.4s                 

        fmul            v22.4s, v22.4s, v20.4s      
        fmul            v23.4s, v23.4s, v16.4s      
        fmul            v24.4s, v24.4s, v20.4s      
        fmul            v25.4s, v25.4s, v16.4s      

        rev64           v26.4s, v9.4s                 
        rev64           v27.4s, v11.4s                 
        rev64           v20.4s, v13.4s                 
        rev64           v16.4s, v15.4s                 

        fmul            v26.4s, v26.4s, v21.4s      
        fmul            v27.4s, v27.4s, v17.4s      
        fmul            v20.4s, v20.4s, v21.4s      
        fmul            v16.4s, v16.4s, v17.4s      

        trn1            v21.4s, v19.4s, v19.4s      
        trn2            v19.4s, v19.4s, v19.4s      
        trn1            v17.4s, v18.4s, v18.4s      
        trn2            v18.4s, v18.4s, v18.4s      

        fmul            v21.4s, v21.4s, v31.4s
        fmul            v19.4s, v19.4s, v31.4s
        fmul            v17.4s, v17.4s, v31.4s
        fmul            v18.4s, v18.4s, v31.4s

        fmla            v23.4s, v10.4s, v21.4s      
        fmls            v25.4s, v14.4s, v21.4s      
        fmla            v22.4s, v8.4s, v19.4s      
        fmls            v24.4s, v12.4s, v19.4s      

        fmla            v26.4s, v9.4s, v18.4s      
        fmla            v27.4s, v11.4s, v17.4s      
        fmls            v20.4s, v13.4s, v18.4s      
        fmls            v16.4s, v15.4s, v17.4s      

        fsub            v18.4s, v23.4s, v25.4s
        fsub            v17.4s, v24.4s, v22.4s
        fsub            v19.4s, v20.4s, v26.4s
        fsub            v21.4s, v16.4s, v27.4s

        fadd            v22.4s, v24.4s, v22.4s
        fadd            v23.4s, v25.4s, v23.4s
        fadd            v24.4s, v20.4s, v26.4s
        fadd            v25.4s, v16.4s, v27.4s

        fmul            v17.4s, v17.4s, v31.4s
        fmul            v18.4s, v18.4s, v31.4s
        fmul            v19.4s, v19.4s, v31.4s
        fmul            v21.4s, v21.4s, v31.4s

        rev64           v22.4s, v22.4s
        rev64           v24.4s, v24.4s
        rev64           v23.4s, v23.4s
        rev64           v25.4s, v25.4s

        fsub            v8.4s, v0.4s, v22.4s
        fsub            v9.4s, v1.4s, v24.4s
        fsub            v10.4s, v2.4s, v17.4s
        fsub            v11.4s, v3.4s, v19.4s

        fsub            v12.4s, v4.4s, v23.4s
        fsub            v13.4s, v5.4s, v25.4s
        fadd            v14.4s, v6.4s, v18.4s
        fsub            v15.4s, v7.4s, v21.4s

        fadd            v0.4s, v0.4s, v22.4s
        fadd            v1.4s, v1.4s, v24.4s
        fadd            v2.4s, v2.4s, v17.4s
        fadd            v3.4s, v3.4s, v19.4s

        fadd            v4.4s, v4.4s, v23.4s
        fsub            v6.4s, v6.4s, v18.4s      
        fadd            v5.4s, v5.4s, v25.4s      
        fadd            v7.4s, v7.4s, v21.4s

        stp             q0,  q1,  [x1,  #((0 + 0)*32 + 0)]
        stp             q4,  q5,  [x1,  #((2 + 0)*32 + 0)]
        stp             q2,  q3,  [x10, #((0 + 0)*32 + 0)]
        stp             q6,  q7,  [x10, #((2 + 0)*32 + 0)]

        stp             q8,  q9,  [x11, #((0 + 0)*32 + 0)]
        stp             q12, q13, [x11, #((2 + 0)*32 + 0)]
        stp             q10, q11, [x12, #((0 + 0)*32 + 0)]
        stp             q14, q15, [x12, #((2 + 0)*32 + 0)]
        add             x10, x1, x21
        add             x11, x1, x21, lsl #1
        add             x12, x1, x22

        ldp             q0,  q1,  [x1,  #((0 + 1)*32 + 0)]
        ldp             q4,  q5,  [x1,  #((2 + 1)*32 + 0)]
        ldp             q2,  q3,  [x10, #((0 + 1)*32 + 0)]
        ldp             q6,  q7,  [x10, #((2 + 1)*32 + 0)]

        ldp             q8,  q9,  [x11, #((0 + 1)*32 + 0)]
        ldp             q10, q11, [x11, #((2 + 1)*32 + 0)]
        ldp             q12, q13, [x12, #((0 + 1)*32 + 0)]
        ldp             q14, q15, [x12, #((2 + 1)*32 + 0)]


        ld1             { v24.4s, v25.4s }, [x8], x9
        ld1             { v16.4s, v17.4s }, [x7], #32

        ext8             v18.16b, v24.16b, v24.16b, #8
        ext8             v19.16b, v25.16b, v25.16b, #8

        trn1            v20.4s, v16.4s, v16.4s      
        trn2            v16.4s, v16.4s, v16.4s      
        trn1            v21.4s, v17.4s, v17.4s      
        trn2            v17.4s, v17.4s, v17.4s      

        rev64           v22.4s, v8.4s                 
        rev64           v23.4s, v10.4s                 
        rev64           v24.4s, v12.4s                 
        rev64           v25.4s, v14.4s                 

        fmul            v22.4s, v22.4s, v20.4s      
        fmul            v23.4s, v23.4s, v16.4s      
        fmul            v24.4s, v24.4s, v20.4s      
        fmul            v25.4s, v25.4s, v16.4s      

        rev64           v26.4s, v9.4s                 
        rev64           v27.4s, v11.4s                 
        rev64           v20.4s, v13.4s                 
        rev64           v16.4s, v15.4s                 

        fmul            v26.4s, v26.4s, v21.4s      
        fmul            v27.4s, v27.4s, v17.4s      
        fmul            v20.4s, v20.4s, v21.4s      
        fmul            v16.4s, v16.4s, v17.4s      

        trn1            v21.4s, v19.4s, v19.4s      
        trn2            v19.4s, v19.4s, v19.4s      
        trn1            v17.4s, v18.4s, v18.4s      
        trn2            v18.4s, v18.4s, v18.4s      

        fmul            v21.4s, v21.4s, v31.4s
        fmul            v19.4s, v19.4s, v31.4s
        fmul            v17.4s, v17.4s, v31.4s
        fmul            v18.4s, v18.4s, v31.4s

        fmla            v23.4s, v10.4s, v21.4s      
        fmls            v25.4s, v14.4s, v21.4s      
        fmla            v22.4s, v8.4s, v19.4s      
        fmls            v24.4s, v12.4s, v19.4s      

        fmla            v26.4s, v9.4s, v18.4s      
        fmla            v27.4s, v11.4s, v17.4s      
        fmls            v20.4s, v13.4s, v18.4s      
        fmls            v16.4s, v15.4s, v17.4s      

        fsub            v18.4s, v23.4s, v25.4s
        fsub            v17.4s, v24.4s, v22.4s
        fsub            v19.4s, v20.4s, v26.4s
        fsub            v21.4s, v16.4s, v27.4s

        fadd            v22.4s, v24.4s, v22.4s
        fadd            v23.4s, v25.4s, v23.4s
        fadd            v24.4s, v20.4s, v26.4s
        fadd            v25.4s, v16.4s, v27.4s

        fmul            v17.4s, v17.4s, v31.4s
        fmul            v18.4s, v18.4s, v31.4s
        fmul            v19.4s, v19.4s, v31.4s
        fmul            v21.4s, v21.4s, v31.4s

        rev64           v22.4s, v22.4s
        rev64           v24.4s, v24.4s
        rev64           v23.4s, v23.4s
        rev64           v25.4s, v25.4s

        fsub            v8.4s, v0.4s, v22.4s
        fsub            v9.4s, v1.4s, v24.4s
        fsub            v10.4s, v2.4s, v17.4s
        fsub            v11.4s, v3.4s, v19.4s

        fsub            v12.4s, v4.4s, v23.4s
        fsub            v13.4s, v5.4s, v25.4s
        fadd            v14.4s, v6.4s, v18.4s
        fsub            v15.4s, v7.4s, v21.4s

        fadd            v0.4s, v0.4s, v22.4s
        fadd            v1.4s, v1.4s, v24.4s
        fadd            v2.4s, v2.4s, v17.4s
        fadd            v3.4s, v3.4s, v19.4s

        fadd            v4.4s, v4.4s, v23.4s
        fsub            v6.4s, v6.4s, v18.4s      
        fadd            v5.4s, v5.4s, v25.4s      
        fadd            v7.4s, v7.4s, v21.4s

        stp             q0,  q1,  [x1,  #((0 + 1)*32 + 0)]
        stp             q4,  q5,  [x1,  #((2 + 1)*32 + 0)]
        stp             q2,  q3,  [x10, #((0 + 1)*32 + 0)]
        stp             q6,  q7,  [x10, #((2 + 1)*32 + 0)]

        stp             q8,  q9,  [x11, #((0 + 1)*32 + 0)]
        stp             q12, q13, [x11, #((2 + 1)*32 + 0)]
        stp             q10, q11, [x12, #((0 + 1)*32 + 0)]
        stp             q14, q15, [x12, #((2 + 1)*32 + 0)]
        add             x10, x1, x21
        add             x11, x1, x21, lsl #1
        add             x12, x1, x22

        ldp             q0,  q1,  [x1,  #((0 + 4)*32 + 0)]
        ldp             q4,  q5,  [x1,  #((2 + 4)*32 + 0)]
        ldp             q2,  q3,  [x10, #((0 + 4)*32 + 0)]
        ldp             q6,  q7,  [x10, #((2 + 4)*32 + 0)]

        ldp             q8,  q9,  [x11, #((0 + 4)*32 + 0)]
        ldp             q10, q11, [x11, #((2 + 4)*32 + 0)]
        ldp             q12, q13, [x12, #((0 + 4)*32 + 0)]
        ldp             q14, q15, [x12, #((2 + 4)*32 + 0)]


        ld1             { v24.4s, v25.4s }, [x8], x9
        ld1             { v16.4s, v17.4s }, [x7], #32

        ext8             v18.16b, v24.16b, v24.16b, #8
        ext8             v19.16b, v25.16b, v25.16b, #8

        trn1            v20.4s, v16.4s, v16.4s      
        trn2            v16.4s, v16.4s, v16.4s      
        trn1            v21.4s, v17.4s, v17.4s      
        trn2            v17.4s, v17.4s, v17.4s      

        rev64           v22.4s, v8.4s                 
        rev64           v23.4s, v10.4s                 
        rev64           v24.4s, v12.4s                 
        rev64           v25.4s, v14.4s                 

        fmul            v22.4s, v22.4s, v20.4s      
        fmul            v23.4s, v23.4s, v16.4s      
        fmul            v24.4s, v24.4s, v20.4s      
        fmul            v25.4s, v25.4s, v16.4s      

        rev64           v26.4s, v9.4s                 
        rev64           v27.4s, v11.4s                 
        rev64           v20.4s, v13.4s                 
        rev64           v16.4s, v15.4s                 

        fmul            v26.4s, v26.4s, v21.4s      
        fmul            v27.4s, v27.4s, v17.4s      
        fmul            v20.4s, v20.4s, v21.4s      
        fmul            v16.4s, v16.4s, v17.4s      

        trn1            v21.4s, v19.4s, v19.4s      
        trn2            v19.4s, v19.4s, v19.4s      
        trn1            v17.4s, v18.4s, v18.4s      
        trn2            v18.4s, v18.4s, v18.4s      

        fmul            v21.4s, v21.4s, v31.4s
        fmul            v19.4s, v19.4s, v31.4s
        fmul            v17.4s, v17.4s, v31.4s
        fmul            v18.4s, v18.4s, v31.4s

        fmla            v23.4s, v10.4s, v21.4s      
        fmls            v25.4s, v14.4s, v21.4s      
        fmla            v22.4s, v8.4s, v19.4s      
        fmls            v24.4s, v12.4s, v19.4s      

        fmla            v26.4s, v9.4s, v18.4s      
        fmla            v27.4s, v11.4s, v17.4s      
        fmls            v20.4s, v13.4s, v18.4s      
        fmls            v16.4s, v15.4s, v17.4s      

        fsub            v18.4s, v23.4s, v25.4s
        fsub            v17.4s, v24.4s, v22.4s
        fsub            v19.4s, v20.4s, v26.4s
        fsub            v21.4s, v16.4s, v27.4s

        fadd            v22.4s, v24.4s, v22.4s
        fadd            v23.4s, v25.4s, v23.4s
        fadd            v24.4s, v20.4s, v26.4s
        fadd            v25.4s, v16.4s, v27.4s

        fmul            v17.4s, v17.4s, v31.4s
        fmul            v18.4s, v18.4s, v31.4s
        fmul            v19.4s, v19.4s, v31.4s
        fmul            v21.4s, v21.4s, v31.4s

        rev64           v22.4s, v22.4s
        rev64           v24.4s, v24.4s
        rev64           v23.4s, v23.4s
        rev64           v25.4s, v25.4s

        fsub            v8.4s, v0.4s, v22.4s
        fsub            v9.4s, v1.4s, v24.4s
        fsub            v10.4s, v2.4s, v17.4s
        fsub            v11.4s, v3.4s, v19.4s

        fsub            v12.4s, v4.4s, v23.4s
        fsub            v13.4s, v5.4s, v25.4s
        fadd            v14.4s, v6.4s, v18.4s
        fsub            v15.4s, v7.4s, v21.4s

        fadd            v0.4s, v0.4s, v22.4s
        fadd            v1.4s, v1.4s, v24.4s
        fadd            v2.4s, v2.4s, v17.4s
        fadd            v3.4s, v3.4s, v19.4s

        fadd            v4.4s, v4.4s, v23.4s
        fsub            v6.4s, v6.4s, v18.4s      
        fadd            v5.4s, v5.4s, v25.4s      
        fadd            v7.4s, v7.4s, v21.4s

        stp             q0,  q1,  [x1,  #((0 + 4)*32 + 0)]
        stp             q4,  q5,  [x1,  #((2 + 4)*32 + 0)]
        stp             q2,  q3,  [x10, #((0 + 4)*32 + 0)]
        stp             q6,  q7,  [x10, #((2 + 4)*32 + 0)]

        stp             q8,  q9,  [x11, #((0 + 4)*32 + 0)]
        stp             q12, q13, [x11, #((2 + 4)*32 + 0)]
        stp             q10, q11, [x12, #((0 + 4)*32 + 0)]
        stp             q14, q15, [x12, #((2 + 4)*32 + 0)]
        add             x10, x1, x21
        add             x11, x1, x21, lsl #1
        add             x12, x1, x22

        ldp             q0,  q1,  [x1,  #((0 + 5)*32 + 0)]
        ldp             q4,  q5,  [x1,  #((2 + 5)*32 + 0)]
        ldp             q2,  q3,  [x10, #((0 + 5)*32 + 0)]
        ldp             q6,  q7,  [x10, #((2 + 5)*32 + 0)]

        ldp             q8,  q9,  [x11, #((0 + 5)*32 + 0)]
        ldp             q10, q11, [x11, #((2 + 5)*32 + 0)]
        ldp             q12, q13, [x12, #((0 + 5)*32 + 0)]
        ldp             q14, q15, [x12, #((2 + 5)*32 + 0)]


        ld1             { v24.4s, v25.4s }, [x8], x9
        ld1             { v16.4s, v17.4s }, [x7], #32

        ext8             v18.16b, v24.16b, v24.16b, #8
        ext8             v19.16b, v25.16b, v25.16b, #8

        trn1            v20.4s, v16.4s, v16.4s      
        trn2            v16.4s, v16.4s, v16.4s      
        trn1            v21.4s, v17.4s, v17.4s      
        trn2            v17.4s, v17.4s, v17.4s      

        rev64           v22.4s, v8.4s                 
        rev64           v23.4s, v10.4s                 
        rev64           v24.4s, v12.4s                 
        rev64           v25.4s, v14.4s                 

        fmul            v22.4s, v22.4s, v20.4s      
        fmul            v23.4s, v23.4s, v16.4s      
        fmul            v24.4s, v24.4s, v20.4s      
        fmul            v25.4s, v25.4s, v16.4s      

        rev64           v26.4s, v9.4s                 
        rev64           v27.4s, v11.4s                 
        rev64           v20.4s, v13.4s                 
        rev64           v16.4s, v15.4s                 

        fmul            v26.4s, v26.4s, v21.4s      
        fmul            v27.4s, v27.4s, v17.4s      
        fmul            v20.4s, v20.4s, v21.4s      
        fmul            v16.4s, v16.4s, v17.4s      

        trn1            v21.4s, v19.4s, v19.4s      
        trn2            v19.4s, v19.4s, v19.4s      
        trn1            v17.4s, v18.4s, v18.4s      
        trn2            v18.4s, v18.4s, v18.4s      

        fmul            v21.4s, v21.4s, v31.4s
        fmul            v19.4s, v19.4s, v31.4s
        fmul            v17.4s, v17.4s, v31.4s
        fmul            v18.4s, v18.4s, v31.4s

        fmla            v23.4s, v10.4s, v21.4s      
        fmls            v25.4s, v14.4s, v21.4s      
        fmla            v22.4s, v8.4s, v19.4s      
        fmls            v24.4s, v12.4s, v19.4s      

        fmla            v26.4s, v9.4s, v18.4s      
        fmla            v27.4s, v11.4s, v17.4s      
        fmls            v20.4s, v13.4s, v18.4s      
        fmls            v16.4s, v15.4s, v17.4s      

        fsub            v18.4s, v23.4s, v25.4s
        fsub            v17.4s, v24.4s, v22.4s
        fsub            v19.4s, v20.4s, v26.4s
        fsub            v21.4s, v16.4s, v27.4s

        fadd            v22.4s, v24.4s, v22.4s
        fadd            v23.4s, v25.4s, v23.4s
        fadd            v24.4s, v20.4s, v26.4s
        fadd            v25.4s, v16.4s, v27.4s

        fmul            v17.4s, v17.4s, v31.4s
        fmul            v18.4s, v18.4s, v31.4s
        fmul            v19.4s, v19.4s, v31.4s
        fmul            v21.4s, v21.4s, v31.4s

        rev64           v22.4s, v22.4s
        rev64           v24.4s, v24.4s
        rev64           v23.4s, v23.4s
        rev64           v25.4s, v25.4s

        fsub            v8.4s, v0.4s, v22.4s
        fsub            v9.4s, v1.4s, v24.4s
        fsub            v10.4s, v2.4s, v17.4s
        fsub            v11.4s, v3.4s, v19.4s

        fsub            v12.4s, v4.4s, v23.4s
        fsub            v13.4s, v5.4s, v25.4s
        fadd            v14.4s, v6.4s, v18.4s
        fsub            v15.4s, v7.4s, v21.4s

        fadd            v0.4s, v0.4s, v22.4s
        fadd            v1.4s, v1.4s, v24.4s
        fadd            v2.4s, v2.4s, v17.4s
        fadd            v3.4s, v3.4s, v19.4s

        fadd            v4.4s, v4.4s, v23.4s
        fsub            v6.4s, v6.4s, v18.4s      
        fadd            v5.4s, v5.4s, v25.4s      
        fadd            v7.4s, v7.4s, v21.4s

        stp             q0,  q1,  [x1,  #((0 + 5)*32 + 0)]
        stp             q4,  q5,  [x1,  #((2 + 5)*32 + 0)]
        stp             q2,  q3,  [x10, #((0 + 5)*32 + 0)]
        stp             q6,  q7,  [x10, #((2 + 5)*32 + 0)]

        stp             q8,  q9,  [x11, #((0 + 5)*32 + 0)]
        stp             q12, q13, [x11, #((2 + 5)*32 + 0)]
        stp             q10, q11, [x12, #((0 + 5)*32 + 0)]
        stp             q14, q15, [x12, #((2 + 5)*32 + 0)]
        add             x1, x1, 8*32
        subs            w5, w5, 1
        bgt            temp_label_79

        cmp             w20, #((16384)/4096), lsl #12
        bgt            temp_label_80
        ret
temp_label_81
temp_label_80

        stp             x20, x30, [sp, #-16]!
        mov             w20, #(32768/4)
        mov             x5, #((32768*4) - (32768/1))
        add             x1, x1, x5
        bl              temp_label_46
        mov             x5, #((32768*2) - (32768/2))
        add             x1, x1, x5
        bl              temp_label_46
        ldp             x20, x30, [sp], #16
        ldr             w5, =(32768*6 + 32768/2)
        sub             x1, x1, x5

        ldr             w5, =(32768 - 4*7)










        adrp            x7, ff_tx_tab_32768_float
        add             x7, x7, ff_tx_tab_32768_float










        add             x8, x7, x5
        mov             x9, #-32

        mov             x21, #2*32768
        add             x22, x21, x21, lsl #1

        cmp             w19, #((32768)/4096), lsl #12
        beq            temp_label_82

        mov             w5, #(32768/128)
temp_label_83

        add             x10, x1, x21
        add             x11, x1, x21, lsl #1
        add             x12, x1, x22

        add             x10, x1, x21
        add             x11, x1, x21, lsl #1
        add             x12, x1, x22

        ldp             q0,  q1,  [x1,  #((0 + 0)*32 + 0)]
        ldp             q4,  q5,  [x1,  #((2 + 0)*32 + 0)]
        ldp             q2,  q3,  [x10, #((0 + 0)*32 + 0)]
        ldp             q6,  q7,  [x10, #((2 + 0)*32 + 0)]

        ldp             q8,  q9,  [x11, #((0 + 0)*32 + 0)]
        ldp             q10, q11, [x11, #((2 + 0)*32 + 0)]
        ldp             q12, q13, [x12, #((0 + 0)*32 + 0)]
        ldp             q14, q15, [x12, #((2 + 0)*32 + 0)]


        ld1             { v24.4s, v25.4s }, [x8], x9
        ld1             { v16.4s, v17.4s }, [x7], #32

        ext8             v18.16b, v24.16b, v24.16b, #8
        ext8             v19.16b, v25.16b, v25.16b, #8

        trn1            v20.4s, v16.4s, v16.4s      
        trn2            v16.4s, v16.4s, v16.4s      
        trn1            v21.4s, v17.4s, v17.4s      
        trn2            v17.4s, v17.4s, v17.4s      

        rev64           v22.4s, v8.4s                 
        rev64           v23.4s, v10.4s                 
        rev64           v24.4s, v12.4s                 
        rev64           v25.4s, v14.4s                 

        fmul            v22.4s, v22.4s, v20.4s      
        fmul            v23.4s, v23.4s, v16.4s      
        fmul            v24.4s, v24.4s, v20.4s      
        fmul            v25.4s, v25.4s, v16.4s      

        rev64           v26.4s, v9.4s                 
        rev64           v27.4s, v11.4s                 
        rev64           v20.4s, v13.4s                 
        rev64           v16.4s, v15.4s                 

        fmul            v26.4s, v26.4s, v21.4s      
        fmul            v27.4s, v27.4s, v17.4s      
        fmul            v20.4s, v20.4s, v21.4s      
        fmul            v16.4s, v16.4s, v17.4s      

        trn1            v21.4s, v19.4s, v19.4s      
        trn2            v19.4s, v19.4s, v19.4s      
        trn1            v17.4s, v18.4s, v18.4s      
        trn2            v18.4s, v18.4s, v18.4s      

        fmul            v21.4s, v21.4s, v31.4s
        fmul            v19.4s, v19.4s, v31.4s
        fmul            v17.4s, v17.4s, v31.4s
        fmul            v18.4s, v18.4s, v31.4s

        fmla            v23.4s, v10.4s, v21.4s      
        fmls            v25.4s, v14.4s, v21.4s      
        fmla            v22.4s, v8.4s, v19.4s      
        fmls            v24.4s, v12.4s, v19.4s      

        fmla            v26.4s, v9.4s, v18.4s      
        fmla            v27.4s, v11.4s, v17.4s      
        fmls            v20.4s, v13.4s, v18.4s      
        fmls            v16.4s, v15.4s, v17.4s      

        fsub            v18.4s, v23.4s, v25.4s
        fsub            v17.4s, v24.4s, v22.4s
        fsub            v19.4s, v20.4s, v26.4s
        fsub            v21.4s, v16.4s, v27.4s

        fadd            v22.4s, v24.4s, v22.4s
        fadd            v23.4s, v25.4s, v23.4s
        fadd            v24.4s, v20.4s, v26.4s
        fadd            v25.4s, v16.4s, v27.4s

        fmul            v17.4s, v17.4s, v31.4s
        fmul            v18.4s, v18.4s, v31.4s
        fmul            v19.4s, v19.4s, v31.4s
        fmul            v21.4s, v21.4s, v31.4s

        rev64           v22.4s, v22.4s
        rev64           v24.4s, v24.4s
        rev64           v23.4s, v23.4s
        rev64           v25.4s, v25.4s

        fsub            v8.4s, v0.4s, v22.4s
        fsub            v9.4s, v1.4s, v24.4s
        fsub            v10.4s, v2.4s, v17.4s
        fsub            v11.4s, v3.4s, v19.4s

        fsub            v12.4s, v4.4s, v23.4s
        fsub            v13.4s, v5.4s, v25.4s
        fadd            v14.4s, v6.4s, v18.4s
        fsub            v15.4s, v7.4s, v21.4s

        fadd            v0.4s, v0.4s, v22.4s
        fadd            v1.4s, v1.4s, v24.4s
        fadd            v2.4s, v2.4s, v17.4s
        fadd            v3.4s, v3.4s, v19.4s

        fadd            v4.4s, v4.4s, v23.4s
        fsub            v6.4s, v6.4s, v18.4s      
        fadd            v5.4s, v5.4s, v25.4s      
        fadd            v7.4s, v7.4s, v21.4s

        stp             q0,  q1,  [x1,  #((0 + 0)*32 + 0)]
        stp             q4,  q5,  [x1,  #((2 + 0)*32 + 0)]
        stp             q2,  q3,  [x10, #((0 + 0)*32 + 0)]
        stp             q6,  q7,  [x10, #((2 + 0)*32 + 0)]

        stp             q8,  q9,  [x11, #((0 + 0)*32 + 0)]
        stp             q12, q13, [x11, #((2 + 0)*32 + 0)]
        stp             q10, q11, [x12, #((0 + 0)*32 + 0)]
        stp             q14, q15, [x12, #((2 + 0)*32 + 0)]
        add             x10, x1, x21
        add             x11, x1, x21, lsl #1
        add             x12, x1, x22

        ldp             q0,  q1,  [x1,  #((0 + 1)*32 + 0)]
        ldp             q4,  q5,  [x1,  #((2 + 1)*32 + 0)]
        ldp             q2,  q3,  [x10, #((0 + 1)*32 + 0)]
        ldp             q6,  q7,  [x10, #((2 + 1)*32 + 0)]

        ldp             q8,  q9,  [x11, #((0 + 1)*32 + 0)]
        ldp             q10, q11, [x11, #((2 + 1)*32 + 0)]
        ldp             q12, q13, [x12, #((0 + 1)*32 + 0)]
        ldp             q14, q15, [x12, #((2 + 1)*32 + 0)]


        ld1             { v24.4s, v25.4s }, [x8], x9
        ld1             { v16.4s, v17.4s }, [x7], #32

        ext8             v18.16b, v24.16b, v24.16b, #8
        ext8             v19.16b, v25.16b, v25.16b, #8

        trn1            v20.4s, v16.4s, v16.4s      
        trn2            v16.4s, v16.4s, v16.4s      
        trn1            v21.4s, v17.4s, v17.4s      
        trn2            v17.4s, v17.4s, v17.4s      

        rev64           v22.4s, v8.4s                 
        rev64           v23.4s, v10.4s                 
        rev64           v24.4s, v12.4s                 
        rev64           v25.4s, v14.4s                 

        fmul            v22.4s, v22.4s, v20.4s      
        fmul            v23.4s, v23.4s, v16.4s      
        fmul            v24.4s, v24.4s, v20.4s      
        fmul            v25.4s, v25.4s, v16.4s      

        rev64           v26.4s, v9.4s                 
        rev64           v27.4s, v11.4s                 
        rev64           v20.4s, v13.4s                 
        rev64           v16.4s, v15.4s                 

        fmul            v26.4s, v26.4s, v21.4s      
        fmul            v27.4s, v27.4s, v17.4s      
        fmul            v20.4s, v20.4s, v21.4s      
        fmul            v16.4s, v16.4s, v17.4s      

        trn1            v21.4s, v19.4s, v19.4s      
        trn2            v19.4s, v19.4s, v19.4s      
        trn1            v17.4s, v18.4s, v18.4s      
        trn2            v18.4s, v18.4s, v18.4s      

        fmul            v21.4s, v21.4s, v31.4s
        fmul            v19.4s, v19.4s, v31.4s
        fmul            v17.4s, v17.4s, v31.4s
        fmul            v18.4s, v18.4s, v31.4s

        fmla            v23.4s, v10.4s, v21.4s      
        fmls            v25.4s, v14.4s, v21.4s      
        fmla            v22.4s, v8.4s, v19.4s      
        fmls            v24.4s, v12.4s, v19.4s      

        fmla            v26.4s, v9.4s, v18.4s      
        fmla            v27.4s, v11.4s, v17.4s      
        fmls            v20.4s, v13.4s, v18.4s      
        fmls            v16.4s, v15.4s, v17.4s      

        fsub            v18.4s, v23.4s, v25.4s
        fsub            v17.4s, v24.4s, v22.4s
        fsub            v19.4s, v20.4s, v26.4s
        fsub            v21.4s, v16.4s, v27.4s

        fadd            v22.4s, v24.4s, v22.4s
        fadd            v23.4s, v25.4s, v23.4s
        fadd            v24.4s, v20.4s, v26.4s
        fadd            v25.4s, v16.4s, v27.4s

        fmul            v17.4s, v17.4s, v31.4s
        fmul            v18.4s, v18.4s, v31.4s
        fmul            v19.4s, v19.4s, v31.4s
        fmul            v21.4s, v21.4s, v31.4s

        rev64           v22.4s, v22.4s
        rev64           v24.4s, v24.4s
        rev64           v23.4s, v23.4s
        rev64           v25.4s, v25.4s

        fsub            v8.4s, v0.4s, v22.4s
        fsub            v9.4s, v1.4s, v24.4s
        fsub            v10.4s, v2.4s, v17.4s
        fsub            v11.4s, v3.4s, v19.4s

        fsub            v12.4s, v4.4s, v23.4s
        fsub            v13.4s, v5.4s, v25.4s
        fadd            v14.4s, v6.4s, v18.4s
        fsub            v15.4s, v7.4s, v21.4s

        fadd            v0.4s, v0.4s, v22.4s
        fadd            v1.4s, v1.4s, v24.4s
        fadd            v2.4s, v2.4s, v17.4s
        fadd            v3.4s, v3.4s, v19.4s

        fadd            v4.4s, v4.4s, v23.4s
        fsub            v6.4s, v6.4s, v18.4s      
        fadd            v5.4s, v5.4s, v25.4s      
        fadd            v7.4s, v7.4s, v21.4s

        stp             q0,  q1,  [x1,  #((0 + 1)*32 + 0)]
        stp             q4,  q5,  [x1,  #((2 + 1)*32 + 0)]
        stp             q2,  q3,  [x10, #((0 + 1)*32 + 0)]
        stp             q6,  q7,  [x10, #((2 + 1)*32 + 0)]

        stp             q8,  q9,  [x11, #((0 + 1)*32 + 0)]
        stp             q12, q13, [x11, #((2 + 1)*32 + 0)]
        stp             q10, q11, [x12, #((0 + 1)*32 + 0)]
        stp             q14, q15, [x12, #((2 + 1)*32 + 0)]
        add             x10, x1, x21
        add             x11, x1, x21, lsl #1
        add             x12, x1, x22

        ldp             q0,  q1,  [x1,  #((0 + 4)*32 + 0)]
        ldp             q4,  q5,  [x1,  #((2 + 4)*32 + 0)]
        ldp             q2,  q3,  [x10, #((0 + 4)*32 + 0)]
        ldp             q6,  q7,  [x10, #((2 + 4)*32 + 0)]

        ldp             q8,  q9,  [x11, #((0 + 4)*32 + 0)]
        ldp             q10, q11, [x11, #((2 + 4)*32 + 0)]
        ldp             q12, q13, [x12, #((0 + 4)*32 + 0)]
        ldp             q14, q15, [x12, #((2 + 4)*32 + 0)]


        ld1             { v24.4s, v25.4s }, [x8], x9
        ld1             { v16.4s, v17.4s }, [x7], #32

        ext8             v18.16b, v24.16b, v24.16b, #8
        ext8             v19.16b, v25.16b, v25.16b, #8

        trn1            v20.4s, v16.4s, v16.4s      
        trn2            v16.4s, v16.4s, v16.4s      
        trn1            v21.4s, v17.4s, v17.4s      
        trn2            v17.4s, v17.4s, v17.4s      

        rev64           v22.4s, v8.4s                 
        rev64           v23.4s, v10.4s                 
        rev64           v24.4s, v12.4s                 
        rev64           v25.4s, v14.4s                 

        fmul            v22.4s, v22.4s, v20.4s      
        fmul            v23.4s, v23.4s, v16.4s      
        fmul            v24.4s, v24.4s, v20.4s      
        fmul            v25.4s, v25.4s, v16.4s      

        rev64           v26.4s, v9.4s                 
        rev64           v27.4s, v11.4s                 
        rev64           v20.4s, v13.4s                 
        rev64           v16.4s, v15.4s                 

        fmul            v26.4s, v26.4s, v21.4s      
        fmul            v27.4s, v27.4s, v17.4s      
        fmul            v20.4s, v20.4s, v21.4s      
        fmul            v16.4s, v16.4s, v17.4s      

        trn1            v21.4s, v19.4s, v19.4s      
        trn2            v19.4s, v19.4s, v19.4s      
        trn1            v17.4s, v18.4s, v18.4s      
        trn2            v18.4s, v18.4s, v18.4s      

        fmul            v21.4s, v21.4s, v31.4s
        fmul            v19.4s, v19.4s, v31.4s
        fmul            v17.4s, v17.4s, v31.4s
        fmul            v18.4s, v18.4s, v31.4s

        fmla            v23.4s, v10.4s, v21.4s      
        fmls            v25.4s, v14.4s, v21.4s      
        fmla            v22.4s, v8.4s, v19.4s      
        fmls            v24.4s, v12.4s, v19.4s      

        fmla            v26.4s, v9.4s, v18.4s      
        fmla            v27.4s, v11.4s, v17.4s      
        fmls            v20.4s, v13.4s, v18.4s      
        fmls            v16.4s, v15.4s, v17.4s      

        fsub            v18.4s, v23.4s, v25.4s
        fsub            v17.4s, v24.4s, v22.4s
        fsub            v19.4s, v20.4s, v26.4s
        fsub            v21.4s, v16.4s, v27.4s

        fadd            v22.4s, v24.4s, v22.4s
        fadd            v23.4s, v25.4s, v23.4s
        fadd            v24.4s, v20.4s, v26.4s
        fadd            v25.4s, v16.4s, v27.4s

        fmul            v17.4s, v17.4s, v31.4s
        fmul            v18.4s, v18.4s, v31.4s
        fmul            v19.4s, v19.4s, v31.4s
        fmul            v21.4s, v21.4s, v31.4s

        rev64           v22.4s, v22.4s
        rev64           v24.4s, v24.4s
        rev64           v23.4s, v23.4s
        rev64           v25.4s, v25.4s

        fsub            v8.4s, v0.4s, v22.4s
        fsub            v9.4s, v1.4s, v24.4s
        fsub            v10.4s, v2.4s, v17.4s
        fsub            v11.4s, v3.4s, v19.4s

        fsub            v12.4s, v4.4s, v23.4s
        fsub            v13.4s, v5.4s, v25.4s
        fadd            v14.4s, v6.4s, v18.4s
        fsub            v15.4s, v7.4s, v21.4s

        fadd            v0.4s, v0.4s, v22.4s
        fadd            v1.4s, v1.4s, v24.4s
        fadd            v2.4s, v2.4s, v17.4s
        fadd            v3.4s, v3.4s, v19.4s

        fadd            v4.4s, v4.4s, v23.4s
        fsub            v6.4s, v6.4s, v18.4s      
        fadd            v5.4s, v5.4s, v25.4s      
        fadd            v7.4s, v7.4s, v21.4s

        stp             q0,  q1,  [x1,  #((0 + 4)*32 + 0)]
        stp             q4,  q5,  [x1,  #((2 + 4)*32 + 0)]
        stp             q2,  q3,  [x10, #((0 + 4)*32 + 0)]
        stp             q6,  q7,  [x10, #((2 + 4)*32 + 0)]

        stp             q8,  q9,  [x11, #((0 + 4)*32 + 0)]
        stp             q12, q13, [x11, #((2 + 4)*32 + 0)]
        stp             q10, q11, [x12, #((0 + 4)*32 + 0)]
        stp             q14, q15, [x12, #((2 + 4)*32 + 0)]
        add             x10, x1, x21
        add             x11, x1, x21, lsl #1
        add             x12, x1, x22

        ldp             q0,  q1,  [x1,  #((0 + 5)*32 + 0)]
        ldp             q4,  q5,  [x1,  #((2 + 5)*32 + 0)]
        ldp             q2,  q3,  [x10, #((0 + 5)*32 + 0)]
        ldp             q6,  q7,  [x10, #((2 + 5)*32 + 0)]

        ldp             q8,  q9,  [x11, #((0 + 5)*32 + 0)]
        ldp             q10, q11, [x11, #((2 + 5)*32 + 0)]
        ldp             q12, q13, [x12, #((0 + 5)*32 + 0)]
        ldp             q14, q15, [x12, #((2 + 5)*32 + 0)]


        ld1             { v24.4s, v25.4s }, [x8], x9
        ld1             { v16.4s, v17.4s }, [x7], #32

        ext8             v18.16b, v24.16b, v24.16b, #8
        ext8             v19.16b, v25.16b, v25.16b, #8

        trn1            v20.4s, v16.4s, v16.4s      
        trn2            v16.4s, v16.4s, v16.4s      
        trn1            v21.4s, v17.4s, v17.4s      
        trn2            v17.4s, v17.4s, v17.4s      

        rev64           v22.4s, v8.4s                 
        rev64           v23.4s, v10.4s                 
        rev64           v24.4s, v12.4s                 
        rev64           v25.4s, v14.4s                 

        fmul            v22.4s, v22.4s, v20.4s      
        fmul            v23.4s, v23.4s, v16.4s      
        fmul            v24.4s, v24.4s, v20.4s      
        fmul            v25.4s, v25.4s, v16.4s      

        rev64           v26.4s, v9.4s                 
        rev64           v27.4s, v11.4s                 
        rev64           v20.4s, v13.4s                 
        rev64           v16.4s, v15.4s                 

        fmul            v26.4s, v26.4s, v21.4s      
        fmul            v27.4s, v27.4s, v17.4s      
        fmul            v20.4s, v20.4s, v21.4s      
        fmul            v16.4s, v16.4s, v17.4s      

        trn1            v21.4s, v19.4s, v19.4s      
        trn2            v19.4s, v19.4s, v19.4s      
        trn1            v17.4s, v18.4s, v18.4s      
        trn2            v18.4s, v18.4s, v18.4s      

        fmul            v21.4s, v21.4s, v31.4s
        fmul            v19.4s, v19.4s, v31.4s
        fmul            v17.4s, v17.4s, v31.4s
        fmul            v18.4s, v18.4s, v31.4s

        fmla            v23.4s, v10.4s, v21.4s      
        fmls            v25.4s, v14.4s, v21.4s      
        fmla            v22.4s, v8.4s, v19.4s      
        fmls            v24.4s, v12.4s, v19.4s      

        fmla            v26.4s, v9.4s, v18.4s      
        fmla            v27.4s, v11.4s, v17.4s      
        fmls            v20.4s, v13.4s, v18.4s      
        fmls            v16.4s, v15.4s, v17.4s      

        fsub            v18.4s, v23.4s, v25.4s
        fsub            v17.4s, v24.4s, v22.4s
        fsub            v19.4s, v20.4s, v26.4s
        fsub            v21.4s, v16.4s, v27.4s

        fadd            v22.4s, v24.4s, v22.4s
        fadd            v23.4s, v25.4s, v23.4s
        fadd            v24.4s, v20.4s, v26.4s
        fadd            v25.4s, v16.4s, v27.4s

        fmul            v17.4s, v17.4s, v31.4s
        fmul            v18.4s, v18.4s, v31.4s
        fmul            v19.4s, v19.4s, v31.4s
        fmul            v21.4s, v21.4s, v31.4s

        rev64           v22.4s, v22.4s
        rev64           v24.4s, v24.4s
        rev64           v23.4s, v23.4s
        rev64           v25.4s, v25.4s

        fsub            v8.4s, v0.4s, v22.4s
        fsub            v9.4s, v1.4s, v24.4s
        fsub            v10.4s, v2.4s, v17.4s
        fsub            v11.4s, v3.4s, v19.4s

        fsub            v12.4s, v4.4s, v23.4s
        fsub            v13.4s, v5.4s, v25.4s
        fadd            v14.4s, v6.4s, v18.4s
        fsub            v15.4s, v7.4s, v21.4s

        fadd            v0.4s, v0.4s, v22.4s
        fadd            v1.4s, v1.4s, v24.4s
        fadd            v2.4s, v2.4s, v17.4s
        fadd            v3.4s, v3.4s, v19.4s

        fadd            v4.4s, v4.4s, v23.4s
        fsub            v6.4s, v6.4s, v18.4s      
        fadd            v5.4s, v5.4s, v25.4s      
        fadd            v7.4s, v7.4s, v21.4s

        stp             q0,  q1,  [x1,  #((0 + 5)*32 + 0)]
        stp             q4,  q5,  [x1,  #((2 + 5)*32 + 0)]
        stp             q2,  q3,  [x10, #((0 + 5)*32 + 0)]
        stp             q6,  q7,  [x10, #((2 + 5)*32 + 0)]

        stp             q8,  q9,  [x11, #((0 + 5)*32 + 0)]
        stp             q12, q13, [x11, #((2 + 5)*32 + 0)]
        stp             q10, q11, [x12, #((0 + 5)*32 + 0)]
        stp             q14, q15, [x12, #((2 + 5)*32 + 0)]
        add             x1, x1, 8*32
        subs            w5, w5, 1
        bgt            temp_label_83

        cmp             w20, #((32768)/4096), lsl #12
        bgt            temp_label_84
        ret
temp_label_85
temp_label_84

        stp             x20, x30, [sp, #-16]!
        mov             w20, #(65536/4)
        mov             x5, #((65536*4) - (65536/1))
        add             x1, x1, x5
        bl              temp_label_46
        mov             x5, #((65536*2) - (65536/2))
        add             x1, x1, x5
        bl              temp_label_46
        ldp             x20, x30, [sp], #16
        ldr             w5, =(65536*6 + 65536/2)
        sub             x1, x1, x5

        ldr             w5, =(65536 - 4*7)










        adrp            x7, ff_tx_tab_65536_float
        add             x7, x7, ff_tx_tab_65536_float










        add             x8, x7, x5
        mov             x9, #-32

        mov             x21, #2*65536
        add             x22, x21, x21, lsl #1

        cmp             w19, #((65536)/4096), lsl #12
        beq            temp_label_86

        mov             w5, #(65536/128)
temp_label_87

        add             x10, x1, x21
        add             x11, x1, x21, lsl #1
        add             x12, x1, x22

        add             x10, x1, x21
        add             x11, x1, x21, lsl #1
        add             x12, x1, x22

        ldp             q0,  q1,  [x1,  #((0 + 0)*32 + 0)]
        ldp             q4,  q5,  [x1,  #((2 + 0)*32 + 0)]
        ldp             q2,  q3,  [x10, #((0 + 0)*32 + 0)]
        ldp             q6,  q7,  [x10, #((2 + 0)*32 + 0)]

        ldp             q8,  q9,  [x11, #((0 + 0)*32 + 0)]
        ldp             q10, q11, [x11, #((2 + 0)*32 + 0)]
        ldp             q12, q13, [x12, #((0 + 0)*32 + 0)]
        ldp             q14, q15, [x12, #((2 + 0)*32 + 0)]


        ld1             { v24.4s, v25.4s }, [x8], x9
        ld1             { v16.4s, v17.4s }, [x7], #32

        ext8             v18.16b, v24.16b, v24.16b, #8
        ext8             v19.16b, v25.16b, v25.16b, #8

        trn1            v20.4s, v16.4s, v16.4s      
        trn2            v16.4s, v16.4s, v16.4s      
        trn1            v21.4s, v17.4s, v17.4s      
        trn2            v17.4s, v17.4s, v17.4s      

        rev64           v22.4s, v8.4s                 
        rev64           v23.4s, v10.4s                 
        rev64           v24.4s, v12.4s                 
        rev64           v25.4s, v14.4s                 

        fmul            v22.4s, v22.4s, v20.4s      
        fmul            v23.4s, v23.4s, v16.4s      
        fmul            v24.4s, v24.4s, v20.4s      
        fmul            v25.4s, v25.4s, v16.4s      

        rev64           v26.4s, v9.4s                 
        rev64           v27.4s, v11.4s                 
        rev64           v20.4s, v13.4s                 
        rev64           v16.4s, v15.4s                 

        fmul            v26.4s, v26.4s, v21.4s      
        fmul            v27.4s, v27.4s, v17.4s      
        fmul            v20.4s, v20.4s, v21.4s      
        fmul            v16.4s, v16.4s, v17.4s      

        trn1            v21.4s, v19.4s, v19.4s      
        trn2            v19.4s, v19.4s, v19.4s      
        trn1            v17.4s, v18.4s, v18.4s      
        trn2            v18.4s, v18.4s, v18.4s      

        fmul            v21.4s, v21.4s, v31.4s
        fmul            v19.4s, v19.4s, v31.4s
        fmul            v17.4s, v17.4s, v31.4s
        fmul            v18.4s, v18.4s, v31.4s

        fmla            v23.4s, v10.4s, v21.4s      
        fmls            v25.4s, v14.4s, v21.4s      
        fmla            v22.4s, v8.4s, v19.4s      
        fmls            v24.4s, v12.4s, v19.4s      

        fmla            v26.4s, v9.4s, v18.4s      
        fmla            v27.4s, v11.4s, v17.4s      
        fmls            v20.4s, v13.4s, v18.4s      
        fmls            v16.4s, v15.4s, v17.4s      

        fsub            v18.4s, v23.4s, v25.4s
        fsub            v17.4s, v24.4s, v22.4s
        fsub            v19.4s, v20.4s, v26.4s
        fsub            v21.4s, v16.4s, v27.4s

        fadd            v22.4s, v24.4s, v22.4s
        fadd            v23.4s, v25.4s, v23.4s
        fadd            v24.4s, v20.4s, v26.4s
        fadd            v25.4s, v16.4s, v27.4s

        fmul            v17.4s, v17.4s, v31.4s
        fmul            v18.4s, v18.4s, v31.4s
        fmul            v19.4s, v19.4s, v31.4s
        fmul            v21.4s, v21.4s, v31.4s

        rev64           v22.4s, v22.4s
        rev64           v24.4s, v24.4s
        rev64           v23.4s, v23.4s
        rev64           v25.4s, v25.4s

        fsub            v8.4s, v0.4s, v22.4s
        fsub            v9.4s, v1.4s, v24.4s
        fsub            v10.4s, v2.4s, v17.4s
        fsub            v11.4s, v3.4s, v19.4s

        fsub            v12.4s, v4.4s, v23.4s
        fsub            v13.4s, v5.4s, v25.4s
        fadd            v14.4s, v6.4s, v18.4s
        fsub            v15.4s, v7.4s, v21.4s

        fadd            v0.4s, v0.4s, v22.4s
        fadd            v1.4s, v1.4s, v24.4s
        fadd            v2.4s, v2.4s, v17.4s
        fadd            v3.4s, v3.4s, v19.4s

        fadd            v4.4s, v4.4s, v23.4s
        fsub            v6.4s, v6.4s, v18.4s      
        fadd            v5.4s, v5.4s, v25.4s      
        fadd            v7.4s, v7.4s, v21.4s

        stp             q0,  q1,  [x1,  #((0 + 0)*32 + 0)]
        stp             q4,  q5,  [x1,  #((2 + 0)*32 + 0)]
        stp             q2,  q3,  [x10, #((0 + 0)*32 + 0)]
        stp             q6,  q7,  [x10, #((2 + 0)*32 + 0)]

        stp             q8,  q9,  [x11, #((0 + 0)*32 + 0)]
        stp             q12, q13, [x11, #((2 + 0)*32 + 0)]
        stp             q10, q11, [x12, #((0 + 0)*32 + 0)]
        stp             q14, q15, [x12, #((2 + 0)*32 + 0)]
        add             x10, x1, x21
        add             x11, x1, x21, lsl #1
        add             x12, x1, x22

        ldp             q0,  q1,  [x1,  #((0 + 1)*32 + 0)]
        ldp             q4,  q5,  [x1,  #((2 + 1)*32 + 0)]
        ldp             q2,  q3,  [x10, #((0 + 1)*32 + 0)]
        ldp             q6,  q7,  [x10, #((2 + 1)*32 + 0)]

        ldp             q8,  q9,  [x11, #((0 + 1)*32 + 0)]
        ldp             q10, q11, [x11, #((2 + 1)*32 + 0)]
        ldp             q12, q13, [x12, #((0 + 1)*32 + 0)]
        ldp             q14, q15, [x12, #((2 + 1)*32 + 0)]


        ld1             { v24.4s, v25.4s }, [x8], x9
        ld1             { v16.4s, v17.4s }, [x7], #32

        ext8             v18.16b, v24.16b, v24.16b, #8
        ext8             v19.16b, v25.16b, v25.16b, #8

        trn1            v20.4s, v16.4s, v16.4s      
        trn2            v16.4s, v16.4s, v16.4s      
        trn1            v21.4s, v17.4s, v17.4s      
        trn2            v17.4s, v17.4s, v17.4s      

        rev64           v22.4s, v8.4s                 
        rev64           v23.4s, v10.4s                 
        rev64           v24.4s, v12.4s                 
        rev64           v25.4s, v14.4s                 

        fmul            v22.4s, v22.4s, v20.4s      
        fmul            v23.4s, v23.4s, v16.4s      
        fmul            v24.4s, v24.4s, v20.4s      
        fmul            v25.4s, v25.4s, v16.4s      

        rev64           v26.4s, v9.4s                 
        rev64           v27.4s, v11.4s                 
        rev64           v20.4s, v13.4s                 
        rev64           v16.4s, v15.4s                 

        fmul            v26.4s, v26.4s, v21.4s      
        fmul            v27.4s, v27.4s, v17.4s      
        fmul            v20.4s, v20.4s, v21.4s      
        fmul            v16.4s, v16.4s, v17.4s      

        trn1            v21.4s, v19.4s, v19.4s      
        trn2            v19.4s, v19.4s, v19.4s      
        trn1            v17.4s, v18.4s, v18.4s      
        trn2            v18.4s, v18.4s, v18.4s      

        fmul            v21.4s, v21.4s, v31.4s
        fmul            v19.4s, v19.4s, v31.4s
        fmul            v17.4s, v17.4s, v31.4s
        fmul            v18.4s, v18.4s, v31.4s

        fmla            v23.4s, v10.4s, v21.4s      
        fmls            v25.4s, v14.4s, v21.4s      
        fmla            v22.4s, v8.4s, v19.4s      
        fmls            v24.4s, v12.4s, v19.4s      

        fmla            v26.4s, v9.4s, v18.4s      
        fmla            v27.4s, v11.4s, v17.4s      
        fmls            v20.4s, v13.4s, v18.4s      
        fmls            v16.4s, v15.4s, v17.4s      

        fsub            v18.4s, v23.4s, v25.4s
        fsub            v17.4s, v24.4s, v22.4s
        fsub            v19.4s, v20.4s, v26.4s
        fsub            v21.4s, v16.4s, v27.4s

        fadd            v22.4s, v24.4s, v22.4s
        fadd            v23.4s, v25.4s, v23.4s
        fadd            v24.4s, v20.4s, v26.4s
        fadd            v25.4s, v16.4s, v27.4s

        fmul            v17.4s, v17.4s, v31.4s
        fmul            v18.4s, v18.4s, v31.4s
        fmul            v19.4s, v19.4s, v31.4s
        fmul            v21.4s, v21.4s, v31.4s

        rev64           v22.4s, v22.4s
        rev64           v24.4s, v24.4s
        rev64           v23.4s, v23.4s
        rev64           v25.4s, v25.4s

        fsub            v8.4s, v0.4s, v22.4s
        fsub            v9.4s, v1.4s, v24.4s
        fsub            v10.4s, v2.4s, v17.4s
        fsub            v11.4s, v3.4s, v19.4s

        fsub            v12.4s, v4.4s, v23.4s
        fsub            v13.4s, v5.4s, v25.4s
        fadd            v14.4s, v6.4s, v18.4s
        fsub            v15.4s, v7.4s, v21.4s

        fadd            v0.4s, v0.4s, v22.4s
        fadd            v1.4s, v1.4s, v24.4s
        fadd            v2.4s, v2.4s, v17.4s
        fadd            v3.4s, v3.4s, v19.4s

        fadd            v4.4s, v4.4s, v23.4s
        fsub            v6.4s, v6.4s, v18.4s      
        fadd            v5.4s, v5.4s, v25.4s      
        fadd            v7.4s, v7.4s, v21.4s

        stp             q0,  q1,  [x1,  #((0 + 1)*32 + 0)]
        stp             q4,  q5,  [x1,  #((2 + 1)*32 + 0)]
        stp             q2,  q3,  [x10, #((0 + 1)*32 + 0)]
        stp             q6,  q7,  [x10, #((2 + 1)*32 + 0)]

        stp             q8,  q9,  [x11, #((0 + 1)*32 + 0)]
        stp             q12, q13, [x11, #((2 + 1)*32 + 0)]
        stp             q10, q11, [x12, #((0 + 1)*32 + 0)]
        stp             q14, q15, [x12, #((2 + 1)*32 + 0)]
        add             x10, x1, x21
        add             x11, x1, x21, lsl #1
        add             x12, x1, x22

        ldp             q0,  q1,  [x1,  #((0 + 4)*32 + 0)]
        ldp             q4,  q5,  [x1,  #((2 + 4)*32 + 0)]
        ldp             q2,  q3,  [x10, #((0 + 4)*32 + 0)]
        ldp             q6,  q7,  [x10, #((2 + 4)*32 + 0)]

        ldp             q8,  q9,  [x11, #((0 + 4)*32 + 0)]
        ldp             q10, q11, [x11, #((2 + 4)*32 + 0)]
        ldp             q12, q13, [x12, #((0 + 4)*32 + 0)]
        ldp             q14, q15, [x12, #((2 + 4)*32 + 0)]


        ld1             { v24.4s, v25.4s }, [x8], x9
        ld1             { v16.4s, v17.4s }, [x7], #32

        ext8             v18.16b, v24.16b, v24.16b, #8
        ext8             v19.16b, v25.16b, v25.16b, #8

        trn1            v20.4s, v16.4s, v16.4s      
        trn2            v16.4s, v16.4s, v16.4s      
        trn1            v21.4s, v17.4s, v17.4s      
        trn2            v17.4s, v17.4s, v17.4s      

        rev64           v22.4s, v8.4s                 
        rev64           v23.4s, v10.4s                 
        rev64           v24.4s, v12.4s                 
        rev64           v25.4s, v14.4s                 

        fmul            v22.4s, v22.4s, v20.4s      
        fmul            v23.4s, v23.4s, v16.4s      
        fmul            v24.4s, v24.4s, v20.4s      
        fmul            v25.4s, v25.4s, v16.4s      

        rev64           v26.4s, v9.4s                 
        rev64           v27.4s, v11.4s                 
        rev64           v20.4s, v13.4s                 
        rev64           v16.4s, v15.4s                 

        fmul            v26.4s, v26.4s, v21.4s      
        fmul            v27.4s, v27.4s, v17.4s      
        fmul            v20.4s, v20.4s, v21.4s      
        fmul            v16.4s, v16.4s, v17.4s      

        trn1            v21.4s, v19.4s, v19.4s      
        trn2            v19.4s, v19.4s, v19.4s      
        trn1            v17.4s, v18.4s, v18.4s      
        trn2            v18.4s, v18.4s, v18.4s      

        fmul            v21.4s, v21.4s, v31.4s
        fmul            v19.4s, v19.4s, v31.4s
        fmul            v17.4s, v17.4s, v31.4s
        fmul            v18.4s, v18.4s, v31.4s

        fmla            v23.4s, v10.4s, v21.4s      
        fmls            v25.4s, v14.4s, v21.4s      
        fmla            v22.4s, v8.4s, v19.4s      
        fmls            v24.4s, v12.4s, v19.4s      

        fmla            v26.4s, v9.4s, v18.4s      
        fmla            v27.4s, v11.4s, v17.4s      
        fmls            v20.4s, v13.4s, v18.4s      
        fmls            v16.4s, v15.4s, v17.4s      

        fsub            v18.4s, v23.4s, v25.4s
        fsub            v17.4s, v24.4s, v22.4s
        fsub            v19.4s, v20.4s, v26.4s
        fsub            v21.4s, v16.4s, v27.4s

        fadd            v22.4s, v24.4s, v22.4s
        fadd            v23.4s, v25.4s, v23.4s
        fadd            v24.4s, v20.4s, v26.4s
        fadd            v25.4s, v16.4s, v27.4s

        fmul            v17.4s, v17.4s, v31.4s
        fmul            v18.4s, v18.4s, v31.4s
        fmul            v19.4s, v19.4s, v31.4s
        fmul            v21.4s, v21.4s, v31.4s

        rev64           v22.4s, v22.4s
        rev64           v24.4s, v24.4s
        rev64           v23.4s, v23.4s
        rev64           v25.4s, v25.4s

        fsub            v8.4s, v0.4s, v22.4s
        fsub            v9.4s, v1.4s, v24.4s
        fsub            v10.4s, v2.4s, v17.4s
        fsub            v11.4s, v3.4s, v19.4s

        fsub            v12.4s, v4.4s, v23.4s
        fsub            v13.4s, v5.4s, v25.4s
        fadd            v14.4s, v6.4s, v18.4s
        fsub            v15.4s, v7.4s, v21.4s

        fadd            v0.4s, v0.4s, v22.4s
        fadd            v1.4s, v1.4s, v24.4s
        fadd            v2.4s, v2.4s, v17.4s
        fadd            v3.4s, v3.4s, v19.4s

        fadd            v4.4s, v4.4s, v23.4s
        fsub            v6.4s, v6.4s, v18.4s      
        fadd            v5.4s, v5.4s, v25.4s      
        fadd            v7.4s, v7.4s, v21.4s

        stp             q0,  q1,  [x1,  #((0 + 4)*32 + 0)]
        stp             q4,  q5,  [x1,  #((2 + 4)*32 + 0)]
        stp             q2,  q3,  [x10, #((0 + 4)*32 + 0)]
        stp             q6,  q7,  [x10, #((2 + 4)*32 + 0)]

        stp             q8,  q9,  [x11, #((0 + 4)*32 + 0)]
        stp             q12, q13, [x11, #((2 + 4)*32 + 0)]
        stp             q10, q11, [x12, #((0 + 4)*32 + 0)]
        stp             q14, q15, [x12, #((2 + 4)*32 + 0)]
        add             x10, x1, x21
        add             x11, x1, x21, lsl #1
        add             x12, x1, x22

        ldp             q0,  q1,  [x1,  #((0 + 5)*32 + 0)]
        ldp             q4,  q5,  [x1,  #((2 + 5)*32 + 0)]
        ldp             q2,  q3,  [x10, #((0 + 5)*32 + 0)]
        ldp             q6,  q7,  [x10, #((2 + 5)*32 + 0)]

        ldp             q8,  q9,  [x11, #((0 + 5)*32 + 0)]
        ldp             q10, q11, [x11, #((2 + 5)*32 + 0)]
        ldp             q12, q13, [x12, #((0 + 5)*32 + 0)]
        ldp             q14, q15, [x12, #((2 + 5)*32 + 0)]


        ld1             { v24.4s, v25.4s }, [x8], x9
        ld1             { v16.4s, v17.4s }, [x7], #32

        ext8             v18.16b, v24.16b, v24.16b, #8
        ext8             v19.16b, v25.16b, v25.16b, #8

        trn1            v20.4s, v16.4s, v16.4s      
        trn2            v16.4s, v16.4s, v16.4s      
        trn1            v21.4s, v17.4s, v17.4s      
        trn2            v17.4s, v17.4s, v17.4s      

        rev64           v22.4s, v8.4s                 
        rev64           v23.4s, v10.4s                 
        rev64           v24.4s, v12.4s                 
        rev64           v25.4s, v14.4s                 

        fmul            v22.4s, v22.4s, v20.4s      
        fmul            v23.4s, v23.4s, v16.4s      
        fmul            v24.4s, v24.4s, v20.4s      
        fmul            v25.4s, v25.4s, v16.4s      

        rev64           v26.4s, v9.4s                 
        rev64           v27.4s, v11.4s                 
        rev64           v20.4s, v13.4s                 
        rev64           v16.4s, v15.4s                 

        fmul            v26.4s, v26.4s, v21.4s      
        fmul            v27.4s, v27.4s, v17.4s      
        fmul            v20.4s, v20.4s, v21.4s      
        fmul            v16.4s, v16.4s, v17.4s      

        trn1            v21.4s, v19.4s, v19.4s      
        trn2            v19.4s, v19.4s, v19.4s      
        trn1            v17.4s, v18.4s, v18.4s      
        trn2            v18.4s, v18.4s, v18.4s      

        fmul            v21.4s, v21.4s, v31.4s
        fmul            v19.4s, v19.4s, v31.4s
        fmul            v17.4s, v17.4s, v31.4s
        fmul            v18.4s, v18.4s, v31.4s

        fmla            v23.4s, v10.4s, v21.4s      
        fmls            v25.4s, v14.4s, v21.4s      
        fmla            v22.4s, v8.4s, v19.4s      
        fmls            v24.4s, v12.4s, v19.4s      

        fmla            v26.4s, v9.4s, v18.4s      
        fmla            v27.4s, v11.4s, v17.4s      
        fmls            v20.4s, v13.4s, v18.4s      
        fmls            v16.4s, v15.4s, v17.4s      

        fsub            v18.4s, v23.4s, v25.4s
        fsub            v17.4s, v24.4s, v22.4s
        fsub            v19.4s, v20.4s, v26.4s
        fsub            v21.4s, v16.4s, v27.4s

        fadd            v22.4s, v24.4s, v22.4s
        fadd            v23.4s, v25.4s, v23.4s
        fadd            v24.4s, v20.4s, v26.4s
        fadd            v25.4s, v16.4s, v27.4s

        fmul            v17.4s, v17.4s, v31.4s
        fmul            v18.4s, v18.4s, v31.4s
        fmul            v19.4s, v19.4s, v31.4s
        fmul            v21.4s, v21.4s, v31.4s

        rev64           v22.4s, v22.4s
        rev64           v24.4s, v24.4s
        rev64           v23.4s, v23.4s
        rev64           v25.4s, v25.4s

        fsub            v8.4s, v0.4s, v22.4s
        fsub            v9.4s, v1.4s, v24.4s
        fsub            v10.4s, v2.4s, v17.4s
        fsub            v11.4s, v3.4s, v19.4s

        fsub            v12.4s, v4.4s, v23.4s
        fsub            v13.4s, v5.4s, v25.4s
        fadd            v14.4s, v6.4s, v18.4s
        fsub            v15.4s, v7.4s, v21.4s

        fadd            v0.4s, v0.4s, v22.4s
        fadd            v1.4s, v1.4s, v24.4s
        fadd            v2.4s, v2.4s, v17.4s
        fadd            v3.4s, v3.4s, v19.4s

        fadd            v4.4s, v4.4s, v23.4s
        fsub            v6.4s, v6.4s, v18.4s      
        fadd            v5.4s, v5.4s, v25.4s      
        fadd            v7.4s, v7.4s, v21.4s

        stp             q0,  q1,  [x1,  #((0 + 5)*32 + 0)]
        stp             q4,  q5,  [x1,  #((2 + 5)*32 + 0)]
        stp             q2,  q3,  [x10, #((0 + 5)*32 + 0)]
        stp             q6,  q7,  [x10, #((2 + 5)*32 + 0)]

        stp             q8,  q9,  [x11, #((0 + 5)*32 + 0)]
        stp             q12, q13, [x11, #((2 + 5)*32 + 0)]
        stp             q10, q11, [x12, #((0 + 5)*32 + 0)]
        stp             q14, q15, [x12, #((2 + 5)*32 + 0)]
        add             x1, x1, 8*32
        subs            w5, w5, 1
        bgt            temp_label_87

        cmp             w20, #((65536)/4096), lsl #12
        bgt            temp_label_88
        ret
temp_label_89
temp_label_88

        stp             x20, x30, [sp, #-16]!
        mov             w20, #(131072/4)
        mov             x5, #((131072*4) - (131072/1))
        add             x1, x1, x5
        bl              temp_label_46
        mov             x5, #((131072*2) - (131072/2))
        add             x1, x1, x5
        bl              temp_label_46
        ldp             x20, x30, [sp], #16
        ldr             w5, =(131072*6 + 131072/2)
        sub             x1, x1, x5

        ldr             w5, =(131072 - 4*7)










        adrp            x7, ff_tx_tab_131072_float
        add             x7, x7, ff_tx_tab_131072_float










        add             x8, x7, x5
        mov             x9, #-32

        mov             x21, #2*131072
        add             x22, x21, x21, lsl #1


temp_label_90
temp_label_86
temp_label_82
temp_label_78
temp_label_74
temp_label_70
temp_label_66
temp_label_62
temp_label_58
temp_label_55
temp_label_52
 
        add             x14, x1, x21
        add             x15, x1, x21, lsl #1
        add             x16, x1, x22

        add             x10,  x1, #((0)*32 + 0)
        add             x11, x14, #((0)*32 + 0)
        add             x12, x15, #((0)*32 + 0)
        add             x13, x16, #((0)*32 + 0)

        ldp             q0,  q1,  [x10]
        ldp             q4,  q5,  [x10, #(2*32)]
        ldp             q2,  q3,  [x11]
        ldp             q6,  q7,  [x11, #(2*32)]

        ldp             q8,  q9,  [x12]
        ldp             q10, q11, [x12, #(2*32)]
        ldp             q12, q13, [x13]
        ldp             q14, q15, [x13, #(2*32)]


        ld1             { v24.4s, v25.4s }, [x8], x9
        ld1             { v16.4s, v17.4s }, [x7], #32

        ext8             v18.16b, v24.16b, v24.16b, #8
        ext8             v19.16b, v25.16b, v25.16b, #8

        trn1            v20.4s, v16.4s, v16.4s      
        trn2            v16.4s, v16.4s, v16.4s      
        trn1            v21.4s, v17.4s, v17.4s      
        trn2            v17.4s, v17.4s, v17.4s      

        rev64           v22.4s, v8.4s                 
        rev64           v23.4s, v10.4s                 
        rev64           v24.4s, v12.4s                 
        rev64           v25.4s, v14.4s                 

        fmul            v22.4s, v22.4s, v20.4s      
        fmul            v23.4s, v23.4s, v16.4s      
        fmul            v24.4s, v24.4s, v20.4s      
        fmul            v25.4s, v25.4s, v16.4s      

        rev64           v26.4s, v9.4s                 
        rev64           v27.4s, v11.4s                 
        rev64           v20.4s, v13.4s                 
        rev64           v16.4s, v15.4s                 

        fmul            v26.4s, v26.4s, v21.4s      
        fmul            v27.4s, v27.4s, v17.4s      
        fmul            v20.4s, v20.4s, v21.4s      
        fmul            v16.4s, v16.4s, v17.4s      

        trn1            v21.4s, v19.4s, v19.4s      
        trn2            v19.4s, v19.4s, v19.4s      
        trn1            v17.4s, v18.4s, v18.4s      
        trn2            v18.4s, v18.4s, v18.4s      

        fmul            v21.4s, v21.4s, v31.4s
        fmul            v19.4s, v19.4s, v31.4s
        fmul            v17.4s, v17.4s, v31.4s
        fmul            v18.4s, v18.4s, v31.4s

        fmla            v23.4s, v10.4s, v21.4s      
        fmls            v25.4s, v14.4s, v21.4s      
        fmla            v22.4s, v8.4s, v19.4s      
        fmls            v24.4s, v12.4s, v19.4s      

        fmla            v26.4s, v9.4s, v18.4s      
        fmla            v27.4s, v11.4s, v17.4s      
        fmls            v20.4s, v13.4s, v18.4s      
        fmls            v16.4s, v15.4s, v17.4s      

        fsub            v18.4s, v23.4s, v25.4s
        fsub            v17.4s, v24.4s, v22.4s
        fsub            v19.4s, v20.4s, v26.4s
        fsub            v21.4s, v16.4s, v27.4s

        fadd            v22.4s, v24.4s, v22.4s
        fadd            v23.4s, v25.4s, v23.4s
        fadd            v24.4s, v20.4s, v26.4s
        fadd            v25.4s, v16.4s, v27.4s

        fmul            v17.4s, v17.4s, v31.4s
        fmul            v18.4s, v18.4s, v31.4s
        fmul            v19.4s, v19.4s, v31.4s
        fmul            v21.4s, v21.4s, v31.4s

        rev64           v22.4s, v22.4s
        rev64           v24.4s, v24.4s
        rev64           v23.4s, v23.4s
        rev64           v25.4s, v25.4s

        fsub            v8.4s, v0.4s, v22.4s
        fsub            v9.4s, v1.4s, v24.4s
        fsub            v10.4s, v2.4s, v17.4s
        fsub            v11.4s, v3.4s, v19.4s

        fsub            v12.4s, v4.4s, v23.4s
        fsub            v13.4s, v5.4s, v25.4s
        fadd            v14.4s, v6.4s, v18.4s
        fsub            v15.4s, v7.4s, v21.4s

        fadd            v0.4s, v0.4s, v22.4s
        fadd            v1.4s, v1.4s, v24.4s
        fadd            v2.4s, v2.4s, v17.4s
        fadd            v3.4s, v3.4s, v19.4s

        fadd            v4.4s, v4.4s, v23.4s
        fsub            v6.4s, v6.4s, v18.4s      
        fadd            v5.4s, v5.4s, v25.4s      
        fadd            v7.4s, v7.4s, v21.4s

        zip1            v16.2d, v0.2d, v4.2d
        zip2            v17.2d, v0.2d, v4.2d
        zip1            v18.2d, v1.2d, v5.2d
        zip2            v19.2d, v1.2d, v5.2d

        zip1            v20.2d, v2.2d, v6.2d
        zip2            v21.2d, v2.2d, v6.2d
        zip1            v22.2d, v3.2d, v7.2d
        zip2            v23.2d, v3.2d, v7.2d

        ldp             q0,  q1,  [x10, #(1*32)]
        ldp             q4,  q5,  [x10, #(3*32)]
        ldp             q2,  q3,  [x11, #(1*32)]
        ldp             q6,  q7,  [x11, #(3*32)]

        st1             { v16.4s, v17.4s, v18.4s, v19.4s }, [x10], #64
        st1             { v20.4s, v21.4s, v22.4s, v23.4s }, [x11], #64

        zip1            v20.2d, v8.2d, v12.2d
        zip2            v21.2d, v8.2d, v12.2d
        zip1            v22.2d, v9.2d, v13.2d
        zip2            v23.2d, v9.2d, v13.2d
        zip1            v24.2d, v10.2d, v14.2d
        zip2            v25.2d, v10.2d, v14.2d
        zip1            v26.2d, v11.2d, v15.2d
        zip2            v27.2d, v11.2d, v15.2d

        ldp             q8,  q9,  [x12, #(1*32)]
        ldp             q10, q11, [x12, #(3*32)]
        ldp             q12, q13, [x13, #(1*32)]
        ldp             q14, q15, [x13, #(3*32)]

        st1             { v20.4s, v21.4s, v22.4s, v23.4s }, [x12], #64
        st1             { v24.4s, v25.4s, v26.4s, v27.4s }, [x13], #64


        ld1             { v24.4s, v25.4s }, [x8], x9
        ld1             { v16.4s, v17.4s }, [x7], #32

        ext8             v18.16b, v24.16b, v24.16b, #8
        ext8             v19.16b, v25.16b, v25.16b, #8

        trn1            v20.4s, v16.4s, v16.4s      
        trn2            v16.4s, v16.4s, v16.4s      
        trn1            v21.4s, v17.4s, v17.4s      
        trn2            v17.4s, v17.4s, v17.4s      

        rev64           v22.4s, v8.4s                 
        rev64           v23.4s, v10.4s                 
        rev64           v24.4s, v12.4s                 
        rev64           v25.4s, v14.4s                 

        fmul            v22.4s, v22.4s, v20.4s      
        fmul            v23.4s, v23.4s, v16.4s      
        fmul            v24.4s, v24.4s, v20.4s      
        fmul            v25.4s, v25.4s, v16.4s      

        rev64           v26.4s, v9.4s                 
        rev64           v27.4s, v11.4s                 
        rev64           v20.4s, v13.4s                 
        rev64           v16.4s, v15.4s                 

        fmul            v26.4s, v26.4s, v21.4s      
        fmul            v27.4s, v27.4s, v17.4s      
        fmul            v20.4s, v20.4s, v21.4s      
        fmul            v16.4s, v16.4s, v17.4s      

        trn1            v21.4s, v19.4s, v19.4s      
        trn2            v19.4s, v19.4s, v19.4s      
        trn1            v17.4s, v18.4s, v18.4s      
        trn2            v18.4s, v18.4s, v18.4s      

        fmul            v21.4s, v21.4s, v31.4s
        fmul            v19.4s, v19.4s, v31.4s
        fmul            v17.4s, v17.4s, v31.4s
        fmul            v18.4s, v18.4s, v31.4s

        fmla            v23.4s, v10.4s, v21.4s      
        fmls            v25.4s, v14.4s, v21.4s      
        fmla            v22.4s, v8.4s, v19.4s      
        fmls            v24.4s, v12.4s, v19.4s      

        fmla            v26.4s, v9.4s, v18.4s      
        fmla            v27.4s, v11.4s, v17.4s      
        fmls            v20.4s, v13.4s, v18.4s      
        fmls            v16.4s, v15.4s, v17.4s      

        fsub            v18.4s, v23.4s, v25.4s
        fsub            v17.4s, v24.4s, v22.4s
        fsub            v19.4s, v20.4s, v26.4s
        fsub            v21.4s, v16.4s, v27.4s

        fadd            v22.4s, v24.4s, v22.4s
        fadd            v23.4s, v25.4s, v23.4s
        fadd            v24.4s, v20.4s, v26.4s
        fadd            v25.4s, v16.4s, v27.4s

        fmul            v17.4s, v17.4s, v31.4s
        fmul            v18.4s, v18.4s, v31.4s
        fmul            v19.4s, v19.4s, v31.4s
        fmul            v21.4s, v21.4s, v31.4s

        rev64           v22.4s, v22.4s
        rev64           v24.4s, v24.4s
        rev64           v23.4s, v23.4s
        rev64           v25.4s, v25.4s

        fsub            v8.4s, v0.4s, v22.4s
        fsub            v9.4s, v1.4s, v24.4s
        fsub            v10.4s, v2.4s, v17.4s
        fsub            v11.4s, v3.4s, v19.4s

        fsub            v12.4s, v4.4s, v23.4s
        fsub            v13.4s, v5.4s, v25.4s
        fadd            v14.4s, v6.4s, v18.4s
        fsub            v15.4s, v7.4s, v21.4s

        fadd            v0.4s, v0.4s, v22.4s
        fadd            v1.4s, v1.4s, v24.4s
        fadd            v2.4s, v2.4s, v17.4s
        fadd            v3.4s, v3.4s, v19.4s

        fadd            v4.4s, v4.4s, v23.4s
        fsub            v6.4s, v6.4s, v18.4s      
        fadd            v5.4s, v5.4s, v25.4s      
        fadd            v7.4s, v7.4s, v21.4s

        zip1            v16.2d, v0.2d, v4.2d
        zip2            v17.2d, v0.2d, v4.2d
        zip1            v18.2d, v1.2d, v5.2d
        zip2            v19.2d, v1.2d, v5.2d
        st1             { v16.4s, v17.4s, v18.4s, v19.4s }, [x10]

        zip1            v16.2d, v2.2d, v6.2d
        zip2            v17.2d, v2.2d, v6.2d
        zip1            v18.2d, v3.2d, v7.2d
        zip2            v19.2d, v3.2d, v7.2d
        st1             { v16.4s, v17.4s, v18.4s, v19.4s }, [x11]

        zip1            v20.2d, v8.2d, v12.2d
        zip2            v21.2d, v8.2d, v12.2d
        zip1            v22.2d, v9.2d, v13.2d
        zip2            v23.2d, v9.2d, v13.2d
        st1             { v20.4s, v21.4s, v22.4s, v23.4s }, [x12]

        zip1            v24.2d, v10.2d, v14.2d
        zip2            v25.2d, v10.2d, v14.2d
        zip1            v26.2d, v11.2d, v15.2d
        zip2            v27.2d, v11.2d, v15.2d
        st1             { v24.4s, v25.4s, v26.4s, v27.4s }, [x13]
        add             x10,  x1, #((4)*32 + 0)
        add             x11, x14, #((4)*32 + 0)
        add             x12, x15, #((4)*32 + 0)
        add             x13, x16, #((4)*32 + 0)

        ldp             q0,  q1,  [x10]
        ldp             q4,  q5,  [x10, #(2*32)]
        ldp             q2,  q3,  [x11]
        ldp             q6,  q7,  [x11, #(2*32)]

        ldp             q8,  q9,  [x12]
        ldp             q10, q11, [x12, #(2*32)]
        ldp             q12, q13, [x13]
        ldp             q14, q15, [x13, #(2*32)]


        ld1             { v24.4s, v25.4s }, [x8], x9
        ld1             { v16.4s, v17.4s }, [x7], #32

        ext8             v18.16b, v24.16b, v24.16b, #8
        ext8             v19.16b, v25.16b, v25.16b, #8

        trn1            v20.4s, v16.4s, v16.4s      
        trn2            v16.4s, v16.4s, v16.4s      
        trn1            v21.4s, v17.4s, v17.4s      
        trn2            v17.4s, v17.4s, v17.4s      

        rev64           v22.4s, v8.4s                 
        rev64           v23.4s, v10.4s                 
        rev64           v24.4s, v12.4s                 
        rev64           v25.4s, v14.4s                 

        fmul            v22.4s, v22.4s, v20.4s      
        fmul            v23.4s, v23.4s, v16.4s      
        fmul            v24.4s, v24.4s, v20.4s      
        fmul            v25.4s, v25.4s, v16.4s      

        rev64           v26.4s, v9.4s                 
        rev64           v27.4s, v11.4s                 
        rev64           v20.4s, v13.4s                 
        rev64           v16.4s, v15.4s                 

        fmul            v26.4s, v26.4s, v21.4s      
        fmul            v27.4s, v27.4s, v17.4s      
        fmul            v20.4s, v20.4s, v21.4s      
        fmul            v16.4s, v16.4s, v17.4s      

        trn1            v21.4s, v19.4s, v19.4s      
        trn2            v19.4s, v19.4s, v19.4s      
        trn1            v17.4s, v18.4s, v18.4s      
        trn2            v18.4s, v18.4s, v18.4s      

        fmul            v21.4s, v21.4s, v31.4s
        fmul            v19.4s, v19.4s, v31.4s
        fmul            v17.4s, v17.4s, v31.4s
        fmul            v18.4s, v18.4s, v31.4s

        fmla            v23.4s, v10.4s, v21.4s      
        fmls            v25.4s, v14.4s, v21.4s      
        fmla            v22.4s, v8.4s, v19.4s      
        fmls            v24.4s, v12.4s, v19.4s      

        fmla            v26.4s, v9.4s, v18.4s      
        fmla            v27.4s, v11.4s, v17.4s      
        fmls            v20.4s, v13.4s, v18.4s      
        fmls            v16.4s, v15.4s, v17.4s      

        fsub            v18.4s, v23.4s, v25.4s
        fsub            v17.4s, v24.4s, v22.4s
        fsub            v19.4s, v20.4s, v26.4s
        fsub            v21.4s, v16.4s, v27.4s

        fadd            v22.4s, v24.4s, v22.4s
        fadd            v23.4s, v25.4s, v23.4s
        fadd            v24.4s, v20.4s, v26.4s
        fadd            v25.4s, v16.4s, v27.4s

        fmul            v17.4s, v17.4s, v31.4s
        fmul            v18.4s, v18.4s, v31.4s
        fmul            v19.4s, v19.4s, v31.4s
        fmul            v21.4s, v21.4s, v31.4s

        rev64           v22.4s, v22.4s
        rev64           v24.4s, v24.4s
        rev64           v23.4s, v23.4s
        rev64           v25.4s, v25.4s

        fsub            v8.4s, v0.4s, v22.4s
        fsub            v9.4s, v1.4s, v24.4s
        fsub            v10.4s, v2.4s, v17.4s
        fsub            v11.4s, v3.4s, v19.4s

        fsub            v12.4s, v4.4s, v23.4s
        fsub            v13.4s, v5.4s, v25.4s
        fadd            v14.4s, v6.4s, v18.4s
        fsub            v15.4s, v7.4s, v21.4s

        fadd            v0.4s, v0.4s, v22.4s
        fadd            v1.4s, v1.4s, v24.4s
        fadd            v2.4s, v2.4s, v17.4s
        fadd            v3.4s, v3.4s, v19.4s

        fadd            v4.4s, v4.4s, v23.4s
        fsub            v6.4s, v6.4s, v18.4s      
        fadd            v5.4s, v5.4s, v25.4s      
        fadd            v7.4s, v7.4s, v21.4s

        zip1            v16.2d, v0.2d, v4.2d
        zip2            v17.2d, v0.2d, v4.2d
        zip1            v18.2d, v1.2d, v5.2d
        zip2            v19.2d, v1.2d, v5.2d

        zip1            v20.2d, v2.2d, v6.2d
        zip2            v21.2d, v2.2d, v6.2d
        zip1            v22.2d, v3.2d, v7.2d
        zip2            v23.2d, v3.2d, v7.2d

        ldp             q0,  q1,  [x10, #(1*32)]
        ldp             q4,  q5,  [x10, #(3*32)]
        ldp             q2,  q3,  [x11, #(1*32)]
        ldp             q6,  q7,  [x11, #(3*32)]

        st1             { v16.4s, v17.4s, v18.4s, v19.4s }, [x10], #64
        st1             { v20.4s, v21.4s, v22.4s, v23.4s }, [x11], #64

        zip1            v20.2d, v8.2d, v12.2d
        zip2            v21.2d, v8.2d, v12.2d
        zip1            v22.2d, v9.2d, v13.2d
        zip2            v23.2d, v9.2d, v13.2d
        zip1            v24.2d, v10.2d, v14.2d
        zip2            v25.2d, v10.2d, v14.2d
        zip1            v26.2d, v11.2d, v15.2d
        zip2            v27.2d, v11.2d, v15.2d

        ldp             q8,  q9,  [x12, #(1*32)]
        ldp             q10, q11, [x12, #(3*32)]
        ldp             q12, q13, [x13, #(1*32)]
        ldp             q14, q15, [x13, #(3*32)]

        st1             { v20.4s, v21.4s, v22.4s, v23.4s }, [x12], #64
        st1             { v24.4s, v25.4s, v26.4s, v27.4s }, [x13], #64


        ld1             { v24.4s, v25.4s }, [x8], x9
        ld1             { v16.4s, v17.4s }, [x7], #32

        ext8             v18.16b, v24.16b, v24.16b, #8
        ext8             v19.16b, v25.16b, v25.16b, #8

        trn1            v20.4s, v16.4s, v16.4s      
        trn2            v16.4s, v16.4s, v16.4s      
        trn1            v21.4s, v17.4s, v17.4s      
        trn2            v17.4s, v17.4s, v17.4s      

        rev64           v22.4s, v8.4s                 
        rev64           v23.4s, v10.4s                 
        rev64           v24.4s, v12.4s                 
        rev64           v25.4s, v14.4s                 

        fmul            v22.4s, v22.4s, v20.4s      
        fmul            v23.4s, v23.4s, v16.4s      
        fmul            v24.4s, v24.4s, v20.4s      
        fmul            v25.4s, v25.4s, v16.4s      

        rev64           v26.4s, v9.4s                 
        rev64           v27.4s, v11.4s                 
        rev64           v20.4s, v13.4s                 
        rev64           v16.4s, v15.4s                 

        fmul            v26.4s, v26.4s, v21.4s      
        fmul            v27.4s, v27.4s, v17.4s      
        fmul            v20.4s, v20.4s, v21.4s      
        fmul            v16.4s, v16.4s, v17.4s      

        trn1            v21.4s, v19.4s, v19.4s      
        trn2            v19.4s, v19.4s, v19.4s      
        trn1            v17.4s, v18.4s, v18.4s      
        trn2            v18.4s, v18.4s, v18.4s      

        fmul            v21.4s, v21.4s, v31.4s
        fmul            v19.4s, v19.4s, v31.4s
        fmul            v17.4s, v17.4s, v31.4s
        fmul            v18.4s, v18.4s, v31.4s

        fmla            v23.4s, v10.4s, v21.4s      
        fmls            v25.4s, v14.4s, v21.4s      
        fmla            v22.4s, v8.4s, v19.4s      
        fmls            v24.4s, v12.4s, v19.4s      

        fmla            v26.4s, v9.4s, v18.4s      
        fmla            v27.4s, v11.4s, v17.4s      
        fmls            v20.4s, v13.4s, v18.4s      
        fmls            v16.4s, v15.4s, v17.4s      

        fsub            v18.4s, v23.4s, v25.4s
        fsub            v17.4s, v24.4s, v22.4s
        fsub            v19.4s, v20.4s, v26.4s
        fsub            v21.4s, v16.4s, v27.4s

        fadd            v22.4s, v24.4s, v22.4s
        fadd            v23.4s, v25.4s, v23.4s
        fadd            v24.4s, v20.4s, v26.4s
        fadd            v25.4s, v16.4s, v27.4s

        fmul            v17.4s, v17.4s, v31.4s
        fmul            v18.4s, v18.4s, v31.4s
        fmul            v19.4s, v19.4s, v31.4s
        fmul            v21.4s, v21.4s, v31.4s

        rev64           v22.4s, v22.4s
        rev64           v24.4s, v24.4s
        rev64           v23.4s, v23.4s
        rev64           v25.4s, v25.4s

        fsub            v8.4s, v0.4s, v22.4s
        fsub            v9.4s, v1.4s, v24.4s
        fsub            v10.4s, v2.4s, v17.4s
        fsub            v11.4s, v3.4s, v19.4s

        fsub            v12.4s, v4.4s, v23.4s
        fsub            v13.4s, v5.4s, v25.4s
        fadd            v14.4s, v6.4s, v18.4s
        fsub            v15.4s, v7.4s, v21.4s

        fadd            v0.4s, v0.4s, v22.4s
        fadd            v1.4s, v1.4s, v24.4s
        fadd            v2.4s, v2.4s, v17.4s
        fadd            v3.4s, v3.4s, v19.4s

        fadd            v4.4s, v4.4s, v23.4s
        fsub            v6.4s, v6.4s, v18.4s      
        fadd            v5.4s, v5.4s, v25.4s      
        fadd            v7.4s, v7.4s, v21.4s

        zip1            v16.2d, v0.2d, v4.2d
        zip2            v17.2d, v0.2d, v4.2d
        zip1            v18.2d, v1.2d, v5.2d
        zip2            v19.2d, v1.2d, v5.2d
        st1             { v16.4s, v17.4s, v18.4s, v19.4s }, [x10]

        zip1            v16.2d, v2.2d, v6.2d
        zip2            v17.2d, v2.2d, v6.2d
        zip1            v18.2d, v3.2d, v7.2d
        zip2            v19.2d, v3.2d, v7.2d
        st1             { v16.4s, v17.4s, v18.4s, v19.4s }, [x11]

        zip1            v20.2d, v8.2d, v12.2d
        zip2            v21.2d, v8.2d, v12.2d
        zip1            v22.2d, v9.2d, v13.2d
        zip2            v23.2d, v9.2d, v13.2d
        st1             { v20.4s, v21.4s, v22.4s, v23.4s }, [x12]

        zip1            v24.2d, v10.2d, v14.2d
        zip2            v25.2d, v10.2d, v14.2d
        zip1            v26.2d, v11.2d, v15.2d
        zip2            v27.2d, v11.2d, v15.2d
        st1             { v24.4s, v25.4s, v26.4s, v27.4s }, [x13]
        add             x1, x1, #32*8
        subs            w19, w19, #32*4
        bgt            temp_label_90

        ldp             x19, x20, [sp, #16]
        ldp             d14, d15, [sp, #16*2]
        ldp             d12, d13, [sp, #16*3]
        ldp             d10, d11, [sp, #16*4]
        ldp             d8,  d9,  [sp, #16*5]
        ldp             x21, x22, [sp], #16*6

        ret

temp_label_91
temp_label_49
 
        umov             x10, v23.d[0]
        umov             x11, v23.d[1]


        rev64           v28.4s, v2.4s              
        rev64           v29.4s, v16.4s              
        trn2            v30.4s, v27.4s, v27.4s   
        fmul            v30.4s, v30.4s, v31.4s
        fmul            v16.4s, v16.4s, v30.4s
        fmul            v2.4s, v2.4s, v30.4s   
        trn1            v30.4s, v24.4s, v24.4s   
        fmul            v29.4s, v29.4s, v30.4s   
        fmla            v2.4s, v28.4s, v30.4s   
        fsub            v29.4s, v29.4s, v16.4s   

        rev64           v30.4s, v3.4s              
        rev64           v16.4s, v17.4s              

        trn2            v28.4s, v26.4s, v26.4s   
        fmul            v28.4s, v28.4s, v31.4s

        fmul            v3.4s, v3.4s, v28.4s   
        fmul            v17.4s, v17.4s, v28.4s

        trn1            v28.4s, v25.4s, v25.4s   
        fmul            v16.4s, v16.4s, v28.4s   
        fmla            v3.4s, v30.4s, v28.4s   
        fsub            v16.4s, v16.4s, v17.4s   

        fsub            v28.4s, v29.4s, v2.4s
        fadd            v2.4s, v29.4s, v2.4s
        fadd            v30.4s, v16.4s, v3.4s
        fsub            v29.4s, v16.4s, v3.4s

        fmul            v28.4s, v28.4s, v31.4s
        fmul            v29.4s, v29.4s, v31.4s

        rev64           v30.4s, v30.4s
        rev64           v2.4s, v2.4s

        fsub            v3.4s, v1.4s, v30.4s
        fsub            v16.4s, v8.4s, v28.4s
        fsub            v17.4s, v9.4s, v29.4s

        fadd            v1.4s, v1.4s, v30.4s
        fadd            v8.4s, v8.4s, v28.4s
        fadd            v9.4s, v9.4s, v29.4s

        orr v29.16b, v2.16b, v2.16b

        fsub            v2.4s, v0.4s, v29.4s
        fadd            v0.4s, v0.4s, v29.4s


        trn2            v24.4s, v24.4s, v24.4s   
        trn2            v25.4s, v25.4s, v25.4s   
        trn1            v26.4s, v26.4s, v26.4s   
        trn1            v27.4s, v27.4s, v27.4s   

        fmul            v26.4s, v26.4s, v31.4s
        fmul            v27.4s, v27.4s, v31.4s

        rev64           v28.4s, v6.4s              
        rev64           v29.4s, v20.4s              
        rev64           v30.4s, v7.4s              
        rev64           v23.4s, v21.4s              

        fmul            v6.4s, v6.4s, v27.4s   
        fmul            v7.4s, v7.4s, v26.4s   
        fmla            v6.4s, v28.4s, v24.4s   
        fmla            v7.4s, v30.4s, v25.4s   

        fmul            v29.4s, v29.4s, v24.4s   
        fmul            v23.4s, v23.4s, v25.4s   
        fmls            v29.4s, v20.4s, v27.4s   
        fmls            v23.4s, v21.4s, v26.4s   

        fsub            v28.4s, v29.4s, v6.4s
        fadd            v29.4s, v29.4s, v6.4s
        fadd            v30.4s, v23.4s, v7.4s
        fsub            v23.4s, v23.4s, v7.4s

        fmul            v28.4s, v28.4s, v31.4s
        fmul            v23.4s, v23.4s, v31.4s

        rev64           v29.4s, v29.4s
        rev64           v30.4s, v30.4s

        fsub            v6.4s, v4.4s, v29.4s
        fadd            v20.4s, v5.4s, v30.4s
        fsub            v7.4s, v12.4s, v28.4s
        fadd            v21.4s, v13.4s, v23.4s

        fadd            v4.4s, v4.4s, v29.4s
        fsub            v5.4s, v5.4s, v30.4s   
        fadd            v12.4s, v12.4s, v28.4s   
        fsub            v13.4s, v13.4s, v23.4s

        zip1            v23.2d, v0.2d, v4.2d
        zip2            v24.2d, v0.2d, v4.2d
        zip1            v25.2d, v1.2d, v20.2d
        zip2            v26.2d, v1.2d, v20.2d

        zip1            v27.2d, v8.2d, v12.2d
        zip2            v28.2d, v8.2d, v12.2d
        zip1            v29.2d, v9.2d, v21.2d
        zip2            v30.2d, v9.2d, v21.2d

        orr v20.16b, v5.16b, v5.16b
        orr v21.16b, v7.16b, v7.16b
        mov             x12, x1
        add             x13, x1, #32* 4
        add             x14, x1, #32* 8
        add             x15, x1, #32*12

        zip1            v4.2d,  v2.2d, v6.2d
        zip2            v5.2d,  v2.2d, v6.2d
        zip1            v6.2d,  v3.2d, v20.2d
        zip2            v7.2d,  v3.2d, v20.2d

        zip1            v0.2d, v16.2d, v21.2d
        zip2            v1.2d, v16.2d, v21.2d
        zip1            v2.2d, v17.2d, v13.2d
        zip2            v3.2d, v17.2d, v13.2d

        
        ldp             q8,  q9, [x1, #32*1]
        ldp             q12, q13, [x1, #32*5]

        st1             { v23.4s, v24.4s, v25.4s, v26.4s }, [x12], #64  
        st1             { v27.4s, v28.4s, v29.4s, v30.4s }, [x13], #64  
        st1             {  v4.4s,  v5.4s,  v6.4s,  v7.4s }, [x14], #64  
        st1             {  v0.4s,  v1.4s,  v2.4s,  v3.4s }, [x15], #64  

        mov             v23.d[0], x10
        mov             v23.d[1], x11

        ldp             q6,  q7, [x1, #32*3]
        ldp             q16, q17, [x1, #32*7]


        ld1             { v0.4s, v1.4s }, [x8], x9
        ld1             { v24.4s, v25.4s }, [x7], #32

        ext8             v26.16b, v0.16b, v0.16b, #8
        ext8             v27.16b, v1.16b, v1.16b, #8

        trn1            v28.4s, v24.4s, v24.4s      
        trn2            v24.4s, v24.4s, v24.4s      
        trn1            v29.4s, v25.4s, v25.4s      
        trn2            v25.4s, v25.4s, v25.4s      

        rev64           v30.4s, v10.4s                 
        rev64           v4.4s, v14.4s                 
        rev64           v0.4s, v18.4s                 
        rev64           v1.4s, v22.4s                 

        fmul            v30.4s, v30.4s, v28.4s      
        fmul            v4.4s, v4.4s, v24.4s      
        fmul            v0.4s, v0.4s, v28.4s      
        fmul            v1.4s, v1.4s, v24.4s      

        rev64           v5.4s, v11.4s                 
        rev64           v20.4s, v15.4s                 
        rev64           v28.4s, v19.4s                 
        rev64           v24.4s, v23.4s                 

        fmul            v5.4s, v5.4s, v29.4s      
        fmul            v20.4s, v20.4s, v25.4s      
        fmul            v28.4s, v28.4s, v29.4s      
        fmul            v24.4s, v24.4s, v25.4s      

        trn1            v29.4s, v27.4s, v27.4s      
        trn2            v27.4s, v27.4s, v27.4s      
        trn1            v25.4s, v26.4s, v26.4s      
        trn2            v26.4s, v26.4s, v26.4s      

        fmul            v29.4s, v29.4s, v31.4s
        fmul            v27.4s, v27.4s, v31.4s
        fmul            v25.4s, v25.4s, v31.4s
        fmul            v26.4s, v26.4s, v31.4s

        fmla            v4.4s, v14.4s, v29.4s      
        fmls            v1.4s, v22.4s, v29.4s      
        fmla            v30.4s, v10.4s, v27.4s      
        fmls            v0.4s, v18.4s, v27.4s      

        fmla            v5.4s, v11.4s, v26.4s      
        fmla            v20.4s, v15.4s, v25.4s      
        fmls            v28.4s, v19.4s, v26.4s      
        fmls            v24.4s, v23.4s, v25.4s      

        fsub            v26.4s, v4.4s, v1.4s
        fsub            v25.4s, v0.4s, v30.4s
        fsub            v27.4s, v28.4s, v5.4s
        fsub            v29.4s, v24.4s, v20.4s

        fadd            v30.4s, v0.4s, v30.4s
        fadd            v4.4s, v1.4s, v4.4s
        fadd            v0.4s, v28.4s, v5.4s
        fadd            v1.4s, v24.4s, v20.4s

        fmul            v25.4s, v25.4s, v31.4s
        fmul            v26.4s, v26.4s, v31.4s
        fmul            v27.4s, v27.4s, v31.4s
        fmul            v29.4s, v29.4s, v31.4s

        rev64           v30.4s, v30.4s
        rev64           v0.4s, v0.4s
        rev64           v4.4s, v4.4s
        rev64           v1.4s, v1.4s

        fsub            v10.4s, v8.4s, v30.4s
        fsub            v11.4s, v9.4s, v0.4s
        fsub            v14.4s, v12.4s, v25.4s
        fsub            v15.4s, v13.4s, v27.4s

        fsub            v18.4s, v6.4s, v4.4s
        fsub            v19.4s, v7.4s, v1.4s
        fadd            v22.4s, v16.4s, v26.4s
        fsub            v23.4s, v17.4s, v29.4s

        fadd            v8.4s, v8.4s, v30.4s
        fadd            v9.4s, v9.4s, v0.4s
        fadd            v12.4s, v12.4s, v25.4s
        fadd            v13.4s, v13.4s, v27.4s

        fadd            v6.4s, v6.4s, v4.4s
        fsub            v16.4s, v16.4s, v26.4s      
        fadd            v7.4s, v7.4s, v1.4s      
        fadd            v17.4s, v17.4s, v29.4s

        zip1            v0.2d, v8.2d, v6.2d
        zip2            v1.2d, v8.2d, v6.2d
        zip1            v2.2d, v9.2d, v7.2d
        zip2            v3.2d, v9.2d, v7.2d
        st1             {  v0.4s,  v1.4s,  v2.4s,  v3.4s }, [x12]

        zip1            v4.2d, v12.2d, v16.2d
        zip2            v5.2d, v12.2d, v16.2d
        zip1            v6.2d, v13.2d, v17.2d
        zip2            v7.2d, v13.2d, v17.2d
        st1             {  v4.4s,  v5.4s,  v6.4s,  v7.4s }, [x13]

        zip1            v0.2d, v10.2d, v18.2d
        zip2            v1.2d, v10.2d, v18.2d
        zip1            v2.2d, v11.2d, v19.2d
        zip2            v3.2d, v11.2d, v19.2d
        st1             {  v0.4s,  v1.4s,  v2.4s,  v3.4s }, [x14]

        zip1            v4.2d, v14.2d, v22.2d
        zip2            v5.2d, v14.2d, v22.2d
        zip1            v6.2d, v15.2d, v23.2d
        zip2            v7.2d, v15.2d, v23.2d
        st1             {  v4.4s,  v5.4s,  v6.4s,  v7.4s }, [x15]

        ldp             x19, x20, [sp, #16]
        ldp             d14, d15, [sp, #16*2]
        ldp             d12, d13, [sp, #16*3]
        ldp             d10, d11, [sp, #16*4]
        ldp             d8,  d9,  [sp, #16*5]
        ldp             x21, x22, [sp], #16*6

        ret



	IMPORT ff_tx_tab_32768_float
	IMPORT ff_tx_tab_64_float
	IMPORT ff_tx_tab_128_float
	IMPORT ff_tx_tab_256_float
	IMPORT ff_tx_tab_1024_float
	IMPORT ff_tx_tab_4096_float
	IMPORT ff_tx_tab_131072_float
	IMPORT ff_tx_tab_32_float
	IMPORT ff_tx_tab_65536_float
	IMPORT ff_tx_tab_8192_float
	IMPORT ff_tx_tab_512_float
	IMPORT ff_tx_tab_2048_float
	IMPORT ff_tx_tab_16384_float
	END
