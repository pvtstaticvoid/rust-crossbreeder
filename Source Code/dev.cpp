// C++ program to iterate over all possible
// combinations of array elements

#include <bits/stdc++.h>
using namespace std;

// Function to check if any combination of
// elements of the array sums to k
bool checkSum(int a[], int n, int k)
{
    // Flag variable to check if
    // sum exists
    int flag = 0;

    // Calculate number of bits
    int range = (1 << n) - 1;

    // Generate combinations using bits
    for (int i = 0; i <= range; i++) {

        int x = 0, y = i, sum = 0;

        while (y > 0) {

            if (y & 1 == 1) {

                // Calculate sum
                sum = sum + a[x];
            }
            x++;
            y = y >> 1;
        }

        // If sum is found, set flag to 1
        // and terminate the loop
        if (sum == k)
            return true;
    }

    return false;
}

// Driver Code
int main()
{
    int k = 6;
    int a[] = { 3, 4, 1, 2 };
    int n = sizeof(a)/sizeof(a[0]);
    if (checkSum(a, n, k))
        cout << "Yes";
    else
        cout << "No";

    return 0;
}