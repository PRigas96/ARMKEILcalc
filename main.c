/*----------------------------------------------------------------------------
Mikroepexergastes Project
Panagiotis Rigas
56841
panariga@ee.duth.gr
 *----------------------------------------------------------------------------*/
#include <MKL25Z4.H>
// A is the table that the user sees and means nothing to the pc
int A[4][4] = {{0x0, 0x1, 0x2, 0x3}, {0x4, 0x5, 0x6, 0x7}, {0x8, 0x9, 0xA, 0xB}, {0xC, 0xD, 0xE, 0xF}};
// Apc is the table seen by the pc and to which will match the information
int Apc[4][4] = {{0x0, 0x1, 0x2, 0x3}, {0x4, 0x5, 0x6, 0x7}, {0x8, 0x9, 0xA, 0xB}, {0xC, 0xD, 0xE, 0xF}};
// binary is presented in hex form since c has rejected any binary representation
// b is the table used for action
int B[4] = {0x0001, 0x0010, 0x0100, 0x1000}; 
// c is for storing the results
int C[2] = {0x0, 0x0};


__asm void coder(int* num, int* list, int* dst, int* code)
{
	LDR r5,[r0] ; load value 
	MOVS r6,#0 ; flag column
	MOVS r7,#0 ; flag row
	
	
	// MOVS r5,#0
loopfor
	LDR r4,[r1] ; load pointer of array
	CMP r5,r4
	BEQ done // when done row and col will be loaded to r6r7
	ADDS r1,#4 ; go to next value
	ADDS r6,#1 ; add 1 to flag
	CMP r6,#4 ; check if flag exeeds col range
	BEQ goto1 ; if yes goto1
	B loopfor
	
goto1
	MOVS r6,#0 ; make col flag zero
	ADDS r7,#1 ; add 1 to row flag
	B loopfor

done
	// r0 r5 r1 r4 are now free for use
	//MOVS r0,#0
	//MOVS r1,#0
	MOVS r4,#0
	
	// loopa is for row
	// loopa is for 1st 4bit word
	// it matches the number of row to a word in B matrix
loopa 
	LDR r5,[r3]
	CMP r7,#0 ; check if r7 is 0
	BEQ goto2
	ADDS r3,#4 ; go to next value
	ADDS r4,#4 ; add 4 to r4 so you can go back once finished
	SUBS r7,#1 ; decrease flag
	B loopa
goto2	
	LSLS r5,#16 ; here u l shift by 16 which is 4*4 because its hex and not binary. We are preparing to create the 8bit word
	STR r5,[r2] ; store value
	SUBS r3,r4 ; the reason behind this is that we wanna use this pointer again
	// loopb is for column word
loopb
	LDR r5,[r3]
	CMP r6,#0 ; check if r6 is 0
	BEQ goto3
	ADDS r3,#4 ; go to next value
	SUBS r6,#1 ; decrease flag
	B loopb
	//store values 
goto3	
	LDR r4,[r2] ; load value to r4
	ADDS r4,r5 ; add the other 4bit
	STR r4,[r2] ; store back to r2
	LDR r4,=0x1FFFF04C
	BX lr ; exit subroutine
}

