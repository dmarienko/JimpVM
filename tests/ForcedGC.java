import java.lang.*;
//import java.util.*;


class InnerObject {
  private Object o1 = new Object();
  int [] N; 
  private int n;
  String  s;
  public  InnerObject(String s){ this.s = new String(s); }
  public  InnerObject(int i){ n = i; N = new int[100]; N[0] = n;  }
  public void test(){JVKernel.kputs("After all garbage sweeping I'm steel alive !!!\n"); }
  public void test2() { JVKernel.kputs("."); }
}


/**
 * Test class 'ForcedGC.java' created Thu, 28 Jul 2005 17:34:38 +0300
 */
public class ForcedGC {
  InnerObject inner = new InnerObject(0);
  Object [] a1;
  
  public ForcedGC(){
   JVKernel.kputs("-> constructor\n");    
  }

  
  void test1(){
	JVKernel.kputs("testing creating and collecting array of objects ...\n");
 	for(int i=0;i<40000;i++){
	  a1 = new Object[100];
	  for(int j=0;j<100;j++) a1[j] = new Object();
	  if((i%500)==0) JVKernel.kputs("*"+i+"*\n");	  
	}
   	JVKernel.kputs("--ok--\n");
  }


  void test2(){
	JVKernel.kputs("testing creating and collecting StringBuffer objects ...\n");
	StringBuffer sb;
	for(int i=0;i<10000;i++){
	  sb = new StringBuffer();
	  sb.append("index = ");
	  sb.append(i);
	  sb.append(" ");
  	  sb.append("\n");
	  JVKernel.kputs(".");
	  int [] x = new int[100];
	}
   	JVKernel.kputs("done\n");
  }
  
  
  void test(String [] args){
 	JVKernel.kputs("--> test len = " + args.length + "\n"); 
 
	for(int i=0;i<args.length;i++){
	
 	  if(args[i].equals("1")){ 
  		JVKernel.kputs("--> ENTER test1\n"); 
	    test1(); 
		JVKernel.kputs("--> EXIT test1\n"); 
	
		JVKernel.kputs("len = " + args.length+"\n------\n"); 
		JVKernel.kputs("--> " + args.length  + "\n"); 
	  }
	  
 	  if(args[i].equals("2"))
	    test2(); 
		
   	  if(args[i].equals("3"))
	    test3(); 
	}

	JVKernel.kputs("all gc tests ok\n");
	inner.test();
  }
  
  
  void test3(){
 	String N;
 	boolean b = true;
   	for(int i=0;i<1000000;i++) {
  	  String  s1 = JVKernel.toString(i);
 	  if((i%5000)==0) if(b){ JVKernel.kputs("/\n"); b = false; } else { JVKernel.kputs("\\\n"); b = true; }
 	  N = String.valueOf(i);
 	  new String("1"); //JVKernel.kputs(N+".");
 	  new String(N+"1"); 
      new Object();
 	  int [] x = new int[100];
 	}
  }
  
  public static void main(String [] args){
    JVKernel.kputs("-> Test class 'ForcedGC.java' created: Thu, 28 Jul 2005 17:34:38 +0300\n");    
    ForcedGC t = new ForcedGC();
    t.test(new String[]{"1","2","1","2"});
  }
}	
	
