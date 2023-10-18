namespace std
{
	template <>
	struct hash<pair<::NAMESPACE::zVEC3, ::NAMESPACE::zVEC3>>
	{
		size_t operator()(const pair<::NAMESPACE::zVEC3, ::NAMESPACE::zVEC3>& p) const noexcept
		{
			float sum = 0.0f;

			for (int i = 0; i < 3; i++)
				sum += p.first[i] * 2 - p.second[i];

			return hash<float>{}(sum);
		}
	};

	template <>
	struct hash<::NAMESPACE::zVEC3>
	{
		size_t operator()(const ::NAMESPACE::zVEC3& p) const noexcept
		{
			float sum = 0.0f;

			for (int i = 0; i < 3; i++)
				sum += p[i];

			return hash<float>{}(sum);
		}
	};
}

namespace NAMESPACE
{
	struct MovedVob
	{
		ZOwner<zCVob> vob;
		zVEC3 origin;
		zVEC3 target;
	};

	std::vector<MovedVob> movedVobs;
	std::optional<int> itemIndex;
	bool vobsMoved;
	bool pluginEnabled;

	void SetVobPos(zCVob* vob, const zVEC3& pos)
	{
		const bool collDyn = vob->collDetectionDynamic;
		const bool collStat = vob->collDetectionStatic;

		vob->SetCollDet(false);
		vob->SetPositionWorld(pos);

		vob->SetCollDetDyn(collDyn);
		vob->SetCollDetStat(collStat);
	}

	std::optional<zVEC3> RayTube(zCWorld* world, const zVEC3& origin, const zVEC3& ray, float radius, zCVob* ignoreVob, int rayFlags)
	{
		zMAT4 rayMatrix = Alg_Identity3D();
		rayMatrix.PreScale(zVEC3{ radius });
		rayMatrix.SetAtVector(zVEC3{ ray }.Normalize());
		rayMatrix.SetTranslation(origin);

		constexpr int maxRays = 12;
		std::optional<zVEC3> nearestHit;

		for (int i = 0; i < maxRays; i++)
		{
			const float angle = 2.0f * PI / maxRays * i;
			zVEC3 point{ 0.0f, sinf(angle), cosf(angle) };
			point = rayMatrix * point;

			if (!world->TraceRayNearestHit(point, ray, ignoreVob, rayFlags))
				continue;

			zVEC3 hit = world->traceRayReport.foundIntersection;
			hit = origin + ray * ((hit - origin).Dot(ray) / ray.Dot(ray));

			if (!nearestHit.has_value())
			{
				nearestHit = hit;
				continue;
			}

			if (origin.Distance(hit) < origin.Distance(nearestHit.value()))
				nearestHit = hit;
		}

		return nearestHit;
	}

	zVEC3 GetFocus(zCVob* vob)
	{
		return vob->bbox3D.GetCenter() + (vob->bbox3D.GetCenter() - vob->bbox3D.GetCenterFloor()) * 0.5f;
	}

	bool TryMoveUp(zCVob* vob)
	{
		if (vob->groundPoly ||
			ogame->GetWorld()->TraceRayFirstHit(GetFocus(vob), zVEC3(0, -500, 0), vob,
				zTRACERAY_POLY_IGNORE_TRANSP | zTRACERAY_VOB_IGNORE_CHARACTER))
		{
			return false;
		}

		if (!ogame->GetWorld()->TraceRayNearestHit(vob->GetPositionWorld(), zVEC3(0, 200, 0), vob,
			zTRACERAY_POLY_2SIDED | zTRACERAY_POLY_IGNORE_TRANSP | zTRACERAY_VOB_IGNORE_CHARACTER))
		{
			return false;
		}

		SetVobPos(vob, ogame->GetWorld()->traceRayReport.foundIntersection);
		return true;
	}

	struct RayEntry
	{
		zVEC3 origin;
		zVEC3 ray;
		zVEC3 inter;
	};

	std::vector<RayEntry> entries;

	std::vector<zVEC3> GetVertexes(zCProgMeshProto* visual)
	{
		std::vector<zVEC3> vertexes;
		vertexes.reserve(64u);

		auto& posList{ visual->posList };
		auto& subMeshList{ visual->subMeshList };

		for (int i = 0; i < visual->numSubMeshes; i++)
			for (int p = 0; p < subMeshList[i].triList.GetNum(); p++)
				for (int k = 0; k < 3; k++)
					vertexes.emplace_back(posList[subMeshList[i].wedgeList[subMeshList[i].triList[p].wedge[k]].position]);

		return vertexes;
	}

