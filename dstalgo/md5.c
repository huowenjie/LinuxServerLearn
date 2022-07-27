#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "trace.h"
#include "algos.h"

// 左移数 S[i]
static uint8_t shift_len[64] = { 
    7, 12, 17, 22,
    7, 12, 17, 22,
    7, 12, 17, 22,
    7, 12, 17, 22,

    5, 9,  14, 20,
    5, 9,  14, 20,
    5, 9,  14, 20,
    5, 9,  14, 20,

    4, 11 ,16, 23,
    4, 11 ,16, 23,
    4, 11 ,16, 23,
    4, 11 ,16, 23,

    6, 10, 15, 21,
    6, 10, 15, 21,
    6, 10, 15, 21,
    6, 10, 15, 21
 };

// 常量 t[i]
static uint32_t calc_num[64] = { 
    0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee,
    0xf57c0faf, 0x4787c62a, 0xa8304613, 0xfd469501,
    0x698098d8, 0x8b44f7af, 0xffff5bb1, 0x895cd7be,
    0x6b901122, 0xfd987193, 0xa679438e, 0x49b40821,

    0xf61e2562, 0xc040b340, 0x265e5a51, 0xe9b6c7aa,
    0xd62f105d, 0x02441453, 0xd8a1e681, 0xe7d3fbc8,
    0x21e1cde6, 0xc33707d6, 0xf4d50d87, 0x455a14ed,
    0xa9e3e905, 0xfcefa3f8, 0x676f02d9, 0x8d2a4c8a,

    0xfffa3942, 0x8771f681, 0x6d9d6122, 0xfde5380c,
    0xa4beea44, 0x4bdecfa9, 0xf6bb4b60, 0xbebfbc70,
    0x289b7ec6, 0xeaa127fa, 0xd4ef3085, 0x04881d05,
    0xd9d4d039, 0xe6db99e5, 0x1fa27cf8, 0xc4ac5665,

    0xf4292244, 0x432aff97, 0xab9423a7, 0xfc93a039,
    0x655b59c3, 0x8f0ccc92, 0xffeff47d, 0x85845dd1,
    0x6fa87e4f, 0xfe2ce6e0, 0xa3014314, 0x4e0811a1,
    0xf7537e82, 0xbd3af235, 0x2ad7d2bb, 0xeb86d391 
};

// 循环左移
static inline uint32_t shift_left_loop(uint32_t data, int num)
{
    uint32_t ret = 0;

    num = num % 32;
    ret = (data << num) | (data >> (32 - num));
    return ret;
}

static inline uint32_t md5_calc_non_linear(
    uint32_t a, uint32_t b, uint32_t c, uint32_t d, uint32_t sub_msg, uint32_t index,
    uint32_t (* md5_calc)(uint32_t, uint32_t, uint32_t))
{
    if (!md5_calc) {
        ERR_PRINT("FUNCTION POINTER IS NULL!");
        return 0;
    }

    uint32_t tmp = 
        md5_calc(b, c, d) + 
        a + sub_msg + calc_num[index];

    // 循环左移不定位数
    tmp = shift_left_loop(tmp, shift_len[index]);

    return tmp + b;
}

// 4 个非线性函数
static inline uint32_t md5_calc_f(uint32_t x, uint32_t y, uint32_t z)
{
    return (x & y) | ((~x) & z);
}

static inline uint32_t md5_calc_g(uint32_t x, uint32_t y, uint32_t z)
{
    return (x & z) | (y & (~z));
}

static inline uint32_t md5_calc_h(uint32_t x, uint32_t y, uint32_t z)
{
    return x ^ y ^ z;
}

static inline uint32_t md5_calc_i(uint32_t x, uint32_t y, uint32_t z)
{
    return y ^ (x | (~z));
}

