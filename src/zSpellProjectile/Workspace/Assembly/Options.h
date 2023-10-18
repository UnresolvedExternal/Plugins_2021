namespace NAMESPACE
{
	namespace Options
	{
		ZOPTION(Spells, VectorOption<int>{ 105 });
	}

	namespace Options
	{
		Sub addTrivia(ZSUB(GameEvent::Execute), []()
			{
				Spells.endTrivia += A"... spell numbers separated by '|'";
				Spells.endTrivia += A"this spells will be forced to have projectile collision class";
			});

		Sub load(ZSUB(GameEvent::DefineExternals), []()
			{
				ActiveOptionBase::LoadAll();
			});
	}
}
