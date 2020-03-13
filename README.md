# Dict
#### Marco Ma(XVs32), TZY(nckuoverload)
---

###### tags: `SS`, `linux`, `bloom filter`

---
## Outline

[TOC]

* Introduction
    * bloom-filter
    * quotient-filter??
    * TST
        * 資料輸入順序對 TST 效能的影響
* 設計實驗
    * TST search
* cpy vs ref




---
## Introduction

### Bloom Filter
參考自[隱藏在字典裡的數學](/@sysprog/BkE3uSvdN)
#### Bloom Filter 解說
Bloom filter 利用 hash function，在不用走訪全部元素的前提，**預測**特定字串是否存於資料結構中。因此時間複雜度是 $O(1)$，而非傳統逐一尋訪的 $O(n)$ 。

- 以下圖舉例，需要加入一個字串 `str` 時，將值丟進 `h1(str)`、`h2(str)`、`h3(str)`，分別會得到 3 個 table index，table 內容為布林值，將該 index 設為 1 表示 used。
- 以下圖舉例，查詢時，和加入相互對應，一樣是對要查詢的字串 `str` 做 hash 運算，將得到的結果和 table 的內容比對，若 table 皆為 1，則代表該字串**可能**存在於該 table 中 (bloom filter 尚有 false positive 的問題，稍後會說明)。

小結: 使用 bloom filter 的目的主要在於解決，如果要搜尋某一筆資料 (上述例子為 `str`)，為了避免該資料不存在於該結構中，造成搜尋演算法必須走完某一個支線才得到不存在於結構中的回應。使用 bloom filter 只需要 $O(1)$ 即可完成**是否存在於結構中**的答覆。

