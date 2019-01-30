/*
* ========================================================================    =============
*       Filename:  hello.c
*
*    Description: A simulated carry lookahead adder that sums up two 1024 bit hex numbers.
*                   Input must be a txt file with each 1024 bit hex number on its own line. 
*                   The sum is then printed with a leading '0' appended.
*                   The input is first converted to binary in the form of 
*                    an int[]. The operations are performed on said array. The sum
*                    in binary is then converted back to hex and printed out.
*            
*        Version:  1.0
*        Created:  01/30/2019 01:36:53 PM
*       Revision:  none
*       Compiler:  gcc
*
*         Author:  Sam Fite
*   Organization:  RPI
* ========================================================================    =============
*/

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <ctype.h>
#include <math.h>
#include <string.h>

#define input_size 1024
#define block_size 8

/*  do not touch these defines */
#define digits (input_size + 1)/* 1025 */
#define bits (input_size * 4)/* 4096 */
#define ngroups bits/block_size/* 512 */
#define nsections ngroups/block_size/* 64 */
#define nsupersections nsections/block_size/* 8 */

/*  GLOBAL DEFINITIONS of arrays to be used in steps for easy access */
/* Initial carry. Can be used for later versions i.e set to 1 
 * for subtraction.*/
int c = 0; 
/* Generate, propogate, carry */
int gi[bits] = {0};
int pi[bits] = {0};
int ci[bits] = {0};

/* Group: generate, propogate, carry */
int ggj[ngroups] = {0};
int gpj[ngroups] = {0};
int gcj[ngroups] = {0};

/* Section: generate, propogate, carry */
int sgk[nsections] = {0};
int spk[nsections] = {0};
int sck[nsections] = {0};
 
/* Super section: generate, propogate, carry */
int ssgl[nsupersections] = {0};
int sspl[nsupersections] = {0};
int sscl[nsupersections] = {0};

int sum_bin[bits] = {0}; /* sum in binary form */
char sum_hex[digits] = {0}; /* sum in hex form */

/* Int array of inputs in binary form (little endian) */
/* bin[0] represents 2^0. Big endian format can 
 * be acheived via print_bin_reverse(bin) */
int* bin1=NULL;
int* bin2=NULL;

/* Char array of input in hex form */
char* hex1=NULL;
char* hex2=NULL;

/*=====CLA FUNCTIONS START HERE=====*/
/* https://www.cs.rpi.edu/~chrisc/COURSES/PARALLEL/SPRING-2019/papers/cla.pdf
 * ^^ See above PDF for detailed explanation of calculation process ^^ */

/* Calculates generate and propogate for each bit of the two bin numbers to be
 * added */
void generate_propogate()
{
    int i;
    for( i = 0; i < bits; i++ ) 
    {
        gi[i] = bin1[i] & bin2[i];
        pi[i] = bin1[i] | bin2[i];
    }
}

/* Calculates generate and propogate for each 8 bit group(block) */
void group_g_p()
{
    int i,j;
    for( j = 0; j < ngroups; j++ )
    {
        i = block_size * j;  
        ggj[j] = gi[i+7]
            | (pi[i+7] & gi[i+6])
            | (pi[i+7] & pi[i+6] & gi[i+5])
            | (pi[i+7] & pi[i+6] & pi[i+5] & gi[i+4])
            | (pi[i+7] & pi[i+6] & pi[i+5] & pi[i+4] & gi[i+3])
            | (pi[i+7] & pi[i+6] & pi[i+5] & pi[i+4] & pi[i+3] & gi[i+2])
            | (pi[i+7] & pi[i+6] & pi[i+5] & pi[i+4] & pi[i+3] & pi[i+2] & gi[i+1])
            | (pi[i+7] & pi[i+6] & pi[i+5] & pi[i+4] & pi[i+3] & pi[i+2] & pi[i+1] & gi[i]);
        gpj[j] =  pi[i+7] & pi[i+6] & pi[i+5] & pi[i+4] & pi[i+3] & pi[i+2] & pi[i+1] & pi[i];           
    }
}

