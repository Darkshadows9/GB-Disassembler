#include <stdio.h>
#include <stdlib.h>

#include "sort.h"

size_t *shrinkArray(size_t *array, size_t size)
{
	size_t *array_temp = realloc(array, sizeof(*array_temp) * size);
	if (!array_temp)
	{
		printf("Failed to reallocate memory when shrinking array.\n");
		exit(EXIT_FAILURE);
	}
	array = array_temp;
	return array;
}

size_t deleteDuplicateValuesUnsigned(size_t *array, size_t size)
{
	size_t i;
	size_t j = 0;
	for (i = 0; i < size - 1; i++)
	{
		if (array[i] != array[i + 1])
		{
			array[j] = array[i];
			j++;
		}
	}
	array[j] = array[size - 1];
	j++;
	size = j;
	return size;
}

void quickSortUnsigned(size_t *array, size_t elements) /* Based off: http://alienryderflex.com/quicksort/ */
{
	size_t pivot;
	size_t beg[300];
	size_t end[300];
	size_t i = 0;
	size_t swap;
	beg[0] = 0;
	end[0] = elements;
	while (i <= 299)
	{
		size_t left = beg[i];
		size_t right = end[i] - 1;
		if (left < right && right < elements)
		{
			pivot = array[left];
			while (left < right)
			{
				while (array[right] >= pivot && left < right)
				{
					right--;
				}
				if (left < right)
				{
					array[left] = array[right];
					left++;
				}
				while (left < elements && array[left] <= pivot && left < right)
				{
					left++;
				}
				if (left < right)
				{
					array[right] = array[left];
					right--;
				}
			}
			array[left] = pivot;
			beg[i + 1] = left + 1;
			end[i + 1] = end[i];
			end[i] = left;
			i++;
			if (end[i] - beg[i] > end[i - 1] - beg[i - 1])
			{
				swap = beg[i];
				beg[i] = beg[i - 1];
				beg[i - 1] = swap;
				swap = end[i];
				end[i] = end[i - 1];
				end[i - 1] = swap;
			}
		}
		else
		{
			i--;
		}
	}
}

size_t *sortUnsigned(size_t *array, size_t elements)
{
	if (elements > 1)
	{
		quickSortUnsigned(array, elements);
		elements = deleteDuplicateValuesUnsigned(array, elements);
		array = shrinkArray(array, elements);
	}
	return array;
}
