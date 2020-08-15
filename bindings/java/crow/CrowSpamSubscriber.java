package crow;

import java.nio.charset.StandardCharsets;

public class CrowSpamSubscriber //extends CrowNode
{
	static {
		try {
		//	System.loadLibrary("igris");
		//	System.loadLibrary("nos");
		    System.loadLibrary("crow");
			System.load("/home/mirmik/project/crow/java/crow/libcrowjni.so");
		} catch (UnsatisfiedLinkError e) {
			System.err.println("Native code library failed to load.\n" + e);
			System.exit(1);
		}
	}

	private long instance;

	public void on_message(byte[] arr) 
	{ 
		System.out.println("on_message (need to reimplement)"); 
		System.out.println(arr.length);
		System.out.println(arr[0]);
		String s = new String(arr, StandardCharsets.UTF_8);
		System.out.println(s);
	}

	native private long nativeNew();

	CrowSpamSubscriber() 
	{
		instance = nativeNew();
	}

	native public void bind(int no);
}