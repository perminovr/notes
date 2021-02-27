
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#define QS_TYPE int
void quicksort(QS_TYPE *s_arr, QS_TYPE idx_0, QS_TYPE idx_n)
{
    if (idx_0 < idx_n) {
		QS_TYPE left = idx_0;
		QS_TYPE right = idx_n;
		// QS_TYPE pivot = s_arr[rand() % (right+1)];
		QS_TYPE pivot = s_arr[(left+right) / 2];
		do {
			while (s_arr[left] < pivot) left++;
			while (s_arr[right] > pivot) right--;
			if (left <= right) {
				QS_TYPE tmp = s_arr[left];
				s_arr[left] = s_arr[right];
				s_arr[right] = tmp;
				left++;
				right--;
			}
		} while (left <= right);
		quicksort(s_arr, idx_0, right);
		quicksort(s_arr, left, idx_n);
	}
}

int main()
{
	time_t t;
	QS_TYPE arr[] = {10,9,8,7,6,5,4,3,2,1};
	// QS_TYPE arr[] = {1,2,3,4,5,6,7,8,9,10};
	int arr_size = sizeof(arr)/sizeof(QS_TYPE);
	srand((unsigned)time(&t));
	quicksort(arr, 0, arr_size-1);
	for (int i = 0; i < arr_size; ++i) {
		printf("%d ", arr[i]);
	}
	printf("\n");
	return 0;
}