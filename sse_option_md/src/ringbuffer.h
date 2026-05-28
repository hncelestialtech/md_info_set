#pragma once


#include <atomic>
#include <array>
#include <cstdint>
#include <cstring>



#define L1_MD  1
#define L2_MD  2


#pragma pack(push, 1)




struct DataQuote
{
    uint64_t extime;
    char     inst[20];
    double   last_price;
    int32_t  openinterest;    
    int32_t  volume;
    double   turnover;
    double   bp1;
    double   bp2;
    double   bp3;
    double   bp4;
    double   bp5;
    double   ap1;
    double   ap2;
    double   ap3;
    double   ap4;
    double   ap5;
    uint32_t bv1;
    uint32_t bv2;
    uint32_t bv3;
    uint32_t bv4;
    uint32_t bv5;
    uint32_t av1;
    uint32_t av2;
    uint32_t av3;
    uint32_t av4;
    uint32_t av5;

    DataQuote(){}

    DataQuote(uint64_t extime, char *inst, double last_price, int32_t openinterest,int32_t volume, double turnover, 
            double bp1,double ap1,uint32_t bv1,uint32_t av1)
             :extime(extime),last_price(last_price),openinterest(openinterest),volume(volume),turnover(turnover),
              bp1(bp1),ap1(ap1),bv1(bv1),av1(av1){
        memcpy(this->inst, inst, strlen(inst));
        inst[strlen(inst)] = '\0';
        // std::clog <<__func__<<","<< __LINE__<<",inst:"<<this->inst<<",inst:"<<inst<< std::endl;

    }


    DataQuote(uint64_t extime, char *inst, double last_price, int32_t openinterest,int32_t volume, double turnover, 
            double bp1, double bp2, double bp3, double bp4, double bp5,
            double ap1, double ap2, double ap3, double ap4, double ap5,
            uint32_t bv1, uint32_t bv2, uint32_t bv3, uint32_t bv4, uint32_t bv5, 
            uint32_t av1, uint32_t av2, uint32_t av3, uint32_t av4, uint32_t av5)
             :extime(extime),last_price(last_price),openinterest(openinterest),volume(volume),turnover(turnover),
              bp1(bp1),bp2(bp2),bp3(bp3),bp4(bp4),bp5(bp5),
              ap1(ap1),ap2(ap2),ap3(ap3),ap4(ap4),ap5(ap5),
              bv1(bv1),bv2(bv2),bv3(bv3),bv4(bv4),bv5(bv5),
              av1(av1),av2(av2),av3(av3),av4(av4),av5(av5){
        memcpy(this->inst, inst, strlen(inst));
        inst[strlen(inst)] = '\0';
        // std::clog <<__func__<<","<< __LINE__<<",inst:"<<this->inst<<",inst:"<<inst<< std::endl;
    }

    std::string ToString(){
        std::string ret(inst);
        ret = ret + ","+std::to_string(extime)+","+std::string(inst)+","+std::to_string(last_price)+","+std::to_string(openinterest)+","+ std::to_string(volume)+","+ std::to_string(turnover)
                  + ","+std::to_string(bp1)+","+std::to_string(bp2)+","+std::to_string(bp3)+","+std::to_string(bp4)+","+std::to_string(bp5)
                  + ","+std::to_string(ap1)+","+std::to_string(ap2)+","+std::to_string(ap3)+","+std::to_string(ap4)+","+std::to_string(ap5)
                  + ","+std::to_string(bv1)+","+std::to_string(bv2)+","+std::to_string(bv3)+","+std::to_string(bv4)+","+std::to_string(bv5)
                  + ","+std::to_string(av1)+","+std::to_string(av2)+","+std::to_string(av3)+","+std::to_string(av4)+","+std::to_string(av5);
        return ret;
    }
};



struct Slot {
    uint64_t sequence;
    uint64_t extime;
    uint64_t timestamp_0;
    uint64_t timestamp_1;
    uint32_t res;
    uint32_t  msg_type;
    DataQuote data;
};

#pragma pack(pop)

// 环形缓冲区大小 - 使用2的幂次方以便位运算优化
constexpr size_t RING_BUFFER_SIZE = 8192;

class LockFreeRingBuffer {
private:
    // 缓存行对齐避免false sharing
    struct alignas(64) RingBuffer {
        std::array<Slot, RING_BUFFER_SIZE> slots;
        std::atomic<uint64_t> write_idx{0};
        std::atomic<uint64_t> read_idx{0};
        std::atomic<uint64_t> sequence{0}; // 全局序列号
    };

    RingBuffer m_buffer;

    static constexpr uint64_t INDEX_MASK = RING_BUFFER_SIZE - 1;

    __attribute__((always_inline)) uint64_t get_index(uint64_t seq) const {
        return seq & INDEX_MASK;
    }


    __attribute__((always_inline)) bool is_full(uint64_t read_idx, uint64_t write_idx) const {
        return (write_idx - read_idx) >= RING_BUFFER_SIZE;
    }

public:
    LockFreeRingBuffer() = default;
    ~LockFreeRingBuffer() = default;


