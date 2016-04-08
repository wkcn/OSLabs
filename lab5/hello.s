	.file	"hello.cpp"
	.globl	NEWLINE
	.section	.rodata
.LC0:
	.string	"\r\n"
	.data
	.align 8
	.type	NEWLINE, @object
	.size	NEWLINE, 8
NEWLINE:
	.quad	.LC0
	.text
	.globl	_Z7getcharv
	.type	_Z7getcharv, @function
_Z7getcharv:
.LFB5:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	movl	$4096, %eax
#APP
# 12 "include/io.h" 1
	int 0x16;
# 0 "" 2
#NO_APP
	movb	%al, -1(%rbp)
	movzbl	-1(%rbp), %eax
	popq	%rbp
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE5:
	.size	_Z7getcharv, .-_Z7getcharv
	.globl	_Z9GetCursorv
	.type	_Z9GetCursorv, @function
_Z9GetCursorv:
.LFB6:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	pushq	%rbx
	.cfi_offset 3, -24
	movl	$768, %eax
	movl	$0, %edx
	movl	%edx, %ebx
#APP
# 23 "include/io.h" 1
	int 0x10;
# 0 "" 2
#NO_APP
	movl	%edx, %eax
	movw	%ax, -10(%rbp)
	movzwl	-10(%rbp), %eax
	popq	%rbx
	popq	%rbp
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE6:
	.size	_Z9GetCursorv, .-_Z9GetCursorv
	.globl	_Z9SetCursortt
	.type	_Z9SetCursortt, @function
_Z9SetCursortt:
.LFB7:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	pushq	%rbx
	.cfi_offset 3, -24
	movl	%edi, %edx
	movl	%esi, %eax
	movw	%dx, -12(%rbp)
	movw	%ax, -16(%rbp)
	movzwl	-12(%rbp), %eax
	sall	$8, %eax
	movl	%eax, %edx
	movzwl	-16(%rbp), %eax
	orl	%eax, %edx
	movl	$512, %eax
	movl	$0, %ecx
	movl	%ecx, %ebx
#APP
# 32 "include/io.h" 1
	int 0x10;
# 0 "" 2
#NO_APP
	nop
	popq	%rbx
	popq	%rbp
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE7:
	.size	_Z9SetCursortt, .-_Z9SetCursortt
	.globl	_Z3clsv
	.type	_Z3clsv, @function
_Z3clsv:
.LFB8:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	movl	$3, %eax
#APP
# 49 "include/io.h" 1
	int 0x10
# 0 "" 2
#NO_APP
	nop
	popq	%rbp
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE8:
	.size	_Z3clsv, .-_Z3clsv
	.globl	_Z8DrawCharcttt
	.type	_Z8DrawCharcttt, @function
_Z8DrawCharcttt:
.LFB9:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	pushq	%rbx
	.cfi_offset 3, -24
	movl	%ecx, %eax
	movb	%dil, -28(%rbp)
	movw	%si, -32(%rbp)
	movw	%dx, -36(%rbp)
	movw	%ax, -40(%rbp)
	movzwl	-32(%rbp), %edx
	movl	%edx, %eax
	sall	$2, %eax
	addl	%edx, %eax
	sall	$4, %eax
	movl	%eax, %edx
	movzwl	-36(%rbp), %eax
	addl	%edx, %eax
	addl	%eax, %eax
	movw	%ax, -10(%rbp)
	movzwl	-40(%rbp), %eax
	sall	$8, %eax
	movl	%eax, %edx
	movsbl	-28(%rbp), %eax
	movl	%edx, %ecx
	orl	%eax, %ecx
	movl	$47104, %eax
	movzwl	-10(%rbp), %edx
	movl	%edx, %ebx
#APP
# 66 "include/io.h" 1
	push es;mov es, ax;mov es:[bx],cx;pop es;
# 0 "" 2
#NO_APP
	nop
	popq	%rbx
	popq	%rbp
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE9:
	.size	_Z8DrawCharcttt, .-_Z8DrawCharcttt
	.globl	_Z8DrawTextPKcttt
	.type	_Z8DrawTextPKcttt, @function
_Z8DrawTextPKcttt:
.LFB10:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$24, %rsp
	movq	%rdi, -8(%rbp)
	movl	%ecx, %eax
	movw	%si, -12(%rbp)
	movw	%dx, -16(%rbp)
	movw	%ax, -20(%rbp)
.L10:
	movq	-8(%rbp), %rax
	movzbl	(%rax), %eax
	testb	%al, %al
	je	.L11
	movzwl	-20(%rbp), %ecx
	movzwl	-16(%rbp), %eax
	leal	1(%rax), %edx
	movw	%dx, -16(%rbp)
	movzwl	%ax, %edx
	movzwl	-12(%rbp), %esi
	movq	-8(%rbp), %rax
	leaq	1(%rax), %rdi
	movq	%rdi, -8(%rbp)
	movzbl	(%rax), %eax
	movsbl	%al, %eax
	movl	%eax, %edi
	call	_Z8DrawCharcttt
	jmp	.L10
