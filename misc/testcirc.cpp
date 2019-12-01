#include <bits/stdc++.h>

using namespace std;

void print_circle(int rad) {
	int i, j, r2;

	r2 = rad * rad;

	for (i = -rad; i <= rad; i++) {
		for (j = -rad; j <= rad; j++) {
			cout << ((i * i) + (j * j) <= r2);
		}
		cout << endl;
	}
}

int main(int argc, char **argv) {
	print_circle(atoi(argv[1]));
	return 0;
}
