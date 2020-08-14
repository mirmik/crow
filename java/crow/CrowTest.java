package crow;

import crow.Crow;

public class CrowTest {
	public static void main(String[] args) {
		Crow.diagnostic(true, false);

		Crow.create_udpgate(12, 10010);

		Crow.start_spin();
		Crow.join_spin();
	}
}