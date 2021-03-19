namespace NAMESPACE
{
	namespace Settings
	{
		ZOPTION(SaveTorches, ZENDEF(false, false, true, true));
		ZOPTION(ExchangeTorchOnTeleport, true);
		ZOPTION(TorchRemoveRange, 5000);
		ZOPTION(TorchHotkeyLogicalValue, 30);
		ZOPTION(TorchHotkeyOverride, KeyCombo{});

		ActiveValue<bool> EnableHotkey;
	}

	namespace Settings
	{
		Sub load(ZSUB(GameEvent::DefineExternals), []()
			{
				auto enableHotkey = []()
				{
					EnableHotkey = TorchHotkeyLogicalValue || *TorchHotkeyOverride;
				};

				TorchHotkeyLogicalValue.onChange += enableHotkey;
				TorchHotkeyOverride.onChange += enableHotkey;

				ActiveOptionBase::LoadAll();
			});
	}
}
