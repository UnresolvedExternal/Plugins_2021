namespace NAMESPACE
{
	namespace Options
	{
		ZOPTION(MidnightFix, true);
		ZOPTION(UnconsciousFix, true);
		ZOPTION(WaterWeaponFix, true);
		ZOPTION(PackStringFix, true);
		ZOPTION(GetAmountFix, true);
		ZOPTION(PutInInvFix, true);

#if ENGINE >= Engine_G2
		ZOPTION(RemoveLightFix, true);
#endif

		ZOPTION(EventThrottling, 1);
	}

	namespace Options
	{
		Sub addTrivia(ZSUB(GameEvent::Entry), []()
			{
				MidnightFix.endTrivia += A"... suppresses midnight healing of nearest NPCs";
				MidnightFix.endTrivia += A"allows to avoid such sitiuations as enemy full refreshing during the fight";

				UnconsciousFix.endTrivia += A"... forces MOB interaction interruption on transition to unconscious state, so it accomplishes correctly";
				UnconsciousFix.endTrivia += A"for example, without the fix you may receive no expirience when lay out an NPC sleeped on bed";

				WaterWeaponFix.endTrivia += A"... fixes AI starvation on weapon mode changing while walking in water";
				PackStringFix.endTrivia += A"... fixes inventory bugs related to packString usage (such as items disappearence)";
				GetAmountFix.endTrivia += A"... fixes oCNpcInventory::GetAmount function, which is widely used in scripts and can return wrong results";
				PutInInvFix.endTrivia += A"... fixes oCNpc::PutInInv function, which doesn't remove burning torches from the world properly";
				
#if ENGINE >= Engine_G2
				RemoveLightFix.endTrivia += A"... updates lighting when zCVobLight is removed";
#endif
				
				EventThrottling.endTrivia += A"... throttles some types of input messages in hero's AI queue to preserve his long-lasting starvation";
				EventThrottling.endTrivia += A"ex., changing sword to bow and bow to sword several times without ability to interrupt";
				EventThrottling.endTrivia += A"the option value (if not zero) is number of player's commands in AI queue when throttling starts";
			});

		Sub load(ZSUB(GameEvent::DefineExternals), &ActiveOptionBase::LoadAll);
	}
}
