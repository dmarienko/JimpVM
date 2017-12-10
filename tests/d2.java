import java.lang.*;
import java.io.*;

public class d2 {
  static int a = 9876;
  public int f1;

  public class InnerTest {
    public void test(){ JVKernel.kputs("I'm inner class !\n"); }
  }
	
  public String test1(){
	try {
 	  String s = new String("L12345");
	} catch(NullPointerException e){ return "1"; }
	return null;
  }

  public void test2() throws Exception {
	throw new Exception();
  }

  public void test3(){
	String r;
	try {
	  r = test1();
//	  test2();
	} catch(Exception e){ e.printStackTrace(); }
  }

  public d2(String start){
	for(int i=0;i<20;i++){
	  JVKernel.kputs(start + i + "\n");
	}
	InnerTest it = new InnerTest();
	it.test();
  }

  public void print(){
    JVKernel.kputs("f1 = "+f1+"\n");
  }


  static void testN(){
  }
  
	public static void main(String [] args){
	  int z = 1234;
	  
	  JVKernel.kputs("hello, best World !!!" + z + "\n");
	  JVKernel.kputs("Test 1" + "222222\n");
	  
 	  String s = JVKernel.toString(1234);
	  JVKernel.kputs(" The fucking result are : " + s + " !\n");
	  JVKernel.kputs(" The fucking result 2 are : " + z * 1000 + " !\n");
	  
	  for(int i=0;i<20;i++){
		JVKernel.kputs("index : " + i + "\n");
	  }
	  JVKernel.kputs("Static a = " + a + "\n");
 	  d2 a = new d2("In class : ");
	  a.f1 = 1111;
	  a.print();

	  try {
		a.test2();
	  } catch(Exception e){
		JVKernel.kputs("Exception occured \n");
	  }
	}
}
