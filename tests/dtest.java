import java.lang.*;

class dtest {
	static float fs = 0;
	final int Z = 0;

	public static void main( String s[] ) {
	 int l;
     boolean v, bb[] = new boolean[10];
	 char cs, cc[] = new char[20];	

	 fs = (float) 3.14;
	 cs = 'A';
	 v = true;
	 cc[0] = 'a';
	 cc[1] = 'b';
	 cc[2] = 'c';
	 cc[3] = 'd';

	 bb[0] = true;
	 bb[1] = false;
	 bb[2] = true;
	 bb[3] = true;

//	 JVKernel.kputs( "Test: fs = " + fs + " cs = " + cs + " v = " + v + "\n" );
	 JVKernel.kputs( "Array len = " + cc.length + "\n" );
	 for(l=0;l<cc.length;l++) { JVKernel.kputs( " " + cc[l] );};
	 JVKernel.kputs("\n");

	 for(l=0;l<bb.length;l++) { JVKernel.kputs( " " + bb[l] );};
	 JVKernel.kputs("\n");
	}
}
