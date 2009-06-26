// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.

// long double __floatundixf(du_int a);

#ifdef __x86_64__

.const
.align 4
twop64: .quad 0x43f0000000000000

#define REL_ADDR(_a)	(_a)(%rip)

.text
.align 4
.globl ___floatundixf
___floatundixf:
	movq	%rdi,	 -8(%rsp)
	fildq	-8(%rsp)
	test	%rdi,		%rdi
	js		1f
	ret
1:	faddl	REL_ADDR(twop64)
	ret

#endif // __x86_64__


/* Branch-free implementation is ever so slightly slower, but more beautiful.
   It is likely superior for inlining, so I kept it around for future reference.

#ifdef __x86_64__

.const
.align 4
twop52: .quad 0x4330000000000000
twop84_plus_twop52_neg:
		.quad 0xc530000000100000
twop84: .quad 0x4530000000000000

#define REL_ADDR(_a)	(_a)(%rip)

.text
.align 4
.globl ___floatundixf
___floatundixf:
	movl	%edi,				%esi			// low 32 bits of input
	shrq	$32,				%rdi			// hi 32 bits of input
	orq		REL_ADDR(twop84),	%rdi			// 2^84 + hi (as a double)
	orq		REL_ADDR(twop52),	%rsi			// 2^52 + lo (as a double)
	movq	%rdi,			 -8(%rsp)
	movq	%rsi,			-16(%rsp)
	fldl	REL_ADDR(twop84_plus_twop52_neg)	
	faddl	-8(%rsp)	// hi - 2^52 (as double extended, no rounding occurs)
	faddl	-16(%rsp)	// hi + lo (as double extended)
	ret

#endif // __x86_64__

*/