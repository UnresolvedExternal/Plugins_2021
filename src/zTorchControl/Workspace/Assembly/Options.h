namespace NAMESPACE
{
	namespace Options
	{

#if ENGINE >= Engine_G2
		ZOPTION(SaveTorches, ZENDEF(false, false, true, true));
#endif

		ZOPTION(ExchangeTorchOnTeleport, true);
		ZOPTION(TorchRemoveRange, 5000);
		ZOPTION(UseEngineHotkey, true);
		ZOPTION(AdditionalHotkey, KeyCombo{});

		ActiveValue<bool> EnableHotkey;
	}

	namespace Options
	{
		Sub addTrivia(ZSUB(GameEvent::Execute), []()
			{
#if ENGINE >= Engine_G2
				SaveTorches.endTrivia += A"... forces burning torches saving";
#endif

				ExchangeTorchOnTeleport.endTrivia += A"... replaces the torch with the new one on teleport or world change";
				ExchangeTorchOnTeleport.endTrivia += A"sometimes it helps to recover torch effects";

				TorchRemoveRange.endTrivia += A"... removes burning torches which are too far";
				TorchRemoveRange.endTrivia += A"also removes torches when leaving location";
				TorchRemoveRange.endTrivia += A"set 0 to disable the feature";

				UseEngineHotkey.endTrivia += A"... sets internal logical value for torch hotkey and introduces the associated option [KEYS].keyTorch";
				UseEngineHotkey.endTrivia += A"default hotkey value will be set as 'KEY_T or KEY_NUMPAD9'";

				AdditionalHotkey.endTrivia += A"... this key could be used instead of [KEYS].keyTorch value";
			});

		Sub listenOptions(ZSUB(GameEvent::Execute), []()
			{
				auto enableHotkey = []()
				{
					EnableHotkey = UseEngineHotkey || *AdditionalHotkey;
				};

				UseEngineHotkey.onChange += enableHotkey;
				AdditionalHotkey.onChange += enableHotkey;
			});

		Sub load(ZSUB(GameEvent::DefineExternals), &ActiveOptionBase::LoadAll);
	}
}
