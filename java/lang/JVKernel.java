package java.lang;

public class JVKernel {
  public static native void kputs(String a);
  public static native boolean copyArray(Object src,int bsrc,Object dst,int bdst,int len);

  public static native String toString(int i);
  public static native String toString(float f);

  public static String toString(boolean b){
	if(b) return "true";
	return "false";
  }

  public static String toString(char c){
	char chr[] = new char[1];
	chr[0] = c;
	return new String(chr);
  }
}
