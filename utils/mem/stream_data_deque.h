/**
 * @brief 流数据队列
 * @date 2022/11/28
 * @author laoyao
 */

#pragma once

#include <array>
#include <condition_variable>

class StreamDataDeque
{
  public:

    typedef struct {
        unsigned char *data;
        size_t datalen;
        void *extra_data;
    } StreamData;

    explicit StreamDataDeque(int ele_size);
    virtual ~StreamDataDeque();

    void PushFront(StreamData data);
    void PopBack(StreamData *data);

  private:

#define MAX_RAWDEQUE 3

    [[nodiscard]] bool isNotEmpty() const;
    [[nodiscard]] bool isNotFull() const;

    std::array<StreamData, MAX_RAWDEQUE> stream_data_deque_ {};

    int read_index_ = 0, write_index_ = 0;
    int unread_ = 0;

    std::condition_variable not_empty_;
    std::condition_variable not_full_;

    std::mutex mtx_;
};