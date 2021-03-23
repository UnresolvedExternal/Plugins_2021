namespace NAMESPACE
{
	void SetGameHour()
	{
		Unlocked<float> ticksPerHour = ZENDEF(0x007DE8C0, 0x0082188C, 0x0082F090, 0x0083E168);
		Unlocked<float> ticksPerMinute = ZENDEF(0x008DC51C, 0x0092467C, 0x009842AC, 0x00AB3764);
		Unlocked<float> ticksPerDay = ZENDEF(0x008DC510, 0x0092461C, 0x00984264, 0x00AB371C);

		ticksPerHour = Options::SecondsPerGameHour * 1000.0f;
		ticksPerMinute = ticksPerHour / 60.0f;
		ticksPerDay = ticksPerHour * 24.0f;
	}

	Sub listenSecondsPerGameHour(ZSUB(GameEvent::Execute), []()
		{
			Options::SecondsPerGameHour.onChange += &SetGameHour;
		});
}
