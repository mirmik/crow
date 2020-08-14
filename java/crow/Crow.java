package crow;

public class Crow {
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

    native static void diagnostic(boolean trans, boolean vital);
    native static void start_spin();
    native static void join_spin();

    native static void create_udpgate(int id, int udpport);
}
