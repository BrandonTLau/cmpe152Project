int add(int a, int b) {
    return a + b;
}

int main() {
    int x;
    int i;

    x = 0;
    for (i = 0; i < 5; i = i + 1) {
        x = x + add(i, 1);
    }

    return x;   // expected: 15
}
