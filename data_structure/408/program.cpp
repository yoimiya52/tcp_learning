#include <cstdlib>
#include <cstring>
#include <iostream>
#include <vector>
#include <climits>
#include <algorithm>
using namespace std;
class Solution{
public:

    void print(vector<int> s1){
        for(vector<int>::iterator it = s1.begin();it != s1.end();++it){
            cout << *it << " ";
        }
        cout << endl;
    }

    // 合并两个有序数组,nms1 的空间为m+n,后面n位置补0
    void merge(vector<int>& nums1,int m,vector<int>& nums2 ,int n){
        int i = m - 1;
        int j = n - 1;
        int k = m + n -1;
        while(i>=0 && j>=0){
            if (nums1[i] >= nums2[j]){
                 nums1[k] = nums1[i];
                 i--; 
            }else{
                nums1[k] = nums2[j];
                j--;
            }
            k--;
        }
        while (j>=0){
            nums1[k--] = nums2[j--];
        }
    }
    // 删除 指定元素
    int remove_element(vector<int>& s1,int val){
        int num = 0;
        for(vector<int>::iterator it = s1.begin();it!=s1.end();++it){
            if(*it != val){
                s1[num] = *it;
                num++;
            }
        }
        cout << "num "<<num<<endl;
        return num;
        
    }
    // 删除重复元素
    int remove_duplicates(vector<int>& s1){
        int num = 1;
        int current_flag_num = s1[0];
        for (int i = 1;i<s1.size();i++){
            if (s1[i]!=current_flag_num){
                current_flag_num = s1[i];
                s1[num++] = s1[i];
            }
        }
        cout << "num "<<num<<endl;
        return num;
    }
    // 删除重复元素,出现次数超过两次的元素只出现两次
    int remove_duplicates_2(vector<int>& s1){
        int slow = 1;
        int fast = 1;
        int current_flag_num = s1[0];
        int num_of_num=1;//数字的数量
        while(fast<s1.size()){
            if(s1[fast]!=current_flag_num){
                current_flag_num = s1[fast];
                s1[slow] = s1[fast];
                slow++;
                fast++;
                num_of_num = 1;
            }else if (num_of_num < 2) {
                s1[slow] = s1[fast];
                slow++;
                fast++;
                num_of_num ++;
            }else{
                fast++;
                num_of_num++;
            }
        }
        cout << "num "<<slow<<endl;
        return slow ;
    }
    // 查找多数元素(出现次数大于n/2的数字),摩尔投票,
    int majority_element(vector<int>& s1){
        int count = 0;
        int candidate = s1[0];
        for(vector<int>::iterator it = s1.begin();it != s1.end();++it){
            if (count==0){
                candidate = *it;
                count++;
            }else{
                if (*it == candidate){
                    count++;
                }else{
                    count--;
                }
            }
        }
        cout << "candidate "<<candidate<<endl;
        return candidate;
    }
    // 向右轮转数组
    void rotate(vector<int>& s1, int k){
        vector<int> s2(s1.size());
        for(int i=0;i<s1.size();i++){
            s2[(i+k)%s1.size()] = s1[i];
        }
        s1.assign(s2.begin(),s2.end());
    }
    //最大利润
    // int max_profit(vector<int>& prices){
    //     int buy = 0;
    //     int sell = 0;
    //     int max_profit = 0;
    //     max_profit =  best_buy_point(prices,buy,max_profit);
    //     cout << "max_profit "<<max_profit<<endl;
    //     return max_profit;
    // }
    // int best_buy_point(vector<int>& s,int buy,int current_profit){
    //     int max_profit = 0;
    //     for(int i = buy+1;i<s.size();i++){
    //         if(s[i]>s[buy] && (s[i]-s[buy]) > max_profit){
    //             max_profit = s[i]-s[buy];
    //         }else if(s[i]<s[buy]){
    //             max_profit = best_buy_point(s,i,max_profit);
    //         }
    //     }
    //     return max_profit > current_profit ? max_profit:current_profit;
    // }
    int max_profit_1(vector<int>& prices) {
        if (prices.empty()) return 0;

        int min_price = prices[0];
        int max_profit = 0;

        for (int i = 1; i < prices.size(); i++) {
            // 更新最大利润
            max_profit = max(max_profit, prices[i] - min_price);
            // 更新最低买入价
            min_price = min(min_price, prices[i]);
        }

        return max_profit;
    }
    // 每天都可以交易，获取最大利润，贪心算法(如果每天都能赚钱，在全局来看一定是最优秀的)
    int max_profit_2(vector<int>& s){
        int max_profit = 0;
        for(int i = 1;i<s.size();i++){
            if(s[i]-s[i-1]>0){
                max_profit += s[i] - s[i-1];
            }
        }
        return max_profit;
    }
    // 跳跃游戏，在每一步的都尝试跳到最后
    bool canJump1(vector<int>& s){
        int n = s.size();
        int current_max_position = 0;
        if(n == 1) return true;
        for(int i=0;i<n;i++){
            if(s[i]>0){
                if (i+s[i]+1 >= n){
                    return true;
                }else{
                    current_max_position = max(i+s[i],current_max_position);
                }
            }else if(i>=current_max_position){
                return false;
            }
        }
        return false;
    }
    //优化，熟悉贪心算法
    bool canJump2(vector<int>& s){
        int n = s.size();
        int current_max_position = 0;
        for (int i=0;i<n;i++){
            if(i>current_max_position) return false;
            current_max_position = max(current_max_position,i+s[i]);
            if(current_max_position >= n-1) return true;
        }
        return true;
    }
    //跳跃游戏，找出跳到最后位置的最短跳数,题给数组保证能跳到最后位置
    int jump(vector<int>& s){
        int steps = 0;
        int current_max_position = 0;
        int max_position = 0;
        int size = s.size();
        for(int i = 0;i<size-1;i++){
            max_position = max(max_position,i+s[i]);
            if(i == current_max_position){
                steps++;
                current_max_position = max_position;
            }
        }
        return steps;
    }
    // h 指数
    int h_index(vector<int>& s){
        sort(s.begin(),s.end(),greater<int>());
        int h = 0;
        for(int i=0;i<s.size();i++){
            if(s[i]>=h+1){
                h = i+1;
            }else{
                break;
            }
        }
        return h;
    }
    // 求解除自身外的乘积
    vector<int> productExceptSelf(vector<int>& s){
        int front_result = 1;
        int tail_result = 1;
        int result = 1;
        vector<int> result_s(s.size(),1);
        // 计算前缀
        for(int i = 0;i<s.size();i++){
            result_s[i] = front_result;
            front_result = front_result * s[i];
        }
        for(int i = s.size()-1;i>=0;i--){
            result_s[i] = tail_result * result_s[i];
            tail_result = tail_result * s[i];
        }
        return result_s;
    }
    // 加油站
    int can_complete_circuit(vector<int>& gas, vector<int>& cost){
        int current_gas = 0;
        int current_cost = 0;
        int gas_num = gas.size();
        int start = 0;
        for(int i=0;i< gas_num;i++){
            start = i;
            for(int j = 0;j< gas_num;j++){
                current_gas = current_gas + gas[start % gas_num];
                current_cost = cost[start];
                if(current_cost > current_gas) {
                    i = start;
                    break;
                }else{
                    current_gas = current_gas - current_cost;
                    start = (start+1) % gas_num;
                }
                return i;
            }
            current_gas = 0;
        }
        return -1;
    }
};

int main() {
    Solution s = Solution();
    vector<int> n1 = {1,2,3,4,5};
    vector<int> n2 = {3,4,5,1,2};
    vector<int> n3 = {0,0,1,1,1,2,2,3,3,4};
    //s.merge(n1,3,n2,3);
    //s.remove_element(n1,2);
    // s.remove_duplicates_2(n3);
    int result = s.can_complete_circuit(n1,n2);
    cout<<"result:"<<result<<endl;
    // s.print(n3);
}


