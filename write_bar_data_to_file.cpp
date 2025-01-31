#include <sierrachart.h>

SCDLLName("write bar data to file")
SCSFExport scsf_write_bar_data_to_file (SCStudyInterfaceRef sc)
{
	SCInputRef file = sc.Input[0];

	if (sc.SetDefaults)
	{
		sc.DisplayStudyName = 0;
		sc.GraphName = "write bar data to file";
		sc.GraphRegion = 0;

		char user_directory[MAX_PATH];
		ExpandEnvironmentStrings("%USERPROFILE%", user_directory, MAX_PATH);

		char default_file[MAX_PATH];
		snprintf(default_file, MAX_PATH, "%s\\Desktop\\%s.csv", user_directory, sc.Symbol.GetChars());

		file.Name = "file";
		file.SetPathAndFileName(default_file);

		return;
	}

	n_ACSIL::s_WriteBarAndStudyDataToFile WriteBarAndStudyDataToFileParams;
	WriteBarAndStudyDataToFileParams.StartingIndex = sc.CurrentIndex;
	WriteBarAndStudyDataToFileParams.OutputPathAndFileName = file.GetPathAndFileName();
	sc.WriteBarAndStudyDataToFileEx(WriteBarAndStudyDataToFileParams);
}
