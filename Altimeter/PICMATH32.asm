;  see http://www.piclist.com/techref/microchip/math/32bmath-ph.htm
; for another set of 32 bit math routines, explained slightly differently.

;*******************************************************
;*                   PICMATH32.ASM                     *
;*                                                     *
;*   A SET OF 32 BIT MATH ROUTINES AND MATH TUTORIAL   *
;*           For MicroChip PIC Microcontrollers        *
;*                                                     *
;*   Add32/Subtract32/Multiply32 handle both + and -   *
;*          Divide32 handles only + numbers            *
;*          Divide32x handles both + and -             *
;*-----------------------------------------------------*
;*                    Written By                       *
;*                Fr. Thomas McGahee                   *
;*            tom_mcgahee@mailsnare.com                *
;*                                                     *
;* Permission granted for anyone to use these routines *
;*          for personal or commercial use             *
;*         as long as author is given credit           *
;*           by including this credit block            *
;*                                                     *
;*    Users may modify routines to fit their needs     *
;*******************************************************
;*              Last Updated: April, 2004              *
;*******************************************************

;*******************************************************
;* If you find this tutorial and programs useful, you  *
;*       can let me know by sending an email to:       *
;*              tom_mcgahee@mailsnare.com              *
;*******************************************************

;*******************************************************
;* I have used long descriptive names for my variables *
;* and routines. I have done this to make the programs *
;* easier to understand. Once you understand how the   *
;* programs work, you might want to make the variable  *
;* names and program names smaller. You might also     *
;* choose to eliminate the comments.                   *
;*                                                     *
;* I suggest that if you plan on doing this, you make  *
;* a COPY of this assembly file and make changes only  *
;* to the copy.                                        *
;*******************************************************

;*******************************************************
;* Most material is repeated in various places.        *
;* I wrote this not only as a program, but also as a   *
;* tutorial. I have found that it is better to repeat  *
;* certain information than to force the user to look  *
;* all over the place in order to find it.             *
;*                                                     *
;* I hope that you find this information useful.       *
;* I have tried to include enough comments to make the *
;* logic behind the program understandable.            *
;*                                                     *
;* These routines were all written from scratch by     *
;* myself without reference to any programs written by *
;* anyone else. I refrained from including             *
;* optimizations except in those cases where it would  *
;* be easy to follow the logic. As such, you may find  *
;* a more highly optimized 32 bit divide program, for  *
;* example, but chances are you won't be able to       *
;* understand how it functions.                        * 
;*                                                     *
;* Multiply32 & divide32 are fairly complex, so I have *
;* prefaced them with a quick explanation of the       *
;* math process for those who are interested in        *
;* how it is performed. In addition, divide32 also has *
;* a pseudo-code example so you can follow the logic   *
;* of the program more readily.                        *
;*******************************************************

;*********************************************************************
;BEGIN GENERAL INFO GENERAL INFO GENERAL INFO GENERAL INFO GENERAL INFO 

