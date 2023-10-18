namespace NAMESPACE
{
	int __cdecl Joker_InsertNpcNearToHero()
	{
		int instance;
		ZARGS(instance);

		ZOwner<oCNpc> npc{ static_cast<oCNpc*>(ogame->GetGameWorld()->CreateVob(zVOB_TYPE_NSC, instance)) };

		if (!npc)
			return 0;

		zVEC3 position = player->GetPositionWorld() + player->GetAtVectorWorld() * 750;
		const int rayFlags = zTRACERAY_POLY_IGNORE_TRANSP | zTRACERAY_VOB_IGNORE_CHARACTER | zTRACERAY_STAT_POLY;

		if (ogame->GetWorld()->TraceRayNearestHit(player->GetPositionWorld(), position - player->GetPositionWorld(), player, rayFlags))
			position = ogame->GetWorld()->traceRayReport.foundIntersection - player->GetAtVectorWorld() * 50;

		npc->respawnOn = false;
		ogame->GetSpawnManager()->SpawnNpc(npc.get(), position, 0.0f);
		npc->SetHeadingAtWorld(-player->GetAtVectorWorld());
		return 0;
	}

	ZEXTERNAL(Joker_InsertNpcNearToHero, void, int);
}