/* Calculates generate and propogate for each 8 group section */
void section_g_p()
{
    int k, j;
    for( k = 0; k < nsections; k++ )
    {
        j = 8 * k;
        sgk[k] = ggj[j+7]
            | (gpj[j+7] & ggj[j+6])
            | (gpj[j+7] & gpj[j+6] & ggj[j+5])
            | (gpj[j+7] & gpj[j+6] & gpj[j+5] & ggj[j+4])
            | (gpj[j+7] & gpj[j+6] & gpj[j+5] & gpj[j+4] & ggj[j+3])
            | (gpj[j+7] & gpj[j+6] & gpj[j+5] & gpj[j+4] & gpj[j+3] & ggj[j+2])
            | (gpj[j+7] & gpj[j+6] & gpj[j+5] & gpj[j+4] & gpj[j+3] & gpj[j+2] & ggj[j+1])
            | (gpj[j+7] & gpj[j+6] & gpj[j+5] & gpj[j+4] & gpj[j+3] & gpj[j+2] & gpj[j+1] & ggj[j]);
        spk[k] =  gpj[j+7] & gpj[j+6] & gpj[j+5] & gpj[j+4] & gpj[j+3] & gpj[j+2] & gpj[j+1] & gpj[j];           
    }
}

/* Calculates generate and propogate for each 8 section super section */
void super_section_g_p()
{
    int l, k;
    for( l = 0; l < nsupersections; l++ )
    {
        k = 8 * l;
        ssgl[l] = sgk[k+7]
            | (spk[k+7] & sgk[k+6])
            | (spk[k+7] & spk[k+6] & sgk[k+5])
            | (spk[k+7] & spk[k+6] & spk[k+5] & sgk[k+4])
            | (spk[k+7] & spk[k+6] & spk[k+5] & spk[k+4] & sgk[k+3])
            | (spk[k+7] & spk[k+6] & spk[k+5] & spk[k+4] & spk[k+3] & sgk[k+2])
            | (spk[k+7] & spk[k+6] & spk[k+5] & spk[k+4] & spk[k+3] & spk[k+2] & sgk[k+1])
            | (spk[k+7] & spk[k+6] & spk[k+5] & spk[k+4] & spk[k+3] & spk[k+2] & spk[k+1] & sgk[k]);
        sspl[l] =  spk[k+7] & spk[k+6] & spk[k+5] & spk[k+4] & spk[k+3] & spk[k+2] & spk[k+1] & spk[k];           
    }
}

/* Calculates the carry for each super section */
void super_section_carry()
{
    int l;
    sscl[0] = ssgl[0] | (sspl[0] & c);
    for( l = 1; l < nsupersections; l++ )
    {
        sscl[l] = ssgl[l] | (sspl[l] & sscl[l-1]);
    }
}
  
/* Calculates the carry for each section */
void section_carry()
{
    int k;
    sck[0] = sgk[0] | ( spk[0] & c ); 
    for( k = 1; k < nsections; k++ )
    {
        /* if we are at the start of a new super section */
        if( k % 8 == 0 )
        {
            sck[k] = sgk[k] | ( spk[k] & sscl[(k/8)-1] );
        }
        else
        {
            sck[k] = sgk[k] | ( spk[k] & sck[k-1] );
        }
    }
}

/* Calculates the carry for each group */
void group_carry()
{
    int j;
    gcj[0] = ggj[0] | ( gpj[0] & c );
    for( j = 1; j < ngroups; j++ )
    { 
       /* If we are at the start of a new section */
       if( j % 8 == 0 )
       {
           gcj[j] = ggj[j] | ( gpj[j] & sck[(j/8) - 1] );
       }
       else
       {
           gcj[j] = ggj[j] | ( gpj[j] & gcj[j-1] );
       }
    }
}

/* Calculates the carry for each bit in bin inputs */
void carry()
{
    int i;
    ci[0] = gi[0] | ( pi[0] & c );
    for( i = 1; i < bits; i++ )
    {
        /* If we are at the start of a new group */
        if( i % 8 == 0 )
        {
            ci[i] = gi[i] | ( pi[i] & gcj[(i/8) - 1] );
        }
        else
        {
            ci[i] = gi[i] | ( pi[i] & ci[i-1] );
        }
    }
}
/*=====CLA FUNCTIONS END HERE=====*/

/* Prints the sum of the two input numbers in hex: big endian format */
void print_sum_hex()
{
    int i;
    /* Preappend a '0' for formatting purposes */
    printf("%c",'0');
    for( i = 1; i <= input_size; i++ )
    {
        /* sum_hex[0] = 16^0 (little endian), i.e print 'backwards' for
         * big endian*/
        printf("%c", sum_hex[input_size-i]);
    }
    printf("\n");
}

/* prints the binary string from bin[4095] to bin[0], BIG ENDIAN */
void print_bin_reverse(int* bin)
{
    int i;
    printf("---------BIG ENDIAN BIN---------\n");
    for ( i = 1; i <= bits; i++ )
    {
        printf("%d", bin[bits - i]);
    }
    printf("\n-----------------------------------\n");
}

