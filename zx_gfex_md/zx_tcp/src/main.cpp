#include "ini_parser.h"
#include "gfexRcv.h"

int main(int argc, char* argv[])
{
	CGfexRcv* pGfexRcv = new CGfexRcv("./config.ini");
	if (!pGfexRcv->Init())
	{
		std::cout << "pGfexRcv init fail" << std::endl;
		return -1;
	}
	pGfexRcv->Run();
	return 0;
}