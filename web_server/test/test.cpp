#include "server.h"

int main()
{
    event_loop loop;

    server ser(&loop, "192.168.1.233", 8888);

    loop.evebt_process();

    return 0;
}