/* prints the binary string from bin[0] to bin[4095], LITTLE ENDIAN */
void print_bin(int* bin)
{
    int i;
    printf("---------LITTLE ENDIAN BIN---------\n");
    for ( i = 0; i < bits; i++ )
    {
        printf("%d", bin[i]);
    }
    printf("\n-----------------------------------\n");
}

/* given a hex char and an int array to store return value, populates return
 * array with the corrosponding binary represenation of the hex char */
void bin_from_hex_digit(char c, int* ret)
{
    if (c == '0')
    {
        ret[0] = 0; ret[1] = 0; ret[2] = 0; ret[3] = 0;
    }
    else if (c == '1')
    {
        ret[0] = 0; ret[1] = 0; ret[2] = 0; ret[3] = 1;
    }
    else if (c == '2')
    {
        ret[0] = 0; ret[1] = 0; ret[2] = 1; ret[3] = 0;
    }
    else if (c == '3')
    {
        ret[0] = 0; ret[1] = 0; ret[2] = 1; ret[3] = 1;
    }
    else if (c == '4')
    {
        ret[0] = 0; ret[1] = 1; ret[2] = 0; ret[3] = 0;
    }
    else if (c == '5')
    {
        ret[0] = 0; ret[1] = 1; ret[2] = 0; ret[3] = 1;
    }
    else if (c == '6')
    {
        ret[0] = 0; ret[1] = 1; ret[2] = 1; ret[3] = 0;
    }
    else if (c == '7')
    {
        ret[0] = 0; ret[1] = 1; ret[2] = 1; ret[3] = 1;
    }
    else if (c == '8')
    {
        ret[0] = 1; ret[1] = 0; ret[2] = 0; ret[3] = 0;
    }
    else if (c == '9')
    {
        ret[0] = 1; ret[1] = 0; ret[2] = 0; ret[3] = 1;
    }
    else if (c == 'A')
    {
        ret[0] = 1; ret[1] = 0; ret[2] = 1; ret[3] = 0;
    }
    else if (c == 'B')
    {
        ret[0] = 1; ret[1] = 0; ret[2] = 1; ret[3] = 1;
    }
    else if (c == 'C')
    {
        ret[0] = 1; ret[1] = 1; ret[2] = 0; ret[3] = 0;
    }
    else if (c == 'D')
    {
        ret[0] = 1; ret[1] = 1; ret[2] = 0; ret[3] = 1;
    }
    else if (c == 'E')
    {
        ret[0] = 1; ret[1] = 1; ret[2] = 1; ret[3] = 0;
    }
    else if (c == 'F')
    {
        ret[0] = 1; ret[1] = 1; ret[2] = 1; ret[3] = 1;
    }
    else
    {
        fprintf( stderr, "ERROR: could not find hex number\n");
        exit(EXIT_FAILURE);
    }
}

/* compares int arrays of size 4, returns 0 if equal, -1 otherwise*/
int arrcmp(int* a, int* b)
{
    int i;
    for( i = 0; i < 4; i++ )
    {
        if( a[i] != b[i] )
        {
            return -1;
        }
    }
    return 0;
}

/* given an int array of size 4 representing a binary number (big endian),
 * returns the appropriate hex char*/
char hex_from_bin_digit(int* c)
{
    int zero[4] =     { 0, 0, 0, 0 };
    int one[4] =      { 0, 0, 0, 1 };
    int two[4] =      { 0, 0, 1, 0 };
    int three[4] =    { 0, 0, 1, 1 };
    int four[4] =     { 0, 1, 0, 0 };
    int five[4] =     { 0, 1, 0, 1 };
    int six[4] =      { 0, 1, 1, 0 };
    int seven[4] =    { 0, 1, 1, 1 };
    int eight[4] =    { 1, 0, 0, 0 };
    int nine[4] =     { 1, 0, 0, 1 };
    int ten[4] =      { 1, 0, 1, 0 };
    int eleven[4] =   { 1, 0, 1, 1 };
    int twelve[4] =   { 1, 1, 0, 0 };
    int thirteen[4] = { 1, 1, 0, 1 };
    int fourteen[4] = { 1, 1, 1, 0 };
    int fifteen[4] =  { 1, 1, 1, 1 };
    /* If the given int[] is the same as predefined binary representation */
    if ( arrcmp(c, zero) == 0)
        return '0';
    if ( arrcmp(c, one) == 0)
        return '1';
    if ( arrcmp(c, two) == 0)
        return '2';
    if ( arrcmp(c, three) == 0)
        return '3';
    if ( arrcmp(c, four) == 0)
        return '4';
    if ( arrcmp(c, five) == 0)
        return '5';
    if ( arrcmp(c, six) == 0)
        return '6';
    if ( arrcmp(c, seven) == 0)
        return '7';
    if ( arrcmp(c, eight) == 0)
        return '8';
    if ( arrcmp(c, nine) == 0)
        return '9';
    if ( arrcmp(c, ten) == 0)
        return 'A';
    if ( arrcmp(c, eleven) == 0)
        return 'B';
    if ( arrcmp(c, twelve) == 0)
        return 'C';
    if ( arrcmp(c, thirteen) == 0)
        return 'D';
    if ( arrcmp(c, fourteen) == 0)
        return 'E';
    if ( arrcmp(c, fifteen) == 0)
        return 'F';
    fprintf( stderr, "ERROR: could not find bin number\n");
    exit(EXIT_FAILURE);
}

