namespace NAMESPACE
{
	Sub clearCriticalStatesOnLoad(ZSUB(GameEvent::LoadBegin), Options::ClearCriticalStatesOnLoad, []
		{
			if (!oCInformationManager::GetInformationManager().HasFinished())
			{
				oCInformationManager::GetInformationManager().OnExit();
				oCInformationManager::GetInformationManager().OnTermination();
			}

			if (player && player->inventory2.IsOpen())
			{
				COA(player->interactMob, CastTo<oCMobContainer>(), Close(player));
				COA(player->interactMob, InterruptInteraction(player));

				player->CloseSteal();
				player->CloseDeadNpc();
				player->CloseInventory();
			}

			zCSkyControler::s_activeSkyControler = COA(ogame, world, activeSkyControler);

			if (ogame)
				ogame->showRoutineNpc = nullptr;
		});
}
