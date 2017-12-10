
class SwitchTest{
	public static void switchTest(int value){
		/* Currently, this first TableSwitch starts at an address
		   which is divisible by 4.  If you change the code, try
		   to leave at least one of the switch statements on such
		   an address, since that will test the interpreter's
		   handling of the zero-padding in the xxxSWITCH opcodes.
		*/
		JVKernel.kputs("v = " + value + "\n");
		/* Tableswitch */
		switch(value){
		case 2:
			JVKernel.kputs("Two\n");
			break;
		case 3:
			JVKernel.kputs("Three\n");
			break;
		default:
			JVKernel.kputs("Not two or three\n");
			break;
		}
		/* Lookupswitch can be implemented using a binary search,
		   and often off-by-one errors in binary searches only turn
		   up with either even or odd numbers of items, but not both.
		   Thus, we test both cases.
		*/
		/* Lookupswitch, even */
		switch(value){
		case 2:
			JVKernel.kputs("Two again\n");
			break;
		case 102:
			JVKernel.kputs("One hundred and two\n");
			break;
		default:
			JVKernel.kputs("Not 2 or 102\n");
			break;
		}
		/* Lookupswitch, odd */
		switch(value){
		case 3:
			JVKernel.kputs("Three again\n");
			break;
		case 103:
			JVKernel.kputs("One hundred and three\n");
			break;
		case -1000000003: /* Make sure multi-byte negative values are handled properly */
			JVKernel.kputs("Minus one billion and three\n");
			break;
		default:
			JVKernel.kputs("Not 3, 103, or -1000000003\n");
			break;
		}
	}

	public static void main(String[] args){
		/* Make sure the default case is handled cleanly for values
		   just outside the tableswitch's range:
		*/
		switchTest(1);
		switchTest(2);
		switchTest(3);
		switchTest(4);
		/* Test the lookupswitches */
		switchTest(100);
		switchTest(102);
		switchTest(103);
		switchTest(1000);
		switchTest(-1000000003);
	}
}

