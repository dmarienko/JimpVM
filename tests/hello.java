import java.lang.*;

public class hello { 

  final static int    c0 = 1231;
  final static int    c1 = -2;
//  final float  c2 = 1;
//  final float  c3 = (float)12.31; 
 // float  cf = 777;


  final String c6 = "CONSTANT !!!";
  String c7 = "CONSTANT !!!";

  static int b, c=1;  static {
    b = 556; 
  }

  public static int TEST_ARRAYS() {    int x[] = new int[1024];
   int xx[][] = new int[7][7];
   String ss[] = new String[10];
   String sss[][][] = new String[2][10][10];

   int i, j;
   for(i=0;i<10;i++) x[i] = i+1;
   for(i=0;i<10;i++) JVKernel.kputs("  " + x[i]);
   JVKernel.kputs("\n"); 	    

   for(i=0;i<7;i++)
  	 for(j=0;j<7;j++) xx[i][j] = i*j; 

   for(i=0;i<7;i++) {
     for(j=0;j<7;j++){ 
	   JVKernel.kputs("  " + xx[i][j]); 
	 }
     JVKernel.kputs("\n"); 	 
   }
 
   JVKernel.kputs("\t\tStart test string arrays\n");   
   for(i=0;i<5;i++) ss[i] = "string" + i + "\n";
   for(i=0;i<5;i++) JVKernel.kputs(ss[i]);

   JVKernel.kputs("\t\tMULTI ARRAY TEST HERE\n");
   for(i=0;i<3;i++)
    for(j=0;j<3;j++) sss[1][i][j] = "multi array\n"; 

   for(i=0;i<3;i++)
     for(j=0;j<3;j++) JVKernel.kputs(sss[1][i][j]); 

    return 1;
  }

  public static void TEST_CAST() {   hello h1, h2;
   h1 = new hello(); 

   if( h1 instanceof hello ) 
     JVKernel.kputs("h1 is Object YES\n");    
   else JVKernel.kputs("--- Wrong instanceof:  h1 is not Object class ---\n");
   if( ((Object)h1) instanceof String ) 
     JVKernel.kputs("--- Wrong instanceof:  h1 is not String class ---\n");    
   else JVKernel.kputs("h1 is NOT a String instance: it's right !\n");   
  }

  public static void Logo() {
    String l[] = {
	    "\t _           _\n",
    	"\t  | . |   | |_|\n",
	    "\t \\  | | | | | \n",
	    null };
	int k = 0;
	while( l[k] != null ) JVKernel.kputs(l[k++]);
  }

  public static void main( String argv[] )  {
   int i;
   int res=0;
   AScreen as = new AScreen();

   JVKernel.kputs("\t\tStart\n");
    
   as.setBG(0);
   as.setFG(7);
   
   Logo();
 
    TEST_ARRAYS();

    TEST_CAST();

    JVKernel.kputs("\t\tTest argv[]\n");

    i = 0;
    for(i=0;i<argv.length;i++) 
	  JVKernel.kputs(argv[i] + "\n");

    as.setBG(0);
	as.setFG(6);

    Logo();

    res = 0;
    for(i=0;i<100;i++) res += i*i;
    JVKernel.kputs("Math test = " + res + "\n" );
  }
}

