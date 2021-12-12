package crow;
import crow.CrowNode;

public class CrowPublisher extends CrowNode
{
	private long instance;
	public CrowPublisher()
	{
		instance = nativeNew();
	}

	native private long nativeNew();
}