	void Sort(zVEC3& x, zVEC3& y)
	{
		for (int i = 0; i < 3; i++)
			if (x[i] != y[i])
			{
				if (x[i] > y[i])
					std::swap(x, y);

				return;
			}
	}

	void SplitMesh(const std::vector<zVEC3>& vertexes, float size, std::vector<std::pair<zVEC3, zVEC3>>& segments, std::vector<zVEC3>& points)
	{
		segments.clear();
		points.clear();

		std::unordered_set<std::pair<zVEC3, zVEC3>> segmentSet;
		std::unordered_set<zVEC3> pointSet;

		segmentSet.reserve(512u);
		pointSet.reserve(1024u);
		segments.reserve(4096u);
		points.reserve(16384);

		for (size_t k = 0; k < vertexes.size(); k += 3)
		{
			for (size_t i = 0; i < 3; i++)
			{
				zVEC3 a = vertexes[k + i];

				if (pointSet.insert(a).second)
					points += a;

				zVEC3 b = vertexes[k + (i + 1) % 3];
				Sort(a, b);

				const float distance = a.Distance(b);
				std::pair segment{ a, b };
				
				if (segmentSet.insert(segment).second)
					segments += segment;

				const int steps = distance / size;
				const zVEC3 abStep = (b - a) / (steps + 1);

				for (int step = 1; step <= steps; step++)
				{
					const zVEC3 point = a + abStep * step;
					
					if (pointSet.insert(point).second)
						points += point;
				}
			}

			const zVEC3& a = vertexes[k];
			const zVEC3 ab = vertexes[k + 1] - a;
			const zVEC3 ac = vertexes[k + 2] - a;

			const int steps = static_cast<int>(std::max(ab.Length(), ac.Length()) / size);
			const zVEC3 abStep = ab / (steps + 1);
			const zVEC3 acStep = ac / (steps + 1);

			for (int step = 1; step <= steps; step++)
			{
				const zVEC3 abPos = a + abStep * step;
				const zVEC3 acPos = a + acStep * step;
				segments.emplace_back(abPos, acPos);

				const int substeps = static_cast<int>(abPos.Distance(acPos) / size);
				const zVEC3 abacStep = (acPos - abPos) / (substeps + 1);

				for (int substep = 1; substep <= substeps; substep++)
					points += abPos + abacStep * substep;
			}
		}
	}

	bool TryMoveDownMesh(zCVob* vob)
	{
		zCProgMeshProto* const visual = dynamic_cast<zCProgMeshProto*>(vob->GetVisual());
		
		if (!visual)
			return false;

		std::vector<zVEC3> vertexes = GetVertexes(visual);

		for (zVEC3& vertex : vertexes)
			vertex = vob->trafoObjToWorld * vertex;

		std::vector<std::pair<zVEC3, zVEC3>> segments;
		std::vector<zVEC3> points;

		SplitMesh(vertexes, 3.0f, segments, points);

		// check mesh intersects world
		for (const auto& segment : segments)
			if (ogame->GetWorld()->TraceRayFirstHit(segment.first, segment.second - segment.first, vob, zTRACERAY_STAT_POLY | zTRACERAY_VOB_IGNORE_CHARACTER))
				return false;

		const zVEC3 ray{ 0.0f, -50.0f, 0.0f };
		std::optional<float> minimalShift;

		for (const zVEC3& vertex : points)
		{
			if (!ogame->GetWorld()->TraceRayNearestHit(vertex, ray, vob, zTRACERAY_STAT_POLY | zTRACERAY_VOB_IGNORE_CHARACTER))
				continue;

			const float shift = (vertex - ogame->GetWorld()->traceRayReport.foundIntersection)[VY];

			if (shift < 5.0f)
				return false;

			if (!minimalShift.has_value() || shift < minimalShift.value())
				minimalShift = shift;
		}

		if (!minimalShift.has_value())
			return false; 

		SetVobPos(vob, vob->GetPositionWorld() - zVEC3{ 0.0f, minimalShift.value(), 0.0f });
		return true;
	}

