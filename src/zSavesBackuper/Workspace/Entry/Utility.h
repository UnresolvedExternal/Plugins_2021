#include <filesystem>
#include <iomanip>
#include <ShellAPI.h>

namespace fs = std::filesystem;

namespace NAMESPACE
{
	void LogWarning(const std::string& message)
	{
		cmd << Col16{ CMD_YELLOW } << message << Col16{} << endl;
	}

	void LogError(const std::string& message)
	{
		cmd << Col16{ CMD_RED } << message << Col16{} << endl;
	}

	template <class TOperation, class... TArgs>
	bool DurableOperation(TOperation operation, TArgs&&... args)
	{
		auto& code = (std::forward<TArgs>(args), ...);

		constexpr size_t attempts = 3;

		for (size_t i = 0; i < attempts; i++)
		{
			if (i == 1)
				Sleep(1);

			if (i > 1)
				Sleep(100 * (i - 1));

			code.clear();

			if (!operation(std::forward<TArgs>(args)...))
				continue;

			ASSERT(!code);
			return true;
		}

		return false;
	}

	bool CopyDirContent_SHOperation(const fs::path& from, const fs::path& to)
	{
		SHFILEOPSTRUCT sh{};
		sh.fFlags = FOF_SILENT | FOF_NOCONFIRMATION | FOF_NOERRORUI | FOF_NOCONFIRMMKDIR;
		sh.wFunc = FO_COPY;

		TCHAR newFrom[MAX_PATH];
		std::string fromString = from.string() + "*";
		std::replace(fromString.begin(), fromString.end(), '/', '\\');
		std::copy_n(fromString.begin(), fromString.size() + 1, newFrom);
		newFrom[fromString.size() + 2] = '\0';
		sh.pFrom = newFrom;

		TCHAR newTo[MAX_PATH];
		std::string toString = to.string();
		std::replace(toString.begin(), toString.end(), '/', '\\');
		std::copy_n(toString.begin(), toString.size() + 1, newTo);
		newTo[toString.size() + 2] = '\0';
		sh.pTo = newTo;

		const int result = SHFileOperation(&sh);
		return result == 0;
	}

	bool CopyDirContent_Filestream(const fs::path& from, const fs::path& to, std::error_code& code)
	{
		if (code)
			return false;

		for (const fs::directory_entry& entry : fs::directory_iterator(from, code))
		{
			if (code)
				return false;

			if (entry.is_directory(code))
			{
				fs::path path = to;
				path /= entry.path().filename();

				if (!fs::create_directory(path, code))
					return false;

				if (!CopyDirContent_Filestream(entry.path(), path, code))
					return false;
			}

			if (!entry.is_regular_file(code))
				continue;

			std::string fromString = entry.path().string();
			std::replace(fromString.begin(), fromString.end(), '/', '\\');
			
			std::string toString = (to / entry.path().filename()).string();
			std::replace(toString.begin(), toString.end(), '/', '\\');

			std::ifstream from(fromString, std::ios::binary);
			std::ofstream to(toString, std::ios::binary);

			if (from.peek() != std::ifstream::traits_type::eof())
				to << from.rdbuf();

			if (!from || !to)
				return false;
		}

		return !code;
	}

	bool CopyDirContent_Durable(const fs::path& from, const fs::path& to, std::error_code& code)
	{
		switch (Options::CopyAlgorithm)
		{
		case 1: return DurableOperation([&](auto& code) { return CopyDirContent_SHOperation(from, to); }, code);
		case 2: return DurableOperation([&](auto& code) { return CopyDirContent_Filestream(from, to, code); }, code);
		default: return DurableOperation([&](auto& code) { std::filesystem::copy(from, to, std::filesystem::copy_options::recursive, code); return !code; }, code);
		}

		return false;
	}

	bool IsPathCharValid(char ch)
	{
#define TEST_RANGE(from, to) { if (ch >= from && ch <= to) return true; }

		TEST_RANGE('a', 'z');
		TEST_RANGE('A', 'Z');
		TEST_RANGE('0', '9');

		if (ch == ' ' || ch == '.' || ch == '-' || ch == '_')
			return true;

		return false;

#undef TEST_RANGE
	}

	std::string GenerateSavegameName(uint64_t number, oCSavegameInfo* info)
	{
		std::ostringstream nameBuilder;

		nameBuilder << std::setfill('0');
		nameBuilder << std::setw(4) << number;

		if (!info)
			return nameBuilder.str();

		nameBuilder << "." << std::setw(3) << info->m_TimeDay << "d";
		nameBuilder << "." << std::setw(2) << info->m_TimeHour << "h";
		nameBuilder << "." << std::setw(2) << info->m_TimeMin << "m";

		if (!info->m_SlotNr)
		{
			nameBuilder << ".quicksave";
			return nameBuilder.str();
		}

		size_t nameLength = info->m_Name.Length();

		while (nameLength > 0 && (info->m_Name[nameLength - 1u] == ' ' || info->m_Name[nameLength - 1u] == '.'))
			nameLength -= 1;

		nameBuilder << ".";
		bool lastCharWasValid = false;

		for (size_t i = 0; i < nameLength; i++)
		{
			const char ch = info->m_Name[i];
			lastCharWasValid = IsPathCharValid(ch);

			if (lastCharWasValid)
				nameBuilder << ch;

			if (ch == ' ' || ch == '.')
				lastCharWasValid = false;
		}

		if (!lastCharWasValid)
			nameBuilder << "-";

		return nameBuilder.str();
	}

	std::filesystem::path GetPath(oCSavegameInfo* info)
	{
		return (zoptions->GetDirString(DIR_ROOT) + zoptions->GetDirString(DIR_SAVEGAMES) + info->m_Dir).ToChar();
	}

	uint64_t GetNumber(const std::string& saveName)
	{
		uint64_t number = 0;

		for (size_t i = 0; i < saveName.size() && isdigit(saveName[i]); i++)
		{
			number = number * 10u + static_cast<uint64_t>(saveName[i] - '0');

			if (number >= std::numeric_limits<uint64_t>::max() / 11u)
				return 0;
		}

		return number;
	}
}
