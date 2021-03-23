#if ENGINE >= Engine_G2

namespace NAMESPACE
{
	void SetPostLoadDelay()
	{
		static std::optional<int> originalCode;

		// PostLoadDelay is never set: do not touch the engine code
		if (Options::PostLoadDelay < 0 && !originalCode.has_value())
			return;

		Unlocked<int> postLoadDelayPointer = ZENDEF(0x00000000, 0x00000000, 0x0066B9CF, 0x006C873F);

		if (!originalCode.has_value())
			originalCode = postLoadDelayPointer;

		if (Options::PostLoadDelay < 0)
		{
			postLoadDelayPointer = originalCode.value();
			return;
		}

		// create new variable instead of changing the original
		// thanks to Kirides for the bug report
		static float newValue;
		newValue = static_cast<float>(Options::PostLoadDelay);
		postLoadDelayPointer = reinterpret_cast<int>(&newValue);
	}

	Sub listenPostLoadDelay(ZSUB(GameEvent::Execute), []()
		{
			Options::PostLoadDelay.onChange += &SetPostLoadDelay;
		});
}

#endif