    LockFreeRingBuffer(const LockFreeRingBuffer&) = delete;
    LockFreeRingBuffer& operator=(const LockFreeRingBuffer&) = delete;

    __attribute__((always_inline)) void push_l1(const DataQuote& msgdata, uint64_t extime, uint64_t t0, uint64_t t1, uint32_t res) {
        uint64_t seq = m_buffer.sequence.fetch_add(1, std::memory_order_relaxed);
        
        while (true) {
            uint64_t current_write = m_buffer.write_idx.load(std::memory_order_relaxed);
            uint64_t current_read = m_buffer.read_idx.load(std::memory_order_acquire);
            
            if (!is_full(current_read, current_write)) {
                uint64_t write_pos = get_index(current_write);
                Slot& slot = m_buffer.slots[write_pos];
                
                // 写入数据
                slot.sequence = seq;
                slot.extime  =  extime;
                slot.timestamp_0 = t0;
                slot.timestamp_1 = t1;
                slot.res = res;            
                slot.msg_type = L1_MD;
                memcpy(&slot.data, &msgdata, sizeof(DataQuote));
                //slot.l1_data = msgdata;

                if (m_buffer.write_idx.compare_exchange_weak(
                    current_write, current_write + 1,
                    std::memory_order_release,
                    std::memory_order_relaxed)) {
                    return;
                }
            } else {
                // 队列满时的等待策略
                asm volatile("pause" ::: "memory");
            }
        }
    }

    //推送L2数据到队列
    __attribute__((always_inline)) void push_l2(const DataQuote& msgdata, uint64_t extime, uint64_t t0, uint64_t t1, uint32_t res) {
        uint64_t seq = m_buffer.sequence.fetch_add(1, std::memory_order_relaxed);
        
        while (true) {
            uint64_t current_write = m_buffer.write_idx.load(std::memory_order_relaxed);
            uint64_t current_read = m_buffer.read_idx.load(std::memory_order_acquire);
            
            if (!is_full(current_read, current_write)) {
                uint64_t write_pos = get_index(current_write);
                Slot& slot = m_buffer.slots[write_pos];
                
                slot.sequence = seq;
                slot.extime  =  extime;
                slot.timestamp_0 = t0;
                slot.timestamp_1 = t1;
                slot.res = res;
                slot.msg_type = L2_MD;
                memcpy(&slot.data, &msgdata, sizeof(DataQuote));
                // slot.l2_data = msgdata;

                if (m_buffer.write_idx.compare_exchange_weak(
                    current_write, current_write + 1,
                    std::memory_order_release,
                    std::memory_order_relaxed)) {
                    return;
                }
            } else {
                asm volatile("pause" ::: "memory");
            }
        }
    }

    __attribute__((always_inline)) bool try_pop(Slot& slot) {

        // 弹出数据
        uint64_t current_read = m_buffer.read_idx.load(std::memory_order_relaxed);
        uint64_t current_write = m_buffer.write_idx.load(std::memory_order_acquire);
        
        if (current_read != current_write) {
            uint64_t read_pos = get_index(current_read);
            const Slot& current_slot = m_buffer.slots[read_pos];
            
            // 拷贝数据
            std::memcpy(&slot, &current_slot, sizeof(Slot));
            
            // 更新读取索引
            m_buffer.read_idx.store(current_read + 1, std::memory_order_release);
            return true;
        }
        return false;
    }

    //弹出数据（阻塞版本）
    __attribute__((flatten)) void pop(Slot& slot) {
        while (!try_pop(slot)) {
            asm volatile("pause" ::: "memory");
        }
    }

    //获取队列中元素数量
    __attribute__((always_inline)) uint64_t size() const {
        // 使用memory_order_acquire确保读取到最新的write_idx
        uint64_t current_write = m_buffer.write_idx.load(std::memory_order_acquire);
        uint64_t current_read = m_buffer.read_idx.load(std::memory_order_acquire);
        return current_write - current_read;
    }

    //检查队列是否为空
    __attribute__((always_inline)) bool empty() const {
        // 使用宽松内存序，因为空检查不需要严格同步
        uint64_t current_read = m_buffer.read_idx.load(std::memory_order_relaxed);
        uint64_t current_write = m_buffer.write_idx.load(std::memory_order_relaxed);
        return current_read == current_write;
    }

    //检查队列是否已满
    __attribute__((always_inline)) bool full() const {
        uint64_t current_read = m_buffer.read_idx.load(std::memory_order_acquire);
        uint64_t current_write = m_buffer.write_idx.load(std::memory_order_relaxed);
        return is_full(current_read, current_write);
    }

    //获取队列容量
    constexpr size_t capacity() const {
        return RING_BUFFER_SIZE;
    }

    //获取可用空间
    __attribute__((always_inline)) uint64_t available() const {
        return RING_BUFFER_SIZE - size();
    }

    uint64_t get_next_expected_sequence() const {
        return m_buffer.read_idx.load(std::memory_order_relaxed);
    }


};

