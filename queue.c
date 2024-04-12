#include "types.h"
#include "defs.h"
#include "queue.h"

void initqueue(queue *q)
{
	q->front = -1;
	q->rear = -1;
	q->size = max;
	q->A = (int *)malloc(sizeof(int) * max);
}

void enqueue(queue *q, int d)
{

	if ((q->rear + 1) % q->size == q->front)
	{
		return;
	}

	if (q->front == -1)
	{
		q->front++;
	}
	q->rear = (q->rear + 1) % q->size;
	q->A[q->rear] = d;
	return;
}
int dequeue(queue *q)
{

	int x = -1;
	if (q->front == -1)
	{
		return x;
	}
	x = q->A[q->front];
	if (q->front != -1 && q->front == q->rear)
	{
		q->front = -1;
		q->rear = -1;
	}
	else
		q->front = (q->front + 1) % q->size;
	return x;
}

int isqueuefull(queue q)
{
	return ((q.rear + 1) % q.size == q.front);
}

int isqueueempty(queue q)
{
	return (q.front == -1);
}
