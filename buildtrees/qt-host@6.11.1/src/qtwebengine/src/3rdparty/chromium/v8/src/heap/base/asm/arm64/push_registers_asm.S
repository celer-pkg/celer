	AREA |.text|, CODE, READONLY, ALIGN=4, CODEALIGN

	EXPORT PushAllRegistersAndIterateStack
	ALIGN 4
PushAllRegistersAndIterateStack




    
	stp x19, x20, [sp, #-16]!
	stp x21, x22, [sp, #-16]!
	stp x23, x24, [sp, #-16]!
	stp x25, x26, [sp, #-16]!
	stp x27, x28, [sp, #-16]!
	stp fp, lr,   [sp, #-16]!
    
	mov fp, sp
    
    
    
	mov x7, x2
    
	mov x2, sp
	blr x7
    
	ldp fp, lr, [sp], #16
    
	add sp, sp, #80




	ret

	END
