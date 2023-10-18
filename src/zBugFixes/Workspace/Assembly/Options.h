namespace NAMESPACE
{
	namespace Options
	{
		ZOPTION(UnconsciousFix, true);
		ZOPTION(WaterWeaponFix, false);
		ZOPTION(PackStringFix, true);
		ZOPTION(GetAmountFix, true);
		ZOPTION(PutInInvFix, 2);

		ZOPTION(EventThrottling, 1);

#if ENGINE >= Engine_G2
		ZOPTION(DayMusicFix, false);
#endif

		ZOPTION(EnableNpcFix, true);
		ZOPTION(ClearCriticalStatesOnLoad, true);

		ActiveValue<bool> enablePutInInvFix;
	}

	namespace Options
	{
		Sub addTrivia(ZSUB(GameEvent::Execute), []()
			{
				UnconsciousFix.endTrivia += A"... forces MOB interaction interruption on transition to unconscious state, so it accomplishes correctly";
				UnconsciousFix.endTrivia += A"for example, without the fix you may receive no expirience when lay out an NPC sleeped on bed";

				WaterWeaponFix.endTrivia += A"... fixes AI starvation on weapon mode changing while walking in water";
				PackStringFix.endTrivia += A"... fixes inventory bugs related to packString usage (such as items disappearence)";
				GetAmountFix.endTrivia += A"... fixes oCNpcInventory::GetAmount function, which is widely used in scripts and can return wrong results";

				PutInInvFix.endTrivia += A"... fixes oCNpc::PutInInv function, which doesn't remove burning torches from the world properly";
				PutInInvFix.endTrivia += A"0 - fix disabled";
				PutInInvFix.endTrivia += A"1 - fix enabled";
				PutInInvFix.endTrivia += A"2 - fix enabled but not for Archolos mod";

				EventThrottling.endTrivia += A"... throttles some types of input messages in hero's AI queue to preserve his long-lasting starvation";
				EventThrottling.endTrivia += A"ex., changing sword to bow and bow to sword several times without ability to interrupt";
				EventThrottling.endTrivia += A"the option value (if not zero) is number of player's commands in AI queue when throttling starts";

#if ENGINE >= Engine_G2
				DayMusicFix.endTrivia += A"... if enabled, daytime music will always start when download ends at night and there is no night theme";
#endif

				EnableNpcFix.endTrivia += A"... fixes a crash due to the uninitialized world property of the AI object of the activated NPC";

				ClearCriticalStatesOnLoad.endTrivia += A"... fixes bugs when loading starts while talking or looting";
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
