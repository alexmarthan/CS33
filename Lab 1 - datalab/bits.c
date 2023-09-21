/*
 * CS:APP Data Lab
 *
 * Alexandre Marthan, 505719814
 *
 * bits.c - Source file with your solutions to the Lab.
 *          This is the file you will hand in to your instructor.
 *
 * WARNING: Do not include the <stdio.h> header; it confuses the dlc
 * compiler. You can still use printf for debugging without including
 * <stdio.h>, although you might get a compiler warning. In general,
 * it's not good practice to ignore compiler warnings, but in this
 * case it's OK.
 */


/*
 * TMax - return maximum two's complement integer
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 4
 *   Rating: 1
 */
int tmax(void) {
  
  return ~(1 << 31);
}

/*
 * isZero - returns 1 if x == 0, and 0 otherwise
 *   Examples: isZero(5) = 0, isZero(0) = 1
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 2
 *   Rating: 1
 */
int isZero(int x) {
  return !x; // if 0 will make 1, if anything else will make 0
}

/*
 * bitXor - x^y using only ~ and &
 *   Example: bitXor(4, 5) = 1
 *   Legal ops: ~ &
 *   Max ops: 14
 *   Rating: 1
 */
int bitXor(int x, int y) {
  int a = x & y;
  int b = ~x & ~y;
  int answer = ~a & ~b;
  return answer;
}

/*
 * isNotEqual - return 0 if x == y, and 1 otherwise
 *   Examples: isNotEqual(5,5) = 0, isNotEqual(4,5) = 1
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 6
 *   Rating: 2
 */
int isNotEqual(int x, int y) {
  return !!(x^y); // double exclamation is to convert answer to either 0 or 1
}
/*
 * sign - return 1 if positive, 0 if zero, and -1 if negative
 *  Examples: sign(130) = 1
 *            sign(-23) = -1
 *  Legal ops: ! ~ & ^ | + << >>
 *  Max ops: 10
 *  Rating: 2
 */
int sign(int x) {
    int sign = (!!x) | (x >> 31);
    return sign;
}
/*
 * leastBitPos - return a mask that marks the position of the
 *               least significant 1 bit. If x == 0, return 0
 *   Example: leastBitPos(96) = 0x20
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 6
 *   Rating: 2
 */
int leastBitPos(int x) {

  return x & (~x+1);
}

/*
 * conditional - same as x ? y : z
 *   Example: conditional(2,4,5) = 4
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 16
 *   Rating: 3
 */
int conditional(int x, int y, int z) {
  int booleanConvert = !!x; // make sure x is either 1 or zero
  int mask = booleanConvert << 31; // left shift rises the lsb to the top, making it the msb
  int realMask = mask >> 31; // right shifting makes it be all 1s or all zeroes
  return ((realMask & y) | (~realMask & z));
}

/*
 * replaceByte(x,n,c) - Replace byte n in x with c
 *   Bytes numbered from 0 (LSB) to 3 (MSB)
 *   Examples: replaceByte(0x12345678,1,0xab) = 0x1234ab78
 *   You can assume 0 <= n <= 3 and 0 <= c <= 255
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 10
 *   Rating: 3
 */
int replaceByte(int x, int n, int c) {
  int shifted = c << (n << 3); // shift the byte we are inserting by appropriate nb of bits (8*n)
  int mask = 0xff << (n << 3); // create a mask that zeroes everything but the bytes to be replaced
  int clearByte = x & ~mask; // this zeroes out all bits of chosen byte in x
  return clearByte | shifted; // Or operation makes the shifted byte appear in x
}

/*
 * isAsciiDigit - return 1 if 0x30 <= x <= 0x39 (ASCII codes for characters '0' to '9')
 *   Example: isAsciiDigit(0x35) = 1.
 *            isAsciiDigit(0x3a) = 0.
 *            isAsciiDigit(0x05) = 0.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 15
 *   Rating: 3
 */

// int isAsciiDigit(int x) {
//   // if one of the values within range is equal to x, one of the vars will have value 1
//   int is30 = !(x^0x30);
//   int is31 = !(x^0x31);
//   int is32 = !(x^0x32);
//   int is33 = !(x^0x33);
//   int is34 = !(x^0x34);
//   int is35 = !(x^0x35);
//   int is36 = !(x^0x36);
//   int is37 = !(x^0x37);
//   int is38 = !(x^0x38);
//   int is39 = !(x^0x39);

//   return is30 | is31 | is32 | is33 | is34 | is35 | is36 | is37| is38| is39;
// }

int isAsciiDigit(int x) {
    int Hex3 = !((x >> 4) ^ 0x3); //if it returns zero then it is not ascii digit (must be hex3)
    int bit4 = ((x >> 3) & 1); //if 4th bit from left's value is 1 this will be equal to 1
    int bit23 = !!((x >> 1) & 0x3); //if this returns 1 (and bit4 is 1), then not within range (bit 2 and 3 mustn't be ones if bit 4 is one)

    return (Hex3 & !(bit4 & bit23));
 }
/*
 * isGreater - if x > y  then return 1, else return 0
 *   Example: isGreater(4,5) = 0, isGreater(5,4) = 1
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 24
 *   Rating: 3
 */
int isGreater(int x, int y) {

  // the following vars will have value 0 or -1
  int signOfX = x >> 31;
  int signOfY = y >> 31;

  int sameSign = !(signOfX ^ signOfY); // will equal 1 if same sign, 0 if different signs

  // idea here is that x + ~y computes which one is larger, if x and y have same sign
  // else, negative nb is smaller
  // x + ~y is 0 if x larger, 1 if y larger (unless opposing signs)
  int isLarger = ((x + ~y) >> 31) & sameSign; // if opposing signs, sameSign makes var equal to 0
  int opposingSigns = signOfX & !signOfY; // if we have differently signed numbers, this tells us which one is negative
  return !(isLarger | opposingSigns);
}

/*
 * logicalNeg - implement the ! operator, using all of
 *              the legal operators except !
 *   Examples: logicalNeg(3) = 0, logicalNeg(0) = 1
 *   Legal ops: ~ & ^ | + << >>
 *   Max ops: 12
 *   Rating: 4
 */
int logicalNeg(int x) {
  // here we make use of the property that the Two's complement of zero is zero
  // thus the or statement will always result in zero
  // in the other case, the or statement will be: 0 or -1
  int signOfX = x >> 31;
  int signOfTwosComplement = (~x + 1) >> 31;
  return (signOfX | signOfTwosComplement) + 1;
  }

/*
 * greatestBitPos - return a mask that marks the position of the
 *               most significant 1 bit. If x == 0, return 0
 *   Example: greatestBitPos(96) = 0x40
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 70
 *   Rating: 4
 */
int greatestBitPos(int x) {
  
   // Combine x with its right-shifted version to set all bits following the highest set bit to 1
   x |= (x >> 1);
   x |= (x >> 2);
   x |= (x >> 4);
   x |= (x >> 8);
   x |= (x >> 16);

   // XOR x with its right-shifted version to make all bits except the highest set bit zero
   // if msb was greatest bit, we make a special case (second part of or statement)
   return ((x ^ (x >> 1)) | (1 << 31)) & x;
}



 
