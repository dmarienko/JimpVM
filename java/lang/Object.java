package java.lang;

public class Object {
 /** Returns the string representation of the object
  */
  public String toString(){ return ""; }

  public boolean equals(Object obj){
	return (this == obj);
  }
 
  
  public final native void wait();
  public final native void notify();
  public final native void notifyAll();
  
  public native int hashCode();
}
