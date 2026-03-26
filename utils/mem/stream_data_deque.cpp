/**
 * @brief 流数据队列
 * @date 2022/11/28
 * @author laoyao
 */

#include "stream_data_deque.h"

#include <cstring>
#include <functional>

StreamDataDeque::StreamDataDeque(int ele_size)
{
    for (int i = 0; i < MAX_RAWDEQUE; i++) {
        StreamData stream_data;
        stream_data.data       = new unsigned char[ele_size];
        stream_data.datalen    = ele_size;
        stream_data.extra_data = nullptr;

        stream_data_deque_.at(i) = stream_data;
    }
}

StreamDataDeque::~StreamDataDeque()
{
    for (int i = 0; i < MAX_RAWDEQUE; i++) {
        StreamData &stream_data = stream_data_deque_.at(i);
        delete[] stream_data.data;
    }
}

void StreamDataDeque::PushFront(StreamData data)
{
    std::unique_lock<std::mutex> lock(mtx_);

    not_full_.wait(lock, std::bind(&StreamDataDeque::isNotFull, this));

    StreamData &stream_data = stream_data_deque_.at(write_index_++);

    memcpy(stream_data.data, data.data, data.datalen);
    stream_data.datalen    = data.datalen;
    stream_data.extra_data = data.extra_data;

    if (write_index_ >= MAX_RAWDEQUE) { write_index_ = 0; }

    unread_++;
    not_empty_.notify_one();
}

void StreamDataDeque::PopBack(StreamData *data)
{
    std::unique_lock<std::mutex> lock(mtx_);
    not_empty_.wait(lock, std::bind(&StreamDataDeque::isNotEmpty, this));

    StreamData &stream_data = stream_data_deque_.at(read_index_++);

    memcpy(data->data, stream_data.data, stream_data.datalen);
    data->datalen    = stream_data.datalen;
    data->extra_data = stream_data.extra_data;

    if (read_index_ >= MAX_RAWDEQUE) { read_index_ = 0; }

    unread_--;
    not_full_.notify_one();
}

bool StreamDataDeque::isNotEmpty() const
{
    return unread_ > 0;
}

bool StreamDataDeque::isNotFull() const
{
    return unread_ < MAX_RAWDEQUE;
}