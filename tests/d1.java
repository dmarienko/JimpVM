import java.lang.*;

class A0 {
  public void f0(){ JVKernel.kputs("A0.f0()\n");  }
}

class A extends A0 {
  public void f1(){ JVKernel.kputs("A.f1()\n");  }
}

class B extends A {
  public void f2(){ JVKernel.kputs("B.f2()\n");  }
}



public class d1 {
  public static void main(String [] args){
    B b = new B();
	b.f2();
	b.f1();
	b.f0();
  }
}
