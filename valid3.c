// array + printf + comments test

int main() {
    int a[5];
    int i;
    int sum;

    sum = 0;

    /* fill array with 1..5 */
    for (i = 0; i < 5; i = i + 1) {
        a[i] = i + 1;
        sum = sum + a[i];
    }

    printf("sum=%d\n", sum);   // should print sum=15
    return sum;
}
