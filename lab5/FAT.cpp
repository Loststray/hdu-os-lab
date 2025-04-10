#include <cassert>
#include <cstring>
#include <ios>
#include <iostream>
#include <memory>

using uchar = unsigned char;
using ushort = unsigned short;

struct FCB {
    char name[8];
    char exname[3];
    uchar attribute;
    ushort time;
    ushort date;
    ushort first_block;
};

class FAT16 {
  private:
    static const int BLOCK_SIZE = 1024;   // 每个块的大小
    static const int BLOCK_COUNT = 1000;  // 块的总数
    uchar vdisk[BLOCK_COUNT][BLOCK_SIZE]; // 模拟磁盘

    struct BLOCK0 {
        char info[256];
        ushort root;
        uchar *start_block;
        BLOCK0(const char *str, ushort _root, uchar *ptr)
            : root(_root), start_block(ptr) {
            memcpy(info, str, sizeof(char) * 256);
        }
    };
    BLOCK0 *blk0_ptr;
    FCB* fcb_ptr;

  public:
    FAT16() {
        memset(vdisk, 0, sizeof(vdisk));
        blk0_ptr =
            std::construct_at(reinterpret_cast<BLOCK0 *>(vdisk[0]), "wocaonima",
                              3, reinterpret_cast<uchar *>(vdisk[3]));
        fcb_ptr = reinterpret_cast<FCB*>(vdisk[3]);
        
    }
};

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);
    FAT16 wocao;
}