![](https://i.imgur.com/VAdGeuI.png)

#### Bloom Filter 的的缺點
可參考自[2019q3 第 5 週測驗題](/@sysprog/BJvfgm2_B)

- 錯誤率 (false positive): 假如在搜尋未加入的字串 `str` 時，其 hash 後的 table index 部分與 `str_A` 和 `str_B` 碰撞，則會造成誤判，誤以為該字串以存在於該結構中。
- 不可刪除節點: hash 有可能會發生碰撞，因此 bloom filter 無法得知該 table index 中的 bit 是由哪個字串觸發，如果輕易的更動 table 中的 boolean (bit)，有可能影響到其他字串，進而造成誤判。
- Poor scaling out of RAM of SSD: 主因是 bloom filter 沒辦法限制記憶體的存取範圍。
- The inability to resize dynamically: 將 table 變大時，需要變更 hash function，使其可覆蓋到變大的區域，因此原本就存在於 bloom filter 內的資料也必須要重新經過 hash function 放入新的 table。
- The inability to count the number of occurrences of each item, especially with skewed input distributions: 跟 hash function 類似，除非透過特別的結構去計算。(CQF 有嘗試解決這個問題。)

詳細的解說也可以參考該週其他組別的[共筆](/@yxguo/H1ig2w-nr)。

#### 錯誤率計算:
首先假設我們的所有字串集合 S 裡面有 n 個字串， hash 總共有 k 個，Bloom Filter 的 table 總共 m bits。我們會判斷一個字串存在於 S 內，是看經過轉換後的每個 bits 都被 set 了，我們就會說可能這個字串在 S 內。但試想若是其實這個字串不在 S 內，但是其他的 a b c 等等字串經過轉換後的 index ，剛好涵蓋了我們的目標字串轉換後的 index，就造成了誤判這個字串在 S 內的情況。

誤判的機率為經過 k 個 hash function ，其對應 table 的 k 個 bit 皆為 1 的機率 : ==$(1-e^{-\frac{kn}{m}})^k$== 。

### Quotient Filter
Quotient Filter 可改善 Bloom Filter 的錯誤率問題。把資料經過 hash function 處理為 key。再拆分成商和餘數，商作為 index，餘數作為 data。並把 data 存放至陣列結構的第 index 格。為處理 index 碰撞，陣列結構中另外附加 3-bit 用於表示狀態。並在 index 碰撞時執行移位動作。
由於 Quotient Filter 為每一筆資料都保留獨立空間，因此 Quotient Filter 自身並不會產生誤判，但假如 hash function 產生碰撞，為兩筆不同的資料產生相同的 hash 值，則依然會誤判。

優點：
1. 只有兩次 hash 之間產生碰撞才會導致誤判。
2. 可加入，刪除 key。
缺點：
1. 為每筆資料保留獨立空間，空間需求比 Bloom Filter 大。
2. Quotient Filter 自身的大小無法調整。

名詞說明：
- slot: 用於存放從 key 拆分而來的 data 以及狀態位元。 
- canonical slot: 理論上用於存放某一筆 key 的 slot, 若一筆 key 的 index 是 3 ，則該 key 的 canonical slot 為 3。
- run：由一連串具有相同 index 但不同 data 的 key 所組成。符合前述條件的資料會透過移位動作在陣列中占用連續空間，因而得名。

狀態位元：
- is_occupied: 存在一筆資料的 canonical slot 為本 slot 時，該 bit 為 1。
- is_continuation: 若該 key 對應的 canonical slot 已被使用，則該 bit 為 1。
- is_shifted: 若該 key 的位置不在 canonical slot 上（意即曾被執行移位動作），則該 bit 為 1。

|is_occupied|is_continuation|is_shifted|描述|
|---|---|---|---|
|0|0|0| empty, 該 slot 為空|
|0|0|1| 該 slot 不是任何資料的 canonical slot，而且該 slot 內的 key 曾被移位|
|0|1|0| 無效。該 key 的 canonical slot 已被使用，但該 key 並沒有被移位，本條件不可能出現|
|0|1|1| 該 slot 不是任何資料的 canonical slot，該 key 的 canonical slot 已被使用，而且該 key 曾被移位 |
|1|0|0| 該 slot 為 slot 內的 key 的 canonical slot|
|1|0|1| 該 slot 是某筆資料的 canonical slot，而該 key 曾被移位|
|1|1|0| 無效。該 key 的 canonical slot 已被使用，但該 key 並沒有被移位，本條件不可能出現|
|1|1|1| 該 slot 是某筆資料的 canonical slot，該 key 的 canonical slot 已被使用，而且該 key 曾被移位 |

詳細流程可參考下圖，來自 [Wikipedia](https://en.wikipedia.org/wiki/Quotient_filter)。
![](https://i.imgur.com/463j3KZ.png)



### TST

TST(ternary search tree) 是前綴樹(trie)的一種變種。以時間換取空間，透過表示大於、等於、小於的三叉路徑，取代指數級別產生空指標的前綴樹。平均時間複雜度從前綴樹的 $O(n)$ 增加至 $O(logn)$。

在jserv大神的[介紹文章](https://hackmd.io/@sysprog/BkE3uSvdN)中，提到了[這個](https://www.cs.usfca.edu/~galles/visualization/TST.html)以視覺化呈現 TST 運作過程的網站。

例如往空的 TST 中加入 ```zip``` :

步驟為：
    1.比對```z```，但此時```root```為空，於是```root```的值變更為```z```。
    2.比對```i```，但沒有可供比對的節點，於是創建節點```i```。
    3.比對```p```，但沒有可供比對的節點，於是創建節點```p```。
	

```
              z
              |
         --<--|-->--
        |     =    |
      NULL    |   NULL
              i
              |
         --<--|-->--
        |     =    |
      NULL    |   NULL
              p
              |
         --<--|-->--
        |     =    |
      NULL    |   NULL
             NULL   
  
```



然後加入 ```zeta```:

步驟為：
    1.比對```z```，```root```節點的值等於```z```，於是指標指向```i```。
    2.比對```e```，而```e<i```，於是指標指向```i```的 left child。但 left child 為 ```NULL```,於是創建節點```e```。
    3.比對```t```，但沒有可供比對的節點，於是創建節點```t```。
    3.比對```a```，但沒有可供比對的節點，於是創建節點```a```。

```
                            z
                            |
                       --<--|-->--
                       |    =    |
                     NULL   |   NULL
                            i
                            |
           --------<--------|-->--
          |                 =    |
          e                 |   NULL
          |                 p
     --<--|-->--            |
     |    =    |       --<--|-->--
   NULL   |   NULL     |    =    |
          t          NULL   |   NULL
          |                NULL   
     --<--|-->--
     |    =    |
   NULL   |   NULL
          a
          |
     --<--|-->--
     |    =    |
   NULL   |   NULL
         NULL

```

為求簡化，不顯示指向```NULL```的路徑，TST 則如下圖：

![](https://i.imgur.com/LKaD5Ar.png)

再嘗試加入 ```zoo```:

步驟為：
    1.比對```z```，```root```節點的值等於```z```，於是指標指向```i```。
    2.比對```o```，而```o>i```，於是指標指向```i```的 right child。但 right child 為 ```NULL```,於是創建節點```o```。
    3.比對```o```，但沒有可供比對的節點，於是創建節點```o```。

```
                            z
                            |
                       --<--|-->--
                       |    =    |
                     NULL   |   NULL
                            i
                            |
           --------<--------|-------->--------
          |                 =                |     
          e                 |                o
          |                 p                |
     --<--|-->--            |           --<--|-->--
     |    =    |       --<--|-->--      |    =    |
   NULL   |   NULL     |    =    |    NULL   |   NULL
          t          NULL   |   NULL         o
          |                NULL              |
     --<--|-->--                        --<--|-->--
     |    =    |                        |    =    |
   NULL   |   NULL                    NULL   |   NULL
          a                                 NULL
          |
     --<--|-->--
     |    =    |
   NULL   |   NULL
         NULL

```
不顯示指向```NULL```的路徑的 TST 如下圖：
![](https://i.imgur.com/N1pZMEV.png)

---

### 資料輸入順序對TST效能的影響
雖然 TST 的平均時間複雜度是 $O(logn)$，但從上例可發現，資料的輸入順序將直接影響 TST 的建立。
假設存在 5 筆資料```aa```,```ab```,```ac```,```ad```,```ae```，如果順序輸入至 TST，將會得到：

![](https://i.imgur.com/i0VMB57.png)

此時效率與循序搜尋並無二致。
但把輸入順序更改為```ac```,```ab```,```ad```,```aa```,```ae```，則會得到：

![](https://i.imgur.com/MxZtQOS.png)

當 TST 越接近平衡，同樣數量的節點所需深度（層數）則越少，效率也會更高。

---

### TST 的資料輸入順序調整

回到```cities.txt```資料集，城市、國家名由於語言、譯音等特性，傾向具有同樣的前綴(prefix)。

把```cities.txt```按字典排序為```cities_one_col.txt```，嘗試只依照每筆資料前 2-byte 和 3-byte 分組，分別得到 668 組和 5952 組。


#### 按組別大小排序
把前綴出現次數較多的國家、城市名排序至資料集前方，盡早加入至 TST 中，以提高前綴出現次數較多的名字的搜尋效能。而相同前綴的成員則以產生最淺的 TST 為前提排序，例如七筆資料```aa```,```ab```,```ac```,```ad```,```ae```,```af```,```ag```則排序為```ad```,```ab```,```aa```,```ac```,```af```,```ae```,```ag```，如下圖。

![](https://i.imgur.com/iOWgeK6.png)

## 設計實驗：

### TST search
只使用 s 指令（搜尋符合前綴的名字），透過 perf 測試執行時間 及 cache-miss。

生成兩種各 8000 筆命令，最長 4 字元前綴的輸入文件：
1. 隨機產生名字的 ```random_command.txt``` 命令腳本。
2. 從真實地名的```cities.txt```隨機加入干擾的```simulate_command.txt``` 命令腳本。

產生不同排序方法的資料集：
1. original: 原版本未修改的```cities.txt```資料集。
2. dictionary: 按字典排序的```cities_one_col.txt```資料集，將產生順序搜尋的 TST。
3. prefix(2): 按前 2 字作前綴分組並排序的```city_two_prefix.txt```資料集。
4. prefix(3): 按前 3 字作前綴分組並排序的```city_three_prefix.txt```資料集。




***使用  ```random_command.txt``` 的執行時間數據如下：***

![](https://i.imgur.com/qtOljlT.png)

***使用  ```simulate_command.txt``` 的執行時間數據如下：***

![](https://i.imgur.com/3jmQEYN.png)

於是神奇的事情發生了。在搜尋隨機前綴時，按字典排序所產生的 TST 表現比使用前綴分組的 TST 更好。差距約為 10% 上下。
而使用真實地名時，按前綴分組的 TST 效能表現比按字典排序的 TST 更好，但效能差距約為 1.3% ~ 3.7%。

按 TST 的建立規則，如果以字典順序輸入資料，建成的 TST 效能將幾乎等於循序搜尋。然而實測顯示其效能比原版－－資料隨機排列的```cities.txt```資料集更好，比前綴分組更好或大致持平。此結果與直覺經驗不符。

為嘗試找出原因，以下在同樣條件下另外測量 cache-miss 次數。

***使用  ```random_command.txt``` 的 cache-miss 數據如下：***
![](https://i.imgur.com/xkxMZIl.png)

***使用  ```simulate_command.txt``` 的 cache-miss 數據如下：***
![](https://i.imgur.com/3z7uvxY.png)


不論是搜尋隨機前綴還是真實前綴，字典順序產生的 TST 所帶來的 cache-miss 都明顯較少。比前綴分組約減少 29% ~ 41%。

起因是字典順序產生的 TST 結果上形成了類似 array 的存取方式。

TST 屬於 linked-list 結構，內部各個 node 的空間並不保證連續。然而本例提供了 memory-pool 機制，其本意是減少頻繁進行 malloc 的開銷。但也側面保證了 TST 中各個 node 的連續性，因為只有 TST 會使用到本例中的 memory-pool。

如往 TST 中按字典順序加入七筆資料```A```,```B```,```C```,```D```,```E```,```F```,```G```。則在本例其分佈如下圖，而且每次搜尋都必定從```A```開始循序存取，此存取方式有利於 cache 的運作。

![](https://i.imgur.com/J3e4yKP.jpg)

如按前綴分組，則組別內部的資料排序將盡可能滿足產生平衡 TST，如下圖。順序為```D```,```B```,```A```,```C```,```F```,```E```,```G```。搜尋必定從```D```開始，但存取路徑不保證循序，而且不固定。由於不是每次都會存取鄰近的資料，因此cache miss 數量較多也是合理的。

![](https://i.imgur.com/tvXpjs3.jpg)

### 小結

演算法固然重要，然而演算法課上沒教到的硬體特性同樣重要。即便在演算法的角度上，平衡樹的計算量會比較少。在考量硬體因素後，更多的 cache miss 卻把效能拖低至循序搜尋的水平了。


---
## cpy vs ref

首先從 `test_cpy.c` 中， `#define REF INS` 其中 INS 為 0。所以 REF 為 0 ，CPY 為 1。
接著在 `tst_ins_del()` 的第四個參數才會引入 REF 或是 CPY，兩者的差別主要在於
- CPY 會執行下面這段程式碼，由於 CPY 並沒有使用 memory pool ，所以需要配置一段空間去接收要加入字典中的字串，這邊使用 `strdup(s)` ，該函式會配置一段空間，並且將引入的 s 的字串複製到新空間上。
```cpp
const char *eqdata = strdup(s);
if (!eqdata)
    return NULL;
curr->eqkid = (tst_node *) eqdata;
return (void *) eqdata;
```

- REF 會執行下面這段程式碼，主要就是將節點和字串串連起來。因為原本的字串已經配置在一塊 memory pool 中，所以不需要再額外配置空間。
```cpp
curr->eqkid = (tst_node *) s;
return (void *) s;
```

這兩者的差別主要在於，在每次加入新的字串的時候，需不需要額外配置空間。

- CPY 機制: 在每次加入時將地名暫存至同一 word 字串變數。
- REF 機制: 在每次加入時，將地名依序暫存至 memory pool 中（直到 memory pool 被 free）。

以下可以看到，如果執行 10000 次實驗，紅色為 REF ，綠色為 CPY ，時間差並不明顯，但依然會有一些效能上的差異。



![](https://i.imgur.com/S8sIFzO.png)
:::info
紅色: ref
綠色: cpy
X 軸: 每次執行時間
Y 軸: 執行次數
:::

---


## test_common.c
參考 [yxguo2563 同學的共筆](https://hackmd.io/@yxguo/r1iqjB25B)

將 `test_cpy.c` 和 `test_ref.c` 整合成一份，使用 `Macro` 和編譯時使用 `-D REFMODE` 來搭配，如果有使用該參數則使用 REF 機制，否則使用 CPY 機制。

程式碼實作部份更改如下，首先在一開始會對 `REFMODE` 處理，主要差別在於 memory pool 的使用與否。在 tst 插入節點的部份，會因為是 REF 或是 CPY 而有所不同，因此使用 `MODE` 來搭配。

```cpp
#ifdef REFMODE
#define MODE 0
long poolsize = 2000000 * WRDMAX;
#define BENCH_TEST_FILE "bench_ref.txt"
#else //default cpy
#define MODE 1
#define BENCH_TEST_FILE "bench_cpy.txt"
#endif

...
#ifdef REFMODE
char *pool = (char *) malloc(poolsize * sizeof(char));
char *buf = pool;
#else
char *buf = word;
#endif

if (!tst_ins_del(&root, name, INS, MODE))
```

### 更換 hash function
在 hash function 的選擇上，參考[這篇](https://www.byvoid.com/zht/blog/string-hash-compare)，並且使用字典的字串進行實驗，測試每個 hash function 碰撞的次數。.

```shell
$ ./hash

the collision of SDBM hash is: 0
the collision of RS hash is: 3
the collision of JS hash is: 11
the collision of PJW hash is: 216
the collision of ELF hash is: 216
the collision of BKDR hash is: 1
the collision of AP hash is: 0
the collision of DJB hash is: 4
the collision of Jenkins hash is: 2
the collision of DJB2 hash is: 4
```
