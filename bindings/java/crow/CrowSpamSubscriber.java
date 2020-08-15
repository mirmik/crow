package crow;

import java.nio.charset.StandardCharsets;

public class CrowSpamSubscriber //extends CrowNode
{
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