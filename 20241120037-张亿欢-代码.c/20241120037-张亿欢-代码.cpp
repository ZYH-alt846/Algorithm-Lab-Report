#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <windows.h>

// ================= 全局常量与变量定义 =================
#define MAX_N_SORT 100000
#define MAX_N_KNAP 320000
#define MAX_C      1000000 

int sort_arr[MAX_N_SORT];
int temp_arr[MAX_N_SORT]; 

double weights[MAX_N_KNAP];
double values[MAX_N_KNAP];

// 全局计数器：记录比较操作次数
long long bubble_comp = 0, merge_comp = 0, quick_comp = 0;
// 全局计数器：记录递归子问题规模
long long merge_sub_sizes = 0, quick_sub_sizes = 0;

// 函数声明 
void generateSortData(int n);
void generateKnapData(int n);
void bubbleSort(int arr[], int n);
void mergeSort(int arr[], int l, int r);
void merge(int arr[], int l, int m, int r);
void quickSort(int arr[], int low, int high);

void bruteForceKnapsack(int n, int C, double *max_v);
double dynamicProgrammingKnapsack(int n, int C);
double greedyKnapsack(int n, int C);
void backtrack(int i, int n, double current_w, double current_v, int C, double *max_v);

// 计时辅助
LARGE_INTEGER freq, start_t, end_t;
void init_timer() { QueryPerformanceFrequency(&freq); }
void start_timer() { QueryPerformanceCounter(&start_t); }
double get_time_ms() {
    QueryPerformanceCounter(&end_t);
    return (double)(end_t.QuadPart - start_t.QuadPart) * 1000.0 / (double)freq.QuadPart;
}

