namespace NAMESPACE
{
	void SetGameHour()
	{
		struct Values
		{
			const float ticksPerHour;
			const float ticksPerMinute;
			const float ticksPerDay;

			Values(float ticksPerHour, float ticksPerMinute, float ticksPerDay) :
				ticksPerHour{ ticksPerHour },
				ticksPerMinute{ ticksPerMinute },
				ticksPerDay{ ticksPerDay }
			{

			}

		};

		static std::optional<Values> oldValues;

		if (Options::SecondsPerGameHour <= 0 && !oldValues.has_value())
			return;

		Unlocked<float> ticksPerHour = ZENDEF(0x007DE8C0, 0x0082188C, 0x0082F090, 0x0083E168);
		Unlocked<float> ticksPerMinute = ZENDEF(0x008DC51C, 0x0092467C, 0x009842AC, 0x00AB3764);
		Unlocked<float> ticksPerDay = ZENDEF(0x008DC510, 0x0092461C, 0x00984264, 0x00AB371C);

		if (!oldValues.has_value())
			oldValues.emplace(ticksPerHour, ticksPerMinute, ticksPerDay);

		float percent = 0.0f;

		if (ogame)
			percent = ogame->wldTimer->worldTime / ticksPerDay;

		if (Options::SecondsPerGameHour <= 0)
		{
			ticksPerHour = oldValues->ticksPerHour;
			ticksPerMinute = oldValues->ticksPerMinute;
			ticksPerDay = oldValues->ticksPerDay;
			oldValues.reset();

			if (ogame)
				ogame->wldTimer->worldTime = percent * ticksPerDay;

			return;
		}

		ticksPerHour = Options::SecondsPerGameHour * 1000.0f;
		ticksPerMinute = ticksPerHour / 60.0f;
		ticksPerDay = ticksPerHour * 24.0f;

		if (ogame)
			ogame->wldTimer->worldTime = percent * ticksPerDay;
	}

	Sub listenSecondsPerGameHour(ZSUB(GameEvent::Execute), []()
		{
			Options::SecondsPerGameHour.onChange += &SetGameHour;
		});
}
