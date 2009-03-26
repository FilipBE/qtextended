#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>

int main(int, char **)
{
    int m_id = 0;
    int m_semId = ::semget(m_id, 1, 0);
    struct timespec ts;
    ts.tv_sec = 1;
    ts.tv_nsec = 0;
    struct sembuf ops[1];
    ops[0].sem_num = 0;
    ops[0].sem_op = -1;
    ops[0].sem_flg = SEM_UNDO;
    ::semtimedop(m_semId, ops, 1, &ts);
    return 0;
}

