#include "arrays.h"

/**
 * Write the contents of an integer array to stdout
 * @param arr[] The array being displayed
 * @param len The length of the array
 */
void print_int_array(int arr[], int len)
{
  printf("{");
  printf("%d", arr[0]);
  for (int i = 1; i < len; i++)
  {
    printf(", %d", arr[i]);
  }
  printf("}\n");
}

/**
 * Write the contents of a double array to stdout
 * @param arr[] The array being displayed
 * @param len The length of the array
 */
void print_double_array(double arr[], int len)
{
  printf("{");
  printf("%f", arr[0]);
  for (int i = 1; i < len; i++)
  {
    printf(", %f", arr[i]);
  }
  printf("}\n");
}

/**
 * Concatenates the contents of the first and second arrays into the last one
 * @param arr1[] The first array
 * @param len1 The length of the first array (can also be used to only this number of values from the first array)
 * @param arr2[] The second array
 * @param len2 The length of the second arrays (can also be used to only this number of values from the second array)
 * @param final_arr[] The array containing the result
 */
void concatenate(int arr1[], int len1, int arr2[], int len2, int final_arr[])
{
  for (int i = 0; i < len1; i++)
  {
    final_arr[i] = arr1[i];
  }
  for (int i = 0; i < len2; i++)
  {
    final_arr[i + len1] = arr2[i];
  }
}

/**
 * Returns the sum of all integer values in an array
 * @param arr[] The concerned array
 * @param len The length of the array
 */
int array_sum(int arr[], int len)
{
  int sum = 0;
  for (int i = 0; i < len; i++)
  {
    sum += arr[i];
  }
  return sum;
}