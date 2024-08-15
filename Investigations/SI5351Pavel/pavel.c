#include <stdio.h>
#include <math.h>

/**
 This code calculates:
    R           The output divider limited to values 1, 2, 4, 8, 16, 32, 64, 128
    od          Multisynth output divider restricted to even integers
    a+b/c     The PLL's feedback fractional divider values
 for output frequencies fo from  1.8 to 150 mHz in 1 Hz increments and prints the largest errors (between
 desired and actual output) found.
**/


unsigned od, fo, b, c, fvco, fclk=25000000;
unsigned char a, R;

void setFreq(unsigned fo) {
       
//Find values for od and R
       R=1;
       od = 900000000 / fo;		//od value if we didn't use R

       while (od>128) { 
       	     R = R * 2;
	     od = od/2;
       }
       if (od%2)  od--;
       
} //calcOutputDividers()




int main() {

       fclk=25000000;			//CLKIN frequency

       float maxDiff=0.0;

       for(fo=1800000;fo<=150000000;fo+=1) {
	 setFreq(fo);
	 fvco = od * R * fo;

	 unsigned a = fvco / fclk;
	 unsigned b = (fvco % fclk) >> 5;
	 unsigned c = fclk >> 5;
	 float f = (fclk*((float)a+(float)b/(float)c)) / (od*R);
	 float diff = (float) fo - f;

	 if (diff>maxDiff) {
	   printf("fo=%u, od=%u, R=%u, fvco=%u",fo,od,R,fvco);
	   printf(", a=%u, b=%u, c=%u, f=%f, diff=%f\n",a,b,c,f,diff);
	   maxDiff=diff;
	 }
	 
       }

       	 printf("maxDiff=%f\n",maxDiff);




       return 0;
} //main()





       
       
