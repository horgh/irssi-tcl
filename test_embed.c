#include <stdio.h>
#include <tcl.h>
//#include </usr/include/tcl8.5/tcl.h>

int main()
{
	Tcl_Interp *interp = Tcl_CreateInterp();

	//tcl_interp = Tcl_CreateInterp();
	if (interp == NULL)
	{
		printf("Could not create interpreter\n");
	}
	//Tcl_Init(tcl_interp);

	char *script = "set a fuck";
	//Tcl_EvalEx(tcl_interp, script, -1, NULL);
	int rc = Tcl_Eval(interp, script);
	//const char *a = Tcl_GetStringResult(tcl_interp);
	printf(Tcl_GetStringResult(interp));
	printf("\n");
	char *what = "set a";
	rc = Tcl_Eval(interp, "set a");
	printf(Tcl_GetStringResult(interp));
	printf("\n");
	//printf("Hi %s \n", a);
}
