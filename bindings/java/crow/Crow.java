package crow;

public class Crow {
	static void loadLibrary(String name) 
	{
		try {
			System.loadLibrary(name);
		} catch (UnsatisfiedLinkError e) {
			System.err.println("Native code library failed to load.\n" + name + "\n" + e);
			System.exit(1);
		}
	}

	static {
		System.loadLibrary("igris");
		System.loadLibrary("nos");
		System.loadLibrary("crow");
		System.loadLibrary("crowjni");
	}

	public native static void diagnostic(boolean trans);
	public native static void start_spin();
	public native static void join_spin();

    public native static void create_udpgate(int id, int udpport);
}
