namespace NAMESPACE
{
	void DoFallDamage(oCNpc* target, float height)
	{
		oCNpc::oSDamageDescriptor desc;
		ZeroMemory(&desc, sizeof(desc));

		desc.fDamageMultiplier = 1.0f;
		desc.fDamageTotal = (height + 50.0f - target->GetFallDownHeight()) / 100.0f * target->fallDownDamage;
		desc.enuModeDamage = oEDamageType_Fall;
		desc.dwFieldsValid = oCNpc::oEDamageDescFlag_DamageType | oCNpc::oEDamageDescFlag_Damage;

		target->OnMessage(new oCMsgDamage{ oCMsgDamage::EV_DAMAGE_ONCE, desc }, target);
	}

	void DoFinishDamage(oCNpc* attacker, oCNpc* target)
	{
		target->SetAttribute(NPC_ATR_HITPOINTS, 0);
		target->DoDie(attacker);

		oCNpc::oSDamageDescriptor desc;
		ZeroMemory(&desc, sizeof(desc));
		desc.bIsDead = true;

		target->OnDamage_Sound(desc);
	}

	void SplitDamage(oEDamageType damageType, unsigned long aryDamage[oEDamageIndex_MAX], int totalDamage)
	{
		if (damageType == oEDamageType_Unknown)
			return;

		float divisor = 0.0f;

		for (int i = 0; i < static_cast<int>(oEDamageIndex_MAX); i++)
			if (damageType & (1 << i))
				divisor += 1.0f;

		if (!divisor)
			return;

		const int damage = static_cast<int>(totalDamage / divisor + 0.5f);

		for (int i = 0; i < static_cast<int>(oEDamageIndex_MAX); i++)
			if ((damageType & (1 << i)) && !aryDamage[i])
				aryDamage[i] = damage;
	}

	void DoSpellDamage(oCVisualFX* fx, oCNpc* target, const zVEC3& contactPoint)
	{
		constexpr int COLL_DONOTHING = 0;
		constexpr int COLL_DOEVERYTHING = 1 << 0;
		constexpr int COLL_APPLYHALVEDAMAGE = 1 << 2;
		constexpr int COLL_APPLYDOUBLEDAMAGE = 1 << 3;
		constexpr int COLL_APPLYVICTIMSTATE = 1 << 4;
		constexpr int COLL_DONTKILL = 1 << 5;

		int collideState = COLL_DOEVERYTHING;

		if (Symbol func{ parser, "C_CanNpcCollideWithSpell" })
		{
			ParserScope scope{ parser };
			parser->SetInstance("SELF", target);
			parser->SetInstance("OTHER", dynamic_cast<oCNpc*>(fx->origin));
			collideState = CallParser<int>(parser, func.GetIndex(), fx->GetSpellType());
		}

		if (collideState == COLL_DONOTHING)
			return;

		const float damageScale = (collideState & COLL_APPLYHALVEDAMAGE) ? 0.5f : (collideState & COLL_APPLYDOUBLEDAMAGE) ? 2.0f : 1.0f;

		oCNpc::oSDamageDescriptor desc;
		ZeroMemory(&desc, sizeof(desc));
		desc.fDamageMultiplier = 1.0f;

		desc.fDamageTotal = fx->GetDamage() * fx->GetLevel() * damageScale;
		desc.enuModeDamage = fx->GetDamageType();
		desc.nSpellID = fx->GetSpellType();
		desc.nSpellCat = fx->GetSpellCat();
		desc.nSpellLevel = fx->GetLevel();
		desc.pVobAttacker = fx->inflictor;
		desc.pNpcAttacker = dynamic_cast<oCNpc*>(fx->inflictor);
		desc.pVobHit = target;
		desc.vecLocationHit = contactPoint;
		desc.vecDirectionFly = contactPoint;
		desc.bDamageDontKill = collideState & COLL_DONTKILL;

		if ((collideState & COLL_DOEVERYTHING) || (collideState & COLL_APPLYVICTIMSTATE))
		{
			desc.strVisualFX = (fx->GetDamageType() & oEDamageType_Fire) ? "" : fx->emFXCollDynPerc_S;
			desc.SetFXHit(fx);
		}

		if (fx->emCheckCollision != 2)
		{
			SplitDamage(static_cast<oEDamageType>(fx->GetDamageType()), desc.aryDamage, static_cast<int>(desc.fDamageTotal));
			target->GetEM()->OnMessage(new oCMsgDamage{ oCMsgDamage::EV_DAMAGE_ONCE, desc }, nullptr);
		}

		if (fx->emFXCollDynPerc_S.IsEmpty() || !(fx->GetDamageType() & oEDamageType_Fire))
			return;

		ZOwner<oCVisualFX> dynFx{ fx->CreateAndCastFX(fx->emFXCollDynPerc_S, target, fx->origin) };

		if (!dynFx || !dynFx->IsLooping())
			return;

		int ticksPerDamagePoint = 1000;

		if (Symbol symbol{ parser, "NPC_BURN_TICKS_PER_DAMAGE_POINT" })
			ticksPerDamagePoint = symbol.GetValue<int>(0);

		dynFx->SetDuration(ticksPerDamagePoint * dynFx->GetDamage() / 1000.0f);
	}

	void DoHitDamage(oCNpc* attacker, oCNpc* target)
	{
		oCNpc::oSDamageDescriptor desc;
		ZeroMemory(&desc, sizeof(desc));
		desc.fDamageMultiplier = 1.0f;

		switch (attacker->GetWeaponMode())
		{
		case NPC_WEAPON_FIST:
			desc.enuModeWeapon = oETypeWeapon_Fist;
			break;

		case NPC_WEAPON_DAG:
		case NPC_WEAPON_1HS:
		case NPC_WEAPON_2HS:
			desc.enuModeWeapon = oETypeWeapon_Melee;
			break;

		case NPC_WEAPON_BOW:
		case NPC_WEAPON_CBOW:
			desc.enuModeWeapon = oETypeWeapon_Range;
			break;

		case NPC_WEAPON_MAG:
			desc.enuModeWeapon = oETypeWeapon_Magic;
			break;
		}

		desc.pItemWeapon = attacker->GetWeapon();
		desc.pVobHit = desc.pItemWeapon;
		desc.pVobAttacker = attacker;
		desc.pNpcAttacker = attacker;
		desc.fDamageMultiplier = attacker->GetDamageMultiplier();
		desc.vecLocationHit = attacker->anictrl->hitPosUsed ? attacker->anictrl->hitpos : target->GetPositionWorld();

		desc.dwFieldsValid = oCNpc::oEDamageDescFlag_Damage
			| oCNpc::oEDamageDescFlag_DamageType
			| oCNpc::oEDamageDescFlag_WeaponType
			| oCNpc::oEDamageDescFlag_Attacker
			| oCNpc::oEDamageDescFlag_Npc
			| oCNpc::oEDamageDescFlag_Inflictor
			| oCNpc::oEDamageDescFlag_Weapon;

		if (oCItem* weapon = desc.pItemWeapon)
		{
			desc.fDamageTotal = weapon->GetFullDamage();
			desc.enuModeDamage = weapon->GetDamageTypes();
			weapon->GetDamages(desc.aryDamage);
		}
		else
		{
			desc.fDamageTotal = attacker->GetFullDamage();
			desc.enuModeDamage = attacker->damagetype;
			memcpy(desc.aryDamage, attacker->damage, sizeof(attacker->damage));
		}

		target->OnDamage(desc);
	}
}
