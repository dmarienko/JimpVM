import java.lang.*;
import java.util.*;

/**
 * Test class 'VectorTest.java' created Mon, 23 Oct 2006 10:21:48 +0400
 */	
public class VectorTest {

  public VectorTest(){
   // Add your test code here
  }

  void test(String [] args){
   int i;
	Vector v = new Vector();
	for(i=0;i<100;i++)
		v.addElement("h" + i + "\n");
		
	JVKernel.kputs("Start viewing: " + v.size() + "\n");
	
    for(i=0;i<100;i++)
	 JVKernel.kputs(v.elementAt(i).toString());
  }
  
  public static void main(String [] args){
    VectorTest t = new VectorTest();
    t.test(args);
  }
}	
	