	bool TryMove(zCVob* vob)
	{
		return TryMoveUp(vob) || TryMoveDownMesh(vob);
	}

	Sub clear(ZSUB(GameEvent::LoadBegin, GameEvent::Exit), []
		{
			pluginEnabled = false;
			itemIndex.reset();
			movedVobs.clear();
		});

	void TogglePlugin()
	{
		pluginEnabled = !pluginEnabled;

		if (!pluginEnabled)
			return;
		
		for (oCItem* item : ogame->GetGameWorld()->voblist_items)
			if (!item->HasFlag(ITM_FLAG_NFOCUS) && item->GetHomeWorld())
			{
				MovedVob movedVob;
				movedVob.origin = item->GetPositionWorld();

				if (!TryMove(item))
					continue;

				item->AddRef();
				movedVob.vob.reset(item);
				movedVob.target = item->GetPositionWorld();

				movedVobs += std::move(movedVob);
			}

		vobsMoved = true;
	}

	void ChangeItemIndex(int delta)
	{
		if (movedVobs.empty())
			return;

		const int vobsCount = movedVobs.size();
		auto isInWorld = [](zCVob* vob) { return vob->GetHomeWorld() == ogame->GetWorld(); };
		MovedVob* selectedItem = nullptr;
		int start = 0;

		if (!itemIndex.has_value())
			start = (delta > 0) ? 0 : static_cast<int>(movedVobs.size() - 1);
		else
			start = (itemIndex.value() + vobsCount + delta) % vobsCount;;

		for (int i = 0; i < vobsCount; i++)
		{
			itemIndex = (start + vobsCount + i * delta) % vobsCount;
			MovedVob& currentItem = movedVobs[itemIndex.value()];
			
			if (currentItem.vob->GetHomeWorld() == ogame->GetWorld())
			{
				selectedItem = &currentItem;
				break;
			}
		}

		if (!selectedItem)
		{
			itemIndex.reset();
			return;
		}

		SetVobPos(player, selectedItem->vob->GetPositionWorld() + zVEC3(0.0f, 100.0f, 0.0f));
		COA(zCAICamera::GetCurrent(), ReceiveMsg(zPLAYER_BEAMED));
	}

	void ToggleLift()
	{
		vobsMoved = !vobsMoved;

		for (MovedVob& movedVob : movedVobs)
			if (movedVob.vob->GetHomeWorld())
				SetVobPos(movedVob.vob.get(), vobsMoved ? movedVob.target : movedVob.origin);
	}

	Sub handleKeys(ZSUB(GameEvent::Loop), []
		{
			if (zCConsole::cur_console || ogame->singleStep || !player)
				return;

			if (Options::KeyEnablePlugin->GetToggled())
				return TogglePlugin();

			if (!pluginEnabled)
				return;

			const int delta = Options::KeyGoToPrevItem->GetToggled() ? -1 : Options::KeyGoToNextItem->GetToggled() ? 1 : 0;

			if (delta)
				return ChangeItemIndex(delta);

			if (Options::KeyToggleLift->GetToggled())
				return ToggleLift();
		});

	Sub draw(ZSUB(GameEvent::Loop), []
		{
			if (ogame->singleStep || !pluginEnabled || !player)
				return;

			int y = 3600;
			screen->Print(200, y += 200, Z"Items affected: " + Z static_cast<int>(movedVobs.size()));
			screen->Print(200, y += 200, Z"Selected index: " + (itemIndex.has_value() ? Z itemIndex.value() : Z "-"));
			screen->Print(200, y += 200, Z"Items moved   : " + (vobsMoved ? Z"yes" : Z"no"));

			for (const MovedVob& movedVob : movedVobs)
				if (player->GetPositionWorld().Distance(movedVob.vob->GetPositionWorld()) < 1000)
					zlineCache->Line3D(player->GetPositionWorld(), movedVob.vob->GetPositionWorld(), GFX_RED, true);

			for (const RayEntry& e : entries)
				if (player->GetPositionWorld().Distance(e.origin) < 1000)
				{
					zlineCache->Line3D(e.origin, e.origin + e.ray, GFX_BLUE, true);
					zlineCache->Line3D(e.inter - zVEC3{ -3.0f, 0.0f, 0.0f }, e.inter + zVEC3{ -3.0f, 0.0f, 0.0f }, GFX_GREEN, true);
				}
		});
}