__asm void decoder(int* codednum, int* Binary, int* deocder, int* Apc)
{
	LDR r4,[r0]; load value to be decoded
	MOVS r6,#0 ; make flags 0
	MOVS r7,#0
	//split the 2 numbers
	LSLS r4,#16 ; take 8bit word and make it 2 4-bits
	LSRS r4,#16
	//match with binary positioning
	//make 0001 into 0 0010 into 1 etc.
loop1
	LDR r5,[r1]
	CMP r4,r5
	BEQ goto4
	ADDS r1,#4 ; go to next value
	ADDS r6,#4 ; hold value so u can come back
	ADDS r7,#1 // r7 holds the row flag
	B loop1
goto4
	SUBS r1,r6
	MOVS r6,#0
	// we dont need r6 anymore
loop2
	LDR r4,[r0];
	LSRS r4,#16 ; create the second 4bit word
	LDR r5,[r1]

	CMP r4,r5
	BEQ goto5 ; if equal procced 
	ADDS r1,#4
	ADDS r6,#1 // r6 holds the column flag
	B loop2
goto5
	// this loop is left blank. It is not erased for the purpose of showing the correction
	// ADDS r6,#1 ;add 1 so no zeros exist. We do this to find the positioning in the 4x4 2d array
	// ADDS r7,#1 dont add since we need the last full rows
loop
	CMP r6,#0
	BEQ move
	ADDS r7,#4 ; add one row to r7
	SUBS r6,#1
	B loop
 
move
	LSLS r7,#2
	ADDS r3,r7 ; add value to inital pointer
	LDR r7,[r3] ; get value
	ADDS r2,#4
	STR r7,[r2] ; store 
	LDR r4,=0x1FFFF050
	BX lr ; exit
}
__asm void myFun(int* x,int* y,int* z)
{	
	// manually set stack pointer values
	LDR r0,=0x1FFFF498
	LDR r1,=0x1FFFF494
	LDR r2,=0x1FFFF490

	LDR r3,[r0] ; load value pointed by r0 to r3, num1
	LDR r4,[r1] ; num2
	LDR r5,[r2] ; action
	LDR r7,=0x1ffff004 ; where results will be stored
	MOVS r6,#0 ; init result to zero

	//
	// first we need to address the operator which is stored into r2
	// 
	CMP r3,#0 
	BEQ division
	
	CMP r3,#1
	BEQ multiplication
	
	CMP r3,#2
	BEQ subtraction
	
	CMP r3,#3
	BEQ addition
	// in case of error in r5 
	B err

division
	CMP r4,#0 ; is the divider 0?
	BEQ err
	CMP r5,r4 ; compare the diff
	BLT ready ; if r5 is less than r4 we are reay
	ADDS r6,r6,#1 ; add 1 to result
	SUBS r5,r5,r4 ; sub r5 = r5 - r4 r6 times so far
	B division
	
multiplication
	MOVS r6,r4
	MULS r6,r5,r6 ; multiplie r3xr4 and store to r6
	B ready

subtraction
	CMP r5,r4
	BLT err
	SUBS r6,r5,r4 ; sub r5-r4 and store to r6
 	B ready

addition
	ADDS r6,r5,r4 ; add r5+r4 and store to r6
	B ready
ready	
	STR  r6,[r7] ;store result to value pointed by r7
	ADDS r7,#4 ; go to next value
	BX lr	; exit function
err
	MOVS r6,#0
	STR r6,[r7]
	BX lr
}

int main(void)
{
	int n1, n2, a;
	int *num1, *num2, *num3;
	int *p, *pdec;
	// assign pointers 
	num1 = &n1;
	num2 = &n2;
	num3 = &a;
	p = &A[0][0];
	pdec = &Apc[0][0];
	// input values
	/*-------------------------------------------------
	printf("\nHello!, this programs accepts three numbers!\n1. The first number,\n2. The action to be calculated,\n3. The second number.);
	printf("Please enter the first number: ");
	scanf("%x", &n1);
	printf("\n Provide an action. \n4 for addition,\n3 for substrution\n2 for multiplication,\n1 for division\n: ");
  scanf("%d", &a);
  printf("\n Provide the second number: ") ;
  scanf("%x", &n2)
	---------------------------------------------------*/
	n1 = 0xA;
	n2 = 2;
	a = 0 ;
	// code value
	coder(num1,p,C,B);
	// num1 is now coded
	// and transmited
	// them decoded
	decoder(C, B, C, pdec);
	// and stored
	n1 = C[0];
 	//call my fun
	myFun(num1, num2, num3);

	while(1);
}
// *******************************ARM University Program Copyright © ARM Ltd 2013*************************************   


