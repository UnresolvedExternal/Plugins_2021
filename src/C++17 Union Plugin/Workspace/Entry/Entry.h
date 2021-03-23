// this file is added in Workspace/Assembly/Includes.h

namespace NAMESPACE
{
	Sub helloWorld(ZSUB(GameEvent::Execute), []()
		{
			Message::Info("Hello, World!");
		});
}