int main() {
    init_timer();
    srand((unsigned)time(NULL));
    printf("=== 算法设计与分析实验开始运行 ===\n\n");

    // ==========================================
    // 1. 排序问题测试 (记录比较次数)
    // ==========================================
    printf("[1/4] 正在执行排序算法测试 (记录比较次数)...\n");
    FILE *fs = fopen("sort_data.csv", "w");
    fprintf(fs, "数据规模,冒泡比较次数,归并比较次数,快排比较次数\n");

    int sort_sizes[] = {10, 100, 1000, 2000, 5000, 10000, 100000};
    int num_sort = sizeof(sort_sizes) / sizeof(sort_sizes[0]);

    for(int idx = 0; idx < num_sort; idx++) {
        int n = sort_sizes[idx];
        generateSortData(n);

        // 冒泡
        for(int i=0; i<n; i++) temp_arr[i] = sort_arr[i]; 
        bubble_comp = 0;
        bubbleSort(temp_arr, n);

        // 归并
        for(int i=0; i<n; i++) temp_arr[i] = sort_arr[i];
        merge_comp = 0; merge_sub_sizes = 0;
        mergeSort(temp_arr, 0, n-1);

        // 快排
        for(int i=0; i<n; i++) temp_arr[i] = sort_arr[i];
        quick_comp = 0; quick_sub_sizes = 0;
        quickSort(temp_arr, 0, n-1);

        fprintf(fs, "%d,%I64d,%I64d,%I64d\n", n, bubble_comp, merge_comp, quick_comp);
    }
    fclose(fs);
    printf("-> 排序比较次数已保存\n\n");

    // ==========================================
    // 2. 子问题规模记录 
    // ==========================================
    printf("[2/4] 正在记录子问题规模...\n");
    FILE *fsub = fopen("sort_subproblems.csv", "w");
    fprintf(fsub, "数据规模,归并子问题规模总和,快排子问题规模总和\n");
    
    for(int idx = 0; idx < num_sort; idx++) {
        int n = sort_sizes[idx];
        generateSortData(n);

        for(int i=0; i<n; i++) temp_arr[i] = sort_arr[i];
        merge_sub_sizes = 0;
        mergeSort(temp_arr, 0, n-1);
        long long m_size = merge_sub_sizes;

        for(int i=0; i<n; i++) temp_arr[i] = sort_arr[i];
        quick_sub_sizes = 0;
        quickSort(temp_arr, 0, n-1);
        long long q_size = quick_sub_sizes;

        fprintf(fsub, "%d,%I64d,%I64d\n", n, m_size, q_size);
    }
    fclose(fsub);
    printf("-> 子问题规模数据已保存\n\n");

    // ==========================================
    // 3. 0-1背包问题测试 
    // ==========================================
    printf("[3/4] 正在执行0-1背包算法测试...\n");
    FILE *fk = fopen("knap_times.csv", "w");
    fprintf(fk, "物品数量,背包容量,蛮力法时间(ms),动态规划时间(ms),贪心法时间(ms),回溯法时间(ms)\n");

    int knap_sizes[] = {10, 15, 20, 25, 1000, 2000, 5000, 10000, 20000, 40000};
    int capacities[] = {10000, 100000, 1000000};
    
    for(int c_idx = 0; c_idx < 3; c_idx++) {
        int C = capacities[c_idx];
        printf("--- 测试容量 C = %d ---\n", C);
        
        for(int idx = 0; idx < 10; idx++) {
            int n = knap_sizes[idx];
            generateKnapData(n);

            double t_brute = 0, t_dp = 0, t_greedy = 0, t_back = 0;
            double max_v = 0; 

            // 蛮力法 (n<=25)
            if (n <= 25) {
                start_timer();
                bruteForceKnapsack(n, C, &max_v);
                t_brute = get_time_ms();
            }

            // 动态规划
            if ((long long)n * C <= 2000000000LL && (long long)C * 100 <= 20000000LL) { 
                start_timer();
                t_dp = dynamicProgrammingKnapsack(n, C);
            }

            // 贪心法
            start_timer();
            t_greedy = greedyKnapsack(n, C);

            // 回溯法 (n<=25)
            if (n <= 25) {
                max_v = 0;
                start_timer();
                backtrack(0, n, 0, 0, C, &max_v);
                t_back = get_time_ms();
            }

            fprintf(fk, "%d,%d,%.2f,%.2f,%.2f,%.2f\n", n, C, t_brute, t_dp, t_greedy, t_back);
        }
    }
    fclose(fk);
    printf("-> 背包时间数据已保存\n\n");

    // ==========================================
    // 4. 生成1000物品详细数据 
    // ==========================================
    printf("[4/4] 正在生成1000物品详细数据...\n");
    generateKnapData(1000);
    FILE *fe = fopen("knap_data_1000.csv", "w");
    fprintf(fe, "物品编号,物品重量,物品价值\n");
    for(int i=0; i<1000; i++) {
        fprintf(fe, "%d,%.2f,%.2f\n", i+1, weights[i], values[i]);
    }
    fclose(fe);
    printf("-> 1000物品数据已保存\n\n");

    printf("=== 实验结束 ===\n");
    system("pause");
    return 0;
}

// ================= 辅助函数 =================
void generateSortData(int n) {
    for(int i=0; i<n; i++) sort_arr[i] = rand() % 100000;
}

// 【修复1】：严格限制重量在 1.00 ~ 100.00 之间
void generateKnapData(int n) {
    for(int i=0; i<n; i++) {
        weights[i] = 1.0 + (rand() % 9901) / 100.0; // 1.00 ~ 100.00
        values[i] = 100.0 + (rand() % 90000) / 100.0;
    }
}

// ================= 排序算法 (带计数) =================
void bubbleSort(int arr[], int n) {
    for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < n - i - 1; j++) {
            bubble_comp++;
            if (arr[j] > arr[j + 1]) {
                int temp = arr[j]; arr[j] = arr[j + 1]; arr[j + 1] = temp;
            }
        }
    }
}

