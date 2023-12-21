namespace NAMESPACE
{
	Sub clearCriticalStatesOnLoad(ZSUB(GameEvent::LoadBegin), Options::ClearCriticalStatesOnLoad, []
		{
			if (!oCInformationManager::GetInformationManager().HasFinished())
			{
				oCInformationManager::GetInformationManager().OnExit();
				oCInformationManager::GetInformationManager().OnTermination();
			}

			oCNpc* const self = SaveLoadGameInfo.changeLevel ? oCNpc::dontArchiveThisNpc : player;

			if (self && self->inventory2.IsOpen())
			{
				COA(self->interactMob, CastTo<oCMobContainer>(), Close(self));

				self->CloseSteal();
				self->CloseDeadNpc();
				self->CloseInventory();
			}

			zCSkyControler::s_activeSkyControler = COA(ogame, world, activeSkyControler);
		});
}
