namespace NAMESPACE
{
	namespace Options
	{
		ZOPTION(WhoLetTheDogsOut, A"I don't care");
		ZOPTION(KillNumber, 6);
		ZOPTION(Feofan, A"Feofan");
		ZOPTION(Key, KeyCombo{ KEY_U });
	}

	namespace Options
	{
		Sub addTrivia(ZSUB(GameEvent::Execute), []()
			{
				WhoLetTheDogsOut.endTrivia += A"... the name of the stupid man";
				WhoLetTheDogsOut.endTrivia += A"... leave empty if you don't care";

				Feofan.automaticallyAddEmptyLines = false;
				Feofan.startTrivia += A"";
				Feofan.startTrivia += A"Hey! Dont touch it!";
			});

		Sub load(ZSUB(GameEvent::DefineExternals), []()
			{
				ActiveOptionBase::LoadAll();
			});
	}
}