;*********************************************************************
;* 32 BIT MATH ROUTINES FOR PIC MICROCONTROLLERS
;*            GENERAL INFORMATION
;*
;---------------------------------------------------------------------
;* BINARY REPRESENTATION
;* In the base ten number system each digit has one of ten states.
;* These ten states are: 0 1 2 3 4 5 6 7 8 9
;* The value of a base ten digit depends on it's state and its power.
;* 10^0=1  10^1=10  10^2=100  10^3=1,000  10^4=10,000 etc.
;* In the number 123, the 3 has a value of 3*10^0 which is 3*1=3
;* The 2 has a value of 2*10^1 which is 2*10=20
;* The 1 has a value of 1*10^2 which is 1*100=100
;* The value of the entire number 123 is the sum of the parts. This
;* gives us 100+20+3 which we call "one hundred and twenty three".
;*
;* Binary numbers work in the same way. HOWEVER:
;* Binary bits have only two states. The states are 0 and 1.
;* This simplifies the math tremendously. The rules for addition
;* are: 0+0=0  0+1=1  1+0=1 and 1+1=10 (note the carry). 
;* The rules for multiplication are even simpler:
;* 0*0=0  0*1=0  1*0=0  1*1=1
;*
;* Just as in base ten numbers, the value of a binary bit is a 
;* function of it's state and it's power.
;* The powers of two are: 2^0=1  2^1=2  2^2=4  2^3=8  2^4=16  2^5=32
;* 2^6=64  2^7=128 (and 2^8=256, etc.)
;* If a bit is 0, then it's VALUE is also 0, regardless of its power.
;* If a bit is a 1, then it's VALUE is equal to it's power. 
;* To determine the value of a binary number, add up the values of all
;* it's parts. For example, 00000101 is 4+1=5
;*
;* In binary, 1111 is 8+4+2+1=15. Since a 4 bit binary number can have
;* 16 possible states (0,1,2,....,14,15), these groupings of four
;* bits are referred to as hexadecimal numbers, or hex digits. 
;* Since our "usual" counting numerals are 0 1 2 3 4 5 6 7 8 9
;* hex numerals build on this and become:
;* 0 1 2 3 4 5 6 7 8 9 A B C D E F.
;* Thus A=10  B=11  C=12  D=13  E=14  F=15. At first, hex notation may
;* seem a little strange, but it is a very useful form of notation.
;*
;* The basic binary "word" length is 8 bits. We call 8 bits a byte.
;* 4 bits is half of a byte. Because the words "byte" and "bite" sound
;* the same, engineers began to playfully call 4 bits a "nibble", and
;* the name has stuck. 4 bits is a nibble. Two nibbles make up a byte. 
;* Hex bytes consist of two hex nibbles, and so take on the form
;* XX such as 2B, which is 2*16+11=43
;*
;* To avoid confusion, MPASM uses base specifiers when dealing with
;* numbers. Let's look at how the base ten number 266 might be shown:
;* In decimal it is d'266'
;* In binary it is b'100001010' which is 256+8+2=266
;* In hex it is h'010A' because 01=1*256=256. 0A=10*1=10. 
;* The sum is 256+10=266
;*
;* By the way, MPASM allows several variations on how the radix
;* can be specified. Note also that leading zeros can be left out.
;* Hex notation allows both upper and lower case for A thru F.
;*
;* Here are the radix notations available for numbers:
;* Decimal: d'266' or D'266' or .266  (.266 is confusing. Avoid it).
;* Binary: b'100001010' or B'100001010'
;* Hex: h'010A  or  H'010A'  or  0x010A
;* Although MPASM allows you to choose a default radix, it is usually
;* best to always specify the radix using one of the above notations.
;* A specified radix will always over-ride the selected default radix.
;*
;* To specify a negative number in any radix, specify 0-number.
;* For example, 0-d'266' would yield -266. (Since 0 is the same in
;* any radix, you do not have to specify it's radix explicitly.)
;*
;* Long strings of binary digits become difficult to read. In base
;* ten we use commas every 3 places to the left of the decimal point to
;* help us deal with large numbers like 23,452,751.
;* Binary numbers are sometimes broken up into sets of 8 bits, 
;* called bytes.
 
;* In binary we do not use commas, and MPASM does not allow spaces
;* within numbers. However, when working with binary on PAPER, I
;* have always found it useful to break large binary numbers into
;* bytes and separate them with a space. I also separate
;* hex numbers with a space between each byte. I find that this makes
;* it easier to work with. For example, the 32 bit binary number
;* 01110010100110010101001110101011 looks very intimidating, whereas
;* 01110010 10011001 01010011 10101011 is a bit easier to handle, and
;* 72 99 53 AB (which is the same binary number written in hex) seems 
;* almost friendly by comparison. 
;*
;---------------------------------------------------------------------
;* UNSIGNED REPRESENTATION and SIGNED TWOS COMPLEMENT REPRESENTATION:
;*
;*	32 bits can be used to represent a value up to 4,294,967,295.
;*	That is (256*256*256*256)-1
;*	In this case only positive values can be represented. That is
;*	just fine in the case of addition and multiplication, but
;*	cannot work if you also want to use negative numbers. 
;*
;*	Note that subtraction and division involve the use of negative
;*	numbers, so to perform THESE operations you HAVE to have a way
;*	of representing negative numbers.
;*
;---------------------------------------------------------------------
;* SIGNED BINARY TWOS COMPLEMENT REPRESENTATION
;*	A binary number can be converted into it's negative form by first
;*	complementing the number (make all 1's 0's, and all 0's 1's) and
;*	then adding 1 to the result. Complementing it puts it into
;*	what is called "one's complement notation". After you add one to
;*	THAT, you have what is called "two's complement notation".
;*
;*	One of the features of two's complement notation is that the
;*	MSB (Most Significant Bit) of a negative number is ALWAYS 1. 
;*	In the case of a 32 bit representation, that means 1 bit is 
;*	allocated to sign duty, and the other 31 bits can hold value
;*	information. That is why we have to restrict the input and
;*	output VALUE of our numbers to 31 bits in a 32 bit system.
;*
;*	Please note that the MSB being 1 is NOT the same thing as the
;*	MSB being a negative sign.
;*
;*	Take the number 2 in 32 bit binary. It would be represented as:
;*	00000000 00000000 00000000 00000010. When we complement it we get
;*      11111111 11111111 11111111 11111101  adding 1 to this we get
;*      11111111 11111111 11111111 11111110 which represents -2.
;*
;*	00000000 00000000 00000000 00000010 if we add 2
;*      11111111 11111111 11111111 11111110 and -2, we get
;*	-----------------------------------
;*   (1)00000000 00000000 00000000 00000000 The (1) represents a carry
;*	out from the MSB (Most Significant Bit). Note that the value of
;*	the result (ignoring the carry) is 0.  2-2=0.
;*
;*	To represent negative numbers in a 32 bit system we use
;*	twos complement notation where a negative number will always
;*	have a 1 at the MSB position. You will hopefully have noticed
;*	that in twos complement notation we can perform subtraction
;*	through the addition of negative numbers. Twos complement
;*	notation allows both positive and negative numbers to be
;*	represented and mathematically manipulated. Twos complement
;*	notation is sometimes referred to as twos complement
;*	representation, or signed binary.
;*
;*	With SIGNED binary, 32 bits can represent +/- 2,147,483,647
;*
;*	You might be wondering why I didn't say +/- 2,147,483,648. 
;*	It turns out that 2,147,483,648 and -2,147,483,648 in a 32 bit
;*	representation end up having the same representation. For this
;*	reason this value is disallowed in 32 bit representation.
;*
;*	Let's look at why this is so. Convert to signed binary form:
;*	10000000 00000000 00000000 00000000 is complemented to become
;*	01111111 11111111 11111111 11111111. Add 1 to this and you get
;*	10000000 00000000 00000000 00000000 which is exactly the same
;*	as the original. 
;*
;*	An easy way to help you remember what the disallowed 
;*	value is in any binary system is to make only the MSB a 1.
;*	For example, in a 24 bit binary system the disallowed value will
;*	be 10000000 0000000 0000000. In a 16 bit system the disallowed
;*	value will be 10000000 00000000. In an 8 bit system the
;*	disallowed value will be 10000000. 
;*
;*	By the way, if you use the two's complement method to find the
;*	negative of 0 you get 0. But here there is no problem,
;*	because 0 - 0 = 0 so the result is always correct. Another way
;*	to look at this is to realize that 0 is neither positive nor
;*	negative. Cool!
;*
;*	The really nice thing about two's complement notation is that
;*	when you add numbers in this notation you always get the right
;*	answer. Add 123 and -125 and you get -2. In binary we perform
;*	subtraction through the addition of negative numbers. It
;*	works beautifully.
;*
;*	All 32 bits can be used for addition and multiplication (only)
;*	if the user chooses to *interpret* ALL numbers as positive.
;*
;*	Obviously, subtraction REQUIRES twos complement notation
;*	to allow negative numbers to be used. Subtraction is restricted
;*	to handling 31 bits of input and output for positive values
;*	when the total bit length is 32 bits. 
;*
;*	Division involves a subtraction process, so intermediate 
;*	results at certain stages may go negative. This means 
;*	that numerators, multipliers, and quotients are restricted 
;*	to a 31 bit maximum length, and may ONLY be positive. 
;*
;*	It is the 31 bit restriction to allow subtraction that dictates
;*	the use of only POSITIVE input and output values for the
;*	divide32 routine. 
;*
;*	32 bit binary values are used in sets that each contain 4 bytes.
;*	MSB is leftmost bit of <set>_4, and LSB is rightmost bit 
;*	of <set>_1.
;*
;*	For example: <accumulator> is the general specifier for a 
;*	particular 32 bit set that is comprised of 4 bytes named
;*	accumulator_4  accumulator_3  accumulator_2  accumulator_1
;*
;*	Bits within a byte are specified with a comma and a value from
;*	0 to 7. For example, "accumulator_1,0" would specify the LSB.
;*
;*
;*	It is easy to decode the value of a binary number. Each bit
;*	represents a power value. Just add up all the power values
;*	where there is a 1. For example, in the byte 01101001
;*	we get 1 + 8 + 32 + 64 = 105
;*	
;---------------------------------------------------------------------
;* MAXIMUM INPUT VALUE:
;*	Maximum twos complement input value to any math function is
;*	therefore limited to +/- 2,147,483,647
;*	This is 31 bits for positive numbers.
;*	BE CAREFUL! The REAL limit is a function of the RESULT!
;*	For example, 2,147,483,647 + 3 would violate max RESULT!
;*	In this example the result would APPEAR to be negative!
;*
;*	Note that the math32 routines will not catch that kind
;*	of error, because they do not know whether you are
;*	treating the result as a positive or negative number.
;*	If treating it as a positive number, the answer would be
;*	correct. If treating the answer as a negative number,
;*	the answer would be incorrect.
;*
;*	!!! In the case of add32 and multiply32, you may 
;*	raise the bit limit to 32 if you are *interpreting* 
;*	ALL numbers as positive. Make sure RESULT does not
;*	exceed 32 bit size limit! Maximum value in this
;*	case is raised to 4,294,967,295
;*
;---------------------------------------------------------------------
;* MAXIMUM RESULT:
;*	MAXIMUM result of any operation is limited 
;*	to +/- 2,147,483,647.
;*
;*	!!! In the case of add32 and multiply32, you may 
;*	raise the bit limit to 32 if you are interpreting 
;*	ALL numbers as positive. Make sure ANSWER does not
;*	exceed 32 bit size limit! Maximum answer is then
;*	raised to 4,294,967,295
;*
;*	For multiplication, the SUM of the BITS must be less than 32.
;*	For example, a 16 bit number times a 15 bit number 
;*	will yield a 31 bit result. This is OK. But a 16 bit number
;*	times a 16 bit number yields a 32 bit result that
;*	would have lost any sign information. This is OK if
;*	you are working only with positive numbers, but is
;*	disastrous if working with negative numbers!	
;*
;*	When I refer to a number as being a 15 bit number, I mean
;*	that from the LSB to the left-most "1" there are 15 bits.
;*	Example: 10000000 00000000 is a 16 bit number in 16 bit field.
;*		 01000000 00000000 is a 15 bit number in 16 bit field.
;*		 00000000 00000101 is a 3 bit number in 16 bit field.
;*		 00000000 00000001 is a 1 bit number in 16 bit field.
;*
;*	Bits within a byte are given bit identifiers where 0 is the
;*	LSB, and 7 is the MSB. For example: in the byte accumulator_1 
;*	containing 00001000, the "1" bit is accumulator_1,3.
;*
;*	          Shown visually:
;*	76543210  are the bit identifiers
;*	00001000  is the accumulator_1 contents.
;*	    ^     the "1" bit is at position "3"
;---------------------------------------------------------------------
;* 32 BIT OVERFLOW and ERROR FLAGS:
;*
;*	Certain attempted operations will exceed the 32 bits!
;*
;*	OVERFLOW is detected by the add32 routine, and returned
;*	in _mflag_overflow. Since add32 is called by
;*	subtract32/multiply32, these will also
;*	pass along (and possibly act upon) _mflag_overflow.
;*
;*	Upon entry, multiply32 and divide32 both clear ALL 
;*	math error flags by clearing the mflag register.
;*
;*	!!! Direct calls by the user to add32 and subtract32
;*	require the user to CLEAR at least _mflag_overflow
;*	before calling the routines IF they want to make use of 
;*	_mflag_overflow to report an overflow condition.
;*
;*	!!!Divide32 ALWAYS returns _mflag_overflow cleared, since
;*	it specifically restricts input values to 31 bit
;*	positive values. Thus overflow is logically impossible.
;*	Divide32 provides some additional error checking.
;*
;.....................................................................
;* OVERFLOW IN GENERAL:
;*
;*	!!! User calls to add32 and subtract32 must clear the 
;*	_mflag_overflow before calling the routines if they wish
;*	to make use of this feature.
;*
;*	_mflag_overflow reports:
;*		OVERFLOW from any 32 bit operation.
;*		this includes add32/subtract32/multiply32
;*
;*	BE AWARE of the fact that adding two unsigned
;*	32 bit numbers can generate a 33 bit result,
;*	which CANNOT be contained in a 32 bit number.
;*	Overflow will result and be flagged.
;*
;*	BE AWARE that adding two 31 bit negative numbers
;*	can result in a 32 bit negative result that then
;*	LOOKS like a positive number because the sign bit
;*	got shifted out into the carry and got lost!
;*	Overflow will result and be flagged.
;*
;*	BE AWARE that multiplying two 32 bit unsigned
;*	numbers could result in a 64 bit answer! 
;*	Overflow will result and be flagged.
;*	Carry from the 32nd bit will cause an overflow, and the
;*	program will report the error via _mflag_overflow!
;*	It is YOUR responsibility to check this flag after
;*	add32/subtract32/multiply32
;*
;*	BE AWARE that multiplying two 32 bit signed numbers 
;*	can result in a 33rd bit overlow, and the
;*	remaining 32 bit result is not what you think it is!
;*	_mflag_overflow will report the error.
;*
;.....................................................................
;* SPECIAL ERROR REPORTING FOR CALLS TO DIVIDE32/DIVIDE32X:
;*
;*	divide32 will never return _mflag_overflow set to 1,
;*		so you can ignore it completely.
;*
;*	_mflag_derror reports:
;*		ANY fatal error in divide32. (_dneg and _dzero).
;*		Test this flag first.
;*
;*	_mflag_dneg reports:
;*		Attempt to use negative numbers in divide32.
;*		It will flag this for <numerator> and <denominator>.
;*		(NOT used in divide32x)
;*
;*	_mflag_dzero reports:
;*		Attempt to divide by 0 in divide32.
;*
;*	These flags are automatically cleared upon entry to
;*	divide32. The user does not have to clear them.
;*
;*
;*	BE AWARE that divide32 allows only POSITIVE values,
;*	and their size cannot exceed 31 bits. This is the 
;*	bad news. The good news is that the answer is then
;*	forced to be 31 bits or less and cannot cause
;*	an overflow error.
;*	
;---------------------------------------------------------------------
;* VARIABLE DATA SETS USED FOR MATH OPERATIONS:
;*
;* <ACCUMULATOR>:
;*	primary use: 		<accumulator> for add32.
;*	 add32:			<accumulator> = <accumulator> + <operand>
;*
;*	add32 called by:
;*	 subtract32:		<accumulator> = <accumulator> + 
;*						negated <operand>
;*	 multiply32: 		<accumulator> = <multiplier> * <operand>
;*	 multiply32plus: 	<accumulator2> = <accumulator1> + 
;*						 (<multiplier2> * <operand2>)
;*
;*	<accumulator>:
;*	 aliased as: 		<numerator> for divide32
;*	 aliased as: 		<remainder> AFTER divide32 is done
;*
;*	used as INPUT to:	add32/subtract32/multiply32/*divide32
;*					*aliased as <numerator>
;*	used as OUTPUT from:	add32/subtract32/multiply32/*divide32
;*					*aliased as <remainder>
;*	holds ANSWER for:	add32/subtract32/multiply32
;*	holds <remainder>:	after divide32. (Leftover from division).
;*
;* <OPERAND>:
;*	primary use: 		<operand> for add32
;*	 add32:			<accumulator> = <accumulator> + <operand> 
;*
;*	add32 called by:
;*	 neg32:			<operand> = negated<operand>
;*	 subtract32:		<accumulator> = <accumulator> + 
;*						negated<operand>
;*	 multiply32		<accumulator> = <multiplier> * <operand>
;*	 divide32:		<operand> is aliased as <denominator>
;*				<quotient> = <numerator> / <denominator>
;*	<operand>:
;*	aliased as: 		<denominator> for divide32
;*
;*	used as INPUT to:	add32/negate32/subtract32/multiply32/*divide32
;*					*aliased as <denominator>
;*	used as OUTPUT from:	negate32
;*	holds ANSWER for:	negate32
;*
;* <MULTIPLIER>:
;*	primary use:		<multiplier> in multiply32
;*	 multiply32: 		<accumulator> = <multiplier> * <operand>
;*
;*	<multiplier>:
;*	aliased as: 		<quotient> for divide32
;*
;*	used as INPUT to:	multiply32
;*	used as OUTPUT from:	*divide32 
;*					*aliased as <quotient>
;*
;* <NUMERATOR>:
;*	primary use:		<numerator> in divide32
;*	 divide32:		<quotient> = <numerator> / <denominator>
;*
;*	alias of:		<accumulator>
;*
;*	aliased as:		<remainder> (at end of division it holds remainder)
;*
;*	used as INPUT to:	*add32/*subtract32/*multiply32/divide32 
;*					*aliased as <accumulator>
;*	used as OUTPUT from:	*add32/*subtract32/*multiply32/divide32 
;*					*aliased as <accumulator>
;*	holds <remainder>:	after divide32. (Leftover from divide operation).
;*
;* <DENOMINATOR>:
;*	primary use: 		<denominator> in divide32
;*	 divide32:		<quotient> = <numerator> / <denominator>
;*
;*	alias of:		<operand>
;*
;*	used as INPUT to:	*add32/*negate32/*subtract32/*multiply32/divide32
;*					*aliased as <operand>
;*	used as OUTPUT from:	*negate32 
;*					*aliased as <operand>
;*
;* <QUOTIENT>:
;*	primary use:		<quotient> in divide32 
;*	 divide32:		<quotient> = <numerator> / <denominator>
;*
;*	alias of:		<multiplier>
;*
;*	used as INPUT to:	divide32/*multiply32
;*					*aliased as <multiplier>
;*	used as OUTPUT from:	divide32
;*
;---------------------------------------------------------------------
;* QUICK LOOK AT MATH FUNCTIONS
;*
;* In general, answers are limited to +/- 2,147,483,647.
;* In this implementation, only the divide32 routine is
;* restricted to the use of POSITIVE numbers. All else 
;* may freely use a mix of positive and negative numbers
;*
;* Number notation is twos complement for negative numbers.
;* This restricts positive numbers in general to 31 bits
;* so we can allow 31 bit positive numbers to be negated
;* to 32 bit wide twos complement form.
;*
;* add32
;* Addition:	<accumulator> = <accumulator> + <operand>
;*
;* neg32
;* Negation:	<operand> = negative of <operand>
;*		Negating a negative number makes it positive.
;*		(uses twos complement notation)
;*
;* subtract32 
;* Subtraction:	<accumulator> = <accumulator> - <operand>
;* 
;* multiply32 
;* Multiplication: <accumulator> = <multiplier> * <operand>
;*	Upon entry the <accumulator> is cleared.
;*	(See multiply32plus for multiply that does NOT clear 
;*	the <accumulator>).
;*	NOTE that SUM of input BITS (+ form) must be LESS than 32.
;*	For example, a 22 bit positive number times an 
;*	11 bit positive number will yield a 33 bit positive 
;*	result. Overflow flag will be set.	
;*
;* multiply32plus
;* Multiplication with Addition:
;* <accumulator> = <accumulator1> + (<multiplier2> * <operand2>)
;*	This is included as an added convenience to the basic
;*	math routines. Requires no additional code.
;*
;* divide32
;* Division:	<quotient> = <numerator> / <denominator>
;*		(remainder is returned in numerator>
;*	In this implementation, input values are RESTRICTED to only
;*	POSITIVE numbers. Max positive size of input(s) is 31 bits.
;*	Max positive result is 31 bit number 2,147,483,647
;*
;*
;* divide32x
;* Division:	<quotient> = <numerator> / <denominator>
;*		(remainder is returned in numerator>
;*	In this implementation, input values are within the range
;*	+/- 2,147,483,647. Result has range +/- 2,147,483,647.
;*
;*	The tradeoff is that there is some additional overhead.
;*	Divide32x determines the correct output polarity,
;*	converts inputs to positive only values, calls divide32,
;*	and then converts the <quotient> to a negative form if
;*	the previously determined polarity demands a negative
;*	output value.
;*
;*	(The only possible error is attempted division by zero.)
;*
;*  *******************************************************
;*  * Any remarks made about multiply32 apply also to     *
;*  * multiply32plus. The only difference is that         *
;*  * multiply32 begins by clearing the <accumulator>,    *
;*  * but multiply32plus keeps the current <accumulator>  *
;*  * contents and adds into the existing contents.       *
;*  *******************************************************
;*
;***********************************************************************

;END GENERAL INFO GENERAL INFO GENERAL INFO GENERAL INFO GENERAL INFO
;***********************************************************************
;BEGIN DIRECTIVES DIRECTIVES DIRECTIVES DIRECTIVES DIRECTIVES DIRECTIVES 

;===================================================================
;= THE FOLLOWING SECTION CONTAINS DIRECTIVES, DEFINITIONS, AND     =
;= ASSIGNMENTS THAT ALLOW THE 32 BIT MATH ROUTINES TO WORK.        =
;= SOME OF THIS STUFF YOU WILL NEED TO MERGE INTO YOUR OWN PROGRAM =
;= IN ORDER TO USE THE MATH ROUTINES. SOME OF THIS STUFF IS HERE   =
;= TO ALLOW YOU TO TEST THE MATH ROUTINES IN A STAND-ALONE MODE    =
;= THAT YOU CAN LOAD INTO MPLAB AND PLAY WITH.                     =
;= REMEMBER TO REMOVE THE "EXTRA" CODE WHEN YOU INTEGRATE THE MATH =
;= ROUTINES INTO YOUR OWN PROGRAMS!                                =
;===================================================================

;!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
;! NOTE: set MPLAB assembler for CASE SENSITIVITY OFF. Failure to do !
;! this may cause false error messages to be spit out.               !
;! You have been warned!                                             !
;! Set default radix to decimal. You have been warned!               !
;! Set tabs to 4. You have been warned!                              !
;!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

; First, some directives to allow for testing of the math functions

	list	p=16f876	;this directive MUST come first
				;!!! set this to the version of
				;!!! PIC that you will be using.

include P16F876.INC		;!!! set this for proper 
				;PIC INClude file

;END DIRECTIVES DIRECTIVES DIRECTIVES DIRECTIVES DIRECTIVES 
;*****************************************************
;BEGIN RAM RAM RAM RAM RAM RAM RAM RAM RAM RAM RAM RAM 

;**************************************************
;* Declare RAM Variable usage using CBLOCK method *
;**************************************************

	cblock	h'20'	;bank 0  h'20' to h'6f'. 
			;80 locations (+16 Globals)
			;!!! You may place variables elsewhere 
			;if desired.

;CBLOCK FORMAT is:
;variable_name	;followed by comments

mflag		;8 bit math flag register (see bit definitions below)
isitneg		;a byte used to indicate polarity DURING divide32x

scancounter	;used by divide32 to keep track of bit position

;*** 32 bit SETS ***

	;NOTE: I refer to SETS of bytes such as accumulator_1 thru 
	;accumulator_4 by placing BASE name inside angle brackets.
	;Thus, <accumulator> refers to the entire accumulator SET.

	;NOTE: The ORDER in which sets like <accumulator> are
	;declared is significant, as they are sometimes referenced
	;referenced in the routines using indirect addressing via 
	;fsr/indf method.
	;Do NOT rearrange the order within a set.
	;MSB is leftmost bit of _4 member of set

accumulator_4	;32 bit <accumulator> for 
accumulator_3	;add32/subtract32/multiply32.
accumulator_2	; aliased as <numerator> when doing divide32. 
accumulator_1	; aliased as <remainder> after divide32

goback_4	;32 bit copy used to speed things up
goback_3	;when divide32 routine has to back-track.
goback_2	; !!! these can also be used by other routines
goback_1	; !!! for temporary storage if they don't use
		; !!! the divide32 routine.

operand_4	;32 bit operand for 
operand_3	;add32/neg32/subtract32/lshift32/multiply32
operand_2	;
operand_1	; aliased as <denominator> for divide32

multiplier_4	;32 bit multiplier. 
multiplier_3	;<accumulator> = <multiplier> * <operand>
multiplier_2	; 
multiplier_1	; aliased as <quotient> for division

		endc	;END OF CBLOCK VARIABLE DECLARATIONS

;address of last item should be a max value of 6fh.
;if address exceeds 6fh, then we would have to move to another 
;ram block. Then we would have to keep track of bank useage.
;It is always nice when you can keep all variables in ram bank 0

;END RAM RAM RAM RAM RAM RAM RAM RAM RAM RAM RAM RAM RAM RAM 
;***********************************************************
;BEGIN ALIASES ALIASES ALIASES ALIASES ALIASES ALIASES ALIASES 

;*******************************
; SHARED RESOURCES AND ALIASES *
;*******************************

;These shared resources have different names to reflect their different
;uses within the math routines. This makes the routines easier to 
;understand, and at the same time keeps resource usage smaller by 
;sharing the RAM resources.
;THE USE OF AN ALIAS REQUIRES NO ADDITIONAL RAM!

;NOTE: When using MPLAB to experiment with the program, be aware 
;that the variable WATCH window can only be loaded with the 
;variable names declared in cblock above.
;This means, for example, that to watch the aliased set 
;<numerator> you would have to "watch" <accumulator> 
;in the watch window.

;#DEFINE FORMAT:
;#define alias		text_to_use_in_place_of_alias

;#define is really a
;text replacement mechanism
;used by MPASM

#define numerator_4	accumulator_4	;32 bit <numerator> 
#define numerator_3	accumulator_3	;for divide32
#define	numerator_2	accumulator_2	
#define	numerator_1	accumulator_1

#define remainder_4	accumulator_4	;32 bit <remainder> 
#define remainder_3	accumulator_3	;at end of divide32
#define	remainder_2	accumulator_2		
#define	remainder_1	accumulator_1	

#define	denominator_4	operand_4	;32 bit <denominator> 
#define	denominator_3	operand_3	;for divide32
#define	denominator_2	operand_2
#define	denominator_1	operand_1

#define	quotient_4	multiplier_4	;32 bit <quotient> returned
#define	quotient_3	multiplier_3	;by divide32
#define	quotient_2	multiplier_2
#define	quotient_1	multiplier_1

;*************************
;* mflag bit definitions *
;*************************

			;I use a leading underscore to indicate that
			;we are dealing with a single BIT.
			;Flags below are arranged as they are
			;just for ease of viewing in MPLAB

			;A 1 means the flag is asserted for that error.

#define	_mflag_0	mflag,0	;not used
#define _mflag_dzero	mflag,1	;divide by zero in divide32/x 
#define	_mflag_2	mflag,2	;not used
#define _mflag_dneg	mflag,3	;negative input to divide32
#define	_mflag_4	mflag,4	;not used
#define	_mflag_derror	mflag,5	;general error in divide32/x routine
#define	_mflag_6	mflag,6	;not used
#define	_mflag_overflow	mflag,7	;overflow occured in add/sub/mult32



#define _z 	status,2	;I prefer letters to numbers,
#define _dc 	status,1	; since they are easier to
#define _c 	status,0	; relate to the function

;END ALIASES ALIASES ALIASES ALIASES ALIASES ALIASES ALIASES ALIASES 
;*********************************************************************
;BEGIN TESTING TESTING TESTING TESTING TESTING TESTING TESTING TESTING   

;!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
;!!! TEST PROGRAM TO VERIFY MATH ROUTINES !!!
;!!!   USED *ONLY* FOR TESTING PURPOSES   !!!
;!!!   USER SHOULD REMOVE AFTER TESTING   !!!
;!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

	org	0	;Every PIC has its prime origin at 0

	nop		;This nop is required by my ICD1 In Circuit
			;Debugger, so here it is. You may leave it
			;out with impunity if you don't use an ICD1.

	goto	init	;You have to jump past the interrupt 
			;vector at 0004.
			;I usually have an initialization routine that I
			;always call "init" that I jump to...

	org	d'4'	;Interrupt Service Routine (ISR) vector address.

;this is where vector interrupt would go.
;for testing purposes we do not need an interrupt routine. 
;So I am only including a return from interrupt code.

interrupt_service_routine

	retfie	;Even an "empty" isr should have a return from interrupt.
		;Better Safe than Sorry!

	org	$ ;It is always good to explicitly declare things
		  ;like the origin... even if not required by law.
		  ;$ means "current program counter value"

init

;!!! This is where I would usually have my initialization
;!!! routine(s) that would set up the hardware such as port
;!!! usage, setting up timers, counters, and all that
;!!! nitty-gritty stuff that you just gotta do.	

;!!! since testing of pure software with no hardware requirements
;!!! doesn't use any i/o, and interrupt defaults to it's off state,
;!!! we can skip all the usual init stuff and get right to work.

;Allow Testing of Math Functions HERE.
;This example can be used to test Divide32x
;load <numerator> and <denominator> with 31 bit max size numbers
;(in a 32 bit field, of course)

;Adjust testing routine to test whatever function you want.

	movlw	b'01000000'
	movwf	numerator_4
	movlw	b'00000000'
	movwf	numerator_3
	movlw	b'00000000'
	movwf	numerator_2
	movlw	b'00000000'
	movwf	numerator_1

	movlw	b'00000000'
	movwf	denominator_4
	movlw	b'00000000'
	movwf	denominator_3
	movlw	b'00000000'
	movwf	denominator_2
	movlw	b'00000010'
	movwf	denominator_1
	

	call	divide32x	; answer will be found in <quotient>,
endlessloop
	goto	endlessloop	;set breakpoint here for testing.

		;endless loop is used for testing only.
		;Once this point is reached the program
		;STAYS here. I usually set a BREAKPOINT
		;in MPLAB at endlessloop when doing testing.

		;Use MPLAB to examine results.
		;Remember that WATCH window in MPLAB can only
		;display PRIMARY RAM variable names, NOT
		;the ALIASED names. 	
		;For your convenience, here are the ALIASES:
		;<numerator> is <accumulator>
		;<denominator> is <operand>
		;<quotient> is <multiplier>
		;<remainder> is <numerator> is <accumulator>
		;You should also watch mflag for the 
		;error bits.

		;The MAX number of cycles to do a complete
		;division in divide32 is about 2,800 cycles.
		;With a 20 Mhz clock this is 560 microseconds.

		;Minimum divide time is about 78 cycles due
		;to short-circuit evaluation of division
		;by 1 (which would otherwise have taken more
		;that 5,000 cycles!)
 
		;The larger the denominator, the shorter
		;the time it takes to determine the <quotient>.
				


;!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
;!!!   END OF TEST PROGRAM SECTION   !!!
;!!! REMOVE ONCE TESTING IS FINISHED !!!
;!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

;The following are the actual MATH routines. This is what you will 
;eventually end up copying into your own program. But don't forget 
;to also copy over the RAM declarations and the Alias Definitions 
;and merge them into your own declaration section.

;END TESTING TESTING TESTING TESTING TESTING TESTING TESTING TESTING
;*******************************************************************

;*******************************************************************
;BEGIN MATH MATH MATH MATH MATH MATH MATH MATH MATH MATH MATH MATH  

;*******************************************************************
;BEGIN NEG32 NEG32 NEG32 NEG32 NEG32 NEG32 NEG32 NEG32 NEG32 NEG32 									

;NAME:		neg32

;FUNCTION:	<operand> = negative of <operand>.

;ENTER WITH:	<operand>

;RETURNS:	<operand> = negative of <operand>.

;CHANGED:	<operand> is now negative of original <operand>

;NOTES:		If <operand> comes in positive, it returns negative
;		If <operand> comes in negative, it returns positive
;		Uses twos complement method

;---------------------------------------------------------------	
;Twos complement of a number is generated by first complementing 
;the number and then adding 1 to the result. It makes a positive
;number negative, and a negative number positive.

neg32	;make twos complement of operand

	comf	operand_1,f	;ones complement of operand:
	comf	operand_2,f	;4 bytes to get 32 bits...
	comf	operand_3,f
	comf	operand_4,f

	incf	operand_1	;twos complement of operand
	btfss	_z		;If result is 0 we need ripple carry
	return			;return if no more carries needed

	incf	operand_2	;perform carry by incrementing
	btfss	_z		;If result is 0 we need ripple carry
	return			;return if no more carries needed

	incf	operand_3	;perform carry by incrementing
	btfss	_z		;If result is 0 we need ripple carry
	return			;return if no more carries needed
	incf	operand_4	;perform carry by incrementing
				;Ignore any carry out from MSB
	return			;All 4 bytes ripple carried, 
				;so return.

;---------------------------------------------------------------	
;END NEG32 NEG32 NEG32 NEG32 NEG32 NEG32 NEG32 NEG32 NEG32 NEG32
;***************************************************************

;***************************************************************
;BEGIN LSHIFT32 LSHIFT32 LSHIFT32 LSHIFT32 LSHIFT32 LSHIFT32 

;NAME:		lshift32

;FUNCTION:	single shift left of 32 bit <operand> with zero 
;		fill at LSB. If overflow of MSB occurs, carry 
;		flag is set to 1.
;		This operation effectively multiplies <operand> by 2.
;		<operand> = <operand> * 2

;ENTER WITH:	<operand>.

;RETURNS:	<operand> = <operand> * 2
;		Overflow is returned in carry flag.

;CHANGED:	<operand>

;CALLED BY:	multiply32/divide32

;---------------------------------------------------------------	

;Clear Carry and Shift 32 bits in <operand> once to the LEFT. 
;Return carry. 

lshift32
	bcf	_c		;clear the carry bit to be shifted into LSB.
	rlf	operand_1,f	;_c lshifts into _1. _1 MSB lshifts into _c.
	rlf	operand_2,f	;_c lshifts into _2. _2 MSB lshifts into _c.
	rlf	operand_3,f	;_c lshifts into _3. _3 MSB lshifts into _c.
	rlf	operand_4,f	;_c lshifts into _4. _4 MSB lshifts into _c.
	return			;return with possible overflow in _c

;END LSHIFT32 LSHIFT32 LSHIFT32 LSHIFT32 LSHIFT32 LSHIFT32 
;***************************************************************
;BEGIN ADD32 ADD32 ADD32 ADD32 ADD32 ADD32 ADD32 ADD32 ADD32 

;NAME:		add32

;FUNCTION:	<accumulator> = <operand> + <accumulator>.
;		basic addition of positive and negative numbers.

;ENTER WITH:	<operand> and <accumulator> loaded. 
;		(Calling program must clear _mflag_overflow)
;		(But only if INTERESTED in the overflow condition)

;RETURNS:	<accumulator> = <accumulator> + <operand> 
;		<operand> is unchanged.
;		_mflag_overflow is SET if there is an 
;		overflow of the MSB.
;		(Calling program should CLEAR _mflag_overflow to 0)
;		(But only if it is INTERESTED in the 
;		overflow condition)
		

;NOTES:		This routine allows "sequential adds/subtracts" 
;		to the accumulator from the <operand> set.
;	example: <accumulator> = (((A + B) + C) + D)
;		also, since result of multiply32 is 
;		returned in <accumulator>,
;		you can do things like 
;		<accumulator> = (((A * B) + C ) + D)

;		Calling program is responsible for first clearing 
;		_mflag_overflow if the overflow detection feature 
;		is to be used.
;		(For example, multiply32 routine does 
;		this automatically).
;		If you call add32 directly, YOU must 
;		clear _mflag_overflow.
;		The same applies if you call subtract32.

;CALLED BY:	subtract32/multiply32/divide32

;---------------------------------------------------------------	
;Perform 32 bit add by adding 4 bytes. Implements ripple-carry of bits.

add32
	movf	operand_1,w		;get low part of <operand> 
	addwf	accumulator_1		;into w and add to accumulator
	btfsc	_c			;was there a carry?
	call	accumulator_ripple_2	;if so, do ripple carry

	movf	operand_2,w		;get next part of <operand> 
	addwf	accumulator_2		;into w and add to accumulator
	btfsc	_c			;was there a carry?
	call	accumulator_ripple_3	;if so, do ripple carry


	movf	operand_3,w		;get next part of <operand>
	addwf	accumulator_3		;into w and add to accumulator
	btfsc	_c			;was there a carry?
	call	accumulator_ripple_4	;if so, do ripple carry

	movf	operand_4,w		;get next part of <operand>
	addwf	accumulator_4		;into w and add to accumulator
	btfsc	_c
	bsf	_mflag_overflow		;flag overflow!
	return

;called ripple-carry functions used by add32. 
;Sets _mflag_overflow on overflow of MSB

accumulator_ripple_2		;do ripple carry
	incf	accumulator_2	;via increment
	btfss	_z		;if result is NOT zero
	return			;then ripple carry is done.

accumulator_ripple_3		;otherwise ripply carry next...
	incf	accumulator_3
	btfss	_z
	return			;on non-zero result we are done.

accumulator_ripple_4		;otherwise do last ripple carry...
	incf	accumulator_4
	btfsc	_z
	bsf	_mflag_overflow	;Set _mflag_overflow if accumulator_4
	return			;wrapped around to zero because of 
				;increment. This reports overflow

;END ADD32 ADD32 ADD32 ADD32 ADD32 ADD32 ADD32 ADD32 ADD32 ADD32
;***************************************************************

;***************************************************************
;BEGIN SUBTRACT32 SUBTRACT32 SUBTRACT32 SUBTRACT32 SUBTRACT32 									


;NAME:		subtract32

;FUNCTION:	<accumulator> = <accumulator> - <operand>.

;ENTER WITH:	<operand> and  <accumulator> loaded.

;RETURNS:	<accumulator> = <accumulator> - <operand>.
;		_mflag_overflow is SET if there is 
;		<accumulator> overflow.

;CHANGED:	<operand> is now negative of original <operand>

;NOTES:		divide32 does NOT call this routine

;		This routine allows "sequential adds/subtracts" 
;		to the <accumulator> from the <operand> set.
;	example: <accumulator> = (((A + B) - C) + D)
;		also, since result of multiply32 is returned 
;		in <accumulator>, you can do things like
;		<accumulator> = (((A * B) - C ) + D)
;		Calling program is responsible for first clearing 
;		_mflag_overflow if the overflow detection feature is 
;		to be used. (For example, multiply32 routine 
;		does this automatically). 
;		If you call subtract32 directly, YOU must clear
;		_mflag_overflow. The same applies if you call add32.

;CALLS:		neg32/add32	

;---------------------------------------------------------------	

subtract32	;subtraction is done via addition of 
		;negative number in <operand>

		call	neg32	;create twos complement of operand
		call	add32	;now add negative number to accumulator
		return		;return 
				;(with _mflag_overflow set on overflow).

;END  SUBTRACT32 SUBTRACT32 SUBTRACT32 SUBTRACT32 SUBTRACT32 
;***************************************************************

;***************************************************************
;BEGIN MULTIPLY32 MULTIPLY32 MULTIPLY32 MULTIPLY32 MULTIPLY32 

;---------------------------------------------------------------	
;BEGINNING OF EXPLANATION OF MULTIPLY ALGORITHM

;Multiplication is actually a set of repeated additions. In school 
;you were probably taught to multiply the operand (the top number)
;by the multiplier (the bottom number) starting with the rightmost
;digit of the multiplier. Then underneath that result line you wrote
;down a place holding zero and then multiplied the operand by the 
;second digit of the multiplier. For the third digit of the multiplier
;you put down two place holding zeros and then multiplied the third
;digit times the operand. You repeated this process for each digit 
;in the multiplier. When done, you added up all the result lines to 
;obtain the final answer.
;
;When multiplying in binary we make a few simplifications. First, the
;only digits involved are 0 and 1. So, the ONLY possible combinations
;are: 0*0=0  0*1=0  1*0=0  1*1=1  which is all very simple stuff.

;Assume that the first digit is the multiplier digit. You will notice
;that whenever the multiplier is 0 the result is 0. Further, 
;whenever the multiplier is 1 the result is the same as the operand.
;SUPER Simple stuff.

;Instead of shifting the result, we can shift the operand. It has the
;same effect, but simplifies things for us. So, when multiplying
;we first shift the operand left once (EXCEPT on the FIRST multiply
;bit!), then, IF the multiplier bit is "1", we ADD the current
;operand to the accumulator. (It is called the accumulator because
;it accumulates the result). By doing the addition immediately we 
;simplify the addition of the multiple lines that would normally 
;pile up (especially with 32 bit numbers!)

;If the current multiplier bit was a zero we skip the addition part, 
;because 0 * anything = 0.

;Once we have done this process for each multiplier bit, we will have
;the product (<multiplier> * <operand>) in the accumulator.

;Note that the largest product you are allowed to generate is 31 bits
;wide. A product's bit width is equal to the sum of the multiplier
;bit width and the operand bit width. If you multiplied a 20 bit
;multiplier times a 15 bit operand you would get a 35 bit product
;which would NOT fit in the alloted 32 bits! By the way, the 31 bit
;limitation is so we can process negative numbers as well as positive
;numbers. If you are only dealing with positive numbers, then you
;can handle full 32 bit numbers. 


;END OF EXPLANATION OF MULTIPLY ALGORITHM
;---------------------------------------------------------------	

;NAME:		multiply32

;FUNCTION:	<accumulator> = <multiplier> * <operand>

;ENTER WITH:	<multiplier> and <operand> loaded.
;		(<accumulator> is cleared upon entry so there are 
;		no suprises). (See note below on multiply32plus if 
;		you do NOT want <accumulator> cleared)

;LIMITATIONS:	Largest accumulator and operand value is 
;		+/- 2,147,483,647 (31 bits), OR + 4,294,967,295 
;		(32 bits) if using unsigned binary numbers.

;		Note that the sum of the bits in multiplier and operand 
;		must be 31 or less when using signed math, and 32 or 
;		less when using unsigned math. For example, 
;		multiplying an 8 bit multiplier times a 30 bit operand 
;		would require a 38 bit register set. Since our register
;		sets are fixed at 32 bits, the _flag_overflow bit 
;		would be set.

;		If using multiply32plus, you must also ensure that 
;		the additionof the entry value in <accumulator> 
;		does not cause the final answer to overflow. 
;		_flag_overflow will report such errors.

;RETURNS:	<accumulator> =  <multiplier> * <operand>
;		_mflag_overflow is SET if there is <accumulator> 
;		overflow.

;CHANGED:	<accumulator> and _mflag_overflow

;TRASHED:	<operand>/<multiplier>

;MULTIPLY32PLUS	
;to allow the added functionality of being able to do
;<accumulator2> = <accumulator1> + (<multiplier2> * <operand2>)
;I have included an alternate entry point to the multiply32 
;function that does NOT clear the prior contents of the <accumulator> 
;before performing the multiplication. This function is called 
;multiply32plus. This allows you to effectively ADD the 
;multiplication resultto the PREVIOUS contents of the <accumulator>.

;For example: 
;<accumulator2> = (<operand1>+<accumulator1>) * <multiplier2>
;We get this "extra" function for free, by simply skipping the first
;part of multiply32 that clears <accumulator>.

;Note that you can also do a multiply32 or multiply32plus and FOLLOW 
;it with add32 or subtract32 and only have to enter the <operand>  
;for the second operation. 

;For example: 
;<accumulator2> = (<multiplier1> * <operand1>) + <operand2>

;------------------------------------------------------------
;BEGINNING OF PIC CODE TO IMPLEMENT MULTIPLY32 ALGORITHM



;CALLS:		lshift32/add32

multiply32
	;Entering HERE will give you the simple multiplication function:
	;<accumulator> = <multiplier> * <operand>

	clrf	accumulator_4	;Start with a clean <accumulator>
	clrf	accumulator_3
	clrf	accumulator_2
	clrf	accumulator_1	;Now <old accumulator> is all zeros.

multiply32plus	
	;Entering HERE will give you the expanded multiplication function:
	;<accumulator2> = <accumulator1> + (<multiplier2> * <operand2>)
	;where <accumulator1> is a value you pre-loaded into
	;the <accumulator>, or the result of a previous function that
	;left a result in the <accumulator>. For example,
	;add32/subtract32/multiply32 all place result in <accumulator>.

	;overflow is indicated by _mflag_overflow being set to 1

	clrf	mflag		;clear all math flags
				;(especially _mflag_overflow)
				;flag gets SET in add32 on overflow

	movlw	multiplier_1	;SET UP INDIRECT ADDRESSING
	movwf	fsr		;to handle 4 multiplier bytes

	goto	skipshift_bit0	;NO shift on bit0 of multiplier,
				;because bit 0 represents 
				;multiplication by 1. SPECIAL CASE!

multiply32_loop	
				;do 8 bits for EACH multiplier byte,
				;so we handle all 32 bytes eventually.

	call	lshift32	;left shift 32 bit <operand> 
				;(with carry in/out at each byte)
				;(except NO initial lshift on 
				;multiplier_1,0)
skipshift_bit0	
	btfsc	indf,0		;handle bit 0 of THIS <multiplier> bit
	call	add32		; if 1 then add to accumulator

	call	lshift32	;lshift 32 bit operand with carry in/out
	btfsc	indf,1		;handle bit 1
	call	add32		; if 1 then add to accumulator

	call	lshift32	;lshift 32 bit operand with carry in/out
	btfsc	indf,2		;handle bit 2
	call	add32		; if 1 then add to accumulator

	call	lshift32	;lshift 32 bit operand with carry in/out
	btfsc	indf,3		;handle bit 3
	call	add32		; if 1 then add to accumulator

	call	lshift32	;lshift 32 bit operand with carry in/out
	btfsc	indf,4		;handle bit 4
	call	add32		; if 1 then add to accumulator

	call	lshift32	;lshift 32 bit operand with carry in/out
	btfsc	indf,5		;handle bit 5
	call	add32		; if 1 then add to accumulator

	call	lshift32	;lshift 32 bit operand with carry in/out
	btfsc	indf,6		;handle bit 6
	call	add32		; if 1 then add to accumulator

	call	lshift32	;lshift 32 bit operand with carry in/out
	btfsc	indf,7		;handle bit 7
	call	add32		; if 1 then add to accumulator

	decf	fsr,f		;update fsr to point to next BYTE
	decf	fsr,w		;need it in w, too for endloop test
	sublw	multiplier_4-1	;done all 4 multiplier bytes?
	btfss	_z		; examine _z flag to find out.
	goto	multiply32_loop	;if not, do next multiplier set

	return			;when last byte was multiplier_4, 
				;we are done! _mflag_overflow 
				;reports any overflow.


;END MULTIPLY32 MULTIPLY32 MULTIPLY32 MULTIPLY32 MULTIPLY32 
;***********************************************************

;***********************************************************
;BEGIN DIVIDE32 DIVIDE32 DIVIDE32 DIVIDE32 DIVIDE32 DIVIDE32 									

;-----------------------------------------------------------
;BEGINNING OF VERBOSE DESCRIPTION OF DIVISION ALGORITHM


;Because of the relative complexity of the divide32 routine I am 
;first going to run through the basic division algorithm in 
;simple English. Then I will present the actual code I devised 
;to implement the division algorithm.

;Division is actually a set of repeated subtractions. The denominator 
;is subtracted from the numerator. The process can be speeded up
;considerably if we begin by subtracting the BIGGEST binary multiple
;of the denominator possible, and then continue attempting to 
;subtract the next biggest binary multiple possible. We repeat 
;these actions until the numerator is smaller than the denominator. 
;What is left is the remainder.

;In binary math a number can be divided by two simply by shifting 
;it to the right once. It can be multiplied by two simply by 
;shifting it to the left once.

;The division algorithm that I came up with begins by clearing all
;error flags and clearing the quotient to zero. I then check for 
;certain special entry conditions such as entry of negative values 
;or values that are 32 bits instead of 31 bit maximum length. I 
;also check for division by zero, which is not allowed. If I find 
;division by 1 I short circuit the process to speed things up. 
;On any error I set a error flags and return with quotient 
;holding zero value.

;If there were no input error conditions then we proceed.
;Denominator is made negative (so it can be used to perform 
;subtractions)

;I then enter a scanloop that shifts the entire denominator left 
;one bit at a time. scancounter keeps track of how many shifts 
;were necessary to bring the most significant "0" to bit position 31. 

;Upon exit from scanloop, scancounter contains the number of 
;passes that were made through scanloop..

;A subtractloop is now entered. At the top of the loop a 
;copy of the current numerator is saved in case we subtract when 
;we shouldn't. I add the negated form denominator to the 
;numerator (which is actually a subtraction). If result
;in numerator goes negative I recover the copy (that undoes 
;the subtraction). If subtraction was successful then a "1" is 
;placed at scancounter bit of quotient. That is how we generate 
;the quotient bit-by-bit. I mentioned the phrase "is placed 
;at scancounter bit of quotient." I use the fsr to
;indirectly address the proper byte of the 4 byte (32 bit) 
;quotient. The scancounter is used to generate a COMPUTED GOTO 
;via a jump table to process the proper byte/bit combination 
;of the quotient. Each iteration of the loop I shift the 
;denominator to the right and update the scancounter so we "move" 
;to the next "bit" to be processed. When scancounter rolls over 
;to 0xff we are all done. (rolling over here means zero 
;is decremented and the byte value becomed b'11111111' or hex 0xff).


;END OF VERBOSE DESCRIPTION OF DIVISION ALGORITHM
;----------------------------------------------------------
;----------------------------------------------------------
;BEGINNING OF PSEUDO-CODE DESCRIPTION OF DIVISION ALGORITHM

;DIVIDE32
;CALLING PROGRAM details:

;Load 32 bit NUMERATOR into <numerator> set.
;Load 32 bit denominator into <denominator> set.
;Call divide32
;answer is returned in <quotient> set.
;User can check error flags if necessary.


;DIVIDE32 short form algorithm:
;Clear all error flags.
;Clear quotient.
	
;If MSB of <numerator> is 1
;	then set negative_derror flag and return (must be positive).
;Endif
;
;Convert <denominator> in operand to negative form and 
;keep in <denominator>.
;
;If MSB of <denominator> is 0
;	set error flag and return (div by 0 undefined!).
;	(This also catches attempt to divide by a 
;	negative denominator)
;Endif
;
;Clear error flag set.
;
;If <denominator> is 00000000 00000000 00000000 00000001 (1)
;	copy <numerator> to <quotient>
;	clear <numerator> so <remainder> is 0
;	return (this does an effective short cut divide by 1)
;Endif
;
;Clear <scancounter> (It will hold bit position).
;
;SCANLOOP:
;	If MSB-1 of <denominator>is 0
;		break out of loop and goto SUBTRACTLOOP
;	Endif
;	Increment <scancounter>.
;	clear carry and left-shift <denominator>.
;	Goto SCANLOOP.
;END SCANLOOP
;
;(We now have MSB-1 of <denominator> aligned so it is largest 
;binary multiple of NEGATIVE value of original 
;denominator possible). (We want to subtract BIG pieces FIRST!)
;
;SUBTRACTLOOP:
;	Copy <numerator> into <goback> 
;	(allows recovery from bad subtraction)
;	Add (negform) <denominator> to <numerator>. 
;	(subtract multiple of denominator)
;
;	If MSB of <numerator> is 1, result was negative, so
;		copy <goback> back into <numerator>. 
;		(Back where we were).
;	Endif
;
;	If MSB of <numerator> was 0
;		place 1 at <scanpointer> bit of <quotient>.
;		(This records multiple of denominator).
;	Endif
;
;	Then set carry. Shift <denominator> right. 
;	(This shifts negative denominator)
;	Decrement <scancounter>.
;	If <scancounter>=0xff (Test scancounter bit 7 for 1)
;		then DONE, so RETURN. 
;	Endif
;	goto SUBTRACTLOOP
;END SUBTRACTLOOP
;
;
;END OF PSEUDO-CODE DESCRIPTION OF DIVISION ALGORITHM
;-----------------------------------------------------------
;-----------------------------------------------------------


;NAME:		divide32

;FUNCTION:	<quotient> = <numerator> / <denominator>
		;divide32 ONLY handles POSITIVE 31 bit values!

;ENTER WITH:	<numerator> and  <denominator> loaded.
;		BOTH must be POSITIVE. 31 bit max (MSB must be 0)
;		!!!(see divide32x for version that 
;		handles +/- numbers)

;RETURNS:	<quotient> = <numerator> / <denominator>
;		remainder is in <remainder> 
;		(remainder> is alias for <numerator>)
;		(See ERROR REPORTING for results when an 
;		error occurred)
;
;LIMITATIONS:
;		Largest input or result value is +2,147,483,647.
		;They MUST be positive!
		;!!!(see divide32x for extension to divide32 
		;that allows negative numbers)
;
;		Since routine uses a form of subtraction, 
;		numbers are limited to 31 bits maximum.

;ERROR REPORTING:
;		!!! you can ignore _mflag_overflow! It is never 
;		set by divide32. _mflag_derror flags ALL div errors. 
;		(Test this flag first). _mflag_dneg is set if negative 
;		<numerator> or <denominator> entered._mflag_dzero is 
;		set if <denominator> was 0 on entry. On any 
;		_mflag_derror, <quotient> returns as 0. 

;		On an error, input values MAY be changed.
;		ALWAYS check the _mflag_derror flag. If it is set,
;		then you can check _mflag_dneg and _mflag_dzero to 
;		determine the exact kind of error detected.
;

;CHANGED:	<quotient> contains answer.
;		<numerator> contains remainder
;		(numerator is aliased as <remainder> for convenience)
;		mflag bits may have been changed to reflect 
;		errors encountered.
;		
;TRASHED:	<operand>/<denominator>/<goback>

;ALIASES:	Aliases are used to make routine easier to follow.
;		<denominator> is alias for <operand>
;		<numerator> is alias for <accumulator>
;		<remainder> is also alias for <numerator>/<accumulator> 
;		AFTER division.

;CALLS:		add32/lshift32/neg32

		;The MAX number of cycles to do a complete
		;division in divide32 is about 2,800 cycles.
		;With a 20 Mhz clock this is 560 microseconds.

		;Minimum divide time is about 78 cycles due
		;to short-circuit evaluation of division
		;by 1 (which would otherwise have taken more
		;that 5,000 cycles!) 

		;The larger the denominator, the shorter
		;the time it takes to determine the <quotient>.


;----------------------------------------------------------------
	
;BEGINNING OF PIC CODE THAT IMPLEMENTS DIVISION32

;Here begins the actual divide32 for positive unsigned 31 bit values.

divide32

;Clear error flags.

	clrf	mflag		;clear ALL math flags.
	bsf	_mflag_derror	;assume there will be general_derror...

;Clear <quotient>

	clrf	quotient_4	;clear 4 bytes that comprise <quotient>
	clrf	quotient_3
	clrf	quotient_2
	clrf	quotient_1

	
;If MSB of <numerator> is 1, then set _mflag_dneg and return. 
;(numerator> must be positive).

	btfss	numerator_4,7	;bit 7 of numerator_4 is MSB of <numerator>
	goto	testfornegdiv	; if bit was 0 continue to testfornegdiv
	bsf	_mflag_dneg	; if bit was 1 then numerator was negative!
	return			;Report error! _mflag_derror is ALSO set.

testfornegdiv
;If MSB of <denominator> is 1, then set _mflag_dneg and return 
;(<denominator> must be positive).

	btfss	denominator_4,7	;bit 7 of denominator_4 is MSB
	goto	chkdivby1	; if bit was 0 continue to negatediv
	bsf	_mflag_dneg
	return			;_mflag_derror is ALSO set.



chkdivby1
	movf	denominator_4,w	;this section short circuits division by 1
	btfss	_z
	goto	negateit
	movf	denominator_3,w
	btfss	_z
	goto	negateit
	movf	denominator_2,w
	btfss	_z
	goto	negateit
	movf	denominator_1,w
	sublw	1
	btfss	_z
	goto	negateit

;if attempting to divide by 1, just copy <numerator> into <quotient>
;and then clear <numerator>, since <numerator> on exit should 
;contain the remainder (which in the case of x/1 is always 0)
;This helps speed up division by 1 tremendously.
divby1
	movf	numerator_4,w	;copy numerator into w
	movwf	quotient_4	;copy w (numerator) into quotient
	clrf	numerator_4	;clear numerator (which is now remainder)
	movf	numerator_3,w	;REPEAT for all 4 bytes (32 bits)
	movwf	quotient_3
	clrf	numerator_3
	movf	numerator_2,w
	movwf	quotient_2
	clrf	numerator_2
	movf	numerator_1,w
	movwf	quotient_1
	clrf	numerator_1
	clrf	mflag		;this is NOT an error.
	return			;short-circuit divide by one done. Return.

;Convert <denominator> (in operand) to negative form and 
;keep in <denominator>.

negateit
	call	neg32		;negate <denominator>

;Note: the negative of 0 is 0. So, if MSB is 0 after negation, then 
;original value was 0. If MSB of <denominator> is 0, 
;set _mflag_dzero and return (div by 0 undefined!).
 
	btfsc	denominator_4,7	;MSB of <denominator> 0?
	goto	clear_scan	; if MSB was 1 then all is OK so far... 
	bsf	_mflag_dzero	; if MSB was 0 it is divide by zero error.
	return			;_mflag_derror is ALSO set.
	

;Clear <scancounter> (It will hold bit position).

clear_scan
	clrf	mflag
	clrf	scancounter	;initialize scancounter.

;SCANLOOP:

scanloop

;If MSB-1 of <denominator>is 0, break out of scanloop.
;(since denominator is now in negative form, first bit that is 0
;indicates that previous bit was last 1 indicating negative value)

	btfss	denominator_4,6	;is MSB-1 (still) 1?
	goto	subtractloop	; if not, we do subtractloop!

;	Increment <scancounter>.

	incf	scancounter	;MSB-1 (still) 1; update scancounter
	bcf	_c		;clear carry for left shift
	call	lshift32	;left shift denominator (again).

;	Goto LOOP.

	goto	scanloop	;continue scanning until done.


;(We now have MSB-1 of <denominator> aligned so it is largest 
;binary multiple of negative value of original denominator possible)
;(We want to subtract BIGGEST pieces FIRST!)

;
;SUBTRACTLOOP:

subtractloop
	

;Copy <numerator> into <goback> (allows recovery from bad subtraction)
setgoback
	movf	numerator_4,w	;copy <numerator> into <goback>
	movwf	goback_4

	movf	numerator_3,w
	movwf	goback_3

	movf	numerator_2,w
	movwf	goback_2

	movf	numerator_1,w
	movwf	goback_1

;Add (negform) <denominator> to <numerator>. 
;(subtract multiple of denominator)

	call	add32		;effectively "subtract" current 
				;denominator multiple from numerator.

;	If MSB of <numerator> is 1, result was negative, so
;		copy <goback> back into <numerator>. (Back where we were).

	btfss	numerator_4,7	;If MSB is 1, GOBACK! (undo subtraction!)
	goto	msbwas0		;otherwise continue...
msbwas1
	movf	goback_4,w	;perform GOBACK.
	movwf	numerator_4

	movf	goback_3,w
	movwf	numerator_3

	movf	goback_2,w
	movwf	numerator_2

	movf	goback_1,w
	movwf	numerator_1

	goto	divshift	;if we did GOBACK, then no bit to set. 
				;skip to bottom of loop

msbwas0

;	Otherwise, if MSB of <numerator> was 0, place 1 at 
;		<scancounter> bit of <quotient>. 
;	(This records multiple of denominator).

;	(first determine if quotient_4,_3,_2, or _1. Set fsr accordingly)
;	if scancounter 0 to 7, then quotient_1 is used, etc.

wasquotient_1
	movf	scancounter,w
	sublw	d'7'		;subtract scancounter from 7
	btfss	_c		;_c set when w :lte: 7
	goto	wasquotient_2	;try next
	movlw	quotient_1	;it is quotient_1
	goto	divsetbit	;bits 2,1,0 of scancounter 
				;reveal bit position
wasquotient_2
	movf	scancounter,w	;recover scancounter
	sublw	d'15'		;subtract scancounter from 15
	btfss	_c		;_c will be set when w :lte: 15
	goto	wasquotient_3	;try next
	movlw	quotient_2	;it is quotient_2
	goto	divsetbit	;bits 2,1,0 of scancounter 
				;reveal bit position
wasquotient_3
	movf	scancounter,w	;recover scancounter
	sublw	d'23'		;subtract scancounter from 23
	btfss	_c		;_c will be set when w :lte: 23
	goto	wasquotient_4	;try next
	movlw	quotient_3	;it is quotient_3
	goto	divsetbit	;bits 2,1,0 of scancounter 
				;reveal bit position
wasquotient_4
	movlw	quotient_4	;it MUST be quotient_4 if we got here.



;!!! important! program code from divsetbit to divsetbit7
;!!! MUST all reside in a contiguous 256 byte region
;!!! otherwise it will not run properly. The following
;!!! conditional code ensures proper operation,

if high($+d'27')-high($)==0	;27 is (divsetbit7-divsetbit).
				;This is the section that
				;MUST be contiguous for 
				;proper addressing.

	;If the 27 byte region does not cross into another 
	;256 byte region then it is OK as-is and nothing 
	;has to be done.

else
	;If the 27 byte region would cross a 256 byte boundary, 
	;use org to fix it so it BEGINS at the NEXT 
	;256 byte boundary.

	org	(high($)+1)*d'256'	;org divsetbit to next boundary

endif	;End of conditional processing to insure no 
	;crossing of boundary by jump table

divsetbit
	movwf	fsr		;set up indirect addressing.
	movlw	high(divsetbit0) ;COMPUTED GOTO's need pclath defined
	movwf	pclath		;in terms of LOWEST address TARGET.
				;ALL other TARGETS must be in the
				;same 256 byte segment. 
				;(such as 0100-01ff).

				;If final target is in another segment
				;you can accomplish this by having an
				;intra-segment initial target "goto" 
				;be itself a "goto" to any target in 
				;the same 2k page. 

				;If the initial target of a computed 
				;goto in turn sets the page select bits,
				;then a subsequent call or goto can 
				;even cross page boundaries.

	movf	scancounter,w	;let's look at scancounter one more time.
	andlw	b'00000111'	;AND mask bits 2,1,0. 
				;w contains value 0-7.

	addwf	pcl,f		;based on bits 2,1,0 vector to 
	goto	divsetbit0	;proper routine via COMPUTED GOTOs
	goto	divsetbit1
	goto	divsetbit2
	goto	divsetbit3
	goto	divsetbit4
	goto	divsetbit5
	goto	divsetbit6
	goto	divsetbit7

divsetbit0			;Each divsetbitx routine sets 
	bsf	indf,0		;appropriate bit using bit addressing 
				;via computed goto.
	goto	divshift
divsetbit1
	bsf	indf,1
	goto	divshift
divsetbit2
	bsf	indf,2
	goto	divshift
divsetbit3
	bsf	indf,3
	goto	divshift
divsetbit4
	bsf	indf,4
	goto	divshift
divsetbit5
	bsf	indf,5
	goto	divshift
divsetbit6
	bsf	indf,6
	goto	divshift
divsetbit7			;Last Target address for Computed GOTOs
	bsf	indf,7		;if you reached here, it MUST be bit 7

;	Then set carry. Shift <denominator> right. 
;	(This shifts negative denominator)

divshift			;(skip to here on GOBACK)

	bsf	_c		;set carry and then shift 
	rrf	denominator_4,f	;denominator right 4 bytes
	rrf	denominator_3,f
	rrf	denominator_2,f
	rrf	denominator_1,f

;	Decrement <scancounter>.

	decf	scancounter,f

;	If <scancounter> wrapped to 0xff we are done, so return.

	btfss	scancounter,7	;check for bit 7 is adequate.
	goto	subtractloop	;If not all done, get back to work!
	bcf	_mflag_overflow	;clear any misleading overflow 
				;error flag. True overflow in this
				;routine never happens. 
				;Calling routine can totally 
				;ignore flag.

	return			:Whoopee! We is ALL done!

;	Otherwise goto SUBTRACTLOOP


;END OF PIC CODE THAT IMPLEMENTS THE DIVISION ALGORITHM
;-------------------------------------------------------
	
;END DIVIDE32 DIVIDE32 DIVIDE32 DIVIDE32 DIVIDE32 DIVIDE32
;*******************************************************
;*******************************************************
;BEGIN DIVIDE32X DIVIDE32X DIVIDE32X DIVIDE32X DIVIDE32X

;DIVIDE32X:
;This is a routine that allows the use of 31 bit NEGATIVE numbers 
;as input and output to the divide32 routine (as well as positive 
;31 bit numbers, of course). (31 value bits, 1 sign bit, 
;twos complement form).
;
;It first determines what the sign of the quotient should be, 
;based on the signs of the <numerator> and <denominator>.
;
;Then it converts any negative inputs to a 31 bit positive 
;form. Then it calls the regular divide32 program that only 
;handles positive 31 bit numbers, and determines the 
;positive-value <quotient>. Then if the <quotient> needs to 
;be negative, it replaces <quotient> with the negative of
;the <quotient>. The remainder is returned in <remainder>.
;<remainder> is ALWAYS returned as a positive number.

;If you are absolutely sure that you are using only positive 
;<numerator> and <denominator> values, then divide32 is a 
;better routine to use. THIS routine has an additional overhead 
;associated with it that divide32 does not have. 
;This makes divide32 faster.

;In fact, if you ALWAYS deal with division of positive 
;numbers, then you can leave this divide32x program out 
;entirely and save the code space.

;-----------------------------------------------------------------

;NAME:		divide32x

;FUNCTION:	<quotient> = <numerator> / <denominator>
		;it allows both positive and NEGATIVE input and output!

;ENTER WITH:	<numerator> and  <denominator> loaded.
;		Both must be 32 bit signed numbers
		;(MSB handles sign, which limits VALUE to 31 bits)

;RETURNS:	<quotient> = <numerator> / <denominator>
		;<quotient> is negative when signs require it to be so.
;		remainder is in <remainder> (alias for <numerator>)
		;<remainder> is always in positive form
;		(See ERROR REPORTING for more info)
;
;LIMITATIONS:
;		Largest input or result value is +/- 2,147,483,647.
;
;		Since routine uses a form of subtraction, numbers 
;		are limited to 31 bits maximum plus sign bit.

;ERROR REPORTING:
;		!!! you can ignore _mflag_overflow! It is never 
;		set by divide32. _mflag_derror is set if ANY error 
;		occured. (Test this flag first). 
;		_mflag_dzero is set if <denominator> was 0 on entry.
;
;		On any _mflag_derror, <quotient> returns as 0.
;		On an error, input values MAY be changed.
;		You can check the _mflag_derror flag -or- _mflag_dzero
;		to determine if there was a divide by zero error.
;
;		No other types of error are possible, since this routine
;		specifically allows 32 bit SIGNED binary +/- numbers.
;		It is up to the user to enforce entry of numbers in 32
;		bit twos complement notation.
;

;CHANGED:	<quotient> contains answer.
;		<numerator> contains remainder if operation was successful.
;		(numerator is also aliased as <remainder> for convenience)
;		mflag bits may have been changed to reflect 
;		errors encountered.
;		
;TRASHED:	<operand>/<denominator>/<goback>

;ALIASES:	Aliases are used to make routine easier to follow.
;		<denominator> is alias for <operand>
;		<numerator> is alias for <accumulator>
;		<remainder> is alias for <numerator>/<accumulator> 
;		AFTER division.

;CALLS:		add32/lshift32/neg32/divide32	
;-----------------------------------------------------------------

divide32x	;eXtended form of divide32 that allows 
		;31 bit signed numbers.
		;Largest input or result value is +/- 2,147,483,647.

		;clear isitneg, which is used for polarity 
		;determination

	clrf	isitneg

negdenominator			;if <denominator> is negative,
	btfss	denominator_4,7
	goto	negnumerator	;(if not negative. skip to next check)
				; increment isitneg
	incf	isitneg,f
				; negate <denominator>
	call	neg32
				;endif
negnumerator	
				;if <numerator> is negative,
	btfss	numerator_4,7
	goto	dodivide32 
				; copy <denominator> to <goback>
	movf	denominator_4,w
	movwf	goback_4
	movf	denominator_3,w
	movwf	goback_3
	movf	denominator_2,w
	movwf	goback_2
	movf	denominator_1,w
	movwf	goback_1
				; copy <numerator> to <denominator>
	movf	numerator_4,w
	movwf	denominator_4
	movf	numerator_3,w
	movwf	denominator_3
	movf	numerator_2,w
	movwf	denominator_2
	movf	numerator_1,w
	movwf	denominator_1
				; increment isitneg
	incf	isitneg,f
				; negate <denominator>
	call	neg32
				; copy <denominator> to <numerator>
	movf	denominator_4,w
	movwf	numerator_4
	movf	denominator_3,w
	movwf	numerator_3
	movf	denominator_2,w
	movwf	numerator_2
	movf	denominator_1,w
	movwf	numerator_1
				; copy <goback> to <denominator>
	movf	goback_4,w
	movwf	denominator_4
	movf	goback_3,w
	movwf	denominator_3
	movf	goback_2,w
	movwf	denominator_2
	movf	goback_1,w
	movwf	denominator_1
				;endif
dodivide32
				;call divide32
	call	divide32

				;if isitneg,0 = 1
	btfss	isitneg,0
	return
				; copy <quotient> to <denominator>
	movf	quotient_4,w
	movwf	denominator_4
	movf	quotient_3,w
	movwf	denominator_3
	movf	quotient_2,w
	movwf	denominator_2
	movf	quotient_1,w
	movwf	denominator_1
				; negate <denominator>
	call	neg32
				; copy <denominator> to <quotient>
	movf	denominator_4,w
	movwf	quotient_4
	movf	denominator_3,w
	movwf	quotient_3
	movf	denominator_2,w
	movwf	quotient_2
	movf	denominator_1,w
	movwf	quotient_1
			;endif
	return
		;return with <quotient> either + or -
		;<quotient> is in standard twos complement form.
		;return with <remainder> (always in + form).
		;(quotient_4,7 will be 1 if <quotient> is negative.)

;END DIVIDE32X DIVIDE32X DIVIDE32X DIVIDE32X DIVIDE32X 
;******************************************************

;END MATH MATH MATH MATH MATH MATH MATH MATH MATH MATH
;*****************************************************

	end	;END statement is here so program can be run
		;in stand-alone mode for testing purposes.
		;!!! REMOVE "end" statement when inserting
		;math module into your program


