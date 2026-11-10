// setjmp() / longjmp() for the PlayStation 3 PPU (64-bit PowerPC, ELFv1).
//
// The PSL1GHT newlib ships the 32-bit PowerPC version: it saves r1, r2 and
// r13-r31 with stw and restores them with lwz, so every callee-saved
// register comes back with its upper 32 bits cleared. Pointers survive
// (PS3 addresses fit in 32 bits), but any other 64-bit value does not: a
// 64-bit constant (the multiplier of a division by 1000, kept in r25 across
// a loop), a negative int held sign-extended... libpit's cooperative
// threads (threadsc.c) switch tasks with setjmp/longjmp, so the truncation
// hit whatever a task kept in those registers (the SDL event wait never
// timed out: the pointer moved in jumps). ErrTry/ErrCatch, gnuboy and the
// m68k emulator use setjmp too. These versions, linked from libpit before
// the C library, replace newlib's for the whole program.
//
// jmp_buf is 512 bytes (64 doubles with AltiVec), 8-byte aligned, which is
// exactly enough for everything callee-saved: r1, r2, r14-r31 and LR (8
// bytes each), CR, f14-f31 and v20-v31 (16 bytes each, 16-byte aligned).
// (r13 is the thread pointer, constant within a thread; VRSAVE is not used
// on the PPU.) Layout, b = the buffer:
//
//   b & 8 == 0:  v20-v31 at b+0..191,   CR at b+192
//   b & 8 == 8:  CR at b+0,             v20-v31 at b+8..199
//   always:      r1, r2, r14-r31 at b+200, LR at b+360, f14-f31 at b+368..511

	.machine cell

	.section ".text"
	.align 2
	.globl setjmp
	.section ".opd","aw"
	.align 3
setjmp:
	.quad .L.setjmp,.TOC.@tocbase,0
	.previous
	.type setjmp, @function
.L.setjmp:
	andi.	%r4,%r3,8		// 0 or 8
	add	%r5,%r3,%r4		// the vector block (16-byte aligned)
	mulli	%r6,%r4,24
	subfic	%r6,%r6,192
	add	%r6,%r3,%r6		// the CR slot: b+192 or b+0
	mfcr	%r0
	std	%r0,0(%r6)
	li	%r7,0
	stvx	%v20,%r5,%r7
	li	%r7,16
	stvx	%v21,%r5,%r7
	li	%r7,32
	stvx	%v22,%r5,%r7
	li	%r7,48
	stvx	%v23,%r5,%r7
	li	%r7,64
	stvx	%v24,%r5,%r7
	li	%r7,80
	stvx	%v25,%r5,%r7
	li	%r7,96
	stvx	%v26,%r5,%r7
	li	%r7,112
	stvx	%v27,%r5,%r7
	li	%r7,128
	stvx	%v28,%r5,%r7
	li	%r7,144
	stvx	%v29,%r5,%r7
	li	%r7,160
	stvx	%v30,%r5,%r7
	li	%r7,176
	stvx	%v31,%r5,%r7
	addi	%r8,%r3,200
	std	%r1,0(%r8)
	std	%r2,8(%r8)
	std	%r14,16(%r8)
	std	%r15,24(%r8)
	std	%r16,32(%r8)
	std	%r17,40(%r8)
	std	%r18,48(%r8)
	std	%r19,56(%r8)
	std	%r20,64(%r8)
	std	%r21,72(%r8)
	std	%r22,80(%r8)
	std	%r23,88(%r8)
	std	%r24,96(%r8)
	std	%r25,104(%r8)
	std	%r26,112(%r8)
	std	%r27,120(%r8)
	std	%r28,128(%r8)
	std	%r29,136(%r8)
	std	%r30,144(%r8)
	std	%r31,152(%r8)
	mflr	%r0
	std	%r0,160(%r8)
	stfd	%f14,168(%r8)
	stfd	%f15,176(%r8)
	stfd	%f16,184(%r8)
	stfd	%f17,192(%r8)
	stfd	%f18,200(%r8)
	stfd	%f19,208(%r8)
	stfd	%f20,216(%r8)
	stfd	%f21,224(%r8)
	stfd	%f22,232(%r8)
	stfd	%f23,240(%r8)
	stfd	%f24,248(%r8)
	stfd	%f25,256(%r8)
	stfd	%f26,264(%r8)
	stfd	%f27,272(%r8)
	stfd	%f28,280(%r8)
	stfd	%f29,288(%r8)
	stfd	%f30,296(%r8)
	stfd	%f31,304(%r8)
	li	%r3,0
	blr
	.size setjmp,.-.L.setjmp

	.globl longjmp
	.section ".opd","aw"
	.align 3
longjmp:
	.quad .L.longjmp,.TOC.@tocbase,0
	.previous
	.type longjmp, @function
.L.longjmp:
	andi.	%r6,%r3,8
	add	%r5,%r3,%r6
	mulli	%r7,%r6,24
	subfic	%r7,%r7,192
	add	%r7,%r3,%r7
	ld	%r0,0(%r7)
	mtcrf	0xff,%r0
	li	%r7,0
	lvx	%v20,%r5,%r7
	li	%r7,16
	lvx	%v21,%r5,%r7
	li	%r7,32
	lvx	%v22,%r5,%r7
	li	%r7,48
	lvx	%v23,%r5,%r7
	li	%r7,64
	lvx	%v24,%r5,%r7
	li	%r7,80
	lvx	%v25,%r5,%r7
	li	%r7,96
	lvx	%v26,%r5,%r7
	li	%r7,112
	lvx	%v27,%r5,%r7
	li	%r7,128
	lvx	%v28,%r5,%r7
	li	%r7,144
	lvx	%v29,%r5,%r7
	li	%r7,160
	lvx	%v30,%r5,%r7
	li	%r7,176
	lvx	%v31,%r5,%r7
	addi	%r8,%r3,200
	ld	%r1,0(%r8)
	ld	%r2,8(%r8)
	ld	%r14,16(%r8)
	ld	%r15,24(%r8)
	ld	%r16,32(%r8)
	ld	%r17,40(%r8)
	ld	%r18,48(%r8)
	ld	%r19,56(%r8)
	ld	%r20,64(%r8)
	ld	%r21,72(%r8)
	ld	%r22,80(%r8)
	ld	%r23,88(%r8)
	ld	%r24,96(%r8)
	ld	%r25,104(%r8)
	ld	%r26,112(%r8)
	ld	%r27,120(%r8)
	ld	%r28,128(%r8)
	ld	%r29,136(%r8)
	ld	%r30,144(%r8)
	ld	%r31,152(%r8)
	ld	%r0,160(%r8)
	mtlr	%r0
	lfd	%f14,168(%r8)
	lfd	%f15,176(%r8)
	lfd	%f16,184(%r8)
	lfd	%f17,192(%r8)
	lfd	%f18,200(%r8)
	lfd	%f19,208(%r8)
	lfd	%f20,216(%r8)
	lfd	%f21,224(%r8)
	lfd	%f22,232(%r8)
	lfd	%f23,240(%r8)
	lfd	%f24,248(%r8)
	lfd	%f25,256(%r8)
	lfd	%f26,264(%r8)
	lfd	%f27,272(%r8)
	lfd	%f28,280(%r8)
	lfd	%f29,288(%r8)
	lfd	%f30,296(%r8)
	lfd	%f31,304(%r8)
	// setjmp returns the value given to longjmp, 1 instead of 0
	extsw.	%r3,%r4
	bnelr
	li	%r3,1
	blr
	.size longjmp,.-.L.longjmp
