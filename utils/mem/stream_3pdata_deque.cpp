/**
 * @brief 图像队列-3个plan
 * @date 2022/11/28
 * @author laoyao
 */

#include "stream_3pdata_deque.h"

#include <cstring>
#include <functional>

Stream3PDataDeque::Stream3PDataDeque(const int ele_size[3])
{
    for (int i = 0; i < MAX_RAWDEQUE; i++) {
        StreamData img_unit;
        for (int plane = 0; plane < MAX_QUEUE_3P; plane++) {
            img_unit.img[plane]     = new unsigned char[ele_size[plane]];
            img_unit.datalen[plane] = ele_size[plane];
        }
        img_deque_.at(i) = img_unit;
    }
}

Stream3PDataDeque::~Stream3PDataDeque()
{
    for (int i = 0; i < MAX_RAWDEQUE; i++) {
        StreamData &img_unit = img_deque_.at(i);
        for (auto &plane : img_unit.img) { delete[] plane; }
    }
}

void Stream3PDataDeque::PushFront(StreamData data)
{
    //    std::unique_lock<std::mutex> lock(mtx_);
    //    not_full_.wait(lock, std::bind(&Stream3PDataDeque::isNotFull, this));

    StreamData &img_unit = img_deque_.at(write_index_++);
    for (int plane = 0; plane < MAX_QUEUE_3P; plane++) {
        memcpy(img_unit.img[plane], data.img[plane], data.datalen[plane]);
        img_unit.datalen[plane] = data.datalen[plane];
    }

    if (write_index_ >= MAX_RAWDEQUE) { write_index_ = 0; }

    unread_++;
    not_empty_.notify_one();
}

void Stream3PDataDeque::PopBack(StreamData *data)
{
    //    std::unique_lock<std::mutex> lock(mtx_);
    //    not_empty_.wait(lock, std::bind(&Stream3PDataDeque::isNotEmpty, this));

    StreamData &img_unit = img_deque_.at(read_index_++);

    for (int plane = 0; plane < MAX_QUEUE_3P; plane++) {
        memcpy(data->img[plane], img_unit.img[plane], img_unit.datalen[plane]);
        data->datalen[plane] = img_unit.datalen[plane];
    }

    if (read_index_ >= MAX_RAWDEQUE) { read_index_ = 0; }

    unread_--;
    not_full_.notify_one();
}

bool Stream3PDataDeque::isNotEmpty() const
{
    return unread_ > 0;
}

bool Stream3PDataDeque::isNotFull() const
{
    return unread_ < MAX_RAWDEQUE;
}