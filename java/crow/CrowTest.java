package crow;

import crow.Crow;
import crow.CrowSpamSubscriber;

public class CrowTest {
	public static void main(String[] args) {
		Crow.diagnostic(true, false);

		Crow.create_udpgate(12, 10010);

		var subscriber = new CrowSpamSubscriber();
		subscriber.bind(12);

		Crow.start_spin();
		Crow.join_spin();
	}
}