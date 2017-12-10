import java.lang.*;

public class AScreen {
  static final String TEST_STING = "Jimp VM test";
  
  public static final int BLACK=0, RED=1, GREEN=2, BROWN=3, BLUE=4, MAGENTA=5, 
	CYAN=6, GRAY=7, DARKGRAY=8, LIGHTRED=9, 
	LIGHTGREEN=10, YELLOW=11, LIGHTBLUE=12, 
	LIGHTMAGENTA=13, LIGHTCYAN=14, WHITE=15;
  
  static int defBG, defFG, curBG, curFG;
  
  static String [] fgrnd = {
	"\u001b[0m\u001b[30m",
	"\u001b[0m\u001b[31m",
	"\u001b[0m\u001b[32m",
	"\u001b[0m\u001b[33m",
	"\u001b[0m\u001b[34m",
	"\u001b[0m\u001b[35m",
	"\u001b[0m\u001b[36m",
	"\u001b[0m\u001b[37m",
	"\u001b[1m\u001b[30m",
	"\u001b[1m\u001b[31m",
	"\u001b[1m\u001b[32m",
	"\u001b[1m\u001b[33m",
	"\u001b[1m\u001b[34m",
	"\u001b[1m\u001b[35m",
	"\u001b[1m\u001b[36m",
	"\u001b[1m\u001b[37m",
  };

  static String [] bgrnd = {
	"\u001b[40m",
	"\u001b[41m",
	"\u001b[42m",
	"\u001b[43m",
	"\u001b[44m",
	"\u001b[45m",
	"\u001b[46m",
	"\u001b[47m",
  };

  public static void cls(){ JVKernel.kputs("\u001b[2J"); }
  
  public static void setFG(int num){
	JVKernel.kputs(fgrnd[num]);
	curFG = num;
	//setBG(curBG);
  }

  public static void setBG(int num){
	JVKernel.kputs(bgrnd[num]);
	curBG = num;
  }
  
  public static void gotoXY(int x, int y){
	JVKernel.kputs("\u001b["+y+";"+x+"f"); 
  }

  public static void drawRectangle(int x,int y,int w,int h){
	int i;
	char [] c = new char[w];
	for(i=0;i<w;i++) c[i] = ' ';
	String row = new String(c);
	for(i=0;i<h;i++){
	  gotoXY(x,y+i);
	  JVKernel.kputs(row);
	}
  }

  public static void testScreen(){
    cls();
	setFG(WHITE);
 	setBG(GREEN);
	drawRectangle(2,2,20,10);
	gotoXY(5,5);	
	JVKernel.kputs(TEST_STING);

	setFG(RED);
 	setBG(BLUE);
	drawRectangle(24,2,20,10);
	gotoXY(27,5);	
	JVKernel.kputs(TEST_STING);

	setFG(YELLOW);
 	setBG(MAGENTA);
	drawRectangle(46,2,20,10);
	gotoXY(49,5);	
	JVKernel.kputs(TEST_STING);

	setFG(WHITE);
 	setBG(RED);
	drawRectangle(68,2,20,10);
	gotoXY(71,5);	
	JVKernel.kputs(TEST_STING);


	gotoXY(25,25);	
  }

  public static void main(String [] args){
    cls();

	testScreen();

 	setBG(BLACK);
	setFG(GRAY);
  }    
}
