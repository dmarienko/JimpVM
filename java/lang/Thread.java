package java.lang;

public class Thread implements Runnable {

	private int priority;
	
    /* What will be run. */
    private Runnable target;

    /**
     * The minimum priority that a thread can have. 
     */
    public final static int MIN_PRIORITY = 1;

   /**
     * The default priority that is assigned to a thread. 
     */
    public final static int NORM_PRIORITY = 2;

    /**
     * The maximum priority that a thread can have. 
     */
    public final static int MAX_PRIORITY = 10;

    public synchronized native void init0( Thread t );
    public static native void sleep0( int msec );

    public synchronized native void start();
    public synchronized native void stop0();
    public synchronized native void suspend();
    public synchronized native void resume();
    private native void setPriority0(int newPriority);
	
    public synchronized void stop() {
	  resume();
	  stop0();
	}
	
    public final void setPriority(int newPriority) {
	  if (newPriority > MAX_PRIORITY || newPriority < MIN_PRIORITY) {
	   // throw new IllegalArgumentException();
	   return;
	  }
	  sleep0(1);
	  setPriority0(priority = newPriority);
    }

    private void init( Runnable target, String name ){
	  init0( this );
	  //setPriority( NORM_PRIORITY );
	  this.target = target;
    }


	public static void sleep( int msec ) {
	  sleep0( msec );
	}

    public Thread() {
	  init( null, "Thread-" );
    }

    public Thread( Runnable target ) {
	  init( target, "Thread-" );
    }

    public void run() {
	  if (target != null) target.run();
    }

    private void exit() {
	  target = null;
    }

}

