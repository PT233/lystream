/**
 * @brief 图像队列-3个plan
 * @date 2022/11/28
 * @author laoyao
 */

#pragma once

#include <array>
#include <atomic>
#include <condition_variable>

class Stream3PDataDeque
{
  public:

    typedef struct {
        unsigned char *img[3];
        size_t datalen[3];
    } StreamData;

    explicit Stream3PDataDeque(const int ele_size[3]);

    virtual ~Stream3PDataDeque();

    void PushFront(StreamData data);

    void PopBack(StreamData *data);

  private:

#define MAX_RAWDEQUE 3
#define MAX_QUEUE_3P 3

    [[nodiscard]] bool isNotEmpty() const;

    [[nodiscard]] bool isNotFull() const;

    std::array<StreamData, 3> img_deque_ {};

    int read_index_ = 0, write_index_ = 0;
    int unread_ = 0;

    std::condition_variable not_empty_;
    std::condition_variable not_full_;
    std::mutex mtx_;
};