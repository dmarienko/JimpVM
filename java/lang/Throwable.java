/*
 * @(#)Throwable.java	1.51 03/01/23
 */
package java.lang;
import  java.io.*;

public class Throwable implements Serializable {
 /** use serialVersionUID from JDK 1.0.2 for interoperability */
  private static final long serialVersionUID = -3042686055658047285L;

 /**
  * Native code saves some indication of the stack backtrace in this slot.
  */
  private transient Object backtrace; 

 /**
  * Specific details about the Throwable.  For example, for
  * <tt>FileNotFoundException</tt>, this contains the name of
  * the file that could not be found.
  *
  * @serial
  */
  private String detailMessage;
  

 /*
  * This field is lazily initialized on first use or serialization and
  * nulled out when fillInStackTrace is called.
  */
  
 /**
  * Constructs a new throwable with <code>null</code> as its detail message.
  * The cause is not initialized, and may subsequently be initialized by a
  * call to {@link #initCause}.
  *
  * <p>The {@link #fillInStackTrace()} method is called to initialize
  * the stack trace data in the newly created throwable.
  */
  public Throwable() {
// 	fillInStackTrace();
  }
  
 /**
  * Constructs a new throwable with the specified detail message.  The
  * cause is not initialized, and may subsequently be initialized by
  * a call to {@link #initCause}.
  *
  * <p>The {@link #fillInStackTrace()} method is called to initialize
  * the stack trace data in the newly created throwable.
  *
  * @param   message   the detail message. The detail message is saved for 
  *          later retrieval by the {@link #getMessage()} method.
  */
  public Throwable(String message) {
// 	fillInStackTrace();
	detailMessage = message;
  }

 /**
  * Returns the detail message string of this throwable.
  *
  * @return  the detail message string of this <tt>Throwable</tt> instance
  *          (which may be <tt>null</tt>).
  */
  public String getMessage() {
	return detailMessage;
  }

 /**
  * Creates a localized description of this throwable.
  * Subclasses may override this method in order to produce a
  * locale-specific message.  For subclasses that do not override this
  * method, the default implementation returns the same result as
  * <code>getMessage()</code>.
  *
  * @return  The localized description of this throwable.
  * @since   JDK1.1
  */
  public String getLocalizedMessage() {
	return getMessage();
  }
  
 /**
  * Returns a short description of this throwable.
  * If this <code>Throwable</code> object was created with a non-null detail
  * message string, then the result is the concatenation of three strings: 
  * <ul>
  * <li>The name of the actual class of this object 
  * <li>": " (a colon and a space)
  * <li>The result of the {@link #getMessage} method for this object 
  * </ul>
  * If this <code>Throwable</code> object was created with a <tt>null</tt>
  * detail message string, then the name of the actual class of this object
  * is returned. 
  *
  * @return a string representation of this throwable.
  */
  public String toString() {
	String s = "Throwable"; //getClass().getName();
	String message = getLocalizedMessage();
	return (message != null) ? (s + ": " + message) : s;
  }

  public void printStackTrace() { 
   //printStackTrace(System.err);
  }

  public synchronized native Throwable fillInStackTrace();

}