/*  takes in a binary number (little endian) and converts it to hex (big endian) */
void bin_to_hex_sum(int* bin)
{
    int i, j;
    for( i = 0; i < input_size; i++ )
    {
        int tmp[4];
        for( j = 0; j < 4; j++ )
        {
            /* need tmp in right endian for conversion, bin is little endian */
            tmp[3-j] = bin[(4*i)+j];
        }
        sum_hex[i] = hex_from_bin_digit(tmp);
    }
}

/* converts a hex number in the form char[] to a binary number in the form of an int[] */
void hex_to_bin(char* hex, int* bin)
{
    /* Array to hold binary representation of hex digit */
    int tmp[4];
    int i,j;
    /*  populates the binary array in reverse order with the 4 bits given
     *  by bin_from_hex_digit. i needs to start at 0 because of how we
     *  calculate the index of bin[]*/
    for( i = 0; i < input_size; i++)
    {
        /* hex is big endian so start at last index, tmp is returned in big endian
         * format*/
        bin_from_hex_digit(hex[input_size-(i+1)], tmp);
        for( j = 0; j < 4; j++ )
        {
            /* store bin as little endian, thus need to read tmp in reverse order*/
            bin[(4*i)+j] = tmp[3-j];
        }
    }
}

/* Adds the two binary arrays together and stores result in sum_bin*/
void sum_bins()
{
    int i;
    sum_bin[0] = bin1[0] ^ bin2[0] ^ c;
    for( i = 1; i < bits; i++ )
    {
        sum_bin[i] = bin1[i] ^ bin2[i] ^ ci[i-1];
    }
}

/* Reads two 1024 bit hex numbers from file and populates hex1 and hex2 */
void read_input()
{
    /* Allocate enough space for the leading '0' at the end */
    hex1 = (char*)calloc(digits+1, sizeof(char));
    hex2 = (char*)calloc(digits+1, sizeof(char));
    int elems1, elems2;
    elems1 = scanf("%s", hex1);
    elems2 = scanf("%s", hex2);
    if( elems1 != 1 || elems2 != 1 || elems1 == EOF || elems2 == EOF)
    {
        fprintf( stderr, "ERROR: Reading input from file was unsuccessful\n");
        exit(EXIT_FAILURE);
    }
}

/* Master calculation function called by main*/
void cla()
{
    bin1 = (int*)calloc(bits, sizeof(int));
    bin2 = (int*)calloc(bits, sizeof(int));
    /* convert input and store binary representation in appropraite arrays */
    hex_to_bin(hex1, bin1);
    hex_to_bin(hex2, bin2);

    /* call functions to perform CLA operations in correct order */
    generate_propogate();
    /*  g_p stands for generate and propogate */
    group_g_p();
    section_g_p();
    super_section_g_p();

    super_section_carry();
    section_carry();
    group_carry();
    carry();
    sum_bins();
}

int main(int argc, char* argv[]) 
{
    if ( argc != 1 ) 
    {
        fprintf( stderr, "ERROR: usage is: \"./a.out < input.txt\" \n");
        return EXIT_FAILURE;
    }
    /* populate hex1 and hex2 with input */
    read_input();
    /* run cla function to do calculations */
    cla();
    /* convert our sum from binary to hex */
    bin_to_hex_sum(sum_bin);
    /* print out the sum in hex form */
    print_sum_hex();
    /* you can use print_bin and print_bin_reverse 
     * to see the input/output in binary */
    //print_bin_reverse

    /* free up dynamically allocated memory */
    free(bin1);
    free(bin2);
    free(hex1);
    free(hex2);
    return EXIT_SUCCESS;   

}
