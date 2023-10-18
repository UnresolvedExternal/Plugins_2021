namespace NAMESPACE
{
	FASTHOOK_PATCH_OPT(oCNpc, OnDamage_Hit, enablePlugin);

	// вспомогательная функция
	// делит урон totalDamage между типами из damageType
	void SplitDamage(oEDamageType damageType, unsigned long aryDamage[oEDamageIndex_MAX], float& totalDamage)
	{
		if (damageType == oEDamageType_Unknown)
			return;

		float divisor = 0.0f;

		// считаем количество типов урона
		for (int i = 0; i < oEDamageIndex_MAX; i++)
			if (damageType & (1 << i))
				divisor += 1.0f;

		if (!divisor)
			return;

		// количество урона на каждый тип
		const int damage = static_cast<int>(totalDamage / divisor + 0.5f);
		totalDamage = 0;

		// записываем урон в aryDamage, если там нуль
		for (int i = 0; i < oEDamageIndex_MAX; i++)
			if ((damageType & (1 << i)) && !aryDamage[i])
			{
				aryDamage[i] = damage;
				totalDamage += damage;
			}
	}

	void oCNpc::OnDamage_Hit_Union(oSDamageDescriptor& desc)
	{
		// применяем множитель урона
		for (int i = 0; i < oEDamageIndex_MAX; i++)
			desc.aryDamage[i] = static_cast<int>(desc.aryDamage[i] * desc.fDamageMultiplier);

		desc.fDamageTotal *= desc.fDamageMultiplier;

		// даём определение получеловека через принадлежность к гильдиям
		const bool isSemiHuman = desc.pNpcAttacker &&
			(desc.pNpcAttacker->IsHuman() || desc.pNpcAttacker->IsOrc() || desc.pNpcAttacker->IsGoblin() || desc.pNpcAttacker->IsSkeleton());

		bool divideDamage = true;

		for (int i = 0; i < oEDamageIndex_MAX && divideDamage; i++)
			if (HasFlag(desc.enuModeDamage, 1 << i) && desc.aryDamage[i])
				divideDamage = false;

		// если в aryDamage нули, то заполняем его распределяя fDamageTotal по активным типам урона
		if (divideDamage)
		{
			// если атакующий NPC не получеловек, атака не магическая и общий урон не задан, то используем силу как урон
			// примечание: pFXHit не будет задан при магической атаке, если C_CanNpcCollideWithSpell не вернула
			// ни флага COLL_DOEVERYTHING, ни COLL_APPLYVICTIMSTATE
			if (desc.pNpcAttacker && !isSemiHuman && !desc.pFXHit && !desc.fDamageTotal)
				desc.fDamageTotal = desc.pNpcAttacker->GetAttribute(NPC_ATR_STRENGTH);

			// распределить общий урон между активными типами урона
			SplitDamage(static_cast<oEDamageType>(desc.enuModeDamage), desc.aryDamage, desc.fDamageTotal);
		}

		// для получеловека увеличиваем сырой урон собственным уроном
		if (isSemiHuman)
			for (int i = 0; i < oEDamageIndex_MAX; i++)
				desc.aryDamage[i] += desc.pNpcAttacker->GetDamageByIndex(static_cast<oEIndexDamage>(i));

		// если немагическая атака сделана получеловеком
		if (!desc.pFXHit && isSemiHuman)
		{
			// делим атрибуты между физическими типами урона
			int divisor =
				(HasFlag(desc.enuModeDamage, oEDamageType_Blunt) ? 1 : 0) +
				(HasFlag(desc.enuModeDamage, oEDamageType_Edge) ? 1 : 0) +
				(HasFlag(desc.enuModeDamage, oEDamageType_Point) ? 1 : 0);

			// монстр (скелет или гоблин в данном случае) без физического урона будет иметь рубящий урон
			if (desc.pNpcAttacker->IsMonster() && !divisor)
			{
				desc.enuModeDamage |= oEDamageType_Edge;
				divisor = 1;
			}

			// делим атрибуты между физическими типами урона
			if (divisor)
			{
				const int strength = static_cast<int>(desc.pNpcAttacker->GetAttribute(NPC_ATR_STRENGTH) / static_cast<float>(divisor));
				const int dexterity = static_cast<int>(desc.pNpcAttacker->GetAttribute(NPC_ATR_DEXTERITY) / static_cast<float>(divisor));

				if (HasFlag(desc.enuModeDamage, oEDamageType_Blunt))
					desc.aryDamage[oEDamageIndex_Blunt] += strength;

				if (HasFlag(desc.enuModeDamage, oEDamageType_Edge))
					desc.aryDamage[oEDamageIndex_Edge] += strength;

				// для колющего - ловкость
				if (HasFlag(desc.enuModeDamage, oEDamageType_Point))
					desc.aryDamage[oEDamageIndex_Point] += dexterity;
			}
		}

		// учёт защиты
		bool immortal = true;
		int damageTotal = 0;

		for (int i = 0; i < oEDamageIndex_MAX; i++)
		{
			if (!HasFlag(desc.enuModeDamage, 1 << i))
			{
				desc.aryDamageEffective[i] = 0;
				continue;
			}

			const int protection = GetProtectionByIndex(static_cast<oEDamageIndex>(i));
			immortal = immortal && protection < 0; // неуязвим к атаке, если неуязвим к каждому из активных типов урона

			// вычитаем защиту
			desc.aryDamageEffective[i] = (protection < 0) ? 0 : std::max(static_cast<int>(desc.aryDamage[i]) - protection, 0);

			// суммируем наносимый урон
			damageTotal += desc.aryDamageEffective[i];
		}

		immortal = immortal || HasFlag(NPC_FLAG_IMMORTAL);
		desc.fDamageTotal = immortal ? 0 : damageTotal;
		desc.fDamageEffective = desc.fDamageTotal;
		desc.fDamageReal = desc.fDamageTotal;

		// определяем, критический урон или нет
		// всегда крит, если атака магическая или атакующий - не NPC или атакующий - монстр или атака не в ближнем бою с оружием
		bool hasHit = desc.pVisualFX || !desc.pNpcAttacker || desc.pNpcAttacker->IsMonster() || desc.enuModeWeapon != oETypeWeapon_Melee;

		// если промах возможен, определяем шанс попадания и само попадание
		if (!hasHit)
		{
			int talentNr = Invalid;

			switch (desc.pNpcAttacker->GetWeaponMode())
			{
			case NPC_WEAPON_1HS:
			case NPC_WEAPON_DAG:
				talentNr = NPC_HITCHANCE_1H;
				break;

			case NPC_WEAPON_2HS:
				talentNr = NPC_HITCHANCE_2H;
				break;
			}

			if (talentNr == Invalid)
				hasHit = true;
			else
			{
				const int hitChance = desc.pNpcAttacker->GetHitChance(talentNr);
				hasHit = rand() / static_cast<float>(RAND_MAX + 1) < hitChance / 100.0f;
			}
		}

		int damage = static_cast<int>(desc.fDamageTotal);

		// делим урон, в случае промаха
		if (!hasHit)
		{
			static Symbol npcMinimalPercent{ parser, "NPC_MINIMAL_PERCENT" };
			static const float missMultiplier = npcMinimalPercent ? npcMinimalPercent.GetValue<int>(0) / 100.0f : 0.0f;
			damage = static_cast<int>(damage * missMultiplier);
		}

		// применяем минимальный урон, если атака не магическая
		if (!desc.pFXHit)
		{
			static Symbol npcMinimalDamage{ parser, "NPC_MINIMAL_DAMAGE" };
			static const int minimalDamage = npcMinimalDamage ? npcMinimalDamage.GetValue<int>(0) : 0;

			if (damage < minimalDamage)
			{
				damage = minimalDamage;
				desc.fDamageReal = minimalDamage;
			}
		}

		// барьер смертелен для плывущего NPC
		if (HasFlag(desc.enuModeDamage, oEDamageType_Barrier) && COA(GetAnictrl(), GetWaterLevel()) >= 2)
		{
			damage = GetAttribute(NPC_ATR_HITPOINTS);
			desc.fDamageReal = damage;
		}

		// уменьшаем хп, если цель уязвима
		if (!immortal)
			ChangeAttribute(NPC_ATR_HITPOINTS, -damage);

		// сбрасываем таймеры регенерации
		hpHeal = GetAttribute(NPC_ATR_REGENERATEHP) * 1000.0f;
		manaHeal = GetAttribute(NPC_ATR_REGENERATEMANA) * 1000.0f;
	}
}
