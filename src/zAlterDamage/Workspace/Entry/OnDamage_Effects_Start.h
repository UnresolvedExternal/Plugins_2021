namespace NAMESPACE
{
	FASTHOOK_PATCH_OPT(oCNpc, OnDamage_Effects_Start, enablePlugin);

	// выделил создание эффектов крови в отдельную функцию
	void oCNpc::OnDamage_CreateBlood(oSDamageDescriptor& desc)
	{
		// проверяем, активирован ли режим крови для данного NPC
		if (!bloodEnabled)
			return;

		// проверяем режим крови (общий для всех NPC)
		if (modeBlood < oEBloodMode_Particles)
			return;

		// создаём частицы крови, если NPC находится в мире
		if (GetHomeWorld())
		{
			// создаём воб с визуалом из частиц
			ZOwner<zCParticleFX> visual{ new zCParticleFX{} };
			ZOwner<zCVob> vob{ new zCVob{} };

			vob->SetCollDet(false);
			vob->SetPositionWorld(desc.vecLocationHit);
			vob->SetVisual(visual.get());

			GetHomeWorld()->AddVob(vob.get());

			visual->SetAndStartEmitter(bloodEmitter, false);
		}

		// проверяем, нужно ли создавать лужу крови
		if (modeBlood < oEBloodMode_Decals || !GetAnictrl())
			return;

		// направление и cила потока крови
		zVEC3 bloodRay{ 0.0f, -1.0f, 0.0f };

		if (desc.pNpcAttacker)
		{
			bloodRay = GetPositionWorld() - desc.pNpcAttacker->GetPositionWorld();
			bloodRay.Normalize();
		}

		bloodRay *= bloodDistance;

		// визуальный размер крови
		float bloodSize = 0.0f;

		if (Symbol bloodDamageMax{ parser, "BLOOD_DAMAGE_MAX" })
			bloodSize = std::max<float>(desc.fDamageEffective, bloodDamageMax.GetValue<int>(0));

		bloodSize *= bloodAmount;

		if (Symbol bloodSizeDivisor{ parser, "BLOOD_SIZE_DIVISOR" })
			if (bloodSizeDivisor.GetValue<int>(0) > 0)
				bloodSize /= bloodSizeDivisor.GetValue<int>(0);

		// рандомизированное усиление крови не более 10%
		bloodSize *= 1.0f + 0.1f * rand() / static_cast<float>(RAND_MAX);

		GetAnictrl()->AddBlood(GetPositionWorld(), bloodRay, bloodSize, bloodFlow, &bloodTexture);
	}

	void oCNpc::OnDamage_Effects_Start_Union(oSDamageDescriptor& desc)
	{
		if (!GetAnictrl())
			return;

		// проверка условий создания эффектов крови
		if
		(
			desc.fDamageEffective > 0.0f &&
			!HasFlag(NPC_FLAG_IMMORTAL) &&
			!HasFlag(desc.enuModeDamage, oEDamageType_Fire) &&
			!HasFlag(desc.enuModeWeapon, oETypeWeapon_Fist) &&
			!COA(desc.pNpcAttacker, IsMonster())
		)
		{
			OnDamage_CreateBlood(desc);
		}

		// проверка условий начала горения огнём
		if
		(
			HasFlag(desc.enuModeDamage, oEDamageType_Fire) &&
			desc.aryDamageEffective[oEDamageIndex_Fire] >= 2.0f &&
			!HasBodyStateModifier(BS_MOD_BURNING) &&
			GetAnictrl()->GetWaterLevel() == 0
		)
		{
			// устанавливаем имя визуала горения
			desc.strVisualFX = "VOB_BURN";

			// урон за каждый тик горения
			desc.fDamagePerInterval = 10.0f;

			if (Symbol damagePerInterval{ parser, "NPC_BURN_DAMAGE_POINTS_PER_INTERVALL" })
				desc.fDamagePerInterval = damagePerInterval.GetValue<int>(0);

			// время горения за каждую единицу урона
			float ticksPerDamagePoint = 1000.0f;

			if (Symbol ticksPerDamagePointSymbol{ parser, "NPC_BURN_TICKS_PER_DAMAGE_POINT" })
				ticksPerDamagePoint = ticksPerDamagePointSymbol.GetValue<int>(0);

			// общее время горения
			desc.fTimeDuration = ticksPerDamagePoint * desc.aryDamageEffective[oEDamageIndex_Fire];

			// интервал между нанесением урона
			desc.fTimeInterval = ticksPerDamagePoint;

			desc.enuModeDamage = oEDamageType_Fire;
			std::fill(desc.aryDamage, desc.aryDamage + oEDamageIndex_MAX, 0);
		}

		// проверяем визуал периодического урона
		if (desc.strVisualFX.IsEmpty())
			return;

		// находим активное сообщение периодического урона
		oCMsgDamage* activeDot = nullptr;

		for (zCEventMessage* message : GetEM()->messageList)
			if (oCMsgDamage* dotMessage = dynamic_cast<oCMsgDamage*>(message))
				if (!dotMessage->IsDeleted() && dotMessage->GetSubType() == oCMsgDamage::EV_DAMAGE_PER_FRAME)
				{
					activeDot = dotMessage;
					break;
				}

		// уничтожаем активный периодический урон
		if (activeDot)
		{
			// убираем реакции восприятия старого визуала
			if (oCVisualFX* visual = activeDot->descDamage.pVisualFX)
				visual->SetSendsAssessMagic(false);

			// удаляем сообщение
			OnDamage_Effects_End(activeDot->descDamage);
			activeDot->Delete();
		}

		// создаём визуал периодического урона
		ZOwner<oCVisualFX> visual{ new oCVisualFX{} };
		visual->SetPositionWorld(GetPositionWorld());
		
		if (GetHomeWorld())
			GetHomeWorld()->AddVob(visual.get());

		visual->SetLevel(desc.nSpellLevel, false);
		visual->SetDamage(desc.fDamageTotal);
		visual->SetDamageType(desc.enuModeDamage);
		visual->SetSpellType(desc.nSpellID);
		visual->SetSpellCat(desc.nSpellCat);
		visual->SetByScript(desc.strVisualFX);
		visual->Init(this, 0, desc.pVobAttacker);
		visual->Cast(true);

		desc.SetVisualFX(visual.get());

		// создаём новый дескриптор для периодического урона
		oSDamageDescriptor dotDesc;
		ZeroMemory(&dotDesc, sizeof(dotDesc));

		// если время периодического урона не задано, то урон не наносится
		if (desc.fTimeDuration == 0.0f)
		{
			dotDesc.SetVisualFX(desc.pVisualFX);
			dotDesc.fTimeDuration = std::numeric_limits<float>::max();
			dotDesc.fTimeInterval = 1000.0f;
		}
		// иначе копируем все значения
		else
			dotDesc = desc;

		// отправляем сообщение о периодическом уроне в менеджер событий
		activeDot = new oCMsgDamage{ oCMsgDamage::EV_DAMAGE_PER_FRAME, dotDesc };
		activeDot->SetHighPriority(true);
		GetEM()->OnMessage(activeDot, this);
	}
}
