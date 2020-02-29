#include "Run.h"
Run::Run(QObject *parent) : QObject(parent)
{
}
Run::~Run()
{
}


/*#include "Server.h"
#include <stdlib.h>
#include <iostream>
int main()
{

    Server srv;
    if (srv.WinsockStartup() == -1) return -1;
    if (srv.ServerStartup() == -1) return -1;
    if (srv.ListenStartup() == -1) return -1;
    if (srv.Loop() == -1) return -1;
    system("pause");
    return 0;
}
*/