// MD5 主循环, 512 位数据位一组， 进行四轮运算，总共 64 次循环
static int md5_loop(MD5_CTX *ctx, const uint32_t *sub_msg)
{
    if (!ctx || !sub_msg) {
        ERR_PRINT("INVALID ARGS!");
        return FAILED;
    }

    uint8_t ia = 0;
    uint8_t ib = 0;
    uint8_t ic = 0;
    uint8_t id = 0;
    uint8_t im = 0;
    uint32_t i = 0;

    uint32_t chain[4] = { 
        ctx->A,
        ctx->B,
        ctx->C,
        ctx->D
     };

    // MD5 算法循环
    while (i < 64) {
        ia = (64 - i) % 4;
        ib = (64 - i + 1) % 4;
        ic = (64 - i + 2) % 4;
        id = (64 - i + 3) % 4;

        if (i < 16) {
            im = i;
            chain[ia] = md5_calc_non_linear(chain[ia], chain[ib], chain[ic], chain[id], sub_msg[im], i, md5_calc_f);
        } else if (i > 15 && i < 32) {
            im = ((i % 16) * 5 + 1) % 16;
            chain[ia] = md5_calc_non_linear(chain[ia], chain[ib], chain[ic], chain[id], sub_msg[im], i, md5_calc_g);
        } else if (i > 31 && i < 48) {
            im = ((i % 16) * 3 + 5) % 16;
            chain[ia] = md5_calc_non_linear(chain[ia], chain[ib], chain[ic], chain[id], sub_msg[im], i, md5_calc_h);
        } else {
            im = ((i % 16) * 7) % 16;
            chain[ia] = md5_calc_non_linear(chain[ia], chain[ib], chain[ic], chain[id], sub_msg[im], i, md5_calc_i);
        }

        i++;
    }

    ctx->A += chain[0];
    ctx->B += chain[1];
    ctx->C += chain[2];
    ctx->D += chain[3];

    return SUCCESS;
}

int md5_init(MD5_CTX *ctx)
{
    if (!ctx) {
        ERR_PRINT("NULL_POINTER MD5_CTX!");
        return FAILED;
    }

    memset(ctx, 0, sizeof(MD5_CTX));

    // 初始化链接变量, 大端法字节序
    ctx->A = 0x67452301;
    ctx->B = 0xefcdab89;
    ctx->C = 0x98badcfe;
    ctx->D = 0x10325476;

    return SUCCESS;
}

int md5_update(MD5_CTX *ctx, const uint8_t *msg, uint32_t len)
{
    if (!ctx) { ERR_PRINT("NULL_POINTER MD5_CTX!"); return FAILED; }
    if (!msg) { ERR_PRINT("NULL_POINTER MD5 MSG!"); return FAILED; }
    if (!len) { ERR_PRINT("NULL_POINTER MSG LEN!"); return FAILED; }

    /*
     * 1.比较消息长度与分组长度，如果消息长度小于分组长度则直接拷贝至缓冲区
     *  然后在 final 阶段进行最终填充和运算；
     * 2.如果消息长度大于分组长度，则首先计算整数倍分组长度的数据，剩余的数据
     *  拷贝至缓冲区等待 final 阶段运算；
     * 3.如果是多次调用，每次需要累计缓冲区中的数据，然后最终进行填充和运算；
     * 
     * 总之，仅填充一次
     */

    // 累计消息摘要的总长度，是否会溢出？
    ctx->msg_len += len;

    // 待处理数据长度
    uint32_t msg_len = len;

    // 缓冲区剩余长度
    uint32_t res_len = ctx->len;

    const uint8_t *mp = msg;
    uint8_t *bp = ctx->buff;

    // 调用时有缓冲区剩余数据，则优先处理剩余数据
    if (res_len > 0) {
        // 如果当前剩余数据和新计算的数据长度之和小于缓冲区则将数据拷贝至缓冲区
        if (res_len + msg_len < MD5_BLOCK) {
            // 拷贝数据到缓冲区
            memcpy(bp + res_len, msg, msg_len);

            res_len += msg_len;
            msg_len = 0;

            ctx->len = res_len;
        } else {

            uint32_t buf_len = MD5_BLOCK - res_len;

            // 先拷贝一部分到缓冲区，让缓冲区填满
            memcpy(bp + res_len, mp, buf_len);

            // 直接进行一次运算
            md5_loop(ctx, (const uint32_t *)bp);

            // 更新上下文
            mp += buf_len;
            msg_len -= buf_len;

            // 运算结束之后，清空缓冲区
            res_len = 0;
            ctx->len = res_len;
            memset(ctx->buff, 0, sizeof(MD5_BLOCK));
        }
    }

    // 缓冲区没有剩余数据的情况
    if (res_len == 0) {
        // 首先数据可以分几组
        uint32_t n = msg_len / MD5_BLOCK;

        if (n > 0) {
            // 直接进行运算，然后统计大小
            uint32_t num = n * MD5_BLOCK / 4;
            uint32_t *p = (uint32_t *)mp;

            // 对每个子分组进行运算
            for (uint32_t i = 0; i < num; i += 16) {
                md5_loop(ctx, p + i);
            }

            // 运算完毕后统计剩余数据的大小
            res_len = msg_len - n * MD5_BLOCK;
        } else {
            // 剩余数据大小
            res_len = msg_len;
        }
        ctx->len = res_len;
    }

    // 将剩余数据拷贝至缓冲区
    if (msg_len > 0) {
        memcpy(bp, mp + (msg_len - res_len), res_len);
    }

    return SUCCESS;
}