.L11:
	nop
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE10:
	.size	_Z8DrawTextPKcttt, .-_Z8DrawTextPKcttt
	.globl	_Z9PrintCharct
	.type	_Z9PrintCharct, @function
_Z9PrintCharct:
.LFB11:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	pushq	%rbx
	subq	$24, %rsp
	.cfi_offset 3, -24
	movl	%edi, %edx
	movl	%esi, %eax
	movb	%dl, -28(%rbp)
	movw	%ax, -32(%rbp)
	call	_Z9GetCursorv
	movw	%ax, -10(%rbp)
	movzwl	-10(%rbp), %eax
	sarl	$8, %eax
	movw	%ax, -12(%rbp)
	movzwl	-10(%rbp), %eax
	movzbl	%al, %eax
	movw	%ax, -14(%rbp)
	movsbl	-28(%rbp), %eax
	orb	$14, %ah
	movzwl	-32(%rbp), %edx
	movl	%edx, %ebx
#APP
# 85 "include/io.h" 1
	int 0x10;
# 0 "" 2
#NO_APP
	cmpb	$10, -28(%rbp)
	je	.L14
	cmpb	$8, -28(%rbp)
	je	.L14
	cmpb	$13, -28(%rbp)
	je	.L14
	call	_Z9GetCursorv
	movw	%ax, -16(%rbp)
	movzwl	-16(%rbp), %eax
	sarl	$8, %eax
	movw	%ax, -18(%rbp)
	movzwl	-16(%rbp), %eax
	movzbl	%al, %eax
	movw	%ax, -20(%rbp)
	movzwl	-32(%rbp), %ecx
	movzwl	-14(%rbp), %edx
	movzwl	-12(%rbp), %esi
	movsbl	-28(%rbp), %eax
	movl	%eax, %edi
	call	_Z8DrawCharcttt
	movzwl	-20(%rbp), %edx
	movzwl	-18(%rbp), %eax
	movl	%edx, %esi
	movl	%eax, %edi
	call	_Z9SetCursortt
.L14:
	nop
	addq	$24, %rsp
	popq	%rbx
	popq	%rbp
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE11:
	.size	_Z9PrintCharct, .-_Z9PrintCharct
	.globl	_Z8PrintStrPKct
	.type	_Z8PrintStrPKct, @function
_Z8PrintStrPKct:
.LFB12:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$16, %rsp
	movq	%rdi, -8(%rbp)
	movl	%esi, %eax
	movw	%ax, -12(%rbp)
.L17:
	movq	-8(%rbp), %rax
	movzbl	(%rax), %eax
	testb	%al, %al
	je	.L18
	movzwl	-12(%rbp), %edx
	movq	-8(%rbp), %rax
	movzbl	(%rax), %eax
	movsbl	%al, %eax
	movl	%edx, %esi
	movl	%eax, %edi
	call	_Z9PrintCharct
	addq	$1, -8(%rbp)
	jmp	.L17
.L18:
	nop
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE12:
	.size	_Z8PrintStrPKct, .-_Z8PrintStrPKct
	.globl	_Z9PrintStrNPKctt
	.type	_Z9PrintStrNPKctt, @function
_Z9PrintStrNPKctt:
.LFB13:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$32, %rsp
	movq	%rdi, -24(%rbp)
	movl	%esi, %ecx
	movl	%edx, %eax
	movw	%cx, -28(%rbp)
	movw	%ax, -32(%rbp)
	movw	$0, -2(%rbp)
.L21:
	movzwl	-2(%rbp), %eax
	cmpw	-28(%rbp), %ax
	jnb	.L22
	movzwl	-32(%rbp), %edx
	movq	-24(%rbp), %rax
	movzbl	(%rax), %eax
	movsbl	%al, %eax
	movl	%edx, %esi
	movl	%eax, %edi
	call	_Z9PrintCharct
	addq	$1, -24(%rbp)
	addw	$1, -2(%rbp)
	jmp	.L21
.L22:
	nop
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE13:
	.size	_Z9PrintStrNPKctt, .-_Z9PrintStrNPKctt
	.globl	_Z8PrintNumtt
	.type	_Z8PrintNumtt, @function
_Z8PrintNumtt:
.LFB14:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$40, %rsp
	movl	%edi, %edx
	movl	%esi, %eax
	movw	%dx, -36(%rbp)
	movw	%ax, -40(%rbp)
	movl	$0, -4(%rbp)
