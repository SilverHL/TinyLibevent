#include <iostream>
#include <cstdlib>

using namespace std;

int main()
{
    int a = 10;
    int *pa = new int;
    cout << pa << endl;

    pa = static_cast<int *>(realloc(pa, 10*sizeof(a)));

    cout << pa;

}