int md5_final(MD5_CTX *ctx, uint8_t *result, uint32_t *len)
{
    if (!ctx) { 
        ERR_PRINT("NULL_POINTER MD5_CTX!"); 
        return FAILED; 
    }

    if (!result) {
        *len = 4 * sizeof(uint32_t);
        return SUCCESS;
    }

    if (*len < 4 * sizeof(uint32_t)) {
        ERR_PRINT("BUFF NOT ENOUGH!"); 
        return FAILED; 
    }

    // 填充消息长度为 64 字节(512 位)的整数倍小 8 字节的值
    uint8_t *buff = (uint8_t *)ctx->buff;
    uint32_t num = ctx->len;

    // num 必定小于 64 所以有
    buff[num] = 0x80;
    num++;

    // 如果当前剩余长度不足以存储一个64位的数，则需要全部填充 0 且再做一次摘要运算
    if (num > (MD5_BLOCK - 8)) {
        memset(buff + num, 0, MD5_BLOCK - num);

        // 计算倒数第二个分组
        md5_loop(ctx, (const uint32_t *)buff);

        // 剩余数据清 0
        num = 0;
    }

    // 填充最后一个分组
    memset(buff + num, 0, MD5_BLOCK - num - 8);

    // 最后 8 字节填充数据长度 (单位：bit)
    uint64_t *len_pt = (uint64_t *)(buff + MD5_BLOCK - 8);

    // 这里要是否会溢出？
    *len_pt = ctx->msg_len << 3;

    // 计算最后一个分组数据
    md5_loop(ctx, (const uint32_t *)buff);
    num = 0;
    buff = NULL;

    uint32_t *p = (uint32_t *)result;

    p[0] = ctx->A;
    p[1] = ctx->B;
    p[2] = ctx->C;
    p[3] = ctx->D;

    // 清理数据
    memset(ctx, 0, sizeof(MD5_CTX));
    return SUCCESS;
}

int md5_msg(const uint8_t *in_msg, uint32_t in_len, uint8_t *out_msg, uint32_t *out_len)
{
    MD5_CTX ctx;
    int ret = SUCCESS;

    if (!out_len) {
        ERR_PRINT("OUT LEN IS NULL!");
        return FAILED;
    }

    if (!out_msg) {
        *out_len = 4 * sizeof(uint32_t);
        return SUCCESS;
    }

    if (*out_len < 4 * sizeof(uint32_t)) {
        ERR_PRINT("BUFF NOT ENOUGH!"); 
        return FAILED; 
    }

    ret = md5_init(&ctx);
    if (ret != SUCCESS) {
        ERR_PRINT("md5_init ERROR!"); 
        return ret;
    }

    ret = md5_update(&ctx, in_msg, in_len);
    if (ret != SUCCESS) {
        ERR_PRINT("md5_update ERROR!"); 
        return ret;
    }

    ret = md5_final(&ctx, out_msg, out_len);
    if (ret != SUCCESS) {
        ERR_PRINT("md5_final ERROR!"); 
    }

    return ret;
}
