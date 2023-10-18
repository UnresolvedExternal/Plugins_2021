namespace NAMESPACE
{
	FASTHOOK_PATCH_OPT(oCNpc, EV_DamagePerFrame, enablePlugin);

	int oCNpc::EV_DamagePerFrame_Union(oCMsgDamage* message)
	{
		oSDamageDescriptor& desc = message->descDamage;

		// инициализируем при первом вызове
		if (!message->IsInUse())
		{
			message->SetInUse(true);

			desc.dwFieldsValid |= oEDamageDescFlag_OverlayActivate;
			desc.fTimeCurrent = desc.fTimeInterval;
			desc.fDamageTotal = desc.fDamagePerInterval;
		}

		float timeDebt = ztimer->frameTimeFloat;

		// пока не обработали всё время кадра и OnDamage не сигнализировал об окончании...
		while (timeDebt > 0.0f && !desc.bFinished)
		{
			// обрабатываем всё время, но не более текущего интервала
			const float dt = std::min(timeDebt, desc.fTimeCurrent);

			// обновляем таймеры
			timeDebt -= dt;
			desc.fTimeCurrent -= dt;
			desc.fTimeDuration -= dt;

			// если интервал истёк, то вызываем OnDamage
			if (desc.fTimeCurrent <= 0.0f)
			{
				OnDamage(desc);
				desc.fTimeCurrent += desc.fTimeInterval;
			}
		}

		// возвращаем true, если сообщение полностью обработано и его можно удалить
		return desc.bFinished;
	}
}
