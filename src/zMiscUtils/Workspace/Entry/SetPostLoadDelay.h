#if ENGINE >= Engine_G2

namespace NAMESPACE
{
	void SetPostLoadDelay()
	{
		Unlocked<float> postLoadDelay = ZENDEF(0x00000000, 0x00000000, 0x0082B9D0, 0x008399F8);
		postLoadDelay = static_cast<float>(Options::PostLoadDelay);
	}

	Sub listenPostLoadDelay(ZSUB(GameEvent::Execute), []()
		{
			Options::PostLoadDelay.onChange += &SetPostLoadDelay;
		});
}

#endif
