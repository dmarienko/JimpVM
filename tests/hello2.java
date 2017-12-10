import java.lang.*;

public class hello2 { 
  int a0, a1, a2;

  public hello2(){
 	JVKernel.kputs("-- Default constructor --\n");
  }
  
  public hello2(int a){
	JVKernel.kputs("-- Test putfield start --\n");
    a0 = 111; a1 = 222; a2 = 1;
	JVKernel.kputs("-- Test putfield end --\n");
  }

  public void test(){ 
	JVKernel.kputs("-- test getfield: a0=" + a0 + "\ta1=" + a1 +  "\ta2="+a2+"  --\n");
  }
  
  public static void main(String [] argv){
    hello2 h = new hello2(1);
	h.test();
  }
}
