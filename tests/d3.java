import java.lang.*;
import java.io.*;

class d3_inn {
  public void test() throws Exception {
   JVKernel.kputs("--> d3_inn.test() <--\n");  
    throw new Exception();
  }
}

public class d3 {

  public void test1(){
  try {
   throw new InterruptedException("!!!!!!!!!!!!!!!!");
  } catch(InterruptedException e){
   JVKernel.kputs(e.getMessage()+"\n");
  }
  }


  void test2_1() throws Exception {
    JVKernel.kputs("--> test2_1 <--\n");  
    throw new Exception();
  }

  public void test2(int n) throws Exception,InterruptedException {
    JVKernel.kputs("--> test2 <--\n");
	try {
	 d3_inn t = new d3_inn();
	 t.test();
	} catch(Exception e) {
	    JVKernel.kputs("Try to thrown from the catch()...\n");
    	throw new InterruptedException("@@@@@@@@@@@");  
	  }
  }

  public d3(){ }

	public static void main(String [] args){
	  int a1 = 0;
 	  d3 a = new d3();
	  a.test1();
	  try {
		a.test2(2);
	  } catch(InterruptedException e){
		a1 = 100;
	    JVKernel.kputs("Catched InterruptedException: " + e.getMessage() + "\n");
	  } catch(Exception e){
	    JVKernel.kputs("I'm Exception handler !!!\n");
		a1 = 111;
	  }
	  
	   JVKernel.kputs("And finally a1 = " + a1 +"\n");
  
	}
}
