namespace NAMESPACE
{
	FASTHOOK_PATCH_OPT(oCNpc, OnDamage_Anim, enablePlugin);

	void oCNpc::OnDamage_Anim_Union(oSDamageDescriptor& desc)
	{
		// �������� ������� ������, �� � ����������� ��������
		if (!dynamic_cast<oCAIHuman*>(callback_ai) || !GetAnictrl() || !GetModel())
			return;

		// ��� �������� ����� �� ������� �� ������������� ������� �������������� ��������
		if (HasFlag(desc.enuModeDamage, oEDamageType_Fall))
			return;

		// ������� ���� � �������� �� ������ � ����������
		if (desc.pVobAttacker)
			GetAngles(desc.pVobAttacker, desc.fAzimuth, desc.fElevation);

		// ���������� ������
		if (GetWeaponMode() == NPC_WEAPON_BOW)
			if (zCMorphMesh* mesh = dynamic_cast<zCMorphMesh*>(GetWeapon()))
				mesh->StartAni("S_RELAX", 1.0f, -2.0f);

		// ��� ����� �� ����� ��� �� ������� ������ ������ ��������...
		bool fly = roundf(desc.aryDamageEffective[oEDamageIndex_Barrier]) + roundf(desc.aryDamageEffective[oEDamageIndex_Fly]);

		// ... �� �� � ���� � �� � ������� ���������
		fly = fly && GetAnictrl()->GetWaterLevel() == 0 && GetBodyState() != BS_LIE;

		// ����������� ������� �������� ����: 
		// ������ ��� ����������� �����, ��� �������� ���������, ��������� ����� ��� ������� �����
		bool feelsPain = !desc.bIsUnconscious && !desc.bIsDead && !inventory2.IsOpen() && desc.fDamageReal && !fly;

		// ����� �� ��������� ���� ��� ���������� ������������ �������� ��� ����������� �����
		feelsPain = feelsPain && !(IsSelfPlayer() && (GetAnictrl()->IsInPreHit() || GetAnictrl()->IsInCombo()));

		if (feelsPain)
			for (zCEventMessage* message : GetEM()->messageList)
				if (!message->IsDeleted())
				{
					// �� ��������� ���� ��� �����/������ ����������
					if (oCMsgWeapon* weaponMessage = dynamic_cast<oCMsgWeapon*>(message))
						if (weaponMessage->IsInUse())
						{
							feelsPain = false;
							break;
						}

					// �� ��������� ���� ��� ��������� �������� NPC
					if (oCMsgAttack* attackMessage = dynamic_cast<oCMsgAttack*>(message))
						if (attackMessage->IsInUse())
							if (attackMessage->GetSubType() == oCMsgAttack::EV_ATTACKFINISH)
							{
								feelsPain = false;
								break;
							}
				}

		// �� ��������� ���� ��� ����� ����������
		if (feelsPain && GetWeaponMode() == NPC_WEAPON_MAG)
			if (oCSpell* spell = COA(GetSpellBook(), GetSelectedSpell()))
				if (spell->GetSpellStatus() == SPL_STATUS_CAST)
					feelsPain = false;

		if (feelsPain)
		{
			// �������� ����� ������ ����������� �������� � ������� ���������
			bool justAni = IsBodyStateInterruptable() && !bodyStateInterruptableOverride;

			// ����� � ������ ������ �������� ����� ������
			justAni = justAni || IsSelfPlayer() && GetWeaponMode() != NPC_WEAPON_NONE;

			oCMsgConversation* message = nullptr;

			if (justAni)
				message = new oCMsgConversation(oCMsgConversation::EV_PLAYANI, "T_GOTHIT");
			else
			{
				// ������� �� � ���������� �������� ��������
				ClearEM();
				Interrupt(false, false);
				SetBodyState(BS_STUMBLE);

				// ����� ����������� ��������
				zSTRING stumble = "STUMBLE";

				// ���� ��������� �������, �� ����������� ������ �����
				if (fabsf(desc.fAzimuth) <= 90.0f)
					stumble += "B";

				zSTRING aniName = Z"T_" + GetInterruptPrefix() + stumble;

				// ������� ����� ������������������ ��� ���������� ��������
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

			// ���������� ������� � ������������ ��������
			message->SetHighPriority(true);
			GetEM()->OnMessage(message, this);
		}

		// ��������� ����
		if (fly)
		{
			// ����������� ������ ������ ��� ����� �� �������
			zVEC3 direction = desc.vecDirectionFly;

			if (!HasFlag(desc.enuModeDamage, oEDamageType_Barrier))
			{
				// �� ��������� ���� �������
				zVEC3 from = GetPositionWorld() + GetAtVectorWorld() * 100.0f;

				if (desc.pVobAttacker)
					from = desc.pVobAttacker->GetPositionWorld();
				else
					if (desc.pVobHit)
						from = desc.pVobHit->GetPositionWorld();

				direction = GetPositionWorld() - from;
				direction.Normalize();
			}

			// ��� ����� �� ������� ��� �����
			overrideFallDownHeight = true;

			// ��������� ��������
			static_cast<oCAIHuman*>(callback_ai)->StartFlyDamage(desc.aryDamageEffective[oEDamageIndex_Fly] + desc.aryDamageEffective[oEDamageIndex_Barrier], direction);
		}

		// �������� ���� ��� ���� �� ������
		if (zsound && !desc.bIsDead && !desc.bIsUnconscious)
			StartFaceAni("VISEME", 1.0f, -2.0f);

		if (!desc.bIsDead)
			return;

		// �������� ������
		zSTRING deadAni = "T_DEAD";

		// ������ �������� ��� ��������
		if (GetAnictrl()->GetActionMode() == ANI_ACTION_SWIM || GetAnictrl()->GetActionMode() == ANI_ACTION_DIVE)
			deadAni = "T_DIVE_2_DROWNED";
		else
			// �����, ���� ���� �������
			if (fabs(desc.fAzimuth) <= 90.0f)
				deadAni += "B";

		GetModel()->StartAni(deadAni, zCModel::zMDL_STARTANI_DEFAULT);
	}
}
