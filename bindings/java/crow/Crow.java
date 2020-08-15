package crow;

public class Crow {
	static {
		try {
		//	System.loadLibrary("igris");
		//	System.loadLibrary("nos");
		//    System.loadLibrary("crow");
		//	System.load("/home/mirmik/project/crow/bindings/java/crow/libcrowjni.so");
		} catch (UnsatisfiedLinkError e) {
			System.err.println("Native code library failed to load.\n" + e);
			System.exit(1);
		}
	}

	public native static void diagnostic(boolean trans, boolean vital);
	public native static void start_spin();
	public native static void join_spin();

    public native static void create_udpgate(int id, int udpport);
}
