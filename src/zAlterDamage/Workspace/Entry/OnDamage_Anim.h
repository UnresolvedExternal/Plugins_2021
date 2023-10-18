namespace NAMESPACE
{
	FASTHOOK_PATCH_OPT(oCNpc, OnDamage_Anim, enablePlugin);

	void oCNpc::OnDamage_Anim_Union(oSDamageDescriptor& desc)
	{
		// проверка наличия модели, ИИ и контроллера анимаций
		if (!dynamic_cast<oCAIHuman*>(callback_ai) || !GetAnictrl() || !GetModel())
			return;

		// при активном уроне от падения не проигрываются никакие дополнительные анимации
		if (HasFlag(desc.enuModeDamage, oEDamageType_Fall))
			return;

		// находим углы в градусах от жертвы к атакующему
		if (desc.pVobAttacker)
			GetAngles(desc.pVobAttacker, desc.fAzimuth, desc.fElevation);

		// расслабить тетиву
		if (GetWeaponMode() == NPC_WEAPON_BOW)
			if (zCMorphMesh* mesh = dynamic_cast<zCMorphMesh*>(GetWeapon()))
				mesh->StartAni("S_RELAX", 1.0f, -2.0f);

		// при уроне от полёта или от барьера жертва должна отлететь...
		bool fly = roundf(desc.aryDamageEffective[oEDamageIndex_Barrier]) + roundf(desc.aryDamageEffective[oEDamageIndex_Fly]);

		// ... но не в воде и не в лежачем состоянии
		fly = fly && GetAnictrl()->GetWaterLevel() == 0 && GetBodyState() != BS_LIE;

		// возможность обычной анимации боли: 
		// только при нефатальном ударе, при закрытом инвентаре, ненулевом уроне без эффекта полёта
		bool feelsPain = !desc.bIsUnconscious && !desc.bIsDead && !inventory2.IsOpen() && desc.fDamageReal && !fly;

		// игрок не чувствует боли при выполнении потенциально опасного для противников удара
		feelsPain = feelsPain && !(IsSelfPlayer() && (GetAnictrl()->IsInPreHit() || GetAnictrl()->IsInCombo()));

		if (feelsPain)
			for (zCEventMessage* message : GetEM()->messageList)
				if (!message->IsDeleted())
				{
					// не чувствуем боли при смене/выборе экипировки
					if (oCMsgWeapon* weaponMessage = dynamic_cast<oCMsgWeapon*>(message))
						if (weaponMessage->IsInUse())
						{
							feelsPain = false;
							break;
						}

					// не чувствуем боли при добивании лежащего NPC
					if (oCMsgAttack* attackMessage = dynamic_cast<oCMsgAttack*>(message))
						if (attackMessage->IsInUse())
							if (attackMessage->GetSubType() == oCMsgAttack::EV_ATTACKFINISH)
							{
								feelsPain = false;
								break;
							}
				}

		// не чувствуем боли при касте заклинания
		if (feelsPain && GetWeaponMode() == NPC_WEAPON_MAG)
			if (oCSpell* spell = COA(GetSpellBook(), GetSelectedSpell()))
				if (spell->GetSpellStatus() == SPL_STATUS_CAST)
					feelsPain = false;

		if (feelsPain)
		{
			// выбираем между жёстким блокирующим эффектом и простой анимацией
			bool justAni = IsBodyStateInterruptable() && !bodyStateInterruptableOverride;

			// игрок в боевой стойке получает лёгкий эффект
			justAni = justAni || IsSelfPlayer() && GetWeaponMode() != NPC_WEAPON_NONE;

			oCMsgConversation* message = nullptr;

			if (justAni)
				message = new oCMsgConversation(oCMsgConversation::EV_PLAYANI, "T_GOTHIT");
			else
			{
				// очистка ИИ и прерывание текущего действия
				ClearEM();
				Interrupt(false, false);
				SetBodyState(BS_STUMBLE);

				// выбор направления анимации
				zSTRING stumble = "STUMBLE";

				// если атакующий спереди, то выполняется отскок назад
				if (fabsf(desc.fAzimuth) <= 90.0f)
					stumble += "B";

				zSTRING aniName = Z"T_" + GetInterruptPrefix() + stumble;

				// пробуем найти специализированную для заклинания анимацию
				if (desc.pFXHit)
				{
					static Symbol spellFxAniLetters{ parser, "spellFXAniLetters" };

					if (spellFxAniLetters)
					{
						const zSTRING newAniName = Z"T_" + Z spellFxAniLetters.GetValue<string>(desc.pFXHit->GetSpellType()) + stumble;

						if (GetModel()->GetAniIDFromAniName(newAniName) != Invalid)
							aniName = newAniName;
					}
				}

				message = new oCMsgConversation(oCMsgConversation::EV_PLAYANI_NOOVERLAY, aniName);
			}

			// отправляем команду о проигрывании анимации
			message->SetHighPriority(true);
			GetEM()->OnMessage(message, this);
		}

		// запускаем полёт
		if (fly)
		{
			// направление задано только для урона от барьера
			zVEC3 direction = desc.vecDirectionFly;

			if (!HasFlag(desc.enuModeDamage, oEDamageType_Barrier))
			{
				// по умолчанию удар спереди
				zVEC3 from = GetPositionWorld() + GetAtVectorWorld() * 100.0f;

				if (desc.pVobAttacker)
					from = desc.pVobAttacker->GetPositionWorld();
				else
					if (desc.pVobHit)
						from = desc.pVobHit->GetPositionWorld();

				direction = GetPositionWorld() - from;
				direction.Normalize();
			}

			// нет урона от падения при полёте
			overrideFallDownHeight = true;

			// запускаем анимацию
			static_cast<oCAIHuman*>(callback_ai)->StartFlyDamage(desc.aryDamageEffective[oEDamageIndex_Fly] + desc.aryDamageEffective[oEDamageIndex_Barrier], direction);
		}

		// анимация лица при игре со звуком
		if (zsound && !desc.bIsDead && !desc.bIsUnconscious)
			StartFaceAni("VISEME", 1.0f, -2.0f);

		if (!desc.bIsDead)
			return;

		// анимация смерти
		zSTRING deadAni = "T_DEAD";

		// особая анимацию при плавании
		if (GetAnictrl()->GetActionMode() == ANI_ACTION_SWIM || GetAnictrl()->GetActionMode() == ANI_ACTION_DIVE)
			deadAni = "T_DIVE_2_DROWNED";
		else
			// иначе, если удар спереди
			if (fabs(desc.fAzimuth) <= 90.0f)
				deadAni += "B";

		GetModel()->StartAni(deadAni, zCModel::zMDL_STARTANI_DEFAULT);
	}
}
