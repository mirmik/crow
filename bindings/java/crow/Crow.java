package crow;
import crow.CrowPublisher;
import crow.CrowSubscriber;

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

	native static void init_crow_library();

	static {
		System.loadLibrary("igris");
		System.loadLibrary("nos");
		System.loadLibrary("crow");
		System.loadLibrary("crowjni");
		init_crow_library();
	}

	public native static void diagnostic(boolean trans);
	public native static void start_spin();
	public native static void join_spin();
    public native static void create_udpgate(int id, int udpport);

	public static void main(String[] args) 
	{
		System.out.println("HelloWorld");	
		diagnostic(true);

		create_udpgate(12, 10789);

		CrowPublisher publisher = new CrowPublisher();	
		publisher.bind();

		CrowSubscriber subscriber = new CrowSubscriber();	
		subscriber.bind();

		subscriber.set_crowker_address(".12.127.0.0.1:10009");
		subscriber.set_theme("thm");

		publisher.set_crowker_address(".12.127.0.0.1:10009");
		publisher.set_theme("thm");
		publisher.publish("lalalal".getBytes());
	}
}
