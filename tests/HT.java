import java.lang.*;
import java.util.*;

/**
 * Test class 'HT.java' created Thu, 23 Mar 2006 17:16:13 +0300
 */	
public class HT {

  public HT(){
   // Add your test code here
  }

  void test(String [] args){
    // Add your test code here
	Object o1 = new Object();
	Object o2 = new Object();

    JVKernel.kputs(o1.hashCode()+"\n");    
    JVKernel.kputs(o2.hashCode()+"\n");    
  }
  
  public static void main(String [] args){
    JVKernel.kputs("-> Test class 'HT.java' created: Thu, 23 Mar 2006 17:16:13 +0300\n");    
    HT t = new HT();
    t.test(args);
  }
}	
	
