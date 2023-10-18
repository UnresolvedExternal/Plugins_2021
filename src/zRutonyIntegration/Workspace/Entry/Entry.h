#include <nlohmann/json.hpp>
#include "ThreadPool.h"
#include "JsonSettings.h"

using json = nlohmann::json;

namespace NAMESPACE
{
	struct Donate
	{
		int index;
		double sum;
		std::string currency;

		Donate(json donate)
		{
			index = donate["Index"].get<int>();
			sum = donate["Sum"].get<double>();
			currency = donate["Currency"].get<std::string>();
		}
	};

	std::queue<Donate> donates;
	std::optional<std::future<void>> future;
	std::optional<JsonSettings> settings;

	ThreadPool* pool;

	Sub clearPool(ZSUB(GameEvent::Exit), []()
		{
			if (pool)
				delete pool;
		});

	Sub loadSettings(ZSUB(GameEvent::Init), []()
		{
			zSTRING path = zoptions->GetDirString(DIR_ROOT) + zoptions->GetDirString(DIR_SYSTEM) + PROJECT_NAME + ".json";
			settings.emplace(path.ToChar());
			settings->Load();
		});

	void CheckDonates(int minIndex)
	{
		std::ifstream in(settings->root["RutonyPath"].get<std::string>() + "Data\\Data.json");

		if (!in)
			return;

		std::vector<Donate> newDonates;

		try
		{
			json data;
			in >> data;

			if (!in)
				return;

			json donateList = data["ListDonates"];

			for (auto it = donateList.rbegin(); it != donateList.rend(); it++)
			{
				newDonates.emplace_back(*it);

				if (newDonates.back().index < minIndex)
				{
					newDonates.pop_back();
					break;
				}
			}
		}
		catch (...)
		{
			return;
		}

		for (auto it = newDonates.rbegin(); it != newDonates.rend(); it++)
			donates.push(std::move(*it));
	}

	Sub checkDonates(ZSUB(GameEvent::Loop), []()
		{
			static Symbol func{ parser, "Rutony_ExecuteDonate" };
			static Timer timer;

			if (func.GetType() != Symbol::Type::Func)
				return;

			if (future)
			{
				if (future->wait_for(std::chrono::seconds(0)) != std::future_status::ready)
					return;

				future.reset();
			}

			if (!donates.empty())
			{
				if (!ztimer->frameTime || ogame->singleStep)
					return;

				Donate donate = std::move(donates.front());
				donates.pop();

				CallParser<void>(parser, func.GetIndex(), static_cast<int>(donate.sum * 100 + 0.5), donate.currency.c_str());

				settings->root["LastDonateIndex"] = donate.index;
				future = pool->Execute(&JsonSettings::Save, *settings);
				return;
			}

			if (!timer[0u].Await(500, true))
				return;

			if (!pool)
				pool = new ThreadPool{ 1 };

			future = pool->Execute(&CheckDonates, settings->root["LastDonateIndex"].get<int>() + 1);
		});
}