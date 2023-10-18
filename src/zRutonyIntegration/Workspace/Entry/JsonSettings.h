#pragma once

#include <fstream>
#include <thread>
#include <iomanip>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

class JsonSettings
{
private:
	const std::string path;

public:
	json root;

	JsonSettings(const std::string& path) :
		path{ path }
	{

	}

	void Load()
	{
		for (int attempt = 0; attempt < 3; attempt++)
		{
			if (attempt)
				std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(pow(attempt, 9) + 0.5)));

			std::ifstream in{ path };

			if (!in)
				continue;

			try
			{
				in >> root;
				return;
			}
			catch (std::exception& e)
			{
				cmd << e.what() << endl;
				continue;
			}
		}

		throw std::exception("Cannot load json settings");
	}

	void Save()
	{
		for (int attempt = 0; attempt < 3; attempt++)
		{
			if (attempt)
				std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(pow(attempt, 9) + 0.5)));

			std::ofstream out{ path };

			if (!out)
				continue;

			try
			{
				out << std::setw(4) << root;
				out.flush();

				if (!out)
					continue;

				return;
			}
			catch (...)
			{
				continue;
			}
		}

		throw std::exception("Cannot wtite json settings");
	}
};