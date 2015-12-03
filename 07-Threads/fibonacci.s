	.syntax unified
	.arch armv7-m
	.text
	.align 2
	.thumb
	.thumb_func

	.global fibonacci
	.type fibonacci, function

fibonacci:
  push { r4, r5, lr}	;// function prolog
  subs r4, r0, #0		;// r4 = r0 - 0
  ble .L3		; //if (r0 <= 0) goto .L3

  cmp r4, #1		; //Compare r4 to 1
  beq .L4		;// if (r4 == 1) goto .L4

  sub r0, r4, #1  ;//r0 = r4 - 1
  bl fibonacci		  ;// goto fibonacci @PC relative address

  mov r5, r0		;// r5 = r0
  sub r0, r4, #2	;// r0 = r4 - 2
  bl fibonacci		; //goto fibonacci @PC relative address

  adds r0, r5, r0
  pop { r4, r5, pc}
.L3:
  mov r0, #0
  pop { r4, r5, pc}
.L4:
  mov r0, #1
  pop { r4, r5, pc}


	.size fibonacci, .-fibonacci
	.end