#define max 10

typedef struct queue
{
	int front, rear, size;
	int *A;
} queue;

void initqueue(queue *q);

void enqueue(queue *q, int d);

int dequeue(queue *q);

int isqueuefull(queue q);

int isqueueempty(queue q);
