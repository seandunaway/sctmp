#include <sierrachart.h>

SCDLLName("example")

SCSFExport scsf_example(SCStudyInterfaceRef sc) {
	if (sc.SetDefaults) {
		sc.GraphName = "example";
		return;
	}
}