void merge(int arr[], int l, int m, int r) {
    int n1 = m - l + 1, n2 = r - m;
    int *L = (int*)malloc(n1 * sizeof(int));
    int *R = (int*)malloc(n2 * sizeof(int));
    for (int i = 0; i < n1; i++) L[i] = arr[l + i];
    for (int j = 0; j < n2; j++) R[j] = arr[m + 1 + j];

    int i = 0, j = 0, k = l;
    while (i < n1 && j < n2) {
        merge_comp++;
        if (L[i] <= R[j]) { arr[k] = L[i]; i++; } 
        else { arr[k] = R[j]; j++; }
        k++;
    }
    while (i < n1) { arr[k] = L[i]; i++; k++; }
    while (j < n2) { arr[k] = R[j]; j++; k++; }
    free(L); free(R);
}

void mergeSort(int arr[], int l, int r) {
    if (l < r) {
        merge_sub_sizes += (r - l + 1);
        int m = l + (r - l) / 2;
        mergeSort(arr, l, m);
        mergeSort(arr, m + 1, r);
        merge(arr, l, m, r);
    }
}

int partition(int arr[], int low, int high) {
    int pivot = arr[high], i = (low - 1);
    for (int j = low; j <= high - 1; j++) {
        quick_comp++;
        if (arr[j] < pivot) {
            i++;
            int temp = arr[i]; arr[i] = arr[j]; arr[j] = temp;
        }
    }
    int temp = arr[i + 1]; arr[i + 1] = arr[high]; arr[high] = temp;
    return (i + 1);
}

void quickSort(int arr[], int low, int high) {
    if (low < high) {
        quick_sub_sizes += (high - low + 1);
        int pi = partition(arr, low, high);
        quickSort(arr, low, pi - 1);
        quickSort(arr, pi + 1, high);
    }
}

// ================= 背包算法  =================
void bruteForceKnapsack(int n, int C, double *max_v) {
    *max_v = 0;
    int total = 1 << n; 
    for(int mask = 0; mask < total; mask++) {
        double w = 0, v = 0;
        for(int i=0; i<n; i++) {
            if(mask & (1 << i)) { w += weights[i]; v += values[i]; }
        }
        if(w <= C && v > *max_v) *max_v = v;
    }
}


double dynamicProgrammingKnapsack(int n, int C) {
    start_timer();
    int C_scaled = C * 100;
    double *dp = (double*)calloc(C_scaled + 1, sizeof(double));
    if (!dp) { return 0; } // 内存分配失败保护

    for(int i=0; i<n; i++) {
        int w = (int)(weights[i] * 100.0 + 0.5); // 放大100倍并四舍五入，保留精度
        double v = values[i];
        for(int j=C_scaled; j>=w; j--) {
            if(dp[j-w] + v > dp[j]) dp[j] = dp[j-w] + v;
        }
    }
    double t = get_time_ms();
    free(dp);
    return t;
}


typedef struct {
    int idx;
    double ratio;
} Item;

int compare_items(const void *a, const void *b) {
    double r1 = ((Item*)a)->ratio;
    double r2 = ((Item*)b)->ratio;
    if (r1 < r2) return 1;  // 降序排列
    if (r1 > r2) return -1;
    return 0;
}

double greedyKnapsack(int n, int C) {
    start_timer();
    Item *items = (Item*)malloc(n * sizeof(Item));
    for(int i=0; i<n; i++) { 
        items[i].idx = i; 
        items[i].ratio = values[i] / weights[i]; 
    }

    // 使用 C 标准库快速排序 O(n log n)
    qsort(items, n, sizeof(Item), compare_items);

    double current_w = 0, max_val = 0;
    for(int i=0; i<n; i++) {
        int idx = items[i].idx;
        if(current_w + weights[idx] <= C) {
            current_w += weights[idx]; 
            max_val += values[idx];
        }
    }
    double t = get_time_ms();
    free(items);
    return t;
}

void backtrack(int i, int n, double current_w, double current_v, int C, double *max_v) {
    if (current_w > C) return;
    if (i == n) { if (current_v > *max_v) *max_v = current_v; return; }
    backtrack(i + 1, n, current_w + weights[i], current_v + values[i], C, max_v);
    backtrack(i + 1, n, current_w, current_v, C, max_v);
}
