package java.lang;


public class StringBuffer {
	public String strings[] = null;

 /* Bug is here  !!! */	
//	static
	int count = 0;

	/** Constructs an empty string buffer. */
	public StringBuffer(int n){
	  strings = new String[n];
	}

	public StringBuffer(){
	  this( 16 );
	}

	/** Constructs a string buffer containing the given string.  */
	public StringBuffer(String s) {
 	  this( s.length() + 16 );
 	  append(s);
	}

	public StringBuffer append(boolean b) {
	  return append( JVKernel.toString( b ) );
	}

	public StringBuffer append(char c) {
	  return append( JVKernel.toString( c ) );
	}

	public StringBuffer append(int i) {
	  return append( JVKernel.toString( i ) );
	}

	public StringBuffer append(float f)	{
	  return append( JVKernel.toString( f ) );
	}

	public StringBuffer append( char c[] ) {
	  return append( new String(c) );
	}


	public StringBuffer append( String s ) {
//  JVKernel.debugMark1();
	  if( s == null ) return this;
	  if( strings.length == count ) {
		String newStrings[] = new String[strings.length * 2];
		JVKernel.copyArray( strings, 0, newStrings, 0, strings.length );
		strings = newStrings;
	  }

	  strings[count++] = s;
// JVKernel.debugMark2();
	  
      return this;
	}


	public StringBuffer append( Object obj ) {
	  return append( obj.toString() );
	}

	public void setLength(int zero)	{
	  count = 0;
	}

	public String toString() {
	  return new String( strings, count );
	}
}