.L25:
	movzwl	-36(%rbp), %ecx
	movzwl	%cx, %eax
	imull	$52429, %eax, %eax
	shrl	$16, %eax
	movl	%eax, %edx
	shrw	$3, %dx
	movl	%edx, %eax
	sall	$2, %eax
	addl	%edx, %eax
	addl	%eax, %eax
	subl	%eax, %ecx
	movl	%ecx, %edx
	movl	-4(%rbp), %eax
	cltq
	movb	%dl, -32(%rbp,%rax)
	movzwl	-36(%rbp), %eax
	movzwl	%ax, %eax
	imull	$52429, %eax, %eax
	shrl	$16, %eax
	shrw	$3, %ax
	movw	%ax, -36(%rbp)
	addl	$1, -4(%rbp)
	cmpw	$0, -36(%rbp)
	je	.L24
	jmp	.L25
.L24:
	movl	-4(%rbp), %eax
	subl	$1, %eax
	movl	%eax, -8(%rbp)
.L27:
	cmpl	$0, -8(%rbp)
	js	.L28
	movzwl	-40(%rbp), %edx
	movl	-8(%rbp), %eax
	cltq
	movzbl	-32(%rbp,%rax), %eax
	addl	$48, %eax
	movsbl	%al, %eax
	movl	%edx, %esi
	movl	%eax, %edi
	call	_Z9PrintCharct
	subl	$1, -8(%rbp)
	jmp	.L27
.L28:
	nop
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE14:
	.size	_Z8PrintNumtt, .-_Z8PrintNumtt
	.globl	_Z8PrintHexct
	.type	_Z8PrintHexct, @function
_Z8PrintHexct:
.LFB15:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$24, %rsp
	movl	%edi, %edx
	movl	%esi, %eax
	movb	%dl, -20(%rbp)
	movw	%ax, -24(%rbp)
	movb	$48, -16(%rbp)
	movb	$49, -15(%rbp)
	movb	$50, -14(%rbp)
	movb	$51, -13(%rbp)
	movb	$52, -12(%rbp)
	movb	$53, -11(%rbp)
	movb	$54, -10(%rbp)
	movb	$55, -9(%rbp)
	movb	$56, -8(%rbp)
	movb	$57, -7(%rbp)
	movb	$65, -6(%rbp)
	movb	$66, -5(%rbp)
	movb	$67, -4(%rbp)
	movb	$68, -3(%rbp)
	movb	$69, -2(%rbp)
	movb	$70, -1(%rbp)
	movzwl	-24(%rbp), %edx
	movsbl	-20(%rbp), %eax
	sarl	$4, %eax
	andl	$15, %eax
	cltq
	movzbl	-16(%rbp,%rax), %eax
	movsbl	%al, %eax
	movl	%edx, %esi
	movl	%eax, %edi
	call	_Z9PrintCharct
	movzwl	-24(%rbp), %edx
	movsbl	-20(%rbp), %eax
	andl	$15, %eax
	cltq
	movzbl	-16(%rbp,%rax), %eax
	movsbl	%al, %eax
	movl	%edx, %esi
	movl	%eax, %edi
	call	_Z9PrintCharct
	nop
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE15:
	.size	_Z8PrintHexct, .-_Z8PrintHexct
	.globl	cout
	.bss
	.align 2
	.type	cout, @object
	.size	cout, 2
cout:
	.zero	2
	.text
	.globl	_ZlsR7ostreami
	.type	_ZlsR7ostreami, @function
_ZlsR7ostreami:
.LFB16:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$16, %rsp
	movq	%rdi, -8(%rbp)
	movl	%esi, -12(%rbp)
	movq	-8(%rbp), %rax
	movzwl	(%rax), %eax
	movzwl	%ax, %edx
	movl	-12(%rbp), %eax
	movzwl	%ax, %eax
	movl	%edx, %esi
	movl	%eax, %edi
	call	_Z8PrintNumtt
	movq	-8(%rbp), %rax
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE16:
	.size	_ZlsR7ostreami, .-_ZlsR7ostreami
	.globl	_ZlsR7ostreamPKc
	.type	_ZlsR7ostreamPKc, @function
_ZlsR7ostreamPKc:
.LFB17:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$16, %rsp
	movq	%rdi, -8(%rbp)
	movq	%rsi, -16(%rbp)
	movq	-8(%rbp), %rax
	movzwl	(%rax), %eax
	movzwl	%ax, %edx
	movq	-16(%rbp), %rax
	movl	%edx, %esi
	movq	%rax, %rdi
	call	_Z8PrintStrPKct
	movq	-8(%rbp), %rax
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE17:
	.size	_ZlsR7ostreamPKc, .-_ZlsR7ostreamPKc
	.section	.rodata
.LC1:
	.string	"Hello, Mirai OS!"
	.text
	.globl	main
	.type	main, @function
main:
.LFB18:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	movl	$7, %esi
	movl	$.LC1, %edi
	call	_Z8PrintStrPKct
	movq	NEWLINE(%rip), %rax
	movl	$7, %esi
	movq	%rax, %rdi
	call	_Z8PrintStrPKct
	movl	$0, %eax
	popq	%rbp
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE18:
	.size	main, .-main
	.ident	"GCC: (GNU) 5.3.0"
	.section	.note.GNU-stack,"",@progbits
