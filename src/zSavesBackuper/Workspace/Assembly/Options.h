namespace NAMESPACE
{
	namespace Options
	{
		ZOPTION(BackupToDirPath, std::string{ "\\backups\\" });
		ZOPTION(BackupToDirLimit, 50);
		ZOPTION(BackupToSlotMin, 10);
		ZOPTION(BackupToSlotMax, 20);
		ZOPTION(DoNotBackupSlotMin, 0);
		ZOPTION(DoNotBackupSlotMax, -1);
		ZOPTION(CopyAlgorithm, 2);
	}

	namespace Options
	{
		Sub addTrivia(ZSUB(GameEvent::Execute), []
			{
				BackupToDirPath.endTrivia += A"... absolute or relative path to the directory where the backuped saves are stored in";
				BackupToDirLimit.endTrivia += A"... maximum amount of stored saves in the directory (per mod)";
				BackupToDirLimit.endTrivia += A"set -1 for unlimited storage";
				BackupToDirLimit.endTrivia += A"set 0 to disable backing up to the folder";

				BackupToSlotMax.automaticallyAddEmptyLines = false;
				BackupToSlotMax.endTrivia += A"... range of save slots used to store backups";
				BackupToSlotMax.endTrivia += A"to disable set the maximum slot to be less than the minimum";

				DoNotBackupSlotMin.automaticallyAddEmptyLines = false;
				DoNotBackupSlotMax.automaticallyAddEmptyLines = false;
				DoNotBackupSlotMin.startTrivia += A"";
				DoNotBackupSlotMax.endTrivia += A"... range of slots saving to which does not result in backups";
				DoNotBackupSlotMax.endTrivia += A"to allow backup from all the slots set the maximum slot to be less than the minimum";
				DoNotBackupSlotMax.endTrivia += A"";

				CopyAlgorithm.endTrivia += A"... an internal algorithm for copying the savegame files";
				CopyAlgorithm.endTrivia += A"1 - using SHFileOperation";
				CopyAlgorithm.endTrivia += A"2 - using std::filestream (default)";
				CopyAlgorithm.endTrivia += A"any other value - using std::filesystem::copy";
			});

		Sub load(ZSUB(GameEvent::DefineExternals), []()
			{
				ActiveOptionBase::LoadAll();
			});
	}
}
