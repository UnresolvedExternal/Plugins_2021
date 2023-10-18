namespace NAMESPACE
{
	FASTHOOK_PATCH_OPT(oCNpc, OnDamage_Condition, enablePlugin);

	void oCNpc::OnDamage_Condition_Union(oSDamageDescriptor& desc)
	{
		// такого не должно быть, но всё же...
		if (!GetAnictrl())
			return;

		// бессознательность имеет специфические условия,
		// поэтому пока ставим false
		desc.bIsUnconscious = false;

		// инициализируем смертельность удара
		desc.bIsDead = IsDead();

		// если здоровья ещё много - не переходим ни в одно из состояний
		if (GetAttribute(NPC_ATR_HITPOINTS) > 1)
			return;

		// если нету атакующего НПС или жертва в воде, то бессознательное состояние недопустимо,
		// а смертельность повреждения не может измениться
		if (!desc.pNpcAttacker || GetAnictrl()->IsInWater())
			return;

		const bool hasBlunt = HasFlag(desc.enuModeDamage, oEDamageType_Blunt);
		const bool hasEdge = HasFlag(desc.enuModeDamage, oEDamageType_Edge);

		// замена смерти на бессознательное состояние возможна только в случаях, когда
		// есть дробящий урон или есть рубящий урон от человека или установлен флаг bDamageDontKill
		if (desc.bIsDead && !desc.bDamageDontKill && !hasBlunt && !(desc.pNpcAttacker->IsHuman() && hasEdge))
			return;

		static const int C_DropUnconscious = parser->GetIndex("C_DropUnconscious");

		if (C_DropUnconscious != Invalid)
		{
			ParserScope scope{ parser };
			static Symbol self{ parser, "SELF" };
			static Symbol other{ parser, "OTHER" };

			self.SetValue(0, reinterpret_cast<int>(this));
			other.SetValue(0, reinterpret_cast<int>(desc.pNpcAttacker));

			// вызываем func int C_DropUnconscious()
			desc.bIsUnconscious = CallParser<bool>(parser, C_DropUnconscious);
		}
		else
			// если функции нет, то вводим в бессознательное состояние людей
			desc.bIsUnconscious = IsHuman();

		// если определили бессознательное состояние, то отменяем смерть
		if (desc.bIsUnconscious)
		{
			desc.bIsDead = false;

			// закрываем инвентарь ГГ
			if (IsSelfPlayer())
				CloseInventory();
		}
	}
}
