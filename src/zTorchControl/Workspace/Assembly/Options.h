namespace NAMESPACE
{
	namespace Options
	{
		ZOPTION(SaveTorches, ZENDEF(false, false, true, true));
		ZOPTION(ExchangeTorchOnTeleport, true);
		ZOPTION(TorchRemoveRange, 5000);
		ZOPTION(TorchHotkeyLogicalValue, 30);
		ZOPTION(TorchHotkeyOverride, KeyCombo{});

		ActiveValue<bool> EnableHotkey;
	}

	namespace Options
	{
		Sub addTrivia(ZSUB(GameEvent::Execute), []()
			{
				SaveTorches.endTrivia += A"... forces burning torches saving";

				ExchangeTorchOnTeleport.endTrivia += A"... replaces the torch with the new one on teleport or world change";
				ExchangeTorchOnTeleport.endTrivia += A"sometimes it helps to recover torch effects";

				TorchRemoveRange.endTrivia += A"... removes burning torches which are too far";
				TorchRemoveRange.endTrivia += A"also removes torches when leaving location";
				TorchRemoveRange.endTrivia += A"set 0 to disable the feature";

				TorchHotkeyLogicalValue.endTrivia += A"... sets internal logical value for torch hotkey and introduces the associated option [KEYS].keyTorch";
				TorchHotkeyLogicalValue.endTrivia += A"set 0 for no effect";

				TorchHotkeyOverride.endTrivia += A"... when disabled (#) has no effect";
				TorchHotkeyOverride.endTrivia += A"else the key is used instead of [KEYS].keyTorch value";
				TorchHotkeyOverride.endTrivia += A"key format example: KEY_LSHIFT + KEY_T";
			});

		Sub listenOptions(ZSUB(GameEvent::Execute), []()
			{
				auto enableHotkey = []()
				{
					EnableHotkey = TorchHotkeyLogicalValue || *TorchHotkeyOverride;
				};

				TorchHotkeyLogicalValue.onChange += enableHotkey;
				TorchHotkeyOverride.onChange += enableHotkey;
			});

		Sub load(ZSUB(GameEvent::DefineExternals), &ActiveOptionBase::LoadAll);
	}
}
