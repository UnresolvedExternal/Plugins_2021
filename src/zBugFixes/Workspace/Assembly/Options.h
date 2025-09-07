namespace NAMESPACE
{
	namespace Options
	{
		ZOPTION(PackStringFix, true);
		ZOPTION(GetAmountFix, true);
		ZOPTION(PutInInvFix, 2);

		ZOPTION(EventThrottling, 1);

		ZOPTION(EnableNpcFix, true);
		ZOPTION(ClearCriticalStatesOnLoad, true);
		ZOPTION(InfoLeakFix, true);

		ActiveValue<bool> enablePutInInvFix;
	}

	namespace Options
	{
		Sub addTrivia(ZSUB(GameEvent::Execute), []()
			{
				PackStringFix.endTrivia += A"... fixes inventory bugs related to packString usage (such as items disappearence)";
				GetAmountFix.endTrivia += A"... fixes oCNpcInventory::GetAmount function, which is widely used in scripts and can return wrong results";

				PutInInvFix.endTrivia += A"... fixes oCNpc::PutInInv function, which doesn't remove burning torches from the world properly";
				PutInInvFix.endTrivia += A"0 - fix disabled";
				PutInInvFix.endTrivia += A"1 - fix enabled";
				PutInInvFix.endTrivia += A"2 - fix enabled but not for Archolos mod";

				EventThrottling.endTrivia += A"... throttles some types of input messages in hero's AI queue to preserve his long-lasting starvation";
				EventThrottling.endTrivia += A"ex., changing sword to bow and bow to sword several times without ability to interrupt";
				EventThrottling.endTrivia += A"the option value (if not zero) is number of player's commands in AI queue when throttling starts";

				EnableNpcFix.endTrivia += A"... fixes a crash due to the uninitialized world property of the AI object of the activated NPC";
				ClearCriticalStatesOnLoad.endTrivia += A"... fixes bugs when loading starts while talking or looting";

				InfoLeakFix.endTrivia += A"... fixes oCInfo memory leak";
			});

		Sub load(ZSUB(GameEvent::DefineExternals), []()
			{
				PutInInvFix.onChange += []()
				{
					if (PutInInvFix != 2)
					{
						enablePutInInvFix = PutInInvFix;
						return;
					}

					enablePutInInvFix = !parser->GetSymbol("KURT_ARCHOLOS_HISTORY");
				};

				ActiveOptionBase::LoadAll();
			});
	}
